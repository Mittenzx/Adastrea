#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "SpaceStation.generated.h"

// Forward declarations
class AMarketplaceModule;
class ADockingBayModule;

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
     * Array of currently attached modules (runtime tracking)
     * 
     * This array is populated automatically in BeginPlay by discovering
     * any editor-placed modules (Child Actor Components or directly placed module actors).
     * 
     * Do not edit this in Class Defaults - it's for runtime use only.
     * 
     * To add modules at design-time:
     * - Add Child Actor Components in the Blueprint editor's Components panel
     * - Set Child Actor Class to your module Blueprint (e.g., BP_SpaceStationModule_DockingBay)
     * - Position modules visually in the viewport
     * 
     * Modules are automatically discovered and registered when the station spawns.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station",
        meta=(ToolTip="Currently attached modules (auto-populated from editor-placed components)"))
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

    /**
     * Get the first docking bay module attached to this station
     * @return First docking bay module found, or nullptr if none exist
     * 
     * MVP USE: Essential for accessing docking facilities
     */
    UFUNCTION(BlueprintCallable, Category="Station|Docking")
    ADockingBayModule* GetDockingBayModule() const;

    /**
     * Get all docking bay modules attached to this station
     * @return Array of all docking bay modules
     * 
     * MVP USE: Supports stations with multiple docking facilities
     */
    UFUNCTION(BlueprintCallable, Category="Station|Docking")
    TArray<ADockingBayModule*> GetDockingBayModules() const;

    /**
     * Get total number of docking points across all docking bay modules
     * @return Sum of all docking points from all docking bays
     * 
     * MVP USE: Display total station docking capacity in UI
     * Shows at design-time in editor when modules are added
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Docking")
    int32 GetTotalDockingPoints() const;

    /**
     * Get total maximum docking capacity across all docking bay modules
     * @return Sum of MaxDockedShips from all docking bays
     * 
     * MVP USE: Display station's maximum simultaneous docking capacity
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Docking")
    int32 GetTotalDockingCapacity() const;

    /**
     * Get number of open marketplaces on this station
     * @return Count of marketplaces that are currently open for trading
     * 
     * MVP USE: Display number of active trading facilities
     * Shows at design-time in editor when modules are added
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Trading")
    int32 GetOpenMarketplaceCount() const;

    /**
     * Get total number of marketplace modules on this station
     * @return Count of all marketplace modules (open or closed)
     * 
     * MVP USE: Display total trading facility count
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Trading")
    int32 GetTotalMarketplaceCount() const;

    /**
     * Get names of all marketplaces on this station
     * @return Array of marketplace display names
     * 
     * MVP USE: List available markets in UI
     */
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    TArray<FText> GetMarketplaceNames() const;

    // ====================
    // AGGREGATE MODULE FUNCTIONALITY
    // Methods that combine functionality from all attached modules
    // ====================

    /**
     * Get total power consumption/generation from all modules
     * Positive = station consumes power, Negative = station generates power
     * @return Net power consumption (sum of all module power values)
     * 
     * MVP USE: Can show station power status in UI
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Modules")
    float GetTotalPowerConsumption() const;

    /**
     * Get total power generation from all power-generating modules
     * @return Total power generation (absolute value of negative power modules)
     * 
     * MVP USE: Show power generation capacity
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Modules")
    float GetTotalPowerGeneration() const;

    /**
     * Get net power balance (generation - consumption)
     * Positive = surplus, Negative = deficit
     * @return Net power balance
     * 
     * MVP USE: Indicate if station has enough power
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Modules")
    float GetPowerBalance() const;

    /**
     * Check if station has at least one docking facility
     * @return True if station can dock ships
     * 
     * MVP USE: Essential - stations need docking for player interaction
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Capabilities")
    bool HasDockingCapability() const;

    /**
     * Check if station has at least one marketplace for trading
     * @return True if station can trade goods
     * 
     * MVP USE: Critical - identifies which stations allow trading
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Capabilities")
    bool HasMarketplace() const;

    /**
     * Check if station has cargo storage capability
     * @return True if station has cargo bay modules
     * 
     * MVP USE: Useful for showing station storage capacity
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Capabilities")
    bool HasCargoStorage() const;

    /**
     * Get count of modules in a specific group
     * @param ModuleGroup The group to count (Docking, Storage, Public, etc.)
     * @return Number of modules in that group
     * 
     * MVP USE: Show station facilities breakdown in UI
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Modules")
    int32 GetModuleCountByGroup(EStationModuleGroup ModuleGroup) const;

    /**
     * Get all modules in a specific group
     * @param ModuleGroup The group to filter by
     * @return Array of modules in that group
     * 
     * MVP USE: Query specific facility types
     */
    UFUNCTION(BlueprintCallable, Category="Station|Modules")
    TArray<ASpaceStationModule*> GetModulesByGroup(EStationModuleGroup ModuleGroup) const;

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
