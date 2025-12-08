#include "TradeItemDataAsset.h"
#include "Materials/MaterialDataAsset.h"
#include "AdastreaLog.h"

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
    // Define price calculation constants
    static constexpr float kMinSupplyValue = 0.1f;
    static constexpr float kNormalMultiplier = 1.0f;
    
    // Input validation
    if (Supply < 0.0f || Demand < 0.0f || MarketEventMultiplier < 0.0f)
    {
        UE_LOG(LogAdastreaTrading, Warning, TEXT("TradeItemDataAsset::CalculatePrice - Invalid input parameters for %s"), *ItemID.ToString());
        return BasePrice;
    }

    float CalculatedPrice = BasePrice;

    // Apply supply/demand if enabled
    if (bAffectedBySupplyDemand)
    {
        // Supply reduces price (more supply = lower price)
        // Use FMath::Max to prevent division by very small numbers
        const float SupplyFactor = FMath::Clamp(
            kNormalMultiplier / FMath::Max(Supply, kMinSupplyValue), 
            PriceVolatility.MinPriceDeviation, 
            PriceVolatility.MaxPriceDeviation
        );
        
        // Demand increases price (more demand = higher price)
        const float DemandFactor = FMath::Clamp(
            Demand, 
            PriceVolatility.MinPriceDeviation, 
            PriceVolatility.MaxPriceDeviation
        );
        
        CalculatedPrice *= SupplyFactor * DemandFactor * PriceVolatility.VolatilityMultiplier;
    }

    // Apply market events if enabled
    if (bAffectedByMarketEvents)
    {
        CalculatedPrice *= MarketEventMultiplier;
    }

    // Clamp final price to reasonable bounds
    CalculatedPrice = FMath::Clamp(
        CalculatedPrice, 
        BasePrice * PriceVolatility.MinPriceDeviation,
        BasePrice * PriceVolatility.MaxPriceDeviation
    );

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
    // Input validation
    if (FactionID.IsNone())
    {
        UE_LOG(LogAdastreaTrading, Warning, TEXT("TradeItemDataAsset::CanBeTradedByFaction - Invalid FactionID for %s"), *ItemID.ToString());
        return false;
    }

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
    // Input validation
    if (FactionID.IsNone())
    {
        return false;
    }

    return TradeRestrictions.RequiresPermitFactions.Contains(FactionID);
}

float UTradeItemDataAsset::CalculateContrabandFine(int32 Quantity) const
{
    // Validate quantity
    if (Quantity <= 0)
    {
        return 0.0f;
    }

    // Legal items have no fine
    if (LegalityStatus == ELegalityStatus::Legal)
    {
        return 0.0f;
    }

    return BasePrice * Quantity * ContrabandFineMultiplier;
}

bool UTradeItemDataAsset::HasBehaviorTag(FName Tag) const
{
    // Input validation
    if (Tag.IsNone())
    {
        return false;
    }

    return BehaviorTags.Contains(Tag);
}

float UTradeItemDataAsset::GetTotalVolume(int32 Quantity) const
{
    // Validate quantity
    if (Quantity < 0)
    {
        UE_LOG(LogAdastreaTrading, Warning, TEXT("TradeItemDataAsset::GetTotalVolume - Negative quantity %d for %s"), Quantity, *ItemID.ToString());
        return 0.0f;
    }

    return VolumePerUnit * Quantity;
}

float UTradeItemDataAsset::GetTotalMass(int32 Quantity) const
{
    // Validate quantity
    if (Quantity < 0)
    {
        UE_LOG(LogAdastreaTrading, Warning, TEXT("TradeItemDataAsset::GetTotalMass - Negative quantity %d for %s"), Quantity, *ItemID.ToString());
        return 0.0f;
    }

    return MassPerUnit * Quantity;
}

bool UTradeItemDataAsset::IsHighValue() const
{
    // Define high-value threshold as a constant
    static constexpr float kHighValueThreshold = 1000.0f;
    
    // Consider items with base price above threshold as high-value
    return BasePrice > kHighValueThreshold;
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

#if WITH_EDITOR
EDataValidationResult UTradeItemDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	// Validate basic info
	if (ItemName.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Item Name is empty")));
		Result = EDataValidationResult::Invalid;
	}

	if (ItemID.IsNone())
	{
		ValidationErrors.Add(FText::FromString(TEXT("Item ID is not set")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate pricing
	if (BasePrice <= 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Base Price must be greater than 0")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate physical properties
	if (VolumePerUnit <= 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Volume Per Unit must be greater than 0")));
		Result = EDataValidationResult::Invalid;
	}

	if (MassPerUnit <= 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Mass Per Unit must be greater than 0")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate price volatility settings
	if (PriceVolatility.MinPriceDeviation < 0.0f || PriceVolatility.MinPriceDeviation > 1.0f)
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Min Price Deviation ({0}) must be between 0.0 and 1.0")),
			FText::AsNumber(PriceVolatility.MinPriceDeviation)
		));
		Result = EDataValidationResult::Invalid;
	}

	if (PriceVolatility.MaxPriceDeviation < 1.0f || PriceVolatility.MaxPriceDeviation > 10.0f)
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Max Price Deviation ({0}) must be between 1.0 and 10.0")),
			FText::AsNumber(PriceVolatility.MaxPriceDeviation)
		));
		Result = EDataValidationResult::Invalid;
	}

	if (PriceVolatility.MinPriceDeviation >= PriceVolatility.MaxPriceDeviation)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Min Price Deviation must be less than Max Price Deviation")));
		Result = EDataValidationResult::Invalid;
	}

	// Validate trade restrictions
	if (TradeRestrictions.MinReputationRequired < -100 || TradeRestrictions.MinReputationRequired > 100)
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Min Reputation Required ({0}) must be between -100 and 100")),
			FText::AsNumber(TradeRestrictions.MinReputationRequired)
		));
		Result = EDataValidationResult::Invalid;
	}

	// Warn about potential issues
	if (LegalityStatus == ELegalityStatus::Illegal || LegalityStatus == ELegalityStatus::Contraband)
	{
		if (ContrabandRiskLevel <= 0.0f)
		{
			ValidationErrors.Add(FText::FromString(TEXT("Warning: Illegal/Contraband item should have positive risk level")));
			// Just a warning
		}
	}

	if (Category == ETradeItemCategory::Luxury && BasePrice < 500.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("Warning: Luxury item has relatively low base price")));
		// Just a warning
	}

	// Log validation result
	if (Result == EDataValidationResult::Valid)
	{
		UE_LOG(LogAdastreaTrading, Log, TEXT("TradeItemDataAsset %s passed validation"), *ItemName.ToString());
	}
	else
	{
		UE_LOG(LogAdastreaTrading, Warning, TEXT("TradeItemDataAsset %s failed validation with %d errors"), 
			*ItemName.ToString(), ValidationErrors.Num());
	}

	return Result;
}
#endif
