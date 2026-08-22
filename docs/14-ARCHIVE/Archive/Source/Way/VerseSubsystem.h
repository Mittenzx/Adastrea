#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VerseSubsystem.generated.h"

// Forward declarations
class UFeatDataAsset;
class UWayDataAsset;
class UWayNetworkDataAsset;
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

    // ====================
    // Network Functions
    // ====================

    /**
     * Register a Way Network for reputation tracking.
     * Networks allow micro-alliances between Ways with shared Precepts.
     * @param Network The network to register
     */
    UFUNCTION(BlueprintCallable, Category = "Verse|Networks")
    void RegisterNetwork(UWayNetworkDataAsset* Network);

    /**
     * Unregister a Way Network (e.g., when network dissolves).
     * @param Network The network to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Verse|Networks")
    void UnregisterNetwork(UWayNetworkDataAsset* Network);

    /**
     * Get all registered networks that a Way belongs to.
     * @param Way The Way to check membership for
     * @return Array of networks this Way is part of
     */
    UFUNCTION(BlueprintPure, Category = "Verse|Networks")
    TArray<UWayNetworkDataAsset*> GetNetworksForWay(const UWayDataAsset* Way) const;

    /**
     * Calculate aggregate Verse score for all Ways in a network.
     * This represents the player's overall standing with the network.
     * @param Network The network to calculate score for
     * @return Aggregate network reputation score
     */
    UFUNCTION(BlueprintPure, Category = "Verse|Networks")
    float GetNetworkVerseScore(const UWayNetworkDataAsset* Network) const;

    /**
     * Check if player qualifies for network-wide bonuses.
     * @param Network The network to check
     * @return True if player meets reputation threshold with any member
     */
    UFUNCTION(BlueprintPure, Category = "Verse|Networks")
    bool QualifiesForNetworkBonuses(const UWayNetworkDataAsset* Network) const;

    /**
     * Award a Feat with network spillover effects.
     * When player completes a Feat, reputation gains spillover to network members.
     * @param FeatToRecord The Feat being awarded
     * @param bApplyNetworkEffects Whether to apply network spillover (default true)
     */
    UFUNCTION(BlueprintCallable, Category = "Verse|Networks")
    void RecordFeatWithNetworkEffects(const UFeatDataAsset* FeatToRecord, bool bApplyNetworkEffects = true);

private:
    /** The set of all Feats the player has successfully completed. Should be loaded from save data. */
    UPROPERTY(VisibleAnywhere, Category = "Verse State")
    TSet<TObjectPtr<const UFeatDataAsset>> CompletedFeats;

    /** All registered Way Networks in the universe */
    UPROPERTY(VisibleAnywhere, Category = "Verse State")
    TArray<TObjectPtr<UWayNetworkDataAsset>> RegisteredNetworks;

    /**
     * Placeholder function to load the player's saved Feats.
     * In a real implementation, this would read from a UAdastreaSaveGame object.
     */
    void LoadVerseState();
};
