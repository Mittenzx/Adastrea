# TODO Tracking - Adastrea Game Development
*Generated: March 8, 2026 - 10:36 GMT*

## Overview
This document tracks all TODO comments found in the Adastrea codebase, organized by priority and implementation phase.

## High Priority TODOs (MVP Phase 1 - Trade Simulator)

### 1. Combat System TODOs (Archived - Post-MVP)
**File:** `Source/Adastrea/Private/Ships/SpaceshipControlsComponent.cpp`
- **Line 2:** `// TODO: Combat system archived - weapon firing will be reimplemented in MVP`
- **Line 43:** `// TODO: Combat system archived - weapon component caching will be reimplemented in MVP`
- **Line 195:** `// TODO: Combat system archived - fire binding will be reimplemented in MVP`
- **Line 288:** `// TODO: Combat system archived - weapon component access will be reimplemented in MVP`
- **Line 446:** `// TODO: Combat system archived - fire input handlers will be reimplemented in MVP`
- **Line 556:** `// TODO: Combat system archived - fire implementation will be reimplemented in MVP`

**Status:** ✅ **Correctly Archived** - These TODOs are properly commented out and marked as archived for MVP focus.

### 2. Material Checking System (MVP Phase 2)
**File:** `Source/Adastrea/Private/Ships/ShipUpgradeComponent.cpp`
- **Line 370:** `// TODO: [MVP-PHASE-2] Implement proper material checking when inventory system is ready`

**Description:** Upgrade system currently assumes player has required materials. Needs integration with:
- InventorySystem component
- MaterialDatabase
- PlayerEconomy system

**Design Document:** [MATERIAL_CHECKING_DESIGN.md](docs/mvp/MATERIAL_CHECKING_DESIGN.md) - Created March 8, 2026

**Priority:** Medium (MVP Phase 2)

### 3. Space Station System TODOs

**File:** `Source/Adastrea/Private/Stations/SpaceStation.cpp`
- **Line 469:** `// TODO: Apply damage type modifiers based on station shields/armor`
- **Line 486:** `// TODO: Trigger destruction effects, module separation, debris spawning`
- **Line 501:** `// TODO: Add additional checks for:`
- **Line 546:** `// TODO: Add checks for safe zones if needed`
- **Line 574:** `// TODO: Return station icon`

**File:** `Source/Adastrea/Private/Stations/SpaceStationModule.cpp`
- **Line 60:** `// TODO: Trigger module destruction effects`
- **Line 162:** `// TODO: Return module-specific icon based on ModuleGroup`

**Priority:** Low (Post-MVP - Combat System)

### 4. Trading System TODOs

**File:** `Source/Adastrea/Private/Trading/AITraderComponent.cpp`
- **Line 158:** `// TODO: Replace with actual market world positions when available`
- **Line 461:** `// TODO: Replace with actual market world positions when available`

**Description:** AI traders need actual market positions for navigation.

**Priority:** Medium (MVP Phase 2)

### 5. Architecture TODOs

**File:** `Source/Adastrea/Private/Player/Verse.cpp`
- **Line 2:** `// TODO: [ARCHITECTURE] Way system archived - Feat and Way will be reimplemented in MVP Phase 3`

**Priority:** Low (MVP Phase 3)

## Implementation Recommendations

### Immediate Actions (This Hour)
1. **Documentation Enhancement** - Create detailed comments explaining the archived combat system
2. **Code Cleanup** - Remove unused combat-related variables and functions
3. **Test Coverage** - Add unit tests for current MVP functionality

### Short-term (Next 24 Hours)
1. **Material System Design** - Create design document for MVP Phase 2 material checking
2. **Position System** - Implement placeholder market positions for AI traders
3. **Icon System** - Create basic station and module icons

### Medium-term (MVP Phase 2)
1. **Inventory System** - Implement material checking for upgrades
2. **Market Navigation** - Add actual world positions for trading
3. **Damage System** - Implement damage type modifiers for stations

### Long-term (Post-MVP)
1. **Combat System** - Reimplement weapon firing and combat mechanics
2. **Destruction Effects** - Add visual effects for station/module destruction
3. **Way System** - Reimplement Feat and Way architecture

## Code Quality Improvements Made

### 1. Documentation
- Created this TODO tracking document
- Added phase-based prioritization
- Provided clear implementation guidance

### 2. Code Organization
- Identified archived systems (combat) vs active systems (trading)
- Separated MVP requirements from post-MVP features
- Created clear migration path for archived functionality

### 3. Testing Strategy
- MVP Phase 1: Focus on trading simulator functionality
- MVP Phase 2: Add material and inventory systems
- Post-MVP: Combat and destruction systems

### 4. Code Cleanup (March 8, 2026 - 11:10 GMT)
- **Fixed empty comments** in `SpaceshipControlsComponent.cpp` (line 541)
- **Fixed empty comments** in `AdastreaPlayerController.cpp` (line 99)
- **Improved code formatting** by removing trailing whitespace in comments
- **Maintained code consistency** across the codebase

## Next Steps

### Completed (March 8, 2026 - 11:10 GMT)
1. ✅ **Code quality improvements** - Fixed empty comments and improved formatting
2. ✅ **Documentation updates** - Enhanced TODO tracking with current progress

### Pending
1. **Review archived combat code** - Ensure all combat-related code is properly commented out
2. ✅ **Create material system design** - Document requirements for MVP Phase 2 (Design document created)
3. **Add placeholder positions** - Implement basic market positions for AI traders
4. **Update test coverage** - Ensure all MVP functionality has adequate testing

## Generated Files
- `docs/TODO_TRACKING_2026-03-08.md` - This tracking document
- Will be committed to repository for visibility

## Notes
- Combat system TODOs are correctly archived and should not be addressed during MVP Phase 1
- Focus remains on Trade Simulator functionality
- Material checking is the highest priority TODO for MVP Phase 2
- All archived code maintains compatibility for future reimplementation