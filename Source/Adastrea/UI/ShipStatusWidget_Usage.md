# Ship Status Widget Usage Guide

## Overview

The `UShipStatusWidget` class provides a comprehensive display of spaceship information and statistics. It automatically retrieves data from the player's controlled spaceship and displays all stat categories defined in the `SpaceshipDataAsset`.

## Features

- **Automatic Data Loading**: Automatically gets ship data from the player's controlled spaceship
- **Comprehensive Stats Display**: Shows all 7 stat categories plus lore information
- **Rating Calculations**: Displays computed combat, mobility, and utility ratings
- **Blueprint Extensible**: All update functions are BlueprintNativeEvents for easy customization
- **PlayerController Integration**: Managed by `AAdastreaPlayerController` for show/hide

## Creating a Ship Status Widget Blueprint

### Step 1: Create the Widget Blueprint

1. **Navigate to Content Browser**
   - Recommended location: `Content/UI/HUD/`
   - Create the folder if it doesn't exist

2. **Create the Widget Blueprint**
   - Right-click in Content Browser
   - Select **User Interface → Widget Blueprint**
   - In the "Pick Parent Class" dialog, search for `ShipStatusWidget`
   - Select **ShipStatusWidget**
   - Name it: `WBP_ShipStatus`

### Step 2: Design the UI Layout

Open `WBP_ShipStatus` and design your ship status screen. Here's a recommended layout structure:

```
Canvas Panel (Root)
├── Background Image (full screen overlay with transparency)
├── Main Container (Vertical Box or Canvas)
│   ├── Header Section
│   │   ├── Ship Name (Text Block)
│   │   ├── Ship Class (Text Block)
│   │   └── Close Button
│   ├── Stats Container (Horizontal Box or Grid)
│   │   ├── Left Column
│   │   │   ├── Basic Info Panel
│   │   │   │   ├── Description (Text Block)
│   │   │   │   └── Hull Integrity (Progress Bar)
│   │   │   ├── Core Stats Panel
│   │   │   │   ├── Cargo Capacity
│   │   │   │   ├── Crew Required/Max
│   │   │   │   └── Modular Points
│   │   │   └── Combat Stats Panel
│   │   │       ├── Armor Rating
│   │   │       ├── Shield Strength
│   │   │       ├── Weapon Slots
│   │   │       └── Point Defense
│   │   ├── Middle Column
│   │   │   ├── Mobility Stats Panel
│   │   │   │   ├── Max Speed
│   │   │   │   ├── Acceleration
│   │   │   │   ├── Maneuverability
│   │   │   │   └── Jump Range
│   │   │   └── Utility Stats Panel
│   │   │       ├── Sensor Range
│   │   │       ├── Sensor Resolution
│   │   │       ├── Stealth Rating
│   │   │       └── Repair/Science/Medical
│   │   └── Right Column
│   │       ├── Operational Stats Panel
│   │       │   ├── Power Capacity
│   │       │   ├── Life Support
│   │       │   ├── Hangar/Drone Capacity
│   │       │   └── AI Rating
│   │       ├── Advanced Stats Panel
│   │       │   ├── Diplomacy Rating
│   │       │   ├── Boarding Capability
│   │       │   └── Mining Rating
│   │       └── Lore Panel
│   │           ├── Manufacturer
│   │           ├── Year Built
│   │           └── Rarity Tier
│   └── Footer Section
│       ├── Combat Rating (Progress Bar)
│       ├── Mobility Rating (Progress Bar)
│       └── Utility Rating (Progress Bar)
```

### Step 3: Bind Widget Variables

For each UI element you want to update from C++, you need to make it a variable:

1. Select the widget element in the designer
2. In the Details panel, check **Is Variable**
3. Name it descriptively (e.g., `Text_ShipName`, `ProgressBar_HullIntegrity`, `Text_CargoCapacity`)

### Step 4: Override BlueprintNativeEvents

Override the update functions to bind data to your UI elements:

#### Example: UpdateBasicInfo

1. In the **Graph** view, override `Update Basic Info`
2. Implement like this:

```
Event Update Basic Info (ShipName, ShipClass, Description)
    ↓
Set Text_ShipName → Text = ShipName
    ↓
Set Text_ShipClass → Text = ShipClass
    ↓
Set Text_Description → Text = Description
```

#### Example: UpdateCoreStats

```
Event Update Core Stats (CurrentHull, MaxHull, CargoCapacity, CrewRequired, MaxCrew, ModularPoints)
    ↓
Set ProgressBar_HullIntegrity → Percent = (CurrentHull / MaxHull)
    ↓
Set Text_HullIntegrity → Text = Format("{0} / {1}", CurrentHull, MaxHull)
    ↓
Set Text_CargoCapacity → Text = Format("{0} tons", CargoCapacity)
    ↓
Set Text_Crew → Text = Format("{0} / {1}", CrewRequired, MaxCrew)
    ↓
Set Text_ModularPoints → Text = ModularPoints
```

#### Example: UpdateRatings

