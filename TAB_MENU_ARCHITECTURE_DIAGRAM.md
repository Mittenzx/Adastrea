# Tab Menu Architecture Diagram

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         GAME MODES                               │
│  ┌────────────────────┐              ┌──────────────────────┐   │
│  │  Spaceship Mode    │              │  Third-Person Mode   │   │
│  │  (Flying)          │              │  (Walking/Interior)  │   │
│  └────────┬───────────┘              └──────────┬───────────┘   │
│           │                                      │               │
│           │         Tab Key Pressed              │               │
│           └──────────────┬───────────────────────┘               │
│                          ▼                                       │
└──────────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                    INPUT SYSTEM                                  │
│  ┌────────────────────┐              ┌──────────────────────┐   │
│  │ IMC_Spaceship      │              │ IMC_ThirdPerson      │   │
│  │ ┌────────────────┐ │              │ ┌────────────────┐  │   │
│  │ │ Tab -> Menu    │ │              │ │ Tab -> Menu    │  │   │
│  │ └────────────────┘ │              │ └────────────────┘  │   │
│  └────────┬───────────┘              └──────────┬───────────┘   │
│           └──────────────┬───────────────────────┘               │
│                          ▼                                       │
│           ┌──────────────────────────┐                          │
│           │  MenuAction (IA_Pause)   │                          │
│           └──────────────┬───────────┘                          │
└───────────────────────────┼──────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│            ADASTREA PLAYER CONTROLLER (C++)                      │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              ToggleMainMenu()                            │   │
│  │                      │                                   │   │
│  │         ┌────────────┴───────────┐                      │   │
│  │         ▼                        ▼                       │   │
│  │  ┌─────────────┐         ┌─────────────┐               │   │
│  │  │ ShowMainMenu│         │ HideMainMenu│               │   │
│  │  └──────┬──────┘         └──────┬──────┘               │   │
│  └─────────┼───────────────────────┼──────────────────────┘   │
│            │                       │                           │
│  ┌─────────▼───────────────────────▼──────────────────────┐   │
│  │          Widget Lifecycle Management                    │   │
│  │  • CreateMainMenuWidget()                               │   │
│  │  • Add/Remove from Viewport                             │   │
│  │  • Input Mode Management                                │   │
│  │  • Cursor Visibility                                    │   │
│  │  • State Tracking (bIsMainMenuOpen)                     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                          │                                      │
└──────────────────────────┼──────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                    MENU WIDGET (UMG)                             │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │               MainMenuWidget                              │  │
│  │  ┌─────────────────────────────────────────────────┐     │  │
│  │  │  • Resume Button (calls HideMainMenu)           │     │  │
│  │  │  • Settings Button                              │     │  │
│  │  │  • Return to Main Menu Button                   │     │  │
│  │  │  • Quit Button                                  │     │  │
│  │  └─────────────────────────────────────────────────┘     │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Component Interaction Flow

### Opening Menu (Tab Press)
```
1. Player presses Tab
   │
   ▼
2. Input System triggers MenuAction
   │
   ▼
3. MenuAction bound to ToggleMainMenu()
   │
   ▼
4. ToggleMainMenu() checks bIsMainMenuOpen (false)
   │
   ▼
5. Calls ShowMainMenu()
   │
   ├─▶ CreateMainMenuWidget() - Creates widget if needed
   │
   ├─▶ AddToViewport() - Shows widget
   │
   ├─▶ SetInputMode(GameAndUI) - Enables UI input
   │   └─▶ SetWidgetToFocus() - Focuses menu
   │
   ├─▶ bShowMouseCursor = true - Shows cursor
   │
   └─▶ bIsMainMenuOpen = true - Updates state
```

### Closing Menu (Tab Press Again)
```
1. Player presses Tab
   │
   ▼
2. Input System triggers MenuAction
   │
   ▼
3. MenuAction bound to ToggleMainMenu()
   │
   ▼
4. ToggleMainMenu() checks bIsMainMenuOpen (true)
   │
   ▼
5. Calls HideMainMenu()
   │
   ├─▶ RemoveFromParent() - Hides widget
   │
   ├─▶ SetInputMode(GameOnly) - Restores game input
   │
   ├─▶ bShowMouseCursor = false - Hides cursor
   │
   └─▶ bIsMainMenuOpen = false - Updates state
```

