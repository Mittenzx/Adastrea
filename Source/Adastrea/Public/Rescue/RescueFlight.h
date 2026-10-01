#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/** Kinematic steering shared by the rescue craft (tow drone, escape pod). No collision. */
namespace RescueFlight
{
	/** Speed at which an arriving craft slows: this many cm/s per cm still to go. */
	constexpr float ArriveGain = 1.5f;
	/** Longest steering substep (s), so low frame rates stay stable. */
	constexpr float MaxStep = 1.0f / 30.0f;

	/**
	 * Steer Actor toward Goal (moving at GoalVelocity), accelerating at most Accel toward a
	 * desired velocity capped at MaxSpeed; slows on the way in when bArrive. Updates
	 * Velocity and the actor's location, turns the nose along the relative velocity, and
	 * returns the distance still to go.
	 */
	inline float FlyToward(AActor* Actor, FVector& Velocity, const FVector& Goal, const FVector& GoalVelocity,
		float MaxSpeed, float Accel, float DeltaSeconds, bool bArrive)
	{
		const int32 Steps = FMath::Clamp(FMath::CeilToInt(DeltaSeconds / MaxStep), 1, 16);
		const float Dt = DeltaSeconds / Steps;
		FVector Loc = Actor->GetActorLocation();
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			const FVector ToGoal = Goal + GoalVelocity * Dt * Step - Loc;
			const float Dist = ToGoal.Size();
			const float Speed = bArrive ? FMath::Min(MaxSpeed, Dist * ArriveGain) : MaxSpeed;
			const FVector Desired = GoalVelocity + (Dist > KINDA_SMALL_NUMBER ? ToGoal / Dist * Speed : FVector::ZeroVector);
			Velocity += (Desired - Velocity).GetClampedToMaxSize(Accel * Dt);
			Loc += Velocity * Dt;
		}
		Actor->SetActorLocation(Loc);

		const FVector Rel = Velocity - GoalVelocity;
		if (Rel.SizeSquared() > 100.0f * 100.0f)
		{
			Actor->SetActorRotation(FMath::RInterpTo(Actor->GetActorRotation(), Rel.Rotation(), DeltaSeconds, 4.0f));
		}
		return FVector::Dist(Actor->GetActorLocation(), Goal);
	}
}
