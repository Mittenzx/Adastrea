# Adastrea Content Generation Guide

This guide explains how to use the Python content generation scripts to automatically create all missing Blueprints, Data Assets, Input configurations, Maps, and UI Widgets for the Adastrea project.

## Overview

The Adastrea project is **code-complete** (85% complete) but **content-incomplete** (10% complete). The C++ systems are implemented, but editor content needs to be created. These Python scripts automate that process.

## Quick Start

### One-Command Generation (Recommended)

Run this in the Unreal Editor Python Console to generate **everything**:

```python
import MasterContentGenerator
MasterContentGenerator.generate_all_content()
```

This will create:
- ✓ Essential and additional Blueprints (~50 assets)
- ✓ Data Assets from YAML templates (~161 assets)
- ✓ Enhanced Input system (~35 assets)
- ✓ Test maps (~4 maps)
- ✓ UI Widgets (~12 widgets)

**Total: ~260+ assets created automatically!**

## Available Scripts

### 1. MasterContentGenerator.py

**Purpose**: Orchestrates all other generators - the "one-stop shop" for content generation.

**Usage**:
```python
# In Unreal Editor Python Console
import MasterContentGenerator

# Generate everything (recommended)
MasterContentGenerator.generate_all_content()

# Generate specific categories
MasterContentGenerator.generate_blueprints()
MasterContentGenerator.generate_data_assets()
MasterContentGenerator.generate_input_system()
MasterContentGenerator.generate_maps()
MasterContentGenerator.generate_ui_widgets()

# Validate generated content
MasterContentGenerator.validate_content()
```

**Command Line**:
```bash
# Interactive mode
python MasterContentGenerator.py --interactive

# Generate everything
python MasterContentGenerator.py --all

# Generate specific content
python MasterContentGenerator.py --blueprints --data-assets

# With validation
python MasterContentGenerator.py --all --validate
```

### 2. BlueprintGenerator.py

**Purpose**: Creates Blueprint assets for GameModes, Controllers, Ships, Stations, UI, and more.

**Usage**:
```python
import BlueprintGenerator

# Generate essential blueprints (minimum required)
BlueprintGenerator.generate_essential_blueprints()

# Generate all blueprints (~50 blueprints)
BlueprintGenerator.generate_all_blueprints()

# Create custom blueprint
BlueprintGenerator.create_blueprint("BP_MyShip", "ASpaceship", "/Game/Blueprints/Ships")
```

**What it Creates**:
- ✓ BP_SpaceGameMode
- ✓ BP_GameInstance
- ✓ BP_SpaceshipController
- ✓ BP_PlayerShip
- ✓ BP_Ship_NPC_* variants
- ✓ BP_SpaceStation_Main
- ✓ BP_Module_* variants (10 types)
- ✓ Environmental actors
- ✓ Combat blueprints
- ✓ AI controllers

### 3. DataAssetBatchCreator.py

**Purpose**: Batch converts all YAML templates to Unreal Data Assets.

**Usage**:
```python
import DataAssetBatchCreator

# Convert all YAML templates (~161 files)
DataAssetBatchCreator.batch_convert_all()

# Convert specific categories
DataAssetBatchCreator.batch_convert_spaceships()
DataAssetBatchCreator.batch_convert_personnel()
DataAssetBatchCreator.batch_convert_trading()
DataAssetBatchCreator.batch_convert_stations()
```

**What it Creates**:
- ✓ 24 Spaceship Data Assets
- ✓ 7 Personnel Data Assets
- ✓ 27 Trading Data Assets (items, markets, contracts)
- ✓ 20 Station Module Data Assets
- ✓ 7 Sector Data Assets
- ✓ 5 Way Network Data Assets
- ✓ Combat, Exploration, Audio, and more

### 4. InputSystemGenerator.py

**Purpose**: Generates Enhanced Input Actions, Mapping Contexts, and Input Config Data Asset.

**Usage**:
```python
import InputSystemGenerator

# Generate complete input system (~35 assets)
InputSystemGenerator.generate_complete_input_system()

# Generate specific parts
InputSystemGenerator.generate_input_actions()
InputSystemGenerator.generate_mapping_contexts()
InputSystemGenerator.generate_input_config_data_asset()
```

**What it Creates**:
- ✓ 30+ Input Actions (IA_Move, IA_VerticalMove, IA_Throttle, IA_Look, IA_Fire_Primary, etc.)
- ✓ 3 Input Mapping Contexts (IMC_Spaceship, IMC_Menu, IMC_Station)
- ✓ DA_InputConfig Data Asset

