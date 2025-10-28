# What's Still Needed

This file collates 'What's Still Needed', TODO, missing, or incomplete sections from markdown files across the repository. Each entry includes its source file for context.

---

## Summary

After searching all markdown documentation files in the repository, the following items were found:

- **1 explicit "What's Still Needed" section** in Assets/IterationGuide.md
- **602 total unchecked checklist items** across multiple documentation files
- **2 "TO BE IMPLEMENTED" markers** in code architecture documentation
- **2 TODO comments** in example code snippets

---

## Explicit "What's Still Needed" Sections

### Assets/IterationGuide.md (Lines 47-54)

**Context:** Iteration Guide for Expanding Faction Behaviors, Trait-Driven Gameplay, and Diplomacy

**What's Still Needed:**

- ⏸ Runtime faction state management
- ⏸ Dynamic relationship changes
- ⏸ Trait-driven gameplay effects
- ⏸ Active diplomacy interactions
- ⏸ AI faction behaviors
- ⏸ Player reputation system

---

## "TO BE IMPLEMENTED" Markers

### Assets/IterationGuide.md (Line 65)

**Context:** Architectural Pattern section

```
FactionStateComponent (Runtime State) [TO BE IMPLEMENTED]
```

This component is planned to bridge the static FactionDataAsset templates with runtime state management for dynamic gameplay.

---

## Implementation Priority Checklists

### Assets/IterationGuide.md (Lines 913-954)

**Priority 1: Essential Foundation (Weeks 1-2)**
- [ ] Implement UFactionRuntimeState for dynamic state
- [ ] Implement UPlayerReputationComponent
- [ ] Create basic reputation change system
- [ ] Test reputation affecting trade prices
- [ ] Test reputation affecting NPC hostility

**Priority 2: Core Diplomacy (Weeks 3-4)**
- [ ] Implement UFactionDiplomacyManager
- [ ] Create war/peace declaration system
- [ ] Create alliance formation system
- [ ] Add diplomacy event triggers
- [ ] Create basic diplomatic missions (2-3 types)

**Priority 3: Trait Expansion (Weeks 5-6)**
- [ ] Add trait categories and tags
- [ ] Implement runtime trait addition/removal
- [ ] Create trait template library (20+ traits)
- [ ] Integrate traits with existing systems (trade, combat, research)
- [ ] Add visual feedback for active traits

**Priority 4: Dynamic Behaviors (Weeks 7-10)**
- [ ] Implement sector control system
- [ ] Create faction fleet AI
- [ ] Implement basic economic simulation
- [ ] Add territory change events
- [ ] Create faction evolution system

**Priority 5: Polish and Content (Weeks 11-12)**
- [ ] Create faction storylines (one per major faction)
- [ ] Add faction-specific missions (10+ per faction)
- [ ] Polish UI for reputation and diplomacy
- [ ] Add audio/visual feedback for major events
- [ ] Balance all systems
- [ ] Performance optimization

---

## Files with Significant Unchecked Checklists

The following files contain substantial implementation checklists that represent planned features, testing procedures, or workflow steps:

1. **Assets/PlaytestingChecklist.md** - 276 unchecked items
   - Comprehensive playtesting procedures for all game systems

2. **Assets/SpaceshipDesignerWorkflow.md** - 32 unchecked items
   - Designer workflow steps and validation procedures

3. **Assets/IterationGuide.md** - 26 unchecked items
   - Faction system expansion roadmap (detailed above)

4. **Blueprints/TradingSystemBlueprintGuide.md** - 24 unchecked items
   - Trading system implementation steps and feature additions

5. **Assets/DesignerOnboarding.md** - 22 unchecked items
   - Onboarding tasks for new designers

6. **Assets/TRADING_IMPLEMENTATION_SUMMARY.md** - 20 unchecked items
   - Trading system feature completion tracking

7. **Assets/TradingDesignerWorkflow.md** - 19 unchecked items
   - Trading content creation workflow

8. **Assets/MaterialStationIntegration.md** - 18 unchecked items
   - Material system integration tasks

9. **Assets/HOMEWORLD_IMPLEMENTATION_SUMMARY.md** - 18 unchecked items
   - Homeworld system implementation tracking

10. **Assets/MaterialSystemGuide.md** - 15 unchecked items
    - Material system features and workflows

For the complete list of all 602 unchecked items, please review individual documentation files.

---

## TODO Comments in Example Code

### Blueprints/TradingSystemBlueprintGuide.md (Line 80)

**Context:** Blueprint trading flow example

```
└── TODO: Implement actual purchase logic
```

### CODE_STYLE.md (Line 315)

**Context:** Code style documentation example

```cpp
// TODO: Implement advanced feature
```

---

## Recommendations

1. **Add "What's Still Needed" Sections**: Consider adding explicit "What's Still Needed" sections to the following key documentation files for better future tracking:
   - README.md (project-level missing features)
   - ARCHITECTURE.md (architectural components not yet implemented)
   - CONTRIBUTING.md (contribution processes or tools still needed)
   - Individual system guides that don't yet have implementation checklists

2. **Consolidate Implementation Tracking**: The current implementation checklists are spread across multiple files. Consider:
   - Creating a centralized ROADMAP.md for project-wide feature tracking
   - Linking to GitHub Issues or Projects for better task management
   - Regular updates to this file as items are completed

3. **Prioritize High-Impact Items**: Focus on the Priority 1 items in Assets/IterationGuide.md as they form the foundation for other features:
   - Runtime faction state management
   - Player reputation system
   - Basic diplomacy mechanics

4. **Review Playtesting Checklist**: The 276-item playtesting checklist in Assets/PlaytestingChecklist.md is comprehensive but may need periodic review to ensure it remains current with implemented features.

---

## GitHub Search Links

For deeper exploration and to search for specific patterns:

- [All markdown files in repository](https://github.com/search?q=repo%3AMittenzx%2FAdastrea+extension%3Amd&type=code)
- [Search for "TODO" in markdown](https://github.com/search?q=repo%3AMittenzx%2FAdastrea+TODO+extension%3Amd&type=code)
- [Search for unchecked items](https://github.com/search?q=repo%3AMittenzx%2FAdastrea+%22-%20%5B%20%5D%22+extension%3Amd&type=code)
- [Search for "What's Still Needed"](https://github.com/search?q=repo%3AMittenzx%2FAdastrea+%22What%27s+Still+Needed%22+extension%3Amd&type=code)
- [Search for "TO BE IMPLEMENTED"](https://github.com/search?q=repo%3AMittenzx%2FAdastrea+%22TO+BE+IMPLEMENTED%22+extension%3Amd&type=code)

---

_Last updated: 2025-10-28_
_Total markdown files analyzed: 50_
_This file is generated from repository documentation and should be updated as features are implemented._
