#include "Stations/SpaceStationModule.h"
#include "Stations/SpaceStation.h"
#include "AdastreaLog.h"
#include "UObject/ConstructorHelpers.h"

ASpaceStationModule::ASpaceStationModule()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create static mesh component as root
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ModuleMesh"));
    RootComponent = MeshComponent;

    // Load the cube mesh from engine basic shapes
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Default values
    ModuleType = TEXT("Generic");
    ModulePower = 0.0f;
    ModuleGroup = EStationModuleGroup::Other;
    // REMOVED: ModuleFaction - faction system removed per Trade Simulator MVP

    // Initialize health/integrity values
    CurrentModuleIntegrity = 1000.0f;  // Modules are less durable than full stations
    MaxModuleIntegrity = 1000.0f;
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
