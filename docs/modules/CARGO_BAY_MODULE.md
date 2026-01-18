# Cargo Bay Module - Complete Setup Guide

**Module Type**: `ACargoBayModule`  
**Blueprint Class**: `BP_CargoBayModule`  
**Power Consumption**: 5 units  
**Module Group**: Storage  
**Last Updated**: 2026-01-18

---

## Overview

The Cargo Bay Module provides general-purpose storage facilities for goods and materials. It features automated inventory management, climate control, and secure storage systems.

**Key Features:**
- Low power consumption (5 units)
- Automated inventory management
- Climate-controlled storage
- Secure access controls
- Scalable storage capacity

---

## Quick Start

### Add as Child Actor Component

1. Open your station Blueprint (`BP_SpaceStation`)
2. Go to **Components** panel
3. Click **+ Add** → Search for **Child Actor Component**
4. Rename to "CargoBay"
5. In Details panel:
   - **Child Actor Class**: `BP_CargoBayModule`
6. Position in viewport relative to your station
7. Compile and save

> **Note:** This is the recommended method as it provides visual feedback at design-time. You can see and position your cargo bay module in the editor viewport.

---

## Detailed Configuration

### Step 1: Create/Open the Cargo Bay Blueprint

If `BP_CargoBayModule` doesn't exist:

1. **Content Browser** → Right-Click → **Blueprint Class**
2. Search for and select **CargoBayModule** as parent
3. Name: `BP_CargoBayModule`
4. Open the Blueprint

### Step 2: Add Visual Components (Optional)

**In Components Panel:**

1. **Static Mesh Component** (if not present)
   - Click **+ Add** → **Static Mesh Component**
   - Rename to `StaticMesh_CargoBay`
   - Set Mesh: `SM_CargoBay_01` (your mesh asset)
   - Set Material: `M_CargoBay_Metal`

2. **Point Light** (optional, for interior lighting)
   - Click **+ Add** → **Point Light Component**
   - Rename to `InteriorLight`
   - Configure:
     - **Light Color**: Warm white
     - **Intensity**: 3000.0
     - **Attenuation Radius**: 500.0

3. **Particle System** (optional, for environmental effects)
   - Click **+ Add** → **Particle System Component**
   - Rename to `EnvironmentalFX`
   - Set Template: `P_ClimateControl_Mist`
   - Auto Activate: True

### Step 3: Configure Class Defaults

In **Class Defaults → Module**:

- **Module Type**: "Cargo Bay" (auto-set)
- **Module Power**: 5.0 (low power consumption)
- **Module Group**: Storage

---

## Component Hierarchy Example

```
BP_CargoBayModule (Self)
├─ StaticMesh_CargoBay (Static Mesh)
│  └─ Mesh: SM_CargoBay_01
├─ InteriorLight (Point Light, optional)
│  └─ Intensity: 3000.0
└─ EnvironmentalFX (Particle System, optional)
   └─ Template: P_ClimateControl_Mist
```

---

## Properties Reference

### Module Type

**Type**: `FString`  
**Default**: "Cargo Bay"  
**Access**: EditAnywhere, BlueprintReadOnly  
**Category**: Module

Identifier for this module type. Automatically set by the C++ constructor.

### Module Power

**Type**: `float`  
**Default**: 5.0  
**Access**: EditAnywhere, BlueprintReadOnly  
**Category**: Module

Power consumption in units. Cargo bays are low-power modules (5 units).

### Module Group

**Type**: `EStationModuleGroup`  
**Default**: Storage  
**Access**: EditAnywhere, BlueprintReadOnly  
**Category**: Module

Functional group for organization and filtering. Cargo bays belong to the Storage group.

---

## Usage Patterns

### Single Cargo Bay

**Use Case**: Small outpost, minimal storage needs  
**Configuration**: One cargo bay module

```
BP_SmallStation
├─ DockingBay (Child Actor)
├─ Marketplace (Child Actor)
└─ CargoBay (Child Actor)
```

### Multiple Cargo Bays

**Use Case**: Major trading hub, high volume storage  
**Configuration**: Multiple cargo bay modules

