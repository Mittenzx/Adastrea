# Station Editor Circular Dependency Fix - Blueprint Update Guide

**Version**: 2.0  
**Last Updated**: December 9, 2025  
**Purpose**: Guide for updating Blueprints after circular dependency fix

---

## Overview

The Station Editor has been refactored to eliminate a circular dependency between the `Adastrea` and `StationEditor` modules. This fix maintains full functionality while improving the module architecture.

### What Changed

**Technical Changes:**
- `AdastreaPlayerController` no longer directly includes StationEditor headers
- Uses reflection (`FindFunction`, `ProcessEvent`) instead of direct type casts
- Works with base `UUserWidget` type instead of concrete `UStationEditorWidget` type

**Impact on Blueprints:**
- **Minimal**: Most Blueprints require no changes
- **WBP_StationEditor**: Must implement required custom events
- **Backward compatible**: Existing functionality preserved

---

## Do You Need to Update?

### ✅ No Changes Needed If:

- You're using a Blueprint-based station editor widget
- Your widget is properly set up with `EditorManager` and `ModuleCatalog`
- The editor opens and closes correctly

### ⚠️ Updates Needed If:

- You get errors like "Widget does not have SetStation function"
- The station editor doesn't open or initialize properly
- You created custom C++ code that depends on the old approach

---

## Required Blueprint Changes

### WBP_StationEditor Must Implement

Your station editor widget Blueprint **must** implement these **Custom Events**:

#### 1. SetStation (Custom Event)

**Purpose**: Initialize the editor with a station to edit

**Setup**:
1. Right-click in Event Graph → Add Custom Event
2. Name it: `SetStation`
3. Add Input Parameter:
   - Name: `Station`
   - Type: `Space Station` (object reference)

**Implementation**:
```
Event SetStation (Station: ASpaceStation*)
├─ Set CurrentStation = Station
├─ Branch: Is EditorManager Valid?
│  └─ True:
│     ├─ Set EditorManager->ModuleCatalog = (get from parent/self)
│     ├─ Call EditorManager->BeginEditing(Station)
│     └─ Branch: BeginEditing Success?
│        ├─ True: Call UpdateUI() / Initialize widgets
│        └─ False: Print Error / Show message to player
└─ Call any UI update functions
```

**Required Variables** (if not already present):
- `CurrentStation` (ASpaceStation object reference, Instance Editable, BlueprintReadWrite)
- `EditorManager` (UStationEditorManager object reference, BlueprintReadWrite)
- `ModuleCatalog` (UDataAsset object reference, BlueprintReadWrite)

#### 2. OnClose (Custom Event - Optional but Recommended)

**Purpose**: Clean up when the editor is closed

**Setup**:
1. Right-click in Event Graph → Add Custom Event
2. Name it: `OnClose`
3. No input parameters needed

**Implementation**:
```
Event OnClose
├─ Branch: Is EditorManager Valid?
│  └─ True:
│     ├─ Branch: Is EditorManager->bIsEditing True?
│     │  └─ True:
│     │     ├─ Call EditorManager->Save()
│     │     │  └─ Branch: Save Success?
│     │     │     ├─ True: Print "Changes saved" / Show confirmation
│     │     │     └─ False: Print "Save failed!" / Show error
│     │     └─ Call EditorManager->EndEditing()
└─ Set CurrentStation = nullptr
```

---

## Step-by-Step Migration

### Step 1: Check Your Widget Blueprint

Open `WBP_StationEditor` (or your custom station editor widget)

**Variables Check:**
- [ ] `CurrentStation` variable exists (type: ASpaceStation object reference)
- [ ] `EditorManager` variable exists (type: UStationEditorManager object reference)
- [ ] `ModuleCatalog` variable exists (type: UDataAsset object reference)

If missing, add them:
1. Click the + button in the Variables panel
2. Name them as above
3. Set their types correctly
4. Make them Instance Editable and BlueprintReadWrite

### Step 2: Implement SetStation Event

1. **Add Custom Event**:
   - Right-click in Event Graph
   - Select "Add Custom Event"
   - Name: `SetStation`

2. **Add Input Parameter**:
   - In the Details panel for the event
   - Click + next to Inputs
   - Name: `Station`
   - Type: `Space Station` (object reference)

3. **Connect Nodes**:
   ```
   SetStation Event
   │
   ├─> Set CurrentStation [Station parameter]
   │
   └─> Branch [Is EditorManager Valid?]
       │
       └─ True:
          ├─> Get ModuleCatalog (from self or parent)
          │   └─> Set EditorManager->ModuleCatalog
          │
          ├─> Call EditorManager->BeginEditing [Station parameter]
          │   └─> Branch [Return Value = True?]
          │       ├─ True: Continue with UI setup
          │       └─ False: Print "Failed to begin editing"
          │
          └─> Call your internal UI update functions
   ```

### Step 3: Implement OnClose Event (Optional)

1. **Add Custom Event**:
   - Right-click in Event Graph
   - Select "Add Custom Event"
   - Name: `OnClose`
   - No input parameters needed

2. **Connect Nodes**:
   ```
   OnClose Event
   │
   └─> Branch [Is EditorManager Valid?]
       │
       └─ True:
          └─> Branch [EditorManager->bIsEditing?]
              │
              └─ True:
                 ├─> Call EditorManager->Save()
                 │   └─> Branch [Return Value = True?]
                 │       ├─ True: Print "Changes saved successfully"
                 │       └─ False: Print "Failed to save changes!"
                 │
                 └─> Call EditorManager->EndEditing()
   ```

