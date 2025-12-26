# MainMenuWidget Usage Guide

## Overview

The `MainMenuWidget` is a C++ base class for creating the game's main menu interface. It provides all the core functionality needed for a start menu with full Blueprint extensibility.

## Quick Start

### 1. Create a Widget Blueprint

1. In Unreal Editor, navigate to `Content/UI/Menus/`
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_MainMenu`
4. Open the Widget Blueprint
5. Go to File → Reparent Blueprint
6. Select `MainMenuWidget` as the parent class

### 2. Design the Menu Layout

Add these widgets to the canvas:

```
Canvas Panel (Root)
├── Image - Background
│   └── Set to fill entire screen
├── Vertical Box (Center)
│   ├── Text - Title "Adastrea"
│   ├── Spacer
│   ├── Button - Start Game
│   │   └── Text - "Start Game"
│   ├── Spacer
│   ├── Button - Settings
│   │   └── Text - "Settings"
│   ├── Spacer
│   ├── Button - Quit
│   │   └── Text - "Quit"
│   └── Spacer
└── Text - Version (Bottom-Right)
```

### 3. Bind Button Events

In the Widget Blueprint's Event Graph:

**Start Game Button:**
```
OnClicked → Call "OnStartGameClicked"
```

**Settings Button:**
```
OnClicked → Call "OnSettingsClicked"
```

**Quit Button:**
```
OnClicked → Call "OnQuitClicked"
```

### 4. Configure Default Values

In the Class Defaults panel:
- **Gameplay Level Name**: Set to your main gameplay level (default: "TestLevel")
- **Check For Existing Save**: Enable if you want save game detection
- **Version Text**: Update to match your game version

### 5. Display the Menu

In your MainMenu level's Level Blueprint or GameMode:

```
Event BeginPlay
  ↓
Create Widget (Class: WBP_MainMenu)
  ↓
Add to Viewport
```

## Core Features

### Start Game Handler

Automatically handles:
- ✓ Level transition to gameplay
- ✓ Input mode switching (UI → Game)
- ✓ Mouse cursor hiding
- ✓ Widget cleanup

Default level: "TestLevel" (configurable)

### Settings Handler

Ready for Blueprint override:
```cpp
Event OnSettingsClicked
  ↓
  Create Widget (WBP_SettingsMenu)
  ↓
  Add to Viewport
```

### Quit Handler

Provides quit functionality with optional confirmation:
```cpp
Event ShowQuitConfirmation (Override in Blueprint)
  ↓
  Create Confirmation Dialog
  ↓
  If Confirmed → Quit Game
```

## Blueprint Customization

### Override Event Handlers

You can override any `_Implementation` function in Blueprint:

1. **OnStartGameClicked**: Customize game start behavior
2. **OnSettingsClicked**: Add settings menu logic
3. **OnQuitClicked**: Add custom quit logic
4. **ShowQuitConfirmation**: Show confirmation dialog
5. **TransitionToGameplay**: Customize level loading

### Example: Custom Start Game with Save Check

```
Event OnStartGameClicked (Override)
  ↓
  Does Save Game Exist?
    ├─ Yes → Show "Continue/New Game" dialog
    └─ No → Parent: OnStartGameClicked
```

### Example: Settings Menu

```
Event OnSettingsClicked (Override)
  ↓
  Create Widget: WBP_SettingsMenu
  ↓
  Add to Viewport (On top of main menu)
```

## Properties

### Configuration Properties (EditAnywhere, BlueprintReadWrite)

- **GameplayLevelName** (String): Level to load when starting game
- **bCheckForExistingSave** (Boolean): Whether to check for saves

### Display Properties (BlueprintReadOnly)

- **VersionText** (Text): Version string to display
- **bIsMenuVisible** (Boolean): Current visibility state

## Functions

### BlueprintCallable Functions

```cpp
// Update version display
UpdateVersionText(FText VersionText)

// Toggle menu visibility
SetMenuVisibility(bool bVisible)

// Get configured level name
GetGameplayLevelName() → String
```

### BlueprintNativeEvent Functions (Override in Blueprint)

```cpp
// Initialize menu (called in NativeConstruct)
InitializeMenu()

// Handle button clicks
OnStartGameClicked()
OnSettingsClicked()
OnQuitClicked()

// Level transition
TransitionToGameplay(String LevelName)

// Quit confirmation
ShowQuitConfirmation()
```

## Integration with Other Systems

### With Game Instance

```cpp
Event OnStartGameClicked (Override)
  ↓
  Get Game Instance
  ↓
  Load Player Data
  ↓
  Parent: OnStartGameClicked
```

### With Save System

```cpp
Event InitializeMenu (Override)
  ↓
  Check Save Slot Exists
  ↓
  Update "Start Game" button text
    ├─ If Save Exists → "Continue Game"
    └─ If No Save → "New Game"
  ↓
  Parent: InitializeMenu
```

### With Settings Menu

```cpp
Event OnSettingsClicked (Override)
  ↓
  Hide Main Menu (SetMenuVisibility false)
  ↓
  Create WBP_SettingsMenu
  ↓
  Add to Viewport
  ↓
  Bind Settings Menu Close Event
    ↓ On Close
    Show Main Menu (SetMenuVisibility true)
```

## Best Practices

### DO:
✓ Use the provided event handlers
✓ Override in Blueprint for custom behavior
✓ Call parent implementation when extending
✓ Use categories to organize custom properties
✓ Test input mode transitions

### DON'T:
✗ Modify input mode manually (handled by widget)
✗ Forget to remove widget before level transition
✗ Create multiple instances without cleanup
✗ Block the game thread with long operations

## Logging

The widget logs all major events using `LogAdastrea`:

```
LogAdastrea: MainMenuWidget: Menu constructed
LogAdastrea: MainMenuWidget: Menu initialized
LogAdastrea: MainMenuWidget: Start Game clicked
LogAdastrea: MainMenuWidget: Transitioning to level: TestLevel
```

Enable verbose logging in console:
```
Log LogAdastrea Verbose
```

## Troubleshooting

### Menu doesn't respond to clicks
- Check that `InitializeMenu` is being called
- Verify mouse cursor is visible
- Ensure input mode is set to UI

### Level doesn't load
- Verify `GameplayLevelName` is correct
- Check level exists in project
- Look for errors in log

### Mouse cursor stays visible in game
- Check `TransitionToGameplay` is called
- Verify input mode switches to Game
- Ensure widget is removed before transition

## Example: Complete Menu Implementation

```
WBP_MainMenu Blueprint
├── Designer
│   └── [Menu layout as described above]
├── Graph
│   ├── Event Construct (inherited, do not modify)
│   ├── Button_StartGame.OnClicked
│   │   → Call OnStartGameClicked
│   ├── Button_Settings.OnClicked
│   │   → Call OnSettingsClicked
│   └── Button_Quit.OnClicked
│       → Call OnQuitClicked
└── Class Defaults
    ├── Gameplay Level Name: "MainGameLevel"
    └── Check For Existing Save: true
```

## Version History

- **v1.0** (Current): Initial implementation
  - Core menu functionality
  - Blueprint extensibility
  - Input management
  - Level transitions

## See Also

- [AdastreaHUDWidget.h](AdastreaHUDWidget.h) - In-game HUD widget
- [InventoryWidget.h](InventoryWidget.h) - Inventory UI widget
- [BLUEPRINT_CREATION_GUIDE.md](/BLUEPRINT_CREATION_GUIDE.md) - Blueprint setup guide
- [UI_MOCKUP_DEMO.html](/UI_MOCKUP_DEMO.html) - UI design reference
