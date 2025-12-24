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

// Exploration system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaExploration, Log, All);

// Procedural generation system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaProceduralGen, Log, All);

// Data Asset validation system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaDataAssetValidation, Log, All);

/**
 * Validation Macros for Null Pointer and Actor Checks
 *
 * These macros provide consistent null checking and logging throughout the codebase.
 * They help prevent crashes and provide better debugging information.
 *
 * Usage:
 *   ADASTREA_VALIDATE_PTR(MyPointer, return);
 *   ADASTREA_VALIDATE_ACTOR(MyActor, return false);
 */

// Validates a pointer and logs warning if null
#define ADASTREA_VALIDATE_PTR(Ptr, ReturnStatement) \
    if (!(Ptr)) { \
        UE_LOG(LogAdastrea, Warning, TEXT("%s: Null pointer at %s:%d"), \
            TEXT(__FUNCTION__), TEXT(__FILE__), __LINE__); \
        ReturnStatement; \
    }

// Validates an actor using IsValid() and logs warning if invalid
#define ADASTREA_VALIDATE_ACTOR(Actor, ReturnStatement) \
    if (!IsValid(Actor)) { \
        UE_LOG(LogAdastrea, Warning, TEXT("%s: Invalid actor at %s:%d"), \
            TEXT(__FUNCTION__), TEXT(__FILE__), __LINE__); \
        ReturnStatement; \
    }

// Validates a UObject pointer and logs warning if null
#define ADASTREA_VALIDATE_UOBJECT(UObjectPtr, ReturnStatement) \
    if (!(UObjectPtr)) { \
        UE_LOG(LogAdastrea, Warning, TEXT("%s: Null UObject at %s:%d"), \
            TEXT(__FUNCTION__), TEXT(__FILE__), __LINE__); \
        ReturnStatement; \
    }
