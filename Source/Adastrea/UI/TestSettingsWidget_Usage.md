# TestSettingsWidget Usage Guide

## Overview

The `TestSettingsWidget` is a C++ base class for creating a test configuration screen that appears before the main menu. It provides dropdown menus for configuring testing parameters such as ship type, difficulty level, and debug modes. This is particularly useful for QA testing, development builds, and rapid iteration.

## Quick Start

### 1. Create a Widget Blueprint

1. In Unreal Editor, navigate to `Content/UI/Menus/`
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_TestSettings`
4. Open the Widget Blueprint
5. Go to File → Reparent Blueprint
6. Select `TestSettingsWidget` as the parent class

### 2. Design the Layout

Create a layout with dropdown menus and buttons:

```
Canvas Panel (Root)
├── Image - Background
│   └── Set to fill entire screen with semi-transparent background
├── Vertical Box (Center)
│   ├── Text - Title "Test Configuration"
│   ├── Spacer (20px)
│   ├── Horizontal Box - Ship Type
│   │   ├── Text - Label "Ship Type:"
│   │   └── ComboBox (String) - ShipTypeComboBox
│   ├── Spacer (10px)
│   ├── Horizontal Box - Difficulty
│   │   ├── Text - Label "Difficulty:"
│   │   └── ComboBox (String) - DifficultyComboBox
│   ├── Spacer (10px)
│   ├── Horizontal Box - Debug Mode
│   │   ├── Text - Label "Debug Mode:"
│   │   └── ComboBox (String) - DebugModeComboBox
│   ├── Spacer (10px)
│   ├── CheckBox - Auto Spawn Ship
│   │   └── Text - "Auto-spawn selected ship"
│   ├── Spacer (20px)
│   ├── Horizontal Box - Buttons
│   │   ├── Button - Continue
│   │   │   └── Text - "Continue to Menu"
│   │   └── Button - Reset
│   │       └── Text - "Reset Defaults"
```

### 3. Populate ComboBox Options

In the Widget Blueprint's Event Graph:

**Event Pre Construct:**
```
Event Pre Construct
  ↓
Get Available Ship Types
  ↓
For Each Loop → Add Option to ShipTypeComboBox
  ↓
Get Available Difficulties
  ↓
For Each Loop → Add Option to DifficultyComboBox
  ↓
Get Available Debug Modes
  ↓
For Each Loop → Add Option to DebugModeComboBox
```

**Set Default Selections:**
```
Event Construct
  ↓
Get Selected Ship Type
  ↓
Set Selected Option (ShipTypeComboBox)
  ↓
Get Selected Difficulty
  ↓
Set Selected Option (DifficultyComboBox)
  ↓
Get Selected Debug Mode
  ↓
Set Selected Option (DebugModeComboBox)
  ↓
Get Auto Spawn Enabled
  ↓
Set Checked State (AutoSpawnCheckBox)
```

### 4. Bind Widget Events

**Ship Type ComboBox:**
```
OnSelectionChanged
  ↓
Get Selected Option
  ↓
Get Selected Index
  ↓
Call "OnShipTypeChanged" (Pass: SelectedOption, SelectedIndex)
```

**Difficulty ComboBox:**
```
OnSelectionChanged
  ↓
Get Selected Option
  ↓
Get Selected Index
  ↓
Call "OnDifficultyChanged" (Pass: SelectedOption, SelectedIndex)
```

**Debug Mode ComboBox:**
```
OnSelectionChanged
  ↓
Get Selected Option
  ↓
Get Selected Index
  ↓
Call "OnDebugModeChanged" (Pass: SelectedOption, SelectedIndex)
```

**Auto Spawn CheckBox:**
```
OnCheckStateChanged
  ↓
Is Checked?
  ↓
Call "SetAutoSpawnEnabled" (Pass: IsChecked)
```

**Continue Button:**
```
OnClicked → Call "OnContinueClicked"
```

**Reset Button:**
```
OnClicked → Call "OnResetClicked"
```

### 5. Override TransitionToMainMenu

In the Event Graph, override the transition behavior:

```
Event TransitionToMainMenu (Override)
  ↓
Remove From Parent (this widget)
  ↓
Create Widget (Class: WBP_MainMenu)
  ↓
Add to Viewport
```

### 6. Display the Widget

In your startup level's Level Blueprint or custom GameMode:

```
Event BeginPlay
  ↓
Create Widget (Class: WBP_TestSettings)
  ↓
