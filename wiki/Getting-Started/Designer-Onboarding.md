# Designer Onboarding Guide

Welcome to Adastrea! This guide is designed for designers, artists, and content creators who want to work with the game without needing extensive C++ programming knowledge.

## What You'll Learn

- How to navigate the Adastrea project
- Creating game content using Data Assets
- Working with Blueprints
- Using automation tools
- Where to find help

## Prerequisites

✅ Complete the [Installation Guide](Installation.md) first

## Your First Day

### Hour 1: Understand the Project Structure

The Adastrea project uses a **data-driven design** where most game content is created through **Data Assets** rather than code:

```
Content/
├── DataAssets/          ← Your main workspace
│   ├── Ships/          ← Spaceship configurations
│   ├── Factions/       ← Faction definitions
│   ├── Personnel/      ← Crew members
│   ├── Trading/        ← Trade items and markets
│   └── ...
├── Blueprints/         ← Visual scripting
└── Maps/               ← Game levels
```

**Key Concept**: You create game content by making Data Assets and Blueprints, not by writing C++ code!

### Hour 2: Create Your First Data Asset

Let's create a simple spaceship:

1. **Open Content Browser** in Unreal Editor
2. Navigate to `Content/DataAssets/Ships/`
3. Right-click > Blueprint Class > Search "SpaceshipDataAsset"
4. Name it `DA_MyFirstShip`
5. Double-click to open
6. Fill in the properties:
   - Display Name: "Explorer Scout"
   - Description: "My first custom ship"
   - Set some stats (hull, speed, cargo, etc.)
7. Save (Ctrl+S)

🎉 **Congratulations!** You just created game content!

### Hour 3: Explore Templates

Adastrea includes YAML templates that show you example configurations:

**Location**: `Assets/SpaceshipTemplates/`

**Popular Templates**:
- `Pathfinder_Scout.yaml` - Fast reconnaissance ship
- `Warhammer_Gunship.yaml` - Combat-focused ship
- `Merchant_Trader.yaml` - Cargo hauler

**How to Use**:
1. Open a YAML template in a text editor
2. See the stat values and descriptions
3. Copy those values into your Data Assets
4. Customize as needed

### Hour 4: Learn Blueprint Basics

Blueprints are visual scripts that don't require coding:

**Start Here**: [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)

**Essential Blueprints for Designers**:
- Ship Blueprints (`BP_Spaceship_*`)
- UI Widgets (`WBP_*`)
- Game Mode (`BP_SpaceGameMode`)

## Week 1: Core Systems

### Day 1-2: Spaceship System
**Goal**: Create 3 different ship types

**Resources**:
- [Spaceship System Guide](../Core-Systems/Spaceship-System.md)
- [Ship Templates](../../Assets/SpaceshipTemplates.md)
- YAML Files: `Assets/SpaceshipTemplates/*.yaml`

**Exercise**:
1. Create a fast scout ship
2. Create a heavy combat ship
3. Create a trading vessel

### Day 3-4: Faction System
**Goal**: Create 2 factions with relationships

**Resources**:
- [Faction System Guide](../Core-Systems/Faction-System.md)
- [Faction Setup Guide](../../Assets/FactionSetupGuide.md)

**Exercise**:
1. Create an exploration-focused faction
2. Create a trading-focused faction
3. Set up diplomatic relationships

### Day 5: Personnel System
**Goal**: Create a crew of 5 members

**Resources**:
- [Personnel System Guide](../Core-Systems/Personnel-System.md)
- [Personnel Templates](../../Assets/PersonnelTemplates.md)
- YAML Files: `Assets/PersonnelTemplates/*.yaml`

**Exercise**:
1. Create a Captain
2. Create a Pilot
3. Create an Engineer
4. Create a Medic
5. Create a generic Crew Member

## Essential Tools for Designers

### 1. Content Browser
**Keyboard Shortcut**: Ctrl+Space

Your main workspace for creating and managing Data Assets.

### 2. Blueprint Editor
**Keyboard Shortcut**: Double-click any Blueprint

Visual scripting interface - no code required!

### 3. Level Editor
**Keyboard Shortcut**: Ctrl+O to open maps

Where you build and test game levels.

### 4. Procedural Generation Scripts
**Location**: Root directory

Create random content quickly:
```bash
python ProceduralGenerators.py
```

**What it creates**:
- Random ships
- Random NPCs
- Random quests
- Random star systems

See [Procedural Generation Guide](../Automation-Testing/Procedural-Generation.md)

