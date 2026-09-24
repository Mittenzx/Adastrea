#include "Stations/SpaceStationModule.h"
#include "Stations/SpaceStation.h"
#include "AdastreaLog.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/ScopeLock.h"

// ============================================================================
// Per-class default module meshes
// ============================================================================
// Each native module class maps to (in priority order):
//   1. its own dedicated mesh  /AdastreaShips/Meshes/Station/SM_StationModule_<X>_01
//   2. an optional shared-kit mesh (the research-lab kit shared by all labs)
//   3. the shared shell matching its Content/Data/StationModuleBuilderData.json grid
//      footprint - Standard (2x2x1), Large (3x2x1, DockingBay), ConnectorThin
//      (1x1x1: Corridor/DockingPort/Turret), SolarArray (3x1x1) - with the module's
//      family material (Content/Materials/M_StationModule_*) on slot 0
//   4. /Engine/BasicShapes/Cube (base class, anything not in the table)
// Dedicated/kit meshes that aren't imported yet are probed with
// FPackageName::DoesPackageExist and loaded with LOAD_NoWarn|LOAD_Quiet, so
// not-yet-imported art costs nothing and logs nothing (ConstructorHelpers would log a
// CDO-constructor error per missing asset). Dedicated meshes carry their own slot
// materials; the family material is only applied on the shell fallback.
// To give a module its own mesh: import SM_StationModule_<X>_01 (see
// Tools/import_art_gap_assets.py) and restart the editor - no code change needed.
namespace
{
    enum class EModuleShell : uint8 { None, Standard, Large, ConnectorThin, SolarArray };
    enum class EModuleFamily : uint8 { None, Shell, Connector, Utility, Defence, SolarArray };

    struct FModuleMeshSpec
    {
        const TCHAR* ClassName;     // nearest NATIVE class name (no 'A' prefix)
        const TCHAR* DedicatedMesh; // short asset name under StationMeshDir
        const TCHAR* SharedKitMesh; // optional second choice, nullptr if none
        EModuleShell Shell;
        EModuleFamily Family;
    };

    const TCHAR* const StationMeshDir = TEXT("/AdastreaShips/Meshes/Station/");
    const TCHAR* const ResearchLabKit = TEXT("SM_StationModule_ResearchLab_01");

