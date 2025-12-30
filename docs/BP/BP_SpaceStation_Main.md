# BP_SpaceStation_Main - Space Station Blueprint Guide

> **Main space station actor with modular attachment system for trading and docking**

**Blueprint Type**: Actor  
**Parent Class**: `ASpaceStation` (C++)  
**Location**: `Content/Blueprints/Stations/BP_SpaceStation_Main.uasset`  
**Priority**: 🔶 **HIGH** - Required for trading and docking in MVP

---

## 📋 Overview

`BP_SpaceStation_Main` represents a space station in the game world. It handles module attachments, faction ownership, docking, and market functionality for the trading system.

### Responsibilities

- ✅ Manage station modules (docking, storage, defense, etc.)
- ✅ Handle ship docking and undocking
- ✅ Provide market access for trading
- ✅ Track station ownership and faction
- ✅ Calculate station stats from attached modules
- ✅ Display station visual representation

---

## 🎨 Visual Flow Diagram

![Station Module Attachment Flow](../reference/images/blueprints/station_module_attachment.svg)

**Flow Explanation:**

1. **On Module Attached Event** - Triggered when new module added
2. **Validate Module Attachment** - Checks if attachment is valid
3. **Branch: Is Valid?** - Determines if attachment can proceed
4. **Attach Module to Station** - Physically attaches the module
5. **Update Station Stats** - Recalculates station capabilities

---

## 🔧 Prerequisites

### Required C++ Classes
- ✅ `ASpaceStation` - Parent class with station logic
- ✅ `ASpaceStationModule` - Module actor class
- ✅ `UFactionDataAsset` - Faction ownership
- ✅ `UMarketDataAsset` - Trading market configuration

### Required Blueprints
- ✅ Various `BP_SpaceStationModule_*` blueprints (Docking, Power, Storage, etc.)

### Required Data Assets
- ✅ `UFactionDataAsset` - Faction that owns this station
- ✅ `UMarketDataAsset` - Market inventory and prices

---

## 📦 Components Setup

### Core Components

```yaml
# Root and Mesh
SceneComponent (Root):
  Name: StationRoot
  Location: (0, 0, 0)

StaticMeshComponent:
  Name: StationCoreMesh
  Parent: StationRoot
  Static Mesh: SM_Station_Core
  Collision: BlockAll
  Simulate Physics: false

# Docking Points (Multiple)
SceneComponent (Array):
  Name: DockingPoints
  Count: 4-8 (depending on station size)
  Location: Positioned around station
  Purpose: Ship docking locations

# Module Attachment Points (Multiple)
SceneComponent (Array):
  Name: ModuleAttachmentPoints
  Count: 10-20 (depending on station design)
  Location: Positioned around core for modules
  Purpose: Module connection sockets
  
# Station Systems
SphereComponent:
  Name: DockingRange
  Sphere Radius: 2000 (collision sphere for docking range)
  Collision: Overlap All Dynamic
  Purpose: Detect ships in docking range
```

---

## 🏗️ Implementation Steps

### Step 1: Create the Blueprint

1. Content Browser → `Content/Blueprints/Stations/`
2. Right-click → Blueprint Class
3. Search for `SpaceStation` (your C++ class)
4. Name it `BP_SpaceStation_Main`
5. Open the Blueprint

### Step 2: Configure Station Properties

In Class Defaults:

```yaml
# Station Identity
StationName: "Trading Hub Alpha"
StationDescription: "Major trading station in the sector"

# Faction
FactionDataAsset: DA_Faction_TradingGuild

# Market
MarketDataAsset: DA_Market_TradingHub
EnableTrading: true

# Docking
MaxDockedShips: 4
DockingFee: 100 (credits to dock, 0 for free)
AutoDockingEnabled: true

# Modules
InitialModules:
  - DA_Module_DockingBay (x2)
  - DA_Module_Storage (x1)
  - DA_Module_Power (x1)
  - DA_Module_Habitation (x1)
```

### Step 3: Add Docking Points

1. Add Scene Components for docking points
2. Position them around the station
3. Name them: `DockingPoint_01`, `DockingPoint_02`, etc.
4. Add to array variable: `DockingPoints`

### Step 4: Implement Docking Logic

#### Event: On Begin Overlap (Docking Range)

```
EVENT: On Component Begin Overlap (DockingRange)
├─  Other Actor (AActor)
└─► Exec
    └─► Branch: Is Player Ship?
        ├─► True: Show docking prompt
        └─► False: Ignore
```

**Implementation:**
```
1. Cast to ASpaceship (Other Actor)
2. If valid:
   - Check if ship is player controlled
   - Check if docking points available
   - Show UI prompt: "Press F to Dock"
3. Wait for player input
4. Call DockShip function
```

#### Function: Dock Ship

```
FUNCTION: Dock Ship
├─► Exec (input)
├─  Ship (ASpaceship)
├─  Docking Point Index (int32)
└─► Exec (output)
    ├─  Success (bool)
    └─  Docked Location (Transform)
```

**Node Sequence:**
```
1. Validate Docking
   - Check docking point is available
   - Check ship has docking clearance
   - Check faction relationship (if hostile, deny)

2. Move Ship to Docking Point
   - Get docking point transform
   - Smooth move ship to position (lerp or timeline)
   - Disable ship physics
   - Disable ship input

3. Mark Docking Point as Occupied
   - Set array element to occupied
   - Store ship reference

4. Open Trading UI
   - Create WBP_TradingUI widget
   - Call OpenMarket(MarketDataAsset)
   - Add to viewport

5. Return Success
```

