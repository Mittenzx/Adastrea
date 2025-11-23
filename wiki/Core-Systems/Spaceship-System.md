# Spaceship System

The Spaceship System is the foundation of Adastrea's vehicle mechanics, providing a comprehensive framework for creating and managing spaceships with detailed statistics and customization.

## Overview

**Location**: `Source/Adastrea/Ships/`

**Key Classes**:
- `ASpaceship` - Base ship actor with FloatingPawnMovement
- `USpaceshipDataAsset` - Complete ship specification data asset
- `USpaceshipComponent` - Component for ship functionality

**Documentation**: See [Assets/SpaceshipDataAssetGuide.md](../../Assets/SpaceshipDataAssetGuide.md) for complete guide

## Quick Start

### Creating a Ship in 5 Minutes

1. **Create Data Asset**:
   - Open Content Browser
   - Navigate to `Content/DataAssets/Ships/`
   - Right-click → Blueprint Class → Search "SpaceshipDataAsset"
   - Name it `DA_Ship_MyShip`

2. **Configure Basic Info**:
   - Display Name: "Explorer Scout"
   - Description: "Fast reconnaissance vessel"
   - Manufacturer: "Stellar Dynamics"

3. **Set Core Stats**:
   - Max Hull: 100
   - Max Cargo: 50
   - Crew Min: 1
   - Crew Max: 4

4. **Set Mobility**:
   - Max Speed: 800
   - Acceleration: 400
   - Turn Rate: 120
   - Jump Range: 5

5. **Save and Test**: Assign to a ship Blueprint

## Ship Stat Categories

The system organizes ship properties into 7 categories:

### 1. Core Stats (Fundamentals)
- Hull Strength
- Cargo Capacity
- Crew Requirements
- Modular Points (for upgrades)

### 2. Combat Stats
- Armor Rating
- Shield Capacity
- Weapon Hardpoints
- Point Defense Systems

### 3. Mobility Stats
- Max Speed
- Acceleration
- Maneuverability
- Jump Range
- Fuel Capacity

### 4. Utility Stats
- Sensor Range
- Stealth Rating
- Repair Systems
- Science Capabilities
- Medical Facilities

### 5. Operational Stats
- Power Generation
- Life Support
- Maintenance Requirements
- Hangar Bays
- Drone Capacity

### 6. Advanced Stats
- Diplomatic Facilities
- Boarding Capabilities
- Customization Slots
- Electronic Warfare
- Mining Equipment

### 7. Lore/Flavor
- Manufacturer
- Year Built
- Rarity Tier
- Color Scheme
- Lore Notes

## Ship Templates

Pre-built templates available in `Assets/SpaceshipTemplates/`:

| Ship Type | Role | Focus |
|-----------|------|-------|
| **Pathfinder Scout** | Exploration | Speed, sensors, jump range |
| **Warhammer Gunship** | Combat | Weapons, armor, firepower |
| **Merchant Trader** | Trading | Cargo capacity, efficiency |
| **Starliner Luxury** | Diplomacy | Passenger capacity, diplomacy |
| **Excavator Mining** | Industrial | Mining equipment, storage |
| **Sovereign Command** | Capital | Command, carrier operations |

## Blueprint Functions

All stats are accessible via Blueprint-callable functions:

### Rating Calculations
```
GetCombatRating() → float (0-100)
GetMobilityRating() → float (0-100)
GetUtilityRating() → float (0-100)
GetOverallRating() → float (0-100)
```

### Role Evaluation
```
IsSuitableForRole(Role) → bool
GetSizeCategory() → enum (Small/Medium/Large/Capital)
GetOperationalCost() → float (daily maintenance)
```

### Stat Access
```
GetMaxHull() → float
GetMaxSpeed() → float
GetCargoCapacity() → float
GetWeaponHardpoints() → int
... (all stats accessible)
```

## Ship Creation Workflow

### For Designers

1. **Choose a Template**: Select from 6 pre-built types
2. **Copy YAML**: Use as reference for stat values
3. **Create Data Asset**: Make SpaceshipDataAsset in editor
4. **Fill Properties**: Enter stats from template
5. **Customize**: Adjust values for your design
6. **Test**: Assign to Blueprint and play

### For Programmers

