// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/PatrolController.h"
#include "AI/HostileFighterController.h"
#include "Combat/CombatTeams.h"
#include "Combat/DistressSubsystem.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "AdastreaLog.h"

namespace Patrol
{
	/** Start easing off this far from a distress call (cm). */
	constexpr float SlowDownDistance = 5000.0f;
}

APatrolController::APatrolController()
{
	// Patrol pilots are trained and better armed than raiders: a patrol should win, or the
	// raiders' shields recharge between the few hits AI pilots land on a dodging fighter.
	DamageScale = 2.0f;
	GunSpreadDegrees = 0.5f;
}

int32 APatrolController::GetCombatTeam() const
{
	return CombatTeam::Civil;
}

bool APatrolController::IsPatrolShip(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Cast<APatrolController>(Pawn->GetController()) != nullptr;
}

ASpaceship* APatrolController::SpawnPatrol(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station)
{
	return SpawnPilotedShip(World, ShipClass, Station, APatrolController::StaticClass());
}

void APatrolController::AnswerCall(int32 InCallId)
{
	CallId = InCallId;
	RefreshCall();
	UE_LOG(LogAdastreaCombat, Log, TEXT("%s answering distress call %d"), GetShip() ? *GetShip()->GetName() : *GetName(), CallId);
}

FString APatrolController::GetObjectiveDescription() const
{
	if (CombatState != ECombatPilotState::Idle && GetCombatTarget())
	{
		return Super::GetObjectiveDescription();
	}
	if (CallId != INDEX_NONE)
	{
		return TEXT("Answering a distress call");
	}
	return TargetStation ? FString::Printf(TEXT("Patrolling to %s"), *GetStationDisplayName(TargetStation)) : TEXT("Patrolling");
}

void APatrolController::RefreshCall()
{
	if (CallId == INDEX_NONE)
	{
		return;
	}
	const UDistressSubsystem* Distress = UDistressSubsystem::Get(this);
	if (!Distress || !Distress->GetCallLocation(CallId, CallLocation))
	{
		UE_LOG(LogAdastreaCombat, Log, TEXT("%s: distress call %d closed, back on patrol"), GetShip() ? *GetShip()->GetName() : *GetName(), CallId);
		CallId = INDEX_NONE;
	}
}

ASpaceship* APatrolController::FindTarget() const
{
	const ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return nullptr;
	}
	TArray<ASpaceship*> Candidates;
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		if (!AHostileFighterController::IsHostileShip(*It) || !IsAttackable(*It))
		{
			continue;
		}
		const FVector Where = It->GetActorLocation();
		const bool bSeen = FVector::Dist(Where, Ship->GetActorLocation()) <= SensorRange;
		const bool bAtCall = CallId != INDEX_NONE && FVector::Dist(Where, CallLocation) <= UDistressSubsystem::CallRadius;
		if (bSeen || bAtCall)
		{
			Candidates.Add(*It);
		}
	}
	return PickNearest(Candidates);
}

void APatrolController::TickPilot(float DeltaSeconds)
{
	RefreshCall();

	// Docked between patrol legs: launch if there's a call or a hostile in sight.
	if (State == EAIPilotState::Docked)
	{
		if (CallId == INDEX_NONE && !FindTarget())
		{
			AAIPilotController::TickPilot(DeltaSeconds);
			return;
		}
		UndockShip();
		State = EAIPilotState::Choosing;
		TargetStation = nullptr;
	}
	Super::TickPilot(DeltaSeconds);
}

void APatrolController::TickNoTarget(float DeltaSeconds)
{
	if (CallId != INDEX_NONE)
	{
		// Full speed to the call (SteerToward eases off far out, for docking), slowing only
		// while turning or for the last stretch.
		ASpaceship* Ship = GetShip();
		const float Alignment = FaceToward(CallLocation, DeltaSeconds);
		const float Distance = FVector::Dist(CallLocation, Ship->GetActorLocation());
		const float TurnFactor = Alignment > 0.7f ? 1.0f : 0.3f;
		const float ApproachFactor = FMath::Clamp(Distance / Patrol::SlowDownDistance, 0.3f, 1.0f);
		Ship->SetThrottle(CruiseThrottle * TurnFactor * ApproachFactor);
		return;
	}
	// Nothing to do: carry on flying the station-to-station patrol route.
	AAIPilotController::TickPilot(DeltaSeconds);
}
