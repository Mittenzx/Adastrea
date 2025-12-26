// Copyright Mittenzx. All Rights Reserved.

#include "Performance/LODManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ULODManagerComponent::ULODManagerComponent()
	: HighToMediumDistance(5000.0f)
	, MediumToLowDistance(15000.0f)
	, LowToVeryLowDistance(30000.0f)
	, UpdateFrequency(0.5f)
	, bUsePerformanceLOD(false)
	, TargetFrameRate(60.0f)
	, CurrentLODLevel(ELODLevel::High)
	, DistanceToCamera(0.0f)
	, TimeSinceLastUpdate(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULODManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Force initial LOD update
	ForceUpdateLOD();
}

void ULODManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastUpdate += DeltaTime;

	// Update LOD at specified frequency
	if (TimeSinceLastUpdate >= UpdateFrequency)
	{
		UpdateLOD();
		TimeSinceLastUpdate = 0.0f;
	}
}

void ULODManagerComponent::SetLODLevel(ELODLevel NewLevel)
{
	if (CurrentLODLevel != NewLevel)
	{
		ELODLevel OldLevel = CurrentLODLevel;
		CurrentLODLevel = NewLevel;
		OnLODChanged(NewLevel, OldLevel);
	}
}

void ULODManagerComponent::ForceUpdateLOD()
{
	TimeSinceLastUpdate = UpdateFrequency; // Trigger immediate update
	UpdateLOD();
}

ELODLevel ULODManagerComponent::GetRecommendedLODForDistance(float Distance) const
{
	if (Distance <= HighToMediumDistance)
	{
		return ELODLevel::High;
	}
	else if (Distance <= MediumToLowDistance)
	{
		return ELODLevel::Medium;
	}
	else if (Distance <= LowToVeryLowDistance)
	{
		return ELODLevel::Low;
	}
	else
	{
		return ELODLevel::VeryLow;
	}
}

bool ULODManagerComponent::ShouldCullActor() const
{
	// Cull actors that are significantly beyond VeryLow LOD distance
	float CullDistance = LowToVeryLowDistance * 1.5f;
	return DistanceToCamera > CullDistance;
}

void ULODManagerComponent::UpdateLOD()
{
	// Get distance to camera
	DistanceToCamera = GetDistanceToCamera();

	// Calculate LOD based on distance
	ELODLevel RecommendedLOD = GetRecommendedLODForDistance(DistanceToCamera);

	// Adjust for performance if enabled
	if (bUsePerformanceLOD)
	{
		AdjustLODForPerformance();
	}
	else
	{
		// Apply recommended LOD
		SetLODLevel(RecommendedLOD);
	}
}

float ULODManagerComponent::GetDistanceToCamera() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	// Get player controller and camera
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return 0.0f;
	}

	// Get camera location
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Calculate distance
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	return FVector::Distance(CameraLocation, OwnerLocation);
}

void ULODManagerComponent::AdjustLODForPerformance()
{
	// Get current frame rate
	float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();

	// Calculate performance ratio (1.0 = target, <1.0 = below target)
	float PerformanceRatio = CurrentFPS / TargetFrameRate;

	// Get distance-based recommendation
	ELODLevel DistanceLOD = GetRecommendedLODForDistance(DistanceToCamera);
	int32 DistanceLODIndex = static_cast<int32>(DistanceLOD);

	// Adjust LOD based on performance
	int32 FinalLODIndex = DistanceLODIndex;
	if (PerformanceRatio < 0.9f)
	{
		// Below target FPS - reduce quality
		FinalLODIndex = FMath::Min(DistanceLODIndex + 1, 3); // Max is VeryLow (index 3)
	}
	else if (PerformanceRatio > 1.2f && DistanceLODIndex > 0)
	{
		// Well above target FPS - increase quality if distance allows
		FinalLODIndex = FMath::Max(DistanceLODIndex - 1, 0);
	}

	ELODLevel FinalLOD = static_cast<ELODLevel>(FinalLODIndex);
	SetLODLevel(FinalLOD);
}