    const FModuleMeshSpec ModuleMeshSpecs[] = {
        // Docking / connectors
        { TEXT("DockingBayModule"),      TEXT("SM_StationModule_DockingBay_01"),      nullptr, EModuleShell::Large,         EModuleFamily::Connector },
        { TEXT("DockingPortModule"),     TEXT("SM_StationModule_DockingPort_01"),     nullptr, EModuleShell::ConnectorThin, EModuleFamily::Connector },
        { TEXT("CorridorModule"),        TEXT("SM_StationModule_Corridor_01"),        nullptr, EModuleShell::ConnectorThin, EModuleFamily::Connector },
        // Trade / habitation
        { TEXT("CargoBayModule"),        TEXT("SM_StationModule_CargoBay_01"),        nullptr, EModuleShell::Standard,      EModuleFamily::Utility },
        { TEXT("MarketplaceModule"),     TEXT("SM_StationModule_Market_01"),          nullptr, EModuleShell::Standard,      EModuleFamily::Shell },
        { TEXT("HabitationModule"),      TEXT("SM_StationModule_Habitation_01"),      nullptr, EModuleShell::Standard,      EModuleFamily::Shell },
        { TEXT("BarracksModule"),        TEXT("SM_StationModule_Barracks_01"),        nullptr, EModuleShell::Standard,      EModuleFamily::Defence },
        // Utility / industry
        { TEXT("ReactorModule"),         TEXT("SM_StationModule_Reactor_01"),         nullptr, EModuleShell::Standard,      EModuleFamily::Utility },
        { TEXT("SolarArrayModule"),      TEXT("SM_StationModule_SolarArray_01"),      nullptr, EModuleShell::SolarArray,    EModuleFamily::SolarArray },
        { TEXT("FuelDepotModule"),       TEXT("SM_StationModule_FuelDepot_01"),       nullptr, EModuleShell::Standard,      EModuleFamily::Utility },
        { TEXT("ProcessingModule"),      TEXT("SM_StationModule_Processing_01"),      nullptr, EModuleShell::Standard,      EModuleFamily::Utility },
        { TEXT("FabricationModule"),     TEXT("SM_StationModule_Fabrication_01"),     nullptr, EModuleShell::Standard,      EModuleFamily::Utility },
        // Defence
        { TEXT("TurretModule"),          TEXT("SM_StationModule_Turret_01"),          nullptr, EModuleShell::ConnectorThin, EModuleFamily::Defence },
        { TEXT("ShieldGeneratorModule"), TEXT("SM_StationModule_ShieldGenerator_01"), nullptr, EModuleShell::Standard,      EModuleFamily::Defence },
        // Research: base + tier-1 labs + the 7 tier-4 labs, all falling back to the shared lab kit
        { TEXT("ScienceLabModule"),      TEXT("SM_StationModule_ScienceLab_01"),           ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("ResearchLabModule"),     TEXT("SM_StationModule_ResearchLab_01"),          nullptr,        EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("PhysicsLabModule"),      TEXT("SM_StationModule_PhysicsLab_01"),           ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("MaterialsLabModule"),    TEXT("SM_StationModule_MaterialsLab_01"),         ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("ElectronicsLabModule"),  TEXT("SM_StationModule_ElectronicsLab_01"),       ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("WeaponsLabModule"),      TEXT("SM_StationModule_WeaponsLab_01"),           ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("BiologyLabModule"),      TEXT("SM_StationModule_BiologyLab_01"),           ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("ProjectileWeaponsLab"),  TEXT("SM_StationModule_ProjectileWeaponsLab_01"), ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("BeamWeaponsLab"),        TEXT("SM_StationModule_BeamWeaponsLab_01"),       ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("IonPropulsionLab"),      TEXT("SM_StationModule_IonPropulsionLab_01"),     ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("GravMaterialsLab"),      TEXT("SM_StationModule_GravMaterialsLab_01"),     ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("EncryptionLab"),         TEXT("SM_StationModule_EncryptionLab_01"),        ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("OptronicsLab"),          TEXT("SM_StationModule_OptronicsLab_01"),         ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
        { TEXT("CyberneticsLab"),        TEXT("SM_StationModule_CyberneticsLab_01"),       ResearchLabKit, EModuleShell::Standard, EModuleFamily::Shell },
    };

    const FModuleMeshSpec* FindModuleMeshSpec(const UClass* ModuleClass)
    {
        // Blueprint subclasses (BP_CargoBayModule_C, ...) resolve through their
        // nearest native parent, so a BP that doesn't override ModuleMesh still
        // gets its module's mesh.
        const UClass* NativeClass = ModuleClass;
        while (NativeClass && !NativeClass->HasAnyClassFlags(CLASS_Native))
        {
            NativeClass = NativeClass->GetSuperClass();
        }
        if (!NativeClass)
        {
            return nullptr;
        }
        const FString ClassName = NativeClass->GetName();
        for (const FModuleMeshSpec& Spec : ModuleMeshSpecs)
        {
            if (ClassName.Equals(Spec.ClassName))
            {
                return &Spec;
            }
        }
        return nullptr;
    }

    /** Load an optional station-module mesh by short name; nullptr (silently) if not imported yet. */
    UStaticMesh* LoadOptionalStationMesh(const TCHAR* ShortName)
    {
        if (!ShortName)
        {
            return nullptr;
        }
        const FString PackageName = FString(StationMeshDir) + ShortName;
        if (!FPackageName::DoesPackageExist(PackageName))
        {
            return nullptr;
        }
        const FString ObjectPath = PackageName + TEXT(".") + ShortName;
        return LoadObject<UStaticMesh>(nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
    }

    struct FResolvedModuleMesh
    {
        TWeakObjectPtr<UStaticMesh> Mesh;
        TWeakObjectPtr<UMaterialInterface> FamilyMaterial; // only set on the shell fallback
    };

    /** Does the actual (possibly loading) resolution for one class. Constructor-only. */
    FResolvedModuleMesh ResolveModuleMesh(const UClass* ModuleClass)
    {
        // These always exist (engine content + the committed shell kit/materials),
        // so ConstructorHelpers is fine; function statics = resolved once.
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
        static ConstructorHelpers::FObjectFinder<UStaticMesh> ShellStandardAsset(TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_Shell_Standard.SM_StationModule_Shell_Standard"));
        static ConstructorHelpers::FObjectFinder<UStaticMesh> ShellLargeAsset(TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_Shell_Large.SM_StationModule_Shell_Large"));
        static ConstructorHelpers::FObjectFinder<UStaticMesh> ShellConnectorThinAsset(TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_Shell_ConnectorThin.SM_StationModule_Shell_ConnectorThin"));
        static ConstructorHelpers::FObjectFinder<UStaticMesh> ShellSolarArrayAsset(TEXT("/AdastreaShips/Meshes/Station/SM_StationModule_Shell_SolarArray.SM_StationModule_Shell_SolarArray"));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatShell(TEXT("/Game/Materials/M_StationModule_Shell.M_StationModule_Shell"));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatConnector(TEXT("/Game/Materials/M_StationModule_Connector.M_StationModule_Connector"));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatUtility(TEXT("/Game/Materials/M_StationModule_Utility.M_StationModule_Utility"));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatDefence(TEXT("/Game/Materials/M_StationModule_Defence.M_StationModule_Defence"));
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatSolarArray(TEXT("/Game/Materials/M_StationModule_SolarArray.M_StationModule_SolarArray"));

        UStaticMesh* MeshToUse = CubeMeshAsset.Succeeded() ? CubeMeshAsset.Object : nullptr;
        UMaterialInterface* FamilyMaterial = nullptr;

        if (const FModuleMeshSpec* Spec = FindModuleMeshSpec(ModuleClass))
        {
            UStaticMesh* Dedicated = LoadOptionalStationMesh(Spec->DedicatedMesh);
            if (!Dedicated)
            {
                Dedicated = LoadOptionalStationMesh(Spec->SharedKitMesh);
            }

            if (Dedicated)
            {
                MeshToUse = Dedicated;
            }
            else
            {
                UStaticMesh* Shell = nullptr;
                switch (Spec->Shell)
                {
                case EModuleShell::Standard:      Shell = ShellStandardAsset.Object; break;
                case EModuleShell::Large:         Shell = ShellLargeAsset.Object; break;
                case EModuleShell::ConnectorThin: Shell = ShellConnectorThinAsset.Object; break;
                case EModuleShell::SolarArray:    Shell = ShellSolarArrayAsset.Object; break;
                default: break;
                }
                if (Shell)
                {
                    MeshToUse = Shell;
                    switch (Spec->Family)
                    {
                    case EModuleFamily::Shell:      FamilyMaterial = MatShell.Object; break;
                    case EModuleFamily::Connector:  FamilyMaterial = MatConnector.Object; break;
                    case EModuleFamily::Utility:    FamilyMaterial = MatUtility.Object; break;
                    case EModuleFamily::Defence:    FamilyMaterial = MatDefence.Object; break;
                    case EModuleFamily::SolarArray: FamilyMaterial = MatSolarArray.Object; break;
                    default: break;
                    }
                }
            }
        }

        FResolvedModuleMesh Result;
        Result.Mesh = MeshToUse;
        Result.FamilyMaterial = FamilyMaterial;
        return Result;
    }

    /**
     * Apply the class default mesh. Must run for EVERY construction, not just the
     * CDO: for native classes UE does not copy native-constructor-set subobject
     * properties from the CDO into instances (FObjectInitializer::InitProperties only
     * copies non-native state), so an instance whose constructor skips SetStaticMesh
     * ends up with no mesh. The resolution itself (package probe + load) happens once
     * per native class - normally while building its CDO on the game thread - and is
     * cached; later constructions (spawns, async level loads) only read the cache.
     */
    void ApplyDefaultModuleMesh(UStaticMeshComponent* MeshComponent, const UClass* ModuleClass)
    {
        if (!MeshComponent || !ModuleClass)
        {
            return;
        }

        const UClass* NativeClass = ModuleClass;
        while (NativeClass && !NativeClass->HasAnyClassFlags(CLASS_Native))
        {
            NativeClass = NativeClass->GetSuperClass();
        }
        const FName CacheKey = NativeClass ? NativeClass->GetFName() : NAME_None;

        static FCriticalSection CacheLock;
        static TMap<FName, FResolvedModuleMesh> Cache;

        FResolvedModuleMesh Resolved;
        bool bCached = false;
        {
            FScopeLock Lock(&CacheLock);
            if (const FResolvedModuleMesh* Found = Cache.Find(CacheKey))
            {
                Resolved = *Found;
                bCached = Resolved.Mesh.IsValid();
            }
        }
        if (!bCached)
        {
            if (!IsInGameThread())
            {
                // Never load from the async loading thread; the CDO (built on the game
                // thread at startup) populates the cache before any level instance.
                return;
            }
            Resolved = ResolveModuleMesh(ModuleClass);
            FScopeLock Lock(&CacheLock);
            Cache.Add(CacheKey, Resolved);
        }

        if (UStaticMesh* Mesh = Resolved.Mesh.Get())
        {
            MeshComponent->SetStaticMesh(Mesh);
        }
        if (UMaterialInterface* Mat = Resolved.FamilyMaterial.Get())
        {
            MeshComponent->SetMaterial(0, Mat);
        }
    }
}

ASpaceStationModule::ASpaceStationModule()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create static mesh component as root
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ModuleMesh"));
    RootComponent = MeshComponent;

    // Per-class default mesh (see ApplyDefaultModuleMesh above: resolved once per
    // class, applied on every construction). Blueprint subclasses that override
    // ModuleMesh still win - their overrides are applied after this constructor.
    ApplyDefaultModuleMesh(MeshComponent, GetClass());

    // Default values
    ModuleType = TEXT("Generic");
    ModulePower = 0.0f;
    ModuleGroup = EStationModuleGroup::Other;
    // REMOVED: ModuleFaction - faction system removed per Trade Simulator MVP

    // Note: Module integrity values are now set via default values in header
    // This allows Blueprint/editor customization without constructor override
    // Default values: CurrentModuleIntegrity = 1000.0f, MaxModuleIntegrity = 1000.0f
    
    bIsDestroyed = false;
}

// ====================
// IDamageable Interface Implementation
// ====================

float ASpaceStationModule::ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* DamageInstigator, AActor* DamageCauser)
{
    if (!CanTakeDamage_Implementation())
    {
        return 0.0f;
    }

    // Apply damage to module
    float ActualDamage = FMath::Min(Damage, CurrentModuleIntegrity);
    CurrentModuleIntegrity -= ActualDamage;

    UE_LOG(LogAdastreaStations, Log, TEXT("Module %s took %.1f damage from %s. Integrity: %.1f/%.1f"),
        *GetName(), ActualDamage,
        DamageInstigator ? *DamageInstigator->GetName() : TEXT("Unknown"),
        CurrentModuleIntegrity, MaxModuleIntegrity);

    // Check if module is destroyed
    if (CurrentModuleIntegrity <= 0.0f)
    {
        CurrentModuleIntegrity = 0.0f;
        bIsDestroyed = true;
        UE_LOG(LogAdastreaStations, Warning, TEXT("Module %s has been destroyed!"), *GetName());

        // Module destruction effects - basic implementation for MVP
        // TODO: [COMBAT][POST-MVP] Enhance for full combat system (spawn debris, propagate damage, etc.)
        // Implementation Status: Deferred until Phase 5 (Combat & Security Systems)
        // Current Trade Simulator MVP: Basic destruction logging only
        // Future Implementation:
        // - Debris particle system with physics simulation
        // - Damage propagation to adjacent modules (chain reactions)
        // - Emergency shutdown sequences with visual/audio effects
        // - Crew evacuation animations and notifications
        // - Station-wide alert system activation
        // - Faction reputation impact for destroying stations
        // Priority: Low - Post-MVP feature for visual polish and combat immersion
        // Dependencies: VFX system, audio system, physics simulation, debris spawning
        // Implementation Details:
        // 1. Create UDebrisSpawnerComponent for physics-based debris generation
        // 2. Implement damage propagation system using adjacency graphs
        // 3. Create emergency shutdown VFX/audio sequences
        // 4. Add crew evacuation AI behavior and animations
        // 5. Implement station-wide alert broadcast system
        // 6. Create faction reputation impact system
        // 7. Add salvageable debris with gameplay interaction
        // 8. Implement chain reaction mechanics for volatile modules
        
        // Disable module functionality
        OnModuleDestroyed();
        
        // For Trade Simulator MVP, we just log the destruction
        // Future combat system would:
        // - Spawn debris particles
        // - Propagate damage to parent station
        // - Remove from station's module list
        // - Trigger station-wide effects
    }

    return ActualDamage;
}

bool ASpaceStationModule::CanTakeDamage_Implementation() const
{
    // Cannot take damage if already destroyed
    if (bIsDestroyed)
    {
        return false;
    }

    return true;
}

float ASpaceStationModule::GetHealthPercentage_Implementation() const
{
    if (MaxModuleIntegrity <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentModuleIntegrity / MaxModuleIntegrity;
}

bool ASpaceStationModule::IsDestroyed_Implementation() const
{
    return bIsDestroyed;
}

float ASpaceStationModule::GetMaxHealth_Implementation() const
{
    return MaxModuleIntegrity;
}

float ASpaceStationModule::GetCurrentHealth_Implementation() const
{
    return CurrentModuleIntegrity;
}

// ====================
// ITargetable Interface Implementation
// ====================

bool ASpaceStationModule::CanBeTargeted_Implementation() const
{
    // Cannot target destroyed modules
    if (bIsDestroyed)
    {
        return false;
    }

    // Modules can be targeted for precise strikes
    return true;
}

int32 ASpaceStationModule::GetTargetPriority_Implementation() const
{
    // Module priority depends on type
    // Strategic modules (power, shields) have higher priority
    int32 Priority = 30;  // Base priority for modules

    switch (ModuleGroup)
    {
        case EStationModuleGroup::Power:
            Priority = 80;  // Critical - destroying power modules cripples station
            break;
        case EStationModuleGroup::Defence:
            Priority = 75;  // High - shields and weapons
            break;
        case EStationModuleGroup::Docking:
            Priority = 60;  // Medium-high - prevents reinforcements
            break;
        case EStationModuleGroup::Processing:
        case EStationModuleGroup::Storage:
            Priority = 40;  // Medium - economic damage
            break;
        case EStationModuleGroup::Habitation:
        case EStationModuleGroup::Public:
            Priority = 20;  // Low - civilian targets
            break;
        default:
            Priority = 30;
            break;
    }

    return FMath::Clamp(Priority, 0, 100);
}

FText ASpaceStationModule::GetTargetDisplayName_Implementation() const
{
    return FText::FromString(ModuleType);
}

UTexture2D* ASpaceStationModule::GetTargetIcon_Implementation() const
{
    // TODO: [UI][POST-MVP] Return module-specific icon based on ModuleGroup
    // Implementation Status: Deferred until UI polish phase
    // Current Trade Simulator MVP: Returns nullptr - UI uses default module icon
    // Future Implementation:
    // - ModuleGroup-specific icons (docking, power, storage, defense, etc.)
    // - ModuleType variations within groups (small/medium/large docking bays)
    // - Visual state indicators (operational, damaged, under construction)
    // - Integration with station management UI for module identification
    // Priority: Medium - Improves UI clarity and player targeting experience
    // Dependencies: UI asset system, icon design, module type definitions
    // Implementation Details:
    // 1. Create UModuleIconDataAsset class with icon sets for each ModuleGroup
    // 2. Add ModuleIconDataAsset property to ASpaceStationModule header
    // 3. Design 8-12 unique icons covering all ModuleGroup types
    // 4. Create icon variations for module states (operational/damaged/destroyed)
    // 5. Implement dynamic icon loading system with caching
    // 6. Add LOD system for icons at different zoom levels
    // 7. Create animated icons for special states (under construction, repairing)
    // 8. Integrate with station management UI for visual module identification
    // For Trade Simulator MVP, return nullptr - UI will use default icon
    // Future implementation: Load icon based on ModuleGroup or ModuleType
    
    // Example future implementation:
    // switch (ModuleGroup)
    // {
    //     case EStationModuleGroup::Docking: return DockingIcon;
    //     case EStationModuleGroup::Market: return MarketIcon;
    //     case EStationModuleGroup::Habitation: return HabitationIcon;
    //     default: return GenericModuleIcon;
    // }
    
    return nullptr;
}

FVector ASpaceStationModule::GetAimPoint_Implementation() const
{
    // Return center of module
    return GetActorLocation();
}

float ASpaceStationModule::GetTargetSignature_Implementation() const
{
    // Modules have varying signatures based on type
    switch (ModuleGroup)
    {
        case EStationModuleGroup::Power:
            return 1.5f;  // High energy signature
        case EStationModuleGroup::Defence:
            return 1.3f;  // Active systems
        default:
            return 1.0f;  // Normal signature
    }
}

float ASpaceStationModule::GetDistanceFromLocation_Implementation(FVector FromLocation) const
{
    return FVector::Dist(GetActorLocation(), FromLocation);
}

bool ASpaceStationModule::IsHostileToActor_Implementation(AActor* Observer) const
{
    if (!Observer)
    {
        return false;
    }

    // REMOVED: Faction-based hostility checks
    // MVP Trade Simulator doesn't have combat or faction relationships
    // All entities are neutral for trading purposes

    // Default to non-hostile
    return false;
}

void ASpaceStationModule::OnModuleDestroyed()
{
    // Basic module destruction handling for MVP
    // TODO: [COMBAT][POST-MVP] Enhance for full combat system
    // Implementation Status: Deferred until Phase 5 (Combat & Security Systems)
    // Current Trade Simulator MVP: Basic collision disabling only
    // Future Implementation:
    // - Emergency power shutdown with cascading effects
    // - Atmospheric venting particle effects
    // - Structural integrity warnings to adjacent modules
    // - Crew evacuation protocols and AI behavior
    // - Station-wide emergency broadcast system
    // - Salvageable debris generation for player interaction
    // Priority: Low - Post-MVP feature for visual polish and gameplay depth
    // Dependencies: VFX system, audio system, AI behavior, emergency systems
    // Implementation Details:
    // 1. Create UEmergencyShutdownComponent for power cascade effects
    // 2. Implement atmospheric venting VFX with particle systems
    // 3. Add structural integrity propagation system to adjacent modules
    // 4. Create crew evacuation AI behavior with navigation and animations
    // 5. Implement station-wide emergency broadcast with audio/visual alerts
    // 6. Add salvageable debris spawning with physics and gameplay interaction
    // 7. Create chain reaction system for volatile modules (fuel, weapons)
    // 8. Add environmental hazards (radiation leaks, fire spread, toxic gas)
    
    // Disable collision to prevent further interactions
    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Log detailed destruction info
    UE_LOG(LogAdastreaStations, Warning, TEXT("Module %s (%s) has been destroyed - functionality disabled"),
        *GetName(), *ModuleType);
    
    // Note: For Trade Simulator MVP, we don't need complex destruction effects
    // Future combat system would:
    // - Spawn particle effects
    // - Play destruction sounds
    // - Create debris actors
    // - Notify parent station
    // - Trigger gameplay events
}
