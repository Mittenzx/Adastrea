# Mouse Look Controls - Validation Checklist

## Overview
This checklist guides testing of the mouse look sensitivity improvements for BP_Battleship.

## Changes Summary
- **LookSensitivity**: 1.0 → 50.0 (50x increase)
- **LookSensitivityVertical**: 2.0 → 50.0 (25x increase)
- **Max clamp range**: 10.0 → 100.0
- **Added**: Rotation pivot documentation

## Pre-Flight Checklist

### 1. Build Verification
- [ ] Project compiles without errors using Unreal Build Tool
- [ ] No warnings related to SpaceshipControlsComponent
- [ ] Blueprint BP_Battleship loads without errors

### 2. In-Editor Verification
- [ ] Open BP_Battleship in Blueprint Editor
- [ ] Verify SpaceshipControlsComponent is present
- [ ] Check default values:
  - LookSensitivity = 50.0
  - LookSensitivityVertical = 50.0
- [ ] Verify property clamps show range 0.1 - 100.0

## In-Game Testing

### Test 1: Horizontal (Yaw) Sensitivity
**Objective**: Verify horizontal mouse movement rotates ship smoothly

Steps:
1. Launch game with BP_Battleship
2. Move mouse left slowly
   - [ ] Ship yaws left smoothly
   - [ ] Rotation speed feels natural (not too slow/fast)
3. Move mouse right slowly
   - [ ] Ship yaws right smoothly
   - [ ] Rotation speed matches left movement
4. Move mouse left/right quickly
   - [ ] Ship responds immediately
   - [ ] No lag or stuttering

**Expected**: Ship rotates ~0.8°/frame at 60fps (noticeable, responsive)
**Pass Criteria**: All checkboxes above marked

### Test 2: Vertical (Pitch) Sensitivity
**Objective**: Verify vertical mouse movement rotates ship smoothly

Steps:
1. Move mouse up slowly
   - [ ] Ship pitches up smoothly
   - [ ] Rotation speed feels natural
2. Move mouse down slowly
   - [ ] Ship pitches down smoothly
   - [ ] Rotation speed matches up movement
3. Move mouse up/down quickly
   - [ ] Ship responds immediately
   - [ ] No lag or stuttering

**Expected**: Pitch sensitivity matches yaw (consistent feel)
**Pass Criteria**: All checkboxes above marked

### Test 3: Combined Movement
**Objective**: Verify diagonal mouse movement works correctly

Steps:
1. Move mouse diagonally (up-right)
   - [ ] Ship pitches up AND yaws right simultaneously
   - [ ] Movement feels smooth and natural
2. Circle mouse around screen center
   - [ ] Ship rotates in corresponding circular pattern
   - [ ] No gimbal lock or strange behavior
3. Figure-8 mouse movement
   - [ ] Ship follows figure-8 rotation path
   - [ ] Transitions are smooth

**Pass Criteria**: All checkboxes above marked

### Test 4: Rotation Pivot Center
**Objective**: Verify ship rotates around its center, not an external point

Steps:
1. Enable third-person camera view
2. Rotate ship (yaw left/right)
   - [ ] Ship rotates around its center point
   - [ ] No visible offset or wobble
3. Rotate ship (pitch up/down)
   - [ ] Ship rotates around its center point
   - [ ] No visible offset or wobble
4. Combined rotation
   - [ ] Center of rotation remains constant
   - [ ] Ship doesn't "orbit" an invisible point

**If FAIL**: See troubleshooting section below

### Test 5: Frame Rate Independence
**Objective**: Verify sensitivity works consistently at different frame rates

Test at 30fps:
- [ ] Mouse movement rotates ship smoothly
- [ ] Feels slightly slower but consistent

Test at 60fps:
- [ ] Mouse movement rotates ship smoothly
- [ ] Baseline performance

Test at 120fps:
- [ ] Mouse movement rotates ship smoothly
- [ ] Feels slightly faster but consistent

**Expected**: Rotation speed should be frame-rate independent (DeltaTime scaling)
**Pass Criteria**: Consistent behavior across all frame rates

### Test 6: Sensitivity Adjustment
**Objective**: Verify sensitivity can be adjusted in Blueprint

Steps:
1. Open BP_Battleship in editor
2. Change LookSensitivity to 25.0
   - [ ] Value accepts without error
   - [ ] In-game rotation is slower
