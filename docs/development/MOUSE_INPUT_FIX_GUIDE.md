# Mouse Input Fix Guide

## Problem Summary

Mouse input was causing strafing instead of rotation due to conflicting Input Mapping Contexts.

## Root Cause

Two input mapping contexts were being activated simultaneously:

1. **Blueprint Context** (`Content/Input/IMC_Spaceship.uasset`):
   - Added by TestGameMode via InputConfigDataAsset
   - Priority: 0 (default)
   - **Had Mouse2D incorrectly mapped**

2. **Programmatic Context** (`IMC_SpaceshipControls`):
   - Created by SpaceshipControlsComponent in C++
   - Priority: 0 (default)
   - Maps Mouse2D → LookAction (CORRECT)

When both contexts have the same priority and conflicting mappings, the behavior is undefined.

## Solution

### Code Changes (Already Applied)

1. **Spaceship.cpp** - Conditional input binding:
   - Checks if ASpaceship has configured input actions
   - If yes: Uses ASpaceship's sophisticated handling
   - If no: Delegates to SpaceshipControlsComponent
   - Prevents both systems from binding simultaneously

2. **SpaceshipControlsComponent.cpp** - External context detection:
   - Detects if Blueprint IMC is already registered
   - Only adds programmatic context if no external context exists
   - Logs warning to check Blueprint mappings

3. **Added diagnostic logging** throughout input pipeline

### Blueprint Fix Required

**You MUST verify the Blueprint Input Mapping Context has correct key mappings:**

**File**: `Content/Input/IMC_Spaceship.uasset`

**Required Mappings**:

| Input | Key | Action | Modifiers |
|-------|-----|--------|-----------|
| **Movement** | | | |
| Forward | W | IA_Move | SwizzleAxis (YXZ) |
| Backward | S | IA_Move | SwizzleAxis (YXZ), Negate |
| Right | D | IA_Move | None |
| Left | A | IA_Move | Negate |
| **Look** | | | |
| **Mouse** | **Mouse2D** | **IA_Look** | **None** |
| **Other** | | | |
| Roll Right | E | IA_Roll | None |
| Roll Left | Q | IA_Roll | Negate |
| Fire | LeftMouseButton | IA_Fire_Primary | None |
| Speed | MouseWheelAxis | IA_Speed | None |

### How to Verify/Fix Blueprint Mapping

1. Open Unreal Editor
2. Navigate to `Content/Input/IMC_Spaceship`
3. Double-click to open Input Mapping Context editor
4. Find the **Mouse2D** mapping
5. **Verify it maps to `IA_Look` (NOT IA_Move)**
6. If it maps to IA_Move or has incorrect modifiers:
   - Delete the incorrect mapping
   - Add new mapping: Mouse2D → IA_Look
   - Do NOT add any modifiers to Mouse2D
7. Save the asset

### Testing

After fixing the Blueprint:

1. Launch the game
2. Check console logs for input binding messages:
   ```
   LogAdastreaInput: TestGameMode: Added spaceship mapping context with priority X
   LogAdastreaInput: SpaceshipControlsComponent: External mapping context detected...
   ```

3. Test mouse input:
   - **Left/Right mouse** → Ship should YAW (turn left/right)
   - **Up/Down mouse** → Ship should PITCH (nose up/down)
   - **WASD** → Ship should STRAFE (physical movement)

4. Check for these log messages when moving mouse:
   ```
   LogAdastreaInput: SpaceshipControlsComponent::HandleLook - RAW LookValue: X=... Y=...
   LogAdastreaInput: SpaceshipControlsComponent::OnLookInput - Applying rotation: Pitch=... Yaw=...
   ```

   OR (if using ASpaceship system):
   ```
   LogAdastreaInput: ASpaceship::Look - LookAxisVector: X=... Y=...
   LogAdastreaInput: ASpaceship::Turn - YawInput=...
   LogAdastreaInput: ASpaceship::LookUp - PitchInput=...
   ```

### Expected Behavior After Fix

| Input | Expected Result | Actual Result (Before Fix) |
|-------|-----------------|---------------------------|
| Mouse Left/Right | Yaw rotation (turn ship) | Strafing left/right ❌ |
| Mouse Up/Down | Pitch rotation (nose up/down) | Rotating (unclear) ❌ |
| W/S | Forward/Backward movement | (Assumed working) |
| A/D | Left/Right strafing | (Assumed working) |
| Q/E | Roll rotation | (Assumed working) |

After fix, all inputs should match the "Expected Result" column.

## Architecture Notes

### Input System Design

The Adastrea spaceship input system has two implementations:

1. **SpaceshipControlsComponent** (Basic):
   - Programmatically creates InputActions and MappingContext
   - Simpler, more straightforward
   - Good for quick prototyping
   - Self-contained input handling

2. **ASpaceship + Blueprint IMC** (Advanced):
   - Uses Blueprint-configured InputActions
   - References `InputConfigDataAsset`
   - More flexible and designer-friendly
   - Better integration with game modes
   - Supports multiple control schemes

**Current Recommendation**: Use the Blueprint IMC approach (Option 2) for production, as it's more flexible and integrates with TestGameMode's configuration system.

### Priority System

Enhanced Input System resolves conflicts using priority:
- **Higher priority = takes precedence**
- Default priority = 0
- If same priority, behavior is undefined

**Best Practice**: Use distinct priorities:
- UI/Menu contexts: Priority 10
- Game-specific contexts: Priority 0
- Fallback contexts: Priority -10

## Troubleshooting

### Issue: Mouse still causes strafing after Blueprint fix

**Check**:
1. Did you save the IMC_Spaceship asset after editing?
2. Did you restart the editor/PIE session?
3. Check console for this message:
   ```
   SpaceshipControlsComponent: External mapping context detected - NOT adding programmatic context
   ```
4. If message is missing, SpaceshipControlsComponent might still be adding conflicting context

**Solution**: Increase SpaceshipControlsComponent's `InputMappingPriority` property to 10 in Blueprint.

### Issue: No input working at all

**Check**:
1. Is TestGameMode active? Check logs for "TestGameMode: Added spaceship mapping context"
2. Is InputConfigDataAsset configured? Check TestGameMode's SelectedInputConfig
3. Are InputActions referenced correctly? Check IA_Move and IA_Look exist

**Solution**: Ensure TestGameMode has valid InputConfigDataAsset assigned with proper MappingContext.

### Issue: Logs show both ASpaceship and SpaceshipControlsComponent handling input

This means both systems are binding to the same action, causing double processing.

**Solution**: Ensure ASpaceship's `MoveAction` and `LookAction` properties are either:
- **Both NULL**: Uses SpaceshipControlsComponent only
- **Both VALID**: Uses ASpaceship system only (SpaceshipControlsComponent should detect this)

Never have one NULL and one valid - this creates asymmetric binding.

## Related Files

- `/Source/Adastrea/Private/Ships/Spaceship.cpp` - Main ship input handling
- `/Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp` - Component input handling
- `/Source/Adastrea/Private/TestGameMode.cpp` - Input config application
- `/Content/Input/IMC_Spaceship.uasset` - Blueprint mapping context (FIX HERE!)
- `/Content/Input/Actions/IA_Move.uasset` - Move input action
- `/Content/Input/Actions/IA_Look.uasset` - Look input action

## Summary

**Quick Fix**: Open `Content/Input/IMC_Spaceship`, ensure Mouse2D maps to IA_Look (not IA_Move), save, test.

**Long-term**: Decide on one input system (recommend Blueprint IMC approach) and properly configure priority levels to avoid conflicts.
