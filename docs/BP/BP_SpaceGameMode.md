# BP_SpaceGameMode - Game Mode Setup Guide

> **Main game mode that controls game rules, economy, faction systems, and player spawning**

**Blueprint Type**: Game Mode  
**Parent Class**: `AdastreaGameMode` (C++)  
**Location**: `Content/Blueprints/GameModes/BP_SpaceGameMode.uasset`  
**Priority**: ⚠️ **CRITICAL** - Game won't start without this

---

## 📋 Overview

`BP_SpaceGameMode` is the entry point for Adastrea. It initializes all core game systems on BeginPlay, manages game state, and handles player spawning.

### Responsibilities

- ✅ Initialize economy system and market prices
- ✅ Load and setup faction data and relationships
- ✅ Spawn player ship at starting location
- ✅ Configure game rules and win conditions
- ✅ Manage respawning and game over states

---

## 🎨 Visual Flow Diagram

![BP_SpaceGameMode Setup Flow](images/gamemode_setup.svg)

**Flow Explanation:**

1. **BeginPlay Event** triggers when game starts
2. **Initialize Economy** sets up the trading system
3. **Initialize Factions** loads faction data and relationships
4. **Spawn Player Ship** creates the player's starting ship

---

## 🔧 Prerequisites

Before creating this Blueprint:

### Required C++ Classes
- ✅ `AdastreaGameMode` - Parent class
- ✅ `AdastreaGameState` - Game state tracking
- ✅ `ASpaceship` - Player ship class
- ✅ `UEconomyManager` - Economy subsystem
- ✅ `UFactionManager` - Faction subsystem

### Required Data Assets
- ✅ At least one `USpaceshipDataAsset` for player's starting ship
- ✅ At least 3-5 `UFactionDataAsset` for initial factions
- ✅ Market data assets for initial trading stations

### Required Maps
- ✅ At least one level/map with PlayerStart actor placed

---

## 📦 Class Settings

### Default Classes Configuration

In the Blueprint's Class Defaults (right panel when Blueprint is open):

```
Default Pawn Class: BP_PlayerShip
Player Controller Class: BP_SpaceshipController
Game State Class: AdastreaGameState
Player State Class: PlayerState (default UE5 class)
HUD Class: None (handled by widgets)
```

### Game Mode Properties

```yaml
# Game Rules
bStartPlayersAsSpectators: false
bDelayedStart: false
bUseSeamlessTravel: false

# Player Settings
DefaultPawnClass: BP_PlayerShip
PlayerControllerClass: BP_SpaceshipController

# Starting Resources
StartingCredits: 10000
StartingShipDataAsset: DA_Ship_PlayerScout
StartingLocation: PlayerStart (from map)
```

---

## 🏗️ Implementation Steps

### Step 1: Create the Blueprint

1. In Content Browser, navigate to `Content/Blueprints/GameModes/`
2. Right-click → Blueprint Class
3. Search for `AdastreaGameMode` as parent
4. Name it `BP_SpaceGameMode`
5. Open the Blueprint

### Step 2: Configure Class Defaults

1. Click "Class Defaults" button (top toolbar)
2. In Details panel:
   - Set Default Pawn Class: `BP_PlayerShip`
   - Set Player Controller Class: `BP_SpaceshipController`
   - Set Game State Class: `AdastreaGameState`

### Step 3: Implement BeginPlay Logic

Open Event Graph and implement the following:

#### Event BeginPlay

```
EVENT: BeginPlay
└─► (execution output)
```

This is your entry point. Gets called when the game starts.

#### Initialize Economy System

```
FUNCTION: Initialize Economy
├─► Exec (input)
├─  Target: Get Economy Manager Subsystem
└─► Exec (output)
```

**Implementation:**
1. Add node: `Get Game Instance Subsystem` (select `UEconomyManager`)
2. Store result in local variable: `EconomyManager`
3. Call `Initialize` on EconomyManager
4. This loads market data and initializes price systems

**Verification:**
- Check logs for "Economy system initialized"
- Verify market prices are loaded

#### Initialize Faction System