3. Change LookSensitivity to 75.0
   - [ ] Value accepts without error
   - [ ] In-game rotation is faster
4. Set to extreme values (0.1, 100.0)
   - [ ] Both values work without crashes
   - [ ] Rotation responds appropriately

**Pass Criteria**: All values work as expected

### Test 7: Edge Cases
**Objective**: Verify no crashes or unexpected behavior

Test scenarios:
- [ ] Rapid mouse movement (shake mouse quickly)
- [ ] Leave mouse stationary - ship should stop rotating
- [ ] Alt-Tab out and back in - controls still work
- [ ] Minimize/maximize window - controls still work
- [ ] Change resolution - controls still work

**Pass Criteria**: No crashes, all scenarios work correctly

## Troubleshooting

### Issue: Rotation Appears Off-Center

**Symptoms**: Ship appears to rotate around a point outside its body

**Diagnosis**:
1. Open BP_Battleship in Blueprint Editor
2. Select the mesh component (StaticMesh or SkeletalMesh)
3. Check "Transform" → "Location" in Details panel

**Fix Option A: Adjust in Blueprint**
1. Calculate the mesh's center point visually
2. Adjust mesh location so center aligns with (0, 0, 0) relative to RootComponent
3. Example: If mesh is 1000 units long, move it -500 units on X axis
4. Compile, save, test

**Fix Option B: Re-export Mesh**
1. Open mesh in 3D software (Blender, Maya, etc.)
2. Adjust pivot point to ship's center of mass
3. Re-export FBX/OBJ
4. Reimport to Unreal Engine
5. Update BP_Battleship to use new mesh

**Verification**:
- Ship should rotate in-place around its visual center
- No "orbiting" or offset rotation

### Issue: Sensitivity Still Too Low/High

**If too low (still sluggish)**:
- Increase LookSensitivity beyond 50.0 (up to 100.0)
- Consider increasing TurnRate in Spaceship class

**If too high (too twitchy)**:
- Decrease LookSensitivity below 50.0 (min 0.1)
- Consider mouse DPI settings on player's system

### Issue: Vertical Movement Not Working

**Check**:
1. Verify LookSensitivityVertical is not 0.0
2. Check bInvertLookY setting - may need to toggle
3. Verify input bindings in Input Mapping Context
4. Check logs for "OnLookInput" messages

### Issue: Horizontal and Vertical Don't Match

**Should both be 50.0 now**:
1. Verify both properties in BP_Battleship details
2. If different, they may have been overridden in Blueprint
3. Reset to defaults or manually set both to 50.0

## Performance Metrics

### Expected Performance
- **CPU**: Negligible impact (same calculation, different constant)
- **Memory**: No change
- **FPS**: No measurable difference

### Monitoring
Watch for:
- [ ] FPS drops during rotation
- [ ] Input lag or stuttering
- [ ] Memory leaks during extended play

**All should remain stable**

## Sign-Off

### Code Review
- [ ] Changes reviewed by senior developer
- [ ] No security concerns identified
- [ ] Follows Unreal Engine best practices

### Build Verification
- [ ] Project compiles without errors
- [ ] No new warnings introduced
- [ ] All unit tests pass (if applicable)

### Functional Testing
- [ ] All test scenarios passed
- [ ] No regressions identified
- [ ] Edge cases handled correctly

### Documentation
- [ ] Fix documentation complete (mouse-look-sensitivity-fix.md)
- [ ] Validation checklist complete (this file)
- [ ] Known issues documented

## Approval

**Tested by**: ________________  
**Date**: ________________  
**Build**: ________________  

**Result**: ☐ PASS | ☐ FAIL (see notes)

**Notes**:
_______________________________________________
_______________________________________________
_______________________________________________

## Post-Launch Monitoring

### First 24 Hours
- [ ] Monitor player feedback on Discord/forums
- [ ] Check for crash reports related to controls
- [ ] Review analytics for control-related metrics

### First Week
- [ ] Gather sensitivity preference data
- [ ] Adjust defaults if needed based on feedback
- [ ] Consider adding sensitivity presets (Low/Medium/High)

### Known Limitations
1. Sensitivity is frame-rate independent but feel may vary slightly
2. Very high mouse DPI may require lower sensitivity settings
3. Rotation pivot requires correct Blueprint/mesh configuration

---

**Version**: 1.0  
**Last Updated**: 2026-01-28  
**Maintained by**: Adastrea Development Team
