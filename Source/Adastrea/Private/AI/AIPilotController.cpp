// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIPilotController.h"
#include "AdastreaLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Stations/DockingBayModule.h"
#include "Stations/MarketplaceModule.h"
#include "Trading/CargoComponent.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"

namespace
{
	/** Seconds a ship will hover outside a full/blocked bay before trying another station. */
	constexpr float MaxBlockedSeconds = 20.0f;

	/** Cap on units bought per stop so one ship can't drain a market. */
	constexpr int32 MaxUnitsPerPurchase = 100;
}

AAIPilotController::AAIPilotController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAIPilotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	State = EAIPilotState::Choosing;
	if (ASpaceship* Ship = Cast<ASpaceship>(InPawn))
	{
		Ship->SetThrottle(0.0f);
	}
}

ASpaceship* AAIPilotController::GetShip() const
{
	return Cast<ASpaceship>(GetPawn());
}

void AAIPilotController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetShip())
	{
		TickPilot(DeltaSeconds);
	}
}

void AAIPilotController::TickPilot(float DeltaSeconds)
{
	switch (State)
	{
	case EAIPilotState::Choosing:
		TickChoosing();
		break;
	case EAIPilotState::Flying:
		TickFlying(DeltaSeconds);
		break;
	case EAIPilotState::Docked:
		TickDocked(DeltaSeconds);
		break;
	}
}

// ---------------------------------------------------------------------------
// Choosing
// ---------------------------------------------------------------------------

void AAIPilotController::TickChoosing()
{
	ChooseDestination();
	if (TargetStation)
	{
		DockedSeconds = 0.0f;
		State = EAIPilotState::Flying;
	}
}

void AAIPilotController::ChooseDestination()
{
	// A cargo run planned at the last stop wins.
	if (PlannedDestination && PlannedDestination != LastStation && PlannedDestination->GetDockingBayModule())
	{
		TargetStation = PlannedDestination;
		PlannedDestination = nullptr;
		return;
	}

	TArray<ASpaceStation*> Candidates;
	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		ASpaceStation* Station = *It;
		if (Station != LastStation && Station->GetDockingBayModule())
		{
			Candidates.Add(Station);
		}
	}

	// With only one station in the world, keep going back to it rather than idling forever.
	if (Candidates.IsEmpty() && LastStation && LastStation->GetDockingBayModule())
	{
		Candidates.Add(LastStation);
	}

	TargetStation = Candidates.IsEmpty() ? nullptr : Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	PlannedDestination = nullptr;
}

// ---------------------------------------------------------------------------
// Flying
// ---------------------------------------------------------------------------

void AAIPilotController::TickFlying(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	ADockingBayModule* Bay = TargetStation ? TargetStation->GetDockingBayModule() : nullptr;
	if (!Bay)
	{
		// Station or bay vanished mid-flight.
		Ship->SetThrottle(0.0f);
		State = EAIPilotState::Choosing;
		return;
	}

	switch (ApproachAndDock(Bay, DeltaSeconds))
	{
	case EApproachResult::Docked:
		DockedSeconds = 0.0f;
		State = EAIPilotState::Docked;
		if (bTradeAtStations)
		{
			TradeAtCurrentStation();
		}
		break;
	case EApproachResult::Blocked:
		UE_LOG(LogAdastreaShips, Log, TEXT("AIPilot %s could not dock at %s, trying another station"),
			*Ship->GetName(), *TargetStation->GetName());
		LastStation = TargetStation;
		TargetStation = nullptr;
		State = EAIPilotState::Choosing;
		break;
	case EApproachResult::Flying:
		break;
	}
}

