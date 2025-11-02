# Adastrea Architecture Overview

This document provides a high-level overview of the Adastrea project architecture, code organization, and design patterns.

## Table of Contents

- [Project Structure](#project-structure)
- [Module Organization](#module-organization)
- [Design Patterns](#design-patterns)
- [Data Flow](#data-flow)
- [Blueprint Integration](#blueprint-integration)
- [Extension Points](#extension-points)

## Project Structure

```
Adastrea/
├── Assets/                      # Documentation and templates
│   ├── *Guide.md               # Comprehensive system guides
│   ├── *Templates.md           # Reference documentation
│   ├── *Workflow.md            # Designer workflows
│   ├── *SUMMARY.md             # Technical summaries
│   ├── FactionAITemplates/     # YAML templates for faction AI
│   ├── PersonnelAITemplates/   # YAML templates for personnel AI
│   ├── PersonnelTemplates/     # YAML templates for crew roles
│   ├── SpaceshipTemplates/     # YAML templates for ships
│   └── TradingTemplates/       # YAML templates for trading
│
├── Blueprints/                  # Blueprint documentation
│   ├── HomeworldBlueprintTemplates.md
│   └── TradingSystemBlueprintGuide.md
│
├── Config/                      # Unreal Engine configuration files
│
├── Content/                     # Game content (UE assets)
│   ├── Blueprints/             # Blueprint classes
│   ├── Homeworlds/             # Homeworld assets
│   ├── Maps/                   # Level maps
│   ├── Planets/                # Planet assets
│   ├── SpaceStations/          # Station assets and modules
│   └── UI/                     # UI widgets and assets
│
└── Source/                      # C++ source code
    ├── AdastreaEditor/         # Editor customizations module
    └── Adastrea/               # Core game systems
        ├── AI/                 # AI logic classes
        ├── Characters/         # Character and personnel classes
        ├── Factions/           # Faction system
        ├── Materials/          # Material system
        ├── Planets/            # Planet classes
        ├── Player/             # Player-specific systems
        ├── Ships/              # Spaceship classes
        ├── Stations/           # Space station classes
        ├── Trading/            # Trading economy system
        └── UI/                 # UI widgets and HUD
```

## Module Organization

### Core Module: `Adastrea`

The main game logic module, organized by feature:

#### AI System (`Source/Adastrea/AI/`)
- **NPCLogicBase**: Base class for all NPC AI behaviors
- **FactionLogic**: Strategic AI for faction-level decisions
- **PersonnelLogic**: Individual crew member AI with personality

**Key Features**:
- BlueprintNativeEvent pattern for easy extension
- 8 personality dispositions
- Peaceful early-game priorities
- Social relationship simulation

#### Characters System (`Source/Adastrea/Characters/`)
- **PersonnelDataAsset**: Complete crew member specification
  - 31 distinct roles
  - Skills, traits, relationships
  - Performance and status tracking
  - Command hierarchy

#### Faction System (`Source/Adastrea/Factions/`)
- **FactionDataAsset**: Faction definition and attributes
- **FFactionTrait**: Gameplay modifiers
- **FFactionRelationship**: Inter-faction diplomacy

**Integration Points**:
- Space stations (faction ownership)
- Trading (relationship-based prices)
- AI (faction-level strategic decisions)
- Homeworlds (starting relationships)

#### Material System (`Source/Adastrea/Materials/`)
- **MaterialDataAsset**: Raw materials and refined goods
- Quality tiers and rarity
- Station construction integration

#### Player System (`Source/Adastrea/Player/`)
- **HomeworldDataAsset**: Starting location with faction relationships
- Initial reputation system
- Starting experience customization

#### Ships System (`Source/Adastrea/Ships/`)
- **Spaceship**: Base ship actor with FloatingPawnMovement component
  - Supports 3D space flight movement
  - Configurable movement properties (speed, acceleration, turning)
  - Blueprint-accessible movement component
- **SpaceshipDataAsset**: Complete ship specifications
  - Core, Combat, Mobility stats
  - Utility, Operational, Advanced stats
  - Lore and flavor data
- **SpaceshipInterior**: Interior spaces
- **Component Placeholders**: Engines, weapons, organisms

**Design Philosophy**:
- Data-driven ship creation
- 6 pre-built templates
- Blueprint-callable rating functions
- Designer-friendly without C++ knowledge
- Full 3D movement capabilities for space flight

#### Stations System (`Source/Adastrea/Stations/`)
- **SpaceStation**: Core station actor with module management
- **SpaceStationModule**: Base module class with type system
- Module groups for organization (Docking, Power, Storage, etc.)

**Key Functions**:
- Add/remove/move modules
- Faction integration
- Trait-based gameplay modifiers

#### Trading System (`Source/Adastrea/Trading/`)
- **TradeItemDataAsset**: Tradeable commodities
- **MarketDataAsset**: Trading hubs with dynamic pricing
- **TradeContractDataAsset**: Delivery missions
- **AITraderComponent**: Autonomous trading behavior
- **TradeTransaction**: Transaction workflow

**Features**:
- Supply/demand simulation
- Faction relationship pricing
- Contract quest system
- AI-driven markets

#### UI System (`Source/Adastrea/UI/`)
- **StationEditorWidget**: Station construction UI
- **SectorMapWidget**: Galactic navigation
- **Sector**: Spatial regions

### Editor Module

#### AdastreaEditor Module
This module provides Unreal Editor customizations and tools for the Adastrea game.

## Design Patterns

### 1. Data Asset Pattern

**All game content uses Data Assets for configuration:**

```cpp
UCLASS(BlueprintType)
class ADASTREA_API UYourDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
    float SomeValue;
};
```

**Benefits**:
- Designer-friendly (no C++ required)
- Easy to create variants
- Reusable across Blueprints
- Version control friendly (text-based)

### 2. BlueprintNativeEvent Pattern

**All AI and extensible systems use BlueprintNativeEvent:**

```cpp
UFUNCTION(BlueprintNativeEvent, Category="AI")
void MakeDecision();

// C++ implementation
void UYourClass::MakeDecision_Implementation()
{
    // Default C++ logic
}
```

**Benefits**:
- C++ performance by default
- Blueprint override when needed
- Call parent implementation option
- Best of both worlds

### 3. Trait System Pattern

**Reusable across Factions, Personnel, and other systems:**

```cpp
USTRUCT(BlueprintType)
struct FYourTrait
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TraitID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ModifierValue;
};
```

**Benefits**:
- Flexible gameplay customization
- Stackable modifiers
- Easy to query and apply

### 4. Relationship System Pattern

**Used in Factions and Personnel:**

```cpp
USTRUCT(BlueprintType)
struct FRelationship
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="-100", ClampMax="100"))
    int32 RelationshipValue;
};
```

**Benefits**:
- Consistent diplomacy/social mechanics
- Simple -100 to +100 scale
- Easy to visualize and understand

## Data Flow

### Typical Game System Flow

```
1. Designer creates Data Asset
   ↓
2. Configure properties in Unreal Editor
   ↓
3. Reference Data Asset in Blueprint
   ↓
4. Blueprint calls C++ functions
   ↓
5. C++ processes and returns results
   ↓
6. Blueprint uses results for gameplay
```

### Example: Faction System Flow

```
FactionDataAsset (Created by Designer)
   ↓
SpaceStation references Faction
   ↓
Blueprint checks HasFactionTrait()
   ↓
C++ returns trait info
   ↓
Blueprint applies gameplay modifiers
```

## Blueprint Integration

### Best Practices

1. **All game content configuration is Blueprint-accessible**
   - Use `BlueprintReadOnly` for designer-set values
   - Use `BlueprintReadWrite` for runtime-modified values

2. **All gameplay functions are Blueprint-callable**
   - Mark with `UFUNCTION(BlueprintCallable)`
   - Add clear descriptions and parameter docs
   - Use appropriate categories

3. **Use proper metadata for better UX**
   ```cpp
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats",
             meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
   float Health;
   ```

4. **Struct usage for complex data**
   - Group related properties in structs
   - Mark structs `BlueprintType`
   - Use in arrays for lists of items

### Blueprint-Friendly APIs

Every system provides:
- **Getters**: Pure functions for querying state
- **Setters**: Functions for modifying state
- **Queries**: Boolean checks (Has*, Is*, Can*)
- **Calculations**: Functions returning computed values

Example from SpaceshipDataAsset:
```cpp
UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
float GetCombatRating() const;

UFUNCTION(BlueprintCallable, Category="Spaceship Stats")
bool IsSuitableForRole(const FString& Role) const;
```

## Extension Points

### For Designers (No C++)

1. **Create New Data Assets**
   - Use existing templates as reference
   - Configure all properties in editor
   - Test with Blueprint functions

2. **Override AI Behavior in Blueprints**
   - Create Blueprint based on UFactionLogic or UPersonnelLogic
   - Override BlueprintNativeEvents
   - Call parent implementation when needed

3. **Create Custom Modules**
   - Blueprint extends ASpaceStationModule
   - Set module type and properties
   - Integrate with station system

### For Programmers (C++)

1. **Add New Systems**
   - Follow Data Asset pattern
   - Expose all functionality to Blueprints
   - Create comprehensive documentation
   - Provide YAML templates

2. **Extend Existing Systems**
   - Use inheritance or composition
   - Add new traits or modifiers
   - Maintain backward compatibility

3. **Optimize Performance**
   - Profile before optimizing
   - Use parallel processing where applicable
   - Cache expensive calculations

## System Dependencies

```
Core Dependencies:
- Unreal Engine 5.5
- C++ Standard Library
- Unreal Core, Engine modules

System Interdependencies:
- Faction System ← Personnel, Stations, Trading, AI
- Ship System ← Personnel (crew), Trading (cargo)
- Station System ← Factions, Materials (construction)
- Trading System ← Factions (relationships), Ships (cargo)
- AI System ← Factions, Personnel (behavior)
- Personnel System ← Ships/Stations (assignments)
- Homeworld System ← Factions (relationships)
```

## Performance Considerations

### Current Implementation
- Most systems are lightweight (Data Assets)
- No significant runtime overhead
- Blueprint-callable functions are fast

### Scalability Concerns
- **Many Entities**: Use object pooling for ships/stations
- **Frequent AI Updates**: Stagger updates across frames
- **Large Markets**: Cache price calculations
- **Complex Relationships**: Use spatial partitioning for queries

### Optimization Strategies
1. **LOD for distant stations/ships**
2. **Tick rate reduction for non-critical systems**
3. **Async loading for Data Assets**
4. **Instanced static meshes for station modules**

## Testing Strategy

### Manual Testing
- Editor play testing for immediate feedback
- Use example Data Assets to verify systems
- Blueprint integration testing

### Automated Testing (Future)
- Unit tests for C++ classes
- Blueprint test maps
- Performance benchmarks

## Documentation Maintenance

All systems follow this documentation structure:
1. **System Guide** (Assets/[System]Guide.md) - Complete overview
2. **Templates** (Assets/[System]Templates.md) - Reference examples
3. **Workflow** (Assets/[System]Workflow.md) - Step-by-step process
4. **Summary** (Assets/[System]_SUMMARY.md) - Technical overview
5. **YAML Templates** (Assets/[System]Templates/*.yaml) - Copy-paste configs

## Getting Help

- **For Designers**: See DesignerOnboarding.md in Assets/
- **For Programmers**: Review CONTRIBUTING.md and this document
- **For Quick Start**: Check individual system guides
- **For Examples**: See YAML templates and Blueprint guides

## Future Architecture Plans

### Planned Modules
- **Networking Module**: Multiplayer support
- **Save System**: Persistence and serialization
- **Quest System**: Dynamic mission generation
- **Combat System**: Detailed combat mechanics
- **Exploration System**: Procedural generation

### Planned Improvements
- Custom Unreal Editor tools (using AdastreaEditor module)
- Visual programming for AI behaviors
- In-game station editor UI
- Dynamic event system
- Modding API

---

Last Updated: 2025-10-18
Version: 1.0
