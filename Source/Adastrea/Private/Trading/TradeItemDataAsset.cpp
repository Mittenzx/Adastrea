#include "Trading/TradeItemDataAsset.h"
#include "Misc/DataValidation.h"
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
    // For now, return base price - faction modifiers can be implemented later
    return BasePriceToModify;
}

bool UTradeItemDataAsset::CanBeTradedByFaction(FName FactionID, int32 Reputation) const
{
    // Check if faction is banned
    if (TradeRestrictions.BannedFactions.Contains(FactionID))
    {
        return false;
    }

    // Check reputation requirement
    if (Reputation < TradeRestrictions.MinReputationRequired)
    {
        return false;
    }

    return true;
}

bool UTradeItemDataAsset::RequiresPermit(FName FactionID) const
{
    return TradeRestrictions.RequiresPermitFactions.Contains(FactionID);
}

float UTradeItemDataAsset::CalculateContrabandFine(int32 Quantity) const
{
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
    // Consider high value if base price is above 10000 credits
    return BasePrice > 10000.0f;
}

float UTradeItemDataAsset::OnCalculateCustomPrice_Implementation(float Supply, float Demand, float EventMultiplier, float BaseCalculatedPrice) const
{
    // Default implementation - just return the calculated price
    return BaseCalculatedPrice;
}

void UTradeItemDataAsset::OnItemTraded_Implementation(int32 Quantity, float Price, FName BuyerFactionID, FName SellerFactionID)
{
    // Default implementation - do nothing
    // Blueprint can override for custom behavior
}

#if WITH_EDITOR
EDataValidationResult UTradeItemDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    // Validate required fields
    if (ItemName.IsEmpty())
    {
        Context.AddError(FText::FromString(TEXT("Item name cannot be empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (ItemID.IsNone())
    {
        Context.AddError(FText::FromString(TEXT("Item ID cannot be empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (BasePrice <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Base price must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (VolumePerUnit <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Volume per unit must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (MassPerUnit <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Mass per unit must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (StandardLotSize <= 0)
    {
        Context.AddError(FText::FromString(TEXT("Standard lot size must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (AITradePriority < 1 || AITradePriority > 10)
    {
        Context.AddError(FText::FromString(TEXT("AI trade priority must be between 1 and 10")));
        Result = EDataValidationResult::Invalid;
    }

    return Result;
}
#endif
