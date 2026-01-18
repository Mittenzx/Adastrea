# Docking Bay Module - Complete Setup Guide

**Module Type**: `ADockingBayModule`  
**Blueprint Class**: `BP_SpaceStationModule_DockingBay`  
**Power Consumption**: 50 units  
**Module Group**: Docking  
**Last Updated**: 2026-01-18

---

## Overview

The Docking Bay Module provides docking facilities for larger ships and freighters. It features multiple docking points for simultaneous ship docking, configurable capacity, and Blueprint-accessible docking state tracking.

**Key Features:**
- Multiple docking points (up to 20 ships simultaneously)
- Automatic docking point management
- Power consumption: 50 units
- Tractor beams and docking assistance systems
- Blueprint-accessible API for custom behavior

---

## Quick Start

### Add as Child Actor Component

1. Open your station Blueprint (`BP_SpaceStation`)
2. Go to **Components** panel
3. Click **+ Add** → Search for **Child Actor Component**
4. Rename to "DockingBay"
5. In Details panel:
   - **Child Actor Class**: `BP_SpaceStationModule_DockingBay`
6. Position in viewport relative to your station
7. Compile and save

> **Note:** This is the recommended method as it provides visual feedback at design-time. You can see and position your docking bay module in the editor viewport.

---

## Detailed Configuration

### Step 1: Create/Open the Docking Bay Blueprint

If `BP_SpaceStationModule_DockingBay` doesn't exist:

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search for and select **DockingBayModule** as parent
3. Name: `BP_SpaceStationModule_DockingBay`
4. Open the Blueprint

### Step 2: Add Docking Point Components

Docking points are **Scene Components** that define where ships attach.

**In Components Panel:**

1. Click **+ Add** → Search for **Scene Component**
2. Rename to `DockingPoint_1`
3. In Details panel, set **Transform**:
   - **Location**: (200, 0, 100) - relative to docking bay
   - **Rotation**: (0, 0, 0) - ship faces forward
   - **Scale**: (1, 1, 1)
4. **Repeat** for additional docking points:
   - `DockingPoint_2`: (200, 200, 100)
   - `DockingPoint_3`: (200, -200, 100)
   - `DockingPoint_4`: (400, 0, 100)

**Recommended Spacing:**
- **Minimum**: 200 units apart
- **Optimal**: 300-400 units apart
- **Ensure**: Clear approach path (no collision geometry blocking)

### Step 3: Add Interaction Trigger

This sphere detects when ships enter docking range.

1. Click **+ Add** → **Sphere Component**
2. Rename to `InteractionTrigger`
3. Configure in Details panel:
   - **Sphere Radius**: 1500.0 (detection range)
   - **Collision Preset**: "Overlap All Dynamic"
   - **Generate Overlap Events**: ✓ Enabled
   - **Hidden in Game**: ✓ Enabled (invisible during gameplay)
   - **Visible**: ✓ Enabled in editor (for debugging)

### Step 4: Link Docking Points to Array

1. Open **Class Defaults** (toolbar button)
2. Find **Docking → Docking Points** array
3. Click **+ Add Element** for each docking point
4. Select the Scene Component from dropdown:
   - [0] → `DockingPoint_1`
   - [1] → `DockingPoint_2`
   - [2] → `DockingPoint_3`
   - [3] → `DockingPoint_4`

**⚠️ Critical:** The array must be populated or ships cannot dock!

### Step 5: Configure Docking Capacity

In **Class Defaults → Docking**:

- **Max Docked Ships**: 4 (should equal number of docking points)
- **Module Power**: 50.0 (power consumption)
- **Module Type**: "Docking Bay" (auto-set)

---

## Component Hierarchy Example

```
BP_SpaceStationModule_DockingBay (Self)
├─ StaticMesh_DockingBay (Static Mesh)
│  └─ Mesh: SM_DockingBay_01
├─ InteractionTrigger (Sphere Component)
│  └─ Radius: 1500.0
├─ DockingPoint_1 (Scene Component)
│  └─ Location: (200, 0, 100)
├─ DockingPoint_2 (Scene Component)
│  └─ Location: (200, 200, 100)
├─ DockingPoint_3 (Scene Component)
│  └─ Location: (200, -200, 100)
└─ DockingPoint_4 (Scene Component)
   └─ Location: (400, 0, 100)
```

