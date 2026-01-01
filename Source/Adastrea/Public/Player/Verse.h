#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Verse.generated.h"

// ====================
// Stub Type Definitions
// ====================
// NOTE: These are minimal stub definitions to allow Verse component to compile.
// The full Way system (Feats, Ways, Precepts) has been archived and will be
// reimplemented in the MVP phase. These stubs provide just enough functionality
// to support the existing Verse.cpp implementation without breaking the build.

/**
 * Precepts represent core values and philosophies.
 * Stub enum for MVP - full implementation in Archive/Source/Way/Way.h
 */
UENUM(BlueprintType)
enum class EPrecept : uint8
{
    Honor           UMETA(DisplayName = "Honor"),
    Justice         UMETA(DisplayName = "Justice"),
    Compassion      UMETA(DisplayName = "Compassion"),
    Loyalty         UMETA(DisplayName = "Loyalty"),
    Mastery         UMETA(DisplayName = "Mastery"),
    Innovation      UMETA(DisplayName = "Innovation"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Craftsmanship   UMETA(DisplayName = "Craftsmanship"),
    Strength        UMETA(DisplayName = "Strength"),
    Dominance       UMETA(DisplayName = "Dominance"),
    Cunning         UMETA(DisplayName = "Cunning"),
    Ambition        UMETA(DisplayName = "Ambition"),
    Unity           UMETA(DisplayName = "Unity"),
    Freedom         UMETA(DisplayName = "Freedom"),
    Tradition       UMETA(DisplayName = "Tradition"),
    Progress        UMETA(DisplayName = "Progress"),
    Prosperity      UMETA(DisplayName = "Prosperity"),
    Survival        UMETA(DisplayName = "Survival"),
    Efficiency      UMETA(DisplayName = "Efficiency"),
    Harmony         UMETA(DisplayName = "Harmony")
};

/**
 * Rarity tier for Feats.
 * Stub enum for MVP - full implementation in Archive/Source/Way/Feat.h
 */
UENUM(BlueprintType)
enum class EFeatRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
    Mythic      UMETA(DisplayName = "Mythic")
};

/**
 * Structure defining how strongly a group values a Precept.
 * Stub struct for MVP - full implementation in Archive/Source/Way/Way.h
 */
USTRUCT(BlueprintType)
struct FPreceptValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept")
    EPrecept Precept;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept", meta=(ClampMin="0", ClampMax="100"))
    int32 ImportanceValue;

    FPreceptValue()
        : Precept(EPrecept::Honor)
        , ImportanceValue(50)
    {}
};

/**
 * Data Asset representing a Feat - a major accomplishment.
 * Stub class for MVP - full implementation in Archive/Source/Way/Feat.h
 */
UCLASS(BlueprintType)
class ADASTREA_API UFeatDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** The Title granted by this Feat */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText TitleName;

    /** Unique identifier for this Feat */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName FeatID;

    /** Whether this Feat can only be earned once per playthrough */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
    bool bUniquePerPlaythrough;

    /** How rare and prestigious this accomplishment is */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    EFeatRarity Rarity;

    /**
     * Get alignment strength for a specific Precept
     * @param Precept The Precept to query
     * @return Alignment strength (stub returns 0)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat")
    int32 GetAlignmentStrength(EPrecept Precept) const { return 0; }

    /**
     * Check if this Feat aligns with a specific Precept
     * @param Precept The Precept to check
     * @return True if aligned (stub returns false)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat")
    bool AlignsWith(EPrecept Precept) const { return false; }

    /**
     * Calculate reputation gain with a group
     * @param WayPrecepts The group's valued Precepts
     * @return Reputation gain (stub returns 0)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat")
    int32 CalculateReputationGain(const TArray<FPreceptValue>& WayPrecepts) const { return 0; }
};

/**
 * Data Asset representing a Way - a group's identity and philosophy.
 * Stub class for MVP - full implementation in Archive/Source/Way/Way.h
 */
UCLASS(BlueprintType)
class ADASTREA_API UWayDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * Get all Precepts valued by this Way
     * @return Array of Precept values (stub returns empty array)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way")
    TArray<FPreceptValue> GetPrecepts() const { return TArray<FPreceptValue>(); }
};

