# Station Editor Migration Guide - Blueprint to C++

**Version**: 1.0  
**Last Updated**: December 7, 2025  
**Purpose**: Guide for migrating from Blueprint-based station editor opening logic to C++ implementation

---

## Overview

The station editor opening logic has been moved from Blueprints to C++ in `AAdastreaPlayerController`. This change simplifies Blueprint graphs and provides better performance and maintainability.

### What Changed

**Before (Blueprint-based):**
- Blueprint received `OnStationEditorToggle` event
- Blueprint found nearest station manually
- Blueprint created widget and initialized manager
- Blueprint handled input mode switching

**After (C++ Implementation):**
- All logic handled automatically in C++
- Only need to assign widget class and catalog in Blueprint
- Backward compatible with existing setups

---

## Migration Steps

### Step 1: Update Your Player Controller Blueprint

If you have a Blueprint derived from `AAdastreaPlayerController` (e.g., `BP_SpaceshipController`):

1. **Open your Player Controller Blueprint**

2. **Assign Configuration Properties** (if not already set):
   
   In the **Details** panel, find the "Station Editor" category:
   
   - **Station Editor Widget Class**: Set to your widget Blueprint (e.g., `WBP_StationEditor`)
   - **Module Catalog**: Set to your catalog data asset (e.g., `DA_StationModuleCatalog`)
   - **Station Search Radius**: Adjust if needed (default: 5000 units)

3. **Remove Old Blueprint Logic** (if you have it):
   
   - Find any nodes connected to the `OnStationEditorToggle` event
   - **DELETE** all nodes that:
     - Find nearest station
     - Create station editor widget
     - Initialize editor manager
     - Set input mode
   
   **Important**: The event still exists for backward compatibility, but you no longer need to handle it manually.

### Step 2: Verify Input Bindings

Ensure your Enhanced Input Action is properly bound:

1. **Open your Input Mapping Context** (e.g., `IMC_Spaceship`)

2. **Verify the Station Editor key binding** exists:
   - Input Action: `IA_OpenStationEditor` (or similar)
   - Key: N (default key binding, changed from F9 since F9 is used for screenshots)
   - Trigger: Press

3. **The input action should call**: `ToggleStationEditor` function
   - This is now handled automatically by the C++ implementation

---

## Configuration Reference

### AAdastreaPlayerController Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| **StationEditorWidgetClass** | TSubclassOf\<UStationEditorWidget\> | nullptr | The widget Blueprint class to use for the editor UI |
| **ModuleCatalog** | UStationModuleCatalog* | nullptr | Data asset containing available modules |
| **StationSearchRadius** | float | 5000.0f | Maximum distance to search for nearby stations |

### Required Setup

**Minimum Requirements:**
1. ✅ Set `StationEditorWidgetClass` to your widget Blueprint
2. ✅ Set `ModuleCatalog` to your catalog data asset
3. ✅ Input action bound to call `ToggleStationEditor()`

**Optional Customization:**
- Adjust `StationSearchRadius` for your game's scale
- Override `FindNearestStation()` in Blueprint for custom logic
- Listen to `OnStationEditorToggle` event if you need custom behavior

---

## What the C++ Implementation Does

When `ToggleStationEditor()` is called:

### Opening the Editor

1. **Validate State**: Checks if player is controlling a spaceship
2. **Find Station**: Searches for nearest station within radius using `FindNearestStation()`
3. **Create Widget**: Creates widget instance if it doesn't exist
4. **Initialize Manager**: 
   - Creates `UStationEditorManager` instance
   - Assigns the module catalog
   - Calls `BeginEditing()` with the found station
5. **Show UI**:
   - Adds widget to viewport
   - Switches to `FInputModeGameAndUI()`
   - Shows mouse cursor
6. **Log State**: Logs success/failure for debugging

### Closing the Editor

1. **Save Changes**: Calls `EditorManager->Save()` to commit changes
2. **Hide UI**:
   - Removes widget from viewport
   - Switches back to `FInputModeGameOnly()`
   - Hides mouse cursor
3. **Update State**: Sets `bIsStationEditorOpen = false`

---

## Error Handling

The C++ implementation includes comprehensive error checking:

### Common Warnings/Errors

**"StationEditorWidgetClass is not set!"**
- **Cause**: You forgot to assign the widget class in Blueprint
- **Fix**: Assign your `WBP_StationEditor` to the property

**"No ModuleCatalog assigned"**
- **Cause**: Module catalog property is not set
- **Fix**: Assign your `DA_StationModuleCatalog` data asset
- **Note**: This is a warning - editor will still open but module placement may not work

**"No station found within X units"**
- **Cause**: No station exists nearby, or search radius is too small
- **Fix**: 
  - Move closer to a station, or
  - Increase `StationSearchRadius`, or
  - Place a station in the level

