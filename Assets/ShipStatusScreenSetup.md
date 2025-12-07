# Ship Status Screen Setup Guide

## Quick Start (5 Minutes)

This guide will help you set up the ship status screen that displays comprehensive ship information and statistics.

## Prerequisites

- Unreal Engine 5.6+
- C++ code compiled (ShipStatusWidget class)
- Player Controller Blueprint set up
- A spaceship with SpaceshipDataAsset assigned

## Step 1: Create the Widget Blueprint (2 minutes)

1. **Create Widget Blueprint**
   - Navigate to `Content/UI/HUD/` folder
   - Right-click → **User Interface → Widget Blueprint**
   - Search for parent class: `ShipStatusWidget`
   - Name it: `WBP_ShipStatus`

2. **Design Basic Layout**
   - Open `WBP_ShipStatus`
   - Add a **Canvas Panel** as root
   - Add a semi-transparent **Image** for background
   - Add **Text Blocks** for ship name, class, and stats
   - Add **Progress Bars** for hull integrity and ratings
   - Add a **Button** for closing the screen

3. **Make Widgets Variables**
   - For each element you want to update, check **Is Variable** in Details panel
   - Name them clearly: `Text_ShipName`, `ProgressBar_Hull`, etc.

## Step 2: Implement Update Functions (1 minute)

Override the key update functions in the **Graph** view:

### UpdateBasicInfo (Required)
```
Event Update Basic Info (ShipName, ShipClass, Description)
    ↓
Set Text_ShipName → Text = ShipName
    ↓
Set Text_ShipClass → Text = ShipClass
```

### UpdateCoreStats (Recommended)
```
Event Update Core Stats (CurrentHull, MaxHull, ...)
    ↓
Set ProgressBar_Hull → Percent = (CurrentHull / MaxHull)
    ↓
Set Text_Hull → Text = Format("{0} / {1}", CurrentHull, MaxHull)
```

### UpdateRatings (Recommended)
```
Event Update Ratings (CombatRating, MobilityRating, UtilityRating)
    ↓
Set ProgressBar_Combat → Percent = (CombatRating / 100.0)
    ↓
Set Text_CombatRating → Text = Format("{0}%", CombatRating)
```

## Step 3: Add Close Button (30 seconds)

In the **Graph** view, add button click handler:

```
Button_Close → On Clicked
    ↓
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

## Step 4: Configure Player Controller (30 seconds)

1. Open your Player Controller Blueprint (e.g., `BP_AdastreaPlayerController`)
2. In **Class Defaults**, find **Ship Status** category
3. Set **Ship Status Widget Class** to `WBP_ShipStatus`
4. Compile and save

## Step 5: Add Input Binding (1 minute)

### Option A: Using Enhanced Input (Recommended)

1. Navigate to `Content/Input/Actions/`
2. Create new Input Action: `IA_ToggleShipStatus`
   - Value Type: Digital (bool)
3. Open your Input Mapping Context (e.g., `IMC_Spaceship`)
4. Add mapping:
   - Action: `IA_ToggleShipStatus`
   - Key: `I` (for Info) or your preferred key
5. In Player Controller Blueprint or Character Blueprint:
   ```
   Enhanced Input Action IA_ToggleShipStatus (Started)
       ↓
   Get Player Controller → Cast to AdastreaPlayerController
       ↓
   Toggle Ship Status
   ```

### Option B: Using Legacy Input

In your Player Controller Blueprint:

1. Edit → Project Settings → Input
2. Add Action Mapping:
   - Name: `ToggleShipStatus`
   - Key: `I`
3. In Player Controller Blueprint:
   ```
   InputAction ToggleShipStatus (Pressed)
       ↓
   Toggle Ship Status
   ```

## Testing

1. **Start PIE (Play in Editor)**
2. **Possess a spaceship** (make sure it has a SpaceshipDataAsset assigned)
3. **Press 'I' key** (or your bound key)
4. **Verify** that the ship status screen appears with ship data
5. **Click close button** or **press 'I' again** to close

## Minimal Example Layout

For a quick test, create this simple layout in `WBP_ShipStatus`:

```
Canvas Panel
├── Image (Background, black with 50% opacity)
├── Vertical Box (Center of screen)
│   ├── Text Block (Text_ShipName) - bind to "Display Ship Name"
│   ├── Text Block (Text_ShipClass) - bind to "Display Ship Class"
│   ├── Progress Bar (ProgressBar_Combat) - bind to "Display Combat Rating"
│   ├── Progress Bar (ProgressBar_Mobility) - bind to "Display Mobility Rating"
│   └── Button (Button_Close) - "Close" text
```

Then override just these events:
- `UpdateBasicInfo` → Set the text blocks
- `UpdateRatings` → Set the progress bars (value / 100.0)

## Common Issues

### "Widget doesn't appear"
- Check that `ShipStatusWidgetClass` is set in Player Controller
- Verify you're controlling a spaceship when pressing the key
- Check Output Log for errors: `LogAdastrea: ShipStatusWidget`

### "No ship data showing"
- Ensure your spaceship actor has `ShipDataAsset` property set
- Verify the SpaceshipDataAsset has values configured

### "Input not working"
- Check that input action is bound in Enhanced Input or Project Settings
- Verify input mapping context is added to player controller
- Test in PIE (Play in Editor), not just the widget designer

## Advanced Features

### Real-time Updates

To update stats in real-time while the screen is open:

```
Event Tick (DeltaTime)
    ↓
