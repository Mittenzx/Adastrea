#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "SpaceStation.generated.h"

/**
 * Core space station actor with modular construction system
 * 
 * Space stations are large structures that can be built from individual modules.
 * This class manages the collection of modules and provides functionality for
 * adding, removing, and querying modules.
 * 
 * Features:
 * - Dynamic module management (add/remove/move)
 * - Module filtering by type
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Add modules using AddModule() or AddModuleAtLocation()
 * 3. Query modules with GetModules() or GetModulesByType()
 * 
 * See Also: STATION_EDITOR_README.md for detailed implementation guide
 */
UCLASS()
class ADASTREA_API ASpaceStation : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStation();

    // Array of attached modules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
    TArray<ASpaceStationModule*> Modules;

    /**
     * Add a module to the station (simple version without position)
     * @param Module The module to add
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    void AddModule(ASpaceStationModule* Module);

    /**
     * Add a module to the station at a specific location
     * @param Module The module to add
     * @param RelativeLocation Position relative to the station
     * @return True if module was successfully added
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation);

    /**
     * Remove a module from the station
     * @param Module The module to remove
     * @return True if module was found and removed
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool RemoveModule(ASpaceStationModule* Module);

    /**
     * Move a module to a new location within the station
     * @param Module The module to move
     * @param NewRelativeLocation New position relative to the station
     * @return True if module was found and moved
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    /**
     * Get all attached modules
     * @return Array of all modules attached to this station
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModules() const;

    /**
     * Get all modules of a specific type
     * @param ModuleType The type identifier to filter by
     * @return Array of matching modules
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType) const;

    /**
     * Get the number of attached modules
     * @return Total count of modules
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    int32 GetModuleCount() const;

    /**
     * Set the faction that owns this station
     * @param NewFaction The faction to assign to this station
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    void SetFaction(class UFactionDataAsset* NewFaction);

    /**
     * Get the faction that owns this station
     * @return The faction data asset, or nullptr if no faction is assigned
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station")
    class UFactionDataAsset* GetFaction() const;

protected:
    virtual void BeginPlay() override;

    /** The faction that owns this station (can be null for neutral stations) */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Station", meta=(AllowPrivateAccess="true"))
    class UFactionDataAsset* OwningFaction;
};