```
BP_TradingHub
├─ DockingBay (Child Actor)
├─ Marketplace (Child Actor)
├─ CargoBay_1 (Child Actor)
├─ CargoBay_2 (Child Actor)
└─ CargoBay_3 (Child Actor)
```

### Specialized Storage

**Use Case**: Differentiated storage areas  
**Configuration**: Multiple cargo bays with different purposes

```
BP_IndustrialStation
├─ CargoBay_General (Child Actor)
│  └─ Name: "General Storage"
├─ CargoBay_Hazmat (Child Actor)
│  └─ Name: "Hazardous Materials"
└─ CargoBay_Refrigerated (Child Actor)
   └─ Name: "Perishable Goods"
```

---

## Storage Capacity

The Cargo Bay Module provides storage space for the station. The actual storage capacity is managed by the station's trading and inventory systems.

**Note**: For MVP, the cargo bay is primarily a visual and logical component. Advanced inventory management (station-specific storage) is a post-MVP feature.

### MVP Behavior

- Cargo bays exist as modules on the station
- Provide visual representation of storage facilities
- Consume power (5 units each)
- Can be queried and counted programmatically

### Post-MVP Features (Future)

- Station-specific cargo storage
- Cargo bay capacity limits
- Cargo transfer between bays
- Specialized storage types (refrigerated, hazmat, etc.)
- Inventory tracking per cargo bay

---

## Blueprint Functions (Inherited)

Cargo Bay modules inherit standard module functions:

### GetModuleType()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `FString` - Module type ("Cargo Bay")

Get the type identifier for this module.

### GetModulePower()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `float` - Power consumption (5.0)

Get the power consumption of this module.

### GetModuleGroup()

**Type**: BlueprintCallable, BlueprintPure  
**Returns**: `EStationModuleGroup` - Module group (Storage)

Get the functional group this module belongs to.

---

## Testing Checklist

### Pre-Test Setup

- [ ] Cargo Bay Blueprint created or exists
- [ ] Visual components added (mesh, lights, optional)
- [ ] Module properties configured (type, power, group)
- [ ] Added to station (Child Actor Component)
- [ ] All Blueprints compiled and saved

### In-Game Testing

1. **Place Station in Level**
   - Drag station Blueprint into test level
   - Press Play

2. **Verify Module Exists**
   - Check station has cargo bay module
   - Use: `Station->GetModulesByType("Cargo Bay")`

3. **Verify Visual Representation**
   - Cargo bay visible on station
   - Mesh renders correctly
   - Lights functional (if added)

4. **Verify Power Consumption**
   - Check station power balance
   - Each cargo bay consumes 5 units
   - Use: `Station->GetTotalPowerConsumption()`

---

## Common Issues & Solutions

### Issue: Cargo Bay Not Visible

**Likely Cause**: Static mesh not assigned  
**Solution**:
- Open cargo bay Blueprint
- Select StaticMesh component
- Assign a mesh asset in Details panel
- Compile and save

### Issue: High Power Consumption

**Likely Cause**: Multiple cargo bays or wrong power value  
**Solution**:
- Count cargo bay modules on station
- Each consumes 5 units (correct)
- Add power generation modules if needed
- Check Module Power property = 5.0

### Issue: Module Not Found by Station

**Likely Cause**: Not added to station properly  
**Solution**:
- Verify added as Child Actor Component
- Check station's Modules array in Details (runtime)
- Ensure cargo bay Blueprint is valid
- Recompile station Blueprint

---

## Blueprint Examples

### Count Cargo Bays (Blueprint)

```
Function: GetCargoBayCount
    ↓
Get All Modules (Station)
    ↓
For Each: Module
    ↓
    Branch: GetModuleType() == "Cargo Bay"
        ├─ True: Increment Counter
        └─ False: Continue
    ↓
Return: Count (int32)
```

### Calculate Total Storage Capacity (Post-MVP)

```
Function: GetTotalStorageCapacity
    ↓
Get All Modules By Type: "Cargo Bay"
    ↓
For Each: Cargo Bay
    ↓
    Get Bay Capacity
    ↓
    Add to Total
    ↓
Return: Total Capacity (float)
```

---

## Integration with Station

### Typical Trading Station