**Key Bindings** (Spaceship Controls):
- **A/D**: Strafe Left/Right
- **W/S**: Move Up/Down (Z axis)
- **Mouse Wheel Up**: Increase Throttle
- **Mouse Wheel Down**: Decrease Throttle
- **Mouse Movement**: Look/Camera Control
- **Left Mouse Button**: Fire Primary Weapon
- **Right Mouse Button**: Fire Secondary Weapon
- **Shift**: Boost
- **Ctrl**: Brake

### 5. MapGenerator.py

**Purpose**: Creates test maps/levels with basic environment setup.

**Usage**:
```python
import MapGenerator

# Generate all test maps
MapGenerator.generate_test_maps()

# Create specific map
MapGenerator.create_test_mode_map()
```

**What it Creates**:
- ✓ TestMode.umap (basic test environment)
- ✓ SectorTest_Alpha.umap
- ✓ SectorTest_Beta.umap
- ✓ CombatArena.umap

Each map includes:
- PlayerStart
- Directional Light
- Sky Light
- Post Process Volume
- Basic floor plane
- SpaceshipSpawnPoint markers

### 6. UIWidgetGenerator.py

**Purpose**: Creates UI Widget Blueprints for HUD, menus, and gameplay UI.

**Usage**:
```python
import UIWidgetGenerator

# Generate all UI widgets
UIWidgetGenerator.generate_all_widgets()

# Create custom widget
UIWidgetGenerator.create_widget_blueprint("WBP_MyUI", "UserWidget", "/Game/UI/Custom")
```

**What it Creates**:
- ✓ WBP_SpaceshipHUD
- ✓ WBP_MainMenu
- ✓ WBP_PauseMenu
- ✓ WBP_SettingsMenu
- ✓ WBP_TradingInterface
- ✓ WBP_StationManagement
- ✓ WBP_InventoryWidget
- ✓ WBP_ShipCustomization
- ✓ WBP_QuestLog
- ✓ WBP_SectorMap
- ✓ And more...

### 7. ContentValidator.py

**Purpose**: Validates all generated content to ensure it works correctly.

**Usage**:
```python
import ContentValidator

# Validate all content
ContentValidator.validate_all()

# Validate specific categories
ContentValidator.validate_blueprints()
ContentValidator.validate_data_assets()
ContentValidator.validate_widgets()
ContentValidator.validate_input_system()
ContentValidator.validate_maps()
```

**What it Checks**:
- ✓ Blueprint existence and compilation
- ✓ Data Asset existence
- ✓ Input Action and Mapping Context existence
- ✓ Map existence
- ✓ UI Widget existence
- ✓ Generates detailed validation report

## Step-by-Step Workflow

### Recommended Workflow for New Setup

1. **Open Unreal Editor**
   - Launch Adastrea.uproject in Unreal Engine 5.6

2. **Open Python Console**
   - Tools → Python → Python Console

3. **Run Master Generator**
   ```python
   import MasterContentGenerator
   MasterContentGenerator.generate_all_content()
   ```

4. **Wait for Completion**
   - Watch the console output for progress
   - This may take 5-10 minutes depending on system

5. **Review Generated Content**
   - Check Content Browser for new assets
   - Open a few blueprints to verify

6. **Configure Project Settings**
   - Edit → Project Settings
   - Maps & Modes → Default GameMode → BP_SpaceGameMode
   - Maps & Modes → Game Default Map → MainMenu
   - Input → Enable Enhanced Input System

7. **Test the Game**
   - Open Content/Maps/TestMode.umap
   - Press Play
   - Test controls and basic gameplay

### Incremental Workflow (Generate Specific Content)

If you only need specific content:

1. **Blueprints Only**:
   ```python
   import BlueprintGenerator
   BlueprintGenerator.generate_essential_blueprints()
   ```

2. **Data Assets Only**:
   ```python
   import DataAssetBatchCreator
   DataAssetBatchCreator.batch_convert_all()
   ```

3. **Input System Only**:
   ```python
   import InputSystemGenerator
   InputSystemGenerator.generate_complete_input_system()
   ```

4. **Validate Everything**:
   ```python
   import ContentValidator
   ContentValidator.validate_all()
   ```

## Troubleshooting

### "This script must be run inside Unreal Editor!"

**Problem**: Script was run outside Unreal Editor.

**Solution**: 
1. Open Adastrea.uproject in Unreal Engine
2. Open Python Console (Tools → Python → Python Console)
3. Run the script in the console

### "PyYAML is required"

**Problem**: PyYAML Python library is not installed.

