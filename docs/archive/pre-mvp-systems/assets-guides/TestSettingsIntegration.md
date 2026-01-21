# Test Settings Widget Integration Guide

## Overview

The Test Settings Widget is now integrated with the Adastrea GameMode, allowing it to be displayed as the first screen when you click play. This is useful for development and QA testing to configure test parameters before gameplay begins.

## How It Works

When the game starts:
1. The GameMode's `BeginPlay()` checks if test settings should be shown
2. If enabled, it creates and displays the Test Settings Widget
3. The player configures test settings (ship type, difficulty, debug mode)
4. When the player clicks "Continue", the widget notifies the GameMode
5. The GameMode continues with normal initialization (spawning the player ship)

## Setup Instructions

### 1. Create or Configure Your GameMode Blueprint

1. Open your GameMode Blueprint (or create one based on `AdastreaGameMode`)
2. In the Class Defaults panel, find the **Test Settings** category
3. Configure the following properties:

   - **Test Settings Widget Class**: Set this to your Test Settings Widget Blueprint (e.g., `WBP_TestSettings`)
   - **Show Test Settings On Startup**: Check this box to enable the test settings screen

### 2. Create Your Test Settings Widget Blueprint

If you haven't already created a Test Settings Widget Blueprint:

1. Navigate to `Content/UI/` in the Content Browser
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_TestSettings`
4. Open the Blueprint and go to File → Reparent Blueprint
5. Select `TestSettingsWidget` as the parent class
6. Design your UI layout (see `TestSettingsWidget_Usage.md` for details)

### 3. Configure Your Level

1. Open your level in the editor
2. Go to World Settings
3. Set your GameMode Override to your configured GameMode Blueprint
4. Ensure your GameMode has:
   - `DefaultSpaceshipClass` set to a valid spaceship Blueprint
   - `bAutoSpawnPlayerShip` enabled
   - `TestSettingsWidgetClass` set to your test settings widget
   - `bShowTestSettingsOnStartup` enabled

## Usage Flow

```
Game Starts (BeginPlay)
    ↓
Is TestSettingsWidgetClass set AND bShowTestSettingsOnStartup enabled?
    ├─ Yes → Create and show Test Settings Widget
    │         ↓
    │         Player configures settings
    │         ↓
    │         Player clicks "Continue"
    │         ↓
    │         Widget calls GameMode.OnTestSettingsContinue()
    │         ↓
    │         Widget is removed
    │         ↓
    │         GameMode proceeds with ship spawning
    │
    └─ No → GameMode proceeds directly with ship spawning
```

## Properties

### AdastreaGameMode Properties

#### Test Settings Category

- **TestSettingsWidgetClass** (TSubclassOf<UTestSettingsWidget>)
  - The Widget Blueprint class to display on startup
  - Leave empty to disable test settings
  - Default: `nullptr`

- **bShowTestSettingsOnStartup** (bool)
  - Whether to show the test settings widget on game start
  - Only has effect if TestSettingsWidgetClass is set
  - Default: `true`

#### Player Spawn Category

- **DefaultSpaceshipClass** (TSubclassOf<ASpaceship>)
  - The spaceship Blueprint to spawn for the player
  - Required for auto-spawn to work

- **bAutoSpawnPlayerShip** (bool)
  - Whether to automatically spawn a player ship after test settings
  - Default: `true`

- **bSpawnAtCenter** (bool)
  - If true, spawn at sector center
  - If false, spawn at random position
  - Default: `false`

## Functions

### BlueprintCallable Functions

#### OnTestSettingsContinue()

Called when the test settings widget confirms continuation. This function:
1. Cleans up the test settings widget
2. Proceeds with normal game initialization (ship spawning)
3. Can be called manually from Blueprint if needed

**Usage in Blueprint:**
```
Custom Event
  ↓
Call "OnTestSettingsContinue"
  ↓
Game initialization proceeds
```

#### SpawnPlayerSpaceship()

Spawns the player spaceship in the sector and possesses it. This is now called:
- Automatically from `OnTestSettingsContinue()` if `bAutoSpawnPlayerShip` is enabled
- Can be called manually for custom spawn logic

## Disabling Test Settings

To disable the test settings screen for production builds:

### Option 1: In GameMode Blueprint
- Set `bShowTestSettingsOnStartup` to `false`
- The game will skip directly to spawning the ship

### Option 2: Clear the Widget Class
- Set `TestSettingsWidgetClass` to `None`
- The game will skip directly to spawning the ship

### Option 3: Conditional Blueprint Logic
```
Event BeginPlay (in GameMode Blueprint)
  ↓
