#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerReputationComponent.generated.h"

// Forward declarations
class UFactionDataAsset;

/**
 * Reputation entry tracking player's standing with a faction
 */
USTRUCT(BlueprintType)
struct FPlayerFactionReputation
{
    GENERATED_BODY()

    /** The faction this reputation is with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation")
    FName FactionID;

    /** Current reputation value (-100 to 100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation", meta=(ClampMin="-100", ClampMax="100"))
    int32 ReputationValue;

    /** Whether the player has completed the faction's storyline */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation")
    bool bStorylineCompleted;

    /** Number of missions completed for this faction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation")
    int32 MissionsCompleted;

    FPlayerFactionReputation()
        : FactionID(NAME_None)
        , ReputationValue(0)
        , bStorylineCompleted(false)
        , MissionsCompleted(0)
    {}
};

/**
 * Reputation tier levels for gameplay effects
 */
UENUM(BlueprintType)
enum class EReputationTier : uint8
{
    Hated       UMETA(DisplayName = "Hated (-100 to -76)"),
    Hostile     UMETA(DisplayName = "Hostile (-75 to -51)"),
    Unfriendly  UMETA(DisplayName = "Unfriendly (-50 to -26)"),
    Neutral     UMETA(DisplayName = "Neutral (-25 to 25)"),
    Friendly    UMETA(DisplayName = "Friendly (26 to 50)"),
    Honored     UMETA(DisplayName = "Honored (51 to 75)"),
    Revered     UMETA(DisplayName = "Revered (76 to 100)")
};

/**
 * Player reputation component for tracking standing with all factions.
 * Manages player reputation, provides gameplay effects, and tracks faction relationships.
 * 
 * Usage:
 * - Attach to player pawn or player controller
 * - Modify reputation via Blueprint or C++
 * - Query reputation for trade prices, NPC hostility, quest availability
 * 
 * Example:
 * - Player completes mission for Faction A: ModifyReputation("FactionA", 10)
 * - Check if player can access faction store: GetReputationTier("FactionA") >= Friendly
 * - Apply reputation-based trade discount: GetTradePriceModifier("FactionA")
 */
UCLASS(BlueprintType, ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerReputationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerReputationComponent();

    // ====================
    // Core Properties
    // ====================

    /** All faction reputations tracked for this player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation")
    TArray<FPlayerFactionReputation> FactionReputations;

    /** Default reputation for factions not yet encountered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation", meta=(ClampMin="-100", ClampMax="100"))
    int32 DefaultReputation;

    /** Reputation change multiplier (for difficulty settings or perks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reputation", meta=(ClampMin="0.1", ClampMax="5.0"))
    float ReputationGainMultiplier;

    // ====================
    // Reputation Management
    // ====================

    /**
     * Modify reputation with a faction
     * @param FactionID The faction to modify reputation with
     * @param Delta Amount to change (can be negative)
     * @param bClamp Whether to clamp result to -100/+100
     */
    UFUNCTION(BlueprintCallable, Category="Reputation")
    void ModifyReputation(FName FactionID, int32 Delta, bool bClamp = true);

    /**
     * Set reputation with a faction to a specific value
     * @param FactionID The faction to set reputation for
     * @param NewValue The new reputation value
     */
    UFUNCTION(BlueprintCallable, Category="Reputation")
    void SetReputation(FName FactionID, int32 NewValue);

    /**
     * Get current reputation with a faction
     * @param FactionID The faction to check
     * @return Current reputation value (or DefaultReputation if never encountered)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
    int32 GetReputation(FName FactionID) const;

    /**
     * Get reputation tier with a faction
     * @param FactionID The faction to check
     * @return The reputation tier enum value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
    EReputationTier GetReputationTier(FName FactionID) const;

    /**
     * Check if reputation meets a minimum threshold
     * @param FactionID The faction to check
     * @param Threshold Minimum reputation required
     * @return True if reputation >= threshold
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
    bool IsReputationAtLeast(FName FactionID, int32 Threshold) const;

    /**
     * Get all faction reputations
     * @return Array of all reputation entries
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
    TArray<FPlayerFactionReputation> GetAllReputations() const;

    // ====================
    // Gameplay Effects
    // ====================

    /**
     * Get trade price modifier based on reputation
     * Higher reputation = lower prices (better deals)
     * @param FactionID The faction to check
     * @return Multiplier for trade prices (0.5 = 50% off, 1.5 = 50% markup)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Effects")
    float GetTradePriceModifier(FName FactionID) const;

    /**
     * Check if faction NPCs should be hostile to player
     * @param FactionID The faction to check
     * @return True if NPCs should attack on sight
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Effects")
    bool ShouldBeHostile(FName FactionID) const;

    /**
     * Check if player can access faction's restricted areas/stores
     * @param FactionID The faction to check
     * @return True if player has sufficient reputation
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Effects")
    bool CanAccessRestrictedAreas(FName FactionID) const;

    /**
     * Get quality level of missions offered by faction (based on reputation)
     * @param FactionID The faction to check
     * @return Mission quality level (0-5, higher = better rewards)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Effects")
    int32 GetMissionQualityLevel(FName FactionID) const;

    // ====================
    // Mission Tracking
    // ====================

    /**
     * Record a completed mission for a faction
     * @param FactionID The faction the mission was for
     */
    UFUNCTION(BlueprintCallable, Category="Reputation|Missions")
    void RecordMissionCompleted(FName FactionID);

    /**
     * Get number of missions completed for a faction
     * @param FactionID The faction to check
     * @return Number of completed missions
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Missions")
    int32 GetMissionsCompleted(FName FactionID) const;

    /**
     * Mark a faction's storyline as completed
     * @param FactionID The faction whose storyline was completed
     */
    UFUNCTION(BlueprintCallable, Category="Reputation|Missions")
    void MarkStorylineCompleted(FName FactionID);

    /**
     * Check if faction storyline is completed
     * @param FactionID The faction to check
     * @return True if storyline completed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Missions")
    bool IsStorylineCompleted(FName FactionID) const;

    // ====================
    // Helper Functions
    // ====================

    /**
     * Convert reputation value to tier enum
     * @param ReputationValue The reputation value to convert
     * @return The corresponding tier
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Utility")
    static EReputationTier ValueToTier(int32 ReputationValue);

    /**
     * Get display name for a reputation tier
     * @param Tier The tier to get name for
     * @return Localized display name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation|Utility")
    static FText GetTierDisplayName(EReputationTier Tier);

protected:
    /**
     * Find or create a reputation entry for a faction
     * @param FactionID The faction to find/create entry for
     * @return Pointer to the reputation entry
     */
    FPlayerFactionReputation* FindOrCreateReputation(FName FactionID);

    /**
     * Find a reputation entry (const version)
     * @param FactionID The faction to find
     * @return Pointer to the reputation entry, or nullptr if not found
     */
    const FPlayerFactionReputation* FindReputation(FName FactionID) const;
};