**Solution**:
```python
# Run in Unreal Editor Python Console
import subprocess, sys
subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyyaml'])
```

### "Could not find parent class: ClassName"

**Problem**: Referenced C++ class doesn't exist or isn't compiled.

**Solution**:
1. Ensure project is compiled in Development Editor configuration
2. Check that the C++ class exists in Source/Adastrea/
3. If class is missing, that system may not be implemented yet

### Generated Blueprints Have Errors

**Problem**: Some generated Blueprints show compile errors.

**Solution**:
1. This is normal - Blueprints need manual configuration
2. Open each blueprint and set required properties
3. Add components as needed (see BLUEPRINT_REQUIREMENTS_LIST.md)
4. Compile the blueprint

### Assets Already Exist

**Problem**: Script says "Asset already exists" and skips creation.

**Solution**:
- This is expected behavior - script won't overwrite existing assets
- If you want to regenerate, manually delete the asset first
- Check Content Browser for existing assets

## Manual Configuration Required

While these scripts automate asset creation, some manual configuration is still needed:

### Blueprints
1. **BP_PlayerShip**: Add mesh, camera components, configure properties
2. **BP_SpaceGameMode**: Set DefaultPawnClass, PlayerControllerClass
3. **BP_SpaceshipController**: Configure input bindings

### Data Assets
1. **DA_InputConfig**: Assign Input Actions and Mapping Contexts
2. **DA_Ship_***: Review and adjust ship statistics
3. **DA_Faction_***: Configure faction relationships

### UI Widgets
1. **WBP_SpaceshipHUD**: Design HUD layout, add text/images
2. **WBP_MainMenu**: Add buttons, bind events
3. **WBP_TradingInterface**: Design trading UI

### Maps
1. **TestMode.umap**: Configure World Settings → Game Mode Override
2. Add environmental details
3. Place additional test actors as needed

See BLUEPRINT_REQUIREMENTS_LIST.md and BLUEPRINT_CREATION_GUIDE.md for detailed configuration instructions.

## Integration with Existing Tools

These scripts work alongside existing automation:

- **ProceduralGenerators.py**: Generates procedural content (NPCs, quests, ships)
- **YAMLtoDataAsset.py**: Original YAML import script (extended by DataAssetBatchCreator.py)
- **ScenePopulator.py**: Populates levels with actors
- **TemplateGenerator.py**: Generates YAML templates

Workflow:
1. Use **TemplateGenerator.py** to create YAML templates
2. Use **DataAssetBatchCreator.py** to convert YAML to Data Assets
3. Use **BlueprintGenerator.py** to create Blueprints
4. Use **ScenePopulator.py** to populate levels with actors
5. Use **ContentValidator.py** to verify everything works

## Performance Notes

- **MasterContentGenerator**: Takes 5-10 minutes to complete
- **BlueprintGenerator**: ~2 minutes for all blueprints
- **DataAssetBatchCreator**: ~3-5 minutes for all YAML conversions
- **InputSystemGenerator**: ~1 minute
- **MapGenerator**: ~1 minute
- **UIWidgetGenerator**: ~1 minute

Total time: ~10-15 minutes for complete content generation.

## What Gets Created

| Category | Count | Time | Status |
|----------|-------|------|--------|
| Blueprints | ~50 | 2 min | ✓ Automated |
| Data Assets | ~161 | 5 min | ✓ Automated |
| Input Assets | ~35 | 1 min | ✓ Automated |
| Maps | ~4 | 1 min | ✓ Automated |
| UI Widgets | ~12 | 1 min | ✓ Automated |
| **TOTAL** | **~260** | **~10 min** | ✓ Automated |

## Further Reading

- **BLUEPRINT_REQUIREMENTS_LIST.md**: Complete list of required Blueprints
- **BLUEPRINT_CREATION_GUIDE.md**: Step-by-step Blueprint creation guide
- **YAML_TO_ASSETS_CONVERSION_LIST.md**: List of YAML files to convert
- **ZZ_WHATS_STILL_NEEDED.md**: What's missing and what needs work
- **MASTER_DOCUMENTATION.md**: Complete project documentation

## Support

If you encounter issues:

1. Check this guide's Troubleshooting section
2. Review the Output Log in Unreal Editor for detailed errors
3. Check DOCUMENTATION_INDEX.md for related guides
4. File an issue on GitHub with error details

## Version History

- **1.0** (December 10, 2025) - Initial release of content generation scripts

---

**Happy Content Creating!** 🚀

With these scripts, you can generate 260+ assets in ~10 minutes instead of spending weeks creating them manually!
