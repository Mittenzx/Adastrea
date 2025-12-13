# Station Editor Key Binding Update Instructions

**Date**: December 13, 2025  
**Purpose**: Manual instructions for updating the Station Editor key binding from F9 to N in Unreal Engine Editor

---

## Overview

The Station Editor system has been updated to use the **N** key instead of **F9** to toggle the editor, since F9 is used for screenshots in Unreal Engine. All documentation and code comments have been updated. However, the actual key bindings in the Input Assets must be manually updated in the Unreal Editor since they are stored in binary `.uasset` files.

## What Was Changed

### Documentation & Code (Already Updated via PR)
- ✅ `Assets/StationEditorPIETestingGuide.md` - All F9 references changed to N
- ✅ `Assets/StationEditorMigrationGuide.md` - Updated key reference with explanation
- ✅ `Source/Adastrea/Public/Player/AdastreaPlayerController.h` - Updated comment
- ✅ `Source/Adastrea/Public/Input/InputConfigDataAsset.h` - Updated comment

### Binary Assets (Requires Manual Update in Unreal Editor)
- ⚠️ `Content/Input/Actions/IA_OpenStationEditor.uasset` - Input Action (verify configuration)
- ⚠️ `Content/Input/IMC_Spaceship.uasset` - Input Mapping Context (update key binding here)

---

## Step-by-Step Instructions

### Step 1: Open Unreal Engine Editor

1. Launch Unreal Engine 5.6
2. Open the **Adastrea** project (`Adastrea.uproject`)
3. Wait for the project to fully load

### Step 2: Update Input Mapping Context

1. **Navigate to Content Browser:**
   - Open the Content Browser (Ctrl+Space or Window → Content Browser)
   - Navigate to: `Content/Input/`

2. **Open IMC_Spaceship:**
   - Find and double-click: `IMC_Spaceship.uasset`
   - This opens the Input Mapping Context editor

3. **Locate IA_OpenStationEditor:**
   - Scroll through the mappings list
   - Find the entry for `IA_OpenStationEditor`
   - Look for the current key binding (should show F9)

4. **Change Key Binding:**
   - Click on the key binding field (where it shows F9)
   - Press the **N** key on your keyboard
   - The binding should now display: `N`
   - Verify there are no modifiers (Shift, Ctrl, Alt should be unchecked)

5. **Save Changes:**
   - Press **Ctrl+S** or click **File → Save**
   - Alternatively, click the **Save** button in the toolbar

### Step 3: Verify Input Action Configuration

1. **Open IA_OpenStationEditor:**
   - In Content Browser, navigate to: `Content/Input/Actions/`
   - Double-click: `IA_OpenStationEditor.uasset`

2. **Verify Settings:**
   - **Value Type**: Should be `Digital (bool)` or `Button`
   - **Trigger**: Should be set to **Pressed** (not Down)
   - **Consume Input**: Should be checked (✓)

3. **Save if needed:**
   - If you made any changes, save with Ctrl+S

### Step 4: Verify Input Config Data Asset (Optional)

1. **Navigate to Data Assets:**
   - In Content Browser, go to: `Content/DataAssets/Input/`
   - Open: `DA_InputConfig.uasset` (if it exists)

2. **Check Station Editor Action:**
   - Look for property: `StationEditorAction` or `OpenStationEditorAction`
   - Verify it references: `IA_OpenStationEditor`
   - No changes needed here (just verification)

### Step 5: Test in PIE (Play In Editor)

1. **Start PIE:**
   - Press **Alt+P** or click the **Play** button
   - Ensure you spawn in a spaceship

2. **Approach a Station:**
   - Fly your ship near a space station (within 5000 units)

3. **Test New Key Binding:**
   - Press the **N** key
   - The Station Editor UI should appear
   - If using F9, it should now take a screenshot instead

4. **Test Closing:**
   - Close the Station Editor UI (ESC or Close button)
   - Press **N** again to verify it toggles properly

