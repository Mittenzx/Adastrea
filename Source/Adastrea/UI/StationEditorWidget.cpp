#include "UI/StationEditorWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Factions/FactionDataAsset.h"

void UStationEditorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Widget initialization
    // Blueprint can override this to setup UI elements
}

ASpaceStationModule* UStationEditorWidget::AddModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector RelativeLocation)
{
    if (!CurrentStation || !ModuleClass)
    {
        return nullptr;
    }

    // Spawn the module in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = CurrentStation;
    
    // Spawn at station location + relative offset
    FVector SpawnLocation = CurrentStation->GetActorLocation() + RelativeLocation;
    FRotator SpawnRotation = CurrentStation->GetActorRotation();
    
    ASpaceStationModule* NewModule = World->SpawnActor<ASpaceStationModule>(
        ModuleClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (NewModule)
    {
        // Add the module to the station at the specified location
        CurrentStation->AddModuleAtLocation(NewModule, RelativeLocation);
    }

    return NewModule;
}

bool UStationEditorWidget::RemoveModule(ASpaceStationModule* Module)
{
    if (!CurrentStation || !Module)
    {
        return false;
    }

    bool bSuccess = CurrentStation->RemoveModule(Module);
    
    if (bSuccess)
    {
        // Optionally destroy the module actor
        // Module->Destroy();
    }

    return bSuccess;
}

bool UStationEditorWidget::MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation)
{
    if (!CurrentStation || !Module)
    {
        return false;
    }

    return CurrentStation->MoveModule(Module, NewRelativeLocation);
}

TArray<ASpaceStationModule*> UStationEditorWidget::GetAllModules()
{
    if (!CurrentStation)
    {
        return TArray<ASpaceStationModule*>();
    }

    return CurrentStation->Modules;
}

void UStationEditorWidget::SetStation(ASpaceStation* Station)
{
    CurrentStation = Station;
}

bool UStationEditorWidget::IsValidPlacement(FVector Location)
{
    // Basic validation - can be extended in Blueprint
    // Check if location is within reasonable bounds
    float MaxDistance = 10000.0f; // Maximum distance from station center
    
    if (Location.Size() > MaxDistance)
    {
        return false;
    }

    // Additional validation can be added here:
    // - Check for collisions with existing modules
    // - Check for minimum spacing between modules
    // - Validate against station grid/snap points
    
    return true;
}

// ====================
// Faction Integration Hooks
// ====================

void UStationEditorWidget::SetStationFaction(UFactionDataAsset* NewFaction)
{
    if (CurrentStation)
    {
        CurrentStation->SetFaction(NewFaction);
    }
}

TArray<FString> UStationEditorWidget::GetAvailableModuleTypes() const
{
    TArray<FString> AvailableTypes;
    
    // Basic module types always available
    AvailableTypes.Add(TEXT("Docking"));
    AvailableTypes.Add(TEXT("Power"));
    AvailableTypes.Add(TEXT("Storage"));
    AvailableTypes.Add(TEXT("Habitation"));
    AvailableTypes.Add(TEXT("Connection"));
    
    // Advanced types depend on faction technology level
    if (CurrentStation && CurrentStation->GetFaction())
    {
        int32 TechLevel = CurrentStation->GetFaction()->TechnologyLevel;
        
        if (TechLevel >= 5)
        {
            AvailableTypes.Add(TEXT("Processing"));
        }
        
        if (TechLevel >= 7)
        {
            AvailableTypes.Add(TEXT("Defence"));
            AvailableTypes.Add(TEXT("Public"));
        }
    }
    
    return AvailableTypes;
}

bool UStationEditorWidget::CanAddModuleForFaction(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
    if (!CurrentStation || !ModuleClass)
    {
        return false;
    }
    
    UFactionDataAsset* Faction = CurrentStation->GetFaction();
    if (!Faction)
    {
        // If no faction is assigned, allow basic modules
        return true;
    }
    
    // Get default object to check module properties
    ASpaceStationModule* DefaultModule = ModuleClass->GetDefaultObject<ASpaceStationModule>();
    if (!DefaultModule)
    {
        return false;
    }
    
    // Check technology level requirements
    int32 TechLevel = Faction->TechnologyLevel;
    
    // Defence and Public modules require tech level 7+
    if (DefaultModule->ModuleGroup == EStationModuleGroup::Defence || 
        DefaultModule->ModuleGroup == EStationModuleGroup::Public)
    {
        if (TechLevel < 7)
        {
            return false;
        }
    }
    
    // Processing modules require tech level 5+
    if (DefaultModule->ModuleGroup == EStationModuleGroup::Processing)
    {
        if (TechLevel < 5)
        {
            return false;
        }
    }
    
    return true;
}
