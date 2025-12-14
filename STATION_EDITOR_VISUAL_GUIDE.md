# Station Editor - Visual User Guide

## Quick Visual Reference

This guide shows exactly how the Station Editor works now that interactive placement is implemented.

---

## Before vs After

### ❌ BEFORE (Not Working)
```
User clicks "Build" button
          ↓
      Nothing happens
          ↓
    No visual feedback
          ↓
    Module can't be placed
```

### ✅ AFTER (Now Working!)
```
User clicks "Build" button
          ↓
   Preview ghost appears
          ↓
  Follows cursor in 3D
          ↓
 Green = Valid, Red = Invalid
          ↓
  Left-click to place
          ↓
    Module spawns!
```

---

## The Complete User Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                     STATION EDITOR WORKFLOW                       │
└─────────────────────────────────────────────────────────────────┘

Step 1: OPEN EDITOR
┌─────────────────────────────────┐
│  Player flies near station      │
│  Presses F9 key                 │
│  Editor UI opens                │
└─────────────────────────────────┘
                ↓

Step 2: SELECT MODULE
┌─────────────────────────────────┐
│  Left Panel: Module List        │
│  ┌─────────────────────────┐   │
│  │ 🏭 Reactor Module        │   │
│  │ Cost: 1000 | 30s        │   │
│  │ [Build] ← Click this!   │   │
│  └─────────────────────────┘   │
│  ┌─────────────────────────┐   │
│  │ 🛡️ Shield Module         │   │
│  │ Cost: 800 | 20s         │   │
│  │ [Build]                 │   │
│  └─────────────────────────┘   │
└─────────────────────────────────┘
                ↓

Step 3: PREVIEW MODE ACTIVATED
┌─────────────────────────────────────────────┐
│  Center Panel: 3D Viewport                  │
│                                             │
│          [Station]                          │
│              │                              │
│              │     👻 ← Ghost preview       │
│              │        appears here!         │
│              │                              │
│         Existing                            │
│         modules                             │
│                                             │
└─────────────────────────────────────────────┘
                ↓

Step 4: MOVE CURSOR
┌─────────────────────────────────────────────┐
│  Preview follows your mouse cursor          │
│                                             │
│          [Station]                          │
│              │                              │
│              │  🟢 ← Green = Valid!         │
│              │                              │
│              │  🟢 ← Still valid            │
│              │                              │
│              🔴 ← Red = Invalid (too far)   │
│                                             │
└─────────────────────────────────────────────┘
                ↓

Step 5: PLACE MODULE (Left Click)
┌─────────────────────────────────────────────┐
│  Click when preview is green                │
│                                             │
│          [Station]                          │
│              │                              │
│              │  🏭 ← Module placed!         │
│              │                              │
│              │                              │
│              │                              │
│                                             │
│  Statistics updated automatically           │
└─────────────────────────────────────────────┘
                ↓

Step 6: REPEAT OR CANCEL
┌─────────────────────────────────────────────┐
│  Option A: Select another module            │
│     → Preview changes to new module         │
│     → Can place multiple modules            │
│                                             │
│  Option B: Right-click to cancel            │
│     → Preview disappears                    │
│     → Exit placement mode                   │
│                                             │
│  Option C: Close editor                     │
│     → Saves all placements                  │
│     → Returns to normal gameplay            │
└─────────────────────────────────────────────┘
```

---

## Color-Coded Validation

The preview changes color to show if placement is valid:

### 🟢 GREEN = Valid Placement
```
✅ No collision with other modules
✅ Close enough to station
✅ Player has required tech level
✅ Player can afford the cost
✅ Editor is in edit mode

👉 Left-click will place the module
```

### 🔴 RED = Invalid Placement
```
❌ One or more problems:
   • Too close to another module (collision)
   • Too far from station
   • Not enough credits
   • Tech level too low
   • Station not being edited

