// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "StationModuleTypes.generated.h"

/**
 * Module classification for space station organization
 * Groups modules by primary function for easy filtering and management
 */
UENUM(BlueprintType)
enum class EStationModuleGroup : uint8
{
    All         UMETA(DisplayName="All"),           // All modules (no filtering)
    Docking     UMETA(DisplayName="Docking"),      // Ship docking bays and hangers
    Power       UMETA(DisplayName="Power"),        // Reactors and generators
    Storage     UMETA(DisplayName="Storage"),      // Cargo and material storage
    Processing  UMETA(DisplayName="Processing"),   // Manufacturing and refinement
    Defence     UMETA(DisplayName="Defence"),      // Weapons and shields
    Habitation  UMETA(DisplayName="Habitation"),   // Living quarters and crew facilities
    Public      UMETA(DisplayName="Public"),       // Public spaces, markets, entertainment
    Connection  UMETA(DisplayName="Connection"),   // Corridors and connectors
    Other       UMETA(DisplayName="Other")         // Miscellaneous modules
};
