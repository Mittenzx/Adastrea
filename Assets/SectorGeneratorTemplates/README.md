# Sector Generator Templates

This directory contains YAML templates for configuring the Sector Generator system. These templates serve as starting points for creating different types of sectors.

## Available Templates

### MiningSection.yaml
**Theme**: Resource extraction and industrial operations  
**Best For**: Mining gameplay, resource gathering, industrial zones  
**Contents**:
- Trading and mining stations
- Rich asteroid fields
- Security patrols
- Cargo transports
- Mining ships

**Density**: Normal (1.0)  
**Faction Focus**: Trading Guild, Mining Cooperative

---

### FrontierOutpost.yaml
**Theme**: Remote, sparsely populated frontier  
**Best For**: Exploration, survival gameplay, early-game areas  
**Contents**:
- Small independent outpost
- Scattered asteroids
- Space debris
- Scout ships
- Navigation beacons

**Density**: Sparse (0.5)  
**Faction Focus**: Independent settlers

---

### MilitaryBorder.yaml
**Theme**: Heavily defended military zone  
**Best For**: Combat gameplay, high-security areas, faction borders  
**Contents**:
- Military command stations
- Defensive platforms
- Heavy patrol presence
- Fighter squadrons
- Scanner arrays
- Minefield markers

**Density**: High (1.5)  
**Faction Focus**: Military faction

---

### TradeHub.yaml
**Theme**: Busy commercial sector  
**Best For**: Trading gameplay, economic hubs, mission variety  
**Contents**:
- Trading stations and marketplaces
- Warehouses
- Heavy merchant traffic
- Security escorts
- Refueling depots
- Docking beacons

**Density**: Above Normal (1.3)  
**Faction Focus**: Trading Guild

---

## How to Use These Templates

### Method 1: Direct Blueprint Creation

1. Open Unreal Editor
2. Create a new `USectorGeneratorConfig` Blueprint Data Asset
3. Open the YAML template in a text editor
4. Manually configure the Blueprint properties to match the YAML values
5. Save the Data Asset

### Method 2: YAML Import (If Supported)

If your project has YAML import tools:

1. Use `YAMLtoDataAsset.py` script to convert YAML to Data Asset
2. Import the generated asset into your project
3. Review and adjust as needed

### Method 3: Copy and Customize

1. Choose a template that closely matches your needs
2. Copy the YAML file and rename it
3. Modify the values to create your custom configuration
4. Create the corresponding Data Asset in Unreal

## Customization Guide

### Adjusting Object Counts

Modify `MinCount` and `MaxCount` for any object type:

```yaml
- ObjectType: Station
  MinCount: 1    # Change to adjust minimum spawned
  MaxCount: 3    # Change to adjust maximum spawned
```

### Changing Density

Modify the `DensityMultiplier`:

```yaml
DensityMultiplier: 1.0   # 1.0 = normal, 2.0 = double, 0.5 = half
```

### Adjusting Distribution

Change the `DistributionPattern`:

```yaml
DistributionPattern: Random      # Uniform distribution
DistributionPattern: Clustered   # Grouped together
DistributionPattern: Grid        # Regular grid
DistributionPattern: Orbital     # Ring pattern
DistributionPattern: Scattered   # Wide spread
```

### Setting Faction Ownership

Assign factions to objects:

```yaml
OwnerFaction: "DA_Faction_YourFaction"   # Replace with your faction Data Asset path
```

### Distance Constraints

Control how far objects spawn from center:

```yaml
MinDistanceFromCenter: 1000000.0   # 10km minimum
MaxDistanceFromCenter: 5000000.0   # 50km maximum
```

## Creating New Templates

To create a new template:

1. Copy an existing template that's closest to your needs
2. Rename the file to match your sector type
3. Update the `ConfigName` and `Description`
4. Choose an appropriate `NamingTheme`
5. Modify object definitions:
   - Add new object types
   - Remove unwanted object types
   - Adjust counts and distribution
   - Set faction ownership
6. Test the configuration in-editor
7. Document your template in this README

## Template Combination

You can combine elements from multiple templates:

**Example: Mining Frontier**
- Base: `FrontierOutpost.yaml`
- Add from `MiningSection.yaml`:
  - Asteroid fields
  - Mining ships
  - Small mining station

**Example: Contested Trade Route**
- Base: `TradeHub.yaml`
- Add from `MilitaryBorder.yaml`:
  - Defensive platforms
  - Military patrols
- Result: Defended trade hub

## Blueprint Class References

These templates reference Blueprint classes that you need to create or modify:

### Stations
- `BP_SpaceStation_Trading`
- `BP_SpaceStation_Mining`
- `BP_SpaceStation_Military`
- `BP_SpaceStation_MainHub`
- `BP_SpaceStation_SmallOutpost`
- `BP_SpaceStation_Marketplace`
- `BP_SpaceStation_Warehouse`
- `BP_SpaceStation_SupplyDepot`
- `BP_SpaceStation_RefuelingDepot`
- `BP_DefensePlatform`

### Ships
- `BP_Spaceship_Fighter`
- `BP_Spaceship_Corvette`
- `BP_Spaceship_Cruiser`
- `BP_Spaceship_Freighter`
- `BP_Spaceship_Miner`
- `BP_Spaceship_Scout`
- `BP_Spaceship_Courier`
- `BP_Spaceship_Trader`

### Objects
- `BP_Asteroid_Single`
- `BP_AsteroidCluster_Rich`
- `BP_SpaceDebris`
- `BP_SpaceCloud_Nebula`

### Waypoints
- `BP_NavigationBeacon`
- `BP_ScannerBuoy`
- `BP_MinefieldMarker`
- `BP_DockingBeacon`

**Note**: Update these paths to match your project's actual Blueprint class names and locations.

## Faction References

Templates reference these faction Data Assets:

- `DA_Faction_TradingGuild`
- `DA_Faction_MiningCooperative`
- `DA_Faction_Military`
- `DA_Faction_SectorPolice`
- `DA_Faction_Independent`

Update these to match your project's faction Data Assets.

## Ship Data References

Templates reference these spaceship Data Assets:

- `DA_Ship_Fighter_Interceptor`
- `DA_Ship_Corvette_Patrol`
- `DA_Ship_Corvette_Escort`
- `DA_Ship_Cruiser_Heavy`
- `DA_Ship_Freighter_Small`
- `DA_Ship_Freighter_Medium`
- `DA_Ship_Freighter_Large`
- `DA_Ship_Miner_Industrial`
- `DA_Ship_Scout_Light`
- `DA_Ship_Courier_Fast`
- `DA_Ship_Trader_Independent`

Update these to match your project's ship Data Assets.

## Testing Your Templates

1. **Start Small**: Begin with low object counts to test quickly
2. **Check Spacing**: Ensure objects don't overlap
3. **Verify Factions**: Check that faction ownership is applied correctly
4. **Test Patterns**: Try different distribution patterns
5. **Adjust Density**: Fine-tune the multiplier for desired feel
6. **Iterate**: Generate, review, adjust, regenerate

## Performance Considerations

- **High Density**: May cause performance issues (>100 objects total)
- **Grid Pattern**: Can be expensive for large object counts
- **Preview First**: Use preview mode before committing
- **Stagger Spawning**: For runtime generation, spawn over multiple frames

## Related Documentation

- `SectorGeneratorGuide.md` - Complete usage guide
- `SectorMapBuildingGuide.md` - Creating sectors
- `FactionSetupGuide.md` - Setting up factions

---

**Last Updated**: 2025-12-19  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
