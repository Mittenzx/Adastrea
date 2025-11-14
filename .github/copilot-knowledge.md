# GitHub Copilot - Adastrea Knowledge Base

This file provides GitHub Copilot with quick-reference information about the Adastrea repository structure, key files, and important context for code generation and navigation.

## Repository Structure

### Root Level Organization

```
Adastrea/
├── Source/                      # C++ source code
│   ├── Adastrea/               # Main game module
│   ├── AdastreaEditor/         # Editor customizations
│   └── StationEditor/          # Station editor plugin
├── Content/                     # Unreal Engine content (Blueprints, assets)
├── Config/                      # Engine and game configuration
├── Plugins/                     # Custom plugins
├── Assets/                      # Documentation and templates
├── Docs/                        # Additional documentation
├── .github/                     # GitHub configuration
│   ├── copilot-instructions.md # Copilot custom instructions
│   ├── agents/                 # Custom agent definitions
│   └── workflows/              # CI/CD workflows
└── [Python Scripts]            # Various automation scripts
```

### Source Code Structure

```
Source/Adastrea/
├── AI/                         # AI logic classes
│   ├── NPCLogicBase.cpp/.h    # Base class for all AI
│   ├── FactionLogic.cpp/.h    # Faction-level AI
│   └── PersonnelLogic.cpp/.h  # Personnel AI
├── Combat/                     # Combat system
│   ├── WeaponComponent.cpp/.h
│   ├── TargetingComponent.cpp/.h
│   ├── BoardingComponent.cpp/.h
│   └── CombatHealthComponent.cpp/.h
├── Ships/                      # Spaceship classes
│   ├── Spaceship.cpp/.h
│   ├── SpaceshipDataAsset.cpp/.h
│   └── SpaceshipInterior.cpp/.h
├── Stations/                   # Space station classes
│   ├── SpaceStation.cpp/.h
│   └── SpaceStationModule.cpp/.h
├── Factions/                   # Faction system
│   └── FactionDataAsset.cpp/.h
├── Characters/                 # Character and personnel
│   └── PersonnelDataAsset.cpp/.h
├── Trading/                    # Trading economy
│   └── TradingDataAsset.cpp/.h
├── Materials/                  # Material system
│   └── MaterialDataAsset.cpp/.h
├── Navigation/                 # Navigation components
│   └── NavigationComponent.cpp/.h
├── Input/                      # Input handling
│   └── InputConfigDataAsset.cpp/.h
├── Planets/                    # Planet classes
│   └── Planet.h
└── UI/                         # UI widgets
```

## Key Files and Their Purposes

### Core Documentation Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview, quick start guide |
| `ARCHITECTURE.md` | System architecture and design patterns |
| `CODE_STYLE.md` | C++ and Blueprint coding standards |
| `CONTRIBUTING.md` | Contribution guidelines |
| `ROADMAP.md` | Development roadmap and future plans |
| `DOCUMENTATION_INDEX.md` | Complete index of all documentation |

### System Documentation (Assets/ directory)

| File | System |
|------|--------|
| `Assets/SpaceshipSetupGuide.md` | Spaceship Data Asset system |
| `Assets/FactionSetupGuide.md` | Faction system |
| `Assets/PersonnelSetupGuide.md` | Personnel management |
| `Assets/StationConstructionGuide.md` | Space station system |
| `Assets/TradingSetupGuide.md` | Trading economy |
| `Assets/AISystemGuide.md` | AI implementation |
| `Assets/MaterialSystemGuide.md` | Material system |
| `Assets/WayNetworkGuide.md` | Way navigation system |

### Quick Reference Guides

| File | Purpose |
|------|---------|
| `CHEATSHEET.md` | Common commands and workflows |
| `QUICKSTART.md` | Fast project setup |
| `CONTENT_CREATION_QUICKSTART.md` | Content creation basics |
| `DATA_MANAGEMENT_QUICK_REFERENCE.md` | Data asset workflows |
| `TESTING_QUICK_REFERENCE.md` | Testing procedures |

### Automation Scripts

| Script | Purpose |
|--------|---------|
| `YAMLtoDataAsset.py` | Convert YAML to Unreal Data Assets |
| `ProceduralGenerators.py` | Generate procedural content |
| `ScenePopulator.py` | Populate levels with actors |
| `GuideGenerator.py` | Generate documentation |
| `AdastreaAssetValidator.py` | Validate asset integrity |
| `SetupCheck.py` / `SetupCheck.sh` | Validate development environment |
| `SmokeTest.py` | Run smoke tests |
| `AutomationRunner.py` | Run automated tests |

## Core Concepts

### Data Assets

All game content uses Blueprint Data Assets:
- **Spaceship Data Assets** (`USpaceshipDataAsset`): Define ship types with stats
- **Faction Data Assets** (`UFactionDataAsset`): Define factions with traits and relationships
- **Personnel Data Assets** (`UPersonnelDataAsset`): Define crew members with skills
- **Trading Data Assets** (`UTradingDataAsset`): Define tradeable items
- **Material Data Assets** (`UMaterialDataAsset`): Define materials and resources

### Key Systems