```
BP_TradingStation
├─ StaticMesh_StationCore (Static Mesh)
├─ DockingBay (Child Actor Component)
│  └─ BP_SpaceStationModule_DockingBay
├─ Marketplace (Child Actor Component)
│  └─ BP_SpaceStationModule_Marketplace
├─ CargoBay (Child Actor Component)
│  └─ BP_CargoBayModule
└─ DetectionRange (Sphere Component)
```

**Purpose of Each Module:**
- **DockingBay**: Ships dock here
- **Marketplace**: Players trade goods
- **CargoBay**: Storage facility (visual/logical)

### Industrial Station (Heavy Storage)

```
BP_IndustrialStation
├─ DockingBay_Large (Child Actor)
├─ Marketplace (Child Actor)
├─ CargoBay_1 (Child Actor)
├─ CargoBay_2 (Child Actor)
├─ CargoBay_3 (Child Actor)
├─ CargoBay_4 (Child Actor)
└─ ReactorModule (Child Actor)
```

**Power Calculation:**
- 4 cargo bays × 5 units = 20 units
- 1 docking bay × 50 units = 50 units
- 1 marketplace × 40 units = 40 units
- **Total**: 110 units (reactor must provide sufficient power)

---

## Positioning Guidelines

### Single Cargo Bay

Position near the station's core or attached to the marketplace:

```
Location: (0, 300, 0) - Side of station
Rotation: (0, 0, 0)
Scale: (1, 1, 1)
```

### Multiple Cargo Bays

Arrange symmetrically for visual appeal:

```
CargoBay_1: (0, 300, 100)   # Right side, upper
CargoBay_2: (0, 300, -100)  # Right side, lower
CargoBay_3: (0, -300, 100)  # Left side, upper
CargoBay_4: (0, -300, -100) # Left side, lower
```

---

## Visual Customization

### Mesh Variants

Create different visual styles for cargo bays:

**Standard Cargo Bay:**
- Mesh: `SM_CargoBay_Standard`
- Material: `M_CargoBay_Metal`
- Use: General purpose storage

**Refrigerated Cargo Bay:**
- Mesh: `SM_CargoBay_Refrigerated`
- Material: `M_CargoBay_Insulated`
- Particle: `P_ColdMist`
- Use: Perishable goods

**Hazmat Cargo Bay:**
- Mesh: `SM_CargoBay_Hazmat`
- Material: `M_CargoBay_Hazmat_Yellow`
- Light: Red warning lights
- Use: Dangerous materials

### Material Variations

Different materials for different station types:

**Clean/New:**
- `M_CargoBay_Clean` - Pristine, well-maintained

**Industrial:**
- `M_CargoBay_Industrial` - Weathered, used

**Luxury:**
- `M_CargoBay_Luxury` - Polished, decorative

---

## Performance Considerations

- **Power**: 5 units each (very low)
- **Poly Count**: Use LODs for distant cargo bays
- **Collision**: Simple box collision recommended
- **Updates**: Static (no per-frame updates)
- **Scalability**: Can have 10+ cargo bays with minimal performance impact

---

## Advanced Topics

### Custom Cargo Bay Types (Post-MVP)

Create specialized cargo bay variants:

1. Create Blueprint based on `BP_CargoBayModule`
2. Name: `BP_CargoBay_Refrigerated`
3. Override Module Type: "Refrigerated Cargo Bay"
4. Add custom properties:
   - Temperature: -20°C
   - Capacity: 5000 units
   - Suitable for: Perishable goods

### Dynamic Cargo Management (Post-MVP)

Add inventory tracking to cargo bays:

```cpp
// Custom properties in Blueprint
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float StorageCapacity = 10000.0f;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
float CurrentLoad = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<FCargoItem> StoredItems;
```

---

## Related Documentation

- [Complete Trading and Docking Guide](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)
- [Docking Bay Module Guide](DOCKING_BAY_MODULE.md)
- [Marketplace Module Guide](MARKETPLACE_MODULE.md)
- [Module Configuration Guide](../reference/SPACESTATION_MODULE_CONFIG_GUIDE.md)

---

## Version History

**v1.0** (2026-01-18)
- Initial documentation
- Basic setup guide
- Integration examples
- Visual customization guide

---

**Maintained by**: Adastrea Development Team  
**For Support**: See main project README.md
