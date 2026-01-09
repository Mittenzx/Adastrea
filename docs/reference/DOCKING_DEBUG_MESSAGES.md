# Docking System Debug Messages Reference

**Last Updated**: January 9, 2026  
**Purpose**: Debug messages for troubleshooting docking and trading issues  
**Applies To**: Trade Simulator MVP

---

## 🎯 Overview

This document lists all debug messages displayed on-screen during the docking and trading process. These messages help identify exactly where the docking system stops or fails.

### Color Coding

Debug messages use color-coded categories:

| Color | Meaning | Example |
|-------|---------|---------|
| **Green** | ✓ Success - Operation completed | "Ship docked successfully" |
| **Cyan** | ℹ️ Info - Function called | "RequestDocking() called" |
| **Yellow** | ⚠️ Warning - Unexpected state | "Already docking - ignoring input" |
| **Red** | ❌ Error - Operation failed | "No station in range" |

---

## 📍 Message Flow - Normal Docking Sequence

### Phase 1: Approaching Station

#### Message 1: Entering Docking Range
```
[DOCKING] Ship entered docking range of station: [Station Name]
```
- **Color**: Green
- **Trigger**: Ship enters station's docking range collision sphere
- **Function**: `ASpaceship::SetNearbyStation()`
- **What it means**: The ship is now close enough to request docking

#### Message 2: Docking Prompt Created
```
[DOCKING] WBP_DockingPrompt created and added to viewport
```
- **Color**: Green
- **Trigger**: First time near a station, widget is created
- **Function**: `ASpaceship::ShowDockingPrompt()`
- **What it means**: "Press F to Dock" UI should be visible

#### Message 3: Docking Prompt Visible
```
[DOCKING] Docking prompt now visible
```
- **Color**: Cyan
- **Trigger**: Each time ship enters docking range
- **Function**: `ASpaceship::ShowDockingPrompt()`
- **What it means**: Docking prompt UI is shown to player

---

### Phase 2: Requesting Docking (Player Presses F)

#### Message 4: Docking Requested
```
[DOCKING] RequestDocking() called - Player pressed docking key
```
- **Color**: Cyan
- **Trigger**: Player presses F key (or configured docking key)
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Docking request initiated

#### Message 5: Station Validation
```
[DOCKING] Station in range: [Station Name]
```
- **Color**: Green
- **Trigger**: Station reference is valid
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: System found a nearby station

#### Message 6: Module Type Check
```
[DOCKING] Station is a valid docking module
```
- **Color**: Green
- **Trigger**: Station has docking module (DockingBay or DockingPort)
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Station can accept docking requests

#### Message 7: Slot Availability
```
[DOCKING] Docking slots available: X/Y free
```
- **Color**: Green
- **Trigger**: Station has free docking slots
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Station can accept this ship (X slots free out of Y total)

#### Message 8: Getting Docking Point
```
[DOCKING] GetAvailableDockingPoint() called on [Module Name]
```
- **Color**: Cyan
- **Trigger**: Requesting docking point from station
- **Function**: `ADockingBayModule::GetAvailableDockingPoint()`
- **What it means**: System is finding a physical docking location

#### Message 9: Docking Point Found
```
[DOCKING] Docking point found: Index X of Y points
```
- **Color**: Green
- **Trigger**: Valid docking point retrieved
- **Function**: `ADockingBayModule::GetAvailableDockingPoint()`
- **What it means**: System knows where to place the ship

#### Message 10: Docking Point Location
```
[DOCKING] Docking point found at location: X=1000 Y=500 Z=300
```
- **Color**: Green
- **Trigger**: Docking point location retrieved
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Ship will dock at these coordinates

#### Message 11: Distance Check
```
[DOCKING] Distance check passed: 1500 units (within 2000 max)
```
- **Color**: Green
- **Trigger**: Ship is within DockingRange
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Ship is close enough to dock

#### Message 12: Starting Docking Sequence
```
[DOCKING] All checks passed - starting docking sequence
```
- **Color**: Cyan
- **Trigger**: All validation passed
- **Function**: `ASpaceship::RequestDocking()`
- **What it means**: Ship will now move to docking point

---

### Phase 3: Ship Movement to Docking Point

#### Message 13: Navigation Started
```
[DOCKING] NavigateToDockingPoint() called - Moving ship to docking point
```
- **Color**: Cyan
- **Trigger**: Beginning ship movement
- **Function**: `ASpaceship::NavigateToDockingPoint()`
- **What it means**: Ship is moving to docking position

#### Message 14: Target Position
```
[DOCKING] Target docking position: X=1000 Y=500 Z=300
```
- **Color**: Cyan
- **Trigger**: Target transform calculated
- **Function**: `ASpaceship::NavigateToDockingPoint()`
- **What it means**: These are the coordinates ship will dock at