1. **Review C++ Class**: `Source/Adastrea/Ships/SpaceshipDataAsset.h`
2. **Understand Stats**: See stat category organization
3. **Extend**: Add custom properties via inheritance
4. **Expose**: Mark new properties `UPROPERTY(BlueprintReadOnly)`
5. **Functions**: Add helper functions as needed

## Integration with Other Systems

The Spaceship System integrates with:

- **Personnel System**: Crew assignments and skills
- **Combat System**: Weapons and damage calculations
- **Trading System**: Cargo and economy
- **Faction System**: Ship ownership and relations
- **Navigation System**: Movement and autopilot
- **Customization System**: Upgrades and modifications

## YAML Templates

Located in `Assets/SpaceshipTemplates/`:

```yaml
# Example: Fast Scout
ship:
  basic_info:
    display_name: "Pathfinder Scout"
    description: "Fast reconnaissance vessel"
    manufacturer: "Frontier Explorers"
  
  core_stats:
    max_hull: 80
    max_cargo: 30
    crew_min: 1
    crew_max: 3
  
  mobility_stats:
    max_speed: 1000
    acceleration: 500
    turn_rate: 150
    jump_range: 8
```

See [Assets/SpaceshipTemplates.md](../../Assets/SpaceshipTemplates.md) for all templates.

## Advanced Topics

### Ship Ratings

Rating functions calculate effectiveness scores (0-100):

- **Combat Rating**: Based on weapons, armor, shields
- **Mobility Rating**: Based on speed, acceleration, turn rate
- **Utility Rating**: Based on sensors, repair, special systems

Use ratings for:
- AI ship selection
- Mission requirements
- Difficulty scaling
- Player recommendations

### Size Categories

Ships are classified by tonnage:
- **Small** (< 500 tons): Fighters, shuttles
- **Medium** (500-2000 tons): Corvettes, frigates
- **Large** (2000-10000 tons): Destroyers, cruisers
- **Capital** (> 10000 tons): Battleships, carriers

### Customization System

Ships support modular upgrades:
- Weapon hardpoints
- Shield generators
- Engine modules
- Utility systems
- Cargo expansions

See [Assets/ShipCustomizationGuide.md](../../Assets/ShipCustomizationGuide.md)

## Common Use Cases

### "I want to create a fast scout ship"
- High max speed (800-1200)
- High acceleration (400-600)
- High turn rate (120-180)
- High sensor range (5000-10000)
- Low to medium hull (60-100)
- Small crew (1-3)

### "I want to create a combat ship"
- Medium to high hull (120-200)
- Multiple weapon hardpoints (4-8)
- High armor rating (50-100)
- Good shields (100-200)
- Medium speed (500-800)
- Medium to large crew (4-10)

### "I want to create a trading vessel"
- High cargo capacity (200-500)
- Low to medium combat (basic defense)
- Medium speed (500-700)
- Good fuel capacity
- Medium crew (3-8)
- Economic efficiency

## Documentation Resources

**Complete Guides** (in repository):
- [SpaceshipDataAssetGuide.md](../../Assets/SpaceshipDataAssetGuide.md) - Complete system guide
- [SpaceshipDesignerWorkflow.md](../../Assets/SpaceshipDesignerWorkflow.md) - Step-by-step workflow
- [SpaceshipTemplates.md](../../Assets/SpaceshipTemplates.md) - Template reference
- [SpaceshipInteriorGuide.md](../../Assets/SpaceshipInteriorGuide.md) - Interior layouts
- [ShipControlGuide.md](../../Assets/ShipControlGuide.md) - Control systems

**YAML Templates**:
- `Assets/SpaceshipTemplates/*.yaml` - 6 ship configurations

**Example Blueprints**:
- `Content/Blueprints/Ships/BP_Spaceship` - Base ship Blueprint
- `Content/DataAssets/Ships/DA_*` - Example ship Data Assets

## Getting Help

- **System Guide**: [Assets/SpaceshipDataAssetGuide.md](../../Assets/SpaceshipDataAssetGuide.md)
- **Templates**: [Assets/SpaceshipTemplates.md](../../Assets/SpaceshipTemplates.md)
- **Workflow**: [Assets/SpaceshipDesignerWorkflow.md](../../Assets/SpaceshipDesignerWorkflow.md)
- **Questions**: [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)

---

**The Spaceship System is fully implemented and production-ready!**

*For complete technical documentation, see Assets/SpaceshipDataAssetGuide.md*
