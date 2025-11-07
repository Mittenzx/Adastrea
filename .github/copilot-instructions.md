# GitHub Copilot – Adastrea Project Instructions

This file provides GitHub Copilot with project-specific context, coding standards, and best practices for the Adastrea open-world space flight game built on Unreal Engine 5.x.

## Project Overview

Adastrea is an open-world space flight game featuring exploration, combat, faction diplomacy, crew management, and dynamic trading economies. The project emphasizes:
- **Data-Driven Design**: All game content uses Blueprint Data Assets
- **Designer-Friendly**: Minimal C++ knowledge required for content creation
- **Modular Architecture**: Independent systems that integrate seamlessly
- **Blueprint First**: Full Blueprint exposure for all systems

## Technology Stack

- **Engine**: Unreal Engine 5.6
- **Languages**: C++ (primary), Blueprint (visual scripting)
- **Platforms**: Windows, Mac, Linux
- **Build System**: Unreal Build Tool (UBT)
- **Version Control**: Git

## Coding Standards

### C++ Style Guidelines

#### Naming Conventions

**Classes and Types:**
- `UCLASS()` derived from UObject: `UClassName` (e.g., `USpaceshipDataAsset`, `UFactionLogic`)
- `ACLASS()` derived from AActor: `AClassName` (e.g., `ASpaceStation`, `ASpaceStationModule`)
- Structs: `FStructName` (e.g., `FFactionTrait`, `FPersonnelSkill`)
- Enums: `EEnumName` (e.g., `EStationModuleGroup`, `EPersonnelRole`)
- Interfaces: `IInterfaceName` for implementation (e.g., `IMyInterface`)

**Variables:**
- Member variables: PascalCase (e.g., `MaxHealth`, `CurrentSpeed`)
- Boolean properties: Prefix with `b` (e.g., `bIsAlive`, `bIsEnabled`)
- Local variables: camelCase or PascalCase (be consistent within file)
- Constants: `kConstantName` or `ALL_CAPS` (e.g., `kMaxSpeed`, `MAX_CREW`)
- Always initialize pointers to `nullptr`

**Functions:**
- PascalCase with verb-noun combinations (e.g., `GetHealth`, `CalculateDamage`, `UpdateState`)
- Use `const` for functions that don't modify state
- BlueprintNativeEvent functions need `_Implementation` suffix in .cpp

#### File Structure

**Header Files (.h):**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "YourClassName.generated.h"

// Forward declarations for performance
class UOtherClass;
class AOtherActor;

/**
 * Brief one-line description of the class
 * 
 * Detailed description explaining:
 * - What this class does
 * - When to use it
 * - How it integrates with other systems
 * 
 * Usage Example:
 * - Create a Blueprint based on this class
 * - Configure properties in editor
 * - Use Blueprint functions to interact
 */
UCLASS(BlueprintType)
class ADASTREA_API UYourClassName : public UDataAsset
{
    GENERATED_BODY()

public:
    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
    FText DisplayName;

