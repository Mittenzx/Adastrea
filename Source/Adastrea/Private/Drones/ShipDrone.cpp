#include "Drones/ShipDrone.h"
#include "Drones/DroneBayComponent.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"
#include "Audio/AudioEventLibrary.h"

namespace ShipDroneTuning
{
	/** Distances (cm) at which a leg counts as reached. */
	constexpr float WaypointReach = 250.0f;
	constexpr float DockReach = 80.0f;
	/** Arrival gain: desired closing speed per cm of distance left (1/s). */
	constexpr float ArriveGain = 1.5f;
	/** Longest flight integration step (s). */
	constexpr float MaxStep = 1.0f / 30.0f;
	/** Drones always get at least this much speed over the ship's, so they can catch it. */
	constexpr float ChaseSpeedFactor = 1.5f;
	/** Spread of work spots around the side of the rock facing the ship (degrees). */
	constexpr float WorkSpotCone = 55.0f;
	/** A drone that can't get home in this long lands where it is. */
	constexpr float ReturnTimeout = 45.0f;
	constexpr float DrillSpinDegPerSec = 900.0f;
	/** Gas skimming: how deep in the pocket the drone works (fraction of its radius), the intake
	 *  fan's spin, and how fast the drone drifts around the pocket while it fills (deg/s). */
	constexpr float SkimDepth = 0.6f;
	constexpr float IntakeSpinDegPerSec = 540.0f;
	constexpr float SkimDriftDegPerSec = 8.0f;
	constexpr float DepletedMinInterval = 0.5f;
}

AShipDrone::AShipDrone()
	: BodySize(250.0f)
	, DrillDust(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Asteroid_Dust.NS_Asteroid_Dust")))
	, State(EShipDroneState::Launching)
	, Velocity(FVector::ZeroVector)
	, WorkDirLocal(FVector::ForwardVector)
	, WorkSurfaceFraction(0.85f)
	, OreAboard(0.0f)
	, StateTime(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	SetCanBeDamaged(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
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

	// Built from engine shapes, +X forward (drill end). Sizes are for BodySize = 250 cm.
	// Engine cube/sphere are 100 uu across; cylinder/cone are 100 uu tall along Z.
	Body = MakePart(TEXT("Body"), CubeMesh.Object, HullMat.Object);
	Body->SetRelativeScale3D(FVector(2.5f, 2.0f, 1.1f));

	Clamp = MakePart(TEXT("Clamp"), CylinderMesh.Object, HullMat.Object);
	Clamp->SetRelativeLocation(FVector(135.0f, 0.0f, 0.0f));
	Clamp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	Clamp->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.25f));

	Drill = MakePart(TEXT("Drill"), ConeMesh.Object, HullMat.Object);
	Drill->SetRelativeLocation(FVector(190.0f, 0.0f, 0.0f));
	Drill->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	Drill->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.9f));

	Lamp = MakePart(TEXT("Lamp"), SphereMesh.Object, LampMat.Object);
	Lamp->SetRelativeLocation(FVector(-60.0f, 0.0f, 60.0f));
	Lamp->SetRelativeScale3D(FVector(0.35f));
	Lamp->SetCastShadow(false);
}

void AShipDrone::InitDrone(UDroneBayComponent* InBay)
{
	Bay = InBay;
	SetActorScale3D(FVector(BodySize / 250.0f));
	Velocity = InBay ? InBay->GetShipVelocity() : FVector::ZeroVector;
	SetState(EShipDroneState::Launching);
}

void AShipDrone::BeginPlay()
{
	Super::BeginPlay();
	if (UNiagaraSystem* Dust = DrillDust.LoadSynchronous())
	{
		DustFX = UNiagaraFunctionLibrary::SpawnSystemAttached(Dust, Drill, NAME_None, FVector::ZeroVector,
			FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, false);
	}
}

