#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Way/Way.h"
#include "FactionDataAsset.generated.h"

// Forward declarations
class UWayDataAsset;
struct FFeatPreceptAlignment;

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

    // ====================
    // Way System Integration
    // ====================

    /**
     * Ways (guilds/schools/syndicates) associated with this faction
     * These are smaller organizations that operate within or alongside the faction
     * Player reputation with Ways can influence faction standing
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways")
    TArray<UWayDataAsset*> AssociatedWays;

    /**
     * Reputation spillover percentage from associated Ways to faction (0-100)
     * When player gains reputation with a Way, this % applies to the faction
     * Example: 25 = gaining 100 rep with a Way grants 25 rep to the faction
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways", meta=(ClampMin="0", ClampMax="100"))
    int32 WayReputationSpillover;

    /**
     * Reverse spillover: faction reputation affects associated Ways (0-100)
     * When player gains faction reputation, this % flows to associated Ways
     * Example: 15 = gaining 100 faction rep grants 15 rep to each associated Way
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways", meta=(ClampMin="0", ClampMax="100"))
    int32 FactionToWaySpillover;

    /**
     * Whether this faction's diplomacy is influenced by Way Networks
     * If true, the faction considers network memberships in diplomatic decisions
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways")
    bool bNetworkInfluencesDiplomacy;

    /**
     * Weight of Way reputation in overall faction standing (0.0-1.0)
     * 0.0 = Way reputation doesn't affect faction standing
     * 0.5 = Way reputation is 50% of faction standing calculation
     * 1.0 = Way reputation completely determines faction standing
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways", meta=(ClampMin="0.0", ClampMax="1.0"))
    float WayReputationWeight;

    /**
     * Precepts that define this faction's philosophical alignment
     * Derived from or aligned with associated Ways' Precepts
     * Used for determining faction reactions to player Feats
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Philosophy")
    TArray<FPreceptValue> FactionPrecepts;

    /**
     * Whether faction traits are automatically derived from Way Precepts
     * If true, faction gains traits based on its Ways' primary Precepts
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction|Ways")
    bool bDeriveTraitsFromWays;

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

    // ====================
    // Way System Hooks
    // ====================

    /**
     * Get all Ways associated with this faction
     * @return Array of associated Way data assets
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    TArray<UWayDataAsset*> GetAssociatedWays() const;

    /**
     * Check if a specific Way is associated with this faction
     * @param Way The Way to check
     * @return True if the Way is associated with this faction
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    bool HasAssociatedWay(const UWayDataAsset* Way) const;

    /**
     * Calculate faction reputation gain from a Way reputation gain
     * @param WayReputationGain Reputation gained with a Way
     * @return Spillover reputation for the faction
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    int32 CalculateWayReputationSpillover(int32 WayReputationGain) const;

    /**
     * Calculate Way reputation gain from faction reputation gain (reverse spillover)
     * @param FactionReputationGain Reputation gained with the faction
     * @return Spillover reputation for each associated Way
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    int32 CalculateFactionToWaySpillover(int32 FactionReputationGain) const;

    /**
     * Get aggregated reputation score across all associated Ways
     * @param WayReputationScores Map of Way IDs to their reputation scores
     * @return Weighted aggregate reputation from all Ways
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    float CalculateAggregateWayReputation(const TMap<FName, float>& WayReputationScores) const;

    /**
     * Calculate combined faction standing including Way influence
     * @param BaseReputation Base faction reputation
     * @param WayAggregate Aggregate reputation from Ways
     * @return Combined reputation score with Way weighting applied
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    float CalculateCombinedStanding(int32 BaseReputation, float WayAggregate) const;

    /**
     * Get all Precepts valued by this faction
     * @return Array of faction Precepts
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Philosophy")
    TArray<FPreceptValue> GetFactionPrecepts() const;

    /**
     * Check if faction values a specific Precept
     * @param Precept The Precept to check
     * @return True if faction has this Precept
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Philosophy")
    bool HasFactionPrecept(EPrecept Precept) const;

    /**
     * Get the importance value for a specific Precept
     * @param Precept The Precept to query
     * @return Importance value (0-100), returns 0 if not valued
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Philosophy")
    int32 GetPreceptImportance(EPrecept Precept) const;

    /**
     * Calculate philosophical alignment with another faction based on shared Precepts
     * @param OtherFaction The faction to check alignment with
     * @return Alignment score (0.0-1.0), higher means more aligned
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Philosophy")
    float CalculatePhilosophicalAlignment(const UFactionDataAsset* OtherFaction) const;

    /**
     * Get traits derived from associated Ways' Precepts
     * Automatically generates traits based on Way philosophies
     * @return Array of dynamically generated traits
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    TArray<FFactionTrait> GetDerivedTraitsFromWays() const;

    /**
     * Get all Ways that share a specific Precept with this faction
     * @param Precept The Precept to match
     * @param MinimumImportance Minimum importance value to consider (default 50)
     * @return Array of Ways that strongly value this Precept
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Ways")
    TArray<UWayDataAsset*> GetWaysByPrecept(EPrecept Precept, int32 MinimumImportance = 50) const;

    /**
     * Calculate reputation modifier for a Feat based on Precept alignment
     * @param FeatPrecepts The Precepts embodied by the Feat
     * @return Reputation modifier multiplier (0.0-3.0)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Faction|Philosophy")
    float CalculateFeatReputationModifier(const TArray<FFeatPreceptAlignment>& FeatPrecepts) const;

private:
    // Performance optimization: cached relationship map for O(1) lookups
    // Built lazily on first access
    mutable TMap<FName, const FFactionRelationship*> RelationshipCache;
    mutable bool bRelationshipCacheValid;
    
    // Rebuild the relationship cache from the array
    void RebuildRelationshipCache() const;
};
