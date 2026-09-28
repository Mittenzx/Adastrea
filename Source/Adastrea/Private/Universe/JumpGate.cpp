// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/JumpGate.h"
#include "Universe/GalaxySubsystem.h"
#include "AdastreaHUD.h"
#include "AdastreaLog.h"
#include "SpaceSectorMap.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor GateOnline(0.25f, 0.85f, 1.0f, 1.0f);
	const FLinearColor GateOffline(1.0f, 0.62f, 0.2f, 1.0f);
	constexpr int32 RingSegments = 36;

	/** Sector position on the system map (the same layout the map draws). */
	FVector2D OrbitPos(const FGalaxySectorDef& S)
	{
		const float A = FMath::DegreesToRadians(S.OrbitAngle);
		return FVector2D(FMath::Cos(A), FMath::Sin(A)) * S.OrbitRadius;
	}
}

// ---------------------------------------------------------------------------
// AJumpGate
// ---------------------------------------------------------------------------

AJumpGate::AJumpGate()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Ring = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Ring"));
	Ring->SetupAttachment(Root);
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision); // ships fly through
	Ring->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Ring->SetStaticMesh(CubeMesh.Object);
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetIntensity(80000.0f);
	Light->SetAttenuationRadius(30000.0f);
	Light->SetCastShadows(false);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(700.0f);
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AJumpGate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildRing();
	RefreshAppearance();
}

void AJumpGate::BeginPlay()
{
	Super::BeginPlay();
	BuildRing();
	RefreshAppearance();
}

void AJumpGate::SetTargetSector(FName InTargetSectorId)
{
	TargetSectorId = InTargetSectorId;
	RefreshAppearance();
}

void AJumpGate::BuildRing()
{
	Ring->ClearInstances();
	const float SegLen = 2.0f * PI * RingRadius / RingSegments * 1.08f; // slight overlap, no gaps
	for (int32 i = 0; i < RingSegments; ++i)
	{
		const float A = 2.0f * PI * i / RingSegments;
		const FVector Radial(0.0f, FMath::Cos(A), FMath::Sin(A));
		const FVector Tangent(0.0f, -FMath::Sin(A), FMath::Cos(A));
		// Cube is 100 cm: X = depth along the gate axis, Y = along the ring, Z = radial thickness.
		const FRotator Rot = FRotationMatrix::MakeFromXY(FVector::ForwardVector, Tangent).Rotator();
		const float Thick = (i % 9 == 0) ? 6.0f : 3.5f; // four heavier struts read as a gate, not a hoop
		Ring->AddInstance(FTransform(Rot, Radial * (RingRadius + Thick * 50.0f), FVector(4.0f, SegLen / 100.0f, Thick)));
	}
	Label->SetRelativeLocation(FVector(0.0f, 0.0f, RingRadius + 900.0f));
}

void AJumpGate::RefreshAppearance()
{
	const bool bOnline = IsOnline();
	const FLinearColor Col = bOnline ? GateOnline : GateOffline;

	if (!RingMaterial && Ring->GetStaticMesh())
	{
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			RingMaterial = UMaterialInstanceDynamic::Create(Base, this);
			Ring->SetMaterial(0, RingMaterial);
		}
	}
	if (RingMaterial)
	{
		RingMaterial->SetVectorParameterValue(TEXT("Color"), Col);
	}
	Light->SetLightColor(Col);

	const FString Dest = GetDestinationName().ToUpper();
	Label->SetText(FText::FromString(bOnline
		? FString::Printf(TEXT("JUMP GATE\n%s"), *Dest)
		: FString::Printf(TEXT("JUMP GATE - OFFLINE\n%s"), *Dest)));
	Label->SetTextRenderColor(Col.ToFColor(true));
}

bool AJumpGate::IsOnline() const
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	const FGalaxySectorDef* Target = Galaxy ? Galaxy->FindSector(TargetSectorId) : nullptr;
	return Target && Target->HasLevel();
}

