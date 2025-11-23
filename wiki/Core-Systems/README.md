# Core Systems Overview

Adastrea features 22+ major game systems, all implemented with Blueprint-friendly C++ and full Data Asset support.

## System Categories

### 🚀 Vehicle & Movement Systems
- **[Spaceship System](Spaceship-System.md)** - Complete ship creation and management
- **[Station System](Station-System.md)** - Modular space station construction
- **Navigation System** - Autopilot, pathfinding, and AI following

### 👥 Character & Social Systems
- **[Personnel System](Personnel-System.md)** - Crew management with 31 roles
- **[Faction System](Faction-System.md)** - Diplomacy and reputation
- **[Way System](Way-System.md)** - Guilds and alliances
- **[AI System](AI-System.md)** - Personality-driven NPC behavior

### 💰 Economy & Trade Systems
- **[Trading System](Trading-System.md)** - Dynamic economy with supply/demand
- **[Material System](Material-System.md)** - Resource production and consumption
- **Inventory System** - Item management and storage

### ⚔️ Combat & Conflict Systems
- **[Combat System](Combat-System.md)** - Weapons, shields, and targeting
- **Boarding System** - 5-phase ship boarding mechanics
- **Defense System** - Point defense and countermeasures

### 🎯 Progression & Content Systems
- **Quest System** - 10 quest types with procedural generation
- **Tutorial System** - Interactive step-by-step tutorials
- **Save System** - Multiple slots with auto-save
- **Player Progression** - Leveling, reputation, and unlocks

### 🎮 UI & Input Systems
- **HUD System** - Complete heads-up display
- **Enhanced Input System** - 30+ actions with remapping
- **UI Widgets** - Inventory, trading, customization interfaces

### 🌍 World & Environment Systems
- **Homeworld System** - Starting locations and backgrounds
- **Sector System** - Governance and territory control
- **Exploration System** - Scanners and discovery

### 🔧 Technical Systems
- **Audio System** - Sound effects and dynamic music
- **Performance System** - LOD and optimization tools
- **Testing System** - Automated smoke and screenshot tests

## Quick Access by Use Case

### "I want to create..."

**...a new ship type**  
→ [Spaceship System Guide](Spaceship-System.md)  
→ Templates: `Assets/SpaceshipTemplates/`

**...a new faction**  
→ [Faction System Guide](Faction-System.md)  
→ Templates: `Assets/FactionAITemplates/`

**...crew members**  
→ [Personnel System Guide](Personnel-System.md)  
→ Templates: `Assets/PersonnelTemplates/`

**...trade goods**  
→ [Trading System Guide](Trading-System.md)  
→ Templates: `Assets/TradingTemplates/`

**...a guild/alliance**  
→ [Way System Guide](Way-System.md)  
→ Templates: `Assets/WayNetworkTemplates/`

**...weapons/combat**  
→ [Combat System Guide](Combat-System.md)  
→ Templates: `Assets/CombatTemplates/`

**...quests/missions**  
→ Quest System Guide (see repository `Assets/QuestSystemGuide.md`)

## System Documentation Structure

Each major system includes:

1. **System Guide** (`[System]Guide.md`) - Complete documentation
2. **Quick Reference** (`[System]QuickReference.md`) - Fast lookup
3. **Workflow Guide** (`[System]Workflow.md`) - Step-by-step process
4. **Templates** (`[System]Templates.md`) - Example configurations
5. **YAML Files** (`Assets/[System]Templates/*.yaml`) - Copy-paste ready

## System Integration

All systems are designed to work together seamlessly:

```
Factions ←→ Trading ←→ Materials
   ↓           ↓
Personnel ←→ Ships ←→ Stations
   ↓           ↓
  AI ←→ Combat ←→ Quests
```

**Example Integration**:
- Factions affect trading prices
- Personnel skills affect ship performance
- Ship equipment affects combat effectiveness
- Quests reward reputation with factions
- Ways provide network bonuses

## Data-Driven Design

All systems use **Data Assets** for configuration:

