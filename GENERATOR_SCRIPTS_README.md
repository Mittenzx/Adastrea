# Adastrea Template and Guide Generator Scripts

This directory contains Python scripts for generating YAML templates, Markdown documentation guides, AND importing YAML templates as Unreal Engine Data Assets for the Adastrea project. These scripts can be run from the command line or directly within Unreal Editor's Python environment.

## Overview

Four powerful scripts are provided:

1. **TemplateGenerator.py** - Generate YAML template files for game systems
2. **GuideGenerator.py** - Generate Markdown documentation and guides
3. **EditorUtilities.py** - Unified interface for both, optimized for Unreal Editor
4. **YAMLtoDataAsset.py** - Import YAML templates as Unreal Engine Data Assets (NEW!)

> **Complete Workflow**: Create YAML template → Edit in text editor → Import as Data Asset → Use in Blueprints!

## Quick Start

### Command Line Usage

```bash
# Generate a spaceship template
python TemplateGenerator.py --type spaceship --name "Striker" --class Fighter

# Generate a system guide
python GuideGenerator.py --type system --name "MySystem"

# Interactive menu mode
python EditorUtilities.py --menu
```

### Unreal Editor Usage

1. Enable "Python Editor Script Plugin" in Unreal Engine
2. Go to **Tools → Python → Execute Python Script**
3. Select `EditorUtilities.py` or run commands in Python console:

```python
import EditorUtilities
EditorUtilities.show_menu()

# Or call specific functions
EditorUtilities.generate_spaceship("MyShip", "Fighter")
EditorUtilities.generate_system_guide("MySystem")
```

### Importing YAML as Data Assets (NEW!)

1. Enable "Python Editor Script Plugin" in Unreal Engine
2. Go to **Tools → Python → Execute Python Script**
3. Select `YAMLtoDataAsset.py` or run in Python console:

```python
import YAMLtoDataAsset

# Import single file
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")

# Batch import all ships
YAMLtoDataAsset.batch_import_spaceships()

# Interactive menu
YAMLtoDataAsset.show_menu()
```

> **See [YAML_IMPORT_GUIDE.md](YAML_IMPORT_GUIDE.md) for complete import documentation!**

---

## YAMLtoDataAsset.py (NEW!)

### Purpose
Reads existing YAML template files and creates Unreal Engine Data Assets from them automatically.

### What It Does
- **Imports YAML** templates as Data Assets
- **Maps properties** from YAML to Unreal asset fields
- **Batch import** entire directories at once
- **Updates existing** assets without duplication
- **Auto-installs** PyYAML if needed

### Supported Types
- ✅ Spaceships (all ship classes)
- ✅ Personnel (all crew roles)
- 🚧 Trade Items (coming soon)
- 🚧 Markets (coming soon)
- 🚧 Contracts (coming soon)
- 🚧 Faction AI (coming soon)

### Usage

**Single Import:**
```python
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Fighter_Viper.yaml")
# Creates: /Game/Spaceships/DataAssets/DA_Ship_Viper
```

**Batch Import:**
```python
import YAMLtoDataAsset
YAMLtoDataAsset.batch_import_spaceships()
# Imports ALL YAML files in Assets/SpaceshipTemplates/
```

### Complete Workflow Example

```bash
# 1. Generate YAML template (optional - or edit existing)
python TemplateGenerator.py --type spaceship --name "MyShip" --class Fighter

# 2. Edit the YAML file in your text editor
# Assets/SpaceshipTemplates/Fighter_MyShip.yaml

# 3. In Unreal Editor Python Console:
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Fighter_MyShip.yaml")

# 4. Data Asset created at /Game/Spaceships/DataAssets/DA_Ship_MyShip!
```

---

## TemplateGenerator.py

### Purpose
Creates YAML template files for various game systems with pre-filled structure and helpful comments.

### Supported Template Types

| Type | Description | Example |
|------|-------------|---------|
| `spaceship` | Spaceship configuration | Fighter, Scout, Trader |
| `personnel` | Character/crew member | Captain, Pilot, Engineer |
| `tradeitem` | Tradeable item | Resources, Manufactured goods |
| `market` | Trading market | Standard, FreePort, BlackMarket |
| `contract` | Mission/quest | Delivery, Combat, Exploration |
| `factionai` | AI faction behavior | Diplomatic, Aggressive, Trading |
| `material` | Material properties | Metals, Composites, Exotic |