**"Not controlling a spaceship"**
- **Cause**: Player is not currently possessing a spaceship
- **Fix**: Only allow opening editor when in a spaceship

---

## Advanced Customization

### Override Station Finding Logic

If you need custom logic to find stations (e.g., only stations you own):

```cpp
// In your Blueprint-derived PlayerController

UFUNCTION(BlueprintCallable, Category="Player|Station Editor")
ASpaceStation* FindNearestStation() override
{
    // Custom logic here
    // Example: Find only stations owned by player
    
    return NearestOwnedStation;
}
```

### Listen to Editor State Changes

You can still use the event for custom behavior:

```cpp
// In Blueprint Event Graph

// OnStationEditorToggle Event
// -> Check IsStationEditorOpen()
// -> Play sound, show tutorial, etc.
```

### Check Editor State

Use these functions to query editor state:

- `IsStationEditorOpen()` - Returns true if editor is visible
- `GetStationEditorWidget()` - Returns the widget instance (or nullptr)

---

## Testing Your Migration

### Checklist

- [ ] Player Controller has widget class assigned
- [ ] Player Controller has module catalog assigned
- [ ] Input action is bound correctly
- [ ] Can open editor near a station (press M or your key)
- [ ] Mouse appears and UI shows
- [ ] Can close editor (press M again)
- [ ] Mouse disappears and input returns to game
- [ ] Module placement works (requires catalog)
- [ ] Changes are saved when closing

### Troubleshooting

**Editor won't open:**
1. Check Output Log for error messages
2. Verify widget class is assigned in Blueprint
3. Verify you're near a station (within 5000 units)
4. Verify you're controlling a spaceship

**Widget appears but no modules shown:**
1. Check that `ModuleCatalog` is assigned
2. Verify catalog has module entries
3. Check that modules have valid classes assigned

**Input doesn't respond after closing:**
1. This should be handled automatically
2. Check for any Blueprint code that might override input mode
3. Verify `HideStationEditor()` is being called

---

## Benefits of C++ Implementation

### For Designers
- ✅ Simpler Blueprint graphs (less to maintain)
- ✅ Fewer nodes to connect manually
- ✅ Automatic error handling and validation
- ✅ Better performance (no Blueprint overhead)
- ✅ Consistent behavior across all projects

### For Programmers
- ✅ Single source of truth in C++
- ✅ Easier to debug and maintain
- ✅ Better compile-time checking
- ✅ More flexible for future enhancements
- ✅ Follows Unreal best practices

---

## Backward Compatibility

The `OnStationEditorToggle` event is **still broadcast** for backward compatibility:

- Existing Blueprints that listen to this event will still work
- Event fires when editor opens OR closes
- You can still add custom behavior by listening to this event
- Event will be marked deprecated in future versions

**Migration Timeline:**
- **Now**: Event is active, C++ logic is primary
- **Next Release**: Event marked as deprecated
- **Future Release**: Event may be removed (with plenty of warning)

---

## Example Blueprint Setup

### Before (Manual Setup - No Longer Needed)

```
OnStationEditorToggle Event
  -> Get All Actors of Class (SpaceStation)
  -> For Each Loop
  -> Get Distance To
  -> Select Nearest
  -> Create Widget (StationEditorWidget)
  -> Set Station
  -> Add to Viewport
  -> Set Input Mode UI Only
```

### After (Automatic - Just Configure Properties)

```
Class Defaults (Details Panel):
  Station Editor Widget Class: WBP_StationEditor
  Module Catalog: DA_StationModuleCatalog
  Station Search Radius: 5000.0

(Optional) OnStationEditorToggle Event
  -> Play Sound
  -> Show Tutorial Hint
```

**Result**: ~15 Blueprint nodes eliminated! ✨

---

## Support

If you encounter issues:

1. Check the Output Log for detailed error messages
2. Review this migration guide
3. See [StationEditorPIETestingGuide.md](StationEditorPIETestingGuide.md) for testing instructions
4. Refer to [StationEditorSystemGuide.md](StationEditorSystemGuide.md) for C++ API details

---

## Related Documentation

- [StationEditorSystemGuide.md](StationEditorSystemGuide.md) - Complete C++ API reference
- [StationEditorPIETestingGuide.md](StationEditorPIETestingGuide.md) - Testing in PIE
- [ENHANCED_INPUT_GUIDE.md](../ENHANCED_INPUT_GUIDE.md) - Enhanced Input System
- [BLUEPRINT_GUIDE_FOR_BEGINNERS.md](../BLUEPRINT_GUIDE_FOR_BEGINNERS.md) - Blueprint basics

---

**Last Updated**: December 7, 2025  
**Maintained by**: Adastrea Development Team
