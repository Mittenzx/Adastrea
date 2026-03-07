# Visual Debug Flow - Docking Process

**Quick Visual Guide to Debug Messages**

---

## 🎯 Complete Docking Flow with Debug Messages

```
┌─────────────────────────────────────────────────────────────┐
│                  PHASE 1: APPROACHING                       │
└─────────────────────────────────────────────────────────────┘

    Player flies ship toward station...
              ↓
    🟢 [DOCKING] Ship entered docking range of station: [Name]
              ↓
    🟢 [DOCKING] WBP_DockingPrompt created and added to viewport
              ↓
    🔵 [DOCKING] Docking prompt now visible
              ↓
    "Press F to Dock" UI appears on screen


┌─────────────────────────────────────────────────────────────┐
│              PHASE 2: PRESSING F (VALIDATION)               │
└─────────────────────────────────────────────────────────────┘

    Player presses F key...
              ↓
    🔵 [DOCKING] RequestDocking() called - Player pressed docking key
              ↓
    ┌──────────────────────────┐
    │   VALIDATION CHECKS      │
    └──────────────────────────┘
              ↓
    🟢 [DOCKING] Station in range: [Station Name]
              ↓
    🟢 [DOCKING] Station is a valid docking module
              ↓
    🟢 [DOCKING] Docking slots available: X/Y free
              ↓
    🔵 [DOCKING] GetAvailableDockingPoint() called on [Module]
              ↓
    🟢 [DOCKING] Docking point found: Index X of Y points
              ↓
    🟢 [DOCKING] Docking point found at location: X=... Y=... Z=...
              ↓
    🟢 [DOCKING] Distance check passed: 1500 units (within 2000 max)
              ↓
    🔵 [DOCKING] All checks passed - starting docking sequence


┌─────────────────────────────────────────────────────────────┐
│                  PHASE 3: SHIP MOVEMENT                     │
└─────────────────────────────────────────────────────────────┘

    Ship moves to docking point...
              ↓
    🔵 [DOCKING] NavigateToDockingPoint() called - Moving ship
              ↓
    🔵 [DOCKING] Target docking position: X=... Y=... Z=...
              ↓
    🟢 [DOCKING] Ship teleported to docking point successfully


┌─────────────────────────────────────────────────────────────┐
│              PHASE 4: COMPLETING DOCKING                    │
└─────────────────────────────────────────────────────────────┘

    Finalizing docking...
              ↓
    🔵 [DOCKING] CompleteDocking() called - Finalizing
              ↓
    🟢 [DOCKING] Ship state updated: bIsDocked=true
              ↓
    🔵 [DOCKING] DockShip() called on station module
              ↓
    🟢 [DOCKING] Ship docked successfully - occupied slots: 1/4
              ↓
    🟢 [DOCKING] Station notified of successful docking
              ↓
    🟢 [DOCKING] Player controller found
              ↓
    🟢 [DOCKING] Ship input disabled
              ↓
    🟢 [DOCKING] Ship mesh hidden (player is 'inside' station)
              ↓
    🔵 [DOCKING] Creating trading UI widget...
              ↓
    🟢 [DOCKING] Trading UI widget created and added to viewport
              ↓
    🟢 [DOCKING] Input mode set to UI only, mouse cursor shown
              ↓
    ╔═══════════════════════════════════════════════════════╗
    ║ 🟢 [DOCKING] ✓ DOCKING COMPLETE - Trading UI visible ║
    ╚═══════════════════════════════════════════════════════╝
              ↓
    🎉 SUCCESS! Player can now trade.


┌─────────────────────────────────────────────────────────────┐
│              PHASE 5: UNDOCKING (OPTIONAL)                  │
└─────────────────────────────────────────────────────────────┘

    Player closes trading UI or presses undock...
              ↓
    🔵 [UNDOCKING] Undock() called - Beginning undock sequence
              ↓
    🔵 [UNDOCKING] UndockShip() called on station module
              ↓
    🟢 [UNDOCKING] Ship undocked successfully - occupied slots: 0/4
              ↓
    🟢 [UNDOCKING] Station notified: [Station Name]
              ↓
    🟢 [UNDOCKING] Ship state updated: bIsDocked=false
              ↓
    🟢 [UNDOCKING] Trading UI widget removed from viewport
              ↓
    🟢 [UNDOCKING] Ship input re-enabled
              ↓
    🟢 [UNDOCKING] Ship mesh shown
              ↓
    🟢 [UNDOCKING] Input mode set to game only, mouse cursor hidden
              ↓
    🟢 [UNDOCKING] Forward impulse applied (500 units)
              ↓
    ╔═══════════════════════════════════════════════════════╗
    ║ 🟢 [UNDOCKING] ✓ UNDOCKING COMPLETE - Player has     ║
    ║                control                                 ║
    ╚═══════════════════════════════════════════════════════╝
              ↓
    🎉 SUCCESS! Player can fly again.
```

