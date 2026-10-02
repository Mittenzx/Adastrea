#include "Drones/LootDrone.h"
#include "Combat/RaidSubsystem.h"
#include "Rescue/RescueFlight.h"
#include "Ships/Spaceship.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"

namespace LootTuning
{
	/** Close enough to the work point to clamp on (cm). */
	constexpr float ClampDistance = 150.0f;
	/** Close enough to a raider to land (cm). */
	constexpr float LandDistance = 400.0f;
	/** The drone rides this far above the hull top (cm). */
	constexpr float HullStandoff = 90.0f;
	/** Slots spread along the hull by this share of its length. */
	constexpr float SlotSpread = 0.25f;
	/** A drone with nowhere to land gives up after this long (s). */
	constexpr float MaxSeconds = 120.0f;
}

ALootDrone::ALootDrone()
	: Speed(7000.0f)
	, Acceleration(6000.0f)
	, WorkSeconds(3.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	SetCanBeDamaged(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HullMat(TEXT("/Game/Materials/M_Mining_Hull.M_Mining_Hull"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LampMat(TEXT("/Game/Materials/M_NavLights.M_NavLights"));

	auto MakePart = [this](const TCHAR* Name, UStaticMesh* Mesh, UMaterialInterface* Mat)
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Root);
		Part->SetStaticMesh(Mesh);
		if (Mat)
		{
			Part->SetMaterial(0, Mat);
		}
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetGenerateOverlapEvents(false);
		Part->SetMobility(EComponentMobility::Movable);
		return Part;
	};

	// Engine shapes, +X forward: a flat body with a cargo pod slung under it.
	Body = MakePart(TEXT("Body"), CubeMesh.Object, HullMat.Object);
	Body->SetRelativeScale3D(FVector(2.0f, 1.6f, 0.6f));

	Pod = MakePart(TEXT("Pod"), CylinderMesh.Object, HullMat.Object);
	Pod->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f));
	Pod->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	Pod->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.6f));

	Lamp = MakePart(TEXT("Lamp"), SphereMesh.Object, LampMat.Object);
	Lamp->SetRelativeLocation(FVector(90.0f, 0.0f, 40.0f));
	Lamp->SetRelativeScale3D(FVector(0.35f));
	Lamp->SetCastShadow(false);
}

void ALootDrone::StartJob(int32 InRaidId, ASpaceship* InHome, ASpaceship* InTarget, int32 InSlot)
{
	RaidId = InRaidId;
	Home = InHome;
	Target = InTarget;
	Slot = InSlot;
	State = ELootDroneState::Outbound;
	StateTime = 0.0f;
	Velocity = InHome ? InHome->GetVelocity() : FVector::ZeroVector;
	UE_LOG(LogAdastreaCombat, Log, TEXT("Loot drone %s launched from %s for %s"), *GetName(), *GetNameSafe(InHome), *GetNameSafe(InTarget));
}

FVector ALootDrone::GetWorkPoint() const
{
	const ASpaceship* Ship = Target.Get();
	if (!Ship || !Ship->ShipRoot)
	{
		return GetActorLocation();
	}
	const FBox Hull = Ship->GetHullLocalBounds();
	FTransform Frame = Ship->ShipRoot->GetComponentTransform();
	Frame.SetScale3D(FVector::OneVector);
	if (!Hull.IsValid)
	{
		return Frame.TransformPosition(FVector(0.0f, 0.0f, 300.0f));
	}
	const float Along = (Slot % 2 == 0 ? 1.0f : -1.0f) * Hull.GetExtent().X * LootTuning::SlotSpread * (1 + Slot / 2);
	return Frame.TransformPosition(FVector(Hull.GetCenter().X + Along, Hull.GetCenter().Y, Hull.Max.Z + LootTuning::HullStandoff));
}

void ALootDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	URaidSubsystem* Raids = URaidSubsystem::Get(this);
	ASpaceship* Ship = Target.Get();
	if (State != ELootDroneState::Returning && (!Ship || !Raids || !Raids->FindRaid(RaidId)))
	{
		// The target's gone (or the raid ended): go home with whatever is aboard.
		State = ELootDroneState::Returning;
		StateTime = 0.0f;
	}

	switch (State)
	{
	case ELootDroneState::Outbound:
		if (RescueFlight::FlyToward(this, Velocity, GetWorkPoint(), Ship->GetVelocity(), Speed, Acceleration, DeltaSeconds, true) < LootTuning::ClampDistance)
		{
			State = ELootDroneState::Working;
			StateTime = 0.0f;
			UE_LOG(LogAdastreaCombat, Log, TEXT("Loot drone %s clamped onto %s"), *GetName(), *Ship->GetName());
		}
		break;

	case ELootDroneState::Working:
		SetActorLocationAndRotation(GetWorkPoint(), Ship->GetActorRotation());
		Velocity = Ship->GetVelocity();
		if (StateTime >= WorkSeconds)
		{
			Raids->NotifyDroneAtTarget(this);
			State = ELootDroneState::Returning;
			StateTime = 0.0f;
		}
		break;

	case ELootDroneState::Returning:
	{
		ASpaceship* Landing = Raids ? Raids->GetDroneHome(RaidId, Home.Get()) : nullptr;
		if (!Landing || StateTime > LootTuning::MaxSeconds)
		{
			UE_LOG(LogAdastreaCombat, Log, TEXT("Loot drone %s has nowhere to land; its load is lost"), *GetName());
			Destroy();
			return;
		}
		Home = Landing;
		if (RescueFlight::FlyToward(this, Velocity, Landing->GetActorLocation(), Landing->GetVelocity(), Speed * 1.3f, Acceleration, DeltaSeconds, true) < LootTuning::LandDistance)
		{
			Raids->NotifyDroneHome(this);
			Destroy();
		}
		break;
	}
	}
}
