# Quick Start Guide: Template and Guide Generator Scripts

This guide will get you started using the new template and guide generator scripts in 2 minutes!

## What These Scripts Do

- **Generate YAML templates** for spaceships, personnel, trading items, markets, contracts, factions, and materials
- **Generate Markdown guides** for system documentation, workflows, quick references, and API docs
- **Work anywhere**: Command line OR Unreal Editor

## Quick Examples

### 1. Generate a Spaceship Template

```bash
python TemplateGenerator.py --type spaceship --name "Viper" --class Fighter
```

This creates `Assets/SpaceshipTemplates/Fighter_Viper.yaml` ready to customize!

### 2. Generate a Character Template

```bash
python TemplateGenerator.py --type personnel --name "Captain Smith" --role Captain
```

This creates `Assets/PersonnelTemplates/Captain_CaptainSmith.yaml` with all fields!

### 3. Generate System Documentation

```bash
python GuideGenerator.py --type system --name "MyNewSystem"
```

This creates a complete system guide in `Assets/MyNewSystemSystemGuide.md`!

### 4. Interactive Mode (Easiest!)

```bash
python EditorUtilities.py --menu
```

This shows a friendly menu with all options!

## Using in Unreal Editor

### Step 1: Enable Python Plugin
1. Open Unreal Editor
2. Edit → Plugins
3. Search "Python Editor Script Plugin"
4. Enable and restart

### Step 2: Run the Script
1. Tools → Python → Execute Python Script
2. Navigate to project root
3. Select `EditorUtilities.py`
4. Choose from the menu!

### Or Use Python Console
```python
import EditorUtilities
EditorUtilities.show_menu()

# Or call directly:
EditorUtilities.generate_spaceship("MyShip", "Fighter")
EditorUtilities.generate_system_guide("MySystem")
```

## Common Use Cases

### Creating a New Ship Type
```bash
# 1. Generate template
python TemplateGenerator.py --type spaceship --name "Interceptor" --class Fighter

# 2. Edit Assets/SpaceshipTemplates/Fighter_Interceptor.yaml
# 3. Customize the values
# 4. Create Data Asset in Unreal Editor from template
```

### Documenting a New System
```bash
# Generate all documentation at once
python EditorUtilities.py
# Select option 14, enter system name
```

### Quick Template Creation
```bash
# Interactive mode is fastest for one-offs
python TemplateGenerator.py
# Answer the prompts!
```

## What Gets Generated?

### Templates (YAML)
- Pre-filled with all required fields
- Inline comments explaining each property
- Recommended value ranges
- Ready to customize

### Guides (Markdown)
- Complete documentation structure
- Examples and code snippets
- Best practices
- Troubleshooting sections

## Need More Help?

- **Full Documentation**: [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md)
- **Command Help**: `python TemplateGenerator.py --help`
- **Script Overview**: [SCRIPTS_README.md](SCRIPTS_README.md)

## Tips

✓ Use descriptive names for templates  
✓ Customize templates immediately after generation  
✓ Generate documentation when creating new systems  
✓ Use interactive mode for quick one-offs  
✓ Use batch mode for automation  

That's it! Start generating templates and guides in seconds! 🚀
