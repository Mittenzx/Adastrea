// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Adastrea Logging Categories
 * 
 * This file defines all custom log categories used in the Adastrea project.
 * Using dedicated log categories allows for better filtering and debugging.
 * 
 * Usage:
 *   UE_LOG(LogAdastreaShips, Warning, TEXT("Ship %s has invalid data"), *ShipName);
 * 
 * To enable verbose logging for a category:
 *   Log LogAdastreaShips Verbose
 */

// Core game systems
DECLARE_LOG_CATEGORY_EXTERN(LogAdastrea, Log, All);

// Spaceship system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaShips, Log, All);

// Faction system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaFactions, Log, All);

// Personnel/Crew system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaPersonnel, Log, All);

// AI systems
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaAI, Log, All);

// Station system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaStations, Log, All);

// Trading/Economy system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaTrading, Log, All);

// Player modifications
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaPlayerMods, Log, All);

// Material system
DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaMaterials, Log, All);
