// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Input/InputConfigDataAsset.h"
#include "AdastreaLog.h"

UInputConfigDataAsset::UInputConfigDataAsset()
{
    SpaceshipMappingContext = nullptr;
    MenuMappingContext = nullptr;
    SpaceshipContextPriority = 0;
    MenuContextPriority = 1;

    // All input actions default to nullptr
    MoveAction = nullptr;
    LookAction = nullptr;
    BoostAction = nullptr;
    BrakeAction = nullptr;
    RollAction = nullptr;
    ThrottleUpAction = nullptr;
    ThrottleDownAction = nullptr;
    ToggleFlightAssistAction = nullptr;
    ToggleTravelModeAction = nullptr;

    FirePrimaryAction = nullptr;
    FireSecondaryAction = nullptr;
    TargetLockAction = nullptr;
    NextTargetAction = nullptr;
    PreviousTargetAction = nullptr;
    NearestTargetAction = nullptr;

    InteractAction = nullptr;
    DockAction = nullptr;
    BoardAction = nullptr;

    AutopilotAction = nullptr;
    SetWaypointAction = nullptr;
    ClearWaypointAction = nullptr;
    NextWaypointAction = nullptr;

    MenuAction = nullptr;
    MapAction = nullptr;
    InventoryAction = nullptr;
    QuestLogAction = nullptr;
    ScreenshotAction = nullptr;
    StationEditorAction = nullptr;

    CycleCameraAction = nullptr;
    ZoomInAction = nullptr;
    ZoomOutAction = nullptr;
    FreeLookAction = nullptr;
}

UInputMappingContext* UInputConfigDataAsset::GetSpaceshipMappingContext() const
{
    return SpaceshipMappingContext;
}

UInputMappingContext* UInputConfigDataAsset::GetMenuMappingContext() const
{
    return MenuMappingContext;
}

bool UInputConfigDataAsset::IsValid() const
{
    // Check essential input actions
    bool bHasEssentialActions = 
        MoveAction != nullptr &&
        LookAction != nullptr &&
        FirePrimaryAction != nullptr &&
        InteractAction != nullptr &&
        MenuAction != nullptr;

    bool bHasMappingContexts = 
        SpaceshipMappingContext != nullptr;

    return bHasEssentialActions && bHasMappingContexts;
}

TArray<FString> UInputConfigDataAsset::GetUnassignedActions() const
{
    TArray<FString> UnassignedActions;

    // Check movement actions
    if (!MoveAction) UnassignedActions.Add(TEXT("MoveAction"));
    if (!LookAction) UnassignedActions.Add(TEXT("LookAction"));
    if (!BoostAction) UnassignedActions.Add(TEXT("BoostAction"));
    if (!BrakeAction) UnassignedActions.Add(TEXT("BrakeAction"));
    if (!RollAction) UnassignedActions.Add(TEXT("RollAction"));
    if (!ThrottleUpAction) UnassignedActions.Add(TEXT("ThrottleUpAction"));
    if (!ThrottleDownAction) UnassignedActions.Add(TEXT("ThrottleDownAction"));
    if (!ToggleFlightAssistAction) UnassignedActions.Add(TEXT("ToggleFlightAssistAction"));
    if (!ToggleTravelModeAction) UnassignedActions.Add(TEXT("ToggleTravelModeAction"));

    // Check combat actions
    if (!FirePrimaryAction) UnassignedActions.Add(TEXT("FirePrimaryAction"));
    if (!FireSecondaryAction) UnassignedActions.Add(TEXT("FireSecondaryAction"));
    if (!TargetLockAction) UnassignedActions.Add(TEXT("TargetLockAction"));
    if (!NextTargetAction) UnassignedActions.Add(TEXT("NextTargetAction"));
    if (!PreviousTargetAction) UnassignedActions.Add(TEXT("PreviousTargetAction"));
    if (!NearestTargetAction) UnassignedActions.Add(TEXT("NearestTargetAction"));

    // Check interaction actions
    if (!InteractAction) UnassignedActions.Add(TEXT("InteractAction"));
    if (!DockAction) UnassignedActions.Add(TEXT("DockAction"));
    if (!BoardAction) UnassignedActions.Add(TEXT("BoardAction"));

    // Check navigation actions
    if (!AutopilotAction) UnassignedActions.Add(TEXT("AutopilotAction"));
    if (!SetWaypointAction) UnassignedActions.Add(TEXT("SetWaypointAction"));
    if (!ClearWaypointAction) UnassignedActions.Add(TEXT("ClearWaypointAction"));
    if (!NextWaypointAction) UnassignedActions.Add(TEXT("NextWaypointAction"));

    // Check system actions
    if (!MenuAction) UnassignedActions.Add(TEXT("MenuAction"));
    if (!MapAction) UnassignedActions.Add(TEXT("MapAction"));
    if (!InventoryAction) UnassignedActions.Add(TEXT("InventoryAction"));
    if (!QuestLogAction) UnassignedActions.Add(TEXT("QuestLogAction"));
    if (!ScreenshotAction) UnassignedActions.Add(TEXT("ScreenshotAction"));
    if (!StationEditorAction) UnassignedActions.Add(TEXT("StationEditorAction"));

    // Check camera actions
    if (!CycleCameraAction) UnassignedActions.Add(TEXT("CycleCameraAction"));
    if (!ZoomInAction) UnassignedActions.Add(TEXT("ZoomInAction"));
    if (!ZoomOutAction) UnassignedActions.Add(TEXT("ZoomOutAction"));
    if (!FreeLookAction) UnassignedActions.Add(TEXT("FreeLookAction"));

    // Check mapping contexts
    if (!SpaceshipMappingContext) UnassignedActions.Add(TEXT("SpaceshipMappingContext"));
    if (!MenuMappingContext) UnassignedActions.Add(TEXT("MenuMappingContext"));

    return UnassignedActions;
}
