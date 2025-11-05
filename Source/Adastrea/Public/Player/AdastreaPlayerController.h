// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AdastreaPlayerController.generated.h"

/**
 * Player Controller for Adastrea
 * 
 * This class manages player input, interaction with the game world, and player-specific
 * state within the Adastrea game. It serves as the primary interface between the player
 * and their controlled entities (spaceships, characters, etc.).
 * 
 * Key Responsibilities:
 * - Processing player input for spaceship flight controls
 * - Managing camera and viewport settings
 * - Handling UI interactions and HUD updates
 * - Coordinating with AdastreaGameInstance for persistent player data
 * - Managing player reputation and faction relationships
 * 
 * Usage:
 * - Set as the Player Controller Class in the Game Mode
 * - Create Blueprints derived from this class for project-specific functionality
 * - Override input handling functions to customize controls
 * - Use Blueprint events to extend behavior without C++ changes
 * 
 * Integration Points:
 * - Works with ASpaceship for vehicle control
 * - Communicates with UPlayerReputationComponent for faction standing
 * - Accesses UAdastreaGameInstance for save/load operations
 * - Updates UI widgets through the HUD system
 */
UCLASS(BlueprintType)
class ADASTREA_API AAdastreaPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAdastreaPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

public:
	/**
	 * Called when the player possesses a new spaceship
	 * Override in Blueprints to handle custom possession logic
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Player")
	void OnPossessSpaceship(class ASpaceship* NewSpaceship);
	virtual void OnPossessSpaceship_Implementation(class ASpaceship* NewSpaceship);

	/**
	 * Get the currently controlled spaceship, if any
	 * @return The possessed spaceship actor, or nullptr if not controlling a ship
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player")
	class ASpaceship* GetControlledSpaceship() const;

	/**
	 * Check if the player is currently controlling a spaceship
	 * @return True if the player is possessing a spaceship actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Player")
	bool IsControllingSpaceship() const;
};
