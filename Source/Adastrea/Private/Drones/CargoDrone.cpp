#include "Drones/CargoDrone.h"
#include "Drones/DroneBayComponent.h"
#include "Rescue/RescueFlight.h"
#include "Ships/Spaceship.h"
#include "Trading/TradeItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"

namespace CargoDroneTuning
{
	/** Distances (cm) at which a leg counts as reached. */
	constexpr float WaypointReach = 250.0f;
	constexpr float ClampDistance = 150.0f;
	constexpr float DockReach = 120.0f;
	/** The drone rides this far above the other hull's top (cm). */
	constexpr float HullStandoff = 90.0f;
	/** Slots spread along the hull by this share of its length. */
	constexpr float SlotSpread = 0.25f;
	/** Drones always get this much speed over the ship's, so they can catch it. */
	constexpr float ChaseSpeedFactor = 1.5f;
	/** A drone that can't get where it's going in this long lands at once. */
	constexpr float LegTimeout = 60.0f;
}

ACargoDrone::ACargoDrone()
	: Speed(6000.0f)
	, Acceleration(9000.0f)
	, WorkSeconds(2.5f)
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

void ACargoDrone::InitDrone(UDroneBayComponent* InBay, ASpaceship* InOther, int32 InSlot)
{
	Bay = InBay;
	Other = InOther;
	Slot = InSlot;
	Velocity = InBay ? InBay->GetShipVelocity() : FVector::ZeroVector;
	SetState(ECargoDroneState::Launching);
	Pod->SetVisibility(!Load.IsEmpty());
}

float ACargoDrone::GetLoadVolume() const
{
	float Volume = 0.0f;
	for (const FCargoEntry& Entry : Load)
	{
		Volume += Entry.Item ? Entry.Item->GetTotalVolume(Entry.Quantity) : 0.0f;
	}
	return Volume;
}

FVector ACargoDrone::GetWorkPoint() const
{
	const ASpaceship* Ship = Other.Get();
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
	const float Along = (Slot % 2 == 0 ? 1.0f : -1.0f) * Hull.GetExtent().X * CargoDroneTuning::SlotSpread * (1 + Slot / 2);
	return Frame.TransformPosition(FVector(Hull.GetCenter().X + Along, Hull.GetCenter().Y, Hull.Max.Z + CargoDroneTuning::HullStandoff));
}

void ACargoDrone::SetState(ECargoDroneState NewState)
{
	State = NewState;
	StateTime = 0.0f;
}

void ACargoDrone::ForceStow()
{
	if (UDroneBayComponent* B = Bay.Get())
	{
		B->NotifyCargoDroneHome(this);
		B->NotifyCargoDroneStowed(this);
	}
	Load.Reset();
	Destroy();
}

void ACargoDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	UDroneBayComponent* B = Bay.Get();
	if (!B)
	{
		Destroy();
		return;
	}
	const float MaxSpeed = FMath::Max(Speed, B->GetShipVelocity().Size() * CargoDroneTuning::ChaseSpeedFactor);
	const float Accel = FMath::Max(Acceleration, MaxSpeed * 2.0f);
	ASpaceship* Ship = Other.Get();
	if ((State == ECargoDroneState::Outbound || State == ECargoDroneState::Working) && (!Ship || !B->IsTransferTarget(Ship)))
	{
		// The job ended or the other ship is gone: home with whatever is aboard.
		SetState(ECargoDroneState::Returning);
	}

	switch (State)
	{
	case ECargoDroneState::Launching:
		if (RescueFlight::FlyToward(this, Velocity, B->GetHatchApproach(), B->GetShipVelocity(), MaxSpeed, Accel, DeltaSeconds, true) < CargoDroneTuning::WaypointReach)
		{
			SetState(Ship && B->IsTransferTarget(Ship) ? ECargoDroneState::Outbound : ECargoDroneState::Returning);
		}
		break;

	case ECargoDroneState::Outbound:
		if (RescueFlight::FlyToward(this, Velocity, GetWorkPoint(), Ship->GetVelocity(), MaxSpeed, Accel, DeltaSeconds, true) < CargoDroneTuning::ClampDistance)
		{
			SetState(ECargoDroneState::Working);
			UE_LOG(LogAdastrea, Verbose, TEXT("Cargo drone %s clamped onto %s"), *GetName(), *Ship->GetName());
		}
		else if (StateTime > CargoDroneTuning::LegTimeout)
		{
			SetState(ECargoDroneState::Returning);
		}
		break;

	case ECargoDroneState::Working:
		SetActorLocationAndRotation(GetWorkPoint(), Ship->GetActorRotation());
		Velocity = Ship->GetVelocity();
		if (StateTime >= WorkSeconds)
		{
			B->NotifyCargoDroneAtTarget(this, Ship);
			Pod->SetVisibility(!Load.IsEmpty());
			SetState(ECargoDroneState::Returning);
		}
		break;

	case ECargoDroneState::Returning:
		if (RescueFlight::FlyToward(this, Velocity, B->GetHatchApproach(), B->GetShipVelocity(), MaxSpeed, Accel, DeltaSeconds, true) < CargoDroneTuning::WaypointReach)
		{
			SetState(ECargoDroneState::Landing);
		}
		else if (StateTime > CargoDroneTuning::LegTimeout)
		{
			ForceStow();
		}
		break;

	case ECargoDroneState::Landing:
		if (RescueFlight::FlyToward(this, Velocity, B->GetHatchLocation(), B->GetShipVelocity(), MaxSpeed, Accel, DeltaSeconds, true) < CargoDroneTuning::DockReach
			|| StateTime > CargoDroneTuning::LegTimeout)
		{
			B->NotifyCargoDroneHome(this);
			// Out again with the next load while the job is on; otherwise stay aboard.
			if (B->ReloadCargoDrone(this))
			{
				Pod->SetVisibility(!Load.IsEmpty());
				SetState(ECargoDroneState::Launching);
			}
			else
			{
				B->NotifyCargoDroneStowed(this);
				Destroy();
			}
		}
		break;
	}
}