Add to Viewport
```

## Core Features

### Dropdown Settings

The widget provides three main configuration dropdowns:

1. **Ship Type**: Select which ship to spawn for testing
   - Default Fighter, Scout Ship, Cargo Hauler, Gunship, etc.
   - Values can be customized in Blueprint or C++

2. **Difficulty**: Configure gameplay difficulty
   - Very Easy, Easy, Normal, Hard, Very Hard
   - Can affect AI behavior, damage scaling, etc.

3. **Debug Mode**: Enable debug visualizations
   - None, Show FPS, Show Collision, Show Navigation, etc.
   - Useful for development and debugging

### Auto-Spawn Option

- Checkbox to enable/disable automatic ship spawning
- When enabled, the selected ship will spawn when gameplay starts
- Integrates with GameMode's ship spawning system

### Settings Persistence

Settings are stored in the widget and can be accessed by:
- GameMode during gameplay initialization
- PlayerController for input/control configuration
- GameInstance for cross-level persistence (if implemented)

## Blueprint Customization

### Override Event Handlers

You can override any `_Implementation` function:

1. **OnShipTypeChanged**: React to ship selection changes
2. **OnDifficultyChanged**: React to difficulty changes
3. **OnDebugModeChanged**: Apply debug settings immediately
4. **ApplySettings**: Custom logic for applying settings
5. **TransitionToMainMenu**: Create and show main menu widget
6. **LoadDefaultSettings**: Customize default values

### Example: Custom Ship Spawning

```
Event OnContinueClicked (Override)
  ↓
Get Selected Ship Type
  ↓
Branch: Is "Battlecruiser"?
    ├─ Yes → Show Warning Dialog ("Large ship, may affect performance")
    └─ No → Parent: OnContinueClicked
```

### Example: Apply Debug Settings

```
Event OnDebugModeChanged (Override)
  ↓
Parent: OnDebugModeChanged
  ↓
Branch: SelectedDebugMode
  ├─ "Show FPS" → Execute Console Command "stat fps"
  ├─ "Show Collision" → Execute Console Command "show collision"
  ├─ "God Mode" → Enable Cheat Manager God Mode
  └─ "None" → Disable all debug modes
```

### Example: Save Settings to Game Instance

```
Event ApplySettings (Override)
  ↓
Get Game Instance
  ↓
Cast to Custom Game Instance
  ↓
Set Test Ship Type (SelectedShipType)
  ↓
Set Test Difficulty (SelectedDifficulty)
  ↓
Set Test Debug Mode (SelectedDebugMode)
  ↓
Parent: ApplySettings
```

## Integration with GameMode

### Reading Test Settings

In your GameMode or PlayerController:

```cpp
// C++ Example
UTestSettingsWidget* TestSettings = Cast<UTestSettingsWidget>(
    FindWidgetOfClass<UTestSettingsWidget>()
);

if (TestSettings)
{
    FString ShipType = TestSettings->GetSelectedShipType();
    bool bShouldSpawn = TestSettings->GetAutoSpawnEnabled();
    
    if (bShouldSpawn)
    {
        SpawnShipOfType(ShipType);
    }
}
```

```
Blueprint Example:
  Get All Widgets Of Class (UTestSettingsWidget)
    ↓
  Get First Element
    ↓
  Get Selected Ship Type
    ↓
  Use for spawning logic
```

### Integration with Ship Spawning

In your GameMode's `BeginPlay`:

```
Event BeginPlay
  ↓
Find Widget (WBP_TestSettings)
  ↓
Is Valid?
    ├─ Yes → Get Settings and spawn accordingly
    └─ No → Use default gameplay setup
```

## Properties

### Configuration Properties (EditAnywhere, BlueprintReadWrite)

- **AvailableShipTypes** (Array<String>): Ship types for dropdown
- **AvailableDifficulties** (Array<String>): Difficulty levels
- **AvailableDebugModes** (Array<String>): Debug mode options
- **bShowMainMenuAfter** (Boolean): Show main menu after applying

### State Properties (BlueprintReadOnly)

- **SelectedShipType** (String): Currently selected ship
- **SelectedDifficulty** (String): Currently selected difficulty
- **SelectedDebugMode** (String): Currently selected debug mode
- **bAutoSpawnShip** (Boolean): Auto-spawn enabled state
- **bIsSettingsVisible** (Boolean): Widget visibility state

## Functions

### BlueprintCallable Functions

```cpp
// Getters for current selections
GetSelectedShipType() → String
GetSelectedDifficulty() → String
GetSelectedDebugMode() → String
GetAutoSpawnEnabled() → Boolean

// Setters
SetAutoSpawnEnabled(bool bEnabled)

// Get available options (for populating dropdowns)
GetAvailableShipTypes() → Array<String>
GetAvailableDifficulties() → Array<String>
GetAvailableDebugModes() → Array<String>
```

### BlueprintNativeEvent Functions (Override in Blueprint)

```cpp
// Initialization
InitializeTestSettings()

// Button handlers
OnContinueClicked()
OnResetClicked()

// Dropdown change handlers
OnShipTypeChanged(String SelectedShip, int32 SelectionIndex)
OnDifficultyChanged(String SelectedDifficulty, int32 SelectionIndex)
OnDebugModeChanged(String SelectedDebugMode, int32 SelectionIndex)