AAIPilotController::EApproachResult AAIPilotController::ApproachAndDock(ADockingBayModule* Bay, float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	const FVector DockLocation = GetDockLocation(Bay);
	const float Distance = FVector::Dist(Ship->GetActorLocation(), DockLocation);

	if (Distance > Ship->GetEffectiveDockingRange() * 0.9f)
	{
		SteerToward(DockLocation, DeltaSeconds);
		return EApproachResult::Flying;
	}

	// In range: stop and ask for a berth the same way the player does.
	Ship->SetThrottle(0.0f);
	DockedSeconds += DeltaSeconds;

	Ship->SetNearbyStation(Bay);
	float DockDistance = 0.0f;
	FString StationName;
	if (Ship->CanRequestDocking(DockDistance, StationName))
	{
		DockPoint = Bay->GetAvailableDockingPoint();
		Ship->RequestDocking();
	}

	if (Ship->IsDocked())
	{
		if (Ship->MovementComponent)
		{
			Ship->MovementComponent->StopMovementImmediately();
		}
		UE_LOG(LogAdastreaShips, Log, TEXT("AIPilot %s docked at %s"), *Ship->GetName(), *Bay->GetName());
		return EApproachResult::Docked;
	}

	if (DockedSeconds > MaxBlockedSeconds)
	{
		Ship->SetNearbyStation(nullptr);
		return EApproachResult::Blocked;
	}
	return EApproachResult::Flying;
}

void AAIPilotController::UndockShip()
{
	ASpaceship* Ship = GetShip();

	// Ship::Undock frees the bay slot using NearbyStation, so clear that only afterwards.
	Ship->Undock();
	Ship->SetNearbyStation(nullptr);
	DockPoint = nullptr;
	if (Ship->MovementComponent)
	{
		Ship->MovementComponent->Velocity = Ship->GetActorForwardVector() * 500.0f;
	}
}

void AAIPilotController::SteerToward(const FVector& Location, float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	const FVector ToTarget = Location - Ship->GetActorLocation();
	const float Distance = ToTarget.Size();
	if (Distance < KINDA_SMALL_NUMBER)
	{
		return;
	}

	FRotator Desired = ToTarget.Rotation();
	Desired.Roll = 0.0f;
	const FRotator Current = Ship->GetActorRotation();
	Ship->SetActorRotation(FMath::RInterpConstantTo(Current, Desired, DeltaSeconds, Ship->TurnRate));

	// Ease off while still turning toward the target and again on final approach.
	const float Alignment = FVector::DotProduct(Ship->GetActorForwardVector(), ToTarget / Distance);
	const float TurnFactor = Alignment > 0.7f ? 1.0f : 0.3f;
	const float ApproachFactor = FMath::Clamp(Distance / (Ship->GetEffectiveDockingRange() * 4.0f), 0.25f, 1.0f);
	Ship->SetThrottle(CruiseThrottle * TurnFactor * ApproachFactor);
}

// ---------------------------------------------------------------------------
// Docked
// ---------------------------------------------------------------------------

void AAIPilotController::TickDocked(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();

	// Hold station: the ship's throttle decay would otherwise let it drift off the dock.
	if (DockPoint)
	{
		Ship->SetActorLocationAndRotation(DockPoint->GetComponentLocation(), DockPoint->GetComponentRotation());
	}

	DockedSeconds += DeltaSeconds;
	if (DockedSeconds < DwellTime)
	{
		return;
	}

	LastStation = TargetStation;
	ChooseDestination();
	if (!TargetStation)
	{
		// Nowhere to go yet; keep waiting at the dock.
		TargetStation = LastStation;
		DockedSeconds = 0.0f;
		return;
	}

	UndockShip();
	DockedSeconds = 0.0f;
	State = EAIPilotState::Flying;
}

// ---------------------------------------------------------------------------
// Trading
// ---------------------------------------------------------------------------

UMarketDataAsset* AAIPilotController::GetMarket(const ASpaceStation* Station) const
{
	const AMarketplaceModule* Marketplace = Station ? Station->GetMarketplaceModule() : nullptr;
	UMarketDataAsset* Market = Marketplace ? Marketplace->GetMarketData() : nullptr;
	return (Market && Market->bAllowAITraders) ? Market : nullptr;
}

FVector AAIPilotController::GetDockLocation(const ADockingBayModule* Bay) const
{
	const USceneComponent* Point = Bay->GetAvailableDockingPoint();
	return Point ? Point->GetComponentLocation() : Bay->GetActorLocation();
}