1. **Spaceship System**: Data-driven ship definitions with 6 stat categories
2. **Faction System**: Diplomacy, traits, and inter-faction relationships
3. **Personnel System**: 31 role types, skills, traits, relationships
4. **Station System**: Modular construction with faction integration
5. **Trading System**: Dynamic supply/demand with contract quests
6. **AI System**: BlueprintNativeEvent-based with faction and personnel levels
7. **Combat System**: Weapon components, targeting, boarding
8. **Material System**: Resource management and trading

## Common File Patterns

### C++ Class Naming

- `UClassName` - UObject-derived classes (e.g., `USpaceshipDataAsset`)
- `AClassName` - AActor-derived classes (e.g., `ASpaceStation`)
- `FStructName` - Structs (e.g., `FFactionTrait`)
- `EEnumName` - Enums (e.g., `EPersonnelRole`)
- `IInterfaceName` - Interfaces (e.g., `IInteractable`)

### Blueprint Naming

- `BP_SystemName_Purpose` - Blueprints (e.g., `BP_Ship_Fighter`)
- `WBP_WidgetName` - Widget Blueprints (e.g., `WBP_HUD_Main`)
- `DA_Type_Name` - Data Assets (e.g., `DA_Faction_SolarisUnion`)

### File Extensions

- `.cpp` / `.h` - C++ source and header files
- `.uasset` - Unreal Engine assets (in Content/)
- `.umap` - Unreal Engine maps/levels
- `.md` - Markdown documentation
- `.yaml` - YAML templates for data assets
- `.py` - Python automation scripts
- `.json` - Configuration files

## Development Workflow

### Creating New Content

1. **Data Assets**: Use YAML templates in `Assets/[System]Templates/`
2. **Blueprints**: Follow `BLUEPRINT_CREATION_GUIDE.md`
3. **C++ Classes**: Follow `CODE_STYLE.md` and header template
4. **Documentation**: Update relevant guide in `Assets/` directory

### Testing

- **Setup Validation**: Run `SetupCheck.py` or `SetupCheck.sh`
- **Smoke Tests**: Run `SmokeTest.py` for basic functionality
- **Asset Validation**: Run `AdastreaAssetValidator.py`
- **In-Editor**: Test in Unreal Editor play mode

### Code Review

- Follow conventional commit format (feat:, fix:, docs:, etc.)
- Ensure all public APIs have documentation
- Test Blueprint integration
- Run validation scripts before committing

## Important Constraints

### Build System

- Requires Unreal Engine 5.6
- Uses Unreal Build Tool (UBT)
- C++ compiler: MSVC (Windows), Clang (Mac/Linux)
- No additional build dependencies

### Performance

- Data Assets loaded on-demand
- Blueprint-callable functions optimized
- AI logic uses BlueprintNativeEvent for performance
- LOD system for distant objects

### Design Philosophy

1. **Data-Driven**: All content in Data Assets, not hardcoded
2. **Designer-Friendly**: Minimal C++ knowledge required
3. **Modular**: Systems are independent and integrate cleanly
4. **Blueprint First**: Full Blueprint exposure for all systems
5. **Documentation**: Every system has comprehensive guides

## Quick Command Reference

### Setup and Validation

```bash
# Validate development environment
python SetupCheck.py

# Run smoke tests
python SmokeTest.py

# Validate assets
python AdastreaAssetValidator.py
```

### Content Generation

```bash
# Convert YAML to Data Assets
python YAMLtoDataAsset.py input.yaml

# Generate procedural content
python ProceduralGenerators.py --config config.json

# Populate scene
python ScenePopulator.py --level MyLevel
```

### Documentation

```bash
# Generate system guides
python GuideGenerator.py --system Spaceship
```

## Integration Points

### System Dependencies

- **Spaceship ↔ Combat**: Ships use combat components
- **Faction ↔ Trading**: Faction relationships affect prices
- **Personnel ↔ AI**: Personnel AI uses faction context
- **Station ↔ Trading**: Stations host trading posts
- **Material ↔ Trading**: Materials are tradeable items
- **AI ↔ Faction**: Faction AI coordinates NPC behavior

### Blueprint Integration

All C++ systems expose Blueprint-callable functions:
- `UFUNCTION(BlueprintCallable)` for actions
- `UFUNCTION(BlueprintPure)` for getters
- `BlueprintNativeEvent` for overridable logic

## Troubleshooting

### Common Issues

1. **Compilation Errors**: Regenerate project files
2. **Missing Assets**: Check Content/ directory structure
3. **Blueprint Errors**: Verify Data Asset references
4. **AI Not Working**: Check BlueprintNativeEvent implementations
5. **Performance Issues**: Profile before optimizing

### Debug Tools

- Unreal Editor output log
- Visual Studio debugger
- Blueprint debugger
- Python script verbose mode (`--verbose`)

## Version Information

- **Engine**: Unreal Engine 5.6
- **Language Standard**: C++17
- **Blueprint Version**: Nativized for performance
- **Last Updated**: 2025-11-14

---

This knowledge base is maintained alongside `.github/copilot-instructions.md` to provide GitHub Copilot with comprehensive context about the Adastrea project structure and conventions.
