#include "UI/StationEditorWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Factions/FactionDataAsset.h"

void UStationEditorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Widget initialization
    // Blueprint can override this to setup UI elements
    EnsureEditorManager();
}

void UStationEditorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Update construction progress
    if (EditorManager)
    {
        EditorManager->UpdateConstruction(InDeltaTime);
    }
}

void UStationEditorWidget::EnsureEditorManager()
{
    if (!EditorManager)
    {
        EditorManager = NewObject<UStationEditorManager>(this);
    }
}

ASpaceStationModule* UStationEditorWidget::AddModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector RelativeLocation)
{
    if (!CurrentStation || !ModuleClass)
    {
        return nullptr;
    }

    EnsureEditorManager();
    
    // Use the manager if available
    if (EditorManager && EditorManager->bIsEditing)
    {
        FVector WorldPosition = CurrentStation->GetActorLocation() + RelativeLocation;
        return EditorManager->PlaceModule(ModuleClass, WorldPosition, CurrentStation->GetActorRotation());
    }

    // Fallback to direct placement
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

    EnsureEditorManager();
    
    // Use the manager if available
    if (EditorManager && EditorManager->bIsEditing)
    {
        return EditorManager->RemoveModule(Module);
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

    EnsureEditorManager();
    
    // Use the manager if available
    if (EditorManager && EditorManager->bIsEditing)
    {
        FVector WorldPosition = CurrentStation->GetActorLocation() + NewRelativeLocation;
        return EditorManager->MoveModule(Module, WorldPosition);
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
    
    EnsureEditorManager();
    
    if (EditorManager && Station)
    {
        EditorManager->BeginEditing(Station);
    }
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

// ====================
// Undo/Redo
// ====================

bool UStationEditorWidget::Undo()
{
    EnsureEditorManager();
    return EditorManager ? EditorManager->Undo() : false;
}

bool UStationEditorWidget::Redo()
{
    EnsureEditorManager();
    return EditorManager ? EditorManager->Redo() : false;
}

bool UStationEditorWidget::CanUndo() const
{
    return EditorManager ? EditorManager->CanUndo() : false;
}

bool UStationEditorWidget::CanRedo() const
{
    return EditorManager ? EditorManager->CanRedo() : false;
}

// ====================
// Construction Queue
// ====================

int32 UStationEditorWidget::QueueConstruction(TSubclassOf<ASpaceStationModule> ModuleClass, FVector Position)
{
    EnsureEditorManager();
    if (EditorManager && CurrentStation)
    {
        FVector WorldPosition = CurrentStation->GetActorLocation() + Position;
        return EditorManager->QueueConstruction(ModuleClass, WorldPosition, FRotator::ZeroRotator);
    }
    return -1;
}

bool UStationEditorWidget::CancelConstruction(int32 QueueId)
{
    EnsureEditorManager();
    return EditorManager ? EditorManager->CancelConstruction(QueueId) : false;
}

TArray<FConstructionQueueItem> UStationEditorWidget::GetConstructionQueue() const
{
    return EditorManager ? EditorManager->GetConstructionQueue() : TArray<FConstructionQueueItem>();
}

// ====================
// Statistics
// ====================

FStationStatistics UStationEditorWidget::GetStationStatistics() const
{
    return EditorManager ? EditorManager->GetStationStatistics() : FStationStatistics();
}

float UStationEditorWidget::GetPowerBalance() const
{
    return EditorManager ? EditorManager->GetPowerBalance() : 0.0f;
}

float UStationEditorWidget::GetDefenseRating() const
{
    return EditorManager ? EditorManager->GetDefenseRating() : 0.0f;
}

// ====================
// Connections
// ====================

TArray<FModuleConnection> UStationEditorWidget::GetAllConnections() const
{
    return EditorManager ? EditorManager->GetAllConnections() : TArray<FModuleConnection>();
}

TArray<FModuleConnection> UStationEditorWidget::GetModuleConnections(ASpaceStationModule* Module) const
{
    return EditorManager ? EditorManager->GetModuleConnections(Module) : TArray<FModuleConnection>();
}

// ====================
// Notifications
// ====================

TArray<FStationNotification> UStationEditorWidget::GetNotifications() const
{
    return EditorManager ? EditorManager->GetNotifications() : TArray<FStationNotification>();
}

int32 UStationEditorWidget::GetUnreadNotificationCount() const
{
    return EditorManager ? EditorManager->GetUnreadNotificationCount() : 0;
}

void UStationEditorWidget::MarkNotificationRead(int32 NotificationId)
{
    if (EditorManager)
    {
        EditorManager->MarkNotificationRead(NotificationId);
    }
}

// ====================
// View Mode
// ====================

void UStationEditorWidget::SetViewMode(EStationEditorViewMode NewMode)
{
    EnsureEditorManager();
    if (EditorManager)
    {
        EditorManager->SetViewMode(NewMode);
    }
}

EStationEditorViewMode UStationEditorWidget::GetViewMode() const
{
    return EditorManager ? EditorManager->GetViewMode() : EStationEditorViewMode::Edit;
}

// ====================
// Module Upgrade
// ====================

bool UStationEditorWidget::CanUpgradeModule(ASpaceStationModule* Module) const
{
    return EditorManager ? EditorManager->CanUpgradeModule(Module) : false;
}

bool UStationEditorWidget::UpgradeModule(ASpaceStationModule* Module)
{
    EnsureEditorManager();
    return EditorManager ? EditorManager->UpgradeModule(Module) : false;
}
