# Procedural Generation Quick Start

Get started with procedural content generation in under 5 minutes!

## What You Can Generate

- 🌌 **Galaxy Systems** - Star systems with planets, asteroids, and stations
- 👥 **NPCs** - Crew members with names, skills, and backstories
- 📜 **Quests** - Missions with objectives, rewards, and locations
- 🚀 **Ship Variants** - Randomized ship designs with unique stats

## Installation

**No installation needed!** ProceduralGenerators.py uses only Python standard library.

Requirements:
- Python 3.6 or higher (included with Unreal Engine 5.x)

## Quick Commands

### Generate Everything at Once

```bash
python ProceduralGenerators.py --type all
```

This creates:
- 5 star systems
- 20 NPCs
- 15 quests
- 8 ship variants

Perfect for testing or creating a new game sector!

### Generate Specific Content

```bash
# 10 star systems
python ProceduralGenerators.py --type galaxy --count 10

# 25 random NPCs
python ProceduralGenerators.py --type npc --count 25

# 20 quests
python ProceduralGenerators.py --type quest --count 20

# 5 fighter variants
python ProceduralGenerators.py --type ship --count 5
```

### Interactive Mode

For a guided experience:

```bash
python ProceduralGenerators.py
```

This launches an interactive menu where you can select what to generate.

## Where Files Are Created

All generated content goes to:

```
Assets/ProceduralContent/
├── Galaxy/          # Star systems
├── Personnel/       # NPCs
├── Quests/          # Missions
└── Ships/           # Ship variants
```

## Import to Unreal Engine

After generating YAML files, import them to Unreal:

```python
# In Unreal Editor Python Console
import YAMLtoDataAsset

# Import all generated NPCs
YAMLtoDataAsset.batch_import_personnel()

# Or import specific files
YAMLtoDataAsset.import_personnel_yaml(
    "Assets/ProceduralContent/Personnel/Captain_JohnSmith.yaml"
)
```

## Example Workflows

### Create a New Star System

```bash
# 1. Generate the system
python ProceduralGenerators.py --type galaxy --count 1

# 2. Generate NPCs for the system
python ProceduralGenerators.py --type npc --count 10

# 3. Generate quests in the system
python ProceduralGenerators.py --type quest --count 5

# 4. Files are in Assets/ProceduralContent/
# 5. Import to Unreal with YAMLtoDataAsset.py
```

### Populate a New Faction

```bash
# Generate crew members
python ProceduralGenerators.py --type npc --count 30

# Generate their ships
python ProceduralGenerators.py --type ship --count 10

# Generate faction missions
python ProceduralGenerators.py --type quest --count 20
```

### Create Enemy Fleet Variants

```bash
# Generate varied fighters
python ProceduralGenerators.py --type ship --count 5 --ship-class Fighter

# Generate gunships
python ProceduralGenerators.py --type ship --count 3 --ship-class Gunship

# Use in AI spawning for variety
```

## Advanced Options

### Generate Specific Roles

```bash
# Only generate captains
python ProceduralGenerators.py --type npc --count 5 --role Captain

# Only generate engineers
python ProceduralGenerators.py --type npc --count 10 --role Engineer
```

### Generate Specific Quest Types

```bash
# Combat missions only
python ProceduralGenerators.py --type quest --count 10 --quest-type Combat

# Delivery contracts only
python ProceduralGenerators.py --type quest --count 15 --quest-type Delivery
```

### Generate Specific Ship Classes

```bash
# Scout ships
python ProceduralGenerators.py --type ship --count 5 --ship-class Scout

# Traders
python ProceduralGenerators.py --type ship --count 3 --ship-class Trader

# Gunships
python ProceduralGenerators.py --type ship --count 4 --ship-class Gunship
```

## Tips & Tricks

✓ **Review Before Importing**: Check a few generated files first
✓ **Edit Generated Content**: YAML files can be manually tweaked
✓ **Mix Procedural and Manual**: Combine with hand-crafted content
✓ **Use Version Control**: Commit useful generated content
✓ **Batch Import**: Use YAMLtoDataAsset.py for bulk importing

## Common Use Cases

| Task | Command |
|------|---------|
| Populate universe | `--type all` |
| Fill crew roster | `--type npc --count 20` |
| Create mission board | `--type quest --count 30` |
| Generate enemy variants | `--type ship --count 10` |
| Quick test environment | `--type all` |

## Troubleshooting

**Problem**: No output files  
**Solution**: Check that Assets/ProceduralContent/ folder exists (auto-created)

**Problem**: Import fails  
**Solution**: Verify YAML syntax, ensure YAMLtoDataAsset.py is available

**Problem**: Duplicate names  
**Solution**: Generator uses randomization - run again or edit names manually

**Problem**: Stats seem wrong  
**Solution**: Review generated files and adjust manually if needed

## Next Steps

1. **Generate Test Content**: Run `--type all` to try everything
2. **Review Output**: Check files in Assets/ProceduralContent/
3. **Import to Unreal**: Use YAMLtoDataAsset.py
4. **Customize**: Edit generated YAML files as needed
5. **Read Full Guide**: See [PROCEDURAL_GENERATION_GUIDE.md](PROCEDURAL_GENERATION_GUIDE.md)

## See Also

- [PROCEDURAL_GENERATION_GUIDE.md](PROCEDURAL_GENERATION_GUIDE.md) - Complete documentation
- [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md) - All generator tools
- [YAML_IMPORT_GUIDE.md](YAML_IMPORT_GUIDE.md) - Import instructions

---

**Ready to generate content?** Run `python ProceduralGenerators.py` to get started!
