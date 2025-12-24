# Procedural Content Generation Guide

This guide explains how to use the procedural content generation system in Adastrea to automatically create galaxies, NPCs, quests, and ship variants.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Galaxy Generation](#galaxy-generation)
- [NPC Generation](#npc-generation)
- [Quest Generation](#quest-generation)
- [Ship Variant Generation](#ship-variant-generation)
- [Batch Generation](#batch-generation)
- [Unreal Editor Integration](#unreal-editor-integration)
- [Output Format](#output-format)
- [Customization](#customization)
- [Best Practices](#best-practices)

## Overview

The **ProceduralGenerators.py** script provides powerful tools for generating randomized game content that follows Adastrea's data asset conventions. All generated content is saved as YAML files that can be imported directly into Unreal Engine as Data Assets.

### What Can Be Generated

1. **Galaxy/Star Systems** - Complete star systems with:
   - Randomized star types and properties
   - 1-8 planets with varied characteristics
   - Asteroid belts with resources
   - Space stations
   - Faction presence and danger levels

2. **NPCs/Personnel** - Random crew members with:
   - Realistic names and biographies
   - Role-specific skills and experience
   - Personality traits and characteristics
   - Generated backstories

3. **Quests/Missions** - Procedural contracts with:
   - Various quest types (Delivery, Combat, Exploration, etc.)
   - Randomized objectives and locations
   - Difficulty-scaled rewards
   - Special conditions

4. **Ship Variants** - Randomized ship designs with:
   - Class-appropriate stat ranges
   - Unique names
   - Varied capabilities and equipment

## Quick Start

### Command Line Usage

```bash
# Generate 5 star systems
python ProceduralGenerators.py --type galaxy --count 5

# Generate 20 random NPCs
python ProceduralGenerators.py --type npc --count 20

# Generate 10 quests
python ProceduralGenerators.py --type quest --count 10

# Generate 8 ship variants
python ProceduralGenerators.py --type ship --count 8

# Generate everything at once
python ProceduralGenerators.py --type all
```

### Interactive Menu

Run the interactive menu for guided generation:

```bash
python ProceduralGenerators.py --menu
```

Or simply:

```bash
python ProceduralGenerators.py
```

This presents a user-friendly menu with all generation options.

## Galaxy Generation

### Basic Usage

```bash
# Generate 10 star systems
python ProceduralGenerators.py --type galaxy --count 10
```

### What Gets Generated

Each star system includes:

- **Star Properties**:
  - Type: Red Dwarf, Yellow Dwarf, Blue Giant, Red Giant, or White Dwarf
  - Mass and luminosity
  - 3D coordinates in galaxy space

- **Planets** (1-8 per system):
  - Planet type: Rocky, Gas Giant, Ice, Desert, Ocean, Lava, or Terran
  - Orbit distance and size
  - Gravity, temperature, atmosphere
  - Habitability rating (0-100)
  - Population (for habitable worlds)
  - Resource deposits
  - Moons

- **Asteroid Belts** (0-3 per system):
  - Density (Sparse, Moderate, Dense)
  - Resource types
  - Orbit distance

- **Space Stations** (0-2 per system):
  - Station type: Trading Hub, Mining Station, Research Facility, Military Outpost
  - Population
  - Orbit location

- **System Info**:
  - Controlling factions
  - Danger level: Safe, Low, Moderate, High, Extreme
  - Traffic level: None, Light, Moderate, Heavy
  - Jump gate presence

### Example Output

```yaml
SystemName: "Nova Proxima"
SystemID: "NovaProxima"

StarType: "Yellow Dwarf"
StarMass: 1.2  # Solar masses
StarLuminosity: 18.5  # Solar luminosity

Coordinates:
  X: 2500
  Y: 1800
  Z: 150

Planets:
  - Name: "Terra Prime"
    Type: "Terran"
    Habitability: 85
    Population: 500000
    Resources:
      - "Water"
      - "Minerals"
      - "Organics"
```

### Use Cases

- **Procedural Universe**: Generate hundreds of systems for exploration
- **Quick Prototyping**: Create test environments rapidly
- **Content Variety**: Ensure diverse systems without manual creation
- **Modding Support**: Easy content creation for mod makers

## NPC Generation

### Basic Usage

```bash
# Generate 20 random NPCs
python ProceduralGenerators.py --type npc --count 20

# Generate 5 Captains
python ProceduralGenerators.py --type npc --count 5 --role Captain

# Generate 10 Engineers
python ProceduralGenerators.py --type npc --count 10 --role Engineer
```

### Available Roles

- Captain
- Pilot
- Navigator
- Engineer
- Gunner
- Trader
- Scientist
- MedicalOfficer
- SecurityOfficer
- CargoSpecialist
- Communications
- SensorOperator

### What Gets Generated

Each NPC includes:

- **Identity**:
  - Realistic first and last name
  - Age (25-65)
  - Gender
  - Species (currently Human, expandable)
  - Nationality/Origin

- **Biography**:
  - Procedurally generated backstory
  - Career path
  - Notable achievements
  - Personality description

- **Skills**:
  - Role-appropriate primary skills
  - Skill levels (1-10)
  - Experience points per skill

- **Status**:
  - Morale, Health, Fatigue
  - Loyalty and Reputation
  - Salary requirements

- **Personality**:
  - Personality type (Leader, Analytical, Adventurous, etc.)
  - 1-3 random traits with gameplay modifiers
  - Examples: Natural Leader (+20% morale), Combat Veteran (+15% tactical)

### Example Output

```yaml
PersonnelID: "Captain_JohnSmith"
PersonnelName: "John Smith"

Biography: |
  This individual started as a cargo hauler and worked their way 
  up through the ranks. They are known for their calm demeanor 
  under pressure and successfully completed 45 deep space missions.

Age: 45
Gender: "Male"
PrimaryRole: "Captain"

Skills:
  - SkillName: "Leadership"
    SkillLevel: 10
    ExperiencePoints: 5000
  - SkillName: "Tactical"
    SkillLevel: 9
    ExperiencePoints: 4200
```

### Use Cases

- **Crew Roster**: Quickly populate ships with varied crew
- **NPC Variety**: Create diverse populations for stations
- **Recruitment Pool**: Generate candidates for hiring
- **Quest Givers**: Create characters for missions

## Quest Generation

### Basic Usage

```bash
# Generate 15 random quests
python ProceduralGenerators.py --type quest --count 15

# Generate 5 Combat missions
python ProceduralGenerators.py --type quest --count 5 --quest-type Combat

# Generate Delivery contracts
python ProceduralGenerators.py --type quest --count 10 --quest-type Delivery
```

### Quest Types

- **Delivery**: Transport cargo between locations
- **Procurement**: Acquire and deliver specific goods
- **Escort**: Protect convoy or VIP
- **Combat**: Eliminate hostile forces
- **Exploration**: Survey uncharted systems
- **Rescue**: Emergency response missions
- **Investigation**: Research and intelligence gathering
- **Smuggling**: Covert delivery missions
- **Mining**: Resource extraction contracts

### Difficulty Levels

- **Easy**: Low risk, moderate rewards
- **Medium**: Balanced risk/reward
- **Hard**: High risk, good rewards
- **VeryHard**: Extreme risk, excellent rewards

Difficulty automatically scales:
- Reward credits
- Reputation gains
- Experience points
- Time limits

### What Gets Generated

Each quest includes:

- **Basic Info**:
  - Quest name appropriate to type
  - Unique ID
  - Procedurally generated description
  - Difficulty rating

- **Locations**:
  - Origin and destination star systems
  - Distance calculation
  - Coordinates

- **Requirements**:
  - Minimum reputation needed
  - Time limit (in hours)
  - Special conditions (stealth, combat expected)

- **Rewards**:
  - Credits (scaled by difficulty)
  - Reputation gain
  - Experience points
  - Bonus items (optional)

- **Penalties**:
  - Failure credit penalty
  - Reputation loss
  - Collateral deposit

### Example Output

```yaml
ContractName: "Urgent Delivery"
ContractID: "Quest_Delivery_4521"
ContractType: Delivery
Difficulty: Hard

OriginLocation:
  LocationName: "Alpha Centauri"
  Coordinates: [1500, 2300, 450]

DestinationLocation:
  LocationName: "Beta Prime"
  Coordinates: [8500, 4200, 780]

Distance: 8542.5
TimeLimit: 48

Rewards:
  Credits: 15000
  ReputationGain: 30
  ExperiencePoints: 450

bRepeatable: true
bCombatExpected: false
```

### Use Cases

- **Dynamic Content**: Create endless mission variety
- **Quest Boards**: Populate mission terminals
- **AI Trading**: Contracts for autonomous traders
- **Event Generation**: Random encounters and opportunities

## Ship Variant Generation

### Basic Usage

```bash
# Generate 5 Fighter variants
python ProceduralGenerators.py --type ship --count 5 --ship-class Fighter

# Generate Scout variants
python ProceduralGenerators.py --type ship --count 3 --ship-class Scout

# Generate Trader variants
python ProceduralGenerators.py --type ship --count 4 --ship-class Trader
```

### Ship Classes

Each class has appropriate stat ranges:

- **Fighter**: Fast, agile, combat-focused
  - Hull: 800-1200
  - Speed: 600-900
  - Weapons: 2-4 slots

- **Scout**: Very fast, excellent sensors
  - Hull: 600-1000
  - Speed: 800-1100
  - Weapons: 1-2 slots

- **Trader**: High cargo, moderate speed
  - Hull: 1000-1500
  - Cargo: 500-2000
  - Weapons: 0-2 slots

- **Gunship**: Heavy weapons, armored
  - Hull: 1200-1800
  - Weapons: 4-8 slots
  - Speed: 500-700

- **Transport**: Maximum cargo capacity
  - Hull: 1500-2500
  - Cargo: 1000-5000
  - Weapons: 0-1 slots

### What Gets Generated

Each ship variant includes:

- **Unique Name**: Procedurally generated (e.g., "Thunder Striker", "Silent Voyager")
- **Base Stats**: All stats within class-appropriate ranges
- **Combat Stats**: Armor, shields, weapon slots
- **Mobility**: Speed, acceleration, maneuverability
- **Utility**: Sensors, stealth, science ratings
- **Operational**: Power, life support, crew capacity
- **Lore**: Manufacturer, year built, rarity tier
- **Colors**: Randomized primary and secondary colors

### Example Output

```yaml
BasicInfo:
  ShipName: "Crimson Pathfinder"
  ShipClass: "Scout"
  ShipID: "CrimsonPathfinder"

CoreStats:
  HullStrength: 875.3
  CargoCapacity: 68.2
  MaxSpeed: 1045.8
  
CombatStats:
  WeaponSlots: 2
  ArmorRating: 35.4
  ShieldStrength: 512.7

Lore:
  Manufacturer: "Vanguard Aerospace"
  RarityTier: "Uncommon"
```

### Use Cases

- **Enemy Variety**: Generate unique pirate/NPC ships
- **Loot Rewards**: Create rare ship variants as rewards
- **Market Inventory**: Stock ship dealers
- **Fleet Diversity**: Varied ships for factions

## Batch Generation

Generate a complete content pack with one command:

```bash
python ProceduralGenerators.py --type all
```

This generates:
- 5 star systems
- 20 NPCs
- 15 quests
- 8 ship variants

Perfect for:
- New game sectors
- Expansion content
- Test environments
- Mod packs

## Unreal Editor Integration

### Method 1: Import Generated YAML Files

1. Generate content with ProceduralGenerators.py
2. Use YAMLtoDataAsset.py to import:

```python
import YAMLtoDataAsset

# Import a generated NPC
YAMLtoDataAsset.import_personnel_yaml(
    "Assets/ProceduralContent/Personnel/Captain_JohnSmith.yaml"
)

# Batch import all generated NPCs
YAMLtoDataAsset.batch_import_personnel()
```

### Method 2: Direct Python Execution in Editor

1. Enable "Python Editor Script Plugin"
2. Open Python Console (Tools → Python)
3. Run directly:

```python
import ProceduralGenerators

generator = ProceduralGenerators.ProceduralGenerators()

# Generate and save content
generator.generate_galaxy(num_systems=5)
generator.generate_npcs(count=20)
generator.generate_quests(count=10)
```

### Method 3: Editor Script Execution

1. Tools → Python → Execute Python Script
2. Select ProceduralGenerators.py
3. Content is generated immediately

## Output Format

All generated content is saved to:

```
Assets/ProceduralContent/
├── Galaxy/          # Star system YAML files
├── Personnel/       # NPC YAML files
├── Quests/          # Quest/mission YAML files
└── Ships/           # Ship variant YAML files
```

### File Naming Conventions

- **Galaxy**: `System_{SystemID}.yaml`
- **Personnel**: `{Role}_{PersonnelID}.yaml`
- **Quests**: `Quest_{Type}_{ID}.yaml`
- **Ships**: `{Class}_{ShipID}.yaml`

All files follow the same YAML structure as hand-crafted templates and can be:
- Edited manually
- Imported to Unreal Engine
- Version controlled
- Shared with team members

## Customization

### Extending Name Generation

Edit the `NameGenerator` class in ProceduralGenerators.py:

```python
class NameGenerator:
    # Add more first names
    FIRST_NAMES_MALE += ["Custom", "Names", "Here"]
    
    # Add custom planet prefixes
    PLANET_PREFIXES += ["Mythos", "Legend", "Ancient"]
```

### Adding New Ship Classes

Add to the `generate_ship_variant` method:

```python
class_stats = {
    # ... existing classes ...
    "Corvette": {
        "HullStrength": (900, 1400),
        "CargoCapacity": (200, 400),
        "MaxSpeed": (650, 850),
        "WeaponSlots": (3, 5)
    }
}
```

### Custom Quest Types

Add to quest generation:

```python
quest_types = [
    # ... existing types ...
    "Diplomacy", "Sabotage", "Espionage"
]
```

Then add corresponding name templates and descriptions.

### Adjusting Randomness

Modify probability distributions:

```python
# More rare traits
num_traits = random.randint(1, 2)  # Instead of 1-3

# Safer systems
danger_level = random.choice(["Safe", "Safe", "Low", "Moderate"])

# More habitable planets
if planet_type == "Terran":
    habitability = random.randint(70, 95)  # Higher minimum
```

## Best Practices

### Generation Strategy

✓ **Generate in Batches**: Create content in groups (10-20 items)
✓ **Review Generated Content**: Check a few samples before mass import
✓ **Mix Procedural and Manual**: Combine generated content with hand-crafted
✓ **Use Filters**: Generate specific types when needed
✓ **Version Control**: Commit generated YAML files

### Quality Control

✓ **Spot Check**: Review random samples from each batch
✓ **Balance Testing**: Verify stat distributions are appropriate
✓ **Name Review**: Check for inappropriate name combinations
✓ **Lore Consistency**: Edit biographies for narrative coherence

### Performance

✓ **Generate Offline**: Create content outside of editor when possible
✓ **Batch Import**: Use batch functions for importing to Unreal
✓ **Cleanup**: Remove test/unused generated content regularly
✓ **Organize**: Keep generated content in separate folders

### Integration

✓ **Template First**: Review existing templates before generating
✓ **Follow Conventions**: Generated content matches manual templates
✓ **Test Import**: Import one file first to verify compatibility
✓ **Document Changes**: Note any customizations to generator

## Common Workflows

### Creating a New Galaxy Sector

```bash
# 1. Generate star systems
python ProceduralGenerators.py --type galaxy --count 20

# 2. Review and edit systems in Assets/ProceduralContent/Galaxy/

# 3. Generate stations and NPCs for populated systems
python ProceduralGenerators.py --type npc --count 50

# 4. Create missions for the sector
python ProceduralGenerators.py --type quest --count 30

# 5. Import to Unreal Editor
# (Use YAMLtoDataAsset.py or manual import)
```

### Populating a Space Station

```bash
# Generate varied crew
python ProceduralGenerators.py --type npc --count 20

# Edit generated NPCs to assign to station

# Generate station-specific quests
python ProceduralGenerators.py --type quest --count 10
```

### Creating Enemy Fleet Variants

```bash
# Generate fighter variants
python ProceduralGenerators.py --type ship --count 5 --ship-class Fighter

# Generate gunship variants
python ProceduralGenerators.py --type ship --count 3 --ship-class Gunship

# Assign to pirate factions
```

## Troubleshooting

### Issue: Generated content doesn't import

**Solution**: Verify YAML syntax is valid. Run a YAML validator on generated files.

### Issue: Names seem repetitive

**Solution**: Expand name lists in NameGenerator class or add middle initials/titles.

### Issue: Stats seem unbalanced

**Solution**: Adjust ranges in generation methods. Review difficulty multipliers.

### Issue: Biographies are generic

**Solution**: Add more template variations in BiographyGenerator class.

### Issue: Too much/too little content

**Solution**: Adjust count parameters. Use batch generation for large sets.

## Advanced Usage

### Programmatic Generation

Use the generator as a library:

```python
from ProceduralGenerators import ProceduralGenerators, NameGenerator

generator = ProceduralGenerators()

# Generate specific content
for i in range(100):
    system = generator.generate_star_system()
    # Process or filter system
    if system['DangerLevel'] == 'Safe':
        generator.save_star_system(system)
```

### Custom Filtering

Generate content with specific criteria:

```python
# Only generate captains with high loyalty
while len(captains) < 10:
    npc = generator.generate_npc(role="Captain")
    if npc['Loyalty'] >= 80:
        generator.save_npc(npc)
        captains.append(npc)
```

### Integration with Other Tools

Combine with existing Adastrea scripts:

```python
# Generate content
generator.generate_galaxy(num_systems=10)

# Validate with SchemaValidator
from SchemaValidator import validate_yaml_files

# Import with YAMLtoDataAsset
from YAMLtoDataAsset import batch_import_all
```

## API Reference

### ProceduralGenerators Class

#### `generate_star_system(system_name=None, num_planets=None)`
Generate a single star system.

#### `save_star_system(system_data)`
Save star system to YAML file.

#### `generate_galaxy(galaxy_name, num_systems=10)`
Generate multiple star systems.

#### `generate_npc(role=None, gender=None)`
Generate a random NPC.

#### `save_npc(npc_data)`
Save NPC to YAML file.

#### `generate_npcs(count=10, role=None)`
Generate multiple NPCs.

#### `generate_quest(quest_type=None)`
Generate a quest/mission.

#### `save_quest(quest_data)`
Save quest to YAML file.

#### `generate_quests(count=10, quest_type=None)`
Generate multiple quests.

#### `generate_ship_variant(base_class="Fighter")`
Generate a ship variant.

#### `save_ship_variant(ship_data)`
Save ship to YAML file.

#### `generate_ship_variants(count=5, ship_class=None)`
Generate multiple ship variants.

### NameGenerator Class

#### `generate_person_name(gender=None)`
Generate realistic person name.

#### `generate_star_name()`
Generate star system name.

#### `generate_planet_name()`
Generate planet name.

#### `generate_ship_name()`
Generate ship name.

### BiographyGenerator Class

#### `generate_biography(role, age)`
Generate character backstory.

## See Also

- [TemplateGenerator.py](TemplateGenerator.py) - Manual template creation
- [YAMLtoDataAsset.py](YAMLtoDataAsset.py) - Import generated content
- [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md) - Generator overview
- [YAML_IMPORT_GUIDE.md](YAML_IMPORT_GUIDE.md) - Import documentation

## Contributing

To improve procedural generation:

1. Expand name lists in NameGenerator
2. Add new quest types and templates
3. Improve biography generation
4. Add new ship classes
5. Enhance planet type variety
6. Add faction-specific generation

Submit improvements via pull requests!

---

**Version**: 1.0  
**Last Updated**: 2025-11-07  
**Maintained by**: Adastrea Development Team