Is Shipping Build?
  ├─ Yes → Set "bShowTestSettingsOnStartup" to false
  └─ No → Keep default (true)
  ↓
Parent: BeginPlay
```

## Advanced Customization

### Custom Continuation Logic

You can override the continuation behavior in your GameMode Blueprint:

```
Event OnTestSettingsContinue (Override)
  ↓
Get Test Settings Widget
  ↓
Get Selected Ship Type
  ↓
Branch: Custom validation logic
  ├─ Valid → Parent: OnTestSettingsContinue
  └─ Invalid → Show error message, don't continue
```

### Reading Test Settings

The test settings are stored in the widget and can be accessed:

```cpp
// C++ Example
if (TestSettingsWidget)
{
    FString ShipType = TestSettingsWidget->GetSelectedShipType();
    FString Difficulty = TestSettingsWidget->GetSelectedDifficulty();
    bool bAutoSpawn = TestSettingsWidget->GetAutoSpawnEnabled();
    
    // Use these values for custom spawn logic
}
```

```
Blueprint Example:
  Get Test Settings Widget
    ↓
  Get Selected Ship Type
    ↓
  Use in custom spawn logic
```

### Multiple Test Screens

You can chain multiple test/configuration screens:

```
Event OnTestSettingsContinue (Override)
  ↓
Remove current widget
  ↓
Branch: Need another config screen?
  ├─ Yes → Create and show next config widget
  └─ No → Parent: OnTestSettingsContinue
```

## Troubleshooting

### Widget doesn't appear

**Check:**
1. `TestSettingsWidgetClass` is set in GameMode
2. `bShowTestSettingsOnStartup` is enabled
3. Widget Blueprint is properly parented to `TestSettingsWidget`
4. GameMode is set as the level's GameMode Override

### Widget appears but continue doesn't work

**Check:**
1. Continue button is bound to `OnContinueClicked` function
2. GameMode has `OnTestSettingsContinue` function (should be automatic with AdastreaGameMode)
3. Check the log for "TestSettingsWidget: Notified GameMode to continue"

### Ship doesn't spawn after continue

**Check:**
1. `bAutoSpawnPlayerShip` is enabled in GameMode
2. `DefaultSpaceshipClass` is set to a valid spaceship Blueprint
3. A `SpaceSectorMap` actor exists in the level
4. Check the log for spawn-related errors

## Best Practices

### DO:
✓ Use for development and QA testing builds
✓ Disable in shipping builds
✓ Test with and without the widget enabled
✓ Log all settings for debugging
✓ Validate settings before applying

### DON'T:
✗ Show in final release builds (unless intended)
✗ Hardcode settings that should be configurable
✗ Skip validation of user input
✗ Forget to handle the case where widget class is not set

## Example Configuration

### Development Build
```
GameMode Settings:
  - TestSettingsWidgetClass: WBP_TestSettings
  - bShowTestSettingsOnStartup: true
  - DefaultSpaceshipClass: BP_Fighter
  - bAutoSpawnPlayerShip: true
```

### Production Build
```
GameMode Settings:
  - TestSettingsWidgetClass: None (or keep but disable)
  - bShowTestSettingsOnStartup: false
  - DefaultSpaceshipClass: BP_Fighter
  - bAutoSpawnPlayerShip: true
```

## Logging

The system logs all events to the `LogAdastrea` category:

```
LogAdastrea: AdastreaGameMode: Showing test settings widget on startup
LogAdastrea: AdastreaGameMode: Test settings widget created and added to viewport
LogAdastrea: TestSettingsWidget: Widget constructed
LogAdastrea: TestSettingsWidget: Settings initialized
LogAdastrea: TestSettingsWidget: Continue clicked
LogAdastrea: TestSettingsWidget: Notified GameMode to continue
LogAdastrea: AdastreaGameMode: Test settings confirmed, continuing with game initialization
LogAdastrea: AdastreaGameMode: Successfully spawned player spaceship
```

Enable verbose logging:
```
Log LogAdastrea Verbose
```

## Related Documentation

- [TestSettingsWidget_Usage.md](../Source/Adastrea/UI/TestSettingsWidget_Usage.md) - Complete widget usage guide
- [AdastreaGameMode.h](../Source/Adastrea/AdastreaGameMode.h) - GameMode class reference
- [BLUEPRINT_CREATION_GUIDE.md](../BLUEPRINT_CREATION_GUIDE.md) - Blueprint setup guide

## Version History

- **v1.0** (Current): Initial integration
  - Test settings widget can be shown on game startup
  - GameMode waits for test settings confirmation before spawning ship
  - Backward compatible (widget is optional)
  - Blueprint-friendly configuration