#### Message 15: Ship Moved
```
[DOCKING] Ship teleported to docking point successfully
```
- **Color**: Green
- **Trigger**: Ship moved to docking point (instant in MVP)
- **Function**: `ASpaceship::NavigateToDockingPoint()`
- **What it means**: Ship is now at docking location

---

### Phase 4: Completing Docking

#### Message 16: Finalizing Docking
```
[DOCKING] CompleteDocking() called - Finalizing docking process
```
- **Color**: Cyan
- **Trigger**: Beginning finalization
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Ship is being locked in

#### Message 17: State Updated
```
[DOCKING] Ship state updated: bIsDocked=true, bIsDocking=false
```
- **Color**: Green
- **Trigger**: Internal state changed
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Ship is now flagged as docked

#### Message 18: Station Notified (Module)
```
[DOCKING] DockShip() called on station module
```
- **Color**: Cyan
- **Trigger**: Telling station a ship docked
- **Function**: `ADockingBayModule::DockShip()`
- **What it means**: Station is updating its docked ship count

#### Message 19: Ship Slot Updated
```
[DOCKING] Ship docked successfully - occupied slots: 1/4
```
- **Color**: Green
- **Trigger**: Station updated docked count
- **Function**: `ADockingBayModule::DockShip()`
- **What it means**: Station now tracks this ship (1 of 4 slots used)

#### Message 20: Station Notified (Ship)
```
[DOCKING] Station notified of successful docking
```
- **Color**: Green
- **Trigger**: Station acknowledged docking
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Station knows ship is docked

#### Message 21: Player Controller Found
```
[DOCKING] Player controller found
```
- **Color**: Green
- **Trigger**: Got player controller reference
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Can disable input and show UI

#### Message 22: Input Disabled
```
[DOCKING] Ship input disabled
```
- **Color**: Green
- **Trigger**: Player input disabled
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Player cannot control ship while docked

#### Message 23: Ship Hidden
```
[DOCKING] Ship mesh hidden (player is 'inside' station)
```
- **Color**: Green
- **Trigger**: Ship actor hidden
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Ship is invisible (simulating being inside)

#### Message 24: Creating Trading UI
```
[DOCKING] Creating trading UI widget...
```
- **Color**: Cyan
- **Trigger**: About to create trading interface
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Trading UI is being created

#### Message 25: Trading UI Created
```
[DOCKING] Trading UI widget created and added to viewport
```
- **Color**: Green
- **Trigger**: Widget successfully created
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Trading UI should be visible

#### Message 26: Input Mode Changed
```
[DOCKING] Input mode set to UI only, mouse cursor shown
```
- **Color**: Green
- **Trigger**: Input mode changed for UI
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: Player can now interact with trading UI

#### Message 27: Docking Complete (Success Banner)
```
═══════════════════════════════════════════════════
[DOCKING] ✓ DOCKING COMPLETE - Trading UI should be visible
═══════════════════════════════════════════════════
```
- **Color**: Green
- **Trigger**: All docking steps completed successfully
- **Function**: `ASpaceship::CompleteDocking()`
- **What it means**: **DOCKING SUCCESSFUL** - Trading should work now

---

### Phase 5: Undocking

#### Message 28: Undocking Requested
```
[UNDOCKING] Undock() called - Beginning undock sequence
```
- **Color**: Cyan
- **Trigger**: Player closes trading UI or presses undock
- **Function**: `ASpaceship::Undock()`
- **What it means**: Undocking process started

#### Message 29: Station Notified (Undocking Module)
```
[UNDOCKING] UndockShip() called on station module
```
- **Color**: Cyan
- **Trigger**: Telling station ship is leaving
- **Function**: `ADockingBayModule::UndockShip()`
- **What it means**: Station is freeing the docking slot

#### Message 30: Slot Freed
```
[UNDOCKING] Ship undocked successfully - occupied slots: 0/4
```
- **Color**: Green
- **Trigger**: Station freed the slot
- **Function**: `ADockingBayModule::UndockShip()`
- **What it means**: Docking slot is now available (0 of 4 used)

#### Message 31: Station Notified (Undocking Ship)
```
[UNDOCKING] Station notified: [Station Name]
```
- **Color**: Green
- **Trigger**: Station acknowledged undocking
- **Function**: `ASpaceship::Undock()`
- **What it means**: Station knows ship left

#### Message 32: State Updated
```
[UNDOCKING] Ship state updated: bIsDocked=false
```
- **Color**: Green
- **Trigger**: Internal state changed
- **Function**: `ASpaceship::Undock()`
- **What it means**: Ship is no longer docked

#### Message 33: Trading UI Removed
```
[UNDOCKING] Trading UI widget removed from viewport
```
- **Color**: Green
- **Trigger**: Trading interface closed
- **Function**: `ASpaceship::Undock()`
- **What it means**: Trading UI is hidden

