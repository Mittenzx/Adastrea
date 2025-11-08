#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VerseSubsystem.generated.h"

// Forward declarations
class UFeatDataAsset;
class UWayDataAsset;
enum class EReputationLevel : uint8;

/**
 * @class UVerseSubsystem
 * @brief Manages the player's "Verse" (業) - their collection of completed Feats and Titles.
 *
 * This central subsystem is the brain of the "Way of the Star-Seeker" reputation system.
 * It tracks the player's accomplishments and calculates their standing with all Ways in the universe
 * based on how the Precepts of their Feats align with the Precepts of the Ways.
 */
UCLASS()
class ADASTREA_API UVerseSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem

    /**
     * Awards a Feat to the player, adding it to their permanent record.
     * This is the primary method for progressing the player's legend.
     * @param FeatToRecord The Feat that the player has accomplished.
     */
    UFUNCTION(BlueprintCallable, Category = "Verse")
    void RecordFeat(const UFeatDataAsset* FeatToRecord);

    /**
     * Calculates the player's current Verse (reputation score) with a specific Way.
     * The calculation is based on the alignment of Precepts between the player's completed Feats and the Way's philosophy.
     * @param TargetWay The Way to calculate the score for.
     * @return A numerical score representing the player's standing.
     */
    UFUNCTION(BlueprintPure, Category = "Verse")
    float GetVerseScore(const UWayDataAsset* TargetWay) const;

    /**
     * Gets the player's qualitative reputation level with a specific Way.
     * @param TargetWay The Way to check the reputation level for.
     * @return The corresponding EReputationLevel (e.g., Trusted, Neutral, Distrusted).
     */
    UFUNCTION(BlueprintPure, Category = "Verse")
    EReputationLevel GetReputationLevel(const UWayDataAsset* TargetWay) const;

    /**
     * Checks if the player has completed a specific Feat.
     * @param FeatToCheck The Feat to check for.
     * @return True if the player has completed the Feat, false otherwise.
     */
    UFUNCTION(BlueprintPure, Category = "Verse")
    bool HasCompletedFeat(const UFeatDataAsset* FeatToCheck) const;

private:
    /** The set of all Feats the player has successfully completed. Should be loaded from save data. */
    UPROPERTY(VisibleAnywhere, Category = "Verse State")
    TSet<TObjectPtr<const UFeatDataAsset>> CompletedFeats;

    /**
     * Placeholder function to load the player's saved Feats.
     * In a real implementation, this would read from a UAdastreaSaveGame object.
     */
    void LoadVerseState();
};
