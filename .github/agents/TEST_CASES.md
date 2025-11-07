# Custom Agent Examples and Test Cases

This file contains example interactions with the Adastrea Developer Expert custom agent to verify it provides accurate, helpful responses.

## Test Case 1: Understanding Core Systems

**Query:**
```
@workspace /agent adastrea-developer 
What are the main game systems in Adastrea and how are they organized?
```

**Expected Response Should Include:**
- List of 9 core systems (Spaceship, Station, Faction, Personnel, AI, Trading, Player, Material, Editor)
- File locations (e.g., `Source/Adastrea/Ships/`, `Source/Adastrea/Factions/`)
- Mention of Data Asset pattern as primary design
- Blueprint integration approach

---

## Test Case 2: Creating a New Data Asset

**Query:**
```
@workspace /agent adastrea-developer 
I want to create a new weapon system. What's the correct approach following Adastrea patterns?
```

**Expected Response Should Include:**
- Create `UWeaponDataAsset` inheriting from `UPrimaryDataAsset`
- File location: `Source/Adastrea/[NewFolder]/WeaponDataAsset.h`
- Blueprint exposure with `UPROPERTY(EditAnywhere, BlueprintReadOnly)`
- Category organization
- Reference to similar systems (SpaceshipDataAsset pattern)
- YAML template creation
- Documentation requirements

---

## Test Case 3: Blueprint Integration

**Query:**
```
@workspace /agent adastrea-developer 
How do I make my C++ function callable from Blueprints?
```

**Expected Response Should Include:**
- `UFUNCTION(BlueprintCallable, Category="YourCategory")`
- Use `BlueprintPure` for getters with no side effects
- `BlueprintNativeEvent` for overridable functions
- Proper parameter documentation
- Category naming conventions
- Reference to existing examples in codebase

---

## Test Case 4: System Integration

**Query:**
```
@workspace /agent adastrea-developer 
How does the faction reputation system integrate with trading?
```

**Expected Response Should Include:**
- `UPlayerReputationComponent` provides `GetTradePriceModifier()`
- Returns multiplier (0.7x to 1.5x based on reputation)
- 7 reputation tiers (Hated to Revered)
- File location: `Source/Adastrea/Player/PlayerReputationComponent.h`
- Integration example code snippet
- Reference to trading system files

---

## Test Case 5: Coding Standards

**Query:**
```
@workspace /agent adastrea-developer 
What naming convention should I use for a new faction trait enum?
```

**Expected Response Should Include:**
- Enum naming: `EEnumName` format
- Example: `EFactionTraitCategory`
- PascalCase for enum values
- File location guidelines
- Reference to existing enums (EStationModuleGroup, EPersonnelRole)
- Blueprint exposure requirements

---

## Test Case 6: File Organization

**Query:**
```
@workspace /agent adastrea-developer 
Where should I put new AI behavior classes?
```

**Expected Response Should Include:**
- Location: `Source/Adastrea/AI/`
- Base class pattern: inherit from `UNPCLogicBase`
- BlueprintNativeEvent pattern usage
- File naming conventions
- Reference to existing AI classes (FactionLogic, PersonnelLogic)

---

## Test Case 7: Documentation Requirements

**Query:**
```
@workspace /agent adastrea-developer 
I implemented a new planetary system. What documentation do I need to create?
```

**Expected Response Should Include:**
- System Guide: `Assets/PlanetarySystemGuide.md`
- Templates: `Assets/PlanetaryTemplates.md`
- Workflow: `Assets/PlanetaryWorkflow.md`
- YAML templates folder: `Assets/PlanetaryTemplates/`
- Update main README.md
- Doxygen-style code comments
- Reference to documentation structure pattern

---

## Test Case 8: Data Asset Pattern

**Query:**
```
@workspace /agent adastrea-developer 
Explain the Data Asset pattern used in Adastrea
```

**Expected Response Should Include:**
- Primary design pattern for all game content
- Designer-friendly, no C++ required
- Inherit from `UPrimaryDataAsset`
- Blueprint-accessible properties
- YAML template support
- Examples: SpaceshipDataAsset, FactionDataAsset, PersonnelDataAsset
- Version control friendly (text-based)

---

## Test Case 9: Trait System

**Query:**
```
@workspace /agent adastrea-developer 
How do I add a new faction trait?
```

**Expected Response Should Include:**
- `FFactionTrait` struct usage
- 5 trait categories (Military, Economic, Scientific, Diplomatic, Special)
- File: `Source/Adastrea/Factions/FactionDataAsset.h`
- Trait fields: TraitID, DisplayName, Description, ModifierValue, Category
- Blueprint-callable functions: `HasTrait()`, `GetTraitModifier()`
- YAML template format in `Assets/FactionSetupGuide.md`

---

## Test Case 10: BlueprintNativeEvent Pattern