### Step 4: Update Event Construct (If Needed)

Ensure `EditorManager` is created in `Event Construct`:

```
Event Construct
│
└─> Branch [Is EditorManager Valid?]
    │
    └─ False: (EditorManager doesn't exist yet)
       └─> NewObject
           ├─ Outer: GetOwningPlayer() or suitable stable object
           ├─ Class: StationEditorManager
           └─> Set EditorManager [result]
```

**Note**: Use `GetOwningPlayer()` or another stable object as the outer instead of `Self` to ensure proper lifetime management. The widget may be destroyed and recreated, but the EditorManager should persist.

### Step 5: Test Your Changes

1. **Compile and Save** the Blueprint
2. **Set up PlayerController**:
   - Open `BP_AdastreaPlayerController`
   - Set `StationEditorWidgetClass` to `WBP_StationEditor`
   - Set `ModuleCatalog` to your catalog data asset
3. **Play in Editor**:
   - Fly near a station
   - Press M (or your configured key)
   - Verify editor opens
   - Test module placement
   - Close editor and verify it saves

---

## Troubleshooting

### "Widget does not have SetStation function"

**Cause**: Blueprint doesn't have the SetStation custom event  
**Solution**: Add the SetStation custom event as described in Step 2

**How to verify**:
1. Open your widget Blueprint
2. Look in Event Graph for "SetStation" custom event
3. If missing, add it following the instructions above

### "EditorManager is null"

**Cause**: EditorManager not created in Event Construct  
**Solution**: Add NewObject call in Event Construct (see Step 4)

**How to verify**:
1. Check Event Construct in your widget
2. Ensure EditorManager is created with NewObject
3. Verify it's assigned to the EditorManager variable

### "Module placement doesn't work"

**Cause**: ModuleCatalog not properly assigned  
**Solution**: 
1. Ensure ModuleCatalog property exists on your widget
2. In SetStation event, assign ModuleCatalog to EditorManager
3. Verify the catalog data asset has module entries

### "Changes don't save"

**Cause**: OnClose event not implemented or not calling Save()  
**Solution**: Implement OnClose event as described in Step 3

---

## Visual Example

### Before Migration (Old - may not work)

```
AdastreaPlayerController.cpp includes StationEditor headers
                    ↓
            Creates widget directly
                    ↓
            Casts to UStationEditorWidget
                    ↓
            Accesses EditorManager directly
                    ↓
            CIRCULAR DEPENDENCY! ❌
```

### After Migration (New - working)

```
AdastreaPlayerController.cpp uses reflection
                    ↓
            Calls FindFunction("SetStation")
                    ↓
            Calls ProcessEvent()
                    ↓
            Blueprint handles initialization
                    ↓
            NO CIRCULAR DEPENDENCY! ✅
```

---

## Benefits

### For the Project
- ✅ Builds successfully without linker errors
- ✅ Cleaner module architecture
- ✅ No circular dependencies
- ✅ Easier to maintain and extend

### For Designers
- ✅ Blueprint-first approach
- ✅ More flexibility in customization
- ✅ Can create multiple editor implementations
- ✅ Better debugging with Blueprint debugger

### For Programmers
- ✅ Follows Unreal best practices
- ✅ Better separation of concerns
- ✅ Easier to unit test
- ✅ More extensible architecture

---

## Advanced: Creating Custom Editors

You can now easily create alternate editor implementations:

```
WBP_SimpleEditor
├─ Implements: SetStation, OnClose
├─ Minimal UI for quick edits
└─ No 3D preview

WBP_AdvancedEditor
├─ Implements: SetStation, OnClose
├─ Full-featured UI
└─ 3D preview with camera controls

WBP_FactionEditor
├─ Implements: SetStation, OnClose
├─ Faction-specific theming
└─ Faction-based restrictions
```

Just change `StationEditorWidgetClass` in your PlayerController Blueprint!

---

## FAQ

**Q: Do I need to recompile C++?**  
A: No, this is a pure code fix. Just update your Blueprints.

**Q: Will my existing stations break?**  
A: No, station data is unchanged. Only the editor interface changed.

**Q: Can I still use the old approach?**  
A: No, the old approach had circular dependencies and wouldn't build. This new approach is required.

**Q: Is this backward compatible?**  
A: Yes, as long as you implement the required custom events in your Blueprint.

**Q: What if I have multiple editor widgets?**  
A: Each widget must implement SetStation and OnClose independently.

**Q: Can I override the behavior in C++?**  
A: Yes, but avoid creating new circular dependencies. Use interfaces instead.

---

## Related Documentation

- `.github/MODULE_DEPENDENCY_FIX.md` - Technical details of the fix
- `Assets/StationEditorMigrationGuide.md` - Original Blueprint to C++ migration
- `MODULE_DEPENDENCY_GUIDE.md` - Module architecture guidelines
- `STATION_EDITOR_CPP_IMPLEMENTATION.md` - C++ reference

---

## Support

If you encounter issues:

1. Review this guide carefully
2. Check the Output Log for detailed error messages
3. Verify all required events are implemented
4. Test in a clean level with a simple station

For technical questions:
- See `.github/MODULE_DEPENDENCY_FIX.md`
- Check the development Discord
- Review existing widget Blueprints in the project

---

**Last Updated**: December 9, 2025  
**Version**: 2.0  
**Maintained by**: Adastrea Development Team
