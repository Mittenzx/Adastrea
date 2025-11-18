// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AdastreaGameInstance.generated.h"

/**
 * Game Instance for Adastrea
 * 
 * This class maintains game-wide state that persists across level transitions
 * and manages core systems that need to exist throughout the game session.
 * 
 * Key Responsibilities:
 * - Managing save/load operations for player progress
 * - Storing persistent player data (credits, reputation, inventory)
 * - Managing global economy and market conditions
 * - Handling game settings and configuration
 * 
 * Game Instance Lifecycle:
 * - Created when the game starts, before any level loads
 * - Persists across all level transitions
 * - Destroyed only when the game closes
 * 
 * Usage:
 * - Access via UGameplayStatics::GetGameInstance()
 * - Store data that needs to survive level changes
 * - Initialize managers and subsystems in Init()
 * - Create Blueprints derived from this class for project-specific state
 * 
 * Integration Points:
 * - Trading system for market state and economy
 * - Save/load system for persistence
 * - Player progression and career tracking
 */
UCLASS(BlueprintType)
class ADASTREA_API UAdastreaGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAdastreaGameInstance();

	virtual void Init() override;

	virtual void Shutdown() override;

	/**
	 * Save the current game state to a save slot
	 * @param SlotName The name of the save slot
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save System")
	bool SaveGame(const FString& SlotName);

	/**
	 * Load game state from a save slot
	 * @param SlotName The name of the save slot to load
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Save System")
	bool LoadGame(const FString& SlotName);

	/**
	 * Get the player's current credits
	 * @return The amount of credits the player has
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player")
	int32 GetPlayerCredits() const { return PlayerCredits; }

	/**
	 * Add or remove credits from the player's balance
	 * @param Amount The amount to add (positive) or subtract (negative)
	 */
	UFUNCTION(BlueprintCallable, Category="Player")
	void ModifyPlayerCredits(int32 Amount);

protected:
	/**
	 * Player's current credit balance
	 */
	UPROPERTY(BlueprintReadOnly, Category="Player")
	int32 PlayerCredits;

	/**
	 * Initialize game systems and managers
	 * Called during Init()
	 */
	virtual void InitializeGameSystems();
};
