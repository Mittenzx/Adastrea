# Trading Ship (Player) - Blueprint Guide

> **⚠️ DEPRECATED**: This guide has been consolidated into the unified Trading and Docking Complete Guide.  
> **See**: [Trading and Docking Complete Guide](../docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)

---

> **Create the player's trading ship with flight controls and cargo management**

**Blueprint Type**: Pawn  
**Complexity**: Intermediate  
**Time to Complete**: 40-50 minutes  
**Last Updated**: 2026-01-08  
**Status**: **DEPRECATED** - Refer to unified guide above

---

## 📋 Overview

### What You'll Build

A player-controlled spaceship that can fly in 3D space, dock at trading stations, and carry cargo for trading. This is the core player avatar for the Trade Simulator MVP.

**End Result**: Players can fly their ship between stations, see their cargo space, and interact with trading terminals.

### Why This Blueprint Matters

The trading ship is the player's primary interface with the game world. Without it, players can't complete the core "buy low, sell high" gameplay loop.

---

## ✅ Prerequisites

Before starting, ensure you have:

- [ ] Unreal Engine 5.6+
- [ ] `FloatingPawnMovement` component available (standard UE5 component)
- [ ] `UCameraComponent` and `USpringArmComponent` available (standard UE5 components)
- [ ] `UCargoComponent` C++ class compiled (Ships module) - See [Ship Cargo System Guide](ShipCargoSystemGuide.md)
- [ ] At least one `USpaceshipDataAsset` created (e.g., `DA_Ship_CompactTrader`)

**If you're missing any prerequisites**, see:
- [Ship Cargo System Guide](ShipCargoSystemGuide.md) - For creating cargo component
- [MVP Quick Start](../docs/mvp/README.md) - For building the Ships module

---

## 🔗 Dependencies

### Other Blueprints This Needs

| Blueprint | Purpose | Location |
|-----------|---------|----------|
| `WBP_TradingUI` | Trading interface | `Content/UI/Trading/` |
| `BP_TradingStation` | Docking target | `Content/Blueprints/Stations/` |

### C++ Classes Used

| Class | Module | What It Provides |
|-------|--------|------------------|
| `UFloatingPawnMovement` | Engine | 3D flight physics and movement |
| `USpringArmComponent` | Engine | Camera boom for third-person view |
| `UCameraComponent` | Engine | Player's view of the ship |
| `UCargoComponent` | Ships | Cargo space and inventory management |
| `USpaceshipDataAsset` | Ships | Ship stats (cargo capacity, speed) |

---

## 🎯 Setup Checklist

### Step 1: Create the Blueprint

1. **Open Content Browser** in Unreal Editor
2. **Navigate to**: `Content/Blueprints/Ships/`
3. **Right-click** → Blueprint Class
4. **Choose Parent Class**: `Pawn`
5. **Name it**: `BP_TradingShip`
6. **Double-click** to open the Blueprint Editor

### Step 2: Add Components

In the **Components** panel, add these components:

1. **Static Mesh** (DefaultSceneRoot)
   - Rename to: `ShipMesh`
   - This will be your ship's 3D model
   
2. **Floating Pawn Movement**
   - Search: "Floating Pawn Movement"
   - Click "Add Component"
   
3. **Spring Arm**
   - Attach to: `ShipMesh`
   - This creates camera boom
   
4. **Camera**
   - Attach to: `SpringArm`
   - This is the player's view
   
5. **Cargo Component** (Custom)
   - Search: "Cargo Component"
   - This manages cargo inventory

### Step 3: Configure Components

**ShipMesh Settings:**
- Set **Static Mesh**: Choose a spaceship mesh (or use basic cube for prototype)
- Set **Collision**: Generate Overlap Events = `True`

**SpringArm Settings:**
- **Target Arm Length**: `800.0` (camera distance)
- **Use Pawn Control Rotation**: `True`
- **Socket Offset**: `Z = 200.0` (camera height above ship)

**Camera Settings:**
- No changes needed (defaults are fine)

**FloatingPawnMovement Settings:**
- **Max Speed**: `2000.0`
- **Acceleration**: `1500.0`
- **Deceleration**: `2000.0`
- **Turning Boost**: `8.0`

---

## 📦 Variables to Create

