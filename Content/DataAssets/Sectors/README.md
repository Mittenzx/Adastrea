# Sector Council Data Assets

This directory contains Sector Council Data Assets for different regions of space.

## Creating Sector Councils

### Method 1: Manual Creation in Unreal Editor

1. Right-click in this folder
2. Blueprint Class → Parent Class: `SectorCouncilDataAsset`
3. Name: `DA_Council_SectorName`
4. Double-click to open and configure properties
5. Use YAML templates from `Assets/SectorTemplates/` for reference

### Method 2: Using Templates

Copy values from YAML templates in `/Assets/SectorTemplates/`:

- `CoreSystemsSector.yaml` - Safe, balanced governance
- `FrontierMiningSector.yaml` - Dangerous mining territory  
- `TradeHubSector.yaml` - Commerce focused
- `ResearchSector.yaml` - Scientific hub
- `MilitaryBorderSector.yaml` - Border defense zone
- `IndependentColoniesSector.yaml` - Minimal governance
- `ContestedTerritorySector.yaml` - Political intrigue

## Existing Sectors

This directory will contain sector council data assets as they are created.

### Naming Convention

`DA_Council_[SectorName]`

Examples:
- `DA_Council_CoreSystems`
- `DA_Council_FrontierMining`
- `DA_Council_CrossroadsHub`
- `DA_Council_NexusResearch`

## Integration

Each Sector Council Data Asset should have a corresponding:
1. **ASpaceSectorMap actor** in the level with matching SectorName
2. **Content population** (stations, ships, resources)
3. **Way Data Assets** for all Ways represented on the council

## Documentation

For detailed guidance, see:
- `/Assets/SectorCreationGuide.md` - Complete creation guide
- `/Assets/SectorTemplates/README.md` - Template documentation
- `/Assets/WaySystemGuide.md` - Way system integration

---

**Note:** These Data Assets are created in Unreal Editor. The YAML templates provide
the configuration values to use when creating them.