## Class Relationships

```
┌──────────────────────────────────────────────────────────────────┐
│                    AAdastreaPlayerController                      │
│                                                                   │
│  Properties:                                                      │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ TSubclassOf<UUserWidget> MainMenuWidgetClass              │ │
│  │ UUserWidget* MainMenuWidget                                │ │
│  │ bool bIsMainMenuOpen                                       │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  Public Functions:                                                │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ void ToggleMainMenu()                                      │ │
│  │ bool IsMainMenuOpen() const                                │ │
│  │ UUserWidget* GetMainMenuWidget() const                     │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  Protected Functions:                                             │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ UUserWidget* CreateMainMenuWidget()                        │ │
│  │ void ShowMainMenu()                                        │ │
│  │ void HideMainMenu()                                        │ │
│  └────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
                           │
                           │ uses
                           ▼
┌──────────────────────────────────────────────────────────────────┐
│                   UInputConfigDataAsset                           │
│                                                                   │
│  Input Mapping Contexts:                                          │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ UInputMappingContext* SpaceshipMappingContext              │ │
│  │ UInputMappingContext* ThirdPersonMappingContext            │ │
│  │ UInputMappingContext* MenuMappingContext                   │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  Input Actions:                                                   │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ UInputAction* MenuAction                                   │ │
│  │ UInputAction* MoveAction                                   │ │
│  │ UInputAction* LookAction                                   │ │
│  │ ... (other actions)                                        │ │
│  └────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

## State Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      MENU STATE                              │
│                                                              │
│     ┌───────────────────┐                                   │
│     │                   │                                   │
│     │  Menu Closed      │                                   │
│     │  (bIsMainMenuOpen │                                   │
│     │   = false)        │                                   │
│     │                   │                                   │
│     └─────────┬─────────┘                                   │
│               │                                             │
│               │ Tab pressed                                 │
│               │ ToggleMainMenu()                            │
│               │                                             │
│               ▼                                             │
│     ┌─────────────────────┐                                │
│     │  ShowMainMenu()     │                                │
│     │  • Create widget    │                                │
│     │  • Add to viewport  │                                │
│     │  • GameAndUI mode   │                                │
│     │  • Show cursor      │                                │
│     │  • Set state true   │                                │
│     └─────────┬───────────┘                                │
│               │                                             │
│               ▼                                             │
│     ┌───────────────────┐                                  │
│     │                   │                                  │
│     │  Menu Open        │◀────────────┐                    │
│     │  (bIsMainMenuOpen │             │                    │
│     │   = true)         │             │                    │
│     │                   │             │ Other UI action    │
│     └─────────┬─────────┘             │ closes menu        │
│               │                       │                    │
│               │ Tab pressed again     │                    │
│               │ ToggleMainMenu()      │                    │
│               │                       │                    │
│               ▼                       │                    │
│     ┌─────────────────────┐           │                    │
│     │  HideMainMenu()     │───────────┘                    │
│     │  • Remove from vp   │                                │
│     │  • GameOnly mode    │                                │
│     │  • Hide cursor      │                                │
│     │  • Set state false  │                                │
│     └─────────┬───────────┘                                │
│               │                                             │
│               └──────────┐                                  │
│                          │                                  │
│                          ▼                                  │
│               Back to Menu Closed                           │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## Input Mode States

```
┌────────────────────────────────────────────────────────────┐
│                    INPUT MODE FLOW                          │
│                                                             │
│  Game Playing (Menu Closed)                                │
│  ┌──────────────────────────────────────────────────────┐ │
│  │  Input Mode: GameOnly                                 │ │
│  │  Mouse Cursor: Hidden                                 │ │
│  │  Input Focus: Game Controls                           │ │
│  │                                                        │ │
│  │  • Player can control ship/character                  │ │
│  │  • All game inputs active                             │ │
│  │  • No UI interaction                                  │ │
│  └──────────────────────────────────────────────────────┘ │
│                          │                                 │
│                          │ Press Tab                       │
│                          ▼                                 │
│  ┌──────────────────────────────────────────────────────┐ │
│  │           TRANSITION (ShowMainMenu)                   │ │
│  │  • SetInputMode(GameAndUI)                            │ │
│  │  • SetWidgetToFocus(MenuWidget)                       │ │
│  │  • bShowMouseCursor = true                            │ │
│  └──────────────────────────────────────────────────────┘ │
│                          │                                 │
│                          ▼                                 │
│  Menu Open                                                 │
│  ┌──────────────────────────────────────────────────────┐ │
│  │  Input Mode: GameAndUI                                │ │
│  │  Mouse Cursor: Visible                                │ │
│  │  Input Focus: Menu Widget                             │ │
│  │                                                        │ │
│  │  • Player can see game in background                  │ │
│  │  • Can interact with menu buttons                     │ │
│  │  • Game inputs suppressed                             │ │
│  └──────────────────────────────────────────────────────┘ │
│                          │                                 │
│                          │ Press Tab or Resume             │
│                          ▼                                 │
│  ┌──────────────────────────────────────────────────────┐ │
│  │           TRANSITION (HideMainMenu)                   │ │
│  │  • SetInputMode(GameOnly)                             │ │
│  │  • bShowMouseCursor = false                           │ │
│  └──────────────────────────────────────────────────────┘ │
│                          │                                 │
│                          └─────────► Back to Game Playing  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## File Structure

