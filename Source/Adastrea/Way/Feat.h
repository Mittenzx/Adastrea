#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Way.h"
#include "Feat.generated.h"

/**
 * Structure defining which Precepts are embodied by a Feat.
 * When a player accomplishes a Feat, groups that value these Precepts will react.
 */
USTRUCT(BlueprintType)
struct FFeatPreceptAlignment
{
    GENERATED_BODY()

    /** The Precept this Feat embodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept")
    EPrecept Precept;

    /** How strongly this Feat represents the Precept (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept", meta=(ClampMin="0", ClampMax="100"))
    int32 AlignmentStrength;

    /** Optional note about why this Feat aligns with the Precept */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept", meta=(MultiLine=true))
    FText ReasonDescription;

    FFeatPreceptAlignment()
        : Precept(EPrecept::Honor)
        , AlignmentStrength(50)
        , ReasonDescription(FText::FromString(TEXT("This feat demonstrates this precept.")))
    {}
};

/**
 * Rarity tier for Feats, determining how legendary the accomplishment is.
 * Rarer Feats grant more prestigious Titles and have greater impact on reputation.
 */
UENUM(BlueprintType)
enum class EFeatRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),      // Basic accomplishments most players will achieve
    Uncommon    UMETA(DisplayName = "Uncommon"),    // Notable achievements requiring some effort
    Rare        UMETA(DisplayName = "Rare"),        // Impressive accomplishments requiring skill
    Epic        UMETA(DisplayName = "Epic"),        // Exceptional deeds few players achieve
    Legendary   UMETA(DisplayName = "Legendary"),   // World-renowned accomplishments of legend
    Mythic      UMETA(DisplayName = "Mythic")       // Once-in-a-generation legendary feats
};

/**
 * Data Asset representing a Feat - a major, logged accomplishment.
 * Feats grant the player Titles that become part of their legend (Verse).
 * 
 * When a player completes a Feat:
 * 1. They earn the associated Title
 * 2. The accomplishment is recorded in their Verse (UVerseComponent)
 * 3. Groups (Ways) react based on alignment with their Precepts
 * 4. The player's reputation with groups is affected
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on UFeatDataAsset
 * - Configure what the Feat represents (Title, Precepts, Requirements)
 * - Reference in gameplay systems (quests, achievements, boss defeats)
 * - Award to player when they complete the requirements
 * 
 * Example Feats:
 * - "Star-Charter": First to fully map an uncharted sector (Discovery + Innovation)
 * - "Dragon-Slayer": Defeated a capital ship single-handedly (Strength + Honor)
 * - "Trade-Prince": Established profitable routes with 10+ stations (Prosperity + Cunning)
 * - "Peace-Bringer": Negotiated end to faction war (Justice + Compassion)
 */
UCLASS(BlueprintType)
class ADASTREA_API UFeatDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UFeatDataAsset();

    // ====================
    // Core Identity
    // ====================

    /** The Title granted by this Feat (e.g., "Star-Charter", "Dragon-Slayer") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText TitleName;

    /** Description of what was accomplished to earn this Feat */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
    FText Description;

    /** Unique identifier for this Feat */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName FeatID;

    /** How rare and prestigious this accomplishment is */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    EFeatRarity Rarity;

    /** Optional flavor text describing the legend of this Feat */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
    FText LegendaryDescription;

    // ====================
    // Precept Alignment
    // ====================

    /** Which Precepts this Feat embodies (determines group reactions) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Philosophy")
    TArray<FFeatPreceptAlignment> PreceptAlignments;

    // ====================
    // Gameplay Requirements
    // ====================

    /** Optional: Prerequisites that must be met before this Feat can be earned */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
    TArray<UFeatDataAsset*> PrerequisiteFeats;

    /** Whether this Feat can only be earned once per playthrough */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
    bool bUniquePerPlaythrough;

    /** Whether this Feat is hidden until earned (for secret achievements) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Requirements")
    bool bHiddenUntilEarned;

    // ====================
    // Reputation Impact
    // ====================

    /** Base reputation gain with groups that value aligned Precepts */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reputation", meta=(ClampMin="0", ClampMax="100"))
    int32 BaseReputationGain;

    /** Reputation gain is multiplied by (PreceptValue * AlignmentStrength / 10000) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reputation")
    float ReputationMultiplier;

    // ====================
    // Precept Query Functions
    // ====================

    /**
     * Get all Precept alignments for this Feat
     * @return Array of Precept alignments
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Precepts")
    TArray<FFeatPreceptAlignment> GetPreceptAlignments() const;

    /**
     * Check if this Feat aligns with a specific Precept
     * @param Precept The Precept to check for
     * @return True if this Feat has alignment with the Precept
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Precepts")
    bool AlignsWith(EPrecept Precept) const;

    /**
     * Get the alignment strength for a specific Precept
     * @param Precept The Precept to query
     * @return Alignment strength (0-100), returns 0 if no alignment
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Precepts")
    int32 GetAlignmentStrength(EPrecept Precept) const;

    /**
     * Get the strongest Precept alignment for this Feat
     * @param OutPrecept The most strongly aligned Precept
     * @return True if Feat has any alignments defined
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Precepts")
    bool GetPrimaryAlignment(EPrecept& OutPrecept) const;

    /**
     * Calculate reputation gain with a group based on their Precept values
     * @param WayPrecepts The Precepts valued by a group
     * @return Calculated reputation gain amount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Reputation")
    int32 CalculateReputationGain(const TArray<FPreceptValue>& WayPrecepts) const;

    // ====================
    // Requirements Functions
    // ====================

    /**
     * Check if player can earn this Feat (prerequisites met)
     * @param EarnedFeats List of Feats the player has already earned
     * @return True if all prerequisites are satisfied
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Requirements")
    bool CanBeEarned(const TArray<UFeatDataAsset*>& EarnedFeats) const;

    /**
     * Get list of prerequisite Feats
     * @return Array of required Feats
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Requirements")
    TArray<UFeatDataAsset*> GetPrerequisites() const;

    // ====================
    // Helper Functions
    // ====================

    /**
     * Get display name for a Feat rarity
     * @param Rarity The rarity to get name for
     * @return Localized display name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Utility")
    static FText GetRarityDisplayName(EFeatRarity Rarity);

    /**
     * Get color associated with a Feat rarity (for UI)
     * @param Rarity The rarity to get color for
     * @return Color representing the rarity tier
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Feat|Utility")
    static FLinearColor GetRarityColor(EFeatRarity Rarity);
};
