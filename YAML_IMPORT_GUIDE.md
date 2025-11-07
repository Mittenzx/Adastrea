# YAML to Data Asset Importer - Quick Start Guide

This script reads the existing YAML template files in the `Assets/` directory and creates Unreal Engine Data Assets from them automatically!

## What It Does

Instead of manually creating Data Assets in Unreal Editor, you can:
1. Edit YAML templates in your favorite text editor
2. Run this script in Unreal Editor
3. Automatically create Data Assets with all properties set!

## Quick Start

### Step 1: Enable Python Plugin

1. Open Unreal Editor
2. Edit → Plugins
3. Search "Python Editor Script Plugin"
4. Enable and restart

### Step 2: Run the Script

**Method A: Execute Script File**
1. Tools → Python → Execute Python Script
2. Select `YAMLtoDataAsset.py`
3. Interactive menu appears!

**Method B: Python Console**
1. Tools → Python → Open Python Console
2. Run:
```python
import YAMLtoDataAsset
YAMLtoDataAsset.show_menu()
```

## Usage Examples

### Import a Single Ship

1. Edit `Assets/SpaceshipTemplates/Scout_Pathfinder.yaml`
2. In Unreal Editor Python Console:
```python
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")
```
3. Data Asset created at `/Game/Spaceships/DataAssets/DA_Ship_PathfinderScout`!

### Import All Spaceships

```python
import YAMLtoDataAsset
YAMLtoDataAsset.batch_import_spaceships()
```

This imports ALL YAML files in `Assets/SpaceshipTemplates/` as Data Assets!

### Import Personnel

```python
import YAMLtoDataAsset
YAMLtoDataAsset.import_personnel_yaml("Assets/PersonnelTemplates/Captain.yaml")
```

### Import Everything

```python
import YAMLtoDataAsset
importer = YAMLtoDataAsset.get_importer()
importer.batch_import_spaceships()
importer.batch_import_personnel()
# etc...
```

## Interactive Menu

When you run the script, you see:

```
============================================================
YAML to Data Asset Importer
============================================================

Single Import:
  1. Import Spaceship YAML
  2. Import Personnel YAML
  3. Import Trade Item YAML
  4. Import Market YAML
  5. Import Contract YAML
  6. Import Faction AI YAML

Batch Import:
  7. Import All Spaceships
  8. Import All Personnel
  9. Import All Trading Templates
  10. Import All Faction AI Templates

  0. Exit
============================================================
```

## How It Works

1. **Reads YAML**: Parses your YAML template files
2. **Creates Data Asset**: Uses Unreal's Python API to create the asset
3. **Sets Properties**: Maps YAML fields to Data Asset properties
4. **Saves**: Automatically saves the asset

## Workflow

### Designer Workflow

1. **Copy Template**: Copy an existing YAML template
2. **Edit Values**: Modify stats, names, descriptions in your text editor
3. **Import**: Run the importer in Unreal Editor
4. **Use**: Data Asset is ready to use in Blueprints!

### Example: Creating a New Ship

```bash
# 1. Copy existing template
cp Assets/SpaceshipTemplates/Fighter_Viper.yaml Assets/SpaceshipTemplates/Fighter_MyShip.yaml

# 2. Edit the new file (change name, stats, etc.)

# 3. In Unreal Editor:
#    Tools → Python → Execute Python Script → YAMLtoDataAsset.py
#    Select option 1, enter: Assets/SpaceshipTemplates/Fighter_MyShip.yaml
```

Done! Your ship is now a Data Asset in `/Game/Spaceships/DataAssets/`!

## Supported Template Types

- ✅ **Spaceships** - All ship types (Fighter, Scout, Trader, etc.)
- ✅ **Personnel** - All crew roles (Captain, Pilot, Engineer, etc.)
- 🚧 **Trade Items** - Coming soon
- 🚧 **Markets** - Coming soon  
- 🚧 **Contracts** - Coming soon
- 🚧 **Faction AI** - Coming soon

## Property Mapping

The script automatically maps YAML fields to Unreal properties:

```yaml
# YAML
BasicInfo:
  ShipName: "Viper"
  ShipClass: "Fighter"
  
# Becomes
Data Asset Property: ship_name = "Viper"
Data Asset Property: ship_class = "Fighter"
```

## Advantages Over Manual Creation

✓ **Faster**: Import 20 ships in seconds vs. hours manually  
✓ **Accurate**: No typos or forgotten fields  
✓ **Version Control**: YAML files track in Git easily  
✓ **Shareable**: Send YAML files to team, they import  
✓ **Editable**: Use any text editor, not just Unreal  

## Updating Existing Assets

If you run the importer on a YAML file that already has a Data Asset:
- The script **loads the existing asset** 
- **Updates the properties** from the YAML
- No duplication!

## Troubleshooting

### "SpaceshipDataAsset class not found"

**Solution**: Ensure your project is compiled and C++ classes are loaded.

### "PyYAML not available"

**Solution**: The script will auto-install PyYAML. If it fails:
```python
import subprocess, sys
subprocess.check_call([sys.executable, "-m", "pip", "install", "pyyaml"])
```

### "Asset path invalid"

**Solution**: The script creates assets in `/Game/Spaceships/DataAssets/` etc. Ensure these folders exist or the script will create them.

## Integration with Other Scripts

This script works perfectly with the template generator scripts:

1. **TemplateGenerator.py** - Creates NEW YAML template files
2. **YAMLtoDataAsset.py** - Imports YAML files as Data Assets
3. **EditorUtilities.py** - Provides unified menu for both

Complete workflow:
```python
import EditorUtilities
# Generate YAML template
EditorUtilities.generate_spaceship("MyShip", "Fighter")
# Edit the YAML file in your text editor...
# Import as Data Asset
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Fighter_MyShip.yaml")
```

## Requirements

- Unreal Engine 5.5+
- Python Editor Script Plugin enabled
- PyYAML (auto-installed if missing)
- Project must be compiled (for C++ Data Asset classes)

## Files

- `YAMLtoDataAsset.py` - Main importer script
- `Assets/SpaceshipTemplates/*.yaml` - Ship templates
- `Assets/PersonnelTemplates/*.yaml` - Personnel templates
- `Assets/TradingTemplates/*.yaml` - Trading templates
- `Assets/FactionAITemplates/*.yaml` - AI templates

## Next Steps

1. **Try it**: Import one of the existing templates
2. **Create**: Make your own YAML template
3. **Import**: Convert to Data Asset
4. **Use**: Reference in your Blueprints!

---

**Questions?** Check the main documentation:
- [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md) - Template generation
- [SCRIPTS_README.md](SCRIPTS_README.md) - All scripts overview

---

*Last Updated: 2025-11-07*