Branch: Is Ship Status Open?
    True → Refresh Ship Status (every 0.5 seconds using timer)
```

### Custom Stat Panels

Add more detailed stat panels by:
1. Creating more Text Blocks for each stat
2. Overriding `UpdateCombatStats`, `UpdateMobilityStats`, etc.
3. Binding each stat to its corresponding widget

### Color-Coded Ratings

Color code ratings based on value:

```
Event Update Ratings (CombatRating, ...)
    ↓
Branch: CombatRating >= 75
    True → Set Color = Green
    False → Branch: CombatRating >= 50
        True → Set Color = Yellow
        False → Set Color = Red
```

## Complete Stat Categories

The ShipStatusWidget supports all these stat categories:

1. **Basic Info**: Name, Class, Description
2. **Core Stats**: Hull, Cargo, Crew, Modular Points
3. **Combat Stats**: Armor, Shields, Weapons, Point Defense
4. **Mobility Stats**: Speed, Acceleration, Maneuverability, Jump Range, Fuel
5. **Utility Stats**: Sensors, Stealth, Repair, Science, Medical
6. **Operational Stats**: Power, Life Support, Maintenance, Hangars, Drones, AI
7. **Advanced Stats**: Diplomacy, Boarding, Customization, E-War, Mining
8. **Lore Info**: Manufacturer, Year Built, Rarity, Notes
9. **Ratings**: Combat, Mobility, Utility (0-100 calculated values)

Override the corresponding `Update[Category]Stats` functions to display them.

## Blueprint Functions

Available functions to call from Blueprint:

- `Toggle Ship Status` - Show/hide the screen
- `Is Ship Status Open` - Check if screen is visible
- `Get Ship Status Widget` - Get widget instance
- `Refresh Ship Status` - Manually update all displayed data
- `Initialize With Spaceship` - Initialize with specific spaceship

## Integration Examples

### HUD Integration

Add a button to your main HUD:

```
HUD Button "Ship Info" (Clicked)
    ↓
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

### Context Menu Integration

Add to a radial menu or right-click context menu:

```
Menu Item "View Ship Status" (Selected)
    ↓
Toggle Ship Status
```

### Keyboard Shortcut

Bind multiple keys for easy access:
- `I` - Info/Inventory
- `Tab` - Quick stats
- `H` - Help/Ship info

## Next Steps

1. **Customize the layout** to match your game's art style
2. **Add animations** for smooth transitions (fade in/out)
3. **Implement tabs** if you have many stats to show
4. **Add tooltips** for stat explanations
5. **Link to ship customization** screen for modifications

## See Also

- [Ship Status Widget Usage Guide](../../Source/Adastrea/UI/ShipStatusWidget_Usage.md) - Detailed implementation guide
- [Spaceship Data Asset Guide](SpaceshipDataAssetGuide.md) - Complete ship stats reference
- [Enhanced Input Guide](EnhancedInputImplementation.md) - Input system setup
- [HUD System Guide](../Source/Adastrea/UI/AdastreaHUDWidget.h) - Main HUD integration

## Support

If you encounter issues:
1. Check the Output Log for `LogAdastrea` messages
2. Verify all prerequisites are met
3. Test with a minimal layout first
4. Refer to existing widget examples in `Content/UI/`

---

**Estimated Setup Time**: 5 minutes  
**Difficulty**: Beginner  
**Required Skills**: Basic Blueprint knowledge, UI design