```
Event Update Ratings (CombatRating, MobilityRating, UtilityRating)
    ↓
Set ProgressBar_CombatRating → Percent = (CombatRating / 100.0)
    ↓
Set Text_CombatRating → Text = Format("{0}%", CombatRating)
    ↓
Set ProgressBar_MobilityRating → Percent = (MobilityRating / 100.0)
    ↓
Set Text_MobilityRating → Text = Format("{0}%", MobilityRating)
    ↓
Set ProgressBar_UtilityRating → Percent = (UtilityRating / 100.0)
    ↓
Set Text_UtilityRating → Text = Format("{0}%", UtilityRating)
```

### Step 5: Add Close Button Functionality

Add a button to close the ship status screen:

1. Add a Button widget to your layout
2. In the **Graph** view, add an `On Clicked` event for the button
3. Implement:

```
Button_Close → On Clicked
    ↓
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status (closes the screen)
```

### Step 6: Configure in PlayerController Blueprint

1. Open your Player Controller Blueprint (e.g., `BP_AdastreaPlayerController`)
2. In the **Class Defaults**, find **Ship Status** category
3. Set **Ship Status Widget Class** to your `WBP_ShipStatus`
4. Compile and save

## Using the Ship Status Widget

### In C++

```cpp
// Get the player controller
AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController());
if (PC)
{
    // Toggle the ship status screen
    PC->ToggleShipStatus();
    
    // Check if it's open
    if (PC->IsShipStatusOpen())
    {
        // Get the widget
        UShipStatusWidget* Widget = PC->GetShipStatusWidget();
        // Manually refresh data if needed
        Widget->RefreshShipStatus();
    }
}
```

### In Blueprints

```
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status (shows/hides the screen)
```

### Input Binding

To bind a key to toggle the ship status screen:

1. Open your Input Action asset (e.g., `IA_ToggleShipStatus`)
2. Or create a new Input Action in `Content/Input/`
3. In your Player Controller Blueprint or GameMode:
   - Bind the input action to call `Toggle Ship Status`

Example in Blueprint:
```
IA_ToggleShipStatus (Triggered)
    ↓
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

## Advanced Customization

### Custom Stat Categories

You can add custom stat display sections by:

1. Adding new Text Blocks in the designer
2. Creating a new Blueprint function to update them
3. Calling that function from one of the existing Update events

### Dynamic Styling

Use the rating values to dynamically style elements:

```
Event Update Ratings (CombatRating, MobilityRating, UtilityRating)
    ↓
Branch: CombatRating >= 75
    True → Set ProgressBar_Combat Color = Green
    False → Branch: CombatRating >= 50
        True → Set ProgressBar_Combat Color = Yellow
        False → Set ProgressBar_Combat Color = Red
```

### Real-time Updates

To update the display in real-time while the screen is open:

1. Override `NativeTick` in Blueprint
2. Call `Refresh Ship Status` periodically
3. Add a timer to control update frequency

## Blueprint Properties

The following properties are available in Blueprint for direct access:

- `DisplayShipName` (FText) - Currently displayed ship name
- `DisplayShipClass` (FText) - Currently displayed ship class
- `DisplayDescription` (FText) - Currently displayed description
- `DisplayCombatRating` (float) - Calculated combat rating (0-100)
- `DisplayMobilityRating` (float) - Calculated mobility rating (0-100)
- `DisplayUtilityRating` (float) - Calculated utility rating (0-100)
- `CurrentSpaceship` (ASpaceship*) - Reference to the displayed spaceship
- `ShipDataAsset` (USpaceshipDataAsset*) - Reference to the ship's data asset

## Troubleshooting

### Widget Doesn't Show

- Check that `ShipStatusWidgetClass` is set in the Player Controller
- Verify you're calling `ToggleShipStatus()` when controlling a spaceship
- Check the log for error messages: `LogAdastrea: ShipStatusWidget`

### Data Not Displaying

- Ensure your spaceship has a `ShipDataAsset` assigned
- Check that widget variables are properly bound in the designer
- Verify that update functions are properly overridden in Blueprint

### Input Not Working

- Make sure input action is properly bound in the Enhanced Input system
- Check that the input mapping context is added to the player controller
- Verify input mode allows UI input when the widget is open

## Example Implementations

See the following for reference:

- `Content/UI/HUD/WBP_SpaceShipHUD.uasset` - Example HUD layout patterns
- `Content/UI/Stations/WBP_StationEditor.uasset` - Similar panel-based UI
- `Source/Adastrea/UI/ShipCustomizationWidget.h` - Similar stat display widget

## Integration with Other Systems

### HUD Integration

The ship status widget can be integrated with the main HUD:

- Add a button to the HUD that calls `Toggle Ship Status`
- Show a mini version of ship stats on the HUD permanently
- Link to ship customization from the ship status screen

### Save/Load Integration

Ship data is automatically loaded from the `SpaceshipDataAsset`. If you want to show dynamic values (current hull, etc.), these are retrieved from the `ASpaceship` instance.

## Performance Considerations

- The widget only updates when explicitly refreshed
- Consider calling `RefreshShipStatus()` only when values change
- Use Blueprint timers for periodic updates if needed (e.g., every 0.5 seconds)
- The widget is created once and reused for show/hide

## See Also

- [Spaceship Data Asset Guide](../../../Assets/SpaceshipDataAssetGuide.md) - Complete guide to ship stats
- [Enhanced Input Guide](../../../Assets/EnhancedInputImplementation.md) - Input system setup
- `AAdastreaPlayerController` - Player controller integration
- `UShipCustomizationWidget` - Related ship customization UI
