#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Stations/SpaceStationModule.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/ITargetable.h"
#include "Ships/ShipUpgradeDataAsset.h"
#include "SpaceStation.generated.h"

// Forward declarations
class AMarketplaceModule;
class ADockingBayModule;
class AOutfittingModule;
class UMarketDataAsset;
class UStationLayoutDataAsset;

/**
 * One module entry of a station blueprint string (see ASpaceStation::ExportBlueprintString).
 * Plain C++ (not reflected) - only the codec and its two callers (the Station Editor's
 * blueprint export/import and the save system) use it.
 */
struct FStationBlueprintEntry
{
    /** Native module class name ("CargoBayModule"), or a full class path for Blueprint modules */
    FString ItemID;

    /** Position in grid cells relative to the station origin (world axes) */
    FIntVector GridPos = FIntVector::ZeroValue;

    /** World yaw, normalised to 0/90/180/270 */
    int32 YawDegrees = 0;

    /** First module of the station (informational; Python plan-mode tool uses it) */
    bool bIsCore = false;
};

/** How a station treats stolen cargo (FCargoEntry::StolenFrom). */
UENUM(BlueprintType)
enum class EStationLaw : uint8
{
    /** From the sector: High/Medium security is Lawful, Low/None is a Fence; pirate-owned and black-market stations are Fences. */
    Auto,
    /** Customs scans ships as they dock: stolen goods are confiscated and fined. Its market won't buy them. */
    Lawful,
    /** No questions asked: the market buys stolen goods, at a discount. */
    Fence,
};

/** What a customs scan on docking found and did. */
struct FCustomsResult
{
    int32 UnitsConfiscated = 0;
    int32 Fine = 0;
    bool bScanned = false;
};

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
     * Default scene root. The base C++ class has no visual/collision components
     * of its own (those come from Blueprint subclasses like BP_SpaceStation), but
     * every station still needs a real root so GetActorLocation()/SetActorLocation()
     * mean something - without one, AActor::GetActorLocation() always returns the
     * zero vector, which breaks module placement math and station lookup by
     * distance (e.g. the builder's "spawn a fresh station from scratch" path).
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station")
    USceneComponent* StationRoot;

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
        // MODULE CONSTRUCTION & MANAGEMENT (Station Builder)
        // Re-enabled POST-MVP for station building gameplay. Consumed by the
        // in-game station builder (STATION_BUILDER.md) and ASpaceStation::BuildFromLayout.
        // ====================

        /**
         * Add a module to the station (simple version, position set later).
         * @param Module The module to add (also registered in Modules)
         */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        void AddModule(ASpaceStationModule* Module);

        /**
         * Add a module to the station at a specific location.
         * @param Module The module to add
         * @param RelativeLocation Position relative to the station (unreal-cm)
         * @return True if the module was successfully added
         */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        bool AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation);

        /**
         * Remove a module from the station.
         * @param Module The module to remove
         * @return True if found and removed
         */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        bool RemoveModule(ASpaceStationModule* Module);

        /**
         * Move a module to a new location within the station.
         * @param Module The module to move
         * @param NewRelativeLocation New position relative to the station
         * @return True if found and moved
         */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

        /** Get all modules of a specific type (e.g. "ReactorModule"). */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType) const;

        /** Number of attached modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Builder")
        int32 GetModuleCount() const;

        /**
         * Build the station from a layout data asset: spawns each module's subclass at
         * GridPos*GridSpacing, applies Rotation, registers them, and recomputes power /
         * docking / storage aggregates. The player's in-game builder commits a design
         * to this.
         * @param Layout The filled layout asset
         * @return Number of modules successfully spawned
         */
        UFUNCTION(BlueprintCallable, Category="Station|Builder")
        int32 BuildFromLayout(class UStationLayoutDataAsset* Layout);

        // ====================
        // BLUEPRINT STRING (STATION_BUILDER.md format)
        // SchemaVersion;PlotX,Y,Z;GridSpacing;ModuleID:ItemID:gx,gy,gz:rot:isCore;...
        // Shared by the Station Editor's blueprint export/import and the save system.
        // ====================

        /** Actor tag on stations spawned by the player's builder and modules the player placed. */
        static const FName PlayerBuiltTag;

        /**
         * Encode this station's modules as a blueprint string.
         * @param GridSpacing Grid cell size (cm) to quantise module positions to
         * @param bPlayerBuiltOnly Only include modules tagged PlayerBuiltTag
         */
        FString ExportBlueprintString(float GridSpacing, bool bPlayerBuiltOnly = false) const;

        /**
         * Spawn and attach every module in a blueprint string, tagged PlayerBuiltTag.
         * A direct reconstruction like BuildFromLayout: no credits or materials are
         * charged. Modules keep the exact world placement they were exported with.
         * @return Number of modules spawned
         */
        int32 BuildFromBlueprintString(const FString& Blueprint);

        /** Destroy every module tagged PlayerBuiltTag. @return Number removed */
        int32 RemovePlayerBuiltModules();

        /** Parse a blueprint string. Malformed module entries are skipped. @return false if the header is malformed */
        static bool ParseBlueprintString(const FString& Blueprint, float& OutGridSpacing, TArray<FStationBlueprintEntry>& OutEntries);

        /** Resolve a blueprint ItemID to a module class (bare native name or full class path). */
        static UClass* ResolveBlueprintModuleClass(const FString& ItemID);

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
    // SHIP OUTFITTING
    // ====================

    /** Every outfitting module (general bay and specialist workshops) attached to this station. */
    UFUNCTION(BlueprintCallable, Category="Station|Outfitting")
    TArray<AOutfittingModule*> GetOutfittingModules() const;

    /**
     * The module that fits upgrades of Category here: of those servicing it, the one with
     * the highest tier. Null if nothing on this station services the category.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Outfitting")
    AOutfittingModule* GetOutfittingModuleFor(EShipUpgradeCategory Category) const;

    /** Whether any working outfitting module is attached. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Capabilities")
    bool HasOutfitting() const;

    // ====================
    // AGGREGATE MODULE FUNCTIONALITY
    // Methods that combine functionality from all attached modules
    // ====================

    /**
     * The station's core (anchor) module, if it has one
     * @return The first AStationCoreModule among Modules, or nullptr
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Modules")
    class AStationCoreModule* GetCoreModule() const;

    /**
     * World bounds of the whole station: the actor plus every attached module.
     * AActor::GetActorBounds only covers the station actor, whose root carries no geometry.
     */
    FBox GetStationBounds() const;

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

        // ====================
        // MODULE ECONOMY / SERVICE AGGREGATES (Phase 1)
        // Totals across all attached modules of the relevant type. These surface
        // station-wide capability (storage, fuel, power output, shields, crew,
        // firepower) that HUD / trading / combat layers can read directly.
        // ====================

        /** Total cargo volume across all CargoBay modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Economy")
        int32 GetTotalStorageCapacity() const;

        /** Total cargo currently stored across all CargoBay modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Economy")
        int32 GetTotalCargoStored() const;

        /** Total fuel capacity (litres) across all FuelDepot modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Economy")
        float GetTotalFuelCapacity() const;

        /** Total fuel currently stored across all FuelDepot modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Economy")
        float GetTotalFuelStored() const;

        /** Effective power output from all online/undamaged Reactor modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Power")
        float GetTotalReactorOutput() const;

        /** Effective power output from all illuminated Solar Array modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Power")
        float GetTotalSolarOutput() const;

        /** Total shield strength (max) across all ShieldGenerator modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Defence")
        float GetTotalShieldStrength() const;

        /** Total current shield points across all ShieldGenerator modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Defence")
        float GetTotalCurrentShieldStrength() const;

        /** Combined DPS of all operational Turret modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Defence")
        float GetTotalTurretDps() const;

        /** Total crew berths across all Barracks modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Habitation")
        int32 GetTotalCrewCapacity() const;

        /** Total residents across all Habitation modules. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Habitation")
        int32 GetTotalResidents() const;

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status", meta=(ClampMin="0.0"))
    float CurrentStructuralIntegrity = 10000.0f;

    /** Maximum structural integrity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Status", meta=(ClampMin="0.0"))
    float MaxStructuralIntegrity = 10000.0f;

    /** Flag indicating if station is destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station Status")
    bool bIsDestroyed;

    /** If true the station cannot take any damage (safe zones, quest protection) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Defense")
    bool bIsInvulnerable = false;

    /** Whether station shields are currently active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Defense")
    bool bShieldsActive = false;

    /** Current shield strength; shields absorb damage before structural integrity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Defense", meta=(ClampMin="0.0"))
    float CurrentShieldStrength = 0.0f;

    /** Sensor signature multiplier for detection systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Sensors", meta=(ClampMin="0.1", ClampMax="10.0"))
    float SensorSignatureMultiplier = 2.0f;

    /** Display name for this station */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Station")
        FText StationName;

