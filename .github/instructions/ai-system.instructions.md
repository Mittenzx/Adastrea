# AI System Custom Instructions

<!-- applyTo: Source/Adastrea/AI/**/*.cpp, Source/Adastrea/AI/**/*.h -->

## AI System Overview

This directory contains the AI logic system for Adastrea, implementing a hierarchical AI architecture with BlueprintNativeEvent patterns for extensibility.

## Key Classes

### NPCLogicBase (`NPCLogicBase.cpp/.h`)
- **Base class** for all AI logic
- Uses `BlueprintNativeEvent` for all decision functions
- Provides default C++ implementations with Blueprint override capability
- Must be subclassed, never instantiated directly

### FactionLogic (`FactionLogic.cpp/.h`)
- **Strategic AI** for faction-level decisions
- Manages faction relationships, territory control, fleet movements
- Decision functions: `DecideDiplomaticAction`, `DecideEconomicAction`, `DecideMilitaryAction`
- Blueprint-overridable for custom faction behaviors

### PersonnelLogic (`PersonnelLogic.cpp/.h`)
- **Individual AI** for crew members and NPCs
- 8 personality dispositions: Friendly, Suspicious, Greedy, Generous, Cowardly, Brave, Lawful, Chaotic
- Decision functions: `DecideAction`, `ReactToEvent`, `EvaluateRelationship`
- Uses traits and skills from PersonnelDataAsset

## Coding Standards for AI System

### BlueprintNativeEvent Pattern

Always use this pattern for AI decision functions:

```cpp
// In header (.h)
UFUNCTION(BlueprintNativeEvent, Category="AI Logic")
void DecideAction(AActor* Target);

// In implementation (.cpp)
void UYourAIClass::DecideAction_Implementation(AActor* Target)
{
    // Default C++ implementation
    // This can be overridden in Blueprints
}
```

### Decision Function Naming

- Use verb-noun pattern: `Decide[Action]`, `Evaluate[Condition]`, `React[ToEvent]`
- Examples: `DecideMilitaryAction`, `EvaluateThreat`, `ReactToAttack`

### State Management

- Use `UPROPERTY()` for AI state variables
- Mark as `BlueprintReadOnly` if Blueprints should read state
- Initialize all state in constructor or BeginPlay

### Performance Considerations

- AI logic runs on tick or timer - keep functions fast
- Cache expensive calculations (pathfinding, line of sight)
- Use `BlueprintPure` for getters that don't modify state
- Stagger AI updates across multiple frames if needed

## Integration Points

### With Faction System
- AI logic accesses `UFactionDataAsset` for faction traits
- Faction relationships influence AI decisions
- AI can modify faction relationships based on events

### With Personnel System
- AI logic accesses `UPersonnelDataAsset` for skills and traits
- Personnel traits affect decision-making
- AI tracks and updates personnel relationships

### With Combat System
- AI evaluates threats using combat component data
- Decision functions consider weapon ranges, damage types
- Tactical AI uses targeting component info

## Common Patterns

### Decision Making Flow

```cpp
1. Gather context (environment, relationships, resources)
2. Evaluate options using traits and skills
3. Apply personality modifiers
4. Make decision (can be overridden in Blueprint)
5. Execute action
6. Update state and relationships
```

### Blueprint Override Pattern

Designers can override any `_Implementation` function in Blueprint:
1. Create Blueprint based on AI class
2. Override BlueprintNativeEvent function
3. Optionally call parent implementation
4. Add custom logic

## Testing AI Logic

- Test default C++ implementation first
- Verify Blueprint override works correctly
- Test personality variations (all 8 dispositions)
- Validate faction integration
- Check performance under load (100+ NPCs)

## Common Pitfalls

- Don't forget `_Implementation` suffix in .cpp
- Don't make AI decisions in Tick unless necessary (use timers)
- Don't access destroyed actors (check `IsValid()`)
- Don't create circular AI dependencies
- Don't hardcode behavior (use traits from Data Assets)

## Documentation Requirements

- Document decision logic with comments explaining WHY
- Add examples of when to override in Blueprint
- Explain trait and skill influences on decisions
- Note performance characteristics

---

When working with AI code, prioritize extensibility and Blueprint integration over raw performance. The system is designed to allow designers to customize NPC behavior without C++ knowledge.
