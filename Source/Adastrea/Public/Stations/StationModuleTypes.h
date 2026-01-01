// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "StationModuleTypes.generated.h"

/**
 * Module classification for space station organization
 * Groups modules by primary function for easy filtering and management
 * 
 * MVP SCOPE (Trade Simulator):
 * - Docking: Required for trading interface
 * - Storage: Cargo and goods management
 * - Habitation: Basic station functionality
 * - Public: Market and trading areas
 * - Connection: Station navigation
 * 
 * POST-MVP SCOPE:
 * - Defence: Weapons and shields (combat system)
 * - Power/Processing: Advanced station mechanics
 */
UENUM(BlueprintType)
enum class EStationModuleGroup : uint8
{
    All         UMETA(DisplayName="All"),           // All modules (no filtering)
    Docking     UMETA(DisplayName="Docking"),      // Ship docking bays and hangers (MVP)
    Power       UMETA(DisplayName="Power"),        // Reactors and generators (POST-MVP)
    Storage     UMETA(DisplayName="Storage"),      // Cargo and material storage (MVP)
    Processing  UMETA(DisplayName="Processing"),   // Manufacturing and refinement (POST-MVP)
    Defence     UMETA(DisplayName="Defence"),      // Weapons and shields (POST-MVP: Combat system)
    Habitation  UMETA(DisplayName="Habitation"),   // Living quarters and crew facilities (MVP)
    Public      UMETA(DisplayName="Public"),       // Public spaces, markets, entertainment (MVP)
    Connection  UMETA(DisplayName="Connection"),   // Corridors and connectors (MVP)
    Other       UMETA(DisplayName="Other")         // Miscellaneous modules
};