FString AJumpGate::GetDestinationName() const
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (const FGalaxySectorDef* Target = Galaxy ? Galaxy->FindSector(TargetSectorId) : nullptr)
	{
		const FStarSystemDef* Sys = Galaxy->FindSystem(Target->SystemId);
		const FGalaxySectorDef* Here = Galaxy->FindSector(Galaxy->ResolveCurrentSectorId(this));
		// Name the system too when the gate crosses a jump lane.
		if (Sys && (!Here || Here->SystemId != Target->SystemId))
		{
			return FString::Printf(TEXT("%s (%s)"), *Target->Name.ToString(), *Sys->Name.ToString());
		}
		return Target->Name.ToString();
	}
	return TargetSectorId.ToString();
}

FString AJumpGate::GetDisplayName() const
{
	return FString::Printf(TEXT("Gate: %s"), *GetDestinationName());
}

FVector AJumpGate::GetArrivalLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * ExitDistance;
}

FRotator AJumpGate::GetArrivalRotation() const
{
	return GetActorForwardVector().Rotation();
}

void AJumpGate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	// Keep the label turned towards the camera.
	if (PC->PlayerCameraManager)
	{
		const FVector ToCam = PC->PlayerCameraManager->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0f, ToCam.Rotation().Yaw, 0.0f));
	}

	ASpaceship* Ship = Cast<ASpaceship>(PC->GetPawn());
	if (!Ship || Ship->IsDocked())
	{
		return;
	}

	// Inside the ring: close to the gate plane and within the ring radius.
	const FVector Rel = Ship->GetActorLocation() - GetActorLocation();
	const FVector Fwd = GetActorForwardVector();
	const float Axial = FVector::DotProduct(Rel, Fwd);
	const float Radial = (Rel - Fwd * Axial).Size();
	const bool bInRing = FMath::Abs(Axial) < 2000.0f && Radial < RingRadius;

	if (!bArmed)
	{
		bArmed = Rel.Size() > RingRadius * 2.0f;
		return;
	}
	if (!bInRing)
	{
		return;
	}

	bArmed = false;
	AAdastreaHUD* HUD = Cast<AAdastreaHUD>(PC->GetHUD());
	if (!IsOnline())
	{
		if (HUD)
		{
			HUD->ShowMessage(FString::Printf(TEXT("Jump gate offline: %s has no level yet"), *GetDestinationName()), 3.0f, true);
		}
		return;
	}

	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	FString Reason;
	if (!Galaxy || !Galaxy->BeginJump(PC, TargetSectorId, Reason))
	{
		if (HUD)
		{
			HUD->ShowMessage(FString::Printf(TEXT("Jump failed: %s"), *Reason), 3.0f, true);
		}
		return;
	}
	if (HUD)
	{
		HUD->ShowMessage(FString::Printf(TEXT("Jumping to %s..."), *GetDestinationName()), 2.0f, false);
	}
}

// ---------------------------------------------------------------------------
// UJumpGateWorldSubsystem
// ---------------------------------------------------------------------------

bool UJumpGateWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UJumpGateWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(&InWorld);
	if (!Galaxy)
	{
		return;
	}
	const FName SectorId = Galaxy->ResolveCurrentSectorId(&InWorld);
	if (!SectorId.IsNone())
	{
		SpawnGates(SectorId);
	}
	else
	{
		UE_LOG(LogAdastrea, Log, TEXT("JumpGates: level '%s' isn't a galaxy sector; no gates spawned"), *InWorld.GetMapName());
	}

	if (Galaxy->IsJumpInProgress())
	{
		// The player's ship may not be spawned/possessed yet; wait for it.
		ArrivalWaited = 0.0f;
		InWorld.GetTimerManager().SetTimer(ArrivalTimer, this, &UJumpGateWorldSubsystem::TryCompleteArrival, 0.1f, true);
	}
}

