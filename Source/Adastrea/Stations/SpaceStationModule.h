// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.generated.h"

// Forward declaration
class UFactionDataAsset;

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

/**
 * Base class for all space station modules
 * 
 * Modules are the building blocks of space stations. Each module represents a
 * functional component that can be attached to a station.
 * 
 * Features:
 * - Type classification for module identification
 * - Power requirement tracking
 * - Group categorization for organization
 * - Faction assignment (modules can have different owners than the parent station)
 * 
 * Usage:
 * - Create Blueprint based on this class for specific module types
 * - Configure ModuleType, ModulePower, and ModuleGroup
 * - Optionally assign a ModuleFaction for multi-faction stations
 * - Add to ASpaceStation using AddModule() or AddModuleAtLocation()
 */
UCLASS()
class ADASTREA_API ASpaceStationModule : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStationModule();

    // Type identifier for this module (e.g., "Docking Bay", "Reactor Core")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    FString ModuleType;

    // Power consumption or generation (negative = generates, positive = consumes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    float ModulePower;

    // Functional group for organization and filtering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    EStationModuleGroup ModuleGroup;

    // Faction this module belongs to (can differ from station faction for multi-faction stations)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    UFactionDataAsset* ModuleFaction;

    /**
     * Get the faction assigned to this module
     * @return The faction data asset, or nullptr if none assigned
     */
    UFUNCTION(BlueprintCallable, Category="Module")
    UFactionDataAsset* GetModuleFaction() const;

    /**
     * Set the faction for this module
     * @param NewFaction The faction to assign to this module
     */
    UFUNCTION(BlueprintCallable, Category="Module")
    void SetModuleFaction(UFactionDataAsset* NewFaction);
};