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
