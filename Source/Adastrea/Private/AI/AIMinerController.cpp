// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIMinerController.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Interfaces/ITargetable.h"
#include "Mining/Asteroid.h"
#include "Mining/MiningLaserComponent.h"
#include "Ships/Spaceship.h"
#include "Stations/DockingBayModule.h"
#include "Stations/SpaceStation.h"
#include "Trading/CargoComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"

void AAIMinerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	MinerState = EAIMinerState::SeekingAsteroid;

	// Every ship carries a laser component but only ships with a MiningRating switch it on.
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->MiningLaser)
	{
		Ship->MiningLaser->bMiningEnabled = true;
		Ship->MiningLaser->MiningPower = FMath::Max(Ship->MiningLaser->MiningPower, 1.0f);
	}
}

void AAIMinerController::TickPilot(float DeltaSeconds)
{
	switch (MinerState)
	{
	case EAIMinerState::SeekingAsteroid:
		TickSeeking();
		break;
	case EAIMinerState::ToAsteroid:
		TickToAsteroid(DeltaSeconds);
		break;
	case EAIMinerState::Mining:
		TickMining(DeltaSeconds);
		break;
	case EAIMinerState::ToStation:
		TickToStation(DeltaSeconds);
		break;
	case EAIMinerState::Docked:
		TickMinerDocked(DeltaSeconds);
		break;
	}
}

bool AAIMinerController::IsMinable(const AAsteroid* Rock) const
{
	return Rock && !Rock->IsActorBeingDestroyed() && ITargetable::Execute_CanBeTargeted(Rock);
}

float AAIMinerController::SurfaceDistance(const AAsteroid* Rock) const
{
	const ASpaceship* Ship = GetShip();
	const FVector Muzzle = Ship->MiningLaser ? Ship->MiningLaser->GetComponentLocation() : Ship->GetActorLocation();
	return FVector::Dist(Muzzle, Rock->GetActorLocation()) - Rock->GetRadius();
}

// ---------------------------------------------------------------------------
// Finding and reaching a rock
// ---------------------------------------------------------------------------

void AAIMinerController::TickSeeking()
{
	ASpaceship* Ship = GetShip();
	Ship->SetThrottle(0.0f);

	AAsteroid* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AAsteroid> It(GetWorld()); It; ++It)
	{
		if (IsMinable(*It))
		{
			const float Distance = FVector::Dist(Ship->GetActorLocation(), It->GetActorLocation());
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				Best = *It;
			}
		}
	}

	if (Best)
	{
		TargetAsteroid = Best;
		MinerState = EAIMinerState::ToAsteroid;
	}
}

void AAIMinerController::TickToAsteroid(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	if (!IsMinable(TargetAsteroid))
	{
		AbandonAsteroid();
		return;
	}

	if (SurfaceDistance(TargetAsteroid) > Ship->MiningLaser->Range * StandOffFraction)
	{
		SteerToward(TargetAsteroid->GetActorLocation(), DeltaSeconds);
		return;
	}

	// In laser range: stop, lock the rock and open fire.
	Ship->SetThrottle(0.0f);
	if (Ship->MiningLaser->SetTarget(TargetAsteroid))
	{
		Ship->MiningLaser->StartMining();
		MinerState = EAIMinerState::Mining;
		UE_LOG(LogAdastreaShips, Log, TEXT("AIMiner %s mining %s"), *Ship->GetName(), *TargetAsteroid->GetName());
	}
	else
	{
		AbandonAsteroid();
	}
}

void AAIMinerController::AbandonAsteroid()
{
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->MiningLaser)
	{
		Ship->MiningLaser->StopMining();
		Ship->MiningLaser->ClearTarget();
	}
	TargetAsteroid = nullptr;
	MinerState = EAIMinerState::SeekingAsteroid;
}

// ---------------------------------------------------------------------------
// Mining
// ---------------------------------------------------------------------------

