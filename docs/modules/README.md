# Space Station Modules - Documentation Index

**Last Updated**: 2026-01-18  
**Purpose**: Index of all space station module documentation

---

## Quick Navigation

| Module | Power | Group | Status | Guide |
|--------|-------|-------|--------|-------|
| **Docking Bay** | 50 units | Docking | ✅ MVP | [Guide](DOCKING_BAY_MODULE.md) |
| **Marketplace** | 40 units | Public | ✅ MVP | [Guide](MARKETPLACE_MODULE.md) |
| **Cargo Bay** | 5 units | Storage | ✅ MVP | [Guide](CARGO_BAY_MODULE.md) |
| **Reactor** | -100 units | Power | 🔄 Post-MVP | Coming soon |
| **Solar Array** | -20 units | Power | 🔄 Post-MVP | Coming soon |
| **Shield Generator** | 75 units | Defense | 🔄 Post-MVP | Coming soon |
| **Turret** | 30 units | Defense | 🔄 Post-MVP | Coming soon |
| **Habitation** | 15 units | Living | 🔄 Post-MVP | Coming soon |
| **Science Lab** | 25 units | Research | 🔄 Post-MVP | Coming soon |
| **Fabrication** | 60 units | Industry | 🔄 Post-MVP | Coming soon |
| **Fuel Depot** | 10 units | Storage | 🔄 Post-MVP | Coming soon |

Legend:
- ✅ MVP - Documentation complete, ready for use
- 🔄 Post-MVP - Coming in future releases
- ⚠️ WIP - Work in progress

---

## MVP Modules (Available Now)

### Docking Bay Module

**Purpose**: Ship docking facilities  
**Power**: 50 units (consumption)  
**Key Features**:
- Multiple docking points (up to 20)
- Automatic docking point management
- Interaction trigger for ships
- Blueprint-accessible API

**[📖 Read Full Guide](DOCKING_BAY_MODULE.md)**

---

### Marketplace Module

**Purpose**: Trading and commerce  
**Power**: 40 units (consumption)  
**Key Features**:
- Configurable market data assets
- Dynamic pricing support
- Can be opened/closed
- Multiple marketplaces per station

**[📖 Read Full Guide](MARKETPLACE_MODULE.md)**

---

### Cargo Bay Module

**Purpose**: General storage  
**Power**: 5 units (consumption)  
**Key Features**:
- Low power consumption
- Automated inventory management
- Climate control
- Scalable capacity

**[📖 Read Full Guide](CARGO_BAY_MODULE.md)**

---

## Post-MVP Modules (Coming Soon)

### Power Generation

**Reactor Module**
- Power: -100 units (generation)
- Purpose: Primary power generation
- Group: Power

**Solar Array Module**
- Power: -20 units (generation)
- Purpose: Renewable energy
- Group: Power

### Defense

**Shield Generator Module**
- Power: 75 units (consumption)
- Purpose: Station defense shields
- Group: Defense

**Turret Module**
- Power: 30 units (consumption)
- Purpose: Weapon emplacements
- Group: Defense

### Living & Support

**Habitation Module**
- Power: 15 units (consumption)
- Purpose: Crew quarters
- Group: Living

**Barracks Module**
- Power: 20 units (consumption)
- Purpose: Military personnel housing
- Group: Military

### Research & Industry

**Science Lab Module**
- Power: 25 units (consumption)
- Purpose: Research and development
- Group: Research

**Fabrication Module**
- Power: 60 units (consumption)
- Purpose: Manufacturing
- Group: Industry

**Processing Module**
- Power: 45 units (consumption)
- Purpose: Resource processing
- Group: Industry

### Storage & Logistics

**Fuel Depot Module**
- Power: 10 units (consumption)
- Purpose: Fuel storage and refueling
- Group: Storage

### Connectivity

**Corridor Module**
- Power: 2 units (consumption)
- Purpose: Module connections
- Group: Structure

**Docking Port Module**
- Power: 15 units (consumption)
- Purpose: Small ship docking
- Group: Docking

---

## Adding Modules to Stations

### Method 1: Child Actor Components (Recommended)

Best for visual design and positioning control.

1. Open station Blueprint
2. Components panel → **+ Add** → **Child Actor Component**
3. Set **Child Actor Class** to your module Blueprint
4. Position in viewport
5. Compile and save