/**
 * Structure tracking when and where a Feat was earned.
 * This creates a chronological record of the player's legendary journey.
 */
USTRUCT(BlueprintType)
struct FEarnedFeat
{
    GENERATED_BODY()

    /** The Feat that was accomplished */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feat")
    UFeatDataAsset* Feat;

    /** When this Feat was earned (in-game timestamp) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feat")
    FDateTime EarnedTimestamp;

    /** Optional: Location where the Feat was earned (sector name, station name, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feat")
    FText LocationEarned;

    /** Optional: Additional context about how the Feat was earned */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feat", meta=(MultiLine=true))
    FText EarnedContext;

    FEarnedFeat()
        : Feat(nullptr)
        , EarnedTimestamp(FDateTime::MinValue())
        , LocationEarned(FText::GetEmpty())
        , EarnedContext(FText::GetEmpty())
    {}
};

/**
 * The Verse Component - Player's legendary record of earned Titles.
 * 
 * This component is attached to the player to track all Feats they have accomplished
 * throughout their journey. The "Verse" is the player's living legend - a chronological
 * record of their greatest achievements that defines who they are becoming.
 * 
 * The Verse influences:
 * - How groups (Ways) react to the player based on shared values
 * - Reputation gains with Schools and Syndicates
 * - Available dialogue options and quest paths
 * - NPC attitudes and behaviors toward the player
 * 
 * Unlike traditional faction reputation, the Verse is permanent and grows over time.
 * Each earned Title adds to the player's legend and shapes their identity in the world.
 * 
 * Usage:
 * - Attach to player pawn or player controller
 * - Award Feats when player completes major accomplishments
 * - Query Verse to check player's Titles and values
 * - Use in dialogue, quest, and AI systems to create dynamic reactions
 * 
 * Example:
 * - Player defeats legendary pirate captain → Earns "Pirate-Hunter" Title
 * - Military Schools (value Strength + Justice) react positively
 * - Pirate Syndicates (value Freedom + Cunning) react negatively
 * - The Title becomes part of player's permanent legend
 */