---

## 🚨 Error Flow Examples

### Error 1: No Station in Range

```
    Player presses F key...
              ↓
    🔵 [DOCKING] RequestDocking() called - Player pressed docking key
              ↓
    🔴 [DOCKING] ERROR: No station in range
              ↓
    ❌ STOP - Fly closer to station
```

**Fix**: Fly within 2000 units of station

---

### Error 2: Widget Class Not Set

```
    Ship enters docking range...
              ↓
    🟢 [DOCKING] Ship entered docking range of station: [Name]
              ↓
    🔴 [DOCKING] ERROR: DockingPromptWidgetClass not set in Blueprint
              ↓
    ❌ STOP - No docking prompt appears
```

**Fix**:
1. Open BP_PlayerShip
2. Set DockingPromptWidgetClass = WBP_DockingPrompt
3. Save and test again

---

### Error 3: No Docking Points

```
    Validation checks...
              ↓
    🟢 [DOCKING] Station is a valid docking module
              ↓
    🟢 [DOCKING] Docking slots available: 4/4 free
              ↓
    🔵 [DOCKING] GetAvailableDockingPoint() called on [Module]
              ↓
    🔴 [DOCKING] ERROR: No docking points defined in module
              ↓
    ❌ STOP - Station has no physical docking locations
```

**Fix**:
1. Open station Blueprint
2. Add scene components for docking points
3. Add them to DockingPoints array
4. Save and test again

---

### Error 4: Too Far

```
    Validation checks...
              ↓
    🟢 [DOCKING] Docking point found at location: X=1000 Y=500 Z=300
              ↓
    🔴 [DOCKING] ERROR: Too far from docking point (3000 > 2000)
              ↓
    ❌ STOP - Ship too far from docking point
```

**Fix**:
- Fly closer to station, OR
- Increase DockingRange in BP_PlayerShip to 3000

---

### Error 5: No Trading UI Class

```
    Completing docking...
              ↓
    🟢 [DOCKING] Ship mesh hidden (player is 'inside' station)
              ↓
    🔵 [DOCKING] Creating trading UI widget...
              ↓
    🔴 [DOCKING] ERROR: TradingInterfaceClass not set in Blueprint
              ↓
    ❌ STOP - No trading UI appears
```

**Fix**:
1. Open BP_PlayerShip
2. Set TradingInterfaceClass = WBP_TradingUI
3. Save and test again

---

## 📊 Color Key Reference

```
🟢 Green  = Success  → Keep going, everything is working
🔵 Cyan   = Info     → Function called, normal operation
🟡 Yellow = Warning  → Check this, might be issue
🔴 Red    = Error    → STOP! Fix this problem!
```

---

## 🎯 Quick Diagnosis

### "Where do I look?"

**Find the last message you saw, then:**

| Last Message Seen | What It Means | Where to Look |
|-------------------|---------------|---------------|
| Nothing | Not in range | Fly closer to station |
| "Ship entered range" | Widget issue | Check DockingPromptWidgetClass |
| "RequestDocking() called" | Validation failed | Check red error after this |
| "All checks passed" | Navigation issue | Check ship movement code |
| "Ship teleported" | Completion issue | Check CompleteDocking |
| "DOCKING COMPLETE" | Success! | Trading should work |

---

## 📸 Screenshot Guide

**Capture these when reporting issues:**

1. **Full screen** showing all visible debug messages
2. **Ship position** relative to station
3. **Blueprint properties** (if requested)
4. **Last 5-10 messages** in sequence

---

## 🔍 Message Search Tips

**Looking for specific info?**

- **Station name**: Look for "Station in range: [name]"
- **Distance**: Look for "Distance check passed: X units"
- **Docking slots**: Look for "Docking slots available: X/Y"
- **Point location**: Look for "Docking point found at location"
- **UI creation**: Look for "Trading UI widget created"

---

## ✅ Success Banners

**These mean it worked:**

```
╔═══════════════════════════════════════════════════════╗
║ 🟢 [DOCKING] ✓ DOCKING COMPLETE - Trading UI visible ║
╚═══════════════════════════════════════════════════════╝
```

```
╔═══════════════════════════════════════════════════════╗
║ 🟢 [UNDOCKING] ✓ UNDOCKING COMPLETE - Player has     ║
║                control                                 ║
╚═══════════════════════════════════════════════════════╝
```

**If you see these, the system is working!** 🎉

---

## 🎓 Reading the Flow

1. **Start at top** - First message appears when entering range
2. **Follow arrows** - Messages appear in order
3. **Green = progress** - Things are working
4. **Red = stop** - Problem found, fix it
5. **Banner = success** - Phase complete

---

**Use this visual guide to track your docking progress!** 📍