### Command Line Usage

#### Interactive Mode
```bash
python TemplateGenerator.py
```

Presents a menu to select template type and input parameters.

#### Batch Mode

**Spaceship Templates:**
```bash
python TemplateGenerator.py --type spaceship --name "Striker" --class Fighter
python TemplateGenerator.py --type spaceship --name "Recon One" --class Scout
```

**Personnel Templates:**
```bash
python TemplateGenerator.py --type personnel --name "John Smith" --role Captain
python TemplateGenerator.py --type personnel --name "Sarah Jones" --role Engineer
```

**Trade Item Templates:**
```bash
python TemplateGenerator.py --type tradeitem --name "Titanium Ore" --category RawMaterials
python TemplateGenerator.py --type tradeitem --name "Luxury Goods" --category Luxury
```

**Market Templates:**
```bash
python TemplateGenerator.py --type market --name "Central Hub" --market-type Standard
python TemplateGenerator.py --type market --name "Underground Exchange" --market-type BlackMarket
```

**Contract Templates:**
```bash
python TemplateGenerator.py --type contract --name "Delivery Mission" --contract-type Delivery
python TemplateGenerator.py --type contract --name "Pirate Hunt" --contract-type Combat
```

**Faction AI Templates:**
```bash
python TemplateGenerator.py --type factionai --name "Trade Federation"
python TemplateGenerator.py --type factionai --name "Pirate Clans"
```

**Material Templates:**
```bash
python TemplateGenerator.py --type material --name "Steel Alloy"
python TemplateGenerator.py --type material --name "Exotic Crystal"
```

#### Custom Output Directory
```bash
python TemplateGenerator.py --type spaceship --name "MyShip" --class Fighter --output Custom/Path/
```

### Generated File Locations

By default, templates are created in:

- **Spaceships**: `Assets/SpaceshipTemplates/`
- **Personnel**: `Assets/PersonnelTemplates/`
- **Trading**: `Assets/TradingTemplates/`
- **Faction AI**: `Assets/FactionAITemplates/`
- **Materials**: `Assets/`

### Template Features

Each generated template includes:

- ✓ Pre-filled structure with all required fields
- ✓ Inline comments explaining each property
- ✓ Recommended value ranges
- ✓ Auto-generated timestamp
- ✓ Consistent naming conventions
- ✓ Ready to customize and use

### Example: Generated Spaceship Template

```yaml
# Striker - Fighter Class
# Generated on 2025-11-07 11:22:45
# Customize this template for your ship design

BasicInfo:
  ShipName: "Striker"
  Description: "A fighter class vessel designed for [describe purpose]."
  ShipClass: "Fighter"
  ShipID: "Striker"

CoreStats:
  HullStrength: 1000.0          # Durability of the ship's hull
  CargoCapacity: 100.0          # Cargo space in tons
  CrewRequired: 2               # Minimum crew to operate
  MaxCrew: 6                    # Maximum crew capacity
  ModularPoints: 8              # Customization points

# ... (complete template with all stats)
```

---

## GuideGenerator.py

### Purpose
Creates comprehensive Markdown documentation guides for game systems.

### Supported Guide Types

| Type | Description | Audience |
|------|-------------|----------|
| `system` | Complete system documentation | All users |
| `workflow` | Step-by-step designer workflow | Designers |
| `quickref` | One-page quick reference | Quick lookup |
| `api` | Complete API reference | Programmers |

### Command Line Usage

#### Interactive Mode
```bash
python GuideGenerator.py
```

#### Batch Mode

**System Guide** (Complete documentation):
```bash
python GuideGenerator.py --type system --name "MySystem"
```

**Workflow Guide** (Designer-focused):
```bash
python GuideGenerator.py --type workflow --name "MySystem"
```

**Quick Reference** (Cheat sheet):
```bash
python GuideGenerator.py --type quickref --name "MySystem"
```

