# Station Editor - Complete Implementation Guide

## Problem Statement

The Station Editor currently displays the module list correctly, but clicking the "Build" button does nothing visible. There is no interactive placement preview in the center 3D viewport, and modules cannot be placed.

## Root Cause Analysis

Looking at the current implementation in `StationEditorWidgetCpp.cpp`:

1. **Build Button Click Handler**: When a module is selected, `PlaceModuleAtCursor()` is called immediately
2. **No Preview Mode**: The preview system exists but isn't being shown to the user
3. **Direct Placement**: The code attempts to place modules immediately via line trace, bypassing interactive placement
4. **Missing Interaction**: No mouse tracking in the 3D viewport to show placement preview

## Architecture Overview

The Station Editor has three main systems that need to work together:

```
┌─────────────────────┐
│  StationEditorWidget │  ← UI Layer (Left Panel)
│   - Module List     │
│   - Statistics      │
└──────────┬──────────┘
           │
           ↓ Manages
┌──────────────────────┐
│ StationEditorManager │  ← Logic Layer
│   - Validation       │
│   - Placement        │
│   - Preview Control  │
└──────────┬───────────┘
           │
           ↓ Controls
┌──────────────────────┐
│ StationBuildPreview  │  ← Visual Layer (Center 3D View)
│   - Ghost Preview    │
│   - Validity Color   │
└──────────────────────┘
```

## Solution: Two-Step Placement System

### Step 1: Enter Preview Mode (Build Button)
When user clicks a module's "Build" button:
1. Store selected module class
2. Show the preview actor in 3D space
3. Enable mouse tracking to update preview position
4. Change cursor to indicate placement mode

### Step 2: Confirm Placement (3D View Click)
When user clicks in the 3D viewport:
1. Validate placement at preview position
2. Spawn actual module if valid
3. Hide preview
4. Exit placement mode

## Implementation Changes Needed

### 1. Modify StationEditorWidgetCpp.h

Add new state tracking and methods:

```cpp
// In UStationEditorWidgetCpp class, add:

protected:
    // =====================
    // Placement Mode State
    // =====================
    
    /** Whether we're currently in placement mode */
    UPROPERTY()
    bool bIsInPlacementMode = false;
    
    /** Module class selected for placement */
    UPROPERTY()
    TSubclassOf<ASpaceStationModule> PendingPlacementModule;
    
    /**
     * Enter placement mode with a module
     * @param ModuleClass The module to preview
     */
    void EnterPlacementMode(TSubclassOf<ASpaceStationModule> ModuleClass);
    
    /**
     * Exit placement mode
     */
    void ExitPlacementMode();
    
    /**
     * Update preview position to follow cursor
     */
    void UpdatePreviewPosition();
    
    /**
     * Handle click in 3D viewport to confirm placement
     */
    UFUNCTION()
    void OnViewportClicked();
    
    /**
     * Handle right-click to cancel placement
     */
    UFUNCTION()
    void OnViewportRightClicked();
```

### 2. Modify StationEditorWidgetCpp.cpp

#### A. Change OnModuleButtonClicked

Replace the immediate placement with preview mode:

```cpp
void UStationEditorWidgetCpp::OnModuleButtonClicked(TSubclassOf<ASpaceStationModule> ModuleClass)
{
    if (!ModuleClass)
    {
        return;
    }
    
    // Enter placement mode instead of placing immediately
    EnterPlacementMode(ModuleClass);
}
```

#### B. Implement EnterPlacementMode

```cpp
void UStationEditorWidgetCpp::EnterPlacementMode(TSubclassOf<ASpaceStationModule> ModuleClass)
{
    if (!EditorManager || !ModuleClass)
    {
        return;
    }
    
    // Exit any existing placement mode
    if (bIsInPlacementMode)
    {
        ExitPlacementMode();
    }
    
    // Store selected module
    PendingPlacementModule = ModuleClass;
    bIsInPlacementMode = true;
    
    // Show preview with this module
    EditorManager->ShowPreview(ModuleClass);
    
    UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Entered placement mode for %s"), 
        *ModuleClass->GetName());
}
```

#### C. Implement ExitPlacementMode

```cpp
void UStationEditorWidgetCpp::ExitPlacementMode()
{
    if (!EditorManager)
    {
        return;
    }
    
    bIsInPlacementMode = false;
    PendingPlacementModule = nullptr;
    
    // Hide preview
    EditorManager->HidePreview();
    
    UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Exited placement mode"));
}
```

#### D. Update NativeTick to Track Mouse

```cpp
void UStationEditorWidgetCpp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Update construction progress
    if (EditorManager)
    {
        EditorManager->UpdateConstruction(InDeltaTime);
    }
    
    // Update preview position if in placement mode
    if (bIsInPlacementMode)
    {
        UpdatePreviewPosition();
    }
}
```

#### E. Implement UpdatePreviewPosition