**No C++ Required**:
- Create content through the Unreal Editor
- Use YAML templates as reference
- Customize with Blueprint functions
- Test immediately in Play mode

**Blueprint Exposure**:
- All systems expose functions to Blueprint
- Call C++ functions from visual scripts
- Override behaviors without recompiling
- Mix C++ performance with Blueprint flexibility

## Getting Started with Systems

### For Designers
1. **Start Simple**: Begin with Spaceship or Personnel system
2. **Use Templates**: Copy YAML examples
3. **Iterate Quickly**: Create, test, refine
4. **Combine Systems**: Add faction traits, crew skills, etc.

### For Programmers
1. **Read Architecture**: See [Architecture Overview](../Developer-Guides/Architecture.md)
2. **Review Code Style**: See [Code Style Guide](../Developer-Guides/Code-Style.md)
3. **Understand Patterns**: Data Assets, BlueprintNativeEvent
4. **Extend Systems**: Add new features via inheritance

## System Status

All core systems are **fully implemented** and **production-ready**:

✅ **Implemented** (22 systems)  
✅ **Blueprint Exposed** (all systems)  
✅ **Data Asset Support** (all systems)  
✅ **Documented** (all systems)  
✅ **Template Files** (all major systems)  

See [Project Roadmap](../Developer-Guides/Roadmap.md) for planned features.

## Complete System List

### High Priority (Essential Gameplay)
1. [Spaceship System](Spaceship-System.md) - Ships with 33 stats
2. [Faction System](Faction-System.md) - Diplomacy and relationships
3. [Personnel System](Personnel-System.md) - 31 crew roles
4. [Trading System](Trading-System.md) - Dynamic economy
5. [Combat System](Combat-System.md) - 6 weapon types
6. [Way System](Way-System.md) - Guilds and networks
7. [Material System](Material-System.md) - Resources
8. [AI System](AI-System.md) - 8 personality types

### Medium Priority (Gameplay Enhancement)
9. Station System - Modular construction
10. Quest System - 10 quest types
11. Navigation System - Pathfinding
12. Enhanced Input - 30+ actions
13. HUD System - Complete UI
14. Save System - Multi-slot saves
15. Tutorial System - Interactive guides
16. Audio System - Effects and music

### Supporting Systems
17. Exploration System - Scanners
18. Homeworld System - Starting locations
19. Sector System - Governance
20. Inventory System - Item management
21. Player Progression - Leveling
22. Performance Tools - Optimization

### Development Tools
23. Testing Automation - Smoke tests
24. Procedural Generation - Random content
25. Scene Population - Level automation
26. Asset Validation - Quality checks

## System Dependencies

Some systems depend on others:

**Core Foundation**:
- Spaceship → Required by: Combat, Navigation, Personnel assignment
- Faction → Required by: Trading, AI, Diplomacy
- Personnel → Required by: AI, Ship operations

**Optional Integration**:
- Ways → Enhances: Faction, Trading
- Material → Enhances: Trading, Stations
- Sector → Enhances: Faction, Governance

**Recommendation**: Implement core systems first, then add optional systems.

## Documentation Locations

**In This Wiki**:
- High-level overviews
- Getting started guides
- Quick references

**In Repository** (`Assets/` directory):
- Complete system guides
- Technical specifications
- YAML templates
- API references

**Cross-Reference**: Wiki pages link to detailed repository docs.

## Next Steps

📖 **Learn a System**: Pick one from the list above  
🎯 **Create Content**: Follow the system's workflow guide  
🔧 **Extend Systems**: Add custom features via Blueprint  
🤝 **Contribute**: Share your improvements  

## Getting Help

- 📚 [Documentation Index](../Reference/Documentation-Index.md)
- 💬 [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)
- 🐛 [Issue Tracker](https://github.com/Mittenzx/Adastrea/issues)
- 🎓 [Tutorial Videos](https://github.com/Mittenzx/Adastrea/wiki) (coming soon)

---

**All systems are ready to use!** Start exploring and creating.

*For detailed technical documentation, see the `Assets/` directory in the repository*
