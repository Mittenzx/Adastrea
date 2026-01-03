# Adastrea File Structure Standard

**Version**: 1.0  
**Last Updated**: 2026-01-02  
**Status**: Official Standard

---

## Overview

This document establishes the official file structure standard for the Adastrea project. All contributors must follow this structure when adding new files or reorganizing existing content.

---

## 📁 Root Directory Structure

```
Adastrea/
├── .github/                    # GitHub configuration (workflows, templates, instructions)
├── Assets/                     # YAML templates and asset generation configs
├── Blueprints/                 # Legacy Blueprint location (being phased out)
├── Config/                     # Unreal Engine configuration files
├── Content/                    # Unreal Engine content (Blueprints, assets, materials)
├── Docs/                       # Legacy docs location (use docs/ instead)
├── Plugins/                    # Unreal Engine plugins
├── Source/                     # C++ source code
├── Tools/                      # Python scripts and automation tools
├── docs/                       # PRIMARY documentation directory
├── tests/                      # Test infrastructure
└── wiki/                       # Wiki content
```

---

## 📚 Documentation Structure (`docs/`)

### Primary Subdirectories

```
docs/
├── BP/                         # Blueprint documentation
├── archive/                    # Deprecated/historical documentation
├── content-generation/         # Content creation guides
├── development/                # Developer guides and workflows
├── mvp/                        # Trade Simulator MVP documentation
├── reference/                  # Quick references and cheat sheets
├── setup/                      # Setup and installation guides
├── systems/                    # System-specific documentation
└── testing/                    # Testing documentation
```

### Documentation Categories

#### `docs/BP/` - Blueprint Documentation

**Purpose**: Documentation for specific Blueprint classes and widgets

**Naming Convention**: Match the Blueprint name exactly
- Widget Blueprints: `WBP_[WidgetName].md`
- Actor Blueprints: `BP_[ActorName].md`
- Other Blueprints: `BP_[Category]_[Name].md`

**Examples**:
- `WBP_TradingUI.md` - Trading interface widget
- `WBP_TradeItemRow.md` - Trade item row widget
- `BP_PlayerShip.md` - Player ship Blueprint
- `BP_SpaceStation_Main.md` - Main space station Blueprint

**Required Sections**:
1. Overview with purpose and responsibilities
2. Visual flow diagrams (SVG preferred)
3. Component descriptions
4. Event documentation
5. Usage examples

---

#### `docs/reference/` - Quick References

**Purpose**: Quick lookup guides and cheat sheets

**Naming Convention**: `[TOPIC]_QUICK_REFERENCE.md` or `[TOPIC]_GUIDE.md`

**Examples**:
- `MVP_QUICK_REFERENCE.md`
- `TRADING_QUICK_REFERENCE.md`
- `BLUEPRINT_IMAGE_GUIDE.md`
- `UI_WIDGET_REFERENCE.md`

**Content Guidelines**:
- Concise, scannable format
- Use tables for comparisons
- Include code examples
- Link to detailed guides in other directories

---

#### `docs/mvp/` - MVP Documentation

**Purpose**: Trade Simulator MVP-specific documentation

**Scope**: Phase 1 implementation, trading systems, core gameplay loop

**Key Files**:
- `TRADING_MVP_GUIDE.md` - Comprehensive MVP guide
- `PHASE_1_CHECKLIST.md` - Implementation checklist
- `IMPLEMENTATION_SUMMARY.md` - Progress tracking
- `BLUEPRINT_IMPLEMENTATION_GUIDE.md` - Blueprint setup guide

---

#### `docs/development/` - Developer Guides

**Purpose**: Development workflows, processes, and best practices

**Content Types**:
- Setup procedures
- Build instructions
- Development workflows
- Tool usage guides
- Best practices

---

#### `docs/systems/` - System Documentation

**Purpose**: Comprehensive documentation for game systems

**Naming Convention**: `[SYSTEM_NAME]_GUIDE.md`

**Examples**:
- Trading system
- Faction system
- Combat system
- AI system

---

#### `docs/setup/` - Setup Guides

**Purpose**: Installation and environment setup

**Content Types**:
- Engine installation
- IDE configuration
- Plugin setup
- Cloud build setup
- Docker configuration

---

#### `docs/testing/` - Testing Documentation

