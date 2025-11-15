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
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastrea, Log, All);

// Spaceship system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaShips, Log, All);

// Faction system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaFactions, Log, All);

// Personnel/Crew system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaPersonnel, Log, All);

// AI systems
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaAI, Log, All);

// Station system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaStations, Log, All);

// Trading/Economy system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaTrading, Log, All);

// Player modifications
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaPlayerMods, Log, All);

// Material system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaMaterials, Log, All);

// Combat system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaCombat, Log, All);

// Navigation system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaNavigation, Log, All);

// Quest system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaQuest, Log, All);

// Way/Guild system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaWay, Log, All);

// Input system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaInput, Log, All);
