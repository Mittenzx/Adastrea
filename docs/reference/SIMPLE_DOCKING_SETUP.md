# Simple Docking Setup Guide

**Last Updated**: January 8, 2026  
**Status**: MVP Simplified System (Trade Simulator)  
**For**: Beginners learning Unreal Engine and Blueprint

---

## 🎯 Overview

This guide explains the **simplified docking system** designed for the Trade Simulator MVP. The docking process is intentionally simple:

1. **Fly near a station**
2. **Press F to dock** (when in range)
3. **Trade** (buy/sell cargo)
4. **Press F to undock**
5. **Fly to another station**
6. **Repeat!**

No complex animations, no timeline curves, no complicated setup. Just gameplay.

---

## 📦 What You Need

### In Your Blueprint

1. **Spaceship Blueprint** (BP_PlayerShip or similar)
2. **Station Blueprint** with `DockingBayModule` component
3. **Input Mapping Context** with F key bound to "Dock" action

### In C++ (Already Implemented)

✅ Simplified docking logic in `ASpaceship` class  
✅ Instant teleport to docking point  
✅ Distance check (default: 2000 units)  
✅ Input binding for `DockAction`

---

## 🚀 Quick Setup (5 Steps)

### Step 1: Create Input Action (If Not Exists)

**Location**: `Content/Input/Actions/`

1. Right-click in Content Browser → **Input** → **Input Action**
2. Name it: `IA_Dock`
3. Open it
4. **Value Type**: Digital (bool) - press = true, release = false
5. Save

### Step 2: Add to Input Mapping Context

**Location**: `Content/Input/IMC_Spaceship.uasset`

1. Open your spaceship input mapping context
2. Click **+** to add new mapping
3. Select `IA_Dock` action
4. Add keyboard mapping: **F** key
5. Save

### Step 3: Configure Input Config Data Asset

**Location**: `Content/DataAssets/Input/DA_InputConfig.uasset`

1. Open the Input Config Data Asset
2. Find **"Input Actions → Interaction"** category
3. Set **Dock Action** = `IA_Dock`
4. Save

### Step 4: Setup Station with Docking Points

**In Your Station Blueprint:**

1. Add `DockingBayModule` component (if not already present)
2. Add **Scene Components** as children (these are docking points)
   - Name them: `DockingPoint_01`, `DockingPoint_02`, etc.
   - Position them where ships should dock (e.g., near hangar bay entrance)
   - Rotate them to face outward (ship will match this rotation)
3. Select the `DockingBayModule` component
4. In **Details Panel** → **Docking** category:
   - Set **Max Docked Ships**: 4 (or however many points you created)
   - Populate **Docking Points** array with your scene components
5. Save

**Important**: Number of docking points should equal or exceed `MaxDockedShips`!

### Step 5: Configure Ship Docking Settings

**In Your Spaceship Blueprint (BP_PlayerShip):**

1. Open the Blueprint
2. In **Details Panel** → **Docking** category:
   - **Docking Range**: 2000.0 (how close player must be to dock)
   - **Docking Prompt Widget Class**: Assign your UI widget for "Press F to Dock" prompt
   - **Trading Interface Class**: Assign your trading UI widget
3. Save

---

## 🎮 How It Works (User Experience)

### Player Perspective:

1. **Fly toward station**
2. When within 2000 units of docking point:
   - Docking prompt appears: "Press F to Dock"