#### Function: Undock Ship

```
FUNCTION: Undock Ship
├─► Exec (input)
├─  Ship (ASpaceship)
└─► Exec (output)
    └─  Success (bool)
```

**Implementation:**
```
1. Find ship's docking point
2. Clear docking point (set to unoccupied)
3. Move ship away from station (offset 500 units)
4. Re-enable ship physics and input
5. Close trading UI if open
6. Charge docking fee (if any)
```

### Step 5: Implement Module Attachment

#### Event: On Module Attached

```
EVENT: On Module Attached (Custom Event)
├─  Module Actor (ASpaceStationModule)
├─  Attachment Point Index (int32)
└─► Exec
    └─► Call ValidateModuleAttachment
```

**Implementation:**
```
1. Validate Module Attachment
   - Check attachment point exists
   - Check attachment point is empty
   - Check module type is compatible
   - Check faction permissions

2. Attach Module to Station
   - Get attachment point transform
   - Attach actor to component
   - Call SetAttachedToStation on module
   - Add module to AttachedModules array

3. Update Station Stats
   - Recalculate total power
   - Recalculate total storage
   - Recalculate total defenses
   - Update UI if visible
```

### Step 6: Implement Market Access

#### Function: Open Station Market

```
FUNCTION: Open Station Market
├─► Exec (input)
├─  Player Controller (APlayerController)
└─► Exec (output)
```

**Called when player docks and requests trading.**

**Implementation:**
```
1. Check if player is docked
2. Check if trading is enabled
3. Check faction relationship (discount/markup)
4. Create Trading UI widget
5. Pass MarketDataAsset to widget
6. Show widget to player
```

---

## 🔌 Integration Points

### With Faction System

```cpp
// Station belongs to a faction
UFactionDataAsset* OwningFaction = GetFactionDataAsset();

// Faction affects trading prices
float PriceMultiplier = FactionManager->GetPriceMultiplier(
    PlayerFaction, OwningFaction);
```

### With Trading System

```cpp
// Station provides market access
UMarketDataAsset* Market = GetMarketDataAsset();
TradingUI->OpenMarket(Market);
```

### With Player Ship

```cpp
// Docking disables ship controls
ASpaceship* Ship = DockedShips[DockingPointIndex];
Ship->DisableInput(PlayerController);
Ship->SetActorEnableCollision(false);
```

---

## 🎮 Testing

### In-Editor Testing

1. **Place Station in Level**:
   - Drag BP_SpaceStation_Main into level
   - Position at (0, 0, 0) or desired location

2. **Test Docking Range**:
   - Fly player ship near station
   - Verify docking prompt appears
   - Press F (or docking key)
   - Ship should move to docking point

3. **Test Trading**:
   - After docked, trading UI should appear
   - Verify market items load
   - Test buying items
   - Undock and verify items in cargo

4. **Test Modules** (Optional):
   - Spawn module actors
   - Call attach module function
   - Verify module attaches visually
   - Check station stats update

### Verification Checklist

- [ ] Station spawns without errors
- [ ] Docking range collision works
- [ ] Can dock player ship
- [ ] Trading UI opens when docked
- [ ] Market items display correctly
- [ ] Can complete purchases
- [ ] Can undock ship
- [ ] Ship regains control after undocking
- [ ] Faction ownership displays
- [ ] No performance issues

---

## ⚠️ Common Issues

### Issue 1: Can't Dock

**Symptoms**: Docking prompt doesn't appear

**Causes**:
- Docking range collision not configured
- Overlap events not firing
- Ship not detected as player

**Solutions**:
1. Verify SphereComponent collision set to Overlap
2. Check overlap events bound in Event Graph
3. Test Cast to Spaceship succeeds
4. Verify player controller is valid

### Issue 2: Trading UI Doesn't Open

**Symptoms**: Dock works but no UI

**Causes**:
- Widget class not set
- Market Data Asset missing
- Widget creation fails

**Solutions**:
1. Verify WBP_TradingUI class exists
2. Set MarketDataAsset in station properties
3. Add debug logs before CreateWidget
4. Check viewport add succeeds

### Issue 3: Ship Stuck After Undocking

**Symptoms**: Can't control ship after undocking

**Causes**:
- Input not re-enabled
- Physics not restored
- Ship still marked as docked

**Solutions**:
1. Call EnableInput in undock function
2. Re-enable collision and physics
3. Clear docked flag on ship
4. Verify player controller is valid

---

## 🎯 MVP Checklist

For Trade Simulator MVP, this station must:

- [x] Support ship docking (automatic or manual)
- [x] Open trading UI when docked
- [x] Provide market inventory (10-20 items)
- [x] Allow undocking to return to flight
- [x] Display faction ownership
- [ ] Support multiple docked ships (nice to have)
- [ ] Module attachment system (Phase 2)
- [ ] Station missions/quests (not MVP)

---

## 🔗 Related Blueprints

**Required Before**:
- [BP_SpaceGameMode](BP_SpaceGameMode.md) - Places stations in world
- [BP_PlayerShip](BP_PlayerShip.md) - Ships that dock here

**Required With**:
- [WBP_TradingUI](WBP_TradingUI.md) - Trading interface opened here
- [BP_SpaceStationModule](BP_SpaceStationModule.md) - Attached modules

**Data Assets**:
- `UFactionDataAsset` - Station ownership
- `UMarketDataAsset` - Trading inventory

---

**Last Updated**: December 30, 2025  
**Status**: Essential for MVP  
**Difficulty**: ⭐⭐⭐ Intermediate-Advanced
