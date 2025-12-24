# ROADMAP Update Guidelines

This document provides clear guidelines on when and how to update the project roadmaps. **As of December 2025, we now have multiple focused roadmaps** in addition to the main roadmap.

## Roadmap Structure

The project now has **6 roadmap files**:

1. **[ROADMAP.md](ROADMAP.md)** - Main project roadmap (high-level)
2. **[ROADMAP_CORE_SYSTEMS.md](ROADMAP_CORE_SYSTEMS.md)** - AI, Combat, Navigation, Quest, Exploration
3. **[ROADMAP_CONTENT_CREATION.md](ROADMAP_CONTENT_CREATION.md)** - Blueprints, Data Assets, Maps, UI
4. **[ROADMAP_ECONOMY_TRADING.md](ROADMAP_ECONOMY_TRADING.md)** - Trading, Markets, Factions, Ways
5. **[ROADMAP_STATION_PLAYER.md](ROADMAP_STATION_PLAYER.md)** - Stations, Progression, Inventory, HUD
6. **[ROADMAP_TECHNICAL.md](ROADMAP_TECHNICAL.md)** - Tools, Testing, CI/CD, Documentation

**When to update which roadmap**:
- Update **ROADMAP.md** for major milestones and phase completions
- Update **specific area roadmaps** for detailed progress in that domain
- You may need to update **multiple roadmaps** if your work spans areas

## Table of Contents