#### Message 34: Input Re-enabled
```
[UNDOCKING] Ship input re-enabled
```
- **Color**: Green
- **Trigger**: Player input enabled
- **Function**: `ASpaceship::Undock()`
- **What it means**: Player can control ship again

#### Message 35: Ship Shown
```
[UNDOCKING] Ship mesh shown
```
- **Color**: Green
- **Trigger**: Ship actor visible again
- **Function**: `ASpaceship::Undock()`
- **What it means**: Ship is visible in world

#### Message 36: Input Mode Restored
```
[UNDOCKING] Input mode set to game only, mouse cursor hidden
```
- **Color**: Green
- **Trigger**: Input mode changed for gameplay
- **Function**: `ASpaceship::Undock()`
- **What it means**: Back to normal flight controls

#### Message 37: Impulse Applied
```
[UNDOCKING] Forward impulse applied (500 units)
```
- **Color**: Green
- **Trigger**: Ship pushed away from station
- **Function**: `ASpaceship::Undock()`
- **What it means**: Ship is moving away from station

#### Message 38: Undocking Complete (Success Banner)
```
═══════════════════════════════════════════════════
[UNDOCKING] ✓ UNDOCKING COMPLETE - Player has control
═══════════════════════════════════════════════════
```
- **Color**: Green
- **Trigger**: All undocking steps completed
- **Function**: `ASpaceship::Undock()`
- **What it means**: **UNDOCKING SUCCESSFUL** - Player can fly

---

## 🚨 Error Messages

### Approaching Station Errors

#### ERROR: No Docking Prompt Widget Class
```
[DOCKING] ERROR: DockingPromptWidgetClass not set in Blueprint
```
- **Color**: Red
- **Cause**: `DockingPromptWidgetClass` property not set in BP_PlayerShip
- **Fix**: Assign `WBP_DockingPrompt` widget class in ship Blueprint

#### ERROR: Widget Creation Failed
```
[DOCKING] ERROR: Failed to create WBP_DockingPrompt widget
```
- **Color**: Red
- **Cause**: Widget class exists but creation failed
- **Fix**: Check widget Blueprint compiles without errors

#### ERROR: No Player Controller
```
[DOCKING] ERROR: No player controller found for widget creation
```
- **Color**: Red
- **Cause**: Ship has no player controller
- **Fix**: Ensure ship is possessed by player controller

---

### Requesting Docking Errors

#### ERROR: No Station in Range
```
[DOCKING] ERROR: No station in range
```
- **Color**: Red
- **Cause**: `NearbyStation` is null (not in docking range)
- **Fix**: Move closer to station (within docking range sphere)

#### ERROR: Not a Docking Module
```
[DOCKING] ERROR: Station is not a docking module
```
- **Color**: Red
- **Cause**: Station doesn't have DockingBayModule or DockingPortModule
- **Fix**: Add docking module to station Blueprint

#### ERROR: No Docking Slots Available
```
[DOCKING] ERROR: No docking slots available (4/4 occupied)
```
- **Color**: Red
- **Cause**: All docking slots are full
- **Fix**: Wait for other ships to undock, or increase `MaxDockedShips`

#### ERROR: No Docking Capacity
```
[DOCKING] ERROR: No docking capacity (4/4 occupied)
```
- **Color**: Red
- **Cause**: Module reports full capacity
- **Fix**: Check `MaxDockedShips` value in module

#### ERROR: No Docking Points Defined
```
[DOCKING] ERROR: No docking points defined in module
```
- **Color**: Red
- **Cause**: `DockingPoints` array is empty in module
- **Fix**: Add scene components to `DockingPoints` array in station Blueprint

#### ERROR: Failed to Get Docking Point
```
[DOCKING] ERROR: Failed to get docking point (null pointer)
```
- **Color**: Red
- **Cause**: `GetAvailableDockingPoint()` returned null
- **Fix**: Check docking points are properly configured

#### ERROR: Too Far from Docking Point
```
[DOCKING] ERROR: Too far from docking point (3000 units > 2000 max)
```
- **Color**: Red
- **Cause**: Ship is beyond `DockingRange` distance
- **Fix**: Move closer to station, or increase `DockingRange` in ship

---

### Navigation Errors

#### ERROR: Invalid Docking Point
```
[DOCKING] ERROR: Invalid docking point (null pointer)
```
- **Color**: Red
- **Cause**: Docking point became invalid during navigation
- **Fix**: Check docking point scene components exist

---

### Completing Docking Errors

#### ERROR: No Player Controller (Docking)
```
[DOCKING] ERROR: No player controller found - cannot disable input or create UI
```
- **Color**: Red
- **Cause**: Lost player controller reference
- **Fix**: Ensure ship stays possessed during docking

