# Procedural Content Examples

This document provides practical examples of using the procedural content generation system.

## Example 1: Create a New Galaxy Sector

Generate a complete galaxy sector with systems, NPCs, and missions:

```bash
# Step 1: Generate star systems
python ProceduralGenerators.py --type galaxy --count 15

# Step 2: Generate NPCs to populate the sector
python ProceduralGenerators.py --type npc --count 50

# Step 3: Generate missions for players
python ProceduralGenerators.py --type quest --count 30

# Step 4: Generate ship variants for variety
python ProceduralGenerators.py --type ship --count 12
```

**Result**: 107 YAML files ready to import into Unreal Engine!

## Example 2: Populate a Specific Faction

Create content for a trading faction:

```bash
# Generate trader NPCs
python ProceduralGenerators.py --type npc --count 15 --role Trader

# Generate trading ships
python ProceduralGenerators.py --type ship --count 8 --ship-class Trader

# Generate trade missions
python ProceduralGenerators.py --type quest --count 20 --quest-type Delivery
```

## Example 3: Create Enemy Fleet Varieties

Generate diverse enemy ships:

```bash
# Pirate fighters
python ProceduralGenerators.py --type ship --count 10 --ship-class Fighter

# Pirate gunships
python ProceduralGenerators.py --type ship --count 5 --ship-class Gunship

# Scout ships for recon
python ProceduralGenerators.py --type ship --count 3 --ship-class Scout
```

Each variant will have unique stats, names, and colors!

## Example 4: Quick Test Environment

Generate everything you need for testing:

```bash
python ProceduralGenerators.py --type all
```

This creates:
- 5 star systems (each with 1-8 planets, stations, asteroids)
- 20 NPCs (varied roles and personalities)
- 15 quests (multiple types and difficulties)
- 8 ship variants (ready for use)

## Example 5: Create a Story-Rich System

Generate a single detailed system and customize it:

```bash
# Generate one system
python ProceduralGenerators.py --type galaxy --count 1

# Review the generated file in Assets/ProceduralContent/Galaxy/
# Example: System_AlphaPrime.yaml

# Manually edit to add story elements:
# - Rename planets to fit your narrative
# - Adjust faction presence
# - Set danger/traffic levels
# - Add lore notes

# Generate NPCs for key roles
python ProceduralGenerators.py --type npc --count 5 --role Captain
python ProceduralGenerators.py --type npc --count 3 --role Scientist

# Edit NPC biographies to fit your story
```

## Example 6: Create Mission Board Content

Generate varied missions for a station:

```bash
# Combat missions
python ProceduralGenerators.py --type quest --count 5 --quest-type Combat

# Delivery missions
python ProceduralGenerators.py --type quest --count 8 --quest-type Delivery

# Exploration missions
python ProceduralGenerators.py --type quest --count 4 --quest-type Exploration

# Rescue missions
python ProceduralGenerators.py --type quest --count 3 --quest-type Rescue
```

Result: 20 varied missions with appropriate rewards and difficulty!

## Example 7: Import Generated Content

After generating, import to Unreal Engine:

```python
# In Unreal Editor Python Console
import YAMLtoDataAsset

# Import all generated personnel
files = [
    "Assets/ProceduralContent/Personnel/Captain_JohnSmith.yaml",
    "Assets/ProceduralContent/Personnel/Engineer_SarahJones.yaml",
    # ... etc
]

for file in files:
    YAMLtoDataAsset.import_personnel_yaml(file)

# Or batch import
YAMLtoDataAsset.batch_import_personnel()
```

## Example 8: Customize Generated Content

Generated files are editable! Example workflow:

```bash
# 1. Generate initial content
python ProceduralGenerators.py --type npc --count 10 --role Captain

# 2. Review in Assets/ProceduralContent/Personnel/

# 3. Edit a file (Captain_JohnSmith.yaml):
#    - Change name to match your story
#    - Adjust skills for balance
#    - Add custom biography
#    - Set specific loyalty/reputation values

# 4. Import the customized file
# (In Unreal Editor)
import YAMLtoDataAsset
YAMLtoDataAsset.import_personnel_yaml(
    "Assets/ProceduralContent/Personnel/Captain_JohnSmith.yaml"
)
```

## Example 9: Generate Progression Content

Create content for different game stages:

```bash
# Early game - easy content
python ProceduralGenerators.py --type quest --count 10 --quest-type Delivery
python ProceduralGenerators.py --type ship --count 5 --ship-class Scout

# Edit generated quests to set difficulty="Easy"
# Use for starting area

# Mid game - moderate challenge
python ProceduralGenerators.py --type quest --count 15 --quest-type Combat

# Edit to set difficulty="Medium" or "Hard"
# Use for intermediate zones

# Late game - high difficulty
python ProceduralGenerators.py --type quest --count 8 --quest-type Exploration

# Edit to set difficulty="VeryHard"
# Use for end-game areas
```

## Example 10: Create Faction-Specific Systems

Generate systems for different factions:

