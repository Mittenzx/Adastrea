// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdastreaGameMode.generated.h"

// Forward declarations
class ASpaceship;
class UTestSettingsWidget;

/**
 * Base game mode for Adastrea
 * 
 * This is the primary game mode that controls gameplay rules and state.
 * 
 * Features:
 * - Optional test settings pre-screen for development/QA testing
 * - Automatic player spaceship spawning with flexible location options
 * - Configurable spawn behavior (sector-based or fallback location)
 * - Blueprint-friendly customization
 * 
 * Usage:
 * 1. Set as the GameMode Override in World Settings
 * 2. Configure DefaultSpaceshipClass to the desired spaceship Blueprint
 * 3. Toggle bAutoSpawnPlayerShip to enable/disable automatic spawning
 * 4. If using SpaceSectorMap, choose spawn location type with bSpawnAtCenter
 * 5. If no SpaceSectorMap exists, set FallbackSpawnLocation for spawn position
 * 6. Set TestSettingsWidgetClass to show test settings on startup (optional)
 */
UCLASS()
class AAdastreaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAdastreaGameMode();

	/**
	 * Test Settings Widget class to display on game start
	 * If set, this widget will be shown before spawning the player ship
	 * Useful for development and QA testing to configure test parameters
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Settings")
	TSubclassOf<UTestSettingsWidget> TestSettingsWidgetClass;

	/**
	 * Whether to show the test settings widget on startup
	 * Only has effect if TestSettingsWidgetClass is set
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Settings")
	bool bShowTestSettingsOnStartup;

	/**
	 * Default spaceship class to spawn for the player
	 * Set this to your spaceship Blueprint in editor or via derived Blueprint
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	TSubclassOf<ASpaceship> DefaultSpaceshipClass;

	/**
	 * Whether to automatically spawn a player spaceship on level start
	 * If a SpaceSectorMap exists, uses sector-based spawn location
	 * If no SpaceSectorMap exists, uses FallbackSpawnLocation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	bool bAutoSpawnPlayerShip;

	/**
	 * If true, spawn at sector center. If false, spawn at random position in sector
	 * Only applies when a SpaceSectorMap exists in the level
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	bool bSpawnAtCenter;

	/**
	 * Fallback spawn location (world space) when no SpaceSectorMap is found in the level
	 * Used to allow ship spawning in test levels or simple maps without sectors
	 * Default is (0, 0, 0) - world origin. For test levels, consider using (0, 0, 100)
	 * to spawn slightly above the ground plane
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawn")
	FVector FallbackSpawnLocation;

protected:
	virtual void BeginPlay() override;

	/**
	 * Spawns the player spaceship in the sector and possesses it
	 * Called automatically from BeginPlay if bAutoSpawnPlayerShip is true
	 * Can be called manually after test settings are confirmed
	 */
	UFUNCTION(BlueprintCallable, Category="Player Spawn")
	virtual void SpawnPlayerSpaceship();

	/**
	 * Called when test settings widget confirms continuation
	 * Proceeds with normal game initialization (ship spawning, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category="Test Settings")
	virtual void OnTestSettingsContinue();

private:
	/**
	 * Show the test settings widget
	 * Called from BeginPlay if bShowTestSettingsOnStartup is true
	 */
	void ShowTestSettingsWidget();

	/**
	 * Reference to the active test settings widget instance
	 */
	UPROPERTY()
	UTestSettingsWidget* TestSettingsWidget;
};