### 5. YAML Import Tool
**Location**: `YAMLtoDataAsset.py`

Import YAML templates directly into Unreal:
```bash
python YAMLtoDataAsset.py
```

## Common Designer Workflows

### Creating a New Ship
1. Copy a YAML template from `Assets/SpaceshipTemplates/`
2. Modify the stats to your liking
3. Create Data Asset in Content Browser
4. Fill in the properties based on your YAML
5. Test in game

### Creating a New Faction
1. Review example factions in `Assets/FactionSetupGuide.md`
2. Create FactionDataAsset in Content Browser
3. Set name, traits, relationships
4. Assign to stations/NPCs

### Creating a New Quest
1. Review quest types in `Assets/QuestSystemGuide.md`
2. Create QuestDataAsset in Content Browser
3. Set objectives and rewards
4. Add to quest giver NPCs

## Designer-Friendly Resources

### Documentation You Should Read

**Essential** (Read First):
- ✅ [Content Creation Quick Start](Content-Creation.md)
- ✅ [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)
- ✅ This Guide

**Important** (Read Soon):
- [Spaceship System](../Core-Systems/Spaceship-System.md)
- [Faction System](../Core-Systems/Faction-System.md)
- [Personnel System](../Core-Systems/Personnel-System.md)

**Reference** (Keep Handy):
- [Quick References](../Reference/Quick-References.md)
- [YAML Templates](../Reference/YAML-Templates.md)
- [Cheat Sheet](../Reference/Cheat-Sheet.md)

### YAML Template Locations

All templates are in the `Assets/` directory:

| System | Template Location |
|--------|------------------|
| Ships | `Assets/SpaceshipTemplates/` |
| Factions | `Assets/FactionAITemplates/` |
| Personnel | `Assets/PersonnelTemplates/` |
| Trading | `Assets/TradingTemplates/` |
| Ways (Guilds) | `Assets/WayNetworkTemplates/` |
| Combat | `Assets/CombatTemplates/` |

### No-Code Customization Options

You can customize these without any C++ knowledge:

✅ **Ship Stats** - All numeric values  
✅ **Faction Traits** - Behaviors and modifiers  
✅ **Crew Skills** - Skill levels and roles  
✅ **Trade Items** - Prices and availability  
✅ **Quest Objectives** - Goals and rewards  
✅ **UI Text** - All displayed text  
✅ **Visual Elements** - Colors, icons, meshes  

## Getting Help

### When You're Stuck

1. **Check the Wiki** - Search for your topic
2. **Review Examples** - Look at existing Data Assets
3. **Read Templates** - YAML files explain values
4. **Ask Questions** - Open a discussion on GitHub

### Common Questions

**Q: Do I need to know C++?**  
A: No! Most content creation uses Data Assets and Blueprints.

**Q: How do I test my changes?**  
A: Click the Play button in Unreal Editor (Alt+P).

**Q: Can I break something?**  
A: It's hard to break things with Data Assets. Worst case, delete and recreate.

**Q: Where do I find examples?**  
A: Check `Content/DataAssets/` for existing examples and `Assets/` for YAML templates.

**Q: How do I share my content?**  
A: Export Data Assets or commit to Git (see [Contributing Guide](../Developer-Guides/Contributing.md)).

## Pro Tips

💡 **Always save before testing** (Ctrl+S)  
💡 **Use descriptive names** like `DA_Ship_FastScout` not `DA_Ship1`  
💡 **Copy existing Data Assets** as templates  
💡 **Test early and often** with the Play button  
💡 **Keep YAML files open** for reference while working  
💡 **Use folders** to organize your Data Assets  
💡 **Document your designs** in the Description field  

## Next Steps

🎯 **Ready to create content?** → [Content Creation Guide](Content-Creation.md)  
🎯 **Want to learn Blueprints?** → [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)  
🎯 **Explore game systems?** → [Core Systems Overview](../Core-Systems/README.md)  
🎯 **Try automation?** → [Procedural Generation](../Automation-Testing/Procedural-Generation.md)  

## Community Resources

- 💬 [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions) - Ask questions
- 🐛 [Issue Tracker](https://github.com/Mittenzx/Adastrea/issues) - Report bugs
- 📚 [Full Documentation](../Reference/Documentation-Index.md) - Everything
- 🎨 [Contributing Guide](../Developer-Guides/Contributing.md) - Share your work

---

**Welcome to the team!** You're now ready to start creating amazing content for Adastrea.

*For the original onboarding guide, see [Assets/DesignerOnboarding.md](../../Assets/DesignerOnboarding.md)*