void AAIMinerController::TickMining(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	UMiningLaserComponent* Laser = Ship->MiningLaser;

	if (!IsMinable(TargetAsteroid))
	{
		// Rock is spent; the laser already dropped its lock. Move on to the next one.
		AbandonAsteroid();
		return;
	}

	Ship->SetThrottle(0.0f);
	FaceToward(TargetAsteroid->GetActorLocation(), DeltaSeconds);

	switch (Laser->GetStatus())
	{
	case EMiningStatus::CargoFull:
		Laser->StopMining();
		Laser->ClearTarget();
		DockedSeconds = 0.0f;
		ChooseSellStation();
		MinerState = TargetStation ? EAIMinerState::ToStation : EAIMinerState::SeekingAsteroid;
		UE_LOG(LogAdastreaShips, Log, TEXT("AIMiner %s hold full, heading to %s"),
			*Ship->GetName(), TargetStation ? *TargetStation->GetName() : TEXT("(no station)"));
		break;
	case EMiningStatus::OutOfRange:
		// Drifted or the rock moved; close the gap again.
		Laser->StopMining();
		MinerState = EAIMinerState::ToAsteroid;
		break;
	case EMiningStatus::NoTarget:
		Laser->SetTarget(TargetAsteroid);
		break;
	default:
		break; // Mining, OffAim (still turning) or Idle: keep going.
	}
}

// ---------------------------------------------------------------------------
// Selling
// ---------------------------------------------------------------------------

void AAIMinerController::ChooseSellStation()
{
	const ASpaceship* Ship = GetShip();
	const UCargoComponent* Cargo = Ship->CargoComponent;
	const TArray<FCargoEntry> Contents = Cargo ? Cargo->GetCargoContents() : TArray<FCargoEntry>();

	ASpaceStation* Best = nullptr;
	float BestScore = -1.0f;
	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		if (!It->GetDockingBayModule())
		{
			continue;
		}

		// Stations without a usable market score below any that have one.
		float Score = 0.0f;
		if (UMarketDataAsset* Market = GetMarket(*It))
		{
			Score = 1.0f;
			for (const FCargoEntry& Entry : Contents)
			{
				if (Entry.Item && Entry.Quantity > 0)
				{
					Score += Market->GetItemPrice(Entry.Item, false) * Entry.Quantity;
				}
			}
		}
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = *It;
		}
	}
	TargetStation = Best;
}

void AAIMinerController::TickToStation(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	ADockingBayModule* Bay = TargetStation ? TargetStation->GetDockingBayModule() : nullptr;
	if (!Bay)
	{
		ChooseSellStation();
		Ship->SetThrottle(0.0f);
		return;
	}

	switch (ApproachAndDock(Bay, DeltaSeconds))
	{
	case EApproachResult::Docked:
		SellAllCargo();
		DockedSeconds = 0.0f;
		MinerState = EAIMinerState::Docked;
		break;
	case EApproachResult::Blocked:
		// Bay is full; ask again (may pick the same station once a slot frees up).
		DockedSeconds = 0.0f;
		ChooseSellStation();
		break;
	case EApproachResult::Flying:
		break;
	}
}

void AAIMinerController::SellAllCargo()
{
	ASpaceship* Ship = GetShip();
	UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent;
	UCargoComponent* Cargo = Ship->CargoComponent;
	UMarketDataAsset* Market = GetMarket(TargetStation);
	if (!Trader || !Cargo || !Market)
	{
		UE_LOG(LogAdastreaShips, Warning, TEXT("AIMiner %s: no market at %s, keeping the ore"),
			*Ship->GetName(), TargetStation ? *TargetStation->GetName() : TEXT("(none)"));
		return;
	}

	const int32 CreditsBefore = Trader->GetCredits();
	for (const FCargoEntry& Entry : Cargo->GetCargoContents())
	{
		if (Entry.Item && Entry.Quantity > 0)
		{
			Trader->SellItem(Market, Entry.Item, Entry.Quantity, Cargo);
		}
	}
	UE_LOG(LogAdastreaShips, Log, TEXT("AIMiner %s sold its load at %s for %d credits"),
		*Ship->GetName(), *TargetStation->GetName(), Trader->GetCredits() - CreditsBefore);
}

void AAIMinerController::TickMinerDocked(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();

	// Hold station so the throttle decay can't drift the ship off the dock.
	if (DockPoint)
	{
		Ship->SetActorLocationAndRotation(DockPoint->GetComponentLocation(), DockPoint->GetComponentRotation());
	}

	DockedSeconds += DeltaSeconds;
	if (DockedSeconds < DwellTime)
	{
		return;
	}

	UndockShip();
	LastStation = TargetStation;
	DockedSeconds = 0.0f;
	MinerState = EAIMinerState::SeekingAsteroid;
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------

ASpaceship* AAIMinerController::SpawnAIMiner(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station)
{
	return SpawnPilotedShip(World, ShipClass, Station, AAIMinerController::StaticClass());
}