void UJumpGateWorldSubsystem::TryCompleteArrival()
{
	UWorld* World = GetWorld();
	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World);
	if (!World || !Galaxy)
	{
		return;
	}
	ArrivalWaited += 0.1f;
	APlayerController* PC = World->GetFirstPlayerController();
	const bool bReady = PC && Cast<ASpaceship>(PC->GetPawn());
	// Give the level's own spawn a moment; after that CompletePendingJump brings the ship in itself.
	if (!bReady && ArrivalWaited < 3.0f)
	{
		return;
	}
	World->GetTimerManager().ClearTimer(ArrivalTimer);

	AJumpGate* ArrivalGate = FindGateTo(Galaxy->GetPendingJumpFrom());
	if (ArrivalGate)
	{
		ArrivalGate->Disarm();
	}
	Galaxy->CompletePendingJump(World, ArrivalGate);
}

AJumpGate* UJumpGateWorldSubsystem::FindGateTo(FName SectorId) const
{
	if (SectorId.IsNone() || !GetWorld())
	{
		return nullptr;
	}
	for (TActorIterator<AJumpGate> It(GetWorld()); It; ++It)
	{
		if (It->TargetSectorId == SectorId)
		{
			return *It;
		}
	}
	return nullptr;
}

FVector UJumpGateWorldSubsystem::GetSectorCenter(FName SectorId) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}
	for (TActorIterator<ASpaceSectorMap> It(World); It; ++It)
	{
		if (It->SectorId == SectorId)
		{
			return It->GetActorLocation();
		}
	}
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;
	for (TActorIterator<ASpaceStation> It(World); It; ++It)
	{
		Sum += It->GetActorLocation();
		++Count;
	}
	if (Count > 0)
	{
		return Sum / Count;
	}
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		return It->GetActorLocation();
	}
	return FVector::ZeroVector;
}

void UJumpGateWorldSubsystem::SpawnGates(FName SectorId)
{
	UWorld* World = GetWorld();
	UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World);
	const FGalaxySectorDef* Here = Galaxy ? Galaxy->FindSector(SectorId) : nullptr;
	if (!World || !Here)
	{
		return;
	}
	const FStarSystemDef* HereSys = Galaxy->FindSystem(Here->SystemId);
	const FVector Center = GetSectorCenter(SectorId);

	int32 Spawned = 0;
	for (const FName DestId : Galaxy->GetGateDestinations(SectorId))
	{
		if (FindGateTo(DestId))
		{
			continue; // placed by hand in the level
		}
		const FGalaxySectorDef* Dest = Galaxy->FindSector(DestId);
		if (!Dest)
		{
			continue;
		}

		// Lay the gate out in the direction the destination lies on the maps:
		// system-map direction inside a system, universe-map direction across a lane.
		FVector2D Dir2D;
		if (Dest->SystemId == Here->SystemId)
		{
			Dir2D = OrbitPos(*Dest) - OrbitPos(*Here);
		}
		else
		{
			const FStarSystemDef* DestSys = Galaxy->FindSystem(Dest->SystemId);
			Dir2D = (DestSys && HereSys) ? DestSys->Position - HereSys->Position : FVector2D(1.0f, 0.0f);
		}
		Dir2D = Dir2D.GetSafeNormal();
		if (Dir2D.IsNearlyZero())
		{
			Dir2D = FVector2D(1.0f, 0.0f);
		}
		const FVector Dir(Dir2D.X, Dir2D.Y, 0.0f);
		const FVector Location = Center + Dir * AutoGateDistance;
		const FRotator Facing = (-Dir).Rotation(); // forward points back at the sector: arrivals fly in

		AJumpGate* Gate = World->SpawnActorDeferred<AJumpGate>(AJumpGate::StaticClass(), FTransform(Facing, Location), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Gate)
		{
			Gate->TargetSectorId = DestId;
			Gate->bAutoSpawned = true;
			Gate->FinishSpawning(FTransform(Facing, Location));
#if WITH_EDITOR
			Gate->SetActorLabel(Gate->GetDisplayName());
#endif
			++Spawned;
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("JumpGates: sector '%s' - spawned %d gate(s) around %s"),
		*SectorId.ToString(), Spawned, *Center.ToString());
}
