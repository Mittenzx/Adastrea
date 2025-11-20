# HUD Usage Guide for Adastrea

This guide explains how to use the enhanced HUD system to display player and ship information in Adastrea.

## Overview

The `UAdastreaHUDWidget` class provides a complete HUD system for displaying:
- **Player Information**: Name, level, and credits
- **Ship Status**: Health, shields, speed, and weapons
- **Ship Information**: Name, class, and hull integrity
- **Combat Information**: Target details and weapon status
- **Notifications**: Alerts and messages

## Table of Contents

1. [Quick Start](#quick-start)
2. [Player Information Display](#player-information-display)
3. [Ship Information Display](#ship-information-display)
4. [Ship Status Display](#ship-status-display)
5. [Blueprint Integration](#blueprint-integration)
6. [Auto-Update System](#auto-update-system)
7. [Examples](#examples)

---

## Quick Start

### Creating a HUD Widget Blueprint

1. In Unreal Editor, navigate to `Content/UI/HUD/`
2. The existing `WPB_SpaceShipHUD` Blueprint is based on `UAdastreaHUDWidget`
3. Open the Blueprint to customize the visual design

### Basic Setup in C++

```cpp
// In your player controller or game mode
UAdastreaHUDWidget* HUDWidget = CreateWidget<UAdastreaHUDWidget>(GetWorld(), HUDWidgetClass);
if (HUDWidget)
{
    HUDWidget->AddToViewport();
    HUDWidget->InitializeHUD();
}
```

### Basic Setup in Blueprint

1. In your Player Controller Blueprint, add the HUD widget to viewport on `BeginPlay`
2. Call `Initialize HUD` to set up default values
3. The HUD will automatically update based on the controlled spaceship

---

## Player Information Display

The HUD provides several functions to display player information:

### Update Player Name

```cpp
// C++
HUDWidget->UpdatePlayerName(FText::FromString("Captain Stellar"));
```

**Blueprint**: Call `Update Player Name` node with a Text parameter.

### Update Player Level

```cpp
// C++
HUDWidget->UpdatePlayerLevel(15);
```

**Blueprint**: Call `Update Player Level` node with an Integer parameter.

### Update Player Credits

```cpp
// C++
HUDWidget->UpdatePlayerCredits(50000);
```

**Blueprint**: Call `Update Player Credits` node with an Integer parameter.

### Update All Player Info at Once

```cpp
// C++
HUDWidget->UpdatePlayerInfo(
    FText::FromString("Captain Stellar"),
    15,    // Level
    50000  // Credits
);
```

**Blueprint**: Call `Update Player Info` node with Name, Level, and Credits parameters.

### Auto-Refresh Player Info

The HUD can automatically fetch player information from the Game Instance:

```cpp
// C++
HUDWidget->RefreshPlayerInfo();
```

**Blueprint**: Call `Refresh Player Info` node. This will:
1. Get the `UAdastreaGameInstance`
2. Fetch current player credits
3. Update the HUD displays automatically

---

## Ship Information Display

Display information about the player's current spaceship:

### Update Ship Name

```cpp
// C++
HUDWidget->UpdateShipName(FText::FromString("USS Enterprise"));
```

**Blueprint**: Call `Update Ship Name` node.

### Update Ship Class

```cpp
// C++
HUDWidget->UpdateShipClass(FText::FromString("Heavy Cruiser"));
```

**Blueprint**: Call `Update Ship Class` node.

### Update Ship Hull Integrity

```cpp
// C++
HUDWidget->UpdateShipIntegrity(
    750.0f,   // Current integrity
    1000.0f   // Max integrity
);
```

**Blueprint**: Call `Update Ship Integrity` node. This calculates and stores the percentage internally.

### Update All Ship Info at Once

```cpp
// C++
HUDWidget->UpdateShipInfo(
    FText::FromString("USS Enterprise"),
    FText::FromString("Heavy Cruiser"),
    750.0f,   // Current integrity
    1000.0f   // Max integrity
);
```

**Blueprint**: Call `Update Ship Info` node.

### Auto-Refresh Ship Info

The HUD can automatically fetch ship information from the controlled spaceship:

```cpp
// C++
HUDWidget->RefreshShipInfo();
```

**Blueprint**: Call `Refresh Ship Info` node. This will:
1. Get the currently controlled `ASpaceship`
2. Fetch ship name from the ship's `GetShipName()` function
3. Fetch ship class from the ship's `GetShipClass()` function
4. Fetch hull integrity from `CurrentHullIntegrity` and `MaxHullIntegrity`
5. Update all HUD displays automatically

---

## Ship Status Display

The HUD also provides status information about the ship:

### Update Health

```cpp
// C++
HUDWidget->UpdateHealth(
    800.0f,   // Current health
    1000.0f   // Max health
);
```

### Update Shields

```cpp
// C++
HUDWidget->UpdateShield(
    450.0f,  // Current shield
    500.0f   // Max shield
);
```

### Update Speed

```cpp
// C++
HUDWidget->UpdateSpeed(
    2500.0f,  // Current speed
    3000.0f   // Max speed
);
```

### Update Weapon Status

```cpp
// C++
HUDWidget->UpdateWeaponStatus(
    0,        // Weapon slot index
    85.0f,    // Current ammo/energy
    100.0f    // Max ammo/energy
);
```

---

## Blueprint Integration

### Creating Custom Display Logic

All update functions are `BlueprintNativeEvent`, which means you can override them in Blueprint:

1. Open your HUD Widget Blueprint
2. In the Functions panel, override any `Update...` function
3. Add your custom UI update logic
4. Optionally call the parent implementation

**Example: Custom UpdatePlayerCredits**

In Blueprint:
1. Override `UpdatePlayerCredits`
2. Get the Credits parameter
3. Format it as currency (e.g., "50,000 CR")
4. Update a Text Block widget
5. Optionally add animations or effects

### Accessing HUD State Variables

The HUD stores current values in Blueprint-readable properties:

- `CurrentPlayerName` (Text)
- `CurrentPlayerLevel` (Integer)
- `CurrentPlayerCredits` (Integer)
- `CurrentShipName` (Text)
- `CurrentShipClass` (Text)
- `ShipIntegrityPercent` (Float, 0.0 to 1.0)
- `HealthPercent` (Float, 0.0 to 1.0)
- `ShieldPercent` (Float, 0.0 to 1.0)
- `CurrentSpeedValue` (Float)
- `bHasTarget` (Boolean)

These can be accessed in Blueprint for data binding or custom logic.

---

## Auto-Update System

The HUD includes an automatic update system that runs every frame:

### How It Works

1. The HUD's `NativeTick` function calls `UpdateHUDFromGameState`
2. By default, it caches a reference to the controlled spaceship
3. You can override `UpdateHUDFromGameState` in Blueprint to add custom auto-update logic

### Example: Auto-Updating Player and Ship Info

In Blueprint:
1. Override `UpdateHUDFromGameState`
2. Call `Refresh Player Info` every few seconds (use a timer)
3. Call `Refresh Ship Info` every frame or on ship possession
4. Update health, shields, and speed from the spaceship component

### Performance Considerations

- Avoid calling expensive operations every frame
- Use timers for infrequent updates (like credits)
- Cache references to avoid repeated lookups
- Only update UI when values actually change

---

## Examples

### Example 1: Initializing the HUD on Game Start

**Blueprint (Player Controller):**
```
Event BeginPlay
  ├─ Create Widget (Class: WBP_SpaceShipHUD)
  ├─ Add to Viewport
  ├─ Initialize HUD
  ├─ Refresh Player Info
  └─ Refresh Ship Info
```

### Example 2: Updating Credits When Trading

**C++:**
```cpp
void AMyPlayerController::OnTradeCompleted(int32 CreditChange)
{
    // Update game instance
    UAdastreaGameInstance* GameInstance = GetGameInstance<UAdastreaGameInstance>();
    GameInstance->ModifyPlayerCredits(CreditChange);
    
    // Update HUD
    if (HUDWidget)
    {
        HUDWidget->RefreshPlayerInfo();
    }
}
```

### Example 3: Displaying Damage to Ship

**C++:**
```cpp
void ASpaceship::TakeDamage(float DamageAmount)
{
    CurrentHullIntegrity = FMath::Max(0.0f, CurrentHullIntegrity - DamageAmount);
    
    // HUD will auto-update on next RefreshShipInfo() call
    // Or manually update it:
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC && PC->MyHUD)
    {
        UAdastreaHUDWidget* HUD = Cast<UAdastreaHUDWidget>(PC->MyHUD->GetWidget());
        if (HUD)
        {
            HUD->UpdateShipIntegrity(CurrentHullIntegrity, MaxHullIntegrity);
        }
    }
}
```

### Example 4: Custom HUD Update in Blueprint

**Blueprint (WBP_SpaceShipHUD):**
```
Override: UpdateHUDFromGameState
  ├─ Get Controlled Spaceship
  ├─ Is Valid?
  │   ├─ Yes:
  │   │   ├─ Get Current Speed (from Spaceship)
  │   │   ├─ Update Speed (Speed, Max Speed)
  │   │   ├─ Get Hull Integrity
  │   │   └─ Update Ship Integrity
  │   └─ No: (do nothing)
  └─ Parent: UpdateHUDFromGameState
```

---

## Integration with SpaceshipDataAsset

The ship information system integrates with `USpaceshipDataAsset`:

### Setting Up a Spaceship with Data Asset

1. Create a `USpaceshipDataAsset` in the editor (e.g., `DA_Ship_Fighter`)
2. Configure all ship properties (name, class, hull strength, etc.)
3. In your Spaceship Blueprint:
   - Set the `Ship Data Asset` property to reference your data asset
4. When the ship spawns, it automatically initializes:
   - `MaxHullIntegrity` from `HullStrength` in the data asset
   - `CurrentHullIntegrity` to full health

### How It Works

**In `ASpaceship::BeginPlay()`:**
```cpp
if (ShipDataAsset)
{
    MaxHullIntegrity = ShipDataAsset->HullStrength;
    CurrentHullIntegrity = MaxHullIntegrity; // Start at full health
}
```

**In `ASpaceship::GetShipName()`:**
```cpp
if (ShipDataAsset)
{
    return ShipDataAsset->ShipName;
}
return FText::FromString(GetName()); // Fallback
```

**In `ASpaceship::GetShipClass()`:**
```cpp
if (ShipDataAsset)
{
    return ShipDataAsset->ShipClass;
}
return FText::FromString("Starship"); // Fallback
```

---

## Troubleshooting

### HUD Not Updating

**Problem**: HUD displays don't update when values change.

**Solutions**:
1. Ensure you're calling the update functions after changing values
2. Check that the HUD widget is added to viewport and visible
3. Verify the widget Blueprint overrides are calling update logic
4. Use `RefreshPlayerInfo()` and `RefreshShipInfo()` if unsure

### Missing Ship Information

**Problem**: Ship name shows as actor name, class shows as "Starship".

**Solutions**:
1. Assign a `SpaceshipDataAsset` to the ship's `Ship Data Asset` property
2. Ensure the data asset has `ShipName` and `ShipClass` filled in
3. The ship must be fully spawned (BeginPlay called) for data asset initialization

### Player Info Shows Defaults

**Problem**: Player name shows as "Captain", level as 1, credits as 0.

**Solutions**:
1. Implement proper save/load system integration
2. Store player data in `UAdastreaGameInstance`
3. Call `RefreshPlayerInfo()` after loading a save game
4. For now, manually call `UpdatePlayerInfo()` with correct values

### Performance Issues

**Problem**: HUD updates causing frame rate drops.

**Solutions**:
1. Don't update every property every frame
2. Use timers for infrequent updates (credits, level)
3. Only update when values actually change
4. Cache expensive lookups (don't call `GetGameInstance()` every frame)

---

## Best Practices

1. **Use Refresh Functions**: Call `RefreshPlayerInfo()` and `RefreshShipInfo()` when appropriate
2. **Override in Blueprint**: Customize display logic in Blueprint for designer-friendly updates
3. **Batch Updates**: Use `UpdatePlayerInfo()` and `UpdateShipInfo()` to update multiple values at once
4. **Cache References**: Store references to frequently accessed objects
5. **Check for Null**: Always validate pointers before use
6. **Update on Events**: Update HUD in response to game events, not constantly
7. **Use Data Assets**: Assign `SpaceshipDataAsset` to spaceships for automatic information

---

## Future Enhancements

Potential additions to the HUD system:

- **Crew Status**: Display active crew members and their roles
- **Mission Objectives**: Show current quest objectives
- **Faction Standing**: Display reputation with nearby factions
- **Resource Levels**: Show fuel, cargo, and supplies
- **Minimap**: Integrate with sector map system
- **Damage Indicators**: Show which ship systems are damaged
- **Communication Log**: Display recent messages and hails

---

## API Reference

### Player Info Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `UpdatePlayerName` | `FText PlayerName` | Update displayed player name |
| `UpdatePlayerLevel` | `int32 Level` | Update displayed player level |
| `UpdatePlayerCredits` | `int32 Credits` | Update displayed credits |
| `UpdatePlayerInfo` | `FText Name, int32 Level, int32 Credits` | Update all player info at once |
| `RefreshPlayerInfo` | None | Auto-fetch and update from Game Instance |

### Ship Info Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `UpdateShipName` | `FText ShipName` | Update displayed ship name |
| `UpdateShipClass` | `FText ShipClass` | Update displayed ship class |
| `UpdateShipIntegrity` | `float Current, float Max` | Update hull integrity display |
| `UpdateShipInfo` | `FText Name, FText Class, float Current, float Max` | Update all ship info at once |
| `RefreshShipInfo` | None | Auto-fetch and update from controlled spaceship |

### Ship Status Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `UpdateHealth` | `float Current, float Max` | Update health display |
| `UpdateShield` | `float Current, float Max` | Update shield display |
| `UpdateSpeed` | `float Current, float Max` | Update speed display |
| `UpdateWeaponStatus` | `int32 Index, float Current, float Max` | Update weapon ammo/energy |
| `UpdateTargetInfo` | `FText Name, float Distance, bool bIsHostile` | Update target info |
| `ClearTargetInfo` | None | Clear target display |
| `ShowAlert` | `FText Message, float Duration, bool bIsWarning` | Show alert message |

### Spaceship Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `GetShipName` | None | Get ship name (from data asset or actor) |
| `GetShipClass` | None | Get ship class (from data asset or default) |

### State Variables (Blueprint ReadOnly)

| Variable | Type | Description |
|----------|------|-------------|
| `CurrentPlayerName` | FText | Current player name |
| `CurrentPlayerLevel` | int32 | Current player level |
| `CurrentPlayerCredits` | int32 | Current player credits |
| `CurrentShipName` | FText | Current ship name |
| `CurrentShipClass` | FText | Current ship class |
| `ShipIntegrityPercent` | float | Hull integrity percentage (0-1) |
| `HealthPercent` | float | Health percentage (0-1) |
| `ShieldPercent` | float | Shield percentage (0-1) |
| `CurrentSpeedValue` | float | Current speed value |
| `bHasTarget` | bool | Whether a target is selected |
| `ControlledSpaceship` | ASpaceship* | Reference to controlled ship |

---

## Conclusion

The enhanced HUD system provides a flexible, Blueprint-friendly way to display player and ship information. By using the `BlueprintNativeEvent` pattern, designers can easily customize the visual presentation while programmers maintain the data management logic.

For questions or issues, refer to the main project documentation or contact the development team.

---

**Last Updated**: 2025-11-20  
**Version**: 1.0  
**Author**: Adastrea Development Team