**Purpose**: Testing strategies, tools, and procedures

**Content Types**:
- Test plans
- Testing tools
- Validation procedures
- CI/CD documentation

---

#### `docs/archive/` - Archived Documentation

**Purpose**: Historical documentation no longer actively maintained

**When to Archive**:
- Document superseded by newer version
- Feature removed from project
- Outdated but potentially useful for reference

---

## 🔧 Source Code Structure (`Source/`)

### Module Organization

```
Source/
├── Adastrea/                   # Main game module
│   ├── Adastrea.Build.cs       # Build configuration
│   ├── Private/                # C++ implementation files
│   │   ├── AI/                 # AI logic
│   │   ├── Characters/         # Character/personnel
│   │   ├── Combat/             # Combat system
│   │   ├── Factions/           # Faction system
│   │   ├── Planets/            # Planet classes
│   │   ├── Player/             # Player systems
│   │   ├── Ships/              # Spaceship classes
│   │   ├── Stations/           # Space stations
│   │   ├── Trading/            # Trading economy
│   │   └── UI/                 # UI widgets (C++)
│   └── Public/                 # C++ header files
│       ├── AI/
│       ├── Characters/
│       ├── Combat/
│       ├── Factions/
│       ├── Interfaces/         # C++ interfaces
│       ├── Planets/
│       ├── Player/
│       ├── Ships/
│       ├── Stations/
│       ├── Trading/
│       └── UI/
└── AdastreaEditor/             # Editor-only code (future)
```

### Header/Implementation Organization

**Public Headers** (`Source/Adastrea/Public/[System]/`):
- Exposed to other modules and Blueprints
- UCLASS, USTRUCT, UENUM declarations
- Public API functions

**Private Implementation** (`Source/Adastrea/Private/[System]/`):
- Implementation files (.cpp)
- Internal helper functions
- Private class implementations

---

## 🎨 Content Structure (`Content/`)

### Content Organization

```
Content/
├── Blueprints/                 # Blueprint classes by system
│   ├── AI/
│   ├── Player/
│   ├── Ships/
│   ├── Stations/
│   └── Trading/
├── DataAssets/                 # Data Asset instances
│   ├── Factions/
│   ├── Items/
│   ├── Ships/
│   ├── Stations/
│   └── Trading/
├── Input/                      # Input mappings and configs
├── Maps/                       # Level maps
├── UI/                         # UMG Widget Blueprints
│   ├── Communication/
│   ├── Dialogue/
│   ├── HUD/
│   ├── Inventory/
│   ├── Map/
│   ├── Menus/
│   ├── Notifications/
│   ├── Quests/
│   ├── Ship/
│   ├── Stations/
│   └── Trading/
└── [Other Asset Categories]/
```

---

## 🛠️ Tools Structure (`Tools/`)

### Tool Categories

**Asset Management**:
- `YAMLtoDataAsset.py` - Convert YAML templates to Data Assets
- `AdastreaAssetValidator.py` - Validate asset configurations
- `organize_content.py` - Content organization utilities

**Code Validation**:
- `check_uproperty.py` - Validate UPROPERTY usage for GC tracking
- `check_null_safety.py` - Pointer safety checks
- `validate_naming.py` - Naming convention validation

**Blueprint Tools**:
- `generate_blueprint_images.py` - Generate Blueprint documentation diagrams
- `generate_bp_guides.py` - Generate Blueprint guide templates

**Testing**:
- `SmokeTest.py` - Basic functionality tests
- `ScreenshotTester.py` - Visual regression testing
- `AutomationRunner.py` - Automation test runner

**Content Generation**:
- `SchemaValidator.py` - YAML schema validation
- `ContentValidator.py` - Content validation
- `GoogleSheetsIntegration.py` - Google Sheets integration

---

## 📝 Naming Conventions

### Widget Blueprints

**Format**: `WBP_[Category]_[Name]`

**Examples**:
- `WBP_TradingUI` (NOT `WBP_TradingInterface`) - Main trading UI
- `WBP_TradeItemRow` - Trade item list row
- `WBP_HUD_Main` - Main HUD
- `WBP_MainMenu` - Main menu

**CRITICAL**: The widget name MUST match exactly between:
1. Unreal Engine asset name (`WBP_TradingUI.uasset`)
2. Documentation filename (`docs/BP/WBP_TradingUI.md`)
3. References in other documentation