```cpp
void UStationEditorWidgetCpp::UpdatePreviewPosition()
{
    if (!EditorManager || !CurrentStation)
    {
        return;
    }
    
    // Get cursor world position
    FVector WorldPosition, WorldDirection;
    if (!GetCursorWorldPosition(WorldPosition, WorldDirection))
    {
        return;
    }
    
    // Line trace to find placement location
    FHitResult HitResult;
    FVector TraceStart = WorldPosition;
    FVector TraceEnd = WorldPosition + WorldDirection * MaxTraceDistance;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwningPlayerPawn());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Update preview position
        EditorManager->UpdatePreview(HitResult.Location, FRotator::ZeroRotator);
        
        // Check if placement is valid at this location
        if (PendingPlacementModule)
        {
            EModulePlacementResult ValidationResult = EditorManager->CanPlaceModule(
                PendingPlacementModule,
                HitResult.Location,
                FRotator::ZeroRotator
            );
            
            // Update preview validity color
            bool bIsValid = (ValidationResult == EModulePlacementResult::Success);
            if (EditorManager->PreviewActor)
            {
                EditorManager->PreviewActor->SetValid(bIsValid);
            }
        }
    }
}
```

#### F. Replace PlaceModuleAtCursor

Keep the function but rename and change it to work from preview:

```cpp
void UStationEditorWidgetCpp::OnViewportClicked()
{
    if (!bIsInPlacementMode || !EditorManager || !PendingPlacementModule || !CurrentStation)
    {
        return;
    }
    
    // Get preview position
    FVector PlacementPosition = FVector::ZeroVector;
    FRotator PlacementRotation = FRotator::ZeroRotator;
    
    if (EditorManager->PreviewActor)
    {
        PlacementPosition = EditorManager->PreviewActor->GetActorLocation();
        PlacementRotation = EditorManager->PreviewActor->GetActorRotation();
    }
    
    // Validate one more time before placement
    EModulePlacementResult ValidationResult = EditorManager->CanPlaceModule(
        PendingPlacementModule,
        PlacementPosition,
        PlacementRotation
    );
    
    if (ValidationResult != EModulePlacementResult::Success)
    {
        UE_LOG(LogAdastreaStations, Warning, TEXT("Station Editor: Cannot place module: %d"), 
            static_cast<int32>(ValidationResult));
        return;
    }
    
    // Place the module
    ASpaceStationModule* PlacedModule = EditorManager->PlaceModule(
        PendingPlacementModule,
        PlacementPosition,
        PlacementRotation
    );
    
    if (PlacedModule)
    {
        UE_LOG(LogAdastreaStations, Log, TEXT("Station Editor: Placed module %s at %s"),
            *PlacedModule->GetName(), *PlacementPosition.ToString());
        
        // Exit placement mode
        ExitPlacementMode();
    }
}

void UStationEditorWidgetCpp::OnViewportRightClicked()
{
    // Cancel placement on right-click
    if (bIsInPlacementMode)
    {
        ExitPlacementMode();
    }
}
```

### 3. Blueprint Input Binding

The click events need to be bound in the Blueprint. Create these in your WBP_StationEditor:

#### Option A: Override NativeOnMouseButtonDown (Recommended)

Add to StationEditorWidgetCpp.h:

```cpp
protected:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
```

Add to StationEditorWidgetCpp.cpp:

```cpp
FReply UStationEditorWidgetCpp::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    
    if (bIsInPlacementMode)
    {
        if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
        {
            OnViewportClicked();
            return FReply::Handled();
        }
        else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
        {
            OnViewportRightClicked();
            return FReply::Handled();
        }
    }
    
    return Reply;
}
```

#### Option B: Use Input Action (Alternative)

If you prefer input actions, create:
1. `IA_StationEditorPlace` - Input Action for left-click
2. `IA_StationEditorCancel` - Input Action for right-click or ESC

Then bind these in your PlayerController when the editor is open.

### 4. Visual Feedback Enhancements

#### A. Update Close Button Handler

```cpp
void UStationEditorWidgetCpp::OnCloseButtonClicked()
{
    // Exit placement mode if active
    if (bIsInPlacementMode)
    {
        ExitPlacementMode();
    }
    
    // Save and end editing
    if (EditorManager)
    {
        EditorManager->Save();
    }
    
    // Remove from parent and restore input
    RemoveFromParent();
    
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}
```

#### B. Add Visual Indicators in Blueprint

In your WBP_StationEditor Blueprint, add:

1. **Placement Mode Indicator**: A text block that shows "Click to place, Right-click to cancel" when `bIsInPlacementMode` is true
2. **Selected Module Display**: Show the currently selected module name
3. **Cursor Change**: Change the cursor icon to a crosshair when in placement mode

### 5. Testing Checklist

Once implemented, test the following workflow:

1. **Open Editor**:
   - [ ] Press F9 near a station
   - [ ] Editor opens with module list visible
   - [ ] Statistics display correctly

2. **Enter Placement Mode**:
   - [ ] Click "Build" on a module in the list
   - [ ] Preview ghost appears in 3D view
   - [ ] Preview follows mouse cursor
   - [ ] Preview shows green for valid placement

3. **Invalid Placement**:
   - [ ] Move cursor to invalid location (too far, collision)
   - [ ] Preview turns red
   - [ ] Clicking does nothing (no module placed)

