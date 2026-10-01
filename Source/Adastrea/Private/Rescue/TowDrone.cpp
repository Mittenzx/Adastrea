#include "Rescue/TowDrone.h"
#include "Rescue/RescueFlight.h"
#include "Ships/Spaceship.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"

namespace TowTuning
{
	/** Close enough to the clamp point to grab on (cm). */
	constexpr float ClampDistance = 150.0f;
	/** Seconds spent clamping and swinging the wreck round toward the station. */
	constexpr float ClampSeconds = 2.5f;
	/** Close enough to the drop point to call it delivered (cm). */
	constexpr float DropTolerance = 250.0f;
	/** Close enough to home to stow (cm). */
	constexpr float HomeTolerance = 400.0f;
	/** The drone rides this far ahead of the hull nose (cm). */
	constexpr float NoseStandoff = 120.0f;
}

ATowDrone::ATowDrone()
	: FreeSpeed(6000.0f)
	, TowSpeed(2500.0f)
	, Acceleration(5000.0f)
	, State(ETowDroneState::Outbound)
	, Velocity(FVector::ZeroVector)
	, DropPoint(FVector::ZeroVector)
	, Home(FVector::ZeroVector)
	, StateTime(0.0f)
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

	// Engine shapes, +X forward; the clamp arms reach back (-X) onto the hull. Engine cube
	// and sphere are 100 uu across; the cylinder is 100 uu tall along Z.
	Body = MakePart(TEXT("Body"), CubeMesh.Object, HullMat.Object);
	Body->SetRelativeScale3D(FVector(2.2f, 2.6f, 1.4f));

	ClampLeft = MakePart(TEXT("ClampLeft"), CylinderMesh.Object, HullMat.Object);
	ClampLeft->SetRelativeLocation(FVector(-150.0f, 110.0f, 0.0f));
	ClampLeft->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	ClampLeft->SetRelativeScale3D(FVector(0.3f, 0.3f, 1.6f));

	ClampRight = MakePart(TEXT("ClampRight"), CylinderMesh.Object, HullMat.Object);
	ClampRight->SetRelativeLocation(FVector(-150.0f, -110.0f, 0.0f));
	ClampRight->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	ClampRight->SetRelativeScale3D(FVector(0.3f, 0.3f, 1.6f));

	Lamp = MakePart(TEXT("Lamp"), SphereMesh.Object, LampMat.Object);
	Lamp->SetRelativeLocation(FVector(40.0f, 0.0f, 80.0f));
	Lamp->SetRelativeScale3D(FVector(0.4f));
	Lamp->SetCastShadow(false);
}

void ATowDrone::StartTow(ASpaceship* InWreck, const FVector& InDropPoint, const FVector& InHome)
{
	Wreck = InWreck;
	DropPoint = InDropPoint;
	Home = InHome;
	State = ETowDroneState::Outbound;
	StateTime = 0.0f;
	UE_LOG(LogAdastreaCombat, Log, TEXT("Tow drone %s launched for %s"), *GetName(), *GetNameSafe(InWreck));
}

void ATowDrone::Release()
{
	State = ETowDroneState::Returning;
	StateTime = 0.0f;
	Velocity = FVector::ZeroVector;
}

FVector ATowDrone::GetClampPointLocal() const
{
	const ASpaceship* Ship = Wreck.Get();
	const FBox Hull = Ship ? Ship->GetHullLocalBounds() : FBox(ForceInit);
	if (!Hull.IsValid)
	{
		return FVector(400.0f, 0.0f, 0.0f);
	}
	return FVector(Hull.Max.X + TowTuning::NoseStandoff, Hull.GetCenter().Y, Hull.GetCenter().Z);
}

FVector ATowDrone::GetClampPointWorld() const
{
	const ASpaceship* Ship = Wreck.Get();
	if (!Ship || !Ship->ShipRoot)
	{
		return GetActorLocation();
	}
	FTransform Frame = Ship->ShipRoot->GetComponentTransform();
	Frame.SetScale3D(FVector::OneVector);
	return Frame.TransformPosition(GetClampPointLocal());
}

void ATowDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	ASpaceship* Ship = Wreck.Get();
	if (!Ship && State != ETowDroneState::Returning)
	{
		Release();
	}

	switch (State)
	{
	case ETowDroneState::Outbound:
	{
		const float Dist = RescueFlight::FlyToward(this, Velocity, GetClampPointWorld(), Ship->GetVelocity(),
			FreeSpeed, Acceleration, DeltaSeconds, true);
		if (Dist < TowTuning::ClampDistance)
		{
			Ship->StopWreckDrift();
			State = ETowDroneState::Clamping;
			StateTime = 0.0f;
			UE_LOG(LogAdastreaCombat, Log, TEXT("Tow drone %s clamped onto %s"), *GetName(), *Ship->GetName());
		}
		break;
	}

	case ETowDroneState::Clamping:
	{
		// Swing the wreck's nose toward the drop point, riding on the nose.
		const FRotator Want = (DropPoint - Ship->GetActorLocation()).Rotation();
		Ship->SetActorRotation(FMath::RInterpTo(Ship->GetActorRotation(), Want, DeltaSeconds, 1.2f));
		SetActorLocationAndRotation(GetClampPointWorld(), Ship->GetActorRotation());
		if (StateTime >= TowTuning::ClampSeconds)
		{
			State = ETowDroneState::Towing;
			StateTime = 0.0f;
			Velocity = FVector::ZeroVector;
		}
		break;
	}

	case ETowDroneState::Towing:
	{
		// Haul the wreck itself; the drone stays clamped on its nose.
		const float Dist = RescueFlight::FlyToward(Ship, Velocity, DropPoint, FVector::ZeroVector,
			TowSpeed, Acceleration * 0.5f, DeltaSeconds, true);
		SetActorLocationAndRotation(GetClampPointWorld(), Ship->GetActorRotation());
		if (Dist < TowTuning::DropTolerance)
		{
			State = ETowDroneState::Arrived;
			StateTime = 0.0f;
			UE_LOG(LogAdastreaCombat, Log, TEXT("Tow drone %s delivered %s"), *GetName(), *Ship->GetName());
		}
		break;
	}

	case ETowDroneState::Arrived:
		SetActorLocationAndRotation(GetClampPointWorld(), Ship->GetActorRotation());
		break;

	case ETowDroneState::Returning:
		if (RescueFlight::FlyToward(this, Velocity, Home, FVector::ZeroVector, FreeSpeed, Acceleration, DeltaSeconds, true) < TowTuning::HomeTolerance)
		{
			Destroy();
		}
		break;
	}
}
