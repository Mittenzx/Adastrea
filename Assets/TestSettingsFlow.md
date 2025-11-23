# Test Settings Widget - Integration Flow Diagram

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Adastrea Game Start                       │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              AdastreaGameMode::BeginPlay()                   │
│                                                              │
│  • Check bShowTestSettingsOnStartup                          │
│  • Check TestSettingsWidgetClass                             │
└─────────────────────────────────────────────────────────────┘
                            ↓
                            ↓
              ┌─────────────┴─────────────┐
              │                           │
        ┌─────▼────┐                ┌─────▼────┐
        │   YES    │                │    NO    │
        │ Show UI  │                │  Skip UI  │
        └─────┬────┘                └─────┬────┘
              │                           │
              ↓                           │
┌─────────────────────────────────────┐  │
│  ShowTestSettingsWidget()            │  │
│                                      │  │
│  1. Get Player Controller            │  │
│  2. CreateWidget<UUserWidget>()      │  │
│  3. Cast to UTestSettingsWidget      │  │
│  4. AddToViewport(Z-order: 100)      │  │
└──────────────┬──────────────────────┘  │
               │                          │
               ↓                          │
┌─────────────────────────────────────┐  │
│    TestSettingsWidget Displayed      │  │
│                                      │  │
│  • User configures ship type         │  │
│  • User sets difficulty              │  │
│  • User enables debug modes          │  │
│  • User toggles auto-spawn           │  │
└──────────────┬──────────────────────┘  │
               │                          │
               ↓                          │
┌─────────────────────────────────────┐  │
│    User Clicks "Continue" Button     │  │
└──────────────┬──────────────────────┘  │
               │                          │
               ↓                          │
┌─────────────────────────────────────┐  │
│  OnContinueClicked_Implementation()  │  │
│                                      │  │
│  1. Apply Settings                   │  │
│  2. Get GameMode                     │  │
│  3. Find OnTestSettingsContinue()    │  │
│  4. Call via ProcessEvent()          │  │
└──────────────┬──────────────────────┘  │
               │                          │
               ↓                          │
               │                          │
               └──────────┬───────────────┘
                          ↓
┌─────────────────────────────────────────────────────────────┐
│           OnTestSettingsContinue()                           │
│                                                              │
│  1. Remove TestSettingsWidget from viewport                  │
│  2. Clean up widget reference                                │
│  3. Check bAutoSpawnPlayerShip                               │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              SpawnPlayerSpaceship()                          │
│                                                              │
│  1. Find SpaceSectorMap in level                             │
│  2. Get spawn position (center or random)                    │
│  3. Spawn DefaultSpaceshipClass                              │
│  4. Possess ship with PlayerController                       │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Gameplay Begins                           │
└─────────────────────────────────────────────────────────────┘
```

## Component Interaction Diagram

```
┌──────────────────┐         ┌──────────────────┐         ┌──────────────────┐
│                  │         │                  │         │                  │
│  AdastreaGameMode│◄───────►│TestSettingsWidget│◄───────►│ PlayerController │
│                  │         │                  │         │                  │
└────────┬─────────┘         └────────┬─────────┘         └──────────────────┘
         │                            │
         │ 1. Create & Show           │
         ├───────────────────────────►│
         │                            │
         │                     2. User Configures
         │                            │
         │ 3. OnTestSettingsContinue()│
         │◄───────────────────────────┤
         │                            │
         │ 4. Remove Widget           │
         ├───────────────────────────►│
         │                            │
         ↓
    Spawn Ship
         ↓
┌──────────────────┐
│                  │
│    Spaceship     │
│                  │
└──────────────────┘
```

## Property Configuration Flow

```
┌─────────────────────────────────────────────────────────────┐
│              GameMode Blueprint (Editor)                     │
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Test Settings Category                              │    │
│  │                                                     │    │
│  │  [✓] bShowTestSettingsOnStartup                     │    │
│  │  [WBP_TestSettings] TestSettingsWidgetClass         │    │
│  │                                                     │    │
│  └────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │ Player Spawn Category                               │    │
│  │                                                     │    │
│  │  [✓] bAutoSpawnPlayerShip                           │    │
│  │  [BP_Fighter] DefaultSpaceshipClass                 │    │
│  │  [ ] bSpawnAtCenter                                 │    │
│  │                                                     │    │
│  └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
                     Configured!
                            ↓
              TestSettings will appear first
```

## State Transition Diagram

```
                    ┌──────────────┐
                    │  Game Start  │
                    └──────┬───────┘
                           ↓
                    ┌──────────────┐
                    │ BeginPlay()  │
                    └──────┬───────┘
                           ↓
                   ┌───────────────┐
                   │ Widget Shown? │
                   └───┬───────┬───┘
                       │       │
                   YES │       │ NO
                       │       │
            ┌──────────▼─┐    └──────────┐
            │ UI State   │               │
            │            │               │
            │ • Visible  │               │
            │ • Editable │               │
            │ • Waiting  │               │
            └──────┬─────┘               │
                   │                     │
            User Clicks Continue         │
                   │                     │
                   ↓                     │
            ┌────────────┐               │
            │ Transition │               │
            │   State    │               │
            │            │               │
            │ • Remove UI│               │
            │ • Clean Up │               │
            └──────┬─────┘               │
                   │                     │
                   └──────────┬──────────┘
                              ↓
                    ┌──────────────────┐
                    │  Gameplay State  │
                    │                  │
                    │  • Ship Spawned  │
                    │  • Player Active │
                    │  • Game Running  │
                    └──────────────────┘