// Transition and application
TransitionToMainMenu()
ApplySettings()
LoadDefaultSettings()
```

## Advanced Usage

### Adding Custom Dropdown Options

In your Blueprint's Class Defaults:

1. Locate **Available Ship Types** array
2. Add new entries for your custom ships
3. These will automatically appear in the dropdown

Or override in C++ constructor:

```cpp
UMyTestSettingsWidget::UMyTestSettingsWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AvailableShipTypes.Add(TEXT("My Custom Ship"));
    AvailableShipTypes.Add(TEXT("Prototype Fighter"));
}
```

### Conditional Settings Display

Show/hide settings based on build configuration:

```
Event Pre Construct
  ↓
Is Shipping Build?
    ├─ Yes → Hide Debug Mode dropdown
    └─ No → Show all settings
```

### Settings Validation

Validate settings before applying:

```
Event OnContinueClicked (Override)
  ↓
Get Selected Ship Type
  ↓
Is Ship Available in Current Level?
    ├─ Yes → Parent: OnContinueClicked
    └─ No → Show Error Message, don't continue
```

## Best Practices

### DO:
✓ Use for development and QA testing builds
✓ Remove or disable in shipping builds
✓ Provide sensible default values
✓ Log all setting changes for debugging
✓ Validate settings before applying

### DON'T:
✗ Include in final release builds (unless intended)
✗ Hardcode ship/option names that might change
✗ Forget to apply settings before transitioning
✗ Block the UI thread with expensive operations

## Logging

The widget logs all events using `LogAdastrea`:

```
LogAdastrea: TestSettingsWidget: Widget constructed
LogAdastrea: TestSettingsWidget: Settings initialized
LogAdastrea:   - Ship Type: Default Fighter
LogAdastrea:   - Difficulty: Normal
LogAdastrea:   - Debug Mode: None
LogAdastrea:   - Auto Spawn: Yes
LogAdastrea: TestSettingsWidget: Ship type changed to: Gunship (index: 3)
LogAdastrea: TestSettingsWidget: Continue clicked
LogAdastrea: TestSettingsWidget: Applying settings...
```

Enable verbose logging:
```
Log LogAdastrea Verbose
```

## Troubleshooting

### Dropdowns are empty
- Check that available options arrays are populated
- Verify `GetAvailable*` functions are called in Pre Construct
- Ensure ComboBox is properly bound to options array

### Settings not applied
- Verify `ApplySettings` is being called
- Check that GameMode reads settings after widget is created
- Ensure widget isn't destroyed before settings are read

### Transition doesn't work
- Override `TransitionToMainMenu` in Blueprint
- Create and add main menu widget in the override
- Remove test settings widget before showing main menu

## Example: Complete Implementation

```
WBP_TestSettings Blueprint
├── Designer
│   ├── Canvas Panel (Root)
│   ├── Background Image
│   ├── Title Text
│   ├── Ship Type Dropdown
│   ├── Difficulty Dropdown
│   ├── Debug Mode Dropdown
│   ├── Auto-Spawn Checkbox
│   ├── Continue Button
│   └── Reset Button
├── Graph
│   ├── Event Pre Construct
│   │   → Populate all dropdowns
│   ├── Event Construct (inherited)
│   │   → Set default selections
│   ├── ShipTypeComboBox.OnSelectionChanged
│   │   → Call OnShipTypeChanged
│   ├── DifficultyComboBox.OnSelectionChanged
│   │   → Call OnDifficultyChanged
│   ├── DebugModeComboBox.OnSelectionChanged
│   │   → Call OnDebugModeChanged
│   ├── AutoSpawnCheckBox.OnCheckStateChanged
│   │   → Call SetAutoSpawnEnabled
│   ├── ContinueButton.OnClicked
│   │   → Call OnContinueClicked
│   ├── ResetButton.OnClicked
│   │   → Call OnResetClicked
│   └── Event TransitionToMainMenu (Override)
│       → Create WBP_MainMenu
│       → Add to Viewport
└── Class Defaults
    ├── Available Ship Types: [Fighter, Scout, Hauler, ...]
    ├── Available Difficulties: [Easy, Normal, Hard]
    └── Show Main Menu After: true
```

## Version History

- **v1.0** (Current): Initial implementation
  - Dropdown configuration for ship, difficulty, debug mode
  - Auto-spawn toggle
  - Settings application and persistence
  - Main menu transition support

## See Also

- [MainMenuWidget.h](MainMenuWidget.h) - Main menu widget
- [AdastreaGameMode.h](/Source/Adastrea/AdastreaGameMode.h) - Game mode with ship spawning
- [SpaceshipDataAsset.h](/Source/Adastrea/Public/Ships/SpaceshipDataAsset.h) - Ship configuration
- [BLUEPRINT_CREATION_GUIDE.md](/BLUEPRINT_CREATION_GUIDE.md) - Blueprint setup guide