3. **Press F key**:
   - Ship instantly teleports to docking point
   - Ship rotates to match docking point orientation
   - Flight controls disabled
   - Ship hidden (you're "inside" the station now)
   - Trading UI opens
4. **Use trading UI** to buy/sell cargo
5. **Press F (or designated undock button)** to leave:
   - Trading UI closes
   - Ship reappears at docking point
   - Flight controls re-enabled
   - Small forward impulse pushes ship away from station
6. **Fly to next station** and repeat!

### Technical Flow:

```
Player presses F
  ↓
ASpaceship::RequestDocking() called
  ↓
Validates:
  - Station nearby? ✓
  - Docking slots available? ✓
  - Within docking range (2000 units)? ✓
  ↓
NavigateToDockingPoint()
  - Instant teleport to docking point location/rotation
  ↓
CompleteDocking()
  - Disable player input
  - Hide ship mesh
  - Show trading UI
  - Set UI input mode
```

---

## ⚙️ Configurable Properties

### ASpaceship Docking Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `DockingRange` | float | 2000.0 | Max distance from docking point to allow docking (units) |
| `DockingPromptWidgetClass` | TSubclassOf | None | UI widget shown when near station ("Press F") |
| `TradingInterfaceClass` | TSubclassOf | None | UI widget for trading interface |

**How to Change in Blueprint:**
1. Open your spaceship Blueprint
2. Search for property name in Details panel
3. Adjust value
4. Save

### DockingBayModule Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `MaxDockedShips` | int32 | 4 | How many ships can dock simultaneously |
| `DockingPoints` | Array | Empty | Scene components marking dock locations |

---

## 🐛 Troubleshooting

### "Press F and nothing happens"

**Check:**
1. Is `DockAction` assigned in `DA_InputConfig`?
2. Is F key mapped to `IA_Dock` in `IMC_Spaceship`?
3. Are you within `DockingRange` (2000 units) of the station?
4. Does the station have `DockingBayModule` with docking points?

**Debug Log:**
- Open **Output Log** in Unreal Editor
- Search for: `LogAdastreaShips`
- Look for warnings like "No station in range" or "Too far from docking point"

### "Ship teleports weird place"

**Fix:**
- Check docking point scene component position and rotation
- The ship will exactly match the docking point's transform
- Rotate docking point to face the direction you want the ship to face

### "Trading UI doesn't open"

**Check:**
1. Is `TradingInterfaceClass` assigned in spaceship Blueprint?
2. Does the widget exist and compile without errors?

### "Can't undock"

**Check:**
- Ensure your trading UI has a "Close" or "Undock" button
- That button should call `Undock()` on the possessed ship
- Or bind F key to call undock when docked

---

## 🎨 Creating Your Own Docking Prompt Widget

**Simple Example (UMG Widget):**

1. Create new **Widget Blueprint**: `WBP_DockingPrompt`
2. Add **Text Block**: "Press F to Dock"
3. Style it (font, color, size)
4. Add **Border** or **Image** for background
5. Save
6. Assign to **Docking Prompt Widget Class** in spaceship Blueprint

The game will automatically show/hide this widget when you're near/far from docking.

---

## 🔧 Advanced: Custom Docking Range Per Station

**Want different stations to have different docking ranges?**

Currently, `DockingRange` is on the ship. To make it per-station:

1. Add `DockingRange` property to `DockingBayModule`
2. Modify `RequestDocking()` to check station's range instead
3. Or keep it simple for MVP and adjust all ships equally!

---

## 📚 Related Documentation

- **Input System**: See `Assets/EnhancedInputImplementation.md`
- **Trading System**: See `Assets/TradingSystemGuide.md` (when created)
- **Station Modules**: See `Assets/StationSystemGuide.md`

---

## 💡 Design Philosophy (MVP Focus)

**Why is docking so simple?**

For the **Trade Simulator MVP**, the goal is:
- ✅ Test if trading gameplay is fun
- ✅ Get player feedback quickly
- ✅ Iterate on core loop (buy → sell → upgrade)

**Not the goal right now:**
- ❌ Cinematic docking sequences
- ❌ Manual docking like Elite Dangerous
- ❌ Realistic physics simulation

Once we **validate the trading loop is fun**, we can add polish like:
- Animated docking sequences
- Docking request/permission system
- Landing pad lights and effects
- Tractor beam visual effects

But first: **make it work, make it playable, make it fun!**

---

## 🎯 Next Steps

After following this guide:

1. **Test basic docking** in editor
2. Create 2-3 stations with different economies
3. Set up simple trade goods (water, food, metal)
4. **Playtest the loop**:
   - Dock at Station A
   - Buy cheap goods
   - Fly to Station B
   - Sell expensive goods
   - Profit!
5. Get feedback: Is this fun? What's confusing?
6. Iterate!

---

**Questions?** Check the code comments in:
- `Source/Adastrea/Public/Ships/Spaceship.h` (lines 338-396)
- `Source/Adastrea/Private/Ships/Spaceship.cpp` (lines 997-1214)

**Need help?** Ask in Discord or open a GitHub issue with:
- What you tried
- What happened
- What you expected
- Screenshots/logs if applicable

Happy docking! 🚀
