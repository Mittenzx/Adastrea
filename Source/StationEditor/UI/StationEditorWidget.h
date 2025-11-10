#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "Factions/FactionDataAsset.h"
#include "StationEditorWidget.generated.h"

/**
 * Station Editor Widget - UI for managing space station modules
 * This class provides C++ hooks for Blueprint UI implementation
 * 
 * This is a runtime, player-facing UI widget that allows players to
 * construct and modify space stations in-game.
 */
UCLASS()
class STATIONEDITOR_API UStationEditorWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Reference to the station being edited
    UPROPERTY(BlueprintReadWrite, Category="Station Editor")
    ASpaceStation* CurrentStation;

    // Add a module to the current station
    // @param ModuleClass - The class of module to spawn
    // @param RelativeLocation - Location relative to the station center
    // @return The newly added module, or nullptr if failed
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    ASpaceStationModule* AddModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector RelativeLocation);

    // Remove a module from the current station
    // @param Module - The module to remove
    // @return true if the module was successfully removed
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool RemoveModule(ASpaceStationModule* Module);

    // Move a module to a new location within the station
    // @param Module - The module to move
    // @param NewRelativeLocation - New location relative to the station center
    // @return true if the module was successfully moved
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    // Get a list of all modules currently attached to the station
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    TArray<ASpaceStationModule*> GetAllModules();

    // Set the station to be edited
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    void SetStation(ASpaceStation* Station);

    // Helper function to validate module placement
    UFUNCTION(BlueprintCallable, Category="Station Editor")
    bool IsValidPlacement(FVector Location);

    // ====================
    // Faction Integration Hooks
    // ====================

    /**
     * Set the faction for the current station
     * @param NewFaction The faction to assign to the station
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    void SetStationFaction(UFactionDataAsset* NewFaction);

    /**
     * Get available module types that match the station's faction technology level
     * @return Array of module types that can be built
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    TArray<FString> GetAvailableModuleTypes() const;

    /**
     * Check if a module can be added based on faction restrictions
     * @param ModuleClass The module class to check
     * @return True if the module can be added
     */
    UFUNCTION(BlueprintCallable, Category="Station Editor|Faction")
    bool CanAddModuleForFaction(TSubclassOf<ASpaceStationModule> ModuleClass) const;

protected:
    // Called when the widget is constructed
    virtual void NativeConstruct() override;
};
