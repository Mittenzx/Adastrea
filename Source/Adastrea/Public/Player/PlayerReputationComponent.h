// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerReputationComponent.generated.h"

// Forward declarations
class UFactionDataAsset;

/**
 * Enum for reputation tiers
 * Defines how NPCs and systems react to the player based on reputation
 */
UENUM(BlueprintType)
enum class EReputationTier : uint8
{
	Hostile UMETA(DisplayName = "Hostile"),           // -100 to -60
	Unfriendly UMETA(DisplayName = "Unfriendly"),     // -59 to -20
	Neutral UMETA(DisplayName = "Neutral"),           // -19 to 19
	Friendly UMETA(DisplayName = "Friendly"),         // 20 to 59
	Honored UMETA(DisplayName = "Honored"),           // 60 to 89
	Revered UMETA(DisplayName = "Revered"),           // 90 to 99
	Exalted UMETA(DisplayName = "Exalted")            // 100
};

/**
 * Player's reputation with a single faction
 * Stored as part of save game data
 */
USTRUCT(BlueprintType)
struct FPlayerFactionReputation
{
	GENERATED_BODY()

	/** The faction this reputation is for */
	UPROPERTY(BlueprintReadWrite, Category="Reputation")
	FName FactionID;

	/** Current reputation value (-100 to 100) */
	UPROPERTY(BlueprintReadWrite, Category="Reputation")
	int32 ReputationValue;

	/** Current reputation tier */
	UPROPERTY(BlueprintReadWrite, Category="Reputation")
	EReputationTier Tier;

	FPlayerFactionReputation()
		: FactionID(NAME_None)
		, ReputationValue(0)
		, Tier(EReputationTier::Neutral)
	{}

	FPlayerFactionReputation(FName InFactionID, int32 InReputation)
		: FactionID(InFactionID)
		, ReputationValue(InReputation)
	{
		Tier = CalculateTier(InReputation);
	}

	/** Calculate tier from reputation value */
	static EReputationTier CalculateTier(int32 Reputation)
	{
		if (Reputation >= 100) return EReputationTier::Exalted;
		if (Reputation >= 90) return EReputationTier::Revered;
		if (Reputation >= 60) return EReputationTier::Honored;
		if (Reputation >= 20) return EReputationTier::Friendly;
		if (Reputation >= -19) return EReputationTier::Neutral;
		if (Reputation >= -59) return EReputationTier::Unfriendly;
		return EReputationTier::Hostile;
	}
};

/**
 * Player Reputation Component
 * Tracks the player's standing with all factions in the game
 * 
 * This component manages:
 * - Reputation values for each faction (-100 to 100 scale)
 * - Reputation tiers (Hostile, Unfriendly, Neutral, Friendly, Honored, Revered, Exalted)
 * - Reputation change events and notifications
 * - Save/load integration for persistent reputation
 * 
 * Usage:
 * 1. Add to player pawn/controller
 * 2. Initialize with faction list from game state
 * 3. Modify reputation via ModifyReputation function
 * 4. Query reputation via GetReputation or GetReputationTier
 * 
 * Integration:
 * - Trading System: Affects prices and availability
 * - Quest System: Determines available quests
 * - Faction AI: Controls NPC hostility/friendliness
 * - Save System: Persists across sessions
 */
UCLASS(ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerReputationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerReputationComponent();

	// ====================
	// Reputation Data
	// ====================

	/** Map of faction ID to reputation data */
	UPROPERTY(BlueprintReadOnly, Category="Reputation")
	TMap<FName, FPlayerFactionReputation> FactionReputations;

	// ====================
	// Core Functions
	// ====================

	/**
	 * Initialize reputation for a faction
	 * @param FactionID The faction to initialize
	 * @param InitialReputation Starting reputation value
	 */
	UFUNCTION(BlueprintCallable, Category="Reputation")
	void InitializeFactionReputation(FName FactionID, int32 InitialReputation = 0);

	/**
	 * Initialize reputation from a faction data asset
	 * @param FactionData The faction data asset
	 */
	UFUNCTION(BlueprintCallable, Category="Reputation")
	void InitializeFromFactionData(UFactionDataAsset* FactionData);

	/**
	 * Modify reputation with a faction
	 * @param FactionID The faction to modify
	 * @param Amount Amount to change (positive or negative)
	 */
	UFUNCTION(BlueprintCallable, Category="Reputation")
	void ModifyReputation(FName FactionID, int32 Amount);

	/**
	 * Get current reputation value with a faction
	 * @param FactionID The faction to query
	 * @return Current reputation value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
	int32 GetReputation(FName FactionID) const;

	/**
	 * Get current reputation tier with a faction
	 * @param FactionID The faction to query
	 * @return Current reputation tier
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
	EReputationTier GetReputationTier(FName FactionID) const;

	/**
	 * Check if player should be hostile to this faction
	 * @param FactionID The faction to check
	 * @return True if reputation is in Hostile tier
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
	bool ShouldBeHostile(FName FactionID) const;

	/**
	 * Get trade price modifier based on reputation
	 * @param FactionID The faction to check
	 * @return Price multiplier (0.7 to 1.3, where 1.0 is normal)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
	float GetTradePriceModifier(FName FactionID) const;

	/**
	 * Get all faction reputations as array
	 * @return Array of all faction reputations
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Reputation")
	TArray<FPlayerFactionReputation> GetAllReputations() const;

protected:
	virtual void BeginPlay() override;
};