- [When to Update Roadmaps](#when-to-update-roadmaps)
- [Which Roadmap to Update](#which-roadmap-to-update)
- [What to Update](#what-to-update)
- [How to Update](#how-to-update)
- [Update Checklist](#update-checklist)
- [Examples](#examples)

---

## When to Update Roadmaps

Update roadmaps when your pull request includes changes that affect:

### ✅ Update Required

1. **System Completion or Major Progress**
   - Completing a system listed in the "In Progress" section
   - Achieving a significant milestone (e.g., 50% → 80% completion)
   - Moving a system from "Planned" to "In Progress"

2. **Phase Transitions**
   - Completing all milestones for a development phase
   - Starting a new development phase ahead of schedule
   - Changing phase timelines or objectives

3. **New Major Features**
   - Adding a new system not previously mentioned in the roadmap
   - Implementing significant functionality that affects multiple systems
   - Adding new tools or automation that impacts development workflow

4. **Timeline Changes**
   - Significant delays or acceleration of planned features
   - Adjusting quarterly milestone dates
   - Reprioritizing development phases

5. **System Maturity Changes**
   - Updating the System Maturity Matrix (completeness percentage)
   - Changing system status (Complete ✅, In Progress 🔄, Planned ⏳)
   - Updating documentation or testing status for systems

### ⏭️ Update Not Required

1. **Minor Bug Fixes**
   - Small bug fixes that don't affect system status
   - Code refactoring without feature changes
   - Internal code improvements

2. **Documentation-Only Changes**
   - Updating guides, tutorials, or examples
   - Fixing typos or improving clarity
   - Adding code comments

3. **Small Incremental Work**
   - Small contributions toward a larger feature
   - Minor enhancements to existing systems
   - Regular maintenance tasks

4. **Build/CI Changes**
   - Build system improvements
   - CI/CD pipeline updates
   - Testing infrastructure changes

**When in doubt**: Ask yourself, "Would someone looking at the roadmaps care about this change?" If yes, update the relevant roadmap(s).

---

## Which Roadmap to Update

Different changes require updates to different roadmaps:

### Main Roadmap (ROADMAP.md)
**Update when**:
- Completing an entire development phase
- Major milestone affecting the whole project
- Changing project-level priorities or timelines
- Adding a new major system not covered by existing roadmaps

**Example changes**:
- "Phase 3 Advanced Systems now complete"
- "Added new Weather System as Phase 4 milestone"
- "Shifted Phase 5 timeline by 1 quarter"

### Core Systems Roadmap (ROADMAP_CORE_SYSTEMS.md)
**Update when working on**:
- AI System (NPCLogicBase, FactionLogic, PersonnelLogic)
- Combat System (weapons, targeting, damage, boarding)
- Navigation System (pathfinding, autopilot, waypoints)
- Quest System (quest types, objectives, quest chains)
- Exploration System (scanning, discoveries, anomalies)
- Procedural generation systems

**Example changes**:
- Created 5 weapon Data Assets
- Implemented advanced AI pathfinding
- Completed Phase 2 of Exploration System

### Content Creation Roadmap (ROADMAP_CONTENT_CREATION.md)
**Update when working on**:
- Blueprint Data Assets (ships, factions, personnel, etc.)
- Actor Blueprints (BP_Ship, BP_Station, BP_Module, etc.)
- Game Maps/Levels
- UI Widgets (WBP_*)
- Material Instances
- Audio Assets
- Input Configuration

**Example changes**:
- Created 10 ship Data Assets
- Built TestLevel.umap with stations
- Implemented WBP_HUD_Main widget
- Completed Week 1 of critical foundation

### Economy & Trading Roadmap (ROADMAP_ECONOMY_TRADING.md)
**Update when working on**:
- Trading System (TradingComponent, TradeItemDataAsset, markets)
- Market System (supply/demand, price simulation)
- Faction System (FactionDataAsset, diplomacy, reputation)
- Way System (guilds, WayDataAsset, WayNetworks, Verse)
- Economic simulation and AI traders

**Example changes**:
- Created 10 trade item Data Assets
- Implemented faction reputation system
- Added 5 guild/Way configurations

### Station & Player Systems Roadmap (ROADMAP_STATION_PLAYER.md)
**Update when working on**:
- Space Station System (modules, ASpaceStation, building)
- Player Progression (leveling, skills, unlocks)
- Inventory System (InventoryComponent, InventoryWidget)
- Ship Customization (loadouts, upgrades, visuals)
- HUD System (AdastreaHUDWidget, ship status display)
- Tutorial System (TutorialManagerSubsystem, tutorial steps)

**Example changes**:
- Created 5 station module Blueprints
- Implemented skill tree system
- Built WBP_Inventory widget

### Technical Infrastructure Roadmap (ROADMAP_TECHNICAL.md)
**Update when working on**:
- Development tools (validation scripts, generators)
- Testing framework (unit tests, integration tests)
- CI/CD Pipeline (GitHub Actions workflows)
- Documentation system (guides, templates, wikis)
- Automation scripts (Python utilities)
- Performance profiling tools

**Example changes**:
- Added new validation script
- Set up automated test suite
- Created CI workflow for builds

### Multiple Roadmaps
**Some changes affect multiple roadmaps**. For example:
- **Combat + Content**: Creating weapon Data Assets affects both ROADMAP_CORE_SYSTEMS.md (Combat System) and ROADMAP_CONTENT_CREATION.md (Data Assets)
- **Trading + Economy**: Implementing faction price modifiers affects both ROADMAP_CORE_SYSTEMS.md and ROADMAP_ECONOMY_TRADING.md
- **Stations + Content**: Building station modules affects both ROADMAP_STATION_PLAYER.md and ROADMAP_CONTENT_CREATION.md

**In these cases, update all relevant roadmaps** to keep them synchronized.

---

## What to Update

Depending on your changes, update the relevant sections in the appropriate roadmap(s):

### In Main Roadmap (ROADMAP.md)

Update if completing major milestones or phase transitions:

```markdown
### ✅ Completed Systems (1.0.0-alpha)
- Add newly completed major systems here

### Phase X: Name (Status)
**Timeline**: ...
**Status**: ✅ Complete / 🔄 In Progress (X% complete) / ⏳ Planned

### Last Updated Date
**Last Updated**: [Current Date]
```

### In Area-Specific Roadmaps

Each area roadmap has similar sections to update:

```markdown
## System Name

**Status**: Update completion percentage
**Priority**: Update if priority changed

### Current State
- Update ✅ Completed items
- Update ❌ Missing items

### Roadmap
#### Phase X: Name (Week/Quarter) - Priority: LEVEL
- [x] Mark completed tasks
- [ ] Keep pending tasks

### SUCCESS MILESTONE
- Update when milestone achieved
```

### Last Updated Date

**Always update** the "Last Updated" date in any roadmap you modify:

```markdown
**Last Updated**: December 20, 2025
**Next Review**: [Month Year]
```

---

## How to Update

### Step-by-Step Process

1. **Identify Affected Roadmaps**
   - Which systems did your PR modify?
   - Which content did you create?
   - Which tools did you add?
   - Determine which roadmap(s) need updates

2. **Update Relevant Sections**
   - Open the appropriate roadmap file(s)
   - Find the sections that need updates
   - Make precise, accurate updates
   - Mark tasks as complete with [x]
   - Update status percentages if applicable

3. **Update the Date**
   - Change the "Last Updated" date in each modified roadmap
   - Use format: `December 20, 2025`

4. **Verify Consistency**
   - Ensure all related sections are updated together
   - Check that statuses are consistent across roadmaps
   - Verify timeline dates are accurate

5. **Include in Your PR**
   - Add all modified roadmap files to your commit
   - Mention the roadmap updates in your PR description
   - Check the "Roadmap updated" box in the PR template

### Best Practices

- **Be Accurate**: Only claim completion when features are truly done
- **Be Specific**: Update exact percentages and milestones
- **Be Consistent**: Keep formatting and style matching existing content
- **Be Concise**: Don't add unnecessary detail; maintain readability
- **Be Honest**: Reflect actual progress, not aspirational goals
- **Update Multiple Roadmaps**: If your work spans areas, update all relevant roadmaps

---

## Update Checklist

Use this checklist when updating roadmaps:

- [ ] Identified which roadmap(s) need updates
- [ ] Updated all relevant roadmap files (may be multiple)
- [ ] Marked completed tasks with [x]
- [ ] Updated status percentages if applicable
- [ ] Updated "Last Updated" date in each modified roadmap
- [ ] Verified all changes are consistent and accurate
- [ ] Checked formatting and markdown rendering
- [ ] Included all modified roadmap files in PR commit
- [ ] Mentioned roadmap updates in PR description

---

## Examples

### Example 1: Completing Combat Weapon Content

**Scenario**: You've created 5 weapon Data Assets and 3 projectile Blueprints.

**Updates Needed**:

1. **ROADMAP_CORE_SYSTEMS.md** (Combat System):
   ```markdown
   #### Phase 1: Weapon Content (Q1 2026) - Priority: CRITICAL
   - [x] Create Weapon Data Assets (5 of 12 complete)
     - [x] DA_Weapon_LaserCannon_Basic
     - [x] DA_Weapon_Railgun_Basic
     - [x] DA_Weapon_Missile_Basic
     - [x] DA_Weapon_Plasma_Basic
     - [x] DA_Weapon_Torpedo_Heavy
   - [x] Create projectile Blueprints (3 of 6 complete)
     - [x] BP_Projectile_Laser
     - [x] BP_Projectile_Railgun
     - [x] BP_Projectile_Missile
   ```

2. **ROADMAP_CONTENT_CREATION.md** (Blueprint Data Assets):
   ```markdown
   #### Weapon Data Assets (5 assets) ✅ COMPLETE
   - [x] DA_Weapon_LaserCannon_Basic
   - [x] DA_Weapon_Railgun_Basic
   - [x] DA_Weapon_Missile_Basic
   - [x] DA_Weapon_Plasma_Basic
   - [x] DA_Weapon_Torpedo_Heavy
   
   #### Projectiles (3 Blueprints) ✅ COMPLETE (partial)
   - [x] BP_Projectile_Laser
   - [x] BP_Projectile_Railgun
   - [x] BP_Projectile_Missile
   - [ ] BP_Projectile_Plasma (pending)
   - [ ] BP_Projectile_Torpedo (pending)
   - [ ] BP_Projectile_Beam (pending)
   ```

3. **Both files**:
   ```markdown
   **Last Updated**: December 20, 2025
   ```

---

### Example 2: Implementing Station Module Blueprints

**Scenario**: You've created 5 station module Blueprints and the station editor UI.

**Updates Needed**:

1. **ROADMAP_STATION_PLAYER.md** (Space Station System):
   ```markdown
   ### Space Station System
   **Status**: ✅ 90% Complete (Code), ⚠️ 30% Complete (Content)
   
   #### Phase 1: Basic Modules (Week 1-2) - Priority: CRITICAL
   **Essential Modules** (5 Blueprints) ✅ COMPLETE
   - [x] BP_Module_Docking
   - [x] BP_Module_Power
   - [x] BP_Module_Storage
   - [x] BP_Module_Defence
   - [x] BP_Module_Trade
   
   **Station Editor UI** (1 Widget) ✅ COMPLETE
   - [x] WBP_StationEditor
   
   **SUCCESS MILESTONE**: ✅ Can build basic station with docking, power, storage, and trade
   ```

2. **ROADMAP_CONTENT_CREATION.md** (Actor Blueprints):
   ```markdown
   #### Station Modules (5 Blueprints) ✅ COMPLETE (Phase 1)
   - [x] BP_Module_Docking
   - [x] BP_Module_Power
   - [x] BP_Module_Storage
   - [x] BP_Module_Defence
   - [x] BP_Module_Trade
   ```

3. **ROADMAP.md** (if this is a major milestone):
   ```markdown
   **December 20, 2025**:
   - ✅ Station construction system playable (Phase 1 modules complete)
   ```

---

### Example 3: Adding New Testing Framework

**Scenario**: You've set up a comprehensive unit testing framework with 20 tests.

**Updates Needed**:

1. **ROADMAP_TECHNICAL.md** (Testing Framework):
   ```markdown
   #### Phase 1: Core Testing (Q1 2026) - Priority: HIGH
   
   **Unit Tests** (Setup) ✅ COMPLETE
   - [x] Create C++ unit test framework
   - [x] Create test base classes
   - [x] Set up test discovery
   - [x] Write unit tests for core systems (20 tests complete)
     - [x] Faction system tests (10 tests)
     - [x] Trading system tests (10 tests)
   ```

2. **Only update ROADMAP.md if this completes a major phase**, otherwise just update ROADMAP_TECHNICAL.md

---

### Example 4: NO UPDATE NEEDED (Multiple Roadmaps)

**Scenario**: You've fixed a minor bug in faction relationship calculations.

**Why No Updates**:
- Bug fix doesn't change system completion status
- Faction System is already marked as "Complete" in all roadmaps
- No new features or milestones achieved
- Timeline and phases remain unchanged

**Action**: Skip updating roadmaps, but mention in CHANGELOG.md instead.

---

## Questions?

If you're unsure whether your PR requires a roadmap update or which roadmap(s) to update:

1. **Ask in Your PR**: Mention in the PR description that you're unsure which roadmaps need updates
2. **Ask Maintainers**: Maintainers can advise during PR review
3. **Err on the Side of Updating**: If truly significant, it's better to update than skip
4. **Use the Roadmap Index**: Refer to the section "Which Roadmap to Update" above for guidance

---

## Related Documentation

- [ROADMAP.md](ROADMAP.md) - Main project roadmap
- [ROADMAP_CORE_SYSTEMS.md](ROADMAP_CORE_SYSTEMS.md) - Core gameplay systems
- [ROADMAP_CONTENT_CREATION.md](ROADMAP_CONTENT_CREATION.md) - Blueprints, assets, maps
- [ROADMAP_ECONOMY_TRADING.md](ROADMAP_ECONOMY_TRADING.md) - Economy and trading
- [ROADMAP_STATION_PLAYER.md](ROADMAP_STATION_PLAYER.md) - Stations and player systems
- [ROADMAP_TECHNICAL.md](ROADMAP_TECHNICAL.md) - Infrastructure and tools
- [CHANGELOG.md](CHANGELOG.md) - Detailed version history (for all changes)
- [CONTRIBUTING.md](CONTRIBUTING.md) - General contribution guidelines
- [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md) - PR checklist

---

**Remember**: The roadmaps are high-level views of project direction in specific areas. Keep them accurate, but don't stress over minor changes. Focus on significant milestones and system status updates. **When in doubt, update the specific area roadmap rather than the main roadmap.**