**API Reference** (Technical documentation):
```bash
python GuideGenerator.py --type api --name "MySystem"
```

#### Custom Output Directory
```bash
python GuideGenerator.py --type system --name "MySystem" --output Docs/
```

### Generated File Locations

By default, guides are created in `Assets/`:

- System Guide: `{SystemName}SystemGuide.md`
- Workflow Guide: `{SystemName}DesignerWorkflow.md`
- Quick Reference: `{SystemName}QuickReference.md`
- API Reference: `{SystemName}APIReference.md`

### Guide Features

Each guide type includes:

**System Guide:**
- Overview and key concepts
- Architecture diagrams
- Data Asset configuration
- Blueprint integration
- C++ API reference
- Workflow examples
- Best practices
- Troubleshooting

**Workflow Guide:**
- Step-by-step instructions
- Phase-based workflow
- Common patterns
- Testing procedures
- Designer-friendly language
- No coding required

**Quick Reference:**
- One-page cheat sheet
- Essential properties table
- Common Blueprint nodes
- Console commands
- Keyboard shortcuts
- Quick troubleshooting

**API Reference:**
- Complete class documentation
- All public functions
- Property descriptions
- Events and delegates
- C++ code examples
- Blueprint usage examples

---

## EditorUtilities.py

### Purpose
Unified interface for template and guide generation, optimized for Unreal Editor Python environment.

### Features

- Works in Unreal Editor Python console
- Interactive menu system
- Batch documentation generation
- Template and guide listing
- Unreal Editor logging integration

### Usage in Unreal Editor

#### Step 1: Enable Python Plugin

1. **Edit → Plugins**
2. Search for "Python Editor Script Plugin"
3. Enable and restart Unreal Editor

#### Step 2: Run Script

**Method A: Execute Script File**
1. **Tools → Python → Execute Python Script**
2. Navigate to project root
3. Select `EditorUtilities.py`

**Method B: Import in Python Console**
1. **Tools → Python → Open Python Console**
2. Run:
   ```python
   import EditorUtilities
   EditorUtilities.show_menu()
   ```

### Menu Options

```
Adastrea Editor Utilities
============================================================

Template Generation:
  1. Generate Spaceship Template
  2. Generate Personnel Template
  3. Generate Trade Item Template
  4. Generate Market Template
  5. Generate Contract Template
  6. Generate Faction AI Template
  7. Generate Material Template

Guide Generation:
  8. Generate System Guide
  9. Generate Workflow Guide
  10. Generate Quick Reference
  11. Generate API Reference

Information:
  12. List All Templates
  13. List All Guides

Batch Operations:
  14. Generate Complete System Documentation Set

  0. Exit
```

### Direct Function Calls

Instead of using the menu, you can call functions directly:

```python
import EditorUtilities

# Templates
EditorUtilities.generate_spaceship("MyShip", "Fighter")
EditorUtilities.generate_personnel("Captain Smith", "Captain")
EditorUtilities.generate_trade_item("Iron Ore", "RawMaterials")

# Guides
EditorUtilities.generate_system_guide("MySystem")
EditorUtilities.generate_workflow_guide("MySystem")
EditorUtilities.generate_quick_reference("MySystem")
EditorUtilities.generate_api_reference("MySystem")

# Information
EditorUtilities.list_templates()
EditorUtilities.list_guides()

# Batch generation
EditorUtilities.generate_complete_docs("MySystem")
```

### Batch Documentation Generation

Generate a complete documentation set for a new system:

```python
import EditorUtilities
EditorUtilities.generate_complete_docs("MyNewSystem")
```

This creates:
- System Guide
- Workflow Guide
- Quick Reference
- API Reference

All in one command!

### Command Line Usage

EditorUtilities can also be run from command line:

```bash
# Show interactive menu
python EditorUtilities.py --menu

# List templates
python EditorUtilities.py --list-templates

# List guides
python EditorUtilities.py --list-guides
```

---

## Workflow Examples

### Creating a New Ship Type

```bash
# 1. Generate the template
python TemplateGenerator.py --type spaceship --name "Interceptor" --class Fighter

# 2. Edit Assets/SpaceshipTemplates/Fighter_Interceptor.yaml
# 3. Customize values as needed
# 4. Create Data Asset in Unreal Editor from template
```

