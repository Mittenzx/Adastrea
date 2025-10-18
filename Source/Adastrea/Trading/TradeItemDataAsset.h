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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pricing", meta=(ClampMin="0.0", ClampMax="5.0"))
	float VolatilityMultiplier;

	// Minimum price deviation as percentage (e.g., 0.5 = 50% below base)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pricing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinPriceDeviation;

	// Maximum price deviation as percentage (e.g., 2.0 = 200% above base)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pricing", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MaxPriceDeviation;

	FPriceVolatility()
		: VolatilityMultiplier(1.0f)
		, MinPriceDeviation(0.5f)
		, MaxPriceDeviation(2.0f)
	{}
};

/**
 * Trade restrictions based on faction relationships
 */
USTRUCT(BlueprintType)
struct FTradeRestrictions
{
	GENERATED_BODY()

	// Factions that ban this item completely
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Restrictions")
	TArray<FName> BannedFactions;

	// Factions that require special permits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Restrictions")
	TArray<FName> RequiresPermitFactions;

	// Minimum faction reputation required to trade (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Restrictions", meta=(ClampMin="-100", ClampMax="100"))
	int32 MinReputationRequired;

	FTradeRestrictions()
		: MinReputationRequired(-100)
	{}
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

	// Trade restrictions based on factions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Legality")
	FTradeRestrictions TradeRestrictions;

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
	 * Calculate current price based on market conditions
	 * @param Supply Current supply level (0.0 to 1.0+, where 1.0 is typical)
	 * @param Demand Current demand level (0.0 to 1.0+, where 1.0 is typical)
	 * @param MarketEventMultiplier Additional price multiplier from events (1.0 = no effect)
	 * @return Calculated price in credits
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Pricing")
	float CalculatePrice(float Supply, float Demand, float MarketEventMultiplier = 1.0f) const;

	/**
	 * Get price after applying faction relationships
	 * @param BasePriceToModify The price to modify
	 * @param BuyerFactionID Faction ID of the buyer
	 * @param SellerFactionID Faction ID of the seller
	 * @return Modified price
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Pricing")
	float GetFactionModifiedPrice(float BasePriceToModify, FName BuyerFactionID, FName SellerFactionID) const;

	/**
	 * Check if item can be traded by a specific faction
	 * @param FactionID The faction attempting to trade
	 * @param Reputation Faction reputation with trading authority (-100 to 100)
	 * @return True if trade is allowed
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
	bool CanBeTradedByFaction(FName FactionID, int32 Reputation) const;

	/**
	 * Check if item requires a permit for trading
	 * @param FactionID The faction to check
	 * @return True if permit is required
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
	bool RequiresPermit(FName FactionID) const;

	/**
	 * Calculate fine for illegal trade if caught
	 * @param Quantity Number of units being traded
	 * @return Fine amount in credits
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Legality")
	float CalculateContrabandFine(int32 Quantity) const;

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
	 * Check if this item is considered high-value
	 * @return True if base price is above luxury threshold
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Item|Properties")
	bool IsHighValue() const;

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
	UFUNCTION(BlueprintNativeEvent, Category="Trade Item|Events")
	void OnItemTraded(int32 Quantity, float Price, FName BuyerFactionID, FName SellerFactionID);
};
