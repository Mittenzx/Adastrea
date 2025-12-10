# BP_ImportAI Testing Checklist

This checklist guides you through testing the new AI ship implementation.

## Pre-Testing Setup

### ✅ Step 1: Verify Files Exist

- [ ] `Source/Adastrea/Ships/SimpleAIMovementComponent.h` exists
- [ ] `Source/Adastrea/Ships/SimpleAIMovementComponent.cpp` exists
- [ ] `CreateAIShip.py` exists in project root
- [ ] `BP_IMPORTAI_IMPLEMENTATION_GUIDE.md` exists
- [ ] `BP_IMPORTAI_QUICKSTART.md` exists

### ✅ Step 2: Build C++ Code

**Visual Studio:**
- [ ] Open `Adastrea.sln`
- [ ] Configuration set to **Development Editor**
- [ ] Build succeeds without errors
- [ ] No warnings related to SimpleAIMovementComponent

**OR Unreal Editor:**
- [ ] Opened Unreal Editor with project
- [ ] Tools → Compile succeeds
- [ ] Output Log shows successful compilation
- [ ] Editor restarts if prompted

### ✅ Step 3: Run Setup Script

- [ ] Opened Unreal Editor
- [ ] Opened Output Log (Window → Developer Tools → Output Log)
- [ ] Ran CreateAIShip.py script in Python console
- [ ] Script completed without errors
- [ ] BP_ImportAI created in `Content/Blueprints/Ships/`

### ✅ Step 4: Configure BP_ImportAI

- [ ] Opened BP_ImportAI in Blueprint Editor
- [ ] Added SimpleAIMovementComponent (Components panel → Add Component)
- [ ] Component visible in Components hierarchy
- [ ] Configured component properties:
  - [ ] Min Distance: 10000.0
  - [ ] Max Distance: 50000.0
  - [ ] Move Speed: 2000.0
  - [ ] Arrival Threshold: 500.0
  - [ ] Turn Rate: 90.0
  - [ ] Show Debug: TRUE (for testing)
  - [ ] Constrain To Horizontal Plane: TRUE
- [ ] Compiled successfully (no errors)
- [ ] Saved Blueprint

## Functional Testing

### ✅ Test 1: Basic Spawning

- [ ] Opened test level (or created new level)
- [ ] Dragged BP_ImportAI from Content Browser into viewport
- [ ] Ship placed successfully
- [ ] Ship has mesh visible

### ✅ Test 2: Auto-Possession

- [ ] Selected BP_ImportAI in World Outliner
- [ ] Opened Details panel
- [ ] Verified **Auto Possess AI** = "Placed in World or Spawned"
- [ ] Verified **AI Controller Class** = "AIController"

### ✅ Test 3: Basic Movement

- [ ] Pressed Alt+P (or Play button) to start PIE
- [ ] Ship starts moving (not stationary)
- [ ] Ship has visible green line to target (debug enabled)
- [ ] Yellow sphere visible at target location
- [ ] Distance text visible above ship

### ✅ Test 4: Target Arrival

- [ ] Watched ship fly toward target
- [ ] Ship arrived at yellow sphere location
- [ ] New target generated automatically
- [ ] Green line points to new location
- [ ] Ship turns and flies to new target

### ✅ Test 5: Continuous Operation

- [ ] Let ship run for 2-3 minutes
- [ ] Ship continues generating and reaching targets
- [ ] No crashes or errors in Output Log
- [ ] Movement remains smooth throughout

## Advanced Testing

### ✅ Test 6: Multiple Ships

- [ ] Added 3-5 BP_ImportAI instances to level
- [ ] Positioned at different locations
- [ ] Started PIE
- [ ] All ships move independently
- [ ] No collision or interference
- [ ] Performance acceptable (check FPS)

### ✅ Test 7: Parameter Adjustment

Test different component settings:

**Fast Ship:**
- [ ] Move Speed: 5000.0
- [ ] Turn Rate: 180.0
- [ ] Ship moves noticeably faster
- [ ] Turns more quickly

**Slow Ship:**
- [ ] Move Speed: 500.0
- [ ] Turn Rate: 30.0
- [ ] Ship moves slowly
- [ ] Turns gradually

**Short Distance:**
- [ ] Min Distance: 5000.0
- [ ] Max Distance: 10000.0
- [ ] Targets closer to ship
- [ ] More frequent target changes

**Long Distance:**
- [ ] Min Distance: 20000.0
- [ ] Max Distance: 100000.0
- [ ] Targets far from ship
- [ ] Longer flight times

### ✅ Test 8: Debug Visualization

- [ ] Set Show Debug to TRUE
- [ ] Verified green line visible
- [ ] Verified yellow sphere at target
- [ ] Verified distance text updates
- [ ] Set Show Debug to FALSE
- [ ] Verified debug visuals disappear
- [ ] Ship continues functioning normally

### ✅ Test 9: Horizontal Plane Constraint