👉 Left-click will do nothing
```

---

## Mouse Controls

### During Normal Editing
```
🖱️ LEFT CLICK        → (nothing, use Build buttons)
🖱️ RIGHT CLICK       → (nothing, use Close button)
🖱️ MOUSE MOVEMENT    → Look around / UI interaction
```

### During Placement Mode
```
🖱️ LEFT CLICK        → Confirm placement (if valid)
🖱️ RIGHT CLICK       → Cancel placement
🖱️ MOUSE MOVEMENT    → Preview follows cursor in 3D
```

---

## UI Layout Reference

```
┌────────────────────────────────────────────────────────────────────┐
│  Header: Station Editor - [Station Name]                          │
│  Power: 500/300 MW | Modules: 3/25 | [Close]                      │
├──────────────┬─────────────────────────────────┬───────────────────┤
│              │                                 │                   │
│  MODULE LIST │      3D VIEWPORT (CENTER)       │   STATISTICS      │
│              │                                 │                   │
│ Reactor      │                                 │ Power: 500 MW     │
│ [Build] ←    │    [Station Model]              │ Consumption: 300  │
│              │                                 │                   │
│ Shield       │         👻 ← Preview            │ Balance: +200 MW  │
│ [Build]      │         appears here            │                   │
│              │                                 │ Defense: 45       │
│ Habitat      │                                 │ Efficiency: B+    │
│ [Build]      │                                 │                   │
│              │                                 │                   │
│ Storage      │                                 │                   │
│ [Build]      │                                 │                   │
│              │                                 │                   │
│  ...more     │                                 │                   │
│              │                                 │                   │
├──────────────┴─────────────────────────────────┴───────────────────┤
│  Construction Queue                                                │
│  1. Shield Module [████████░░] 80% (5s remaining) [Cancel]         │
│  2. Habitat Module [░░░░░░░░░░] 0% (queued) [Cancel]               │
└────────────────────────────────────────────────────────────────────┘
```

---

## Example Scenarios

### Scenario 1: Successful Placement

```
1. Click "Build" on Reactor Module
   Status: Preview appears (green)
   
2. Move cursor over valid location
   Preview: 🟢 Green
   Message: Can place here
   
3. Left-click
   Result: ✅ Module placed!
   Sound: *placement sound*
   Stats: Power +500 MW
```

### Scenario 2: Invalid Placement (Collision)

```
1. Click "Build" on Habitat Module
   Status: Preview appears
   
2. Move cursor too close to existing module
   Preview: 🔴 Red
   Message: Collision detected
   
3. Left-click
   Result: ⚠️ Nothing happens
   Log: "Cannot place module: CollisionDetected"
```

### Scenario 3: Invalid Placement (Not Enough Credits)

```
1. Click "Build" on expensive module
   Status: Preview appears
   
2. Move cursor (player has 500 credits, module costs 1000)
   Preview: 🔴 Red
   Message: Insufficient funds
   
3. Left-click
   Result: ⚠️ Nothing happens
   Log: "Cannot place module: InsufficientFunds"
```

### Scenario 4: Canceling Placement

```
1. Click "Build" on any module
   Status: Preview appears
   
2. Changed your mind
   Action: Right-click
   
3. Result
   Preview: Disappears
   Status: Back to normal mode
   Can: Select different module
```

---

## Tips and Tricks

### 💡 Tip 1: Plan Before Placing
```
• Select module to see preview
• Move cursor around to explore options
• Right-click to cancel if not happy
• Select different module to compare
• No commitment until you left-click!
```

### 💡 Tip 2: Use the Color Feedback
```
🟢 Green = Good to go!
🔴 Red = Find another spot
   
Don't waste time clicking on red positions.
The validation is happening in real-time!
```

### 💡 Tip 3: Build in Sequence
```
1. Place power module first (reactor)
2. Place essential modules next (habitats)
3. Place optional modules last (storage)
   
Preview helps you see connections before placing!
```

### 💡 Tip 4: Check Statistics
```
After each placement:
• Power balance updates
• Module count updates
• Defense rating changes
• Efficiency recalculated
   
