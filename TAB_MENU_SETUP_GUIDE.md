# Tab Key Main Menu Setup Guide

This guide explains how to configure the Tab key to open the main menu in both spaceship and third-person modes.

## Overview

The Tab key has been set up to open/close the main menu (pause menu) when pressed in both:
- **Spaceship mode**: When flying a spaceship
- **Third-person mode**: When walking in stations or ship interiors

## C++ Implementation (Completed)

The following C++ functionality has been implemented:

### AdastreaPlayerController

**New Properties:**
- `MainMenuWidgetClass` - Widget class to use for the main menu
- `MainMenuWidget` - Current menu widget instance
- `bIsMainMenuOpen` - State tracking for menu visibility

**New Functions:**
- `ToggleMainMenu()` - Toggle the menu on/off
- `ShowMainMenu()` - Open the menu
- `HideMainMenu()` - Close the menu
- `IsMainMenuOpen()` - Check if menu is open
- `GetMainMenuWidget()` - Get menu widget instance

### InputConfigDataAsset

**New Properties:**
- `ThirdPersonMappingContext` - Input mapping context for third-person walking
- `ThirdPersonContextPriority` - Priority for third-person context
- `GetThirdPersonMappingContext()` - Getter function

**Existing Property (Already Available):**
- `MenuAction` - Input action for opening menu/pause

## Blueprint Setup Required

### Step 1: Configure Player Controller Blueprint

1. Open your player controller Blueprint (derived from `BP_AdastreaPlayerController`)
2. In the Details panel, find the **Main Menu** category
3. Set `MainMenuWidgetClass` to your menu widget Blueprint (e.g., `WBP_MainMenu` or `WBP_PauseMenu`)

### Step 2: Bind MenuAction to ToggleMainMenu

The binding can be done in one of two ways:

#### Option A: In GameMode Blueprint
```
1. Open your GameMode Blueprint
2. In BeginPlay or input setup:
   - Get Player Controller
   - Get Input Component
   - Bind Action "MenuAction" to call ToggleMainMenu() on the player controller
```

#### Option B: In Player Controller Blueprint
```
1. Open BP_AdastreaPlayerController
2. Override SetupInputComponent function
3. Call parent function first
4. Bind Enhanced Input Action:
   - Action: MenuAction (from InputConfigDataAsset)
   - Event: Triggered
   - Function: ToggleMainMenu (on self)
```

### Step 3: Configure Tab Key in IMC_Spaceship

1. Open `Content/Input/IMC_Spaceship` (Input Mapping Context)
2. Add a new mapping:
   - **Action**: `IA_PauseMenu` (or `MenuAction`)
   - **Key**: `Tab`
   - **Modifiers**: None
   - **Triggers**: Pressed

### Step 4: Create and Configure IMC_ThirdPerson (Future)

When implementing third-person mode:

1. Create a new Input Mapping Context: `Content/Input/IMC_ThirdPerson`
2. Add third-person movement actions (WASD, Jump, etc.)
3. Add the menu action:
   - **Action**: `IA_PauseMenu` (or `MenuAction`)
   - **Key**: `Tab`
   - **Modifiers**: None
   - **Triggers**: Pressed
4. In InputConfigDataAsset:
   - Set `ThirdPersonMappingContext` to `IMC_ThirdPerson`
   - Set `ThirdPersonContextPriority` to appropriate value (e.g., 0)

## Input Action Setup

If `IA_PauseMenu` doesn't exist or isn't configured:

1. Open or create `Content/Input/Actions/IA_PauseMenu`
2. Set **Value Type**: None (it's a simple button press)
3. Ensure it's referenced in your `DA_InputConfig` as `MenuAction`

## Testing

### Test in Spaceship Mode
1. Play the game and enter a spaceship
2. Press Tab
3. Verify the menu appears
4. Press Tab again
5. Verify the menu closes

### Test in Third-Person Mode (When Implemented)
1. Exit the spaceship and enter a station/interior
2. Press Tab
3. Verify the menu appears
4. Press Tab again
5. Verify the menu closes

## Behavior Details

When the menu opens:
- Input mode switches to `GameAndUI` (allows menu interaction while seeing game)
- Mouse cursor becomes visible
- Menu widget is added to viewport

When the menu closes:
- Input mode switches back to `GameOnly`
- Mouse cursor is hidden
- Menu widget is removed from viewport

## Integration with Existing Menu Widget

If you already have a pause/main menu widget:

1. Ensure your menu widget has appropriate buttons:
   - Resume (calls `HideMainMenu` on player controller)
   - Settings
   - Quit/Return to Main Menu

2. The widget doesn't need special initialization - it will be shown/hidden automatically

## Troubleshooting

**Menu doesn't appear:**
- Check that `MainMenuWidgetClass` is set in player controller Blueprint
- Verify Tab key is bound in IMC_Spaceship
- Check that MenuAction is assigned in DA_InputConfig
- Look for error logs in Output Log (search for "MainMenu")

**Menu appears but mouse doesn't work:**
- Check that input mode is set to `GameAndUI` in ShowMainMenu
- Verify widget has clickable buttons

**Tab key doesn't work in third-person mode:**
- Ensure IMC_ThirdPerson is created and configured
- Verify the game switches to third-person context when appropriate
- Check that Tab binding exists in IMC_ThirdPerson

## Code Reference

### C++ Usage Example
```cpp
// In your game code, you can also manually control the menu:
if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
{
    // Toggle menu
    PC->ToggleMainMenu();
    
    // Or check if menu is open
    if (PC->IsMainMenuOpen())
    {
        // Menu is currently displayed
    }
    
    // Get the menu widget
    UUserWidget* MenuWidget = PC->GetMainMenuWidget();
}
```

### Blueprint Example
```
Get Player Controller
└─ Cast to AdastreaPlayerController
   └─ Toggle Main Menu
```

## Related Files

- `Source/Adastrea/Public/Player/AdastreaPlayerController.h` - Header with declarations
- `Source/Adastrea/Player/AdastreaPlayerController.cpp` - Implementation
- `Source/Adastrea/Public/Input/InputConfigDataAsset.h` - Input configuration
- `Content/Input/IMC_Spaceship.uasset` - Spaceship input mappings
- `Content/Input/Actions/IA_PauseMenu.uasset` - Menu action

## See Also

- [Enhanced Input Guide](ENHANCED_INPUT_GUIDE.md)
- [Input Controls Reference](INPUT_CONTROLS_REFERENCE.md)
- [MainMenuWidget Usage](Source/Adastrea/UI/MainMenuWidget_Usage.md)
