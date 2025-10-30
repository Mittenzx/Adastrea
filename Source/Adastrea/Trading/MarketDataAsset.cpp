#include "MarketDataAsset.h"
#include "TradeItemDataAsset.h"
#include "Public/Factions/FactionDataAsset.h"

UMarketDataAsset::UMarketDataAsset()
	: MarketName(FText::FromString(TEXT("Trade Market")))
	, Description(FText::FromString(TEXT("A trading market for buying and selling goods.")))
	, MarketID(NAME_None)
	, MarketType(EMarketType::OpenMarket)
	, MarketSize(EMarketSize::Medium)
	, ControllingFaction(nullptr)
	, TransactionTaxRate(0.05f)
	, SellPriceMarkup(1.2f)
	, BuyPriceMarkdown(0.8f)
	, bAllowPlayerBuying(true)
	, bAllowPlayerSelling(true)
	, bAllowAITraders(true)
	, MinReputationRequired(-50)
	, StockRefreshRate(24.0f)
	, LastStockRefreshTime(0.0f)
	, RandomEventChance(0.1f)
	, AITraderCount(5)
	, AITradeFrequency(10)
	, bAllowAIPriceManipulation(true)
{
}

float UMarketDataAsset::GetItemPrice(UTradeItemDataAsset* TradeItem, bool bIsBuying) const
{
	if (!TradeItem)
	{
		return 0.0f;
	}

	// Find inventory entry for supply/demand data
	FMarketInventoryEntry FoundEntry;
	bool bFound = GetInventoryEntry(TradeItem->ItemID, FoundEntry);

	float Supply = bFound ? FoundEntry.SupplyLevel : 1.0f;
	float Demand = bFound ? FoundEntry.DemandLevel : 1.0f;

	// Get event multiplier
	float EventMultiplier = GetEventPriceMultiplier(TradeItem->ItemID);

	// Calculate base price using item's formula
	float BasePrice = TradeItem->CalculatePrice(Supply, Demand, EventMultiplier);

	// Apply market markup/markdown
	if (bIsBuying)
	{
		// Player buying = higher price
		BasePrice *= SellPriceMarkup;
	}
	else
	{
		// Player selling = lower price
		BasePrice *= BuyPriceMarkdown;
	}

	// Apply transaction tax
	BasePrice *= (1.0f + TransactionTaxRate);

	// Call Blueprint override
	return OnCalculateCustomMarketPrice(TradeItem, bIsBuying, BasePrice);
}

bool UMarketDataAsset::GetInventoryEntry(FName ItemID, FMarketInventoryEntry& OutEntry) const
{
	for (const FMarketInventoryEntry& Entry : Inventory)
	{
		if (Entry.TradeItem && Entry.TradeItem->ItemID == ItemID)
		{
			OutEntry = Entry;
			return true;
		}
	}
	return false;
}

bool UMarketDataAsset::IsItemInStock(FName ItemID, int32 Quantity) const
{
	FMarketInventoryEntry Entry;
	if (GetInventoryEntry(ItemID, Entry))
	{
		return Entry.bInStock && Entry.CurrentStock >= Quantity;
	}
	return false;
}

TArray<FMarketInventoryEntry> UMarketDataAsset::GetItemsByCategory(ETradeItemCategory Category) const
{
	TArray<FMarketInventoryEntry> Result;
	
	for (const FMarketInventoryEntry& Entry : Inventory)
	{
		if (Entry.TradeItem && Entry.TradeItem->Category == Category)
		{
			Result.Add(Entry);
		}
	}
	
	return Result;
}

bool UMarketDataAsset::CanPlayerAccess(int32 PlayerReputation) const
{
	return PlayerReputation >= MinReputationRequired;
}

TArray<FMarketEvent> UMarketDataAsset::GetActiveEventsForItem(FName ItemID) const
{
	TArray<FMarketEvent> Result;
	
	for (const FMarketEvent& Event : ActiveEvents)
	{
		if (!Event.bIsActive)
		{
			continue;
		}

		// If event has no specific items, it affects all items
		if (Event.AffectedItemIDs.Num() == 0 || Event.AffectedItemIDs.Contains(ItemID))
		{
			Result.Add(Event);
		}
	}
	
	return Result;
}

float UMarketDataAsset::GetEventPriceMultiplier(FName ItemID) const
{
	float TotalMultiplier = 1.0f;
	
	TArray<FMarketEvent> Events = GetActiveEventsForItem(ItemID);
	for (const FMarketEvent& Event : Events)
	{
		TotalMultiplier *= Event.PriceMultiplier;
	}
	
	return TotalMultiplier;
}

void UMarketDataAsset::UpdateMarket(float DeltaHours)
{
	// Update stock refresh
	if (StockRefreshRate > 0.0f)
	{
		LastStockRefreshTime += DeltaHours;
		if (LastStockRefreshTime >= StockRefreshRate)
		{
			RefreshStock();
			LastStockRefreshTime = 0.0f;
		}
	}

	// Update supply/demand based on trading activity
	for (FMarketInventoryEntry& Entry : Inventory)
	{
		if (!Entry.TradeItem)
		{
			continue;
		}

		// Gradually normalize supply and demand towards 1.0
		float NormalizationRate = DeltaHours / 24.0f; // Normalize over 24 hours
		Entry.SupplyLevel = FMath::Lerp(Entry.SupplyLevel, 1.0f, NormalizationRate);
		Entry.DemandLevel = FMath::Lerp(Entry.DemandLevel, 1.0f, NormalizationRate);
	}
}

void UMarketDataAsset::RefreshStock()
{
	for (FMarketInventoryEntry& Entry : Inventory)
	{
		if (!Entry.TradeItem)
		{
			continue;
		}

		// Replenish stock
		int32 ReplenishAmount = Entry.TradeItem->ReplenishmentRate;
		Entry.CurrentStock = FMath::Min(Entry.CurrentStock + ReplenishAmount, Entry.MaxStock);
		Entry.bInStock = Entry.CurrentStock > 0;
	}

	OnStockRefreshed();
}

void UMarketDataAsset::UpdateMarketEvents(float CurrentGameTime)
{
	for (FMarketEvent& Event : ActiveEvents)
	{
		if (!Event.bIsActive)
		{
			continue;
		}

		// Check if event should expire
		if (Event.DurationHours > 0.0f)
		{
			float ElapsedTime = CurrentGameTime - Event.StartTime;
			if (ElapsedTime >= Event.DurationHours)
			{
				Event.bIsActive = false;
				OnMarketEventEnded(Event);
			}
		}
	}
}

// BlueprintNativeEvent implementations
float UMarketDataAsset::OnCalculateCustomMarketPrice_Implementation(UTradeItemDataAsset* TradeItem, bool bIsBuying, float BasePrice) const
{
	// Default implementation returns base price
	// Designers can override in Blueprint
	return BasePrice;
}

void UMarketDataAsset::OnStockRefreshed_Implementation()
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}

void UMarketDataAsset::OnMarketEventStarted_Implementation(const FMarketEvent& Event)
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}

void UMarketDataAsset::OnMarketEventEnded_Implementation(const FMarketEvent& Event)
{
	// Default implementation does nothing
	// Designers can override in Blueprint
}