---

## Properties Reference

### Docking Points (Array)

**Type**: `TArray<USceneComponent*>`  
**Access**: EditAnywhere, BlueprintReadOnly  
**Category**: Docking

Array of Scene Components marking ship attachment locations. Each point represents where a ship can dock.

**Important**: Array size should be ≥ MaxDockedShips

### Max Docked Ships

**Type**: `int32`  
**Default**: 4  
**Range**: 1-20  
**Access**: EditAnywhere, BlueprintReadOnly  
**Category**: Docking

Maximum number of ships that can dock simultaneously. Ensure DockingPoints array has at least this many elements.

### Current Docked Ships

**Type**: `int32`  
**Default**: 0  
**Access**: VisibleAnywhere, BlueprintReadOnly  
**Category**: Docking

Current number of docked ships. Updated automatically when ships dock/undock.

---

## Blueprint Functions

### GetAvailableDockingSpots()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `int32` - Number of free docking points

Get the number of available docking spots.

```
Available Spots = MaxDockedShips - CurrentDockedShips
```

### HasAvailableDocking()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `bool` - True if ships can dock

Check if the docking bay has available capacity.

### GetAvailableDockingPoint()

**Type**: BlueprintCallable  
**Returns**: `USceneComponent*` - Docking point or nullptr

Get the first available docking point. Returns nullptr if all points are occupied.

### GetDockingPoints()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `TArray<USceneComponent*>` - All docking points

Get all docking point components.

### DockShip()

**Type**: BlueprintCallable  
**Returns**: `bool` - True if successful

Dock a ship at this module. Increments CurrentDockedShips if capacity available.

### UndockShip()

**Type**: BlueprintCallable  
**Returns**: `bool` - True if successful

Undock a ship from this module. Decrements CurrentDockedShips if any ships are docked.

---

## Viewing Module Information in Station Blueprint

**NEW**: When you add a docking bay module to a station, you can see its information directly in the station Blueprint!

### Station-Level Functions

The station Blueprint provides functions to query docking bay information:

#### GetTotalDockingPoints()

**Type**: BlueprintCallable, BlueprintPure  
**Category**: Station|Docking  
**Returns**: `int32` - Total number of docking points across all docking bays

Shows the sum of all docking points from all docking bay modules attached to the station.

**Example Usage:**
```
In BP_SpaceStation Class Defaults or Event Graph:
  Get Total Docking Points → Print String
  
Output: "This station has 8 docking points"
```

#### GetTotalDockingCapacity()

**Type**: BlueprintCallable, BlueprintPure  
**Category**: Station|Docking  
**Returns**: `int32` - Total maximum simultaneous docking capacity

Shows the sum of MaxDockedShips from all docking bay modules.

**Example Usage:**
```
In BP_SpaceStation:
  Get Total Docking Capacity → Print String
  
Output: "This station can dock 8 ships simultaneously"
```

#### GetDockingBayModule()

**Type**: BlueprintCallable  
**Category**: Station|Docking  
**Returns**: `ADockingBayModule*` - First docking bay module or nullptr

Gets the first docking bay module attached to the station.

#### GetDockingBayModules()

**Type**: BlueprintCallable  
**Category**: Station|Docking  
**Returns**: `TArray<ADockingBayModule*>` - All docking bay modules

Gets all docking bay modules attached to the station.

### How to View in Editor

**Method 1: Event Graph (Design-Time Preview)**

1. Open your station Blueprint (`BP_SpaceStation`)
2. Go to **Event Graph**
3. Add node: **Event Construction Script**
4. Call **Get Total Docking Points**
5. Connect to **Print String**
6. When you add/remove docking bay modules in the Components panel, the print will update in viewport

**Method 2: Class Defaults (Runtime Value)**

The station's `Modules` array in Class Defaults will show all attached modules at runtime. You can iterate through them to see docking bays.

