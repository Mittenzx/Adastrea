# Ship Status Screen - Blueprint Implementation Example

## Overview

This document provides a complete Blueprint implementation example for the Ship Status Screen system.

## File Locations

- **C++ Class**: `Source/Adastrea/UI/ShipStatusWidget.h`
- **Widget Blueprint**: `Content/UI/HUD/WBP_ShipStatus.uasset` (to be created)
- **Player Controller**: `Content/Blueprints/BP_AdastreaPlayerController.uasset`
- **Input Action**: `Content/Input/Actions/IA_ToggleShipStatus.uasset` (to be created)

## Complete Blueprint Implementation

### 1. Widget Blueprint Structure

**File**: `WBP_ShipStatus`

#### Widget Hierarchy
```
Canvas Panel (Root)
├── Image_Background
│   └── Color and Opacity: (R=0, G=0, B=0, A=0.85)
│   └── Size: Fill Screen
├── VerticalBox_Main
│   ├── HorizontalBox_Header
│   │   ├── TextBlock_ShipName
│   │   │   └── Font Size: 48
│   │   │   └── Color: White
│   │   │   └── Is Variable: ✓
│   │   ├── Spacer
│   │   └── Button_Close
│   │       └── Text: "X"
│   ├── HorizontalBox_ClassAndRatings
│   │   ├── TextBlock_ShipClass
│   │   │   └── Font Size: 24
│   │   │   └── Color: Gray
│   │   │   └── Is Variable: ✓
│   │   ├── Spacer
│   │   ├── TextBlock_CombatRating
│   │   │   └── Text: "Combat: 0%"
│   │   │   └── Is Variable: ✓
│   │   ├── TextBlock_MobilityRating
│   │   │   └── Text: "Mobility: 0%"
│   │   │   └── Is Variable: ✓
│   │   └── TextBlock_UtilityRating
│   │       └── Text: "Utility: 0%"
│   │       └── Is Variable: ✓
│   ├── TextBlock_Description
│   │   └── Font Size: 18
│   │   └── Auto Wrap Text: ✓
│   │   └── Is Variable: ✓
│   ├── HorizontalBox_Stats
│   │   ├── VerticalBox_CoreStats
│   │   │   ├── TextBlock_CoreHeader (Text: "CORE STATS")
│   │   │   ├── HorizontalBox_Hull
│   │   │   │   ├── TextBlock_HullLabel (Text: "Hull Integrity:")
│   │   │   │   └── TextBlock_Hull (Is Variable: ✓)
│   │   │   ├── ProgressBar_Hull (Is Variable: ✓)
│   │   │   ├── TextBlock_Cargo (Is Variable: ✓)
│   │   │   ├── TextBlock_Crew (Is Variable: ✓)
│   │   │   └── TextBlock_ModularPoints (Is Variable: ✓)
│   │   ├── VerticalBox_CombatStats
│   │   │   ├── TextBlock_CombatHeader (Text: "COMBAT")
│   │   │   ├── TextBlock_Armor (Is Variable: ✓)
│   │   │   ├── TextBlock_Shield (Is Variable: ✓)
│   │   │   ├── TextBlock_Weapons (Is Variable: ✓)
│   │   │   └── TextBlock_PointDefense (Is Variable: ✓)
│   │   ├── VerticalBox_MobilityStats
│   │   │   ├── TextBlock_MobilityHeader (Text: "MOBILITY")
│   │   │   ├── TextBlock_MaxSpeed (Is Variable: ✓)
│   │   │   ├── TextBlock_Acceleration (Is Variable: ✓)
│   │   │   ├── TextBlock_Maneuverability (Is Variable: ✓)
│   │   │   └── TextBlock_JumpRange (Is Variable: ✓)
│   │   └── VerticalBox_UtilityStats
│   │       ├── TextBlock_UtilityHeader (Text: "UTILITY")
│   │       ├── TextBlock_SensorRange (Is Variable: ✓)
│   │       ├── TextBlock_SensorResolution (Is Variable: ✓)
│   │       ├── TextBlock_StealthRating (Is Variable: ✓)
│   │       └── TextBlock_RepairRating (Is Variable: ✓)
│   └── HorizontalBox_Footer
│       ├── ProgressBar_CombatRating (Is Variable: ✓)
│       ├── ProgressBar_MobilityRating (Is Variable: ✓)
│       └── ProgressBar_UtilityRating (Is Variable: ✓)
```

### 2. Event Graph Implementation

#### Event: Update Basic Info

```blueprint
Event Update Basic Info
    Inputs:
        - Ship Name (Text)
        - Ship Class (Text)
        - Description (Text)
    ↓
    Set Text (TextBlock_ShipName)
        Text: Ship Name
    ↓
    Set Text (TextBlock_ShipClass)
        Text: Ship Class
    ↓
    Set Text (TextBlock_Description)
        Text: Description
```

#### Event: Update Core Stats

```blueprint
Event Update Core Stats
    Inputs:
        - Current Hull (float)
        - Max Hull (float)
        - Cargo Capacity (float)
        - Crew Required (int32)
        - Max Crew (int32)
        - Modular Points (int32)
    ↓
    Set Percent (ProgressBar_Hull)
        Percent: Current Hull / Max Hull
    ↓
    Set Text (TextBlock_Hull)
        Text: Format Text("{0} / {1}", Current Hull, Max Hull)
    ↓
    Set Text (TextBlock_Cargo)
        Text: Format Text("Cargo: {0} tons", Cargo Capacity)
    ↓
    Set Text (TextBlock_Crew)
        Text: Format Text("Crew: {0} / {1}", Crew Required, Max Crew)
    ↓
    Set Text (TextBlock_ModularPoints)
        Text: Format Text("Modules: {0}", Modular Points)
```

#### Event: Update Combat Stats

```blueprint
Event Update Combat Stats
    Inputs:
        - Armor Rating (float)
        - Shield Strength (float)
        - Shield Recharge Rate (float)
        - Weapon Slots (int32)
        - Weapon Power (float)
        - Point Defense (float)
    ↓
    Set Text (TextBlock_Armor)
        Text: Format Text("Armor: {0}", Armor Rating)
    ↓
    Set Text (TextBlock_Shield)
        Text: Format Text("Shields: {0} ({1}/s)", Shield Strength, Shield Recharge Rate)
    ↓
    Set Text (TextBlock_Weapons)
        Text: Format Text("Weapons: {0} slots ({1} power)", Weapon Slots, Weapon Power)
    ↓
    Set Text (TextBlock_PointDefense)
        Text: Format Text("Point Defense: {0}", Point Defense)
```

#### Event: Update Mobility Stats

```blueprint
Event Update Mobility Stats
    Inputs:
        - Max Speed (float)
        - Acceleration (float)
        - Maneuverability (int32)
        - Jump Range (float)
        - Fuel Capacity (float)
        - Fuel Consumption (float)
    ↓
    Set Text (TextBlock_MaxSpeed)
        Text: Format Text("Max Speed: {0}", Max Speed)
    ↓
    Set Text (TextBlock_Acceleration)
        Text: Format Text("Acceleration: {0}", Acceleration)
    ↓
    Set Text (TextBlock_Maneuverability)
        Text: Format Text("Maneuverability: {0}", Maneuverability)
    ↓
    Set Text (TextBlock_JumpRange)
        Text: Format Text("Jump Range: {0} ly", Jump Range)
```

#### Event: Update Utility Stats

```blueprint
Event Update Utility Stats
    Inputs:
        - Sensor Range (float)
        - Sensor Resolution (int32)
        - Stealth Rating (int32)
        - Repair Rating (int32)
        - Science Rating (int32)
        - Medical Rating (int32)
    ↓
    Set Text (TextBlock_SensorRange)
        Text: Format Text("Sensors: {0} units (Res: {1})", Sensor Range, Sensor Resolution)
    ↓
    Set Text (TextBlock_StealthRating)
        Text: Format Text("Stealth: {0}", Stealth Rating)
    ↓
    Set Text (TextBlock_RepairRating)
        Text: Format Text("Repair: {0}", Repair Rating)
```

#### Event: Update Ratings

```blueprint
Event Update Ratings
    Inputs:
        - Combat Rating (float)
        - Mobility Rating (float)
        - Utility Rating (float)
    ↓
    Set Percent (ProgressBar_CombatRating)
        Percent: Combat Rating / 100.0
    ↓
    Set Text (TextBlock_CombatRating)
        Text: Format Text("Combat: {0}%", Combat Rating)
    ↓
    Set Percent (ProgressBar_MobilityRating)
        Percent: Mobility Rating / 100.0
    ↓
    Set Text (TextBlock_MobilityRating)
        Text: Format Text("Mobility: {0}%", Mobility Rating)
    ↓
    Set Percent (ProgressBar_UtilityRating)
        Percent: Utility Rating / 100.0
    ↓
    Set Text (TextBlock_UtilityRating)
        Text: Format Text("Utility: {0}%", Utility Rating)
```

#### Event: Button Close Clicked

```blueprint
Button_Close → On Clicked
    ↓
    Get Player Controller
    ↓
    Cast to AdastreaPlayerController
    ↓
    Toggle Ship Status
```

### 3. Player Controller Setup

**File**: `BP_AdastreaPlayerController`

#### Class Defaults Configuration

1. Open `BP_AdastreaPlayerController` Blueprint
2. Select **Class Defaults** in toolbar
3. In Details panel, find **Ship Status** category:
   - **Ship Status Widget Class**: Select `WBP_ShipStatus`
4. Compile and Save

### 4. Input Action Setup

#### Create Input Action

**File**: `IA_ToggleShipStatus`

1. Navigate to `Content/Input/Actions/`
2. Right-click → Input → Input Action
3. Name: `IA_ToggleShipStatus`
4. Open it and set:
   - **Value Type**: Digital (bool)

#### Add to Input Mapping Context

**File**: `IMC_Spaceship` (or your input mapping context)

1. Open your Input Mapping Context
2. Add Mapping:
   - **Action**: `IA_ToggleShipStatus`
   - **Key**: Keyboard `I` (or your preferred key)

#### Bind Input in Player Controller

**File**: `BP_AdastreaPlayerController` (Event Graph)

```blueprint
Enhanced Input Action IA_ToggleShipStatus (Started)
    ↓
    Toggle Ship Status (self)
```

Or if using a custom Pawn/Character:

```blueprint
Enhanced Input Action IA_ToggleShipStatus (Started)
    ↓
    Get Player Controller
    ↓
    Cast to AdastreaPlayerController
    ↓
    Toggle Ship Status
```

### 5. Alternative: HUD Button Integration

If you want to add a button to your main HUD to open the ship status:

**File**: `WBP_SpaceShipHUD` (or your HUD widget)

```blueprint
Button_ShipInfo → On Clicked
    ↓
    Get Owning Player
    ↓
    Cast to AdastreaPlayerController
    ↓
    Toggle Ship Status
```

## Testing Instructions

1. **Compile C++ Code**: Ensure ShipStatusWidget is compiled
2. **Create Widget**: Create `WBP_ShipStatus` as shown above
3. **Configure Controller**: Set widget class in Player Controller
4. **Setup Input**: Create and bind input action
5. **Test in PIE**:
   - Press Play
   - Possess a spaceship (with SpaceshipDataAsset assigned)
   - Press `I` key
   - Verify ship status screen appears with data
   - Press `I` again or click Close to hide

## Expected Behavior

- Pressing `I` while controlling a spaceship opens the ship status screen
- Screen displays all ship information from the SpaceshipDataAsset
- Progress bars show hull integrity and rating percentages
- Pressing `I` again or clicking Close button hides the screen
- Mouse cursor appears when screen is open
- Game input is restored when screen is closed

## Troubleshooting

### Widget doesn't show
- Check Output Log for `LogAdastrea: ShipStatusWidget` messages
- Verify `ShipStatusWidgetClass` is set in Player Controller Class Defaults
- Ensure you're controlling a spaceship when pressing the key

### No data displays
- Check that your spaceship has `ShipDataAsset` property assigned
- Verify SpaceshipDataAsset has values configured
- Override update functions are properly implemented

### Input doesn't work
- Verify input action is created and bound
- Check input mapping context is added to player controller
- Test that Enhanced Input system is properly configured

## Styling Recommendations

### Colors
- **Background**: Dark semi-transparent (0, 0, 0, 0.85)
- **Headers**: White (#FFFFFF)
- **Subheaders**: Light gray (#CCCCCC)
- **Labels**: Medium gray (#999999)
- **Values**: White (#FFFFFF)
- **Progress bars**: 
  - Green for high values (75-100%)
  - Yellow for medium (50-75%)
  - Red for low (0-50%)

### Fonts
- **Ship Name**: 48pt Bold
- **Ship Class**: 24pt Regular
- **Section Headers**: 18pt Bold
- **Stats Labels**: 14pt Regular
- **Stats Values**: 14pt Bold

### Layout
- **Margins**: 20px between sections
- **Padding**: 10px within panels
- **Alignment**: Left for labels, Right for values
- **Progress bars**: Width 200px, Height 20px

## Advanced Features

### Tab System

Add tabs for different stat categories:

```blueprint
Button_CoreTab → On Clicked
    ↓
    Set Visibility (VerticalBox_CoreStats)
        Visibility: Visible
    ↓
    Set Visibility (VerticalBox_CombatStats)
        Visibility: Collapsed
    ... (repeat for other tabs)
```

### Real-time Updates

Update stats periodically while screen is open:

```blueprint
Event Tick
    ↓
    Branch (Is Ship Status Open)
        True →
            Delay (0.5 seconds)
            ↓
            Refresh Ship Status
```

### Color-Coded Ratings

Change progress bar colors based on rating:

```blueprint
Event Update Ratings (Combat Rating, ...)
    ↓
    Branch (Combat Rating >= 75)
        True → Set Fill Color (ProgressBar_Combat) = Green
        False →
            Branch (Combat Rating >= 50)
                True → Set Fill Color = Yellow
                False → Set Fill Color = Red
```

## See Also

- [Ship Status Widget Usage Guide](../Source/Adastrea/UI/ShipStatusWidget_Usage.md)
- [Ship Status Screen Setup Guide](../Assets/ShipStatusScreenSetup.md)
- [Spaceship Data Asset Guide](../Assets/SpaceshipDataAssetGuide.md)
- [Enhanced Input System Guide](../Assets/EnhancedInputImplementation.md)
