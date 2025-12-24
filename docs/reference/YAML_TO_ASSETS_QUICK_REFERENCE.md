# YAML to Assets Conversion - Quick Reference

Quick reference guide for converting YAML template files to Unreal Engine Data Assets.

## Summary by Category

| # | Category | File Count | Status | Target Data Asset | Priority |
|---|----------|------------|--------|-------------------|----------|
| 1 | Spaceships | 24 | ✅ Supported | `USpaceshipDataAsset` | High |
| 2 | Personnel | 7 | ✅ Supported | `UPersonnelDataAsset` | High |
| 3 | Trading Items | 23 | ⚠️ Partial | `UTradeItemDataAsset` | High |
| 4 | Trading Markets | 2 | ⚠️ Partial | `UMarketDataAsset` | High |
| 5 | Trading Contracts | 2 | ⚠️ Partial | `UContractDataAsset` | High |
| 6 | Station Modules | 20 | ❌ Not Impl | `UStationModuleDataAsset` | High |
| 7 | Sectors | 7 | ❌ Not Impl | `USectorDataAsset` | High |
| 8 | Way Networks | 5 | ❌ Not Impl | `UWayDataAsset` | Medium |
| 9 | Combat Weapons | 1 | ❌ Not Impl | `UWeaponDataAsset` | Medium |
| 10 | Combat VFX | 4 | ❌ Not Impl | `UCombatVFXDataAsset` | Medium |
| 11 | Ship Customization | 3 | ❌ Not Impl | `UShipModuleDataAsset` | Medium |
| 12 | Inventory Items | 3 | ❌ Not Impl | `UInventoryItemDataAsset` | Medium |
| 13 | Exploration | 4 | ❌ Not Impl | Multiple types | Low |
| 14 | Personnel AI | 3 | ⚠️ Partial | `UPersonnelAIDataAsset` | Low |
| 15 | Audio | 2 | ❌ Not Impl | `UAudioDataAsset` | Low |
| 16 | Tutorials | 1 | ❌ Not Impl | `UTutorialDataAsset` | Low |
| 17 | Spaceship Particles | 1 | ❌ Not Impl | `USpaceshipParticleDataAsset` | Low |
| 18 | Materials | 1 | ⚠️ Partial | `UMaterialDataAsset` | Low |
| 19 | ProceduralContent | 48 | N/A | Various | Optional |
| 20 | Population Templates | 2 | N/A | Config files | N/A |

**Total Manual Templates**: 113 files  
**Total with Generated**: 161 files

## Quick Import Commands

### Already Supported

```python
# Import in Unreal Editor Python Console
import YAMLtoDataAsset

# Batch import all spaceships (24 files)
YAMLtoDataAsset.batch_import_spaceships()

# Batch import all personnel (7 files)
YAMLtoDataAsset.batch_import_personnel()

# Import single file
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")
YAMLtoDataAsset.import_personnel_yaml("Assets/PersonnelTemplates/Captain.yaml")
```

### Needs Implementation

The following categories need importer functions added to `YAMLtoDataAsset.py`:

- `batch_import_trade_items()` - 23 files
- `batch_import_markets()` - 2 files
- `batch_import_contracts()` - 2 files
- `batch_import_station_modules()` - 20 files
- `batch_import_sectors()` - 7 files
- `batch_import_way_networks()` - 5 files
- `batch_import_combat_weapons()` - 1 file
- `batch_import_combat_vfx()` - 4 files
- `batch_import_ship_modules()` - 3 files
- `batch_import_inventory_items()` - 3 files
- `batch_import_exploration()` - 4 files
- `batch_import_personnel_ai()` - 3 files
- `batch_import_audio()` - 2 files
- `batch_import_tutorials()` - 1 file
- `batch_import_ship_particles()` - 1 file
- `batch_import_materials()` - 1 file

## File Locations by Category

### Core Systems (Highest Priority)
```
Assets/SpaceshipTemplates/          → 24 YAML files ✅
Assets/PersonnelTemplates/          → 7 YAML files ✅
Assets/TradingTemplates/            → 27 YAML files ⚠️
Assets/StationModuleTemplates/      → 20 YAML files ❌
Assets/SectorTemplates/             → 7 YAML files ❌
```

### Gameplay Systems (Medium Priority)
```
Assets/WayNetworkTemplates/         → 5 YAML files ❌
Assets/CombatTemplates/             → 1 YAML file ❌
Assets/CombatVFXTemplates/          → 4 YAML files ❌
Assets/ShipCustomizationTemplates/  → 3 YAML files ❌
Assets/InventoryTemplates/          → 3 YAML files ❌
```

### Polish & Content (Lower Priority)
```
Assets/ExplorationTemplates/        → 4 YAML files ❌
Assets/PersonnelAITemplates/        → 3 YAML files ⚠️
Assets/AudioTemplates/              → 2 YAML files ❌
Assets/TutorialTemplates/           → 1 YAML file ❌
Assets/SpaceshipParticleTemplates/  → 1 YAML file ❌
Assets/MaterialTemplates.yaml       → 1 YAML file ⚠️
```

### Generated Content (Optional)
```
Assets/ProceduralContent/Galaxy/    → 5 YAML files
Assets/ProceduralContent/Personnel/ → 20 YAML files
Assets/ProceduralContent/Quests/    → 15 YAML files
Assets/ProceduralContent/Ships/     → 8 YAML files
```

## Implementation Roadmap

### Phase 1: Complete High Priority (31 files to implement)
1. ✅ Spaceships - Already done
2. ✅ Personnel - Already done
3. ⚠️ Complete Trading System (27 files)
4. ❌ Add Station Modules (20 files)
5. ❌ Add Sectors (7 files)

### Phase 2: Gameplay Enhancement (19 files)
1. Way Networks (5 files)
2. Combat Weapons (1 file)
3. Combat VFX (4 files)
4. Ship Customization (3 files)
5. Inventory Items (3 files)
6. Personnel AI (3 files)

### Phase 3: Polish & Content (11 files)
1. Exploration Systems (4 files)
2. Audio Templates (2 files)
3. Tutorial Templates (1 file)
4. Spaceship Particles (1 file)
5. Materials (1 file)
6. Other systems as needed

### Phase 4: Procedural Content (48 files - Optional)
- May be regenerated as needed
- Can use existing importers once implemented
- Primarily for testing and examples

## Status Legend

- ✅ **Fully Supported**: Importer exists and tested
- ⚠️ **Partially Supported**: Basic functionality exists, needs completion
- ❌ **Not Implemented**: Needs C++ Data Asset class and Python importer
- N/A: Not applicable (config files or generated content)

## See Also

- [Complete List](YAML_TO_ASSETS_CONVERSION_LIST.md) - Full detailed inventory
- [YAML Import Guide](YAML_IMPORT_GUIDE.md) - How to use the importer
- [YAMLtoDataAsset.py](YAMLtoDataAsset.py) - The import script

---

**Last Updated**: 2025-11-23  
**Version**: 1.0