#### ERROR: Trading UI Widget Creation Failed
```
[DOCKING] ERROR: Failed to create trading UI widget
```
- **Color**: Red
- **Cause**: Widget class exists but creation failed
- **Fix**: Check `WBP_TradingUI` Blueprint compiles without errors

#### ERROR: Trading Interface Class Not Set
```
[DOCKING] ERROR: TradingInterfaceClass not set in Blueprint
```
- **Color**: Red
- **Cause**: `TradingInterfaceClass` property not set in BP_PlayerShip
- **Fix**: Assign `WBP_TradingUI` widget class in ship Blueprint

---

### Undocking Errors

#### WARNING: Not Currently Docked
```
[UNDOCKING] WARNING: Not currently docked - aborting undock
```
- **Color**: Yellow
- **Cause**: Tried to undock when not docked
- **Fix**: This is normal if undock called while in flight

#### ERROR: No Player Controller (Undocking)
```
[UNDOCKING] ERROR: No player controller found
```
- **Color**: Red
- **Cause**: Lost player controller reference
- **Fix**: Ensure ship stays possessed during undocking

---

### Module Errors

#### ERROR: Cannot Dock - All Slots Occupied
```
[DOCKING] ERROR: Cannot dock - all slots occupied (4/4)
```
- **Color**: Red
- **Cause**: Station tried to dock ship when full
- **Fix**: This shouldn't happen if validation works correctly

#### WARNING: No Ships Currently Docked
```
[UNDOCKING] WARNING: No ships currently docked
```
- **Color**: Yellow
- **Cause**: Station tried to undock when count already 0
- **Fix**: This shouldn't happen if validation works correctly

---

## 🔍 Troubleshooting Guide

### "I fly up to the station and nothing happens"

**Check for these messages (or lack thereof):**

1. **Missing**: `[DOCKING] Ship entered docking range of station`
   - **Problem**: Ship not detecting station range
   - **Solution**: Check station has collision sphere for docking range

2. **Missing**: `[DOCKING] WBP_DockingPrompt created and added to viewport`
   - **Problem**: Docking prompt widget not showing
   - **Solution**: 
     - Check for error: `DockingPromptWidgetClass not set in Blueprint`
     - Assign widget class in BP_PlayerShip

3. **See**: `[DOCKING] RequestDocking() called` but no station found
   - **Problem**: Docking key works but no station nearby
   - **Solution**: Check `NearbyStation` is being set properly

4. **See**: `[DOCKING] ERROR: No docking points defined in module`
   - **Problem**: Station has no physical docking locations
   - **Solution**: Add scene components to station's `DockingPoints` array

5. **See**: `[DOCKING] ERROR: Too far from docking point`
   - **Problem**: Ship is detected but too far away
   - **Solution**: 
     - Increase `DockingRange` in ship (default: 2000)
     - Or move closer to station

6. **Last message**: `[DOCKING] All checks passed - starting docking sequence`
   - **Problem**: Docking starts but never completes
   - **Solution**: Check NavigateToDockingPoint is being called

7. **Last message**: `[DOCKING] Ship teleported to docking point successfully`
   - **Problem**: Ship moves but docking doesn't finalize
   - **Solution**: Check CompleteDocking is being called

8. **Missing**: `[DOCKING] Trading UI widget created and added to viewport`
   - **Problem**: Docking works but no trading UI
   - **Solution**: 
     - Check for error: `TradingInterfaceClass not set in Blueprint`
     - Assign widget class in BP_PlayerShip
     - Verify `WBP_TradingUI` exists and compiles

---

## 📝 Message Format

All debug messages follow this format:

```
[SYSTEM] Message text with optional data: value=123
```

- **[SYSTEM]**: One of `[DOCKING]`, `[UNDOCKING]`
- **Message**: Descriptive text
- **Data**: Optional values in messages (distances, counts, names)

---

## 🎛️ Disabling Debug Messages

Debug messages are compiled in and cannot be disabled at runtime without modifying code.

To disable in future builds:
1. Search for `GEngine->AddOnScreenDebugMessage` in modified files
2. Comment out or remove calls
3. Recompile project

---

## 📚 Related Documentation

- **Docking System**: `docs/reference/DOCKING_SYSTEM_QUICK_REFERENCE.md`
- **Simple Docking Setup**: `docs/reference/SIMPLE_DOCKING_SETUP.md`
- **Blueprint Implementation**: `docs/BP/BP_PlayerShip.md`
- **Station Setup**: `docs/BP/BP_SpaceStation_Main.md`

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-09 | Initial debug message system added |

---

**Use this guide to identify exactly where your docking system stops working!**

The messages appear in order from top to bottom as docking progresses. Find the last message you see, then check the troubleshooting section for that point in the flow.