**Advantages:**
- ✅ Visible in editor at design-time
- ✅ Visual positioning control
- ✅ See module layout before playing

### Module Configuration Workflow

Space station modules are configured using a single, component-based placement workflow in the station Blueprint.

Designers place and configure modules directly in the station Blueprint using Child Actor Components for visual, design-time editing.

**See**: [Module Configuration Guide](../reference/SPACESTATION_MODULE_CONFIG_GUIDE.md)

---

## Module Properties Reference

### Common Properties (All Modules)

| Property | Type | Description |
|----------|------|-------------|
| **Module Type** | String | Identifier (e.g., "Docking Bay") |
| **Module Power** | Float | Power consumption (+) or generation (-) |
| **Module Group** | Enum | Functional category |
| **Current Integrity** | Float | Module health |
| **Max Integrity** | Float | Maximum health |
| **Is Destroyed** | Bool | Destruction state |

### Module Groups

```cpp
enum class EStationModuleGroup : uint8
{
    Core,          // Essential systems
    Power,         // Power generation/distribution
    Docking,       // Docking facilities
    Storage,       // Cargo and storage
    Public,        // Public spaces (marketplace, etc.)
    Living,        // Habitation and life support
    Military,      // Defense and security
    Research,      // Science and research
    Industry,      // Manufacturing and processing
    Structure      // Structural elements
};
```

---

## Power Management

### Understanding Power Values

- **Positive values** = Power consumption (e.g., +50)
- **Negative values** = Power generation (e.g., -100)

### Example Station Power Balance

```
Power Generation:
  Reactor Module: -100 units
  Solar Array: -20 units
  Total Generation: -120 units

Power Consumption:
  Docking Bay: +50 units
  Marketplace: +40 units
  Cargo Bay (×3): +15 units (5 each)
  Shield Generator: +75 units
  Total Consumption: +180 units

Net Power: -120 + 180 = +60 units (deficit!)
```

**Solution**: Add another reactor or solar array, or reduce modules.

---

## Blueprint API Common Functions

All modules inherit these functions from `ASpaceStationModule`:

### GetModuleType()

**Returns**: `FString` - Module type identifier

```
Example: "Docking Bay", "Marketplace", "Reactor Core"
```

### GetModulePower()

**Returns**: `float` - Power consumption or generation

```
Positive = Consumes power
Negative = Generates power
```

### GetModuleGroup()

**Returns**: `EStationModuleGroup` - Functional group

```
Example: Docking, Storage, Power, Defense
```

### IsGeneratingPower()

**Returns**: `bool` - True if module generates power

```
True if Module Power < 0
```

---

## Related Documentation

### Core Guides

- [Complete Trading and Docking Guide](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md)
- [Station Editor Complete Guide](../systems/STATION_EDITOR_COMPLETE_GUIDE.md)
- [Module Configuration Guide](../reference/SPACESTATION_MODULE_CONFIG_GUIDE.md)

### Development

- [Trade Simulator MVP Instructions](../../.github/instructions/trade-simulator-mvp.instructions.md)
- [Blueprint Documentation Standards](../../.github/instructions/blueprint-documentation.instructions.md)

### Source Code

- `Source/Adastrea/Public/Stations/SpaceStationModule.h` - Base module class
- `Source/Adastrea/Public/Stations/DockingBayModule.h` - Docking bay module
- `Source/Adastrea/Public/Stations/MarketplaceModule.h` - Marketplace module
- `Source/Adastrea/Public/Stations/CargoBayModule.h` - Cargo bay module

---

## Contributing

### Adding New Module Documentation

When creating documentation for new modules:

1. **Copy template** from existing module guide
2. **Fill in module-specific details**:
   - Overview and purpose
   - Quick start (both methods)
   - Detailed configuration
   - Component hierarchy
   - Properties reference
   - Blueprint functions
   - Testing checklist
   - Common issues
   - Integration examples
3. **Add to this index** with appropriate status
4. **Cross-reference** related documentation

### Documentation Standards

- Use markdown format
- Include code examples
- Provide visual diagrams where helpful
- Add troubleshooting section
- Link to related guides

---

## Version History

**v1.0** (2026-01-18)
- Initial module documentation index
- Complete guides for 3 MVP modules
- Structure for post-MVP modules

---

**Maintained by**: Adastrea Development Team  
**For Support**: See main project [README.md](../../README.md)