4. **Valid Placement**:
   - [ ] Move cursor to valid location near station
   - [ ] Preview is green
   - [ ] Click to place
   - [ ] Module spawns at preview location
   - [ ] Preview disappears
   - [ ] Statistics update (power, module count)

5. **Cancel Placement**:
   - [ ] Click "Build" on a module
   - [ ] Preview appears
   - [ ] Right-click or press ESC
   - [ ] Preview disappears
   - [ ] Can select another module

6. **Multiple Placements**:
   - [ ] Place first module
   - [ ] Click another module's "Build" button
   - [ ] Preview switches to new module
   - [ ] Can place multiple modules in sequence

## Troubleshooting

### Preview Not Visible

**Problem**: Preview actor doesn't appear in viewport.

**Solutions**:
1. Check preview actor is spawned in BeginEditing
2. Verify `EditorManager->ShowPreview()` is called
3. Check preview actor's mesh is set correctly
4. Ensure preview materials are assigned
5. Verify actor isn't hidden or too far from camera

### Preview Not Following Cursor

**Problem**: Preview stays in one location.

**Solutions**:
1. Verify `NativeTick` is calling `UpdatePreviewPosition()`
2. Check `GetCursorWorldPosition()` is returning true
3. Ensure line trace is hitting something (try tracing against a plane)
4. Check `MaxTraceDistance` is large enough (default 10000)

### Click Not Placing Module

**Problem**: Clicking in viewport doesn't place module.

**Solutions**:
1. Verify `bIsInPlacementMode` is true
2. Check `OnViewportClicked()` is being called (add log)
3. Ensure `CanPlaceModule()` returns Success
4. Verify `PlaceModule()` is being called
5. Check for null EditorManager or CurrentStation

### Module Placed at Wrong Location

**Problem**: Module spawns far from preview or in wrong spot.

**Solutions**:
1. Check preview actor's position before placement
2. Verify coordinate system (world vs. local)
3. Ensure station's location is considered in placement
4. Check grid snapping is working correctly

### Performance Issues

**Problem**: UI or preview is laggy.

**Solutions**:
1. Only trace when in placement mode (already implemented)
2. Reduce trace frequency (every N frames instead of every tick)
3. Use simpler preview mesh
4. Optimize collision checking

## Advanced Features (Optional)

Once basic placement works, consider adding:

### 1. Rotation Controls
- Q/E keys to rotate preview 90 degrees
- Mouse wheel for fine rotation
- Snap rotation to cardinal directions

### 2. Grid Visualization
- Show grid lines in viewport when in placement mode
- Highlight snap points near cursor
- Display grid size setting

### 3. Multiple Selection
- Shift+Click to place multiple instances
- Stay in placement mode after placing

### 4. Undo/Redo
- Ctrl+Z to undo last placement
- Already implemented in EditorManager
- Just need UI buttons

### 5. Module Categories
- Filter module list by category
- Favorite modules
- Recently used modules

### 6. Build Queue Mode
- Click multiple times to queue builds
- Each click adds to queue instead of immediate placement
- Show queued positions with numbered markers

## Code Summary

Here's a quick reference of all the changes needed:

### Files to Modify:
1. `Source/StationEditor/UI/StationEditorWidgetCpp.h`
   - Add placement mode state variables
   - Add new method declarations

2. `Source/StationEditor/UI/StationEditorWidgetCpp.cpp`
   - Modify `OnModuleButtonClicked()`
   - Add `EnterPlacementMode()`
   - Add `ExitPlacementMode()`
   - Modify `NativeTick()`
   - Add `UpdatePreviewPosition()`
   - Add `OnViewportClicked()`
   - Add `OnViewportRightClicked()`
   - Add `NativeOnMouseButtonDown()` override

### Key Changes:
- **Before**: Immediate placement via line trace
- **After**: Two-step preview → confirm workflow
- **Preview**: Constantly updated to follow cursor
- **Validation**: Visual feedback via color (green/red)
- **Placement**: Only on explicit click with valid location

## Next Steps

1. **Implement Core Changes**: Add the code modifications listed above
2. **Test Basic Flow**: Verify preview appears and follows cursor
3. **Test Placement**: Confirm clicking places modules correctly
4. **Add Visual Polish**: Placement mode indicator, cursor changes
5. **Document for Designers**: Create Blueprint guide for customization

## Additional Resources

- See `StationEditorManager.h` for all available validation functions
- See `StationBuildPreview.h` for preview customization options
- See `StationGridSystem.h` for grid snapping configuration
- Check `STATION_EDITOR_MOCKUP.html` for UI design reference

## Support

If you encounter issues:
1. Check Unreal Engine's Output Log for error messages
2. Add UE_LOG statements to trace execution flow
3. Use Visual Studio debugger to inspect state
4. Verify all widget names match exactly (case-sensitive)
5. Ensure module catalog has valid entries with meshes

---

**Status**: Complete implementation guide for interactive module placement  
**Last Updated**: 2025-12-14  
**Version**: 1.0