void AShipDrone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DustFX)
	{
		DustFX->DestroyComponent();
		DustFX = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AShipDrone::SetState(EShipDroneState NewState)
{
	if (State != NewState)
	{
		UE_LOG(LogAdastrea, Verbose, TEXT("%s: state %d -> %d (ore %.1f)"), *GetName(), (int32)State, (int32)NewState, OreAboard);
	}
	// Drill dust only where something is being drilled.
	SetCutting(NewState == EShipDroneState::Cutting && !bSkimming);
	State = NewState;
	StateTime = 0.0f;
}

void AShipDrone::SetCutting(bool bCutting)
{
	if (!DustFX)
	{
		return;
	}
	if (bCutting && !DustFX->IsActive())
	{
		DustFX->Activate(true);
	}
	else if (!bCutting && DustFX->IsActive())
	{
		DustFX->Deactivate();
	}
}

bool AShipDrone::ChooseWorkSpot()
{
	UDroneBayComponent* B = Bay.Get();
	AAsteroid* Rock = B ? B->GetWorkTarget() : nullptr;
	if (!Rock)
	{
		return false;
	}
	// Don't mix ores in one hopper.
	const UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	if (OreAboard > 0.0f && HopperOre.IsValid() && (!Type || Type->OreItem != HopperOre.Get()))
	{
		return false;
	}

	// A spot on the side of the rock facing the ship, spread so drones don't stack.
	const FVector RockLoc = Rock->GetActorLocation();
	const FVector ToShip = (B->GetHatchLocation() - RockLoc).GetSafeNormal();
	const FVector Dir = FMath::VRandCone(ToShip, FMath::DegreesToRadians(ShipDroneTuning::WorkSpotCone));

	// Find the real surface along that direction (rocks aren't spheres).
	WorkSurfaceFraction = 0.85f;
	const float Radius = FMath::Max(Rock->GetRadius(), 1.0f);
	if (Rock->Mesh)
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ShipDroneWorkSpot), true);
		if (Rock->Mesh->LineTraceComponent(Hit, RockLoc + Dir * Radius * 1.5f, RockLoc, Params))
		{
			WorkSurfaceFraction = FMath::Clamp(FVector::Dist(Hit.ImpactPoint, RockLoc) / Radius, 0.2f, 1.2f);
		}
	}

	WorkRock = Rock;
	WorkDirLocal = Rock->GetActorQuat().UnrotateVector(Dir);
	bSkimming = Type && Type->GetResourceKind() == EResourceKind::Gas;
	if (bSkimming)
	{
		WorkSurfaceFraction = ShipDroneTuning::SkimDepth; // inside the cloud, not on a surface
	}
	return true;
}

FVector AShipDrone::GetWorkPoint() const
{
	const AAsteroid* Rock = WorkRock.Get();
	if (!Rock)
	{
		return GetActorLocation();
	}
	// The rock shrinks as it empties; the spot follows its surface in.
	const FVector Dir = Rock->GetActorQuat().RotateVector(WorkDirLocal);
	const float DrillReach = bSkimming ? 0.0f : 200.0f * GetActorScale3D().X;
	return Rock->GetActorLocation() + Dir * (Rock->GetRadius() * WorkSurfaceFraction + DrillReach);
}

float AShipDrone::GetMaxSpeed() const
{
	const UDroneBayComponent* B = Bay.Get();
	if (!B)
	{
		return 6000.0f;
	}
	return FMath::Max(B->DroneSpeed, B->GetShipVelocity().Size() * ShipDroneTuning::ChaseSpeedFactor);
}

float AShipDrone::FlyToward(const FVector& Goal, const FVector& GoalVelocity, float DeltaSeconds, bool bArrive)
{
	const UDroneBayComponent* B = Bay.Get();
	const float MaxSpeed = GetMaxSpeed();
	const float Accel = FMath::Max(B ? B->DroneAcceleration : 12000.0f, MaxSpeed * 2.0f);

	// Substeps keep the steering stable (and waypoints reachable) at low frame rates.
	const int32 Steps = FMath::Clamp(FMath::CeilToInt(DeltaSeconds / ShipDroneTuning::MaxStep), 1, 16);
	const float Dt = DeltaSeconds / Steps;
	FVector Loc = GetActorLocation();
	for (int32 Step = 0; Step < Steps; ++Step)
	{
		const FVector StepGoal = Goal + GoalVelocity * Dt * Step;
		const FVector ToGoal = StepGoal - Loc;
		const float Dist = ToGoal.Size();
		const float Speed = bArrive ? FMath::Min(MaxSpeed, Dist * ShipDroneTuning::ArriveGain) : MaxSpeed;
		const FVector Desired = GoalVelocity + (Dist > KINDA_SMALL_NUMBER ? ToGoal / Dist * Speed : FVector::ZeroVector);
		Velocity += (Desired - Velocity).GetClampedToMaxSize(Accel * Dt);
		Loc += Velocity * Dt;
	}
	SetActorLocation(Loc);

	// Nose along the direction of travel relative to the goal (so landing drones face the hatch, not the ship's heading).
	const FVector Rel = Velocity - GoalVelocity;
	if (Rel.SizeSquared() > 100.0f * 100.0f)
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Rel.Rotation(), DeltaSeconds, 5.0f));
	}
	return FVector::Dist(GetActorLocation(), Goal);
}

void AShipDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UDroneBayComponent* B = Bay.Get();
	if (!B)
	{
		Destroy();
		return;
	}
	StateTime += DeltaSeconds;

	switch (State)
	{
	case EShipDroneState::Launching:
		if (FlyToward(B->GetHatchApproach(), B->GetShipVelocity(), DeltaSeconds, true) < ShipDroneTuning::WaypointReach)
		{
			SetState(ChooseWorkSpot() ? EShipDroneState::Outbound : EShipDroneState::Returning);
		}
		break;

	case EShipDroneState::Outbound:
		if (!WorkRock.IsValid() || B->GetWorkTarget() != WorkRock.Get())
		{
			// Job changed or rock gone: re-plan, or head home.
			if (OreAboard > 0.0f || !ChooseWorkSpot())
			{
				SetState(EShipDroneState::Returning);
			}
			break;
		}
		if (FlyToward(GetWorkPoint(), FVector::ZeroVector, DeltaSeconds, true) < ShipDroneTuning::DockReach)
		{
			SetState(EShipDroneState::Cutting);
		}
		break;

	case EShipDroneState::Cutting:
		TickCutting(DeltaSeconds);
		break;

	case EShipDroneState::Returning:
		if (FlyToward(B->GetHatchApproach(), B->GetShipVelocity(), DeltaSeconds, true) < ShipDroneTuning::WaypointReach)
		{
			SetState(EShipDroneState::Landing);
		}
		else if (StateTime > ShipDroneTuning::ReturnTimeout)
		{
			ForceStow();
		}
		break;

	case EShipDroneState::Landing:
		if (FlyToward(B->GetHatchLocation(), B->GetShipVelocity(), DeltaSeconds, true) < ShipDroneTuning::DockReach
			|| StateTime > ShipDroneTuning::ReturnTimeout)
		{
			B->UnloadOre(HopperOre.Get(), OreAboard);
			OreAboard = 0.0f;
			HopperOre = nullptr;
			// Straight back out while the job is on; otherwise stay aboard.
			if (B->GetWorkTarget())
			{
				SetState(EShipDroneState::Launching);
			}
			else
			{
				Stow();
			}
		}
		break;
	}
}

void AShipDrone::TickCutting(float DeltaSeconds)
{
	UDroneBayComponent* B = Bay.Get();
	AAsteroid* Rock = WorkRock.Get();
	if (!IsValid(Rock) || B->GetWorkTarget() != Rock)
	{
		if (OreAboard > 0.0f || !ChooseWorkSpot())
		{
			SetState(EShipDroneState::Returning);
		}
		else
		{
			SetState(EShipDroneState::Outbound);
		}
		return;
	}

	Velocity = FVector::ZeroVector;
	if (bSkimming)
	{
		// In the pocket: drift slowly round it, intake ring spinning, nose into the drift.
		WorkDirLocal = FRotator(0.0f, ShipDroneTuning::SkimDriftDegPerSec * DeltaSeconds, 0.0f).RotateVector(WorkDirLocal);
		const FVector Spot = GetWorkPoint();
		const FVector Drift = Spot - GetActorLocation();
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), Spot, DeltaSeconds, 2.0f));
		if (Drift.SizeSquared() > 1.0f)
		{
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Drift.Rotation(), DeltaSeconds, 2.0f));
		}
		Clamp->AddLocalRotation(FRotator(0.0f, ShipDroneTuning::IntakeSpinDegPerSec * DeltaSeconds, 0.0f));
	}
	else
	{
		// Clamped on: hold the spot, drill pointing into the rock.
		const FVector Spot = GetWorkPoint();
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), Spot, DeltaSeconds, 8.0f));
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), (Rock->GetActorLocation() - Spot).Rotation(), DeltaSeconds, 6.0f));
		Drill->AddLocalRotation(FRotator(0.0f, ShipDroneTuning::DrillSpinDegPerSec * DeltaSeconds, 0.0f));
	}
	const FVector Spot = GetActorLocation();

	const UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	UTradeItemDataAsset* Ore = Type ? Type->OreItem.Get() : nullptr;
	const float Rate = Type ? Type->OreYieldPerSecond * B->MiningPower / FMath::Max(Type->Hardness, 0.1f) : 0.0f;
	if (!Ore || Rate <= 0.0f)
	{
		SetState(EShipDroneState::Returning);
		return;
	}

	// Only cut what still fits in the hopper.
	const float Room = B->HopperCapacity - OreAboard;
	const float CutTime = FMath::Min(DeltaSeconds, Room / Rate);
	const float Removed = Rock->ExtractOre(B->MiningPower, CutTime); // may destroy Rock
	OreAboard += Removed;
	HopperOre = Ore;

	if (!IsValid(Rock) && UAudioEventLibrary::IsLocalPlayerActor(B->GetOwner()))
	{
		// This drone took the last of it: the rock cracks apart at the drill.
		UAudioEventLibrary::PlayEventAtLocation(this, TEXT("Mining.AsteroidDepleted"), Spot, ShipDroneTuning::DepletedMinInterval);
	}
	if (OreAboard >= B->HopperCapacity - KINDA_SMALL_NUMBER || !IsValid(Rock))
	{
		SetState(EShipDroneState::Returning);
	}
}

void AShipDrone::ForceStow()
{
	if (UDroneBayComponent* B = Bay.Get())
	{
		B->UnloadOre(HopperOre.Get(), OreAboard);
	}
	OreAboard = 0.0f;
	HopperOre = nullptr;
	Stow();
}

void AShipDrone::Stow()
{
	SetCutting(false);
	if (UDroneBayComponent* B = Bay.Get())
	{
		B->NotifyDroneStowed(this);
	}
	Destroy();
}
