# Test Settings Widget - Blueprint Implementation Example

This document provides a step-by-step guide with Blueprint pseudo-code for implementing the Test Settings pre-screen.

## Widget Blueprint: WBP_TestSettings

### Designer Layout

```
Canvas Panel
├── Image (BackgroundImage)
│   • Anchor: Fill Screen
│   • Color: Black with 0.8 opacity
│   • Size: Fill
│
├── Vertical Box (MainContainer)
│   • Anchor: Center
│   • Alignment: Middle Center
│   • Padding: 50px all sides
│   
│   ├── Text Block (TitleText)
│   │   • Text: "Test Configuration"
│   │   • Size: 48pt, Bold
│   │   • Color: White
│   │   • Alignment: Center
│   │
│   ├── Spacer (20px)
│   │
│   ├── Horizontal Box (ShipRow)
│   │   ├── Text Block (ShipLabel)
│   │   │   • Text: "Ship Type:"
│   │   │   • Size: 24pt
│   │   │   • Min Width: 200px
│   │   │
│   │   └── ComboBox String (ShipTypeComboBox)
│   │       • Min Width: 300px
│   │
│   ├── Spacer (10px)
│   │
│   ├── Horizontal Box (DifficultyRow)
│   │   ├── Text Block (DifficultyLabel)
│   │   │   • Text: "Difficulty:"
│   │   │   • Size: 24pt
│   │   │   • Min Width: 200px
│   │   │
│   │   └── ComboBox String (DifficultyComboBox)
│   │       • Min Width: 300px
│   │
│   ├── Spacer (10px)
│   │
│   ├── Horizontal Box (DebugRow)
│   │   ├── Text Block (DebugLabel)
│   │   │   • Text: "Debug Mode:"
│   │   │   • Size: 24pt
│   │   │   • Min Width: 200px
│   │   │
│   │   └── ComboBox String (DebugModeComboBox)
│   │       • Min Width: 300px
│   │
│   ├── Spacer (10px)
│   │
│   ├── Check Box (AutoSpawnCheckBox)
│   │   • Text: "Auto-spawn selected ship on start"
│   │   • Checked: True (default)
│   │
│   ├── Spacer (30px)
│   │
│   └── Horizontal Box (ButtonRow)
│       ├── Button (ContinueButton)
│       │   • Text: "Continue to Menu"
│       │   • Size: 200x50px
│       │   • Hover Color: Green
│       │
│       ├── Spacer (20px)
│       │
│       └── Button (ResetButton)
│           • Text: "Reset Defaults"
│           • Size: 200x50px
│           • Hover Color: Orange
```

## Event Graph - Pre Construct

This populates the dropdowns with available options:

```
┌─────────────────────────┐
│ Event Pre Construct     │
│ (Is Design Time = Any)  │
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────────┐
│ Get Available Ship Types     │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ ForEachLoop                  │
│ (Array: Ship Types)          │
└──────┬───────────────────────┘
       │
       │ Loop Body
       ▼
┌──────────────────────────────┐
│ ShipTypeComboBox             │
│ → Add Option (Array Element) │
└──────────────────────────────┘
       │
       │ Completed
       ▼
┌──────────────────────────────┐
│ Get Available Difficulties   │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ ForEachLoop                  │
│ (Array: Difficulties)        │
└──────┬───────────────────────┘
       │
       │ Loop Body
       ▼
┌──────────────────────────────┐
│ DifficultyComboBox           │
│ → Add Option (Array Element) │
└──────────────────────────────┘
       │
       │ Completed
       ▼
┌──────────────────────────────┐
│ Get Available Debug Modes    │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ ForEachLoop                  │
│ (Array: Debug Modes)         │
└──────┬───────────────────────┘
       │
       │ Loop Body
       ▼
┌──────────────────────────────┐
│ DebugModeComboBox            │
│ → Add Option (Array Element) │
└──────────────────────────────┘
```

## Event Graph - Construct

This sets the default selections:

```
┌─────────────────────────┐
│ Event Construct         │
│ (Inherited, runs after  │
│  parent's NativeConstruct)
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────────┐
│ Get Selected Ship Type       │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ ShipTypeComboBox             │
│ → Set Selected Option        │
│   (Option: SelectedShipType) │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ Get Selected Difficulty      │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ DifficultyComboBox           │
│ → Set Selected Option        │
│   (Option: Selected...)      │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ Get Selected Debug Mode      │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ DebugModeComboBox            │
│ → Set Selected Option        │
│   (Option: SelectedDebug...) │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ Get Auto Spawn Enabled       │
│ (From Parent Class)          │
└──────────┬───────────────────┘
           │
           ▼
┌──────────────────────────────┐
│ AutoSpawnCheckBox            │
│ → Set Checked State          │
│   (InCheckedState: bool)     │
└──────────────────────────────┘
```

## Event Graph - ComboBox Bindings

### Ship Type Selection Changed

```
┌─────────────────────────────────┐
│ ShipTypeComboBox                │
│ → OnSelectionChanged            │
│   (Out: SelectedItem, Index)    │
└──────────┬──────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ OnShipTypeChanged               │
│ (Parent Function)               │
│ In: SelectedShip (String)       │
│ In: SelectionIndex (Integer)    │
└─────────────────────────────────┘
```

### Difficulty Selection Changed

