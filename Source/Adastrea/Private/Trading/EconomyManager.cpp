#include "Trading/EconomyManager.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/TradeItemDataAsset.h"
#include "TimerManager.h"
#include "Engine/World.h"

UEconomyManager::UEconomyManager()
	: CurrentGameTime(0.0f)
	, TimeScale(1.0f)
	, UpdateInterval(5.0f)
	, SupplyDemandAdjustmentRate(0.05f)  // 5% change per transaction
	, MinSupplyDemandLevel(0.1f)
	, MaxSupplyDemandLevel(3.0f)
	, EconomicRecoveryRate(0.1f)  // 10% recovery per game hour
{
}

void UEconomyManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("EconomyManager: Initializing"));

	CurrentGameTime = 0.0f;
	TimeScale = 1.0f;
	UpdateInterval = 5.0f;

	// Start update timer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			this,
			&UEconomyManager::UpdateEconomy,
			UpdateInterval,
			true  // Loop
		);

		UE_LOG(LogTemp, Log, TEXT("EconomyManager: Update timer started (interval: %.1f seconds)"), UpdateInterval);
	}
}

void UEconomyManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("EconomyManager: Deinitializing"));

	// Clear timer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	ActiveMarkets.Empty();

	Super::Deinitialize();
}

void UEconomyManager::RegisterMarket(UMarketDataAsset* Market)
{
	if (!Market)
	{
		UE_LOG(LogTemp, Warning, TEXT("EconomyManager: Cannot register null market"));
		return;
	}

	if (ActiveMarkets.Contains(Market))
	{
		UE_LOG(LogTemp, Warning, TEXT("EconomyManager: Market '%s' already registered"), *Market->MarketName.ToString());
		return;
	}

	ActiveMarkets.Add(Market);
	UE_LOG(LogTemp, Log, TEXT("EconomyManager: Registered market '%s' (total: %d)"), *Market->MarketName.ToString(), ActiveMarkets.Num());
}

void UEconomyManager::UnregisterMarket(UMarketDataAsset* Market)
{
	if (!Market)
	{
		return;
	}

	if (ActiveMarkets.Remove(Market) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("EconomyManager: Unregistered market '%s' (total: %d)"), *Market->MarketName.ToString(), ActiveMarkets.Num());
	}
}

bool UEconomyManager::IsMarketRegistered(UMarketDataAsset* Market) const
{
	return Market && ActiveMarkets.Contains(Market);
}

void UEconomyManager::UpdateEconomy()
{
	// Convert update interval to game time hours
	// 1 real second = 1 game minute by default (60x speed)
	float DeltaHours = (UpdateInterval * TimeScale) / 60.0f;
	CurrentGameTime += DeltaHours;

	// Update all markets
	for (UMarketDataAsset* Market : ActiveMarkets)
	{
		if (Market)
		{
			UpdateMarketPrices(Market, DeltaHours);
			SimulateBackgroundActivity(Market, DeltaHours);
		}
	}
}

float UEconomyManager::GetItemPrice(UMarketDataAsset* Market, UTradeItemDataAsset* Item, bool bIsBuying) const
{
	if (!Market || !Item)
	{
		return 0.0f;
	}

	return Market->GetItemPrice(Item, bIsBuying);
}

void UEconomyManager::RecordTransaction(UMarketDataAsset* Market, UTradeItemDataAsset* Item, int32 Quantity, bool bPlayerBought)
{
	if (!Market || !Item || Quantity <= 0)
	{
		return;
	}

	// NOTE: This method modifies the MarketDataAsset's Inventory array at runtime.
	// The Data Asset serves as a template, but runtime market state is mutable and won't persist
	// between sessions unless explicitly saved. For persistent state, consider implementing
	// a separate runtime market state structure that references the Data Asset template.

	// Find inventory entry
	for (FMarketInventoryEntry& Entry : Market->Inventory)
	{
		if (Entry.TradeItem == Item)
		{
			if (bPlayerBought)
			{
				// Player bought from station
				Entry.CurrentStock = FMath::Max(0, Entry.CurrentStock - Quantity);
				Entry.SupplyLevel *= (1.0f - SupplyDemandAdjustmentRate);  // Supply decreased
				Entry.DemandLevel *= (1.0f + SupplyDemandAdjustmentRate);  // Demand increased

				UE_LOG(LogTemp, Log, TEXT("EconomyManager: Player bought %d x %s from %s"), 
					Quantity, *Item->ItemName.ToString(), *Market->MarketName.ToString());
			}
			else
			{
				// Player sold to station
				Entry.CurrentStock += Quantity;
				Entry.SupplyLevel *= (1.0f + SupplyDemandAdjustmentRate);  // Supply increased
				Entry.DemandLevel *= (1.0f - SupplyDemandAdjustmentRate);  // Demand decreased

				UE_LOG(LogTemp, Log, TEXT("EconomyManager: Player sold %d x %s to %s"), 
					Quantity, *Item->ItemName.ToString(), *Market->MarketName.ToString());
			}

			// Clamp supply/demand to reasonable ranges
			Entry.SupplyLevel = FMath::Clamp(Entry.SupplyLevel, MinSupplyDemandLevel, MaxSupplyDemandLevel);
			Entry.DemandLevel = FMath::Clamp(Entry.DemandLevel, MinSupplyDemandLevel, MaxSupplyDemandLevel);

			// Update stock status
			Entry.bInStock = Entry.CurrentStock > 0;

			break;
		}
	}
}

void UEconomyManager::UpdateMarketPrices(UMarketDataAsset* Market, float DeltaHours)
{
	if (!Market)
	{
		return;
	}

	// Gradually return supply/demand to baseline (recovery)
	const float RecoveryRate = EconomicRecoveryRate * DeltaHours;

	for (FMarketInventoryEntry& Entry : Market->Inventory)
	{
		// Move supply/demand back toward 1.0 (normal) over time
		Entry.SupplyLevel = FMath::Lerp(Entry.SupplyLevel, 1.0f, RecoveryRate);
		Entry.DemandLevel = FMath::Lerp(Entry.DemandLevel, 1.0f, RecoveryRate);
	}
}

void UEconomyManager::SimulateBackgroundActivity(UMarketDataAsset* Market, float DeltaHours)
{
	if (!Market)
	{
		return;
	}

	// Simple stock replenishment
	for (FMarketInventoryEntry& Entry : Market->Inventory)
	{
		if (Entry.TradeItem)
		{
			// Replenish stock based on item's replenishment rate
			int32 ReplenishAmount = FMath::RoundToInt(Entry.TradeItem->ReplenishmentRate * DeltaHours);
			
			if (ReplenishAmount > 0)
			{
				Entry.CurrentStock = FMath::Min(Entry.CurrentStock + ReplenishAmount, Entry.MaxStock);
				Entry.bInStock = Entry.CurrentStock > 0;
			}
		}
	}
}
