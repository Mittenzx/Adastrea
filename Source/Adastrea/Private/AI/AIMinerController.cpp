// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIMinerController.h"
#include "Drones/DroneBayComponent.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Interfaces/ITargetable.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidField.h"
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

	// Every ship has a drone bay but only ships with a MiningRating carry mining drones.
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->DroneBay)
	{
		Ship->DroneBay->bMiningEnabled = true;
		Ship->DroneBay->MiningPower = FMath::Max(Ship->DroneBay->MiningPower, 1.0f);
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
	const FVector Hatch = Ship->DroneBay ? Ship->DroneBay->GetHatchLocation() : Ship->GetActorLocation();
	return FVector::Dist(Hatch, Rock->GetActorLocation()) - Rock->GetRadius();
}

// ---------------------------------------------------------------------------
// Finding and reaching a rock
// ---------------------------------------------------------------------------

void AAIMinerController::TickSeeking()
{
	ASpaceship* Ship = GetShip();

	// Leave rocks other miners are already working so a group spreads across the field.
	TSet<const AAsteroid*> Claimed;
	for (TActorIterator<AAIMinerController> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->TargetAsteroid)
		{
			Claimed.Add(It->TargetAsteroid);
		}
	}

	AAsteroid* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AAsteroid> It(GetWorld()); It; ++It)
	{
		if (IsMinable(*It) && !Claimed.Contains(*It))
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
		UE_LOG(LogAdastreaShips, Log, TEXT("AIMiner %s heading for %s"), *Ship->GetName(), *Best->GetName());
		return;
	}

	// Asteroid fields only spawn real rocks near a ship, so with none loaded fly toward the
	// nearest field; its rocks appear as we get close.
	const AAsteroidField* NearestField = nullptr;
	float FieldDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AAsteroidField> It(GetWorld()); It; ++It)
	{
		const float Distance = FVector::Dist(Ship->GetActorLocation(), It->GetActorLocation());
		if (Distance < FieldDistance)
		{
			FieldDistance = Distance;
			NearestField = *It;
		}
	}

	if (NearestField && FieldDistance > NearestField->PromoteRadius * 0.5f)
	{
		SteerToward(NearestField->GetActorLocation(), GetWorld()->GetDeltaSeconds());
	}
	else
	{
		Ship->SetThrottle(0.0f);
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

	if (SurfaceDistance(TargetAsteroid) > Ship->DroneBay->Range * StandOffFraction)
	{
		SteerToward(TargetAsteroid->GetActorLocation(), DeltaSeconds);
		return;
	}

	// In drone range: stop, lock the rock and send the drones out.
	Ship->SetThrottle(0.0f);
	if (Ship->DroneBay->SetTarget(TargetAsteroid))
	{
		Ship->DroneBay->LaunchDrones();
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
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->DroneBay)
	{
		// Drones still out fly home to the ship wherever it goes next.
		Ship->DroneBay->RecallDrones();
		Ship->DroneBay->ClearTarget();
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
	UDroneBayComponent* Bay = Ship->DroneBay;
	Ship->SetThrottle(0.0f);

	if (Bay->IsHoldFull())
	{
		// Wait for every drone to land before leaving with the load.
		Bay->RecallDrones();
		if (Bay->GetDronesOut() > 0)
		{
			return;
		}
		Bay->ClearTarget();
		TargetAsteroid = nullptr; // free the rock for other miners while we sell
		DockedSeconds = 0.0f;
		ChooseSellStation();
		MinerState = TargetStation ? EAIMinerState::ToStation : EAIMinerState::SeekingAsteroid;
		UE_LOG(LogAdastreaShips, Log, TEXT("AIMiner %s hold full, drones aboard, heading to %s"),
			*Ship->GetName(), TargetStation ? *TargetStation->GetName() : TEXT("(no station)"));
		return;
	}

	if (!IsMinable(TargetAsteroid))
	{
		// Rock is spent; the drones are already heading home. Move on to the next one.
		AbandonAsteroid();
		return;
	}

	switch (Bay->GetStatus())
	{
	case EDroneBayStatus::OutOfRange:
		// Drifted or the rock moved; close the gap again (the drones stay on the job).
		MinerState = EAIMinerState::ToAsteroid;
		return;
	case EDroneBayStatus::NoTarget:
		Bay->SetTarget(TargetAsteroid);
		break;
	default:
		break;
	}
	if (!Bay->IsDeployed())
	{
		// Recalled while the hold looked full, but there was room after all: back to work.
		Bay->LaunchDrones();
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

FString AAIMinerController::GetObjectiveDescription() const
{
	switch (MinerState)
	{
	case EAIMinerState::SeekingAsteroid:
		return TEXT("Looking for an asteroid");
	case EAIMinerState::ToAsteroid:
		return FString::Printf(TEXT("Flying to %s"), TargetAsteroid ? *TargetAsteroid->GetName() : TEXT("(lost rock)"));
	case EAIMinerState::Mining:
		return TargetAsteroid
			? FString::Printf(TEXT("Mining %s (%.0f%% ore left)"), *TargetAsteroid->GetName(), TargetAsteroid->GetOreFraction() * 100.0f)
			: FString(TEXT("Mining"));
	case EAIMinerState::ToStation:
		return FString::Printf(TEXT("Hold full, hauling ore to %s"), *GetStationDisplayName(TargetStation));
	case EAIMinerState::Docked:
		return FString::Printf(TEXT("Selling ore at %s, leaving in %.0fs"), *GetStationDisplayName(TargetStation),
			FMath::Max(0.0f, DwellTime - DockedSeconds));
	}
	return FString();
}