### Documenting a New System

```bash
# Generate complete documentation set
python GuideGenerator.py --type system --name "NewSystem"
python GuideGenerator.py --type workflow --name "NewSystem"
python GuideGenerator.py --type quickref --name "NewSystem"
python GuideGenerator.py --type api --name "NewSystem"

# Or use batch generation
python EditorUtilities.py
# Select option 14, enter "NewSystem"
```

### Designer Workflow

1. **Open Unreal Editor**
2. **Tools → Python → Execute Python Script**
3. **Select EditorUtilities.py**
4. **Choose from menu**:
   - Option 1 for new ship
   - Option 2 for new character
   - Option 3 for new trade item
5. **Fill in the prompts**
6. **Template created in Assets/ directory**
7. **Customize in your favorite text editor**

---

## Best Practices

### Template Generation

✓ **Use Descriptive Names**: "FastScout" instead of "Ship1"
✓ **Follow Conventions**: Capitalize classes, use proper roles
✓ **Customize Immediately**: Edit templates right after generation
✓ **Version Control**: Commit templates to track changes
✓ **Test in Editor**: Verify templates work before sharing

### Guide Generation

✓ **Generate Early**: Create guides when starting new systems
✓ **Keep Updated**: Regenerate guides when systems change
✓ **Complete Sets**: Generate all guide types for completeness
✓ **Review Content**: Guides are templates - customize them!
✓ **Link Together**: Reference other guides for navigation

### Organization

✓ **Use Folders**: Keep templates organized by type
✓ **Naming Convention**: Follow DA_, BP_ prefixes for assets
✓ **Documentation**: Keep guides in Assets/ for easy access
✓ **Cleanup**: Remove test/temporary templates regularly

---

## Customization

### Extending Templates

To add new template types:

1. Open `TemplateGenerator.py`
2. Add a new `generate_*_template()` method
3. Follow existing patterns
4. Update the interactive menu
5. Add to command-line arguments

Example:
```python
def generate_weapon_template(self, name: str, weapon_type: str = "Laser"):
    # Implementation here
    pass
```

### Extending Guides

To add new guide types:

1. Open `GuideGenerator.py`
2. Add a new `generate_*_guide()` method
3. Create the template content
4. Update the interactive menu
5. Add to command-line arguments

### Custom Output Locations

Override default output directories:

```python
from TemplateGenerator import TemplateGenerator
from pathlib import Path

generator = TemplateGenerator(Path("/custom/project/root"))
generator.generate_spaceship_template("MyShip", "Fighter", 
                                     output_dir=Path("/custom/output"))
```

---

## Troubleshooting

### Problem: Scripts won't run

**Solution:**
- Ensure Python 3.6+ is installed
- Scripts have no external dependencies
- Check file permissions (chmod +x on Linux/Mac)

### Problem: Can't import in Unreal Editor

**Solution:**
- Verify Python plugin is enabled
- Ensure scripts are in project root directory
- Check Python console for error messages
- Restart Unreal Editor

### Problem: Templates appear in wrong location

**Solution:**
- Run from project root directory
- Or specify --output parameter
- Check that Assets/ folder exists

### Problem: Generated content looks wrong

**Solution:**
- Scripts generate *templates* - customize them!
- Review and edit generated files
- Templates include comments to guide customization

---

## Integration with Development Workflow

### For Designers

1. Use EditorUtilities.py in Unreal Editor
2. Generate templates as needed
3. Customize in text editor
4. Create Data Assets in editor
5. Reference in Blueprints

### For Programmers

1. Create new system in C++
2. Run GuideGenerator for documentation
3. Customize generated guides with actual API
4. Create example templates
5. Share with team

### For Documentation Team

1. Generate initial guides with GuideGenerator
2. Fill in system-specific details
3. Add screenshots and diagrams
4. Review and publish
5. Update when systems change

---

## File Structure

After using the generators, your project structure will look like:

