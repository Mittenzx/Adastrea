# Architecture Overview

> **Complete Technical Documentation**: See [ARCHITECTURE.md](../../ARCHITECTURE.md) in the repository root for the full architecture guide.

## Quick Overview

Adastrea uses a modular, data-driven architecture with full Blueprint integration. The project emphasizes clean separation of concerns, performance optimization, and designer-friendly workflows.

## Key Architectural Principles

### 1. Data-Driven Design
All game content is created through **Data Assets** rather than hardcoded C++:
- Ships, factions, personnel, trading items, etc.
- No recompilation needed for content changes
- Designer-friendly workflow
- Version control friendly

### 2. Blueprint-First Approach
Every system exposes full Blueprint functionality:
- `BlueprintCallable` functions for all operations
- `BlueprintNativeEvent` for extensibility
- C++ performance with Blueprint flexibility
- Override behaviors without touching C++

### 3. Modular Systems
22+ independent game systems that integrate seamlessly:
- Each system is self-contained
- Clear interfaces between systems
- Systems can be enabled/disabled
- Easy to extend and modify

### 4. Component-Based Architecture
Systems use Unreal's component model:
- `ActorComponent` for reusable functionality
- `UObject` for data and logic
- Composition over inheritance
- Flexible actor construction

## System Organization

```
Source/Adastrea/
├── Ships/              # Spaceship system
├── Factions/           # Faction and diplomacy
├── Characters/         # Personnel and crew
├── Trading/            # Economy and trading
├── Stations/           # Space stations
├── Combat/             # Weapons and combat
├── AI/                 # AI logic
├── Player/             # Player systems
├── UI/                 # User interface
└── ...                # 13+ more systems
```

## Data Flow

```
YAML Templates
    ↓
Data Assets (Editor)
    ↓
Blueprint Classes
    ↓
Game Runtime
```

## Core Design Patterns

### Data Asset Pattern
```cpp
UCLASS(BlueprintType)
class USpaceshipDataAsset : public UDataAsset
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetCombatRating() const;
};
```

### BlueprintNativeEvent Pattern
```cpp
UFUNCTION(BlueprintNativeEvent, Category="AI")
void DecideAction(AActor* Target);

// C++ default implementation
void DecideAction_Implementation(AActor* Target) {
    // Can be overridden in Blueprint
}
```

### Trait System Pattern
```cpp
USTRUCT(BlueprintType)
struct FTrait {
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText TraitName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Modifier;
};
```

## System Integration

Systems communicate through:
1. **Direct References** - Data Asset pointers
2. **Events** - Blueprint and C++ events
3. **Interfaces** - C++ interfaces for polymorphism
4. **Subsystems** - Global managers (e.g., VerseSubsystem)

## Performance Considerations

- **Data Assets**: Loaded on demand, memory efficient
- **Object Pooling**: Reuse frequently spawned objects
- **LOD System**: Distance-based quality reduction
- **Async Operations**: Background loading and processing
- **Blueprint Nativization**: Convert Blueprints to C++ (optional)

## Documentation Structure

Each system follows consistent documentation:
1. System Guide (`[System]Guide.md`)
2. Quick Reference (`[System]QuickReference.md`)
3. Workflow Guide (`[System]Workflow.md`)
4. Templates (`[System]Templates.md`)
5. YAML Examples (`Assets/[System]Templates/*.yaml`)

## Development Workflow

```
1. Design system (C++)
2. Expose to Blueprint
3. Create Data Asset types
4. Document with guides
5. Create YAML templates
6. Test in editor
7. Iterate based on feedback
```

## Code Organization

### Headers (.h)
- Forward declarations for performance
- UPROPERTY macros for Blueprint exposure
- UFUNCTION macros for callable functions
- Clear comments and documentation

### Implementation (.cpp)
- Include only what's needed
- Use namespaces for organization
- Keep functions focused and small
- Document complex logic

### Blueprints
- Organized by system in Content/
- Use descriptive naming (BP_*, WBP_*)
- Keep graphs clean and readable
- Comment complex logic

## Testing Strategy

- **Unit Tests**: C++ unit tests for core logic
- **Smoke Tests**: Automated level loading tests
- **Screenshot Tests**: Visual regression testing
- **Integration Tests**: System interaction tests
- **Manual Tests**: Designer playtesting

## Extensibility

The architecture supports extension through:
- **Inheritance**: Extend C++ classes
- **Composition**: Add components
- **Data Assets**: Create new content types
- **Blueprints**: Visual scripting
- **Plugins**: External modules

## Complete Documentation

For detailed technical documentation, architecture diagrams, and system specifications:

📖 **[ARCHITECTURE.md](../../ARCHITECTURE.md)** - Complete architecture guide (in repository root)

## Related Documentation

- [Code Style Guide](Code-Style.md)
- [Contributing Guidelines](Contributing.md)
- [Core Systems Overview](../Core-Systems/README.md)
- [Blueprint Development](../Blueprint-Development/Blueprint-Beginners-Guide.md)

---

**The architecture is designed for scalability, maintainability, and designer-friendliness.**

*Last Updated: November 23, 2025*