public:
    /**
     * The editor label, kept on save so packaged builds (which strip labels) still
     * know what the level designer called the station.
     */
    UPROPERTY(VisibleInstanceOnly, Category="Station")
    FString SavedLabel;

    /**
     * The name to show players: StationName unless it was left at the "Space Station"
     * default, else the editor label, else the class name made readable.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station")
    FString GetDisplayNameString() const;

    virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;

        /**
         * The market data asset for this station. If set, the station assigns it to
         * its marketplace modules during BeginPlay (overriding the module's own
         * default). This puts market specialization on the STATION actor so it
         * persists (per-module edits revert because modules are ChildActorComponents).
         */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Economy")
        UMarketDataAsset* StationMarket;

        /** Assign the given market to all marketplace modules on this station. */
        void ApplyStationMarket();

        /**
         * Raw resources are cheap and plentiful where they're found and dear where they
         * aren't: sets supply, demand and stock of the market's raw-material entries from
         * the galaxy (this sector's resources and fields, then the rest of its system).
         * Entries authored on StationMarket keep their hand-set values.
         */
        void ApplyRegionalSupply(class UMarketDataAsset* Market, const TArray<FName>& AuthoredItemIds) const;

        // ---- Stolen cargo (docs/11-TECHNICAL_SPECS/PIRACY_AND_LAW.md, step 3) ----

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Law")
        EStationLaw Law = EStationLaw::Auto;

        /** Share of the normal sell price a fence pays for stolen goods. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Law", meta=(ClampMin="0.0", ClampMax="1.0"))
        float FenceRate = 0.55f;

        /** Customs fine, as a share of the confiscated goods' base value. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Law", meta=(ClampMin="0.0"))
        float StolenGoodsFineRate = 0.5f;

        /** Law with Auto resolved from the sector's security and the station's owner. */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Law")
        EStationLaw GetEffectiveLaw() const;

        UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station|Law")
        bool BuysStolenGoods() const { return GetEffectiveLaw() == EStationLaw::Fence; }

        /**
         * Customs check as a ship docks (lawful stations only): stolen cargo is confiscated
         * and the ship's trader fined StolenGoodsFineRate of its value (as much as it can
         * pay). The fine goes to the station's owner.
         */
        FCustomsResult ScanDockedShip(class ASpaceship* Ship);
    };