UCLASS(BlueprintType, ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UVerseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVerseComponent();

    // ====================
    // Core Properties
    // ====================

    /** All Feats earned by this player, in chronological order */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Verse")
    TArray<FEarnedFeat> EarnedFeats;

    /** Optional: The Title the player is currently using for display (must be earned) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Verse")
    UFeatDataAsset* DisplayedTitle;

    // ====================
    // Feat Management
    // ====================

    /**
     * Award a Feat to the player, adding it to their Verse
     * @param Feat The Feat to award
     * @param Location Optional location context
     * @param Context Optional additional context
     * @return True if Feat was awarded (false if already earned and unique)
     */
    UFUNCTION(BlueprintCallable, Category="Verse")
    bool AwardFeat(UFeatDataAsset* Feat, FText Location = FText::GetEmpty(), FText Context = FText::GetEmpty());

    /**
     * Check if player has earned a specific Feat
     * @param Feat The Feat to check for
     * @return True if player has earned this Feat
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    bool HasFeat(UFeatDataAsset* Feat) const;

    /**
     * Check if player has earned a Feat by ID
     * @param FeatID The unique identifier of the Feat
     * @return True if player has earned a Feat with this ID
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    bool HasFeatByID(FName FeatID) const;

    /**
     * Get all earned Feats
     * @return Array of all earned Feats with timestamps
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    TArray<FEarnedFeat> GetAllEarnedFeats() const;

    /**
     * Get earned Feat by index (chronological order)
     * @param Index The index of the Feat to retrieve
     * @param OutFeat The earned Feat data
     * @return True if index is valid
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    bool GetEarnedFeatByIndex(int32 Index, FEarnedFeat& OutFeat) const;

    /**
     * Get total count of earned Feats
     * @return Number of Feats in player's Verse
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    int32 GetEarnedFeatCount() const;

    // ====================
    // Display Title Management
    // ====================

    /**
     * Set which Title the player is currently displaying
     * @param Feat The Feat/Title to display (must be earned)
     * @return True if Title was set (false if not earned)
     */
    UFUNCTION(BlueprintCallable, Category="Verse")
    bool SetDisplayedTitle(UFeatDataAsset* Feat);

    /**
     * Get the currently displayed Title
     * @return The Feat/Title being displayed, or nullptr if none set
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse")
    UFeatDataAsset* GetDisplayedTitle() const;

    /**
     * Clear the displayed Title (use default player name)
     * @return True if cleared
     */
    UFUNCTION(BlueprintCallable, Category="Verse")
    bool ClearDisplayedTitle();

    // ====================
    // Precept Analysis
    // ====================

    /**
     * Calculate player's total alignment with a specific Precept based on their Verse
     * This determines how strongly the player embodies a particular value
     * @param Precept The Precept to calculate alignment for
     * @return Total alignment strength (sum of all aligned Feats)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Analysis")
    int32 GetTotalPreceptAlignment(EPrecept Precept) const;

    /**
     * Get player's top 3 most aligned Precepts
     * Reveals what values the player's legend most strongly represents
     * @param OutTopPrecepts Array to fill with top Precepts (up to 3)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Analysis")
    void GetTopAlignedPrecepts(TArray<EPrecept>& OutTopPrecepts) const;

    /**
     * Calculate compatibility score with a group's Precepts
     * Higher score means player's legend aligns well with group's values
     * @param WayPrecepts The Precepts valued by a group
     * @return Compatibility score (0-100+)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Analysis")
    float CalculateWayCompatibility(const TArray<struct FPreceptValue>& WayPrecepts) const;

    /**
     * Get all Feats that align with a specific Precept
     * @param Precept The Precept to filter by
     * @return Array of earned Feats that embody this Precept
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Analysis")
    TArray<FEarnedFeat> GetFeatsAlignedWith(EPrecept Precept) const;

    // ====================
    // Reputation Integration
    // ====================

    /**
     * Calculate total reputation gain from Verse for a specific group
     * @param GroupWay The group's Way (Precepts and values)
     * @return Total reputation points from all earned Feats
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Reputation")
    int32 CalculateVerseReputation(UWayDataAsset* GroupWay) const;

    // ====================
    // Statistics and Queries
    // ====================

    /**
     * Get count of Feats by rarity tier
     * @param Rarity The rarity tier to count
     * @return Number of earned Feats of this rarity
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Statistics")
    int32 GetFeatCountByRarity(EFeatRarity Rarity) const;

    /**
     * Get the most recently earned Feat
     * @param OutFeat The most recent earned Feat
     * @return True if player has earned any Feats
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Statistics")
    bool GetMostRecentFeat(FEarnedFeat& OutFeat) const;

    /**
     * Get Feats earned in a specific time period
     * @param StartTime Start of time period
     * @param EndTime End of time period
     * @return Array of Feats earned during this period
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Verse|Statistics")
    TArray<FEarnedFeat> GetFeatsInTimeRange(FDateTime StartTime, FDateTime EndTime) const;

    // ====================
    // Serialization Support
    // ====================

    /**
     * Export Verse to a save-friendly format
     * Used for save/load systems
     * @return Array of Feat IDs for persistence
     */
    UFUNCTION(BlueprintCallable, Category="Verse|Persistence")
    TArray<FName> ExportFeatIDs() const;

    /**
     * Import Verse from saved data
     * Used for save/load systems
     * @param FeatIDs Array of Feat IDs to load
     * @param AllFeats Array of all available Feat assets to resolve IDs
     * @return True if import was successful
     */
    UFUNCTION(BlueprintCallable, Category="Verse|Persistence")
    bool ImportFeatIDs(const TArray<FName>& FeatIDs, const TArray<UFeatDataAsset*>& AllFeats);

protected:
    /**
     * Find an earned Feat entry by asset reference
     * @param Feat The Feat to find
     * @return Pointer to the earned Feat entry, or nullptr if not found
     */
    const FEarnedFeat* FindEarnedFeat(UFeatDataAsset* Feat) const;
};