**Method 3: Custom Display Variable**

Create a display-only variable in your station Blueprint:

```
In BP_SpaceStation Class Defaults:

1. Add variable: "TotalDockingPointsDisplay" (int32)
2. Set to: VisibleAnywhere, BlueprintReadOnly
3. Category: "Station Info"

In Event Construction Script:
  Get Total Docking Points → Set TotalDockingPointsDisplay

Now you'll see the total in the Details panel!
```

### Example Blueprint Setup

```
Event Construction Script
    ↓
Get Total Docking Points (Self) → Set TotalDockingPointsDisplay
    ↓
Get Total Docking Capacity (Self) → Set TotalDockingCapacityDisplay
    ↓
Get Module Count By Group (Docking) → Set DockingBayCountDisplay
```

Result in Details panel:
```
Station Info:
  Total Docking Points Display: 8
  Total Docking Capacity Display: 8
  Docking Bay Count Display: 2
```

---

## Docking Point Positioning Guide

### Small Station (2 points)

```
DockingPoint_1: (300, 0, 50)
DockingPoint_2: (300, 0, -50)
```

**Use Case**: Outpost, small trading post  
**Max Docked Ships**: 2

### Medium Station (4 points)

```
DockingPoint_1: (200, 200, 100)
DockingPoint_2: (200, -200, 100)
DockingPoint_3: (400, 200, 100)
DockingPoint_4: (400, -200, 100)
```

**Use Case**: Standard trading station  
**Max Docked Ships**: 4

### Large Station (6 points)

```
DockingPoint_1: (200, 0, 150)
DockingPoint_2: (200, 300, 0)
DockingPoint_3: (200, -300, 0)
DockingPoint_4: (400, 0, 150)
DockingPoint_5: (400, 300, 0)
DockingPoint_6: (400, -300, 0)
```

**Use Case**: Major hub, capital station  
**Max Docked Ships**: 6

### Positioning Best Practices

- **Spacing**: 200-400 units apart minimum
- **Height**: Usually same Z value for consistency
- **Clearance**: Ensure 100+ unit radius around each point
- **Symmetry**: Mirror points for visual appeal
- **Access**: Face points toward station entrance/exterior

---

## Adding Visual Feedback (Optional)

### Static Mesh Docking Arms

1. Add **Static Mesh Component** to each docking point
2. Name: `DockingArm_1` (attach to `DockingPoint_1`)
3. Set Mesh: `SM_DockingArm` (your mesh asset)
4. Set Material: `M_DockingArm_Inactive`

### Point Lights

1. Add **Point Light Component** to each docking point
2. Name: `DockingLight_1`
3. Configure:
   - **Light Color**: Green (available), Red (occupied)
   - **Intensity**: 5000.0
   - **Attenuation Radius**: 200.0

### Particle Effects

1. Add **Particle System Component** to each docking point
2. Name: `DockingFX_1`
3. Configure:
   - **Template**: `P_DockingMagneticLock`
   - **Auto Activate**: False (activate when ship docks)

---

## Testing Checklist

### Pre-Test Setup

- [ ] Docking Bay added to station (Child Actor Component or DefaultModuleClasses)
- [ ] 4-6 Scene Components created as docking points
- [ ] Each Scene Component positioned (200-400 units apart)
- [ ] DockingPoints array populated with Scene Component references
- [ ] InteractionTrigger sphere radius = 1500.0
- [ ] InteractionTrigger collision = "Overlap All Dynamic"
- [ ] Generate Overlap Events enabled
- [ ] All Blueprints compiled and saved

### In-Game Testing

1. **Place Station in Level**
   - Drag `BP_SpaceStation` into test level
   - Position away from player start

2. **Test Docking Range Detection**
   - Play in editor
   - Fly toward station
   - At ~1500 units: Docking prompt should appear
   - Prompt shows: "Press F to Dock at [Station Name]"

3. **Test Docking Sequence**
   - Press F key
   - Ship smoothly moves to nearest docking point (3 seconds)
   - Trading UI opens automatically