```
Adastrea/
│
├── Source/Adastrea/
│   ├── Public/
│   │   ├── Player/
│   │   │   └── AdastreaPlayerController.h ◄── Menu system declarations
│   │   │
│   │   └── Input/
│   │       └── InputConfigDataAsset.h ◄── Input mapping contexts
│   │
│   ├── Player/
│   │   └── AdastreaPlayerController.cpp ◄── Menu system implementation
│   │
│   └── Input/
│       └── InputConfigDataAsset.cpp ◄── Input config implementation
│
├── Content/
│   ├── Input/
│   │   ├── IMC_Spaceship.uasset ◄── Spaceship input mappings (incl. Tab)
│   │   ├── IMC_ThirdPerson.uasset ◄── Third-person mappings (to be created)
│   │   └── Actions/
│   │       └── IA_PauseMenu.uasset ◄── Menu action (MenuAction)
│   │
│   └── UI/
│       └── Menus/
│           └── WBP_MainMenu.uasset ◄── Menu widget Blueprint
│
└── Documentation/
    ├── TAB_MENU_SETUP_GUIDE.md ◄── Setup instructions
    ├── TAB_MENU_IMPLEMENTATION_SUMMARY.md ◄── Implementation details
    └── TAB_MENU_ARCHITECTURE_DIAGRAM.md ◄── This file
```

## Integration Points

```
┌──────────────────────────────────────────────────────────────┐
│                     INTEGRATION MAP                           │
│                                                               │
│  ┌─────────────┐     uses      ┌──────────────────┐         │
│  │  GameMode   │──────────────▶│ PlayerController │         │
│  │             │               │                  │         │
│  └─────────────┘               └────────┬─────────┘         │
│                                         │                    │
│                                         │ manages            │
│                                         ▼                    │
│                                ┌─────────────────┐           │
│                                │   MenuWidget    │           │
│                                └────────┬────────┘           │
│                                         │                    │
│                                         │ uses               │
│  ┌─────────────────┐                   │                    │
│  │ InputConfig     │◀──────────────────┘                    │
│  │ DataAsset       │                                        │
│  └────────┬────────┘                                        │
│           │                                                 │
│           │ defines                                         │
│           ▼                                                 │
│  ┌─────────────────┐                                       │
│  │ Input Mapping   │                                       │
│  │ Contexts (IMC)  │                                       │
│  └────────┬────────┘                                       │
│           │                                                │
│           │ contains                                       │
│           ▼                                                │
│  ┌─────────────────┐                                       │
│  │ Input Actions   │                                       │
│  │ (IA)            │                                       │
│  └─────────────────┘                                       │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

This architecture provides a clean separation of concerns, with C++ handling the core logic and Blueprints handling configuration and presentation.
