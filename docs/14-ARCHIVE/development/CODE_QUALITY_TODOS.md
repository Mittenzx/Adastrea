# Code Quality: TODO Management Standards

## Overview
This document outlines standards for TODO comments in the Adastrea codebase to ensure consistent tracking and prioritization of future work.

## TODO Tagging System

### Priority Tags
- `[CRITICAL]`: Must be fixed before release
- `[HIGH]`: Should be addressed in current sprint
- `[MEDIUM]`: Nice to have, can wait
- `[LOW]`: Future enhancement

### Phase Tags
- `[MVP-PHASE-1]`: Trade Simulator MVP (current)
- `[MVP-PHASE-2]`: Inventory & Economy
- `[MVP-PHASE-3]`: Combat & Progression
- `[POST-MVP]`: After MVP completion

### Category Tags
- `[ARCHITECTURE]`: Structural/system design
- `[PERFORMANCE]`: Optimization needed
- `[UI/UX]`: User interface improvements
- `[CONTENT]`: Game content/features
- `[BUG]`: Known bug to fix
- `[REFACTOR]`: Code restructuring needed

## Examples

### Good Examples
```cpp
// TODO: [MVP-PHASE-2][MEDIUM] Implement proper material checking when inventory system is ready
// Related: InventorySystem component, MaterialDatabase, PlayerEconomy

// TODO: [ARCHITECTURE][LOW] Way system archived - Feat and Way will be reimplemented in MVP Phase 3
// Original design: Skill tree system for player progression

// TODO: [PERFORMANCE][HIGH] Optimize pathfinding algorithm for large sectors
// Current O(n²) complexity, target O(n log n)
```

### Bad Examples
```cpp
// TODO: Fix this later
// TODO: Implement something
// FIXME: This is broken
```

## TODO Review Process

### Weekly Review
1. Run TODO scan: `find_todos.py` (in development)
2. Categorize by priority and phase
3. Update project backlog
4. Assign to appropriate sprints

### Before Release
1. All `[CRITICAL]` TODOs must be resolved
2. `[HIGH]` TODOs should be addressed or documented as known issues
3. Update TODO status in release notes

## Tools

### TODO Scanner (Planned)
```bash
# Will scan for TODOs and generate report
python scripts/find_todos.py --categorize --report
```

### Current Manual Process
```bash
# Find all TODOs in source files
grep -r "TODO" Source/ --include="*.cpp" --include="*.h"
```

## Best Practices

1. **Always include context** - What needs to be done and why
2. **Tag appropriately** - Use priority and phase tags
3. **Reference related systems** - Mention connected components
4. **Keep TODOs actionable** - Clear next steps
5. **Regular cleanup** - Remove resolved TODOs promptly

## File Structure
- `docs/development/CODE_QUALITY_TODOS.md` - This document
- `docs/todo-backlog/` - Archived TODO lists by date
- `scripts/find_todos.py` - TODO scanning utility (planned)

---

**Last Updated**: March 8, 2026 00:10 GMT  
**Maintained by**: Adastrea Endless Development Agent