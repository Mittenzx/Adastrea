# Blueprint Zero-Setup Guide - Instant UI Widgets

**Last Updated**: December 16, 2025  
**For**: Designers who want instant results  
**Time**: 2 minutes per widget

---

## What This Guide Does

Shows you how to create fully functional UI widgets **with ZERO manual setup**. All positioning, colors, fonts, and layouts are handled automatically by C++.

## Quick Start

### Create a Universe Map Widget (2 minutes)

1. **Open Content Browser** (bottom of Unreal Editor)
2. **Navigate to** `Content/UI/` folder (create if needed)
3. **Right-click** in empty space → **User Interface** → **Widget Blueprint**
4. **Search for parent class**: Type `UniverseMapWidget`
5. **Select** `UniverseMapWidget` and click **Select**
6. **Name it**: `WBP_UniverseMap`
7. **Save** (Ctrl+S)

**Done!** Your universe map widget now has:
- ✅ Dark blue background
- ✅ Map container with proper sizing
- ✅ Sector grid panel
- ✅ Info panel with sector name and description
- ✅ Exploration progress bar
- ✅ Functional close button

### Create a Ship Status Widget (2 minutes)

1. **Right-click** → **User Interface** → **Widget Blueprint**
2. **Parent class**: Search `ShipStatusWidget`
3. **Name**: `WBP_ShipStatus`
4. **Save**

**Done!** Your ship status widget has:
- ✅ Dark background panel
- ✅ Ship name (large, centered)
- ✅ Ship class display
- ✅ Description text
- ✅ Combat/Mobility/Utility ratings
- ✅ Stats scroll box
- ✅ Functional close button

### Create a Sector Map Widget (2 minutes)

1. **Right-click** → **User Interface** → **Widget Blueprint**
2. **Parent class**: Search `SectorMapWidget`
3. **Name**: `WBP_SectorMap`
4. **Save**

**Done!** Your sector map widget has:
- ✅ Background panel
- ✅ Sector name display
- ✅ Sector description
- ✅ Object count
- ✅ Scrollable object list

---

## How It Works

### The Magic: C++ Auto-Creation

When you create a Widget Blueprint from these C++ classes, the C++ code **automatically creates all UI elements** for you in the `NativeConstruct()` function.

No dragging widgets from Palette.  
No positioning or sizing.  
No color selection.  
No font configuration.

**It just works.**

### What Gets Created Automatically

**UniverseMapWidget** creates:
- Canvas Panel (root)
- Background Image (dark blue, fills screen)
- Map Container Border (centered, 1520x880)
- Sector Grid Panel (inside container)
- Info Panel (right side, 300x880)
- Text blocks for sector name and description
- Progress bar for exploration
- Close button (top-right corner)

**ShipStatusWidget** creates:
- Canvas Panel (root)
- Background Border (centered, 800x900)
- Vertical Box for content layout
- Text blocks for ship name, class, description
- Text blocks for combat, mobility, utility ratings
- Scroll Box for detailed stats
- Close button

**SectorMapWidget** creates:
- Canvas Panel (root)
- Background Border (centered, 600x700)
- Vertical Box for layout
- Text blocks for sector name, description, object count
- Scroll Box for object list

---

## Testing Your Widget

### Quick Test in Editor

1. **Open** your widget Blueprint
2. **Click** the **Designer** tab
3. You should see **all widgets already positioned**
4. **Click** the **Graph** tab
5. You should see **Event Construct** (this is where C++ initializes)
6. **Compile** (green checkmark button)
7. **Play** in Editor

The widget works immediately!

### Add to Level for Testing

1. **Open** a test level
2. **Open Level Blueprint** (Blueprints menu → Open Level Blueprint)
3. **Add Event BeginPlay** (right-click → Add Event → Event BeginPlay)
4. **Create Widget** node:
   - Drag from BeginPlay → type "Create Widget"
   - **Class**: Select your widget (e.g., `WBP_UniverseMap`)
5. **Add to Viewport** node:
   - Drag from Create Widget output → type "Add to Viewport"
6. **Compile and Save**
7. **Play in Editor** (Alt+P)

You should see your widget!

---

## Customization (Optional)

### Want to Change Colors or Positions?

You have **two options**:

#### Option 1: Override in Blueprint Designer (Easy)

1. **Open** your widget Blueprint
2. **Designer** tab
3. **Select** a widget in the hierarchy (e.g., "Text_SectorName")
4. **Details** panel → change color, font, size, position
5. **Compile and Save**

