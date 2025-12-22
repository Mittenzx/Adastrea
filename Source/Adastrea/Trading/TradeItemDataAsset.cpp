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