```

## Data Flow

```
TestSettingsWidget Properties:
┌────────────────────────────┐
│ • SelectedShipType         │───┐
│ • SelectedDifficulty       │   │
│ • SelectedDebugMode        │   │  ApplySettings()
│ • bAutoSpawnShip           │   │
└────────────────────────────┘   ↓
                                 │
                                 ↓
                    ┌────────────────────────┐
                    │   Settings Applied     │
                    │                        │
                    │ Available for:         │
                    │ • GameMode             │
                    │ • PlayerController     │
                    │ • GameInstance         │
                    └────────────────────────┘
                                 │
                                 ↓
                    ┌────────────────────────┐
                    │   Ship Spawning        │
                    │                        │
                    │ Uses:                  │
                    │ • SelectedShipType     │
                    │ • bAutoSpawnShip       │
                    └────────────────────────┘
```

## Error Handling Flow

```
┌─────────────────────────────────────┐
│  ShowTestSettingsWidget()            │
└─────────────────┬───────────────────┘
                  │
                  ↓
          ┌───────────────┐
          │ Widget Class  │
          │    Set?       │
          └───┬───────┬───┘
              │       │
          YES │       │ NO
              │       │
              │       └──────────┐
              ↓                  ↓
    ┌──────────────────┐   ┌─────────────┐
    │ Player Controller│   │   Warning   │
    │    Found?        │   │   + Return  │
    └────┬─────────┬───┘   └─────────────┘
         │         │
     YES │         │ NO
         │         │
         │         └────────────┐
         ↓                      ↓
┌──────────────────┐      ┌──────────┐
│ Create Widget    │      │  Error   │
└────┬─────────────┘      │ + Return │
     │                    └──────────┘
     ↓
┌─────────────┐
│   Widget    │
│  Created?   │
└───┬─────┬───┘
    │     │
YES │     │ NO
    │     │
    │     └──────────┐
    ↓                ↓
┌─────────┐    ┌──────────┐
│ Success │    │  Error   │
│Display! │    │ + Return │
└─────────┘    └──────────┘
```

## Integration Points

```
┌────────────────────────────────────────────────────────────┐
│                    Unreal Engine 5                          │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐  │
│  │              Game Framework Layer                    │  │
│  │                                                      │  │
│  │  ┌──────────────┐         ┌──────────────┐         │  │
│  │  │  GameMode    │◄───────►│PlayerController        │  │
│  │  │  Base        │         │              │         │  │
│  │  └──────┬───────┘         └──────────────┘         │  │
│  │         │                                           │  │
│  │         └──────────────┬──────────────────────────┐│  │
│  │                        ↓                          ││  │
│  │              ┌───────────────────┐                ││  │
│  │              │AdastreaGameMode   │                ││  │
│  │              │                   │                ││  │
│  │              │ + TestSettings    │                ││  │
│  │              └─────────┬─────────┘                ││  │
│  └────────────────────────┼──────────────────────────┘│  │
│                            │                           │  │
│  ┌─────────────────────────┼──────────────────────────┐  │
│  │              UI Layer    ↓                         │  │
│  │                                                     │  │
│  │  ┌──────────────┐         ┌──────────────┐        │  │
│  │  │  UserWidget  │         │              │        │  │
│  │  │  Base        │         │              │        │  │
│  │  └──────┬───────┘         │              │        │  │
│  │         │                 │              │        │  │
│  │         └──────────────┐  │              │        │  │
│  │                        ↓  │              │        │  │
│  │              ┌───────────────────┐       │        │  │
│  │              │TestSettingsWidget │       │        │  │
│  │              │                   │       │        │  │
│  │              │ + Ship Selection  │       │        │  │
│  │              │ + Difficulty      │       │        │  │
│  │              │ + Debug Modes     │       │        │  │
│  │              └───────────────────┘       │        │  │
│  └─────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────┘
```

## Blueprint Usage Example

```
┌────────────────────────────────────────────────────────────┐
│        BP_SpaceGameMode (Blueprint)                         │
│                                                             │
│  Inherits from: AdastreaGameMode                            │
│                                                             │
│  Class Defaults:                                            │
│  ├─ TestSettingsWidgetClass: WBP_TestSettings              │
│  ├─ bShowTestSettingsOnStartup: true                        │
│  ├─ DefaultSpaceshipClass: BP_Fighter                       │
│  └─ bAutoSpawnPlayerShip: true                              │
│                                                             │
│  Event Graph: (optional overrides)                          │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Event OnTestSettingsContinue                         │  │
│  │     ↓                                                 │  │
│  │  Get Test Settings Widget                             │  │
│  │     ↓                                                 │  │
│  │  Read Custom Settings                                 │  │
│  │     ↓                                                 │  │
│  │  Parent: OnTestSettingsContinue                       │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────┘
```

---

## Summary

This visual guide shows how the TestSettingsWidget integrates with the GameMode:

1. **Game starts** → GameMode BeginPlay
2. **Check configuration** → Show widget if enabled
3. **User interaction** → Configure test settings
4. **Continue clicked** → Notify GameMode
5. **Cleanup & spawn** → Remove widget, spawn ship
6. **Gameplay begins** → Normal game flow

The integration is:
- ✅ Modular (can be disabled)
- ✅ Flexible (Blueprint customizable)
- ✅ Robust (error handling at each step)
- ✅ Documented (clear flow diagrams)