### Actor Blueprints

**Format**: `BP_[Category]_[Name]` or `BP_[Name]`

**Examples**:
- `BP_PlayerShip` - Player-controlled ship
- `BP_Ship_NPC_Trader` - NPC trader ship
- `BP_SpaceStation_Main` - Main space station
- `BP_SpaceGameMode` - Game mode

### Data Assets

**Format**: `DA_[Type]_[Name]`

**Examples**:
- `DA_Ship_Pathfinder` - Pathfinder ship data
- `DA_Faction_SolarisUnion` - Solaris Union faction data
- `DA_TradeItem_Water` - Water trade item data

### C++ Classes

**Prefixes**:
- `U` - UObject-derived classes (components, data assets, etc.)
- `A` - AActor-derived classes (pawns, game modes, etc.)
- `F` - Structs
- `E` - Enums
- `I` - Interfaces

**Examples**:
- `USpaceshipDataAsset` - Spaceship Data Asset class
- `ASpaceStation` - Space station actor
- `FTradeTransaction` - Trade transaction struct
- `EWeaponType` - Weapon type enum
- `IIDamageable` - Damageable interface

---

## 🚫 Deprecated Locations

### DO NOT USE

**`/Blueprints/`** (root level):
- ❌ Legacy location
- ✅ Use `Content/Blueprints/[System]/` instead

**`/Docs/`** (root level, capital D):
- ❌ Legacy location
- ✅ Use `docs/` (lowercase) instead

**`Content/UI/Trading/WBP_TradingInterface.uasset`**:
- ❌ Old naming convention
- ✅ Use `WBP_TradingUI.uasset` instead

---

## 📋 File Addition Checklist

When adding new files, verify:

### Documentation Files

- [ ] Placed in correct `docs/` subdirectory
- [ ] Follows naming convention for category
- [ ] Includes front matter (title, date, version)
- [ ] Cross-referenced from relevant guides
- [ ] Added to `DOCUMENTATION_INDEX.md` if major guide

### Blueprint Files

- [ ] Placed in correct `Content/` subdirectory
- [ ] Follows naming convention (WBP_, BP_, DA_)
- [ ] Has corresponding documentation in `docs/BP/`
- [ ] Documentation includes visual diagrams (SVG preferred)
- [ ] Cross-referenced in relevant system guides

### C++ Files

- [ ] Headers in `Public/[System]/`
- [ ] Implementation in `Private/[System]/`
- [ ] Follows Unreal naming conventions (U, A, F, E, I prefixes)
- [ ] Includes Doxygen-style documentation
- [ ] Added to appropriate module's .Build.cs if needed

### Tool Scripts

- [ ] Placed in `Tools/` directory
- [ ] Includes docstring with usage instructions
- [ ] Added to `Tools/README.md` or relevant tool guide
- [ ] Executable permissions set if needed (chmod +x)

---

## 🔄 Migration Guidelines

### Moving Files

When moving files to comply with this standard:

1. **Update all references** in documentation
2. **Update import paths** in code
3. **Test build** after changes
4. **Update guides** that reference the file
5. **Add redirect notes** in old locations if necessary

### Deprecating Locations

When deprecating a directory or file:

1. **Create new location** following standard
2. **Copy content** to new location
3. **Add deprecation notice** to old location
4. **Update all references** to point to new location
5. **Mark for removal** in next major cleanup

---

## 📞 Questions and Updates

### Proposing Changes

To propose changes to this standard:

1. Open GitHub issue describing the change
2. Provide rationale for the change
3. Discuss impact on existing files
4. Update this document after approval
5. Announce changes to team

### Enforcement

This standard is enforced through:

- Code reviews
- Automated validation tools
- CI/CD checks
- Documentation audits

---

## 📚 Related Standards

- `.github/instructions/blueprint-documentation.instructions.md` - Blueprint documentation standard
- `.github/instructions/anti-patterns.instructions.md` - Anti-patterns to avoid
- `CODE_STYLE.md` - C++ and Blueprint coding standards
- `CONTRIBUTING.md` - Contribution guidelines

---

**Last Updated**: 2026-01-02  
**Version**: 1.0  
**Status**: Official Standard - All Contributors Must Follow
