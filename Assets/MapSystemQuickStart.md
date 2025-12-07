# Map System Quick Start

## 5-Minute Setup Guide

Follow these steps to add the map screen to your game:

### Step 1: Create Universe Map Blueprint (2 minutes)

1. Open Unreal Editor
2. Navigate to `Content/UI/` folder
3. Right-click → **User Interface** → **Widget Blueprint**
4. Name it `WBP_UniverseMap`
5. Open the Blueprint
6. In **Class Settings** (top toolbar), set **Parent Class** to `UniverseMapWidget`
7. Save and close

### Step 2: Create Basic UI Layout (2 minutes)

Open `WBP_UniverseMap` and add these elements:

**Designer Layout**:
```
Canvas Panel (root)
├─ Image (Background) - Full screen, dark/space theme
├─ Border (Map Container) - Centered, 80% screen size
│  └─ Canvas Panel (Grid Display)
│     └─ (Your sector icons will go here in Blueprint)
├─ Vertical Box (Info Panel) - Right side
│  ├─ Text Block (Selected Sector Name)
│  ├─ Text Block (Selected Sector Description)
│  └─ Progress Bar (Exploration Progress)
└─ Button (Close Button) - Top right
   └─ Text Block ("X")
```

**Quick Designer Tips**:
- Use Anchors to make it responsive
- Set Z-Order so buttons appear on top
- Add padding for better spacing

### Step 3: Add to Player Controller (1 minute)

1. Open your Player Controller Blueprint (or create one)
2. Add this Blueprint code:

**Event Graph**:
```blueprint
// On Begin Play
Event BeginPlay
├─ Create Widget (Class: WBP_UniverseMap)
├─ Add to Viewport
├─ Set Visibility (Hidden)
└─ Promote to Variable: "MapWidget"

// On M Key Pressed (or your preferred key)
Input Action Map [or Keyboard M]
├─ Get MapWidget
├─ Is Valid?
│  True:
│  ├─ Get Visibility
│  ├─ Branch (Is Hidden?)
│  │  True: Toggle Universe Map Visibility (true)
│  │  False: Toggle Universe Map Visibility (false)
```

**Alternative with Enhanced Input**:
```blueprint
// Setup Input (Event BeginPlay)
├─ Get Player Controller
├─ Get Enhanced Input Subsystem
├─ Add Mapping Context (IMC_Spaceship, Priority: 0)

// Bind Input Action (IA_Map)
IA_Map (Started)
├─ Get MapWidget
└─ Toggle Universe Map Visibility (Toggle)
```

### Step 4: Test It!

1. Hit Play (Alt+P)
2. Press M key
3. Map should appear showing all sectors in your level
4. Press M again to close

## Basic Customization

### Change Map Key Binding

**With Enhanced Input**:
1. Open `Content/Input/IMC_Spaceship`
2. Find or add `IA_Map` mapping
3. Set your preferred key

**Without Enhanced Input**:
1. Edit → Project Settings → Input
2. Add Action Mapping: "ToggleMap"
3. Bind to your preferred key

### Add Sector Icons

In `WBP_UniverseMap` Event Graph:

```blueprint
Event UpdateUniverseGrid (Override)
├─ Get Universe Sector Info
├─ For Each Loop
│  ├─ Create Widget (Your Sector Icon Widget)
│  ├─ Set Position from Grid Coordinates
│  ├─ Set Color (Discovered vs Undiscovered)
│  ├─ Add to Grid Display Canvas
│  └─ Bind Click Event → Set Selected Sector
```

### Style the Map

Add these visual improvements:

1. **Background**: Add animated starfield or nebula
2. **Sector Icons**: Create simple circular icons with text
3. **Grid Lines**: Draw lines connecting sectors
4. **Player Marker**: Highlight current sector with glow effect
5. **Animations**: Add fade in/out when opening/closing

## Next Steps

- Read full guide: `Assets/MapSystemGuide.md`
- Add sector detail panel with `SectorMapWidget`
- Implement fast travel functionality
- Add faction territory colors
- Track exploration achievements

## Common Issues

**Map shows but no sectors appear**:
- Make sure you have `ASpaceSectorMap` actors in your level
- Check Output Log for "UniverseMapWidget: Initialized with X sectors"

**M key doesn't work**:
- Verify input binding is set up
- Check that MapWidget variable is valid
- Make sure Player Controller is using your Blueprint

**Sectors in wrong positions**:
- Ensure sector actors are placed on grid boundaries (every 200km)
- Check that sectors have unique positions

## Example Level Setup

Create a test level with sectors:

1. Create new level: `Maps/MapTest`
2. Add 9 `ASpaceSectorMap` actors in a 3x3 grid:
   - Sector (0,0): Position (0, 0, 0)
   - Sector (1,0): Position (20000000, 0, 0)  [200km east]
   - Sector (0,1): Position (0, 20000000, 0)  [200km north]
   - ... continue for 3x3 grid
3. Name each sector (e.g., "Alpha Sector", "Beta Sector")
4. Place player start in center sector
5. Test map - all 9 sectors should appear in grid

## Performance Tips

- Only update map when visible
- Cache sector positions (don't recalculate every frame)
- Use object pooling for sector icons if you have 100+ sectors
- Limit updates to 1-2 times per second max

---

**Need Help?** See full documentation in `Assets/MapSystemGuide.md`
