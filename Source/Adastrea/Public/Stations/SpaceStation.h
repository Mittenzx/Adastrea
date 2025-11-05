#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "SpaceStation.generated.h"

// Forward declaration
class UFactionDataAsset;

/**
 * Core space station actor with modular construction system
 * 
 * Space stations are large structures that can be built from individual modules.
 * This class manages the collection of modules and provides functionality for
 * adding, removing, and querying modules.
 * 
 * Features:
 * - Dynamic module management (add/remove/move)
 * - Faction ownership and integration
 * - Module filtering by type
 * - Trait and diplomacy system integration
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Assign a Faction using SetFaction() or in the editor
 * 3. Add modules using AddModule() or AddModuleAtLocation()
 * 4. Query modules with GetModules() or GetModulesByType()
 * 5. Use faction functions to check traits and relationships
 * 
 * See Also: STATION_EDITOR_README.md for detailed implementation guide
 */
UCLASS()
class ADASTREA_API ASpaceStation : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStation();

    // Faction this station belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
    UFactionDataAsset* Faction;

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
     * Get the faction this station belongs to
     * @return The faction data asset, or nullptr if none assigned
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    UFactionDataAsset* GetFaction() const;

    /**
     * Set the faction this station belongs to
     * @param NewFaction The faction to assign
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    void SetFaction(UFactionDataAsset* NewFaction);

    // ====================
    // Advanced Trait & Diplomacy Hooks
    // ====================

    /**
     * Check if this station's faction has a specific trait
     * @param TraitID The trait to check for
     * @return True if the faction has the trait
     */
    UFUNCTION(BlueprintCallable, Category="Station|Traits")
    bool HasFactionTrait(FName TraitID) const;

    /**
     * Get the trait modifier value for the station's faction
     * @param TraitID The trait to get modifier for
     * @return The modifier value
     */
    UFUNCTION(BlueprintCallable, Category="Station|Traits")
    float GetFactionTraitModifier(FName TraitID) const;

    /**
     * Check if this station's faction is allied with another faction
     * @param OtherFactionID The faction to check alliance with
     * @return True if allied
     */
    UFUNCTION(BlueprintCallable, Category="Station|Diplomacy")
    bool IsAlliedWithFaction(FName OtherFactionID) const;

    /**
     * Check if this station's faction is at war with another faction
     * @param OtherFactionID The faction to check war status with
     * @return True if at war
     */
    UFUNCTION(BlueprintCallable, Category="Station|Diplomacy")
    bool IsAtWarWithFaction(FName OtherFactionID) const;

    /**
     * Get relationship value between this station's faction and another
     * @param OtherFactionID The faction to get relationship with
     * @return Relationship value (-100 to 100)
     */
    UFUNCTION(BlueprintCallable, Category="Station|Diplomacy")
    int32 GetFactionRelationship(FName OtherFactionID) const;

protected:
    virtual void BeginPlay() override;
};