```
FUNCTION: Initialize Factions
├─► Exec (input)
├─  Target: Get Faction Manager Subsystem
└─► Exec (output)
```

**Implementation:**
1. Add node: `Get Game Instance Subsystem` (select `UFactionManager`)
2. Store result in local variable: `FactionManager`
3. Call `LoadFactions` on FactionManager
4. Call `InitializeRelationships` to set up diplomacy

**Verification:**
- Check logs for "Loaded X factions"
- Verify faction relationships are initialized

#### Spawn Player Ship

```
FUNCTION: Spawn Player Ship
├─► Exec (input)
├─  Ship Data Asset (USpaceshipDataAsset)
├─  Spawn Location (Vector)
└─► Exec (output)
    └─  Spawned Ship (ASpaceship)
```

**Implementation:**
1. Get PlayerStart location:
   - Node: `Get All Actors of Class` (PlayerStart)
   - Node: `Get` (index 0)
   - Node: `Get Actor Location`

2. Create spawn parameters:
   - Node: `Make Transform` from location
   - Add rotation (0, 0, 0) for level spawn

3. Spawn the ship:
   - Node: `Spawn Actor from Class`
   - Class: `BP_PlayerShip`
   - Transform: from above
   - Collision: `Always Spawn`

4. Configure spawned ship:
   - Node: `Cast to Spaceship`
   - Node: `Set Ship Data Asset` (pass starting ship DA)
   - Node: `Possess` (by player controller)

**Verification:**
- Ship spawns at PlayerStart location
- Ship is possessed by player
- Camera follows ship
- HUD appears

---

## 🔌 Integration Points

### With Game Instance

```cpp
// Game Instance provides persistent storage
UAdastreaGameInstance* GameInstance = GetGameInstance<UAdastreaGameInstance>();
if (GameInstance)
{
    // Load player's saved credits, ship, etc.
    int32 Credits = GameInstance->GetPlayerCredits();
}
```

### With Economy System

```cpp
// Economy Manager handles all trading and market prices
UEconomyManager* Economy = GetWorld()->GetSubsystem<UEconomyManager>();
Economy->Initialize();  // Call in BeginPlay
```

### With Faction System

```cpp
// Faction Manager handles diplomacy and relationships
UFactionManager* Factions = GetWorld()->GetSubsystem<UFactionManager>();
Factions->LoadFactions();  // Call in BeginPlay
```

### With Player Controller

The Game Mode spawns and possesses the player's pawn, then the Player Controller takes over input handling.

---

## 🎮 Testing

### In-Editor Testing

1. **Set as Default Game Mode**:
   - Project Settings → Maps & Modes
   - Default GameMode: `BP_SpaceGameMode`

2. **Place in Level**:
   - Or override in World Settings for specific map
   - World Settings → Game Mode Override: `BP_SpaceGameMode`

3. **Add PlayerStart**:
   - Drag PlayerStart actor into level
   - Position where you want player to spawn

4. **Press Play**:
   - Should see initialization logs
   - Player ship should spawn
   - HUD should appear
   - Can control the ship

### Verification Checklist

- [ ] Game starts without errors
- [ ] Economy system initialized (check logs)
- [ ] Factions loaded (check logs)
- [ ] Player ship spawns at PlayerStart
- [ ] Player can control the ship
- [ ] HUD displays correctly
- [ ] No null reference warnings
- [ ] Frame rate is stable

### Debug Commands

Enable console with ` key, then test:

```
# Check if game mode is correct
GetAll AdastreaGameMode

# Verify economy system
EconomyManager.Status

# Check faction count
FactionManager.ListFactions

