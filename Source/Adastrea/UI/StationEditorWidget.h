#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Stations/SpaceStation.h"
#include "Stations/SpaceStationModule.h"
#include "StationEditorWidget.generated.h"

/**
 * Station Editor Widget - UI for managing space station modules
 * This class provides C++ hooks for Blueprint UI implementation
 */
UCLASS()
class ADASTREA_API UStationEditorWidget : public UUserWidget
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

protected:
    // Called when the widget is constructed
    virtual void NativeConstruct() override;
};