- [ ] Set Constrain To Horizontal Plane to FALSE
- [ ] Ship can pitch up/down toward targets
- [ ] Set Constrain To Horizontal Plane to TRUE
- [ ] Ship stays level (yaw only)

## Edge Case Testing

### ✅ Test 10: Spawn Without Component

- [ ] Duplicate BP_ImportAI → BP_ImportAI_NoComp
- [ ] Remove SimpleAIMovementComponent
- [ ] Place in level and test
- [ ] Ship does NOT move (expected)
- [ ] No crashes or errors

### ✅ Test 11: Missing AI Controller

- [ ] Duplicate BP_ImportAI → BP_ImportAI_NoAI
- [ ] Set Auto Possess AI to "Disabled"
- [ ] Place in level and test
- [ ] Ship does NOT move (expected)
- [ ] No crashes or errors

### ✅ Test 12: Extreme Distance Values

- [ ] Set Min Distance: 1000.0
- [ ] Set Max Distance: 1000.0 (equal)
- [ ] Test - should work (validation handles this)
- [ ] Set Min Distance: 50000.0
- [ ] Set Max Distance: 10000.0 (inverted)
- [ ] Test - should work (validation fixes this)

## Performance Testing

### ✅ Test 13: Performance Baseline

- [ ] Placed 1 BP_ImportAI in level
- [ ] Started PIE
- [ ] Noted FPS in editor
- [ ] Checked CPU usage

### ✅ Test 14: Multiple Ships Performance

- [ ] Added 10 BP_ImportAI instances
- [ ] Started PIE
- [ ] Noted FPS
- [ ] Added 20 BP_ImportAI instances
- [ ] Started PIE
- [ ] Noted FPS
- [ ] Performance acceptable for intended use

### ✅ Test 15: Long-Running Stability

- [ ] Started PIE with 5 ships
- [ ] Let run for 10+ minutes
- [ ] No memory leaks (check Task Manager)
- [ ] No performance degradation
- [ ] No crashes

## Integration Testing

### ✅ Test 16: With Player Ship

- [ ] Added BP_ImportAI to level with player ship
- [ ] Started PIE as player
- [ ] AI ship moves independently
- [ ] Player can fly near AI ship
- [ ] No interference between ships

### ✅ Test 17: Save/Load Level

- [ ] Added BP_ImportAI to level
- [ ] Saved level
- [ ] Closed and reopened level
- [ ] BP_ImportAI still present
- [ ] Configuration preserved
- [ ] Test - ship still functions

### ✅ Test 18: Package Test (Optional)

If you have packaging setup:
- [ ] Packaged project
- [ ] Ran packaged build
- [ ] AI ships function in package
- [ ] Performance acceptable

## Logging and Debugging

### ✅ Test 19: Log Output

- [ ] Opened Output Log
- [ ] Filtered to "LogAdastreaAI"
- [ ] Started PIE with BP_ImportAI
- [ ] Verified "New target generated" messages appear
- [ ] Messages include ship name and target location
- [ ] No error messages

### ✅ Test 20: Component Properties

- [ ] Selected BP_ImportAI in level
- [ ] Opened Details panel
- [ ] Found SimpleAIMovementComponent
- [ ] All properties visible and editable
- [ ] Properties have proper ranges and tooltips

## Documentation Verification

### ✅ Test 21: Quick Start Guide

- [ ] Followed BP_IMPORTAI_QUICKSTART.md exactly
- [ ] All steps clear and accurate
- [ ] Successfully created AI ship
- [ ] No missing information

### ✅ Test 22: Implementation Guide

- [ ] Read BP_IMPORTAI_IMPLEMENTATION_GUIDE.md
- [ ] All technical details accurate
- [ ] Code samples correct
- [ ] Troubleshooting section helpful

## Issue Tracking

### Known Issues
Record any issues found during testing:

1. Issue: _______________________________________________
   - Severity: [ ] Critical [ ] High [ ] Medium [ ] Low
   - Reproducible: [ ] Always [ ] Sometimes [ ] Rarely
   - Description: ________________________________________
   - Workaround: _________________________________________

2. Issue: _______________________________________________
   - Severity: [ ] Critical [ ] High [ ] Medium [ ] Low
   - Reproducible: [ ] Always [ ] Sometimes [ ] Rarely
   - Description: ________________________________________
   - Workaround: _________________________________________

### Improvements Suggested
Ideas for future enhancements:

1. ___________________________________________________
2. ___________________________________________________
3. ___________________________________________________

## Final Verification

### ✅ Sign-Off

- [ ] All critical tests passed
- [ ] No blocking issues found
- [ ] Documentation accurate and complete
- [ ] Code follows Adastrea standards
- [ ] Ready for merge

**Tested By:** ___________________  
**Date:** ___________________  
**Build Version:** ___________________  
**Notes:** ___________________

---

## Test Results Summary

**Total Tests:** 22  
**Passed:** ___  
**Failed:** ___  
**Skipped:** ___  

**Overall Status:** [ ] PASS [ ] FAIL [ ] NEEDS WORK

---

**Last Updated:** 2025-12-10  
**Version:** 1.0