5. **Stop PIE:**
   - Press **Esc** then **Stop** or press **Alt+P** again

---

## Troubleshooting

### Issue: N key doesn't open the editor

**Solutions:**
1. Verify the key binding was saved in `IMC_Spaceship`
2. Check that the Input Mapping Context is active (should be by default)
3. Ensure you're controlling a spaceship (not in third-person mode)
4. Check you're within 5000 units of a space station
5. Look at the Output Log for any error messages

### Issue: F9 still opens the editor

**Possible Causes:**
1. The key binding in `IMC_Spaceship` wasn't updated
2. The asset wasn't saved after making changes
3. Old cached data - try restarting the Editor

**Solutions:**
1. Re-open `IMC_Spaceship` and verify the key shows as `N`
2. Delete the `Saved/` and `Intermediate/` folders, then restart Editor
3. Right-click on `IMC_Spaceship` → Asset Actions → Reload

### Issue: Both F9 and N open the editor

**Cause:** Multiple key bindings exist for the same action

**Solution:**
1. Open `IMC_Spaceship`
2. Look for duplicate `IA_OpenStationEditor` entries
3. Remove the F9 binding, keep only the N binding
4. Save the asset

---

## Verification Checklist

After completing the update, verify:

- [ ] `IMC_Spaceship` shows `N` key for `IA_OpenStationEditor`
- [ ] No F9 binding exists for `IA_OpenStationEditor`
- [ ] `IA_OpenStationEditor` has correct trigger settings (Pressed, Digital)
- [ ] PIE test: N key opens Station Editor
- [ ] PIE test: F9 takes screenshot (default UE behavior)
- [ ] PIE test: ESC or Close button closes editor
- [ ] No console errors or warnings related to input

---

## Additional Notes

### Why N Key?

- **F9** is the default Unreal Engine screenshot hotkey
- Conflicts were reported by users
- **N** is easily reachable and not commonly bound
- Mnemonic: "**N**ear station editor" or "**N**avigate to editor"

### Blueprint Integration

No Blueprint changes are required. The input binding change is purely at the Input Action level. The C++ code in `AAdastreaPlayerController::ToggleStationEditor()` handles all the logic and will automatically respond to whichever key is bound to `IA_OpenStationEditor`.

### Custom Key Bindings

If you prefer a different key (not N), you can bind `IA_OpenStationEditor` to any key you want in `IMC_Spaceship`. Just avoid keys that conflict with:
- Flight controls (WASD, Space, Ctrl, Shift, Q, E)
- Common UE hotkeys (F9=Screenshot, F11=Fullscreen, etc.)
- Other game systems (M=Map, I=Inventory, Tab=Menu, etc.)

---

## Related Files

**Documentation (Updated in this PR):**
- `Assets/StationEditorPIETestingGuide.md`
- `Assets/StationEditorMigrationGuide.md`
- `Source/Adastrea/Public/Player/AdastreaPlayerController.h`
- `Source/Adastrea/Public/Input/InputConfigDataAsset.h`

**Binary Assets (Manual Update Required):**
- `Content/Input/IMC_Spaceship.uasset` ⚠️ **UPDATE THIS**
- `Content/Input/Actions/IA_OpenStationEditor.uasset` (verify only)

**Related Systems:**
- `Source/Adastrea/Player/AdastreaPlayerController.cpp` - Station Editor toggle logic
- `Content/UI/Stations/WBP_StationEditor.uasset` - Station Editor widget
- `Content/Blueprints/Controllers/BP_AdastreaPlayerController.uasset` - Player controller BP

---

**Questions or Issues?**

If you encounter problems with this update, please check:
1. Output Log in Unreal Editor for error messages
2. This instruction file for troubleshooting steps
3. The related documentation files listed above

---

**Maintained by**: Adastrea Development Team  
**Last Updated**: December 13, 2025