    // Blueprint-callable functions
    UFUNCTION(BlueprintCallable, Category="Your Category")
    int32 CalculateValue(float Input) const;

protected:
    UPROPERTY()
    int32 InternalValue;

private:
    void InternalHelperFunction();
};
```

#### UPROPERTY Best Practices

- Use `EditAnywhere, BlueprintReadWrite` for designer-editable runtime values
- Use `EditAnywhere, BlueprintReadOnly` for designer-set configuration
- Use `VisibleAnywhere, BlueprintReadOnly` for read-only display values
- Always specify a `Category` for organization
- Use `meta=` tags for constraints:
  - `ClampMin`, `ClampMax` for numeric ranges
  - `UIMin`, `UIMax` for slider ranges
  - `MultiLine=true` for text areas
- Document complex properties with tooltips

#### UFUNCTION Best Practices

- Mark functions `BlueprintCallable` if Blueprints should call them
- Mark functions `BlueprintPure` for getter functions with no side effects
- Use `BlueprintNativeEvent` for functions that can be overridden in Blueprints
- Use `BlueprintImplementableEvent` for Blueprint-only implementations
- Always specify a `Category`
- Add clear Doxygen-style documentation with `@param` and `@return`

### Blueprint Standards

**Naming Convention:**
- Blueprints: `BP_SystemName_Purpose` (e.g., `BP_Ship_Fighter`, `BP_Station_TradeHub`)
- Widgets: `WBP_WidgetName` (e.g., `WBP_HUD_Main`, `WBP_Menu_Settings`)
- Data Assets: `DA_Type_Name` (e.g., `DA_Ship_Pathfinder`, `DA_Faction_SolarisUnion`)

**Organization:**
- Keep Blueprint functions small (max 20-30 nodes)
- Use comment boxes to group related nodes
- Use reroute nodes to keep wires organized
- Add tooltips to complex variables
- Organize variables with categories
- Follow left-to-right, top-to-bottom flow

### Asset Naming

Follow the prefix system:
- `BP_` - Blueprint
- `DA_` - Data Asset
- `M_` - Material
- `MI_` - Material Instance
- `T_` - Texture (with suffixes: `_D` diffuse, `_N` normal, `_R` roughness, etc.)
- `SM_` - Static Mesh
- `SK_` - Skeletal Mesh
- `A_` - Animation
- `S_` - Sound
- `P_` - Particle
- `WBP_` - Widget Blueprint

## Security Practices

### Code Security

- **Never hardcode secrets**: No passwords, API keys, or tokens in source code
- **Validate all inputs**: Especially user input and data from external sources
- **Sanitize file paths**: Prevent directory traversal attacks
- **Use secure defaults**: Enable security features by default
- **Memory safety**: Always initialize pointers to `nullptr` and check before use

### Data Asset Security

- Validate all numeric values with appropriate min/max constraints using `meta=(ClampMin, ClampMax)`
- Sanitize string inputs, especially for names and descriptions
- Avoid exposing internal IDs or system paths in exported data

### Blueprint Security

- Validate inputs in Blueprint functions
- Use bounded loops to prevent infinite execution
- Check for null references before dereferencing
- Limit array sizes to prevent memory issues

## Testing Standards

### Manual Testing Requirements

- Test all changes in Unreal Editor play mode
- Verify Blueprint integration works correctly
- Test with example Data Assets to ensure backward compatibility
- Check that all systems load without errors
- Test edge cases (null values, extreme values, empty arrays)

### Testing Checklist

Before submitting code:
- [ ] Code compiles without errors or warnings
- [ ] All systems work in-editor
- [ ] Blueprint exposure tested
- [ ] Example Data Assets still work
- [ ] No crashes or critical bugs
- [ ] Documentation updated

## Design Patterns

### 1. Data Asset Pattern (Primary Pattern)

All game content uses Data Assets for configuration:
- Spaceships, Factions, Personnel, Trading items, etc.
- Designer-friendly, no C++ required
- Easy to create variants
- Reusable across Blueprints
- Version control friendly

### 2. BlueprintNativeEvent Pattern

All AI and extensible systems use BlueprintNativeEvent:
- Default C++ implementation for performance
- Can be overridden in Blueprints when needed
- Call parent implementation option
- Best of both worlds (C++ performance + Blueprint flexibility)

### 3. Trait System Pattern

Reusable across Factions, Personnel, and other systems:
- Flexible gameplay customization
- Stackable modifiers
- Easy to query and apply
- Consistent pattern across systems

### 4. Relationship System Pattern

Used in Factions and Personnel for diplomacy and social mechanics:
- Consistent -100 to +100 scale
- Easy to visualize and understand
- Support for various relationship types

## Internal Libraries and Utilities

### Logging

- Use `UE_LOG` with appropriate categories instead of direct console output
- Create custom log categories for systems: `DECLARE_LOG_CATEGORY_EXTERN(LogAdastrea, Log, All);`
- Log levels: `Fatal`, `Error`, `Warning`, `Display`, `Log`, `Verbose`, `VeryVerbose`

### Common Utilities

- Use `FMath` for mathematical operations (rounding, clamping, etc.)
- Use `FText` for localized strings, `FString` for internal strings
- Use `TArray` for dynamic arrays, `TMap` for dictionaries
- Use `FName` for identifiers (lightweight, case-insensitive)

## Documentation Requirements

### Code Comments

- Document all public APIs with Doxygen-style comments (`/** */`)
- Explain WHY, not WHAT (code should be self-documenting)
- Add inline comments for complex logic
- Use TODO, FIXME, NOTE tags for special comments

### System Documentation

When adding new systems, create:
1. **System Guide** (`Assets/[System]Guide.md`) - Complete overview
2. **Templates** (`Assets/[System]Templates.md`) - Reference examples
3. **Workflow** (`Assets/[System]Workflow.md`) - Step-by-step process
4. **YAML Templates** (`Assets/[System]Templates/*.yaml`) - Copy-paste configs

### Documentation Style

- Use markdown for all documentation files
- Include code examples where helpful
- Add screenshots for UI changes
- Keep documentation up-to-date with code changes
- Update README.md for major features

## Commit Message Convention

Use conventional commits:
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `refactor:` - Code refactoring (no functional changes)
- `test:` - Adding or updating tests
- `chore:` - Maintenance tasks
- `perf:` - Performance improvements
- `style:` - Code style changes (formatting, missing semicolons, etc.)

Examples:
- `feat: Add new spaceship rating calculation system`
- `fix: Resolve faction relationship crash on null reference`
- `docs: Update trading system guide with new examples`
- `refactor: Simplify personnel skill calculation logic`

## Build and Compilation

### Building the Project

- Open `.uproject` file in Unreal Engine to build automatically
- Or generate Visual Studio project files and build in "Development Editor" configuration
- Ensure no compiler warnings are introduced
- Test in editor after compilation

### Performance Considerations

- Profile before optimizing
- Cache expensive calculations
- Use object pooling for frequently spawned objects
- Stagger updates across frames for AI and systems
- Use LOD for distant stations/ships

## System-Specific Guidelines

### Spaceship System

- All ships defined through `USpaceshipDataAsset`
- 6 stat categories: Core, Combat, Mobility, Utility, Operational, Advanced, Lore
- Provide Blueprint-callable rating functions
- Use YAML templates from `Assets/SpaceshipTemplates/` as reference

### Faction System

- Define factions via `UFactionDataAsset`
- Support traits with gameplay modifiers
- Inter-faction relationships with diplomacy
- Integration with stations, trading, and AI
- Use templates from `Assets/FactionSetupGuide.md`

### Personnel System

- 31 distinct role types
- Skills, traits, relationships, performance tracking
- Command hierarchy support
- Use YAML templates from `Assets/PersonnelTemplates/`

### Station System

- Modular construction with `ASpaceStationModule`
- Faction integration
- Module groups for organization
- Blueprint-friendly APIs

### Trading System

- Dynamic supply/demand simulation
- Faction relationship-based pricing
- Contract quest system
- AI-driven autonomous trading

### AI System

- Base class `UNPCLogicBase` with BlueprintNativeEvent hooks
- Faction-level AI (`UFactionLogic`) for strategic decisions
- Personnel AI (`UPersonnelLogic`) with 8 personality dispositions
- Peaceful early-game priorities
- Social relationship simulation

## File Organization

### Source Code Structure

```
Source/
├── AdastreaEditor/          # Editor customizations
└── Adastrea/                # Core game systems
    ├── AI/                  # AI logic classes
    ├── Characters/          # Character and personnel
    ├── Factions/            # Faction system
    ├── Materials/           # Material system
    ├── Planets/             # Planet classes
    ├── Player/              # Player-specific systems
    ├── Ships/               # Spaceship classes
    ├── Stations/            # Space station classes
    ├── Trading/             # Trading economy
    └── UI/                  # UI widgets
```

### Content Organization

```
Content/
├── Blueprints/              # Blueprint classes
├── DataAssets/              # Data Assets organized by system
├── Maps/                    # Level maps
├── UI/                      # UI widgets
└── [System]/                # System-specific content
```

## Dependencies and Approved Libraries

### Core Dependencies

- Unreal Engine 5.6 Core modules
- Standard C++ Library
- Unreal Engine modules: Engine, CoreUObject, UnrealEd (for editor)

### Adding New Dependencies

- Discuss with team before adding third-party libraries
- Prefer Unreal Engine built-in solutions
- Document new dependencies in README.md
- Update .uproject file if adding plugins

## Common Pitfalls to Avoid

### C++ Pitfalls

- Don't forget `#include "YourClassName.generated.h"` as the last include in headers
- Don't modify UCLASS/USTRUCT without regenerating project files
- Don't use raw pointers without null checks
- Don't call parent constructor explicitly in C++ (Unreal handles this)
- Don't forget to mark BlueprintNativeEvent implementations with `_Implementation` suffix

### Blueprint Pitfalls

- Don't create circular dependencies between Blueprints
- Don't use Tick for everything (prefer timers or events)
- Don't forget to call parent implementation when overriding BlueprintNativeEvents
- Don't hardcode values that should be in Data Assets

### Data Asset Pitfalls

- Don't break existing Data Assets when changing classes
- Don't forget to provide default values
- Don't expose internal implementation details
- Don't skip validation of user-provided values

## Integration Guidelines

### Cross-System Integration

When integrating systems:
1. Use Data Assets for configuration
2. Provide clear Blueprint-callable APIs
3. Document integration points in system guides
4. Create integration examples in documentation
5. Test interactions between systems

### Example Integration Flow

```
Designer creates Data Asset
    ↓
Configure properties in Unreal Editor
    ↓
Reference Data Asset in Blueprint
    ↓
Blueprint calls C++ functions
    ↓
C++ processes and returns results
    ↓
Blueprint uses results for gameplay
```

## Accessibility and Usability

- Make all functionality accessible to designers without C++ knowledge
- Provide clear tooltips for properties
- Use meaningful category names
- Include usage examples in documentation
- Create YAML templates for common configurations

## Version Control Best Practices

- Use `.gitignore` to exclude build artifacts, binaries, and temporary files
- Keep commits focused and atomic
- Write clear commit messages following conventional commits
- Don't commit personal settings or cache files
- Use `.editorconfig` for consistent code formatting

## Questions and Support

- **For Designers**: See `Assets/DesignerOnboarding.md`
- **For Programmers**: Review `CONTRIBUTING.md` and `ARCHITECTURE.md`
- **For Quick Start**: Check individual system guides in `Assets/` folder
- **For Examples**: See YAML templates in `Assets/[System]Templates/`
- **Style Guide**: See `CODE_STYLE.md` for detailed formatting rules

## Additional Resources

- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.0/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [Project README](../README.md)
- [Architecture Overview](../ARCHITECTURE.md)
- [Code Style Guide](../CODE_STYLE.md)
- [Contributing Guidelines](../CONTRIBUTING.md)

---

**Last Updated**: 2025-10-18  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
