#include "Stations/SpaceStationModule.h"
#include "Factions/FactionDataAsset.h"
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
    ModuleFaction = nullptr;
    
    // Initialize health/integrity values
    CurrentModuleIntegrity = 1000.0f;  // Modules are less durable than full stations
    MaxModuleIntegrity = 1000.0f;
    bIsDestroyed = false;
}

UFactionDataAsset* ASpaceStationModule::GetModuleFaction() const
{
    return ModuleFaction;
}

void ASpaceStationModule::SetModuleFaction(UFactionDataAsset* NewFaction)
{
    ModuleFaction = NewFaction;
}

// ====================
// IDamageable Interface Implementation
// ====================

float ASpaceStationModule::ApplyDamage_Implementation(float Damage, EDamageType DamageType, AActor* Instigator, AActor* DamageCauser)
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
        Instigator ? *Instigator->GetName() : TEXT("Unknown"),
        CurrentModuleIntegrity, MaxModuleIntegrity);

    // Check if module is destroyed
    if (CurrentModuleIntegrity <= 0.0f)
    {
        CurrentModuleIntegrity = 0.0f;
        bIsDestroyed = true;
        UE_LOG(LogAdastreaStations, Warning, TEXT("Module %s has been destroyed!"), *GetName());
        
        // TODO: Trigger module destruction effects
        // - Disable module functionality
        // - Spawn debris
        // - Propagate damage to parent station
        // - Remove from station's module list
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
    // TODO: Return module-specific icon based on ModuleGroup
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

    // Check if module belongs to a station
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor && ParentActor->Implements<UFactionMember>())
    {
        // Use parent station's faction relationship
        if (Observer->Implements<UFactionMember>())
        {
            IFactionMember* ParentFaction = Cast<IFactionMember>(ParentActor);
            return ParentFaction->Execute_IsHostileTo(ParentActor, TScriptInterface<IFactionMember>(Observer));
        }
    }

    // If module has its own faction, check that
    if (ModuleFaction && Observer->Implements<UFactionMember>())
    {
        UFactionDataAsset* ObserverFaction = IFactionMember::Execute_GetFaction(Observer);
        if (ObserverFaction)
        {
            // TODO: Query faction relationship
            // For now, consider hostile if different factions
            return ModuleFaction != ObserverFaction;
        }
    }

    // Default to non-hostile
    return false;
}
