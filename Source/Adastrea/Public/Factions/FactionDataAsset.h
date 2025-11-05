#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactionDataAsset.generated.h"

/**
 * Category for organizing faction traits
 */
UENUM(BlueprintType)
enum class EFactionTraitCategory : uint8
{
    Military    UMETA(DisplayName = "Military"),    // Combat and defense bonuses
    Economic    UMETA(DisplayName = "Economic"),    // Trade and resource bonuses
    Scientific  UMETA(DisplayName = "Scientific"),  // Research and technology bonuses
    Diplomatic  UMETA(DisplayName = "Diplomatic"),  // Relationship modifiers
    Special     UMETA(DisplayName = "Special")      // Unique faction abilities
};

/**
 * Trait structure for faction gameplay modifiers
 * Traits define unique characteristics that affect faction behavior and interactions
 */
USTRUCT(BlueprintType)
struct FFactionTrait
{
    GENERATED_BODY()

    // Display name of the trait
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    FText TraitName;

    // Description of what this trait does
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait", meta=(MultiLine=true))
    FText TraitDescription;

    // Unique identifier for the trait
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    FName TraitID;

    // Category for organizing and filtering traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    EFactionTraitCategory Category;

    // Modifier value for gameplay effects (can be positive or negative)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    float ModifierValue;

    FFactionTrait()
        : TraitName(FText::FromString(TEXT("Unknown Trait")))
        , TraitDescription(FText::FromString(TEXT("No description available.")))
        , TraitID(NAME_None)
        , Category(EFactionTraitCategory::Special)
        , ModifierValue(0.0f)
    {}
};

/**
 * Diplomacy relationship structure
 * Defines the relationship between two factions
 */
USTRUCT(BlueprintType)
struct FFactionRelationship
{
    GENERATED_BODY()

    // The faction this relationship is with
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Diplomacy")
    FName TargetFactionID;

    // Relationship status (-100 to 100, negative is hostile, positive is friendly)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Diplomacy", meta=(ClampMin="-100", ClampMax="100"))
    int32 RelationshipValue;

    // Whether there is an active alliance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Diplomacy")
    bool bIsAllied;

    // Whether there is an active war
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Diplomacy")
    bool bAtWar;

    // Trade agreement multiplier (1.0 = normal, higher = better trade)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Diplomacy", meta=(ClampMin="0.0", ClampMax="2.0"))
    float TradeModifier;

    FFactionRelationship()
        : TargetFactionID(NAME_None)
        , RelationshipValue(0)
        , bIsAllied(false)
        , bAtWar(false)
        , TradeModifier(1.0f)
    {}
};

/**
 * Data Asset for storing faction information.
 * This allows designers to create faction configurations as Blueprint Data Assets.
 */
UCLASS(BlueprintType)
class ADASTREA_API UFactionDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Display name of the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FText FactionName;

    // Brief description of the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info", meta=(MultiLine=true))
    FText Description;

    // Faction emblem/logo color
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FLinearColor PrimaryColor;

    // Secondary faction color
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FLinearColor SecondaryColor;

    // Faction's reputation with the player (-100 to 100)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Relations", meta=(ClampMin="-100", ClampMax="100"))
    int32 InitialReputation;

    // Whether this faction is hostile by default
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Relations")
    bool bHostileByDefault;

    // Technology level of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 TechnologyLevel;

    // Military strength of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 MilitaryStrength;

    // Economic power of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 EconomicPower;

    // Unique identifier for the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FName FactionID;

    // Faction traits that provide unique bonuses or penalties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Traits")
    TArray<FFactionTrait> Traits;

    // Relationships with other factions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Diplomacy")
    TArray<FFactionRelationship> FactionRelationships;

    UFactionDataAsset();

    // ====================
    // Trait System Hooks
    // ====================

    /**
     * Get all traits for this faction
     * @return Array of faction traits
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    TArray<FFactionTrait> GetTraits() const;

    /**
     * Check if faction has a specific trait
     * @param TraitID The trait identifier to check for
     * @return True if faction has the trait
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    bool HasTrait(FName TraitID) const;

    /**
     * Get a specific trait by ID
     * @param TraitID The trait identifier to retrieve
     * @param OutTrait The found trait (if any)
     * @return True if trait was found
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    bool GetTraitByID(FName TraitID, FFactionTrait& OutTrait) const;

    /**
     * Get the combined modifier value for a specific trait type
     * Useful for stacking multiple traits that affect the same attribute
     * @param TraitID The trait identifier to get modifiers for
     * @return Total modifier value from all matching traits
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    float GetTraitModifier(FName TraitID) const;

    /**
     * Get all traits in a specific category
     * @param Category The category to filter by
     * @return Array of traits in the specified category
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    TArray<FFactionTrait> GetTraitsByCategory(EFactionTraitCategory Category) const;

    /**
     * Check if faction has any traits in a specific category
     * @param Category The category to check
     * @return True if faction has at least one trait in the category
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    bool HasTraitInCategory(EFactionTraitCategory Category) const;

    /**
     * Get the total modifier value for all traits in a category
     * @param Category The category to sum modifiers for
     * @return Combined modifier value from all traits in the category
     */
    UFUNCTION(BlueprintCallable, Category="Faction Traits")
    float GetCategoryModifierTotal(EFactionTraitCategory Category) const;

    // ====================
    // Diplomacy System Hooks
    // ====================

    /**
     * Get relationship with another faction
     * @param OtherFactionID The faction to get relationship with
     * @param OutRelationship The relationship data (if found)
     * @return True if relationship exists
     */
    UFUNCTION(BlueprintCallable, Category="Faction Diplomacy")
    bool GetRelationship(FName OtherFactionID, FFactionRelationship& OutRelationship) const;

    /**
     * Check if this faction is allied with another
     * @param OtherFactionID The faction to check alliance with
     * @return True if allied
     */
    UFUNCTION(BlueprintCallable, Category="Faction Diplomacy")
    bool IsAlliedWith(FName OtherFactionID) const;

    /**
     * Check if this faction is at war with another
     * @param OtherFactionID The faction to check war status with
     * @return True if at war
     */
    UFUNCTION(BlueprintCallable, Category="Faction Diplomacy")
    bool IsAtWarWith(FName OtherFactionID) const;

    /**
     * Get the relationship value with another faction
     * @param OtherFactionID The faction to get relationship value with
     * @return Relationship value (-100 to 100), returns 0 if no relationship exists
     */
    UFUNCTION(BlueprintCallable, Category="Faction Diplomacy")
    int32 GetRelationshipValue(FName OtherFactionID) const;

    /**
     * Get trade modifier with another faction
     * @param OtherFactionID The faction to get trade modifier with
     * @return Trade modifier (1.0 = normal), returns 1.0 if no relationship exists
     */
    UFUNCTION(BlueprintCallable, Category="Faction Diplomacy")
    float GetTradeModifier(FName OtherFactionID) const;

private:
    // Performance optimization: cached relationship map for O(1) lookups
    // Built lazily on first access
    mutable TMap<FName, const FFactionRelationship*> RelationshipCache;
    mutable bool bRelationshipCacheValid;
    
    // Rebuild the relationship cache from the array
    void RebuildRelationshipCache() const;
};
