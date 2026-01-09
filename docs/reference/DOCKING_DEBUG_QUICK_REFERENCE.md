# Quick Debug Reference Card - Docking System

**Version**: 1.0  
**Date**: January 9, 2026  
**Purpose**: Quick troubleshooting guide for docking issues

---

## 🚀 Quick Start

1. **Open Unreal Editor** with your project
2. **Play in Editor** (PIE)
3. **Fly toward a station**
4. **Watch the screen** - colored messages will appear
5. **Press F** to dock when prompted
6. **Watch the sequence** - messages show each step

---

## 📊 Message Colors

| Color | Meaning | Action |
|-------|---------|--------|
| 🟢 **Green** | Success - Keep going | Normal progress |
| 🔵 **Cyan** | Info - Function called | Expected behavior |
| 🟡 **Yellow** | Warning - Check this | May need attention |
| 🔴 **Red** | Error - Problem found | **STOP - Fix this!** |

---

## 🎯 Expected Message Sequence

### Normal Docking (Should See These):

```
1. [DOCKING] Ship entered docking range ← Station detected
2. [DOCKING] WBP_DockingPrompt created ← UI shown
3. [DOCKING] RequestDocking() called ← You pressed F
4. [DOCKING] Station in range ← Validation passed
5. [DOCKING] Station is a valid docking module ← Can dock here
6. [DOCKING] Docking slots available ← Space for ship
7. [DOCKING] Docking point found ← Physical location ready
8. [DOCKING] Distance check passed ← Close enough
9. [DOCKING] Ship teleported to docking point ← Ship moved
10. [DOCKING] ✓ DOCKING COMPLETE ← SUCCESS!
11. [DOCKING] Trading UI widget created ← Can trade now
```

**If you see message #10, docking worked! Trading UI should be visible.**

---

## ⚠️ Common Problems

### Problem 1: "I see nothing"
**Last Message**: None  
**Cause**: Not in docking range  
**Fix**: Fly closer to station

---

### Problem 2: "No prompt appears"
**Last Message**: `Ship entered docking range`  
**Error**: `DockingPromptWidgetClass not set in Blueprint`  
**Fix**: 
1. Open `BP_PlayerShip` Blueprint
2. Find `DockingPromptWidgetClass` property
3. Set it to `WBP_DockingPrompt`
4. Save and test again

---

### Problem 3: "Prompt shows but F does nothing"
**Last Message**: `WBP_DockingPrompt created`  
**When I Press F**: `ERROR: No station in range`  
**Fix**: 
- Station is too far or collision not working
- Check station has docking range collision sphere
- Increase `DockingRange` in ship (try 3000 instead of 2000)

---

### Problem 4: "Not a docking module"
**Last Message**: `RequestDocking() called`  
**Error**: `Station is not a docking module`  
**Fix**:
1. Open station Blueprint
2. Add `DockingBayModule` or `DockingPortModule` component
3. Save and test

---

### Problem 5: "No docking points defined"
**Last Message**: Several checks pass  
**Error**: `No docking points defined in module`  
**Fix**:
1. Open station Blueprint
2. Select docking module component
3. Add scene components for docking points
4. Add them to `DockingPoints` array
5. Save and test

---

### Problem 6: "Too far from docking point"
**Last Message**: `Docking point found`  
**Error**: `Too far from docking point (3000 > 2000)`  
**Fix**:
- Either fly closer to station
- Or increase `DockingRange` in BP_PlayerShip to 3000+

---

### Problem 7: "Docking works but no trading UI"
**Last Message**: `DOCKING COMPLETE`  
**Error**: `TradingInterfaceClass not set in Blueprint`  
**Fix**:
1. Open `BP_PlayerShip` Blueprint
2. Find `TradingInterfaceClass` property
3. Set it to `WBP_TradingUI`
4. Save and test

---

### Problem 8: "Trading UI widget creation failed"
**Last Message**: `Creating trading UI widget...`  
**Error**: `Failed to create trading UI widget`  
**Fix**:
1. Check `WBP_TradingUI` Blueprint exists
2. Open it and compile
3. Fix any Blueprint errors
4. Save and test

---

## 📸 Taking Screenshots

If asking for help, take screenshots showing:
1. **All debug messages** on screen
2. **Ship position** relative to station
3. **Blueprint properties** (DockingPromptWidgetClass, TradingInterfaceClass)
4. **Station docking module** configuration

---

## 🔍 Finding Issues

**Step 1**: Watch messages as you fly toward station  
**Step 2**: Note the **last message** you see  
**Step 3**: Look up that message in problems above  
**Step 4**: Apply the fix  
**Step 5**: Test again

---

## 📖 Full Documentation

For complete message list and detailed troubleshooting:
- See `docs/reference/DOCKING_DEBUG_MESSAGES.md`

---

## 💡 Pro Tips

1. **Messages stack** - Most recent at bottom of screen
2. **Red messages** mean STOP - something is wrong
3. **Green banners** (with ═══) mean success
4. **Copy-paste messages** when asking for help
5. **All messages** have `[DOCKING]` or `[UNDOCKING]` prefix

---

## 🎮 Testing Checklist

Test each of these:
- [ ] Approach station from different angles
- [ ] Press F when prompt shows
- [ ] Check trading UI opens
- [ ] Try buying/selling items
- [ ] Press undock button
- [ ] Verify ship regains control

---

## 🚨 Emergency Help

**Completely stuck?**

Share these with someone helping you:
1. **Last 5-10 messages** you saw on screen
2. **Screenshot** of ship near station
3. **Screenshot** of BP_PlayerShip properties showing:
   - DockingPromptWidgetClass
   - TradingInterfaceClass
   - DockingRange
4. **Screenshot** of station's docking module showing:
   - DockingPoints array
   - MaxDockedShips

---

**Good luck debugging! The messages will tell you exactly where it stops.** 🚀