```
Adastrea/
├── TemplateGenerator.py          # Template generation script
├── GuideGenerator.py              # Guide generation script
├── EditorUtilities.py             # Unified utilities for editor
├── Assets/
│   ├── SpaceshipTemplates/
│   │   ├── Fighter_*.yaml        # Generated ship templates
│   │   └── Scout_*.yaml
│   ├── PersonnelTemplates/
│   │   ├── Captain_*.yaml        # Generated personnel templates
│   │   └── Engineer_*.yaml
│   ├── TradingTemplates/
│   │   ├── TradeItem_*.yaml      # Generated trade templates
│   │   └── Market_*.yaml
│   ├── FactionAITemplates/
│   │   └── *.yaml                # Generated faction AI templates
│   ├── *SystemGuide.md           # Generated system guides
│   ├── *DesignerWorkflow.md      # Generated workflow guides
│   ├── *QuickReference.md        # Generated quick references
│   └── *APIReference.md          # Generated API references
└── Content/                       # Unreal Engine content
```

---

## Command Reference

### TemplateGenerator.py

```bash
# Help
python TemplateGenerator.py --help

# Spaceship
python TemplateGenerator.py --type spaceship --name NAME --class CLASS

# Personnel  
python TemplateGenerator.py --type personnel --name NAME --role ROLE

# Trade Item
python TemplateGenerator.py --type tradeitem --name NAME --category CATEGORY

# Market
python TemplateGenerator.py --type market --name NAME --market-type TYPE

# Contract
python TemplateGenerator.py --type contract --name NAME --contract-type TYPE

# Faction AI
python TemplateGenerator.py --type factionai --name NAME

# Material
python TemplateGenerator.py --type material --name NAME
```

### GuideGenerator.py

```bash
# Help
python GuideGenerator.py --help

# System Guide
python GuideGenerator.py --type system --name NAME

# Workflow Guide
python GuideGenerator.py --type workflow --name NAME

# Quick Reference
python GuideGenerator.py --type quickref --name NAME

# API Reference
python GuideGenerator.py --type api --name NAME
```

### EditorUtilities.py

```bash
# Interactive menu
python EditorUtilities.py --menu

# List templates
python EditorUtilities.py --list-templates

# List guides
python EditorUtilities.py --list-guides
```

---

## FAQ

**Q: Do I need to install any packages?**  
A: No! All scripts use only Python standard library.

**Q: Can I run these outside Unreal Editor?**  
A: Yes! TemplateGenerator.py and GuideGenerator.py work standalone.

**Q: What version of Python is required?**  
A: Python 3.6 or higher. Unreal Engine 5.5 includes Python 3.9+.

**Q: Can I customize the generated templates?**  
A: Absolutely! They're meant to be starting points - customize freely.

**Q: How do I add new template types?**  
A: Edit the generator scripts and add new methods following existing patterns.

**Q: Are templates ready to use?**  
A: They're ready to *customize*. Fill in specific values for your needs.

**Q: Can I run multiple generators at once?**  
A: Yes, each script can run independently.

**Q: Do changes to scripts affect existing templates?**  
A: No, generated files are independent. Regenerate to update.

---

## Additional Resources

### Related Documentation

- [SCRIPTS_README.md](SCRIPTS_README.md) - Setup validation scripts
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md) - Designer guide

### System Guides

All generated guides are stored in `Assets/`:
- Spaceship System Guide
- Personnel System Guide  
- Trading System Guide
- Faction System Guide
- And more...

### Example Templates

See `Assets/*/Templates/` for example templates:
- SpaceshipTemplates/
- PersonnelTemplates/
- TradingTemplates/
- FactionAITemplates/

---

## Contributing

When adding new template or guide types:

1. Follow existing code patterns
2. Include comprehensive docstrings
3. Add help text and examples
4. Test both interactive and batch modes
5. Update this README
6. Document in CONTRIBUTING.md

---

## License

These scripts are part of the Adastrea project and subject to the same license.

---

## Support

**Issues or Questions?**
- Check this README first
- Review existing templates for examples
- Ask in team Discord/Slack
- Submit issue on GitHub

---

**Last Updated**: 2025-11-07  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team

---

*Happy generating! These tools are designed to save you time and ensure consistency across the project. Customize freely and share your improvements with the team!*
