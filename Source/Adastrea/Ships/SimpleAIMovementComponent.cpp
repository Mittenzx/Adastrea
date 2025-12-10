// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/SimpleAIMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "AdastreaLog.h"

USimpleAIMovementComponent::USimpleAIMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	MinDistance = 10000.0f;
	MaxDistance = 50000.0f;
	MoveSpeed = 2000.0f;
	ArrivalThreshold = 500.0f;
	TurnRate = 90.0f;
	bConstrainToHorizontalPlane = true;
	bShowDebug = false;
	TargetLocation = FVector::ZeroVector;
	CachedMovementComponent = nullptr;
}

void USimpleAIMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache FloatingPawnMovement component for performance
	AActor* Owner = GetOwner();
	if (Owner)
	{
		APawn* PawnOwner = Cast<APawn>(Owner);
		if (PawnOwner)
		{
			CachedMovementComponent = PawnOwner->FindComponentByClass<UFloatingPawnMovement>();
		}
	}

	// Generate initial target on start
	GenerateNewTarget();
}

void USimpleAIMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Only move if we're possessed by an AI controller
	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner || !PawnOwner->GetController() || PawnOwner->GetController()->IsPlayerController())
	{
		return;
	}

	// Check if we've arrived at target
	if (HasArrivedAtTarget())
	{
		// Generate new target
		GenerateNewTarget();
	}
	else
	{
		// Move toward target
		MoveTowardTarget(DeltaTime);
		RotateTowardTarget(DeltaTime);
	}

	// Debug visualization
	if (bShowDebug)
	{
		FVector CurrentLocation = Owner->GetActorLocation();
		
		// Draw line to target
		DrawDebugLine(
			GetWorld(),
			CurrentLocation,
			TargetLocation,
			FColor::Green,
			false,
			-1.0f,
			0,
			10.0f
		);

		// Draw target sphere
		DrawDebugSphere(
			GetWorld(),
			TargetLocation,
			ArrivalThreshold,
			12,
			FColor::Yellow,
			false,
			-1.0f,
			0,
			5.0f
		);

		// Draw distance text
		float Distance = GetDistanceToTarget();
		FString DebugText = FString::Printf(TEXT("Distance: %.0f"), Distance);
		DrawDebugString(
			GetWorld(),
			CurrentLocation + FVector(0, 0, 200),
			DebugText,
			nullptr,
			FColor::White,
			0.0f,
			true
		);
	}
}

void USimpleAIMovementComponent::GenerateNewTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector CurrentLocation = Owner->GetActorLocation();

	// Validate distance parameters
	float ValidMinDistance = FMath::Max(MinDistance, 1000.0f);
	float ValidMaxDistance = FMath::Max(MaxDistance, ValidMinDistance + 1000.0f);

	// Generate random direction
	FVector RandomDirection;
	RandomDirection.X = FMath::RandRange(-1.0f, 1.0f);
	RandomDirection.Y = FMath::RandRange(-1.0f, 1.0f);
	RandomDirection.Z = FMath::RandRange(-1.0f, 1.0f);
	RandomDirection.Normalize();

	// Generate random distance
	float RandomDistance = FMath::RandRange(ValidMinDistance, ValidMaxDistance);

	// Calculate new target
	TargetLocation = CurrentLocation + (RandomDirection * RandomDistance);

	UE_LOG(LogAdastreaAI, Log, TEXT("%s: New target generated at %s (distance: %.0f)"), 
		*Owner->GetName(), *TargetLocation.ToString(), RandomDistance);
}

float USimpleAIMovementComponent::GetDistanceToTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}

	return FVector::Distance(Owner->GetActorLocation(), TargetLocation);
}

bool USimpleAIMovementComponent::HasArrivedAtTarget() const
{
	return GetDistanceToTarget() < ArrivalThreshold;
}

void USimpleAIMovementComponent::MoveTowardTarget(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner)
	{
		return;
	}

	// Get direction to target
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

	// AddMovementInput expects a normalized direction and scale factor (0-1)
	// We use full speed movement (1.0f)
	PawnOwner->AddMovementInput(Direction, 1.0f);
	
	// Apply MoveSpeed to cached FloatingPawnMovement component if present
	if (CachedMovementComponent)
	{
		CachedMovementComponent->MaxSpeed = MoveSpeed;
	}
}

void USimpleAIMovementComponent::RotateTowardTarget(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get direction to target
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Direction = TargetLocation - CurrentLocation;
	
	// Optionally constrain to horizontal plane (ignore Z-axis)
	if (bConstrainToHorizontalPlane)
	{
		Direction.Z = 0;
	}

	if (Direction.SizeSquared() > SMALL_NUMBER)
	{
		// Calculate target rotation
		FRotator TargetRotation = Direction.Rotation();
		FRotator CurrentRotation = Owner->GetActorRotation();

		// Interpolate toward target rotation using TurnRate as interp speed
		// RInterpTo uses the TurnRate as degrees per second naturally
		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			TurnRate
		);

		Owner->SetActorRotation(NewRotation);
	}
}