```
┌─────────────────────────────────┐
│ DifficultyComboBox              │
│ → OnSelectionChanged            │
│   (Out: SelectedItem, Index)    │
└──────────┬──────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ OnDifficultyChanged             │
│ (Parent Function)               │
│ In: SelectedDifficulty (String) │
│ In: SelectionIndex (Integer)    │
└─────────────────────────────────┘
```

### Debug Mode Selection Changed

```
┌─────────────────────────────────┐
│ DebugModeComboBox               │
│ → OnSelectionChanged            │
│   (Out: SelectedItem, Index)    │
└──────────┬──────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ OnDebugModeChanged              │
│ (Parent Function)               │
│ In: SelectedDebugMode (String)  │
│ In: SelectionIndex (Integer)    │
└─────────────────────────────────┘
```

### Auto-Spawn CheckBox

```
┌─────────────────────────────────┐
│ AutoSpawnCheckBox               │
│ → OnCheckStateChanged           │
│   (Out: bIsChecked)             │
└──────────┬──────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ SetAutoSpawnEnabled             │
│ (Parent Function)               │
│ In: bEnabled (Boolean)          │
└─────────────────────────────────┘
```

## Event Graph - Button Handlers

### Continue Button

```
┌─────────────────────────┐
│ ContinueButton          │
│ → OnClicked             │
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────┐
│ OnContinueClicked       │
│ (Parent Function)       │
└─────────────────────────┘
```

### Reset Button

```
┌─────────────────────────┐
│ ResetButton             │
│ → OnClicked             │
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────┐
│ OnResetClicked          │
│ (Parent Function)       │
└─────────────────────────┘
```

## Event Graph - Override TransitionToMainMenu

This is the most important override - it shows the main menu:

```
┌────────────────────────────────┐
│ Event TransitionToMainMenu     │
│ (Override)                     │
└──────────┬─────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Remove From Parent               │
│ (Target: Self)                   │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Create Widget                    │
│ Class: WBP_MainMenu              │
│ (Owning Player: Get Player...)   │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Add to Viewport                  │
│ (Target: Created Widget)         │
│ (Z-Order: 0)                     │
└──────────────────────────────────┘
```

## Level Blueprint - Show Test Settings on Start

In your test level's Level Blueprint:

```
┌─────────────────────────┐
│ Event BeginPlay         │
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Create Widget                    │
│ Class: WBP_TestSettings          │
│ (Owning Player: Get Player       │
│  Controller → Index 0)           │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Add to Viewport                  │
│ (Target: Created Widget)         │
│ (Z-Order: 0)                     │
└──────────────────────────────────┘
```

## GameMode Blueprint - Read Test Settings

Optional: Read settings in your GameMode when spawning:

```
┌─────────────────────────┐
│ Event BeginPlay         │
└──────────┬──────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Get All Widgets of Class         │
│ Class: TestSettingsWidget        │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Get (Index: 0)                   │
│ (From Array)                     │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Is Valid? (Object)               │
└──────┬───────────────┬───────────┘
       │               │
   True│               │False
       ▼               ▼
┌─────────────────┐  ┌──────────────┐
│ Get Auto Spawn  │  │ Use Default  │
│ Enabled         │  │ Setup        │
└────┬────────────┘  └──────────────┘
     │
     ▼
┌──────────────────┐
│ Branch           │
│ (Auto Spawn?)    │
└────┬─────────┬───┘
     │         │
  True│        │False
     ▼         ▼
┌─────────────┐ ┌────────────┐
│ Get Selected│ │ Skip Spawn │
│ Ship Type   │ └────────────┘
└────┬────────┘
     │
     ▼
┌──────────────────────────┐
│ Spawn Ship of Type       │
│ (Type: SelectedShipType) │
└──────────────────────────┘
```

## Advanced: Custom Settings Storage

If you want to persist settings across sessions, add this to your GameInstance:

```
GameInstance Blueprint Variables:
├── TestShipType (String)
├── TestDifficulty (String)
├── TestDebugMode (String)
└── bTestAutoSpawn (Boolean)

Event ApplySettings (Override in WBP_TestSettings):
  ┌───────────────────┐
  │ Parent:           │
  │ ApplySettings     │
  └────┬──────────────┘
       │
       ▼
  ┌───────────────────┐
  │ Get Game Instance │
  └────┬──────────────┘
       │
       ▼
  ┌────────────────────────────┐
  │ Set TestShipType           │
  │ (To: Get Selected Ship...) │
  └────┬───────────────────────┘
       │
       ▼
  ┌────────────────────────────┐
  │ Set TestDifficulty         │
  │ (To: Get Selected Diff...) │
  └────┬───────────────────────┘
       │
       ▼
  ┌────────────────────────────┐
  │ Set TestDebugMode          │
  │ (To: Get Selected Debug...)│
  └────┬───────────────────────┘
       │
       ▼
  ┌────────────────────────────┐
  │ Set bTestAutoSpawn         │
  │ (To: Get Auto Spawn...)    │
  └────────────────────────────┘
```

## Summary

This implementation provides:
- ✓ Dropdown menus for ship type, difficulty, and debug mode
- ✓ Auto-spawn toggle
- ✓ Continue to main menu functionality
- ✓ Reset to defaults
- ✓ Full Blueprint customization
- ✓ Integration with GameMode for test setup
- ✓ Clean separation between test configuration and gameplay

All the heavy lifting is done in C++, while Blueprint provides the UI design and customization flexibility.
