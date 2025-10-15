#include "UI/StationEditorWidget.h"
#include "Kismet/GameplayStatics.h"

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
        // Add the module to the station
        CurrentStation->AddModule(NewModule, RelativeLocation);
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