Make sure your station is balanced!
```

---

## Common Questions

### Q: Why is my preview always red?
**A:** Check these common causes:
- Not close enough to station (max 5000 units)
- Too close to another module (200 unit radius)
- Not enough credits in player account
- Tech level requirement not met
- Not actually in edit mode

### Q: Why doesn't clicking place the module?
**A:** The preview must be:
1. ✅ Visible (ghost showing)
2. ✅ Green (validation passed)
3. ✅ At a valid 3D position

Check the Output Log for error messages.

### Q: Can I rotate modules?
**A:** Not in this version. Currently:
- Rotation is always `FRotator::ZeroRotator`
- Future enhancement: Q/E keys for rotation

### Q: Can I place multiple modules at once?
**A:** Sequential placement only:
1. Place first module
2. Select next module
3. Place second module
4. Repeat...

Future enhancement: Click multiple times to queue builds.

### Q: How do I undo a placement?
**A:** Current version:
- Close editor without saving (Cancel)
- This reverts ALL placements in this session

Future enhancement: Ctrl+Z for individual undo.

### Q: Why does my preview jump around?
**A:** The line trace is hitting different surfaces:
- It traces from camera through cursor
- Hits first visible surface
- If no hit, preview won't show
- Make sure you're looking at the station area

---

## Keyboard & Mouse Quick Reference

```
┌──────────────────────┬────────────────────────────────────────┐
│ Key/Button           │ Action                                 │
├──────────────────────┼────────────────────────────────────────┤
│ F9                   │ Open/Close Station Editor              │
│ Mouse Move           │ Move preview (when in placement mode)  │
│ Left Click (UI)      │ Click Build button to select module    │
│ Left Click (3D)      │ Confirm placement (when preview green) │
│ Right Click (3D)     │ Cancel placement mode                  │
│ ESC                  │ (Future: Cancel placement)             │
│ Q/E                  │ (Future: Rotate preview)               │
│ Ctrl+Z               │ (Future: Undo last placement)          │
└──────────────────────┴────────────────────────────────────────┘
```

---

## Troubleshooting Flowchart

```
Is the editor opening?
    │
    ├─→ NO → Check F9 key binding
    │      → Check near station (< 5000 units)
    │      → Check Output Log for errors
    │
    └─→ YES
        │
        Is module list visible?
        │
        ├─→ NO → Check WBP_StationEditor layout
        │      → Check ModuleListScrollBox widget
        │      → Check module catalog has entries
        │
        └─→ YES
            │
            Does clicking Build show preview?
            │
            ├─→ NO → Check OnModuleButtonClicked() is called
            │      → Check EnterPlacementMode() logic
            │      → Check EditorManager exists
            │      → Check preview actor spawned
            │
            └─→ YES
                │
                Does preview follow cursor?
                │
                ├─→ NO → Check NativeTick() running
                │      → Check UpdatePreviewPosition()
                │      → Check line trace hitting geometry
                │      → Add log to verify cursor tracking
                │
                └─→ YES
                    │
                    Does clicking place module?
                    │
                    ├─→ NO → Check preview is green
                    │      → Check OnViewportClicked() called
                    │      → Check validation passes
                    │      → Check EditorManager->PlaceModule()
                    │
                    └─→ YES
                        │
                        ✅ EVERYTHING WORKS!
```

---

## Success Indicators

You know it's working when:

✅ **Preview appears** after clicking Build  
✅ **Preview follows** your mouse smoothly  
✅ **Color changes** based on location (green/red)  
✅ **Module spawns** when you click (green only)  
✅ **Statistics update** after placement  
✅ **Preview disappears** after placement  
✅ **Can place multiple** modules in sequence  
✅ **Right-click cancels** cleanly  
✅ **No crashes** or errors in log  

If all these work, congratulations! 🎉

---

## Next Steps

Once basic placement works, you might want to:

1. **Add Rotation**: Q/E keys to rotate preview
2. **Add Grid**: Show grid lines in viewport
3. **Add Tooltips**: "Click to place, Right-click to cancel"
4. **Add Sounds**: Audio feedback for placement/cancel
5. **Add Undo**: Ctrl+Z to undo individual placements
6. **Add Hotkeys**: Number keys for common modules
7. **Add Multi-place**: Shift+click to place multiple

See `STATION_EDITOR_COMPLETE_GUIDE.md` for implementation details.

---

**Visual Guide Version**: 1.0  
**Last Updated**: 2025-12-14  
**Status**: Complete and Working ✅