```bash
# Generate 3 systems
python ProceduralGenerators.py --type galaxy --count 3

# Manually edit each system file:

# System 1 - Trading faction
# - Set Factions: ["Free Traders", "Merchant Guild"]
# - Set TrafficLevel: "Heavy"
# - Set DangerLevel: "Low"

# System 2 - Military faction
# - Set Factions: ["United Earth", "Defense Force"]
# - Set TrafficLevel: "Moderate"
# - Set DangerLevel: "High"

# System 3 - Pirate faction
# - Set Factions: ["Pirates", "Outlaws"]
# - Set TrafficLevel: "Light"
# - Set DangerLevel: "Extreme"
```

## Example Generated Files

### Star System Example

```yaml
SystemName: "Alpha Centauri"
StarType: "Yellow Dwarf"
Planets:
  - Name: "New Earth"
    Type: "Terran"
    Habitability: 85
    Population: 500000
  - Name: "Mining Colony Beta"
    Type: "Rocky"
    Habitability: 20
    Population: 5000
Stations:
  - Name: "Alpha Station"
    Type: "Trading Hub"
    Population: 2500
```

### NPC Example

```yaml
PersonnelID: "Captain_JohnSmith"
PersonnelName: "John Smith"
Age: 45
PrimaryRole: "Captain"
Biography: |
  A veteran spacer with 20 years experience,
  known for calm leadership under pressure.
Skills:
  - SkillName: "Leadership"
    SkillLevel: 10
  - SkillName: "Tactical"
    SkillLevel: 9
```

### Quest Example

```yaml
ContractName: "Urgent Delivery"
ContractType: Delivery
Difficulty: Hard
Distance: 8542.5
TimeLimit: 48
Rewards:
  Credits: 15000
  ReputationGain: 30
```

### Ship Example

```yaml
ShipName: "Thunder Striker"
ShipClass: "Fighter"
CoreStats:
  HullStrength: 1050.0
  MaxSpeed: 850.0
  WeaponSlots: 3
```

## Integration Tips

### Filtering Generated Content

Review generated files and:
- ✓ Keep high-quality/interesting items
- ✓ Delete duplicates or low-quality items
- ✓ Edit to fit your game's balance
- ✓ Organize by faction/location/purpose

### Balancing

Generated stats are randomized within ranges:
- Review combat ratings
- Adjust rewards vs. difficulty
- Check NPC skill distributions
- Verify ship stat balance

### Customization

All generated files can be edited:
- Add lore and story elements
- Adjust numerical values
- Link to existing factions
- Add custom tags

### Version Control

- Commit useful generated content
- Create separate branches for experiments
- Tag releases with content packs
- Share with team members

## Performance Considerations

### Large Batches

When generating 100+ items:
```bash
# Generate in smaller batches
for i in {1..5}; do
    python ProceduralGenerators.py --type npc --count 20
done
```

### Import Performance

Import in batches of 20-30 files at a time to avoid editor slowdowns.

### Organization

Create subdirectories:
```
Assets/ProceduralContent/
├── Alpha_Sector/
│   ├── Galaxy/
│   ├── Personnel/
│   └── Quests/
├── Beta_Sector/
│   ├── Galaxy/
│   ├── Personnel/
│   └── Quests/
```

## Common Patterns

### Pattern: Progressive Difficulty

```bash
# Zone 1: Safe starter area
python ProceduralGenerators.py --type quest --count 10
# Edit to set all difficulty="Easy"

# Zone 2: Moderate challenge
python ProceduralGenerators.py --type quest --count 15
# Edit to set difficulty="Medium"

# Zone 3: End game
python ProceduralGenerators.py --type quest --count 8
# Edit to set difficulty="VeryHard"
```

### Pattern: Faction Content Pack

```bash
# For each faction:
python ProceduralGenerators.py --type galaxy --count 3
python ProceduralGenerators.py --type npc --count 15
python ProceduralGenerators.py --type quest --count 20
python ProceduralGenerators.py --type ship --count 5

# Edit all files to reference the faction
```

### Pattern: Story Arc

```bash
# Generate quest chain
python ProceduralGenerators.py --type quest --count 5 --quest-type Delivery

# Edit files to create progression:
# Quest 1: Simple delivery (Easy)
# Quest 2: Longer delivery (Medium)
# Quest 3: Delivery through danger zone (Hard)
# Quest 4: VIP delivery (Hard)
# Quest 5: Critical delivery (VeryHard)
```

## See Also

- [PROCEDURAL_GENERATION_GUIDE.md](PROCEDURAL_GENERATION_GUIDE.md) - Complete documentation
- [PROCEDURAL_GENERATION_QUICKSTART.md](PROCEDURAL_GENERATION_QUICKSTART.md) - Quick start guide
- [YAML_IMPORT_GUIDE.md](YAML_IMPORT_GUIDE.md) - Import instructions
- [TemplateGenerator.py](TemplateGenerator.py) - Manual template creation

---

**Ready to generate?** Start with `python ProceduralGenerators.py --type all` for a complete test!