4. **Test Undocking**
   - Click "Undock" button in trading UI
   - Ship detaches and moves slightly away
   - Flight controls restored

5. **Test Multiple Ships** (if applicable)
   - Second ship should dock at different point
   - Both ships can trade independently

---

## Common Issues & Solutions

### Issue: No Docking Prompt Appears

**Likely Cause**: InteractionTrigger not configured  
**Solution**:
- Verify sphere collision settings
- Check "Generate Overlap Events" enabled
- Increase sphere radius to 2000

### Issue: Ship Doesn't Move to Docking Point

**Likely Cause**: DockingPoints array empty  
**Solution**:
- Open Class Defaults
- Populate DockingPoints array with Scene Component references
- Ensure array has at least 1 entry

### Issue: Ship Docks at Wrong Location

**Likely Cause**: Scene Component position incorrect  
**Solution**:
- Select docking point component
- Adjust Transform → Location in Details panel
- Verify position in viewport

### Issue: Multiple Ships Use Same Point

**Likely Cause**: DockingPoints array has duplicates  
**Solution**:
- Open DockingPoints array
- Ensure each entry is unique
- Remove duplicate references

### Issue: Ship Falls Through Station

**Likely Cause**: Docking point inside collision geometry  
**Solution**:
- Move docking point outside station mesh
- Ensure clear space around docking point
- Check collision settings on station mesh

---

## Blueprint Examples

### Check Docking Availability (Blueprint)

```
Event: OnPlayerRequestsDocking
    ↓
Get Station Reference
    ↓
Get Docking Bay Module
    ↓
Branch: Has Available Docking?
    ├─ True: Proceed with docking
    └─ False: Show "Docking bay full" message
```

### Get Nearest Docking Point (Blueprint)

```
Function: FindNearestDockingPoint
Input: Ship Location (Vector)
    ↓
Get Docking Bay Module
    ↓
Get Docking Points (Array)
    ↓
For Each: Docking Point
    ↓
    Calculate Distance to Ship
    ↓
    Track Minimum Distance
    ↓
Return: Nearest Docking Point
```

---

## Integration with Station

### Station Setup

Your station needs:
1. **DockingBay Module** (this module)
2. **Marketplace Module** (for trading)
3. **Static Mesh** (visual station structure)

### Complete Station Example

```
BP_TradingStation_Agricultural
├─ StaticMesh_StationCore (Static Mesh)
├─ DockingBay (Child Actor Component)
│  └─ Child Actor Class: BP_SpaceStationModule_DockingBay
├─ Marketplace (Child Actor Component)
│  └─ Child Actor Class: BP_SpaceStationModule_Marketplace
└─ DetectionRange (Sphere Component)
```

---

## Performance Considerations

- **Docking Points**: 4-6 recommended, max 20
- **Update Frequency**: Docking state checked only when ships nearby
- **Power Consumption**: 50 units (ensure station has power)
- **Collision**: Use simple sphere for interaction trigger
- **Visual Effects**: Enable only when ships actively docking

---

## Advanced Topics

### Custom Docking Logic

Override docking behavior in Blueprint:

1. Create Blueprint based on `BP_SpaceStationModule_DockingBay`
2. Override `DockShip` function
3. Add custom validation logic
4. Call parent implementation
5. Add custom visual effects

### Dynamic Docking Points

Add/remove docking points at runtime:

1. Spawn Scene Component dynamically
2. Set transform (position, rotation)
3. Add to DockingPoints array
4. Increment MaxDockedShips

---

## Related Documentation

- [Complete Trading and Docking Guide](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)
- [Marketplace Module Guide](MARKETPLACE_MODULE.md)
- [Station Editor Guide](../systems/STATION_EDITOR_COMPLETE_GUIDE.md)
- [Module Configuration Guide](../reference/SPACESTATION_MODULE_CONFIG_GUIDE.md)

---

## Version History

**v1.0** (2026-01-18)
- Initial documentation
- Comprehensive setup guide
- Blueprint examples
- Troubleshooting section

---

**Maintained by**: Adastrea Development Team  
**For Support**: See main project README.md