**Query:**
```
@workspace /agent adastrea-developer 
What is the BlueprintNativeEvent pattern and when should I use it?
```

**Expected Response Should Include:**
- Used for all AI and extensible systems
- Default C++ implementation with `_Implementation` suffix
- Blueprint override capability
- Call parent option
- Example from `UNPCLogicBase`, `UFactionLogic`, `UPersonnelLogic`
- Best for performance + flexibility
- Declaration and implementation example

---

## Test Case 11: Specific File Location

**Query:**
```
@workspace /agent adastrea-developer 
Where is the player reputation component implemented?
```

**Expected Response Should Include:**
- File: `Source/Adastrea/Player/PlayerReputationComponent.h`
- Corresponding .cpp file
- Key functions: `ModifyReputation()`, `GetReputationTier()`, `GetTradePriceModifier()`
- 7 reputation tiers
- Integration with faction system

---

## Test Case 12: Build and Compilation

**Query:**
```
@workspace /agent adastrea-developer 
My C++ class won't compile. What are common issues?
```

**Expected Response Should Include:**
- Missing `#include "ClassName.generated.h"` as last include
- Missing module dependency in `.Build.cs` file
- Forward declarations vs full includes
- UCLASS/UPROPERTY/UFUNCTION macros
- Regenerate project files after UCLASS changes
- Reference to BUILD_TROUBLESHOOTING.md

---

## Test Case 13: Testing and Validation

**Query:**
```
@workspace /agent adastrea-developer 
How do I test my new Data Asset system?
```

**Expected Response Should Include:**
- Create example Data Assets in Content Browser
- Test in Unreal Editor play mode
- Verify Blueprint integration
- Test with example scenarios
- Check against PlaytestingChecklist.md
- No crashes or errors on load
- Reference to testing documentation

---

## Test Case 14: Python Automation

**Query:**
```
@workspace /agent adastrea-developer 
What Python scripts are available for content generation?
```

**Expected Response Should Include:**
- ProceduralGenerators.py - star systems, NPCs, quests, ships
- ScenePopulator.py - scene population with 6 patterns
- InteriorLayoutPopulator.py - interior layouts with 10 room types
- YAMLtoDataAsset.py - YAML import tool
- TemplateGenerator.py - YAML template generator
- Usage examples
- Configuration file formats

---

## Test Case 15: Current Development Status

**Query:**
```
@workspace /agent adastrea-developer 
What are the current development priorities for Adastrea?
```

**Expected Response Should Include:**
- Reference to WHATS_STILL_NEEDED.md
- 58% of core features complete
- Priority items: Integration work, content creation, UI development
- Specific systems: Mission/quest system, AI behavior trees, faction fleet AI
- Current implementation statistics
- ~8,587 lines of C++ code

---

## Verification Checklist

When testing the custom agent, verify that responses:

- [ ] Reference actual files and classes in the Adastrea codebase
- [ ] Provide specific file paths (e.g., `Source/Adastrea/Ships/`)
- [ ] Follow Adastrea's naming conventions and patterns
- [ ] Include code examples when appropriate
- [ ] Reference relevant documentation files
- [ ] Mention integration with other systems
- [ ] Are accurate to the current implementation
- [ ] Avoid generic Unreal Engine advice
- [ ] Include practical, actionable guidance
- [ ] Reference the Data Asset pattern where applicable

---

## False Positive Checks

These queries should NOT be answered by the Adastrea agent (or should redirect):

**Query:** "How do I install Unreal Engine?"
**Expected:** General installation guide, not Adastrea-specific

**Query:** "What's the best C++ IDE?"
**Expected:** Generic IDE comparison, not project-specific

**Query:** "How do Git branches work?"
**Expected:** Generic Git tutorial, not Adastrea workflow

---

## Agent Accuracy Tests

These tests verify the agent knows current implementation details:

### Test: Personnel Role Count
**Query:** "How many personnel role types are there?"
**Expected Answer:** 31 distinct role types

### Test: Faction System Files
**Query:** "What's the diplomacy manager class name?"
**Expected Answer:** `UFactionDiplomacyManager`

### Test: Reputation Tiers
**Query:** "How many reputation tiers exist?"
**Expected Answer:** 7 tiers (Hated, Hostile, Unfriendly, Neutral, Friendly, Honored, Revered)

### Test: Ship Stat Categories
**Query:** "How many stat categories does SpaceshipDataAsset have?"
**Expected Answer:** 7 categories (Core, Combat, Mobility, Utility, Operational, Advanced, Lore)

### Test: AI Personalities
**Query:** "How many personality dispositions are there for personnel AI?"
**Expected Answer:** 8 dispositions (Cautious, Bold, Analytical, Empathetic, Pragmatic, Curious, Disciplined, Creative)

---

**Note:** This test file should be updated as the project evolves. When major systems are added or changed, add corresponding test cases to verify the custom agent remains accurate.

**Last Updated:** 2025-11-07