void AAIPilotController::TradeAtCurrentStation()
{
	ASpaceship* Ship = GetShip();
	UPlayerTraderComponent* Trader = Ship ? Ship->PlayerTraderComponent.Get() : nullptr;
	UCargoComponent* Cargo = Ship ? Ship->CargoComponent.Get() : nullptr;
	UMarketDataAsset* Here = GetMarket(TargetStation);
	if (!Trader || !Cargo || !Here)
	{
		return;
	}

	// Sell whatever we hauled here.
	for (const FCargoEntry& Entry : Cargo->GetCargoContents())
	{
		if (Entry.Item && Entry.Quantity > 0)
		{
			Trader->SellItem(Here, Entry.Item, Entry.Quantity, Cargo);
		}
	}

	// Find the item with the best buy-here / sell-there margin across every other market.
	UTradeItemDataAsset* BestItem = nullptr;
	ASpaceStation* BestStation = nullptr;
	int32 BestStock = 0;
	float BestMargin = 0.0f;

	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		ASpaceStation* Other = *It;
		UMarketDataAsset* OtherMarket = (Other != TargetStation && Other->GetDockingBayModule()) ? GetMarket(Other) : nullptr;
		if (!OtherMarket)
		{
			continue;
		}

		for (const FMarketInventoryEntry& Entry : Here->Inventory)
		{
			if (!Entry.TradeItem || Entry.CurrentStock <= 0)
			{
				continue;
			}

			const float Margin = OtherMarket->GetItemPrice(Entry.TradeItem, false) - Here->GetItemPrice(Entry.TradeItem, true);
			if (Margin > BestMargin)
			{
				BestMargin = Margin;
				BestItem = Entry.TradeItem;
				BestStation = Other;
				BestStock = Entry.CurrentStock;
			}
		}
	}

	if (!BestItem)
	{
		return; // Nothing profitable; the next hop is chosen at random.
	}

	int32 Quantity = FMath::Min(BestStock, MaxUnitsPerPurchase);
	while (Quantity > 0 && !(Trader->CanAfford(Here, BestItem, Quantity) && Cargo->HasSpaceFor(BestItem, Quantity)))
	{
		--Quantity;
	}

	if (Quantity > 0 && Trader->BuyItem(Here, BestItem, Quantity, Cargo))
	{
		PlannedDestination = BestStation;
		UE_LOG(LogAdastreaShips, Log, TEXT("AIPilot %s bought %d x %s for %s (margin %.1f/unit)"),
			*Ship->GetName(), Quantity, *BestItem->ItemName.ToString(), *BestStation->GetName(), BestMargin);
	}
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------

ASpaceship* AAIPilotController::SpawnAITrader(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station)
{
	return SpawnPilotedShip(World, ShipClass, Station, AAIPilotController::StaticClass());
}

ASpaceship* AAIPilotController::SpawnPilotedShip(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station, TSubclassOf<AAIPilotController> PilotClass)
{
	if (!World || !ShipClass)
	{
		return nullptr;
	}

	// Start a short way off the station's bay so the ship doesn't spawn inside geometry.
	FVector Location = FVector::ZeroVector;
	if (Station)
	{
		const ADockingBayModule* Bay = Station->GetDockingBayModule();
		const FVector Origin = Bay ? Bay->GetActorLocation() : Station->GetActorLocation();
		const FVector Offset = FVector(FMath::VRand().X, FMath::VRand().Y, 0.0f).GetSafeNormal() * FMath::FRandRange(4000.0f, 8000.0f);
		Location = Origin + Offset;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASpaceship* Ship = World->SpawnActor<ASpaceship>(ShipClass, Location, FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f), Params);
	if (!Ship)
	{
		return nullptr;
	}

	AAIPilotController* Pilot = World->SpawnActor<AAIPilotController>(PilotClass ? PilotClass : TSubclassOf<AAIPilotController>(AAIPilotController::StaticClass()), FTransform::Identity, Params);
	if (!Pilot)
	{
		Ship->Destroy();
		return nullptr;
	}

	Pilot->LastStation = Station;
	Pilot->Possess(Ship);
	return Ship;
}
