#include "TradeItemDataAsset.h"
#include "../Materials/MaterialDataAsset.h"

UTradeItemDataAsset::UTradeItemDataAsset()
	: ItemName(FText::FromString(TEXT("Trade Item")))
	, Description(FText::FromString(TEXT("A tradeable item.")))
	, ItemID(NAME_None)
	, Category(ETradeItemCategory::Other)
	, MaterialData(nullptr)
	, BasePrice(100.0f)
	, bAffectedBySupplyDemand(true)
	, bAffectedByMarketEvents(true)
	, LegalityStatus(ELegalityStatus::Legal)
	, ContrabandFineMultiplier(5.0f)
	, VolumePerUnit(1.0f)
	, MassPerUnit(1.0f)
	, StandardLotSize(100)
	, TypicalMarketStock(1000)
	, ReplenishmentRate(100)
	, AITradePriority(5)
	, bAIHoardable(false)
	, bAIArbitrageEnabled(true)
{
}

float UTradeItemDataAsset::CalculatePrice(float Supply, float Demand, float MarketEventMultiplier) const
{
	float CalculatedPrice = BasePrice;

	// Apply supply/demand if enabled
	if (bAffectedBySupplyDemand)
	{
		// Supply reduces price (more supply = lower price)
		float SupplyFactor = FMath::Clamp(1.0f / FMath::Max(Supply, 0.1f), PriceVolatility.MinPriceDeviation, PriceVolatility.MaxPriceDeviation);
		
		// Demand increases price (more demand = higher price)
		float DemandFactor = FMath::Clamp(Demand, PriceVolatility.MinPriceDeviation, PriceVolatility.MaxPriceDeviation);
		
		CalculatedPrice *= SupplyFactor * DemandFactor * PriceVolatility.VolatilityMultiplier;
	}

	// Apply market events if enabled
	if (bAffectedByMarketEvents)
	{
		CalculatedPrice *= MarketEventMultiplier;
	}

	// Clamp final price to reasonable bounds
	CalculatedPrice = FMath::Clamp(CalculatedPrice, 
		BasePrice * PriceVolatility.MinPriceDeviation,
		BasePrice * PriceVolatility.MaxPriceDeviation);

	// Call Blueprint override if implemented
	return OnCalculateCustomPrice(Supply, Demand, MarketEventMultiplier, CalculatedPrice);
}

float UTradeItemDataAsset::GetFactionModifiedPrice(float BasePriceToModify, FName BuyerFactionID, FName SellerFactionID) const
{
	// Base implementation - can be extended in Blueprint
	// This would require faction relationship lookup which would be done in a manager class
	// For now, return the base price
	return BasePriceToModify;
}

bool UTradeItemDataAsset::CanBeTradedByFaction(FName FactionID, int32 Reputation) const
{
	// Check if faction is banned from trading this item
	if (TradeRestrictions.BannedFactions.Contains(FactionID))
	{
		return false;
	}

	// Check reputation requirement
	if (Reputation < TradeRestrictions.MinReputationRequired)
	{
		return false;
	}

	// Check legality status
	if (LegalityStatus == ELegalityStatus::Illegal || LegalityStatus == ELegalityStatus::Contraband)
	{
		// Illegal items may still be traded by certain factions (smugglers, etc.)
		// This would be checked in a more complex system
		return false; // For now, assume illegal = cannot trade
	}

	return true;
}

bool UTradeItemDataAsset::RequiresPermit(FName FactionID) const
{
	return TradeRestrictions.RequiresPermitFactions.Contains(FactionID);
}

float UTradeItemDataAsset::CalculateContrabandFine(int32 Quantity) const
{
	if (LegalityStatus == ELegalityStatus::Legal)
	{
		return 0.0f;
	}

	return BasePrice * Quantity * ContrabandFineMultiplier;
}

bool UTradeItemDataAsset::HasBehaviorTag(FName Tag) const
{
	return BehaviorTags.Contains(Tag);
}

float UTradeItemDataAsset::GetTotalVolume(int32 Quantity) const
{
	return VolumePerUnit * Quantity;
}

float UTradeItemDataAsset::GetTotalMass(int32 Quantity) const
{
	return MassPerUnit * Quantity;
}

bool UTradeItemDataAsset::IsHighValue() const
{
	// Consider items with base price > 1000 as high-value
	return BasePrice > 1000.0f;
}

// BlueprintNativeEvent implementations
float UTradeItemDataAsset::OnCalculateCustomPrice_Implementation(float Supply, float Demand, float EventMultiplier, float BaseCalculatedPrice) const
{
	// Default implementation returns the base calculated price
	// Designers can override in Blueprint to add custom logic
	return BaseCalculatedPrice;
}

void UTradeItemDataAsset::OnItemTraded_Implementation(int32 Quantity, float Price, FName BuyerFactionID, FName SellerFactionID)
{
	// Default implementation does nothing
	// Designers can override in Blueprint to trigger custom events
}