**Note**: C++ creates widgets with specific names. Your custom changes will override the C++ defaults.

#### Option 2: Create Custom Widgets (Advanced)

1. **Open** your widget Blueprint
2. **Designer** tab
3. **Delete** auto-created widgets (optional)
4. **Create your own** widgets with the EXACT same names:
   - For UniverseMapWidget: `Text_SectorName`, `Text_SectorDescription`, etc.
   - For ShipStatusWidget: `Text_ShipName`, `Text_ShipClass`, etc.
   - For SectorMapWidget: `Text_SectorName`, `Text_SectorDescription`, etc.
5. **C++ will find and use** your custom widgets instead

**Widget Name Reference** (must match exactly):

**UniverseMapWidget**:
- `SectorGridPanel` (Canvas Panel)
- `Text_SectorName` (Text Block)
- `Text_SectorDescription` (Text Block)
- `ProgressBar_Exploration` (Progress Bar)
- `Text_ExplorationPercent` (Text Block)
- `Button_Close` (Button)

**ShipStatusWidget**:
- `Text_ShipName` (Text Block)
- `Text_ShipClass` (Text Block)
- `Text_Description` (Text Block)
- `Text_CombatRating` (Text Block)
- `Text_MobilityRating` (Text Block)
- `Text_UtilityRating` (Text Block)
- `StatsScrollBox` (Scroll Box)
- `Button_Close` (Button)

**SectorMapWidget**:
- `Text_SectorName` (Text Block)
- `Text_SectorDescription` (Text Block)
- `Text_ObjectCount` (Text Block)
- `ObjectListScrollBox` (Scroll Box)

---

## Disabling Auto-Creation (Advanced)

If you want **full manual control**:

1. **Open** your widget Blueprint
2. **Class Defaults** button (toolbar)
3. **Find** `bAutoCreateMissingWidgets`
4. **Uncheck** it
5. **Compile**

Now C++ won't create widgets automatically. You must create them all manually in Blueprint Designer.

---

## Troubleshooting

### "I don't see any widgets!"

**Solution 1**: Check if auto-create is enabled
- Open Blueprint → Class Defaults → `bAutoCreateMissingWidgets` should be checked

**Solution 2**: Recompile
- Click the green checkmark button in toolbar

**Solution 3**: Check Output Log
- Window → Developer Tools → Output Log
- Look for messages like "UniverseMapWidget: Created background image"

### "My custom widget isn't showing"

**Problem**: Widget name doesn't match C++ property name

**Solution**: 
1. Check widget name exactly matches (case-sensitive!)
2. Check widget type matches (TextBlock, Button, etc.)
3. Recompile Blueprint

### "Close button doesn't work"

**Problem**: Button might not be bound

**Solution**:
- C++ automatically binds `Button_Close` to `OnCloseButtonClicked`
- Check Output Log for errors
- Try recompiling Blueprint

---

## Best Practices

### ✅ DO:
- Start with zero setup and test immediately
- Customize appearance in Designer after confirming it works
- Use exact widget names if creating custom widgets
- Keep `bAutoCreateMissingWidgets` enabled for quick iteration

### ❌ DON'T:
- Don't rename auto-created widgets (C++ won't find them)
- Don't delete widgets without creating replacements
- Don't disable auto-create unless you know what you're doing
- Don't create duplicate widgets with the same name

---

## Next Steps

### Want More Control?
- See **BLUEPRINT_CUSTOMIZATION_GUIDE.md** (coming soon)
- See **BLUEPRINT_CPP_IMPROVEMENTS.md** for technical details on C++ implementation

### Want to Learn More?
- See **BLUEPRINT_CPP_IMPROVEMENTS.md** for technical details
- See **.github/copilot-instructions.md** for project coding standards

### Want to Add Your Own Widgets?
- Study the pattern in `UniverseMapWidget.cpp`
- Look for `CreateDefaultUIWidgets()` function
- Copy the pattern for your own widget classes

---

## Summary

**Before**: 30+ steps to create a functional widget  
**Now**: 3 steps - Create Blueprint, Name it, Save

The C++ code handles:
- ✅ All widget creation
- ✅ All positioning and sizing
- ✅ All colors and styling
- ✅ All fonts and text
- ✅ All event binding (buttons, etc.)

You focus on:
- ✅ Using the widget in your game
- ✅ Optional customization of appearance
- ✅ Creating gameplay logic

**Welcome to instant productivity!** 🚀
