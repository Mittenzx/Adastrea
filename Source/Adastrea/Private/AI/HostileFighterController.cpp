// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HostileFighterController.h"
#include "AI/PatrolController.h"
#include "Combat/CombatTeams.h"
#include "Ships/Spaceship.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

bool AHostileFighterController::IsHostileShip(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Cast<AHostileFighterController>(Pawn->GetController()) != nullptr;
}

ASpaceship* AHostileFighterController::SpawnHostile(UWorld* World, TSubclassOf<ASpaceship> ShipClass, const FVector& Location, const FRotator& Facing)
{
	if (!World || !ShipClass)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASpaceship* Ship = World->SpawnActor<ASpaceship>(ShipClass, Location, Facing, Params);
	if (!Ship)
	{
		return nullptr;
	}
	AHostileFighterController* Pilot = World->SpawnActor<AHostileFighterController>(AHostileFighterController::StaticClass(), FTransform::Identity, Params);
	if (!Pilot)
	{
		Ship->Destroy();
		return nullptr;
	}
	Pilot->Possess(Ship);
	return Ship;
}

int32 AHostileFighterController::GetCombatTeam() const
{
	return CombatTeam::Hostile;
}

ASpaceship* AHostileFighterController::FindTarget() const
{
	TArray<ASpaceship*> Candidates;
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (ASpaceship* Player = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr; IsAttackable(Player))
	{
		Candidates.Add(Player);
	}
	// Patrols shoot at us, so we shoot back.
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		if (APatrolController::IsPatrolShip(*It) && IsAttackable(*It))
		{
			Candidates.Add(*It);
		}
	}
	return PickNearest(Candidates);
}