**Create these variables BEFORE implementing logic:**

### Variable 1: ShipData

- **Type**: `Spaceship Data Asset` (Object Reference)
- **Category**: `Ship Config`
- **Instance Editable**: Yes
- **Tooltip**: "Data asset containing ship stats (cargo capacity, speed, etc.)"
- **Default Value**: None (set per instance)

### Variable 2: CurrentCredits

- **Type**: `Integer`
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "Player's current money for trading"
- **Default Value**: `1000` (starting credits for MVP)

### Variable 3: CurrentStation

- **Type**: `Actor` (Object Reference)
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "Station the player is currently near/docked at"
- **Default Value**: None

### Variable 4: IsDocked

- **Type**: `Boolean`
- **Category**: `Trading`
- **Instance Editable**: No
- **Tooltip**: "True when ship is docked at a station"
- **Default Value**: `False`

### Variable 5: MaxSpeed

- **Type**: `Float`
- **Category**: `Ship Config`
- **Instance Editable**: No
- **Tooltip**: "Maximum flight speed from ship data"
- **Default Value**: `2000.0`

### Variable 6: CargoCapacity

- **Type**: `Integer`
- **Category**: `Ship Config`
- **Instance Editable**: No
- **Tooltip**: "Maximum cargo units from ship data"
- **Default Value**: `10` (starter ship capacity)

---

## 🔧 Functions to Create

**Create these functions BEFORE implementing the main logic:**

### Function 1: InitializeFromShipData

**Purpose**: Loads ship stats from Data Asset and configures components

**Inputs**:
- None (uses `ShipData` variable)

**Outputs**:
- None

