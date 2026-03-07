# YAML to Data Assets - Conversion Documentation

This directory contains comprehensive documentation for converting YAML template files to Unreal Engine Data Assets.

## 📋 Quick Links

- **[Complete Inventory](YAML_TO_ASSETS_CONVERSION_LIST.md)** - Detailed list of all 161 YAML files
- **[Quick Reference](YAML_TO_ASSETS_QUICK_REFERENCE.md)** - Summary table and import commands
- **[CSV List](YAML_FILES_LIST.csv)** - Machine-readable format for scripts
- **[Import Guide](YAML_IMPORT_GUIDE.md)** - How to use the YAMLtoDataAsset.py script

## 📊 Summary

| Metric | Count |
|--------|-------|
| **Total YAML Files** | 161 |
| **Manual Template Files** | 113 |
| **ProceduralContent Files** | 48 |
| **Main Categories** | 18 |
| **Fully Supported** | 2 categories (31 files) |
| **Partially Supported** | 5 categories (31 files) |
| **Not Yet Implemented** | 10 categories (51 files) |

## ✅ Conversion Status by Category

### High Priority (61 files)
- ✅ **Spaceships** (24 files) - Fully supported
- ✅ **Personnel** (7 files) - Fully supported
- ⚠️ **Trading Items** (23 files) - Partial support
- ⚠️ **Markets** (2 files) - Partial support
- ⚠️ **Contracts** (2 files) - Partial support
- ❌ **Station Modules** (20 files) - Needs implementation
- ❌ **Sectors** (7 files) - Needs implementation

### Medium Priority (19 files)
- ❌ **Way Networks** (5 files) - Needs implementation
- ❌ **Combat Weapons** (1 file) - Needs implementation
- ❌ **Combat VFX** (4 files) - Needs implementation
- ❌ **Ship Customization** (3 files) - Needs implementation
- ❌ **Inventory Items** (3 files) - Needs implementation
- ⚠️ **Personnel AI** (3 files) - Partial support

### Low Priority (11 files)
- ❌ **Exploration** (4 files) - Needs implementation
- ❌ **Audio** (2 files) - Needs implementation
- ❌ **Tutorials** (1 file) - Needs implementation
- ❌ **Spaceship Particles** (1 file) - Needs implementation
- ⚠️ **Materials** (1 file) - Partial support

### Optional (48 files)
- **ProceduralContent** - Generated examples, may regenerate as needed

## 🚀 Quick Start

### Import All Supported Content

```python
# In Unreal Editor Python Console
import YAMLtoDataAsset

# Import all spaceships (24 files)
YAMLtoDataAsset.batch_import_spaceships()

# Import all personnel (7 files)
YAMLtoDataAsset.batch_import_personnel()
```

### Import Single File

```python
import YAMLtoDataAsset

# Import specific ship
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")

# Import specific personnel
YAMLtoDataAsset.import_personnel_yaml("Assets/PersonnelTemplates/Captain.yaml")
```

## 📁 File Structure

```
Root/
├── YAML_TO_ASSETS_CONVERSION_LIST.md    # Complete detailed inventory
├── YAML_TO_ASSETS_QUICK_REFERENCE.md    # Quick reference tables
├── YAML_FILES_LIST.csv                  # CSV format for scripts
├── YAML_CONVERSION_README.md            # This file
├── YAML_IMPORT_GUIDE.md                 # Import script usage guide
└── YAMLtoDataAsset.py                   # Import script

Assets/
├── SpaceshipTemplates/                  # 24 ship YAML files ✅
├── PersonnelTemplates/                  # 7 personnel YAML files ✅
├── TradingTemplates/                    # 27 trading YAML files ⚠️
├── StationModuleTemplates/              # 20 station YAML files ❌
├── SectorTemplates/                     # 7 sector YAML files ❌
├── WayNetworkTemplates/                 # 5 way YAML files ❌
├── CombatTemplates/                     # 1 combat YAML file ❌
├── CombatVFXTemplates/                  # 4 VFX YAML files ❌
├── ShipCustomizationTemplates/          # 3 customization YAML files ❌
├── InventoryTemplates/                  # 3 inventory YAML files ❌
├── ExplorationTemplates/                # 4 exploration YAML files ❌
├── PersonnelAITemplates/                # 3 AI YAML files ⚠️
├── AudioTemplates/                      # 2 audio YAML files ❌
├── TutorialTemplates/                   # 1 tutorial YAML file ❌
├── SpaceshipParticleTemplates/          # 1 particle YAML file ❌
├── MaterialTemplates.yaml               # 1 material YAML file ⚠️
└── ProceduralContent/                   # 48 generated YAML files
```

## 🎯 For Developers

### Phase 1: Complete High Priority (31 files to implement)
1. ✅ Spaceships - Already done
2. ✅ Personnel - Already done
3. Complete Trading System (27 files) - Finish partial implementation
4. Add Station Modules (20 files) - Create Data Asset class
5. Add Sectors (7 files) - Create Data Asset class

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

### Required Data Asset Classes

Need to implement in C++:
- `UStationModuleDataAsset` - For station modules
- `USectorDataAsset` - For sectors
- `UWeaponDataAsset` - For combat weapons
- `UCombatVFXDataAsset` - For combat visual effects
- `UInventoryItemDataAsset` - For inventory items
- `UShipModuleDataAsset` - For ship customization
- `UAudioDataAsset` - For audio content
- `UTutorialDataAsset` - For tutorials
- `USpaceshipParticleDataAsset` - For ship VFX
- Various exploration-related Data Assets

## 🎨 For Designers

### What You Can Do Now

**Immediately Import:**
- All 24 spaceship templates
- All 7 personnel templates

**Coming Soon:**
- Trading items, markets, and contracts (partial support exists)
- Station modules (needs C++ implementation)
- All other systems (needs implementation)

### How to Import

1. Open Unreal Editor
2. Enable Python Editor Script Plugin (if not already enabled)
3. Open Python Console: Tools → Python → Open Python Console
4. Run import commands (see Quick Start above)

### Creating New YAML Templates

While waiting for other systems to be implemented, you can:
1. Create YAML templates using existing examples
2. Follow naming conventions in respective template folders
3. Templates will be ready to import once systems are implemented

## 📖 Related Documentation

- [YAML Import Guide](YAML_IMPORT_GUIDE.md) - Detailed usage of YAMLtoDataAsset.py
- [Spaceship Designer Workflow](Assets/SpaceshipDesignerWorkflow.md) - Ship creation
- [Personnel Designer Workflow](Assets/PersonnelDesignerWorkflow.md) - Crew creation
- [Trading Designer Workflow](Assets/TradingDesignerWorkflow.md) - Trading setup
- [Documentation Index](DOCUMENTATION_INDEX.md) - All documentation

## 🔍 Status Legend

- ✅ **Fully Supported** - Importer exists and tested
- ⚠️ **Partially Supported** - Basic functionality exists, needs completion
- ❌ **Not Implemented** - Needs C++ Data Asset class and Python importer

## 📝 Document Information

**Created**: 2025-11-23
**Version**: 1.0
**Maintained by**: Adastrea Development Team
**Related Scripts**: YAMLtoDataAsset.py

## 🤝 Contributing

Found an issue or want to add a new YAML template type?
1. Check [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines
2. Follow existing YAML template patterns
3. Update this documentation when adding new templates
4. Test with the import script before committing

---

For questions or issues with YAML conversion, please refer to the [complete documentation](YAML_TO_ASSETS_CONVERSION_LIST.md) or the [import guide](YAML_IMPORT_GUIDE.md).
