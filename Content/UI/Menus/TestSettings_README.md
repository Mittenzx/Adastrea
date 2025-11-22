# Test Settings Pre-Screen

## Overview

This directory contains assets for the Test Settings pre-screen widget, which allows developers and QA testers to configure testing parameters before entering the game.

## Purpose

The Test Settings screen provides a convenient way to:
- Select which spaceship to spawn for testing
- Configure difficulty level for gameplay testing
- Enable debug visualization modes
- Toggle auto-spawn behavior
- Quickly iterate on different testing scenarios

## Files in This Directory

### Widget Blueprint (To Be Created)
- **WBP_TestSettings** - Main widget blueprint based on `UTestSettingsWidget`
  - Should contain ComboBox widgets for ship selection, difficulty, and debug modes
  - Should contain Continue and Reset buttons
  - Should contain auto-spawn checkbox

## Creating the Widget Blueprint

Follow the instructions in `/Source/Adastrea/UI/TestSettingsWidget_Usage.md` for detailed setup steps.

Quick steps:
1. Create new Widget Blueprint
2. Reparent to `TestSettingsWidget`
3. Add ComboBox widgets for each dropdown
4. Bind button events
5. Override `TransitionToMainMenu` to show main menu

## Usage in Levels

To display the test settings screen on level load:

1. Open your test/development level
2. Open Level Blueprint
3. Add this logic on BeginPlay:
   ```
   Event BeginPlay
     ↓
   Create Widget (WBP_TestSettings)
     ↓
   Add to Viewport
   ```

## Integration with GameMode

The test settings can be read by your GameMode to configure the test session:

```cpp
// In GameMode BeginPlay or similar
UTestSettingsWidget* TestSettings = Cast<UTestSettingsWidget>(
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, UTestSettingsWidget::StaticClass())[0]
);

if (TestSettings && TestSettings->GetAutoSpawnEnabled())
{
    FString ShipType = TestSettings->GetSelectedShipType();
    // Use ShipType to spawn the appropriate ship
}
```

## Removing from Shipping Builds

For production builds, you can:
1. Exclude this widget from packaged builds using exclusion lists
2. Use preprocessor directives to disable the screen
3. Only show the widget if `WITH_EDITOR` or `!UE_BUILD_SHIPPING` is defined

## See Also

- `/Source/Adastrea/UI/TestSettingsWidget.h` - C++ header file
- `/Source/Adastrea/UI/TestSettingsWidget.cpp` - C++ implementation
- `/Source/Adastrea/UI/TestSettingsWidget_Usage.md` - Detailed usage guide
- `/Content/UI/Menus/WPB_MainMenu.uasset` - Main menu widget
