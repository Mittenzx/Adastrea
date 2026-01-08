#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TradeItemDataAsset.generated.h"

// Forward declarations
class UMaterialDataAsset;

/**
 * Enum for trade item categories
 * Categorizes items for market filtering and AI behavior
 */
UENUM(BlueprintType)
enum class ETradeItemCategory : uint8
{
	RawMaterials UMETA(DisplayName = "Raw Materials"),
	RefinedGoods UMETA(DisplayName = "Refined Goods"),
	Components UMETA(DisplayName = "Components"),
	Technology UMETA(DisplayName = "Technology"),
	Food UMETA(DisplayName = "Food & Consumables"),
	Luxury UMETA(DisplayName = "Luxury Goods"),
	Contraband UMETA(DisplayName = "Contraband"),
	Military UMETA(DisplayName = "Military Equipment"),
	Medical UMETA(DisplayName = "Medical Supplies"),
	Data UMETA(DisplayName = "Data & Information"),
	Other UMETA(DisplayName = "Other")
};

/**
 * Enum for trade legality status
 * Determines if item can be traded legally in different jurisdictions
 */
UENUM(BlueprintType)
enum class ELegalityStatus : uint8
{
	Legal UMETA(DisplayName = "Legal"),
	Regulated UMETA(DisplayName = "Regulated"),
	Restricted UMETA(DisplayName = "Restricted"),
	Illegal UMETA(DisplayName = "Illegal"),
	Contraband UMETA(DisplayName = "Contraband")
};

/**
 * Price volatility settings for dynamic pricing
 */
USTRUCT(BlueprintType)
struct FPriceVolatility
{
	GENERATED_BODY()

	// Base volatility multiplier (0.0 = stable, 1.0 = normal, 2.0+ = very volatile)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing", meta=(ClampMin="0.0", ClampMax="5.0"))
	float VolatilityMultiplier;

	// Minimum price deviation as percentage (e.g., 0.5 = 50% below base)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinPriceDeviation;

	// Maximum price deviation as percentage (e.g., 2.0 = 200% above base)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MaxPriceDeviation;

	FPriceVolatility()
		: VolatilityMultiplier(1.0f)
		, MinPriceDeviation(0.5f)
		, MaxPriceDeviation(2.0f)
	{}
};

/**
 * Trade restrictions (placeholder for future use)
 * Currently unused in MVP - all items accessible to all players
 * 
 * Note: Keeping minimal struct to maintain serialization compatibility
 * REMOVED in MVP: BannedFactions, RequiresPermitFactions, MinReputationRequired
 */
USTRUCT(BlueprintType)
struct FTradeRestrictions
{
	GENERATED_BODY()

	// Placeholder field to ensure struct validity for serialization
	// MVP has no trade restrictions - all items accessible to all players
	UPROPERTY()
	bool bReservedForFutureUse = false;

	FTradeRestrictions()
		: bReservedForFutureUse(false)
	{
	}
};

/**
 * Data Asset for tradeable items
 * Combines material data with trade-specific information
 */
UCLASS(BlueprintType)
class ADASTREA_API UTradeItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// BASIC INFO
	// ====================

	// Display name of the trade item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
	FText ItemName;

	// Description of the trade item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info", meta=(MultiLine=true))
	FText Description;

	// Unique identifier for the trade item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
	FName ItemID;

	// Trade category
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
	ETradeItemCategory Category;

	// Optional reference to material data asset for resource-based items
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Info")
	UMaterialDataAsset* MaterialData;

	// ====================
	// PRICING
	// ====================

	// Base price in credits
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing", meta=(ClampMin="0"))
	float BasePrice;

	// Price volatility settings for dynamic pricing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
	FPriceVolatility PriceVolatility;

	// Whether this item's price is affected by supply/demand
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
	bool bAffectedBySupplyDemand;

	// Whether this item's price is affected by market events
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pricing")
	bool bAffectedByMarketEvents;

	// ====================
	// LEGALITY & RESTRICTIONS
	// ====================

	// Legal status of this item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality")
	ELegalityStatus LegalityStatus;

	// REMOVED: TradeRestrictions - simplified for MVP (struct now empty)
	// All items accessible to all players in MVP

	// Contraband fine multiplier if caught (1.0 = base price)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality", meta=(ClampMin="0.0"))
	float ContrabandFineMultiplier;

	// ====================
	// TRADE PROPERTIES
	// ====================

	// Volume per unit (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties", meta=(ClampMin="0.0"))
	float VolumePerUnit;

	// Mass per unit (kilograms)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties", meta=(ClampMin="0.0"))
	float MassPerUnit;

	// Typical lot size for bulk trading
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties", meta=(ClampMin="1"))
	int32 StandardLotSize;

	// Maximum quantity typically available in markets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties", meta=(ClampMin="0"))
	int32 TypicalMarketStock;

	// How quickly stock replenishes (units per game day)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade Properties", meta=(ClampMin="0"))
	int32 ReplenishmentRate;

	// ====================
	// AI TRADER BEHAVIOR
	// ====================

	// Priority for AI traders (1-10, higher = more desirable)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior", meta=(ClampMin="1", ClampMax="10"))
	int32 AITradePriority;

	// Whether AI traders will hoard this during shortages
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
	bool bAIHoardable;

	// Whether AI traders will use this for arbitrage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
	bool bAIArbitrageEnabled;

	// Tags for AI behavior and categorization
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behavior")
	TArray<FName> BehaviorTags;

	// ====================
	// Constructor
	// ====================

	UTradeItemDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Check if this item has a specific behavior tag
	 * @param Tag The tag to check for
	 * @return True if item has the tag
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Tags")
	bool HasBehaviorTag(FName Tag) const;

	/**
	 * Get total volume for a quantity
	 * @param Quantity Number of units
	 * @return Total volume in cubic meters
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
	float GetTotalVolume(int32 Quantity) const;

	/**
	 * Get total mass for a quantity
	 * @param Quantity Number of units
	 * @return Total mass in kilograms
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
	float GetTotalMass(int32 Quantity) const;

	/**
	 * BlueprintNativeEvent: Custom price calculation override
	 * Designers can implement this in Blueprint to add custom pricing logic
	 * @param Supply Current supply level
	 * @param Demand Current demand level
	 * @param EventMultiplier Market event multiplier
	 * @param BaseCalculatedPrice The price calculated by standard formula
	 * @return Final price after custom modifications
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Trade Item|Events")
	float OnCalculateCustomPrice(float Supply, float Demand, float EventMultiplier, float BaseCalculatedPrice) const;

	/**
	 * BlueprintNativeEvent: Called when this item is traded
	 * Allows designers to trigger custom events on trade
	 * @param Quantity Amount traded
	 * @param Price Price per unit
	 * @param BuyerFactionID Buyer's faction
	 * @param SellerFactionID Seller's faction
	 */
	// REMOVED: OnItemTraded event - faction tracking removed per Trade Simulator MVP
	// Can be re-added post-MVP if needed for analytics

#if WITH_EDITOR
	/**
	 * Validate trade item data asset properties
	 * Checks for required fields and logical constraints
	 */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
