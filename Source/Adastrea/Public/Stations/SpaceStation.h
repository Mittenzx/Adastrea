#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "SpaceStation.generated.h"

// Forward declarations
class AMarketplaceModule;

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
 * Implements:
 * - IDamageable: Can receive damage from weapons
 * - ITargetable: Can be targeted by weapons and sensors
 * 
 * REMOVED (Trade Simulator MVP):
 * - IFactionMember interface - faction system not needed for MVP
 * 
 * Usage:
 * 1. Create Blueprint based on this class
 * 2. Add modules using AddModule() or AddModuleAtLocation()
 * 3. Query modules with GetModules() or GetModulesByType()
 * 
 * See Also: STATION_EDITOR_README.md for detailed implementation guide
 */
UCLASS()
class ADASTREA_API ASpaceStation : public AActor, public IDamageable, public ITargetable
{
    GENERATED_BODY()

public:
    ASpaceStation();

    /**
     * Default module classes to spawn when station is created
     * 
     * Configure this array in Class Defaults to specify which module types
     * should be automatically spawned and attached when the station begins play.
     * 
     * Example Usage:
     * 1. Open SpaceStation Blueprint Class Defaults
     * 2. Add entries to DefaultModuleClasses array
     * 3. Select module classes (BP_DockingBayModule, BP_ReactorModule, etc.)
     * 4. Modules will be spawned automatically in BeginPlay
     * 
     * Note: This is separate from the runtime Modules array which tracks
     * all currently attached modules (including dynamically added ones).
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station|Configuration",
        meta=(ToolTip="Module classes to automatically spawn when station is created"))
    TArray<TSubclassOf<ASpaceStationModule>> DefaultModuleClasses;

    /**
     * Array of currently attached modules (runtime tracking)
     * 
     * This array is populated automatically:
     * - BeginPlay spawns modules from DefaultModuleClasses
     * - AddModule() adds dynamically created modules
     * - RemoveModule() removes modules
     * 
     * Do not edit this in Class Defaults - it's for runtime use only.
     * Use DefaultModuleClasses instead to configure initial modules.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station",
        meta=(ToolTip="Currently attached modules (runtime only, populated automatically)"))
    TArray<ASpaceStationModule*> Modules;

    // ====================
    // POST-MVP: MODULE CONSTRUCTION & MANAGEMENT
    // Deferred - Not needed for Trade Simulator MVP (stations are pre-built)
    // Can be re-enabled post-MVP for station building gameplay
    // ====================

    /**
     * [POST-MVP] Add a module to the station (simple version without position)
     * @param Module The module to add
     * 
     * DEFERRED: Module construction not needed for MVP trading.
     * MVP uses pre-built stations with fixed module layouts.
     * Re-enable post-MVP for station building gameplay.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    void AddModule(ASpaceStationModule* Module);

    /**
     * [POST-MVP] Add a module to the station at a specific location
     * @param Module The module to add
     * @param RelativeLocation Position relative to the station
     * @return True if module was successfully added
     * 
     * DEFERRED: Module construction not needed for MVP trading.
     * Re-enable post-MVP for station building gameplay.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    bool AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation);

    /**
     * [POST-MVP] Remove a module from the station
     * @param Module The module to remove
     * @return True if module was found and removed
     * 
     * DEFERRED: Module management not needed for MVP trading.
     * Re-enable post-MVP for station modification gameplay.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    bool RemoveModule(ASpaceStationModule* Module);

    /**
     * [POST-MVP] Move a module to a new location within the station
     * @param Module The module to move
     * @param NewRelativeLocation New position relative to the station
     * @return True if module was found and moved
     * 
     * DEFERRED: Module management not needed for MVP trading.
     * Re-enable post-MVP for station customization gameplay.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    /**
     * [POST-MVP] Get all modules of a specific type
     * @param ModuleType The type identifier to filter by
     * @return Array of matching modules
     * 
     * DEFERRED: Advanced module queries not needed for MVP trading.
     * Use GetModules() and filter in Blueprint if needed.
     * Re-enable post-MVP for complex station management.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType) const;

    /**
     * [POST-MVP] Get the number of attached modules
     * @return Total count of modules
     * 
     * DEFERRED: Convenience wrapper not needed for MVP.
     * Use GetModules().Num() in Blueprint instead.
     * Re-enable post-MVP if frequently used.
     */
    // UFUNCTION(BlueprintCallable, Category="Station|Post-MVP")
    int32 GetModuleCount() const;

    // ====================
    // MVP-CRITICAL: STATION CORE FUNCTIONS
    // Essential for Trade Simulator MVP
    // ====================

    /**
     * Get all attached modules
     * @return Array of all modules attached to this station
     * 
     * MVP USE: May be useful for station UI (showing available facilities)
     * Kept for potential Blueprint UI needs.
     */
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModules() const;

    /**
     * Get the first marketplace module attached to this station
     * Useful for opening trading UI
     * @return First marketplace module found, or nullptr if none exist
     * 
     * MVP USE: Essential for Trade Simulator MVP - finds trading interface
     */
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    AMarketplaceModule* GetMarketplaceModule() const;

    /**
     * Get all marketplace modules attached to this station
     * @return Array of all marketplace modules
     * 
     * MVP USE: Supports stations with multiple markets (e.g., legal + black market)
     */
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    TArray<AMarketplaceModule*> GetMarketplaceModules() const;

    // REMOVED: SetFaction() - faction system removed per Trade Simulator MVP scope

    // ====================
    // INTERFACE IMPLEMENTATIONS
    // ====================

    // IDamageable Interface
    virtual float ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser) override;
    virtual bool CanTakeDamage_Implementation() const override;
    virtual float GetHealthPercentage_Implementation() const override;
    virtual bool IsDestroyed_Implementation() const override;
    virtual float GetMaxHealth_Implementation() const override;
    virtual float GetCurrentHealth_Implementation() const override;

    // ITargetable Interface
    virtual bool CanBeTargeted_Implementation() const override;
    virtual int32 GetTargetPriority_Implementation() const override;
    virtual FText GetTargetDisplayName_Implementation() const override;
    virtual UTexture2D* GetTargetIcon_Implementation() const override;
    virtual FVector GetAimPoint_Implementation() const override;
    virtual float GetTargetSignature_Implementation() const override;
    virtual float GetDistanceFromLocation_Implementation(FVector FromLocation) const override;
    virtual bool IsHostileToActor_Implementation(AActor* Observer) const override;

    // REMOVED: IFactionMember interface methods - faction system removed per Trade Simulator MVP

protected:
    virtual void BeginPlay() override;

    // REMOVED: OwningFaction - faction system removed per Trade Simulator MVP

    /** Current structural integrity (health) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status")
    float CurrentStructuralIntegrity;

    /** Maximum structural integrity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status")
    float MaxStructuralIntegrity;

    /** Flag indicating if station is destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Status")
    bool bIsDestroyed;

    /** Display name for this station */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station")
    FText StationName;
};