# Verify player ship
GetAll Spaceship
```

---

## ⚠️ Common Issues

### Issue 1: Player Ship Doesn't Spawn

**Symptoms**: Game starts but no ship visible

**Causes**:
- No PlayerStart in the level
- Default Pawn Class not set
- Spawn location is invalid

**Solutions**:
1. Add PlayerStart actor to level
2. Verify Default Pawn Class = `BP_PlayerShip` in Class Defaults
3. Check spawn location is valid (not inside collision)
4. Enable "Always Spawn" for collision handling

### Issue 2: Economy Not Initialized

**Symptoms**: Trading UI shows no items

**Causes**:
- Economy Manager subsystem not created
- Market data assets not loaded
- Initialize not called in BeginPlay

**Solutions**:
1. Verify `UEconomyManager` exists in C++
2. Check BeginPlay calls Initialize Economy
3. Ensure market data assets exist in Content/DataAssets/Trading/

### Issue 3: Can't Control Ship

**Symptoms**: Ship spawns but doesn't respond to input

**Causes**:
- Player Controller not set
- Ship not possessed
- Enhanced Input not configured

**Solutions**:
1. Set Player Controller Class to `BP_SpaceshipController`
2. Verify Possess node is called after spawn
3. Check `BP_SpaceshipController` sets up input correctly

### Issue 4: No HUD Appears

**Symptoms**: Ship works but UI is missing

**Causes**:
- HUD widget not created
- `BP_SpaceshipController` doesn't create widget
- Widget not added to viewport

**Solutions**:
1. Don't set HUD Class in Game Mode (use widgets instead)
2. Implement HUD creation in `BP_SpaceshipController`
3. Verify widget is added to viewport in controller

---

## 🔄 Advanced Configuration

### Multiple Starting Ships

Allow player to choose starting ship:

```
1. Create variable: TArray<USpaceshipDataAsset*> StartingShips
2. In BeginPlay, show ship selection UI
3. Wait for player choice
4. Spawn chosen ship
```

### Different Starting Locations

Support multiple spawn points:

```
1. Create variable: TArray<FName> StartLocations
2. In BeginPlay, determine start location (random or choice)
3. Get PlayerStart with matching tag
4. Spawn at that location
```

### Save Game Integration

Load player's saved state:

```
1. In BeginPlay, check for save game
2. If exists, load saved credits, ship, location
3. Spawn ship with saved data
4. Restore inventory and upgrades
```

---

## 📊 Performance Considerations

### Initialization Optimization

- Initialize systems in sequence (not parallel) for stability
- Use async loading for market data if large dataset
- Defer non-critical initialization to after player spawn

### Memory Management

- Economy and faction managers are singletons (one per game)
- Don't recreate on every level load
- Use subsystem pattern for automatic lifecycle management

---

## 🎯 MVP Checklist

For Trade Simulator MVP, this Game Mode must:

- [x] Spawn player ship with basic stats
- [x] Initialize economy with 5-10 stations
- [x] Load 3-5 factions with relationships
- [x] Support docking at stations
- [x] Enable trading UI when docked
- [ ] Handle respawning (not MVP required)
- [ ] Support save/load (basic implementation)
- [ ] Track progression metrics

---

## 🔗 Related Blueprints

**Required Next**:
- [BP_SpaceshipController](BP_SpaceshipController.md) - Player input handling
- [BP_PlayerShip](BP_PlayerShip.md) - The ship this spawns
- [BP_GameInstance](BP_GameInstance.md) - Persistent state storage

**Integrates With**:
- Economy Manager (C++) - Trading system
- Faction Manager (C++) - Diplomacy system
- All Data Assets - Configuration

---

## 📚 Additional Resources

### C++ Reference
- `Source/Adastrea/AdastreaGameMode.h` - Parent class definition
- `Source/Adastrea/AdastreaGameState.h` - Game state tracking

### Documentation
- [Game Mode Architecture](../../ARCHITECTURE.md#game-mode)
- [Subsystem Pattern](../../docs/development/SUBSYSTEM_PATTERN.md)
- [Trading MVP Guide](../mvp/TRADING_MVP_GUIDE.md)

### Unreal Engine Docs
- [Game Mode and Game State](https://docs.unrealengine.com/5.6/en-US/game-mode-and-game-state-in-unreal-engine/)
- [Player Controller](https://docs.unrealengine.com/5.6/en-US/player-controllers-in-unreal-engine/)

---

**Last Updated**: December 30, 2025  
**Status**: Essential for MVP  
**Difficulty**: ⭐⭐ Intermediate