**Implementation**: See [Step 1](#step-1-initialize-ship-from-data-asset) below

### Function 2: DockAtStation

**Purpose**: Handles docking logic when player approaches station

**Inputs**:
- `Station` (Actor Reference) - The station to dock at

**Outputs**:
- `Success` (Boolean) - True if docking succeeded

**Implementation**: See [Step 3](#step-3-implement-docking-logic) below

### Function 3: UndockFromStation

**Purpose**: Undocks ship and allows free flight

**Inputs**:
- None

**Outputs**:
- None

**Implementation**: See [Step 4](#step-4-implement-undocking-logic) below

### Function 4: GetAvailableCargoSpace

**Purpose**: Returns how much cargo space is left

**Inputs**:
- None

**Outputs**:
- `Space` (Integer) - Available cargo units

**Implementation**: See [Step 5](#step-5-implement-cargo-space-calculation) below

### Function 5: OpenTradingUI

**Purpose**: Opens trading interface when docked

**Inputs**:
- None

**Outputs**:
- None

**Implementation**: See [Step 6](#step-6-implement-trading-ui-opening) below

---

## 🛠️ Step-by-Step Implementation

### Step 1: Initialize Ship from Data Asset

**Goal**: Load ship stats from ShipData and apply to components

**Blueprint Nodes**:

```
Event BeginPlay
  ↓
Branch (Is Valid? ShipData)
  ├─ True:
  │   ↓
  │   Get Ship Data → Cargo Capacity
  │   ├─ Set CargoCapacity variable
  │   └─ CargoComponent → Set Max Capacity
  │   ↓
  │   Get Ship Data → Max Speed
  │   ├─ Set MaxSpeed variable
  │   └─ FloatingPawnMovement → Set Max Speed
  │   ↓
  │   Print String: "Ship initialized: [ShipName]"
  │
  └─ False:
      ↓
      Print String: "WARNING: No ShipData assigned!"
```

**Detailed Instructions**:

1. In the **Event Graph**, locate `Event BeginPlay`
2. **Drag from execution pin** and add `Branch` node
3. **Drag `ShipData` variable** to graph and choose "Get"
4. **Connect ShipData** to Branch's Condition pin
5. **From True pin**, add `Get Cargo Capacity` (from ShipData)
6. **Set CargoCapacity variable** with result
7. **Get CargoComponent** reference and call `Set Max Capacity`
8. **Repeat for MaxSpeed** using `Get Max Speed` from ShipData
9. **Add Print String** for debugging

**Expected Result**: 
- Console prints ship name on play
- Cargo component has correct capacity
- Movement component has correct max speed

---

### Step 2: Setup Input Handling

**Goal**: Configure keyboard/mouse input for flight controls

**Blueprint Nodes**:

```
Event InputAxis MoveForward (Axis Value)
  ↓
FloatingPawnMovement → Add Input Vector
  └─ World Direction: Forward Vector × Axis Value

Event InputAxis MoveRight (Axis Value)
  ↓
FloatingPawnMovement → Add Input Vector
  └─ World Direction: Right Vector × Axis Value

Event InputAxis MoveUp (Axis Value)
  ↓
FloatingPawnMovement → Add Input Vector
  └─ World Direction: Up Vector × Axis Value

Event InputAxis Turn (Axis Value)
  ↓
Add Controller Yaw Input
  └─ Val: Axis Value

Event InputAxis LookUp (Axis Value)
  ↓
Add Controller Pitch Input
  └─ Val: Axis Value
```

**Detailed Instructions**:

1. In **Event Graph**, right-click → Add Event → Input → Axis Events
2. Add these axis events:
   - `InputAxis MoveForward`
   - `InputAxis MoveRight`
   - `InputAxis MoveUp`
   - `InputAxis Turn`
   - `InputAxis LookUp`
3. For each movement axis:
   - Get `FloatingPawnMovement` component
   - Call `Add Input Vector`
   - Multiply axis value by appropriate vector (Forward/Right/Up)
4. For camera axes:
   - Call `Add Controller Yaw Input` (for Turn)
   - Call `Add Controller Pitch Input` (for LookUp)

**Expected Result**: 
- WASD moves ship forward/back/strafe
- Space/Ctrl moves ship up/down
- Mouse moves camera

**Note**: Input mappings must exist in Project Settings → Input. For MVP, use default settings or create:
- `MoveForward` → W/S keys
- `MoveRight` → A/D keys
- `MoveUp` → Space/Ctrl keys
- `Turn` → Mouse X
- `LookUp` → Mouse Y

---

### Step 3: Implement Docking Logic

**Goal**: Detect when player is near station and allow docking

**Blueprint Nodes**:

```
Function: DockAtStation (Station input)
  ↓
Branch (Is Valid? Station)
  ├─ True:
  │   ↓
  │   Set IsDocked = True
  │   ↓
  │   Set CurrentStation = Station
  │   ↓
  │   FloatingPawnMovement → Set Active (False)
  │   ↓
  │   Print String: "Docked at [Station Name]"
  │   ↓
  │   Return: True
  │
  └─ False:
      ↓
      Print String: "ERROR: Invalid station"
      ↓
      Return: False
```

**Detailed Instructions**:

1. Open the **DockAtStation** function you created
2. Add `Branch` node checking if Station input is valid
3. **If valid**:
   - Set `IsDocked` to `True`
   - Set `CurrentStation` to the Station input
   - Get `FloatingPawnMovement` and call `Set Active (False)`
   - Add debug print
   - Return `True`
4. **If invalid**:
   - Print error
   - Return `False`

**Expected Result**: 
- Ship stops moving when docked
- IsDocked flag set correctly
- Console confirms docking

---

### Step 4: Implement Undocking Logic

**Goal**: Allow player to leave station and resume flight

**Blueprint Nodes**:

```
Function: UndockFromStation
  ↓
Set IsDocked = False
  ↓
Set CurrentStation = None
  ↓
FloatingPawnMovement → Set Active (True)
  ↓
Print String: "Undocked - ready to fly"
```

**Detailed Instructions**:

1. Open the **UndockFromStation** function
2. Set `IsDocked` to `False`
3. Set `CurrentStation` to `None`
4. Get `FloatingPawnMovement` and call `Set Active (True)`
5. Add debug print

**Expected Result**: 
- Ship can move again after undocking
- Movement controls responsive

---

### Step 5: Implement Cargo Space Calculation

**Goal**: Calculate available cargo space for trading

**Blueprint Nodes**:

```
Function: GetAvailableCargoSpace (returns Integer)
  ↓
CargoComponent → Get Current Cargo Count
  ↓
CargoCapacity - Current Count
  ↓
Return: Available Space
```

**Detailed Instructions**:

1. Open the **GetAvailableCargoSpace** function
2. Get `CargoComponent` reference
3. Call `Get Current Cargo Count`
4. **Subtract** from `CargoCapacity` variable
5. **Return** the result

**Expected Result**: 
- Returns correct available space
- Used by trading UI to prevent over-buying

---

### Step 6: Implement Trading UI Opening

**Goal**: Open trading interface when docked at station

**Blueprint Nodes**:

```
Function: OpenTradingUI
  ↓
Branch (IsDocked?)
  ├─ True:
  │   ↓
  │   Branch (Is Valid? CurrentStation)
  │   ├─ True:
  │   │   ↓
  │   │   Create Widget (WBP_TradingUI)
  │   │   ↓
  │   │   Set Station Reference (CurrentStation)
  │   │   ↓
  │   │   Add to Viewport
  │   │   ↓
  │   │   Set Input Mode UI Only
  │   │   ↓
  │   │   Set Show Mouse Cursor (True)
  │   │
  │   └─ False:
  │       ↓
  │       Print String: "ERROR: No station reference"
  │
  └─ False:
      ↓
      Print String: "Must be docked to trade"
```

**Detailed Instructions**:

1. Open the **OpenTradingUI** function
2. Add `Branch` checking `IsDocked`
3. **If docked**:
   - Check if `CurrentStation` is valid
   - Call `Create Widget` (class: `WBP_TradingUI`)
   - Call widget's initialization function with `CurrentStation`
   - Call `Add to Viewport` on the widget
   - Get Player Controller → `Set Input Mode UI Only`
   - Get Player Controller → `Set Show Mouse Cursor (True)`
4. **If not docked**: Print error message

**Expected Result**: 
- Trading UI appears when docked
- Mouse cursor visible for UI interaction
- Can't open UI when flying

---

### Step 7: Add Interaction Trigger (Optional Input Binding)

**Goal**: Let player press key to dock/trade

**Blueprint Nodes**:

```
Event InputAction Interact
  ↓
Branch (IsDocked?)
  ├─ True:
  │   ↓
  │   OpenTradingUI
  │
  └─ False:
      ↓
      Sphere Trace for Actors
      ├─ Start: Ship Location
      ├─ End: Ship Location + Forward × 500
      └─ Radius: 200
      ↓
      Branch (Hit Something?)
      ├─ True:
      │   ↓
      │   Cast to BP_TradingStation
      │   ├─ Success:
      │   │   ↓
      │   │   DockAtStation (Hit Actor)
      │   │
      │   └─ Fail:
      │       ↓
      │       Print: "Not a trading station"
      │
      └─ False:
          ↓
          Print: "No station nearby"
```

**Detailed Instructions**:

1. Add `InputAction Interact` event (bind to E key in Project Settings)
2. Check `IsDocked` state
3. **If docked**: Call `OpenTradingUI`
4. **If not docked**:
   - Do sphere trace forward from ship
   - Check if hit actor is `BP_TradingStation`
   - If yes: Call `DockAtStation`
   - If no: Print helpful message

**Expected Result**: 
- Press E near station to dock
- Press E when docked to open trading UI
- Helpful messages if player is confused

---

## 🧪 Testing Your Blueprint

### Test 1: Ship Spawns and Moves

**What to Test**: Basic flight controls work

**Steps**:
1. Place `BP_TradingShip` in level or set as Default Pawn in GameMode
2. Press Play (PIE)
3. Use WASD to move ship
4. Use mouse to look around
5. Use Space/Ctrl to move up/down

**Success Criteria**: 
- Ship moves smoothly in all directions
- Camera follows ship
- Console shows "Ship initialized" message

**If it fails**: See [Troubleshooting: Ship Doesn't Move](#common-issue-1-ship-doesnt-move)

---

### Test 2: Docking Works

**What to Test**: Ship can dock at stations

**Steps**:
1. Place `BP_TradingStation` in level (500 units from ship)
2. Press Play
3. Fly ship toward station
4. When close, press E to dock
5. Check ship stops moving
6. Press E again to open trading UI

**Success Criteria**: 
- Console shows "Docked at [Station]"
- Ship becomes stationary
- Trading UI opens

**If it fails**: See [Troubleshooting: Docking Fails](#common-issue-2-docking-fails)

---

### Test 3: Cargo Capacity Loads from Data Asset

**What to Test**: Ship stats load correctly from ShipData

**Steps**:
1. Create `DA_Ship_CompactTrader` with Cargo Capacity = 10
2. Assign to ship's `ShipData` variable
3. Press Play
4. Check console for initialization message
5. Try to open trading UI and check cargo space

**Success Criteria**: 
- Console shows "Ship initialized: Compact Trader"
- Trading UI shows "Cargo: 0/10"
- Can't buy more than 10 units

**If it fails**: See [Troubleshooting: Ship Data Not Loading](#common-issue-3-ship-data-not-loading)

---

### Test 4: Undocking Restores Movement

**What to Test**: Ship can undock and fly again

**Steps**:
1. Dock at station
2. Open trading UI
3. Close UI (ESC key - implement close function)
4. Call `UndockFromStation` (or press U key if bound)
5. Try to fly ship

**Success Criteria**: 
- Ship can move again after undocking
- Console shows "Undocked"
- IsDocked = False

**If it fails**: See [Troubleshooting: Can't Move After Undocking](#common-issue-4-cant-move-after-undocking)

---

## 🐛 Troubleshooting

### Common Issue 1: Ship Doesn't Move

**Symptoms**:
- WASD keys do nothing
- Ship stays in place
- No movement at all

**Causes**:
- Input mappings not configured
- FloatingPawnMovement component inactive
- Ship not possessed by player controller

**Solutions**:
1. Check **Project Settings → Input**:
   - Add `MoveForward` axis (W = 1.0, S = -1.0)
   - Add `MoveRight` axis (D = 1.0, A = -1.0)
   - Add `MoveUp` axis (Space = 1.0, Ctrl = -1.0)
2. Check **FloatingPawnMovement** in Components panel:
   - Verify "Active" is checked
   - Max Speed > 0
3. Check **World Settings → GameMode Override**:
   - Default Pawn Class = `BP_TradingShip`
   - Or ensure Auto Possess Player = Player 0

---

### Common Issue 2: Docking Fails

**Symptoms**:
- Press E near station, nothing happens
- Console shows "No station nearby"
- Can't dock even when touching station

**Causes**:
- Station too far away (>500 units)
- Sphere trace radius too small
- Station not a `BP_TradingStation` class
- Interact input not bound

**Solutions**:
1. **Increase trace distance**:
   - In `InputAction Interact` event
   - Change Forward × 500 to Forward × 1000
2. **Increase trace radius**:
   - Change Radius: 200 to Radius: 500
3. **Verify station class**:
   - Select station in World Outliner
   - Check Details panel → Blueprint class
   - Should be `BP_TradingStation` or child class
4. **Check input binding**:
   - Project Settings → Input
   - Add Action Mapping: `Interact` → E key

---

### Common Issue 3: Ship Data Not Loading

**Symptoms**:
- Console shows "WARNING: No ShipData assigned"
- Cargo capacity = 0
- Ship speed incorrect

**Causes**:
- ShipData variable not set in Blueprint instance
- Data Asset not created
- Data Asset properties not configured

**Solutions**:
1. **Assign ShipData**:
   - Select ship in level (or Blueprint defaults)
   - In Details panel, find `ShipData`
   - Click dropdown and select `DA_Ship_CompactTrader`
2. **Create Data Asset if missing**:
   - Content Browser → Right-click
   - Miscellaneous → Data Asset
   - Choose `SpaceshipDataAsset` class
   - Name it `DA_Ship_CompactTrader`
3. **Configure Data Asset**:
   - Open Data Asset
   - Set Cargo Capacity = 10
   - Set Max Speed = 2000
   - Set Ship Name = "Compact Trader"

---

### Common Issue 4: Can't Move After Undocking

**Symptoms**:
- After closing trading UI, ship still frozen
- WASD keys don't work after undocking
- Ship stuck at station

**Causes**:
- UndockFromStation not called
- FloatingPawnMovement still inactive
- Input mode still set to UI Only

**Solutions**:
1. **Call UndockFromStation** when closing UI:
   - In `WBP_TradingUI` close function
   - Get reference to player ship
   - Call `UndockFromStation` function
2. **Restore input mode**:
   - After undocking
   - Get Player Controller
   - Call `Set Input Mode Game Only`
   - Call `Set Show Mouse Cursor (False)`
3. **Verify movement component active**:
   - In `UndockFromStation` function
   - Ensure `Set Active (True)` is called
   - Check in debugger if component is active

---

### Common Issue 5: Trading UI Shows Wrong Cargo Space

**Symptoms**:
- Trading UI shows incorrect available cargo
- Can buy more than ship capacity
- Cargo space negative

**Causes**:
- GetAvailableCargoSpace calculation wrong
- CargoCapacity not initialized
- CargoComponent not updating count

**Solutions**:
1. **Check calculation logic**:
   - Open `GetAvailableCargoSpace` function
   - Verify: CargoCapacity - Current Count
   - Add Print String to debug values
2. **Verify CargoCapacity initialized**:
   - In `Event BeginPlay`
   - Check if `Set CargoCapacity` is called
   - Check ShipData has valid value
3. **Update cargo count**:
   - When buying/selling items
   - Call `CargoComponent → Add Item` or `Remove Item`
   - Verify component tracks count correctly

---

## ✨ Optional Enhancements

**After the basic Blueprint works, consider adding:**

1. **Ship Visual Indicators**
   - What it adds: Lights, thrust effects, cargo bay visualization
   - Complexity: Simple (add particle systems)
   - Guide: Add particle system components for engine thrust

2. **Speed Boost Mechanic**
   - What it adds: Temporary speed increase (hold Shift)
   - Complexity: Simple
   - Implementation: Add InputAction, multiply MaxSpeed temporarily

3. **Auto-Dock Feature**
   - What it adds: Automatic approach and docking
   - Complexity: Medium (requires pathfinding)
   - Guide: See Navigation System Guide

4. **Fuel System**
   - What it adds: Fuel consumption, refueling at stations
   - Complexity: Medium
   - Note: Consider for Phase 2, not MVP

5. **Ship Damage/Health**
   - What it adds: Health points, repair at stations
   - Complexity: Medium
   - Note: Out of MVP scope (no combat)

---

## 📚 Related Guides

**If you want to learn more:**

- [Ship Cargo System Guide](ShipCargoSystemGuide.md) - How cargo component works
- [Trading UI Widget Guide](TradingSystemBlueprintGuide_SIMPLIFIED.md) - Trading interface
- [Trading Station Actor Guide](TradingStationActorGuide.md) - Station docking
- [Docking UI Guide](DockingUIGuide.md) - Docking prompts
- [Ship Data Assets](../Assets/SpaceshipSystemGuide.md) - Creating ship stats

---

## 🎓 What You Learned

After completing this guide, you now know:

- ✅ How to create a player-controlled Pawn in Unreal Engine
- ✅ How to use FloatingPawnMovement for 3D flight
- ✅ How to implement docking/undocking logic
- ✅ How to integrate custom components (CargoComponent)
- ✅ How to load configuration from Data Assets
- ✅ How to open and close UI widgets from Blueprint

**Next Recommended Blueprint**: [Trading Station Actor](TradingStationActorGuide.md) - Create stations to dock at

---

## 📝 Notes for Advanced Users

**Customization Points:**
- Modify flight physics in FloatingPawnMovement settings
- Change docking detection range (sphere trace radius/distance)
- Add custom events for cargo changes
- Implement auto-save on successful trades

**Blueprint Best Practices Applied:**
- Used components for modularity (Cargo, Movement, Camera)
- Loaded configuration from Data Assets (ship stats)
- Validated references before using (Is Valid checks)
- Added debug prints for troubleshooting

**Performance Considerations:**
- Sphere trace only runs on input action (not every frame)
- Movement calculations handled by C++ component
- UI only created when needed (not persistent)

---

## 🔄 Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-08 | Initial guide for MVP Phase 1 |

---

**Questions or Issues?** 
- Check the [Blueprint Guide for Beginners](../docs/development/BLUEPRINT_GUIDE_FOR_BEGINNERS.md)
- Review the [Blueprint Image Guide](../docs/reference/BLUEPRINT_IMAGE_GUIDE.md) for node documentation
- See [MVP Documentation](../docs/mvp/README.md)

---

**Guide created for**: Trade Simulator MVP (Phase 1)  
**Maintained by**: Adastrea Development Team
