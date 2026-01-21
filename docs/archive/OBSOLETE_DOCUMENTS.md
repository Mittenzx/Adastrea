# Obsolete and Archived Documents

**Last Updated**: January 21, 2026  
**Purpose**: Track documents that are no longer actively used but retained for reference

## 🆕 Latest Update: Pre-MVP Systems Archive (January 21, 2026)

**Non-MVP system documentation archived to focus on Trade Simulator MVP**

### What Happened
- **Archived**: 44 non-MVP system guides from `Assets/` → `docs/archive/pre-mvp-systems/assets-guides/`
- **Archived**: `MASTER_DOCUMENTATION.md` (pre-pivot, Dec 9, 2025) → `docs/archive/pre-mvp-systems/`
- **Focus**: All documentation now aligned with Trade Simulator MVP (Dec 2025 - Mar 2026)
- **Result**: Clean separation between active MVP docs and deferred system docs

**Systems Archived**:
- Exploration & Discovery (2 files)
- Sector & Map Systems (9 files)
- Progression & Player Systems (5 files)
- Visual Effects & Polish (4 files)
- Advanced Ship Systems (3 files)
- Advanced UI Systems (7 files)
- Guild & Network Systems (4 files)
- Settings & Data Management (9 files)
- AI Designer Workflows (1 file)
- Tutorial System (1 file)
- Master Documentation (1 file)

**Total**: 44 files from Assets/, 1 file from root = 45 files archived

See [Pre-MVP Systems Archive README](docs/archive/pre-mvp-systems/README.md) for complete details.

---

## 🆕 Previous Update: Repository Modernization (January 14, 2026)

**Documentation directory consolidation and critical review archival**

### What Happened
- **Consolidated**: `Docs/` directory (capital D, 49 files) → `docs/archive/legacy-Docs/`
- **Archived**: Critical review documents (5 files) → `docs/archive/critical-review-2025/`
- **Archived**: `CURRENT_STATUS.md` (historical snapshot) → critical review archive
- **Created**: `GETTING_STARTED.md` - Modern MVP-focused entry point
- **Result**: Single documentation directory (`docs/`), cleaner root, MVP-focused navigation

See details below for complete list of archived files.

---

## Overview

This document tracks obsolete documentation that has been removed or archived. Major cleanups occurred on December 24, 2025, January 13, 2026, January 14, 2026, and January 21, 2026.

---

## Archived Directories and Files (January 21, 2026)

### Pre-MVP Systems Archive

**Reason**: Focus documentation on Trade Simulator MVP only, defer non-MVP systems

**Action**: Archived 45 files (44 from Assets/, 1 from root) to `docs/archive/pre-mvp-systems/`

#### What Was Archived from Assets/

**Exploration & Discovery Systems (2 files):**
- `DiscoverySystemQuickStart.md` - Discovery mechanics
- `NavigationSystemGuide.md` - Advanced navigation UI

**Sector & Map Systems (9 files):**
- `HomeworldSystemGuide.md` - Player homeworld
- `SECTOR_SYSTEM_SUMMARY.md` - Sector universe structure
- `SectorCreationGuide.md` - Manual sector creation
- `SectorGeneratorGuide.md` - Procedural generation
- `SectorGeneratorQuickStart.md` - Generator quick start
- `SectorMapBuildingGuide.md` - Map building
- `SectorMapPlacementGuide.md` - Object placement
- `SectorSystemQuickReference.md` - Sector reference
- `MapSystemArchitecture.md`, `MapSystemGuide.md`, `MapSystemQuickStart.md` - Map system docs

**Progression & Player Systems (5 files):**
- `PHASE4_QUICK_REFERENCE.md` - Phase 4 features (post-MVP)
- `PROGRESSION_SYSTEM_SUMMARY.md` - Player progression
- `Phase4SystemsSummary.md` - Phase 4 overview
- `PlayerProgressionGuide.md` - XP, levels, unlocks
- `InventorySystemGuide.md` - Inventory beyond cargo

**Visual & Polish (4 files):**
- `SpaceshipParticleQuickReference.md` - Particle effects
- `SpaceshipParticleSetupWorkflow.md` - Particle setup
- `SpaceshipParticleSystemGuide.md` - Particle guide
- `TutorialSystemGuide.md` - Tutorial framework

**Advanced Ship Systems (3 files):**
- `ModularShipSystemGuide.md` - Modular construction
- `ShipCustomizationGuide.md` - Ship customization UI
- `SpaceshipInteriorGuide.md` - Interior views

**Advanced UI Systems (7 files):**
- `HUDSystemGuide.md` - Advanced HUD
- `HUD_QUICK_REFERENCE.md` - HUD reference
- `ShipStatusScreenImplementation.md` - Status UI
- `ShipStatusScreenSetup.md` - Status setup

**Guild & Network Systems (4 files):**
- `WayNetworkWorkflow.md`, `WaySystemAPIReference.md`, `WaySystemOverview.md`, `WaySystemQuickReference.md` - Way system docs

**Settings & Data Management (9 files):**
- `ADVANCED_DATA_MANAGEMENT.md` - Advanced data patterns
- `SaveSystemGuide.md` - Save/load system
- `TestSettings*.md` (5 files) - Settings UI system
- `PerformanceOptimizationGuide.md` - Optimization guide

**AI & Designer Workflows (1 file):**
- `AIDesignerWorkflow.md` - AI content workflow

#### What Was Archived from Root

**Pre-Pivot Master Documentation (1 file):**
- `MASTER_DOCUMENTATION.md` (December 9, 2025) - Comprehensive pre-pivot documentation covering all 22 systems

**New Location**: `docs/archive/pre-mvp-systems/`  
**Documentation**: Complete README with archive context and post-MVP roadmap  
**Status**: ✅ Archived - Preserved for post-MVP development (if MVP succeeds)

**Key Decision**: These are excellent systems but wrong timing. Focus on gameplay validation first, then expand if successful.

---

## Archived Directories and Files (January 14, 2026)

### Documentation Directory Consolidation

**Reason**: Eliminate confusion between `Docs/` (capital D) and `docs/` (lowercase d) directories

**Action**: Entire `Docs/` directory (49 files) moved to `docs/archive/legacy-Docs/`

#### What Was Archived
- **Player Controls & HUD** (6 files)
  - `PLAYER_CONTROLS_*.md` (3 files) - Control schemes and workflows
  - `HUD_*.md` (3 files) - HUD implementation guides
  - `ForwardSpeed*.md` (3 files) - Forward speed indicator feature
  
- **Map System** (3 files)
  - `MapSystem*.md` - Map system implementation and examples
  
- **Test Mode** (3 files)
  - `TestMode_*.md` - Test mode setup and quick reference
  
- **Unreal Directive** (3 files)
  - `UNREAL_DIRECTIVE_*.md` - Implementation guides for best practices
  
- **Other Documentation** (8 files)
  - `AUTOMATION_SETUP.md`
  - `BEST_PRACTICES_CLASS_SETUP.md`
  - `CLASS_SETUP_AUDIT_SUMMARY.md`
  - `CPP_CODE_QUALITY_REPORT.md`
  - `EmptyMapSetup.md`
  - `FACTION_REPUTATION_QUICK_START.md`
  - `README.md` (Docs/ index)
  
- **Visual Assets** (2 files)
  - `station_editor_*.png` - Station editor mockup images
  
- **Nested Archive** (23 files)
  - `Docs/archive/migrated/` - Previously archived documents

**New Location**: `docs/archive/legacy-Docs/`  
**Documentation**: `docs/archive/legacy-Docs/README.md` explains content and context  
**Status**: ✅ Archived - No longer maintained, preserved for reference

### Critical Review Archive

**Reason**: Historical documents from December 2025 review, completed and led to MVP pivot

**Action**: Moved to `docs/archive/critical-review-2025/`

#### What Was Archived
1. **CRITICAL_REVIEW_SUMMARY.md** (9KB)
   - One-page executive summary
   - Verdict: DO NOT FUND (current state Dec 2025)
   - Led to Trade Simulator MVP pivot

2. **CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md** (27KB)
   - Detailed publisher perspective
   - Why publishers won't fund (0/10 criteria met)
   - Competitive analysis vs Elite/X4/Star Citizen

3. **CRITICAL_REVIEW_TECHNICAL_ISSUES.md** (21KB)
   - Technical debt analysis
   - 15 specific problems
   - 890-1360 estimated hidden issues

4. **CRITICAL_REVIEW_ACTION_PLAN.md** (13KB)
   - 12-week recovery roadmap
   - Trade Simulator MVP chosen
   - Success metrics defined

5. **CRITICAL_REVIEW_README.md** (9KB)
   - Navigation guide for review documents

6. **CURRENT_STATUS.md** (Historical)
   - December 2025 status snapshot
   - Honest project assessment
   - Basis for critical review

**New Location**: `docs/archive/critical-review-2025/`  
**Documentation**: `docs/archive/critical-review-2025/README.md` with full context  
**Status**: ✅ Completed - Historical turning point preserved

### New Documentation Created

**GETTING_STARTED.md** (Root)
- Modern MVP-focused entry point
- Replaces multiple outdated getting-started guides
- Quick paths for different user types (players, developers, content creators)
- Current Trade Simulator MVP status and timeline
- Links to essential documentation

**Impact**: Root directory now has 11 markdown files (down from 16)

---

## Archived Files (January 13, 2026)

### Build Artifacts (Excluded from Git)
**Reason**: Build logs should never be committed to source control
- BuildLog.json, BuildLog2.json
- BuildLog.txt, BuildLog2.txt
- BuildLog.uba, BuildLog2.uba

**Action**: Added `BuildLog*.json`, `BuildLog*.txt`, `BuildLog*.uba` to `.gitignore`

### Generated Reports (Archived to Archive/Reports/)
**Reason**: One-time generated reports, snapshots in time

**content_organization_report.txt**
- Generated report from automated content organization
- Statistics: 227 files analyzed, 226 moves planned
- Task completed, current organization is documented elsewhere

**YAML_FILES_LIST.csv**
- Point-in-time inventory of YAML templates
- 30 files cataloged (spaceships, personnel)
- Current templates are in `Assets/` subdirectories

**Location**: `Archive/Reports/` with README explaining each file

### One-Time Scripts (Archived to Archive/Scripts/)
**Reason**: Scripts used once for specific migration tasks

**move-and-update-includes.sh**
- Migrated MyFirstSpaceshipDataAsset.h to Private/Ships
- Task completed, no longer needed

**reingest_docs.bat**
- Setup script for AdastreaDirector plugin
- Plugin no longer exists in repository

**setup_adastrea_director.bat**
- Interactive setup for AdastreaDirector plugin
- Plugin no longer exists in repository

**regenerate_project.bat**
- Project file generation with hardcoded paths
- Use Unreal Engine's native project generation instead

**Location**: `Archive/Scripts/` with README explaining each file

### Documentation Cleanup

**TOOLS_REORGANIZATION_SUMMARY.md**
- Summary document from December 24, 2025 tool reorganization
- Task completed, information preserved in git history
- **Location**: `Archive/Documentation/`

---

## Why Mark as Obsolete?

Documents were removed when they:
- Contained temporary status information (code reviews, summaries)
- Had been superseded by better documentation
- Were historical logs or reports with no ongoing value
- Contained outdated or redundant information
- Were subsets of more comprehensive guides
- Duplicated existing comprehensive documentation

## Removed Documents (December 24, 2025)

### Code Reviews and Status Reports (10 files)
**Reason**: Temporary documents for specific PRs or reviews
- CODE_REVIEW_ACTION_ITEMS.md
- CODE_REVIEW_FINAL.md
- CODE_REVIEW_QUICK_REFERENCE.md
- CODE_REVIEW_REPORT.md
- CODE_REVIEW_SUMMARY.md
- COMMIT_C35BCA0_REVIEW.md
- COMPLETE_REVIEW_SUMMARY.md
- COPILOT_MEMORY_REVIEW.md
- EDITOR_CONFIG_REVIEW.md
- PR_327_REVIEW_AND_RECOMMENDATIONS.md

### Obsolete Summary Documents (16 files)
**Reason**: One-time summaries superseded by current documentation
- BLUEPRINT_QUICK_START_SUMMARY.md
- BP_IMPORTAI_SUMMARY.md
- BUILD_FIX_SUMMARY.md
- COMPILATION_FIXES_SUMMARY.md
- CONTENT_GENERATION_SUMMARY.md
- CONTENT_GENERATION_VISUAL_SUMMARY.md
- MAP_SYSTEM_SUMMARY.md
- PLUGIN_EXPLORATION_SUMMARY.md
- PLUGIN_VISUAL_SUMMARY.md
- PR_327_QUICK_SUMMARY.md
- STATION_EDITOR_CPP_IMPLEMENTATION_SUMMARY.md
- STATION_EDITOR_IMPLEMENTATION_SUMMARY.md
- TAB_MENU_IMPLEMENTATION_SUMMARY.md
- TEST_SUITE_SUMMARY.md
- UE_BUILD_TOOLS_IMPLEMENTATION_SUMMARY.md
- WIDGET_IMPLEMENTATION_SUMMARY.md

### Obsolete Implementation Guides (8 files)
**Reason**: Features now documented in comprehensive guides
- BP_IMPORTAI_IMPLEMENTATION_GUIDE.md
- CONTENT_ORGANIZATION_IMPLEMENTATION.md
- EDITOR_IMPLEMENTATION_GUIDE.md
- FORWARD_SPEED_IMPLEMENTATION.md
- FREE_LOOK_IMPLEMENTATION.md
- SECTOR_GENERATOR_IMPLEMENTATION.md
- STATION_EDITOR_CPP_IMPLEMENTATION.md
- UNREAL_DIRECTIVE_IMPLEMENTATION_PLAN.md

### Duplicate Blueprint Documentation (14 files)
**Reason**: Consolidated into BLUEPRINT_GUIDE_FOR_BEGINNERS.md
**Kept**: BLUEPRINT_GUIDE_FOR_BEGINNERS.md, BLUEPRINT_QUICK_REFERENCE.md, BLUEPRINT_REQUIREMENTS_LIST.md
- BLUEPRINT_CHECKLIST.md
- BLUEPRINT_CPP_IMPROVEMENTS.md
- BLUEPRINT_CREATION_GUIDE.md
- BLUEPRINT_DOCUMENTATION_UPDATE_PLAN.md
- BLUEPRINT_GRAPH_VISUAL_GUIDE.md
- BLUEPRINT_GUIDES_INDEX.md
- BLUEPRINT_LIST_FOR_PIE.md
- BLUEPRINT_PIE_CHECKLIST.md
- BLUEPRINT_PIE_DIAGRAM.md
- BLUEPRINT_QUICK_START_PIE.md
- BLUEPRINT_ROADMAP_VISUAL.md
- BLUEPRINT_ZERO_SETUP_GUIDE.md
- VISUAL_BLUEPRINT_SETUP.md

### Obsolete Testing/PIE Documentation (5 files)
**Reason**: Superseded by comprehensive testing guides
**Kept**: TESTING_AUTOMATION_GUIDE.md, TESTING_COMPREHENSIVE.md, TEST_ARCHITECTURE.md
- BP_IMPORTAI_TESTING_CHECKLIST.md
- PIE_FIX_CHECKLIST.md
- PIE_SETUP_CHECKLIST.md
- QUICK_PIE_FIX.md
- RUN_PIE_SETUP.md

### Duplicate Station Editor Documentation (4 files)
**Reason**: Consolidated into STATION_EDITOR_COMPLETE_GUIDE.md
**Kept**: STATION_EDITOR_COMPLETE_GUIDE.md, STATION_EDITOR_VISUAL_GUIDE.md
- STATION_EDITOR_CPP_BLUEPRINT_GUIDE.md
- STATION_EDITOR_CPP_QUICK_SETUP.md
- STATION_EDITOR_CPP_README.md
- STATION_EDITOR_KEY_UPDATE_INSTRUCTIONS.md

### Duplicate Content Generation Documentation (9 files)
**Reason**: Consolidated into fewer comprehensive guides
**Kept**: MASSIVE_CONTENT_GENERATION_GUIDE.md, PROCEDURAL_GENERATION_GUIDE.md, SCENE_POPULATION_GUIDE.md
- CONTENT_GENERATION_GUIDE.md
- CONTENT_GENERATION_INDEX.md
- CONTENT_GENERATORS_README.md
- FREE_CONTENT_GENERATION_GUIDE.md
- GENERATOR_QUICK_START.md
- GENERATOR_SCRIPTS_README.md
- PROCEDURAL_GENERATION_EXAMPLES.md
- PYTHON_GENERATORS_README.md
- SECTOR_GENERATOR_README.md

### Research and Analysis (2 files)
**Reason**: Interesting but not needed for project documentation
- X4_FOUNDATIONS_RESEARCH.md
- X4_ISSUE_RESPONSE.md

### Build/Error Documentation (6 files)
**Reason**: Temporary fixes, now resolved or in Git history
- build_errors.md
- check_docker_setup_results.md
- BUILD_FIXES_QUICK_REF.md
- COMPILATION_ERROR_PREVENTION.md
- CONTAINER_DIAGNOSTICS.md
- FixBlueprintParent.md

### Obsolete Quickstart Guides (2 files)
**Reason**: Tools no longer used or integrated into better guides
- BP_IMPORTAI_QUICKSTART.md
- FAST_FORWARD_QUICK_START.md

### Other Obsolete Documents (4 files)
- FREE_LOOK_MAINTAINER_CHECKLIST.md (temporary checklist)
- OBSOLETE_GUIDES_REMOVED.md (meta-document, no longer needed)

## Archived Documents (December 9, 2025)

**Status**: ✅ Previously archived to `Docs/archive/migrated/`

21 historical status and completion documents were archived in December 2025:
- Status documents: PROJECT_STATUS_AND_IMPROVEMENTS.md, TASK_COMPLETION_SUMMARY.md, etc.
- Completion summaries: IMPLEMENTATION_COMPLETE.md, PHASE2_SUMMARY.md, etc.
- Fix summaries: CIRCULAR_DEPENDENCY_FIX_SUMMARY.md, PIE_FIX_SUMMARY.md, etc.

See [Docs/archive/migrated/README.md](Docs/archive/migrated/README.md) for complete list.

These files were superseded by:
- **[MASTER_DOCUMENTATION.md](MASTER_DOCUMENTATION.md)** - Comprehensive documentation
- **[CURRENT_STATUS.md](CURRENT_STATUS.md)** - Realistic project status
- **Git commit history** - Detailed change tracking

## New Documentation Structure (December 24, 2025)

All remaining documentation has been organized into:

### Essential Files (Root Directory)
Only 11 essential files remain in root:
- README.md
- ARCHITECTURE.md
- CODE_STYLE.md
- CONTRIBUTING.md
- CHANGELOG.md
- ROADMAP.md
- QUICKSTART.md
- CURRENT_STATUS.md
- MASTER_DOCUMENTATION.md
- DOCUMENTATION_INDEX.md
- OBSOLETE_DOCUMENTS.md (this file)

### Organized Documentation (docs/ Directory)
- `docs/systems/` - Game system documentation
- `docs/development/` - Developer guides (26 files)
- `docs/setup/` - Installation and setup guides (13 files)
- `docs/testing/` - Testing documentation (3 files)
- `docs/content-generation/` - Content creation tools (11 files)
- `docs/reference/` - Quick references and cheatsheets (19 files)
- `docs/archive/` - Historical documents (6 files)

See [docs/README.md](docs/README.md) for complete navigation guide.

### Unchanged Documentation
- `Assets/` - System-specific guides and templates (102 files)
- `Plugins/` - Plugin documentation (40 files)
- `wiki/` - GitHub wiki pages (12 files)
- `.github/` - CI/CD and GitHub configuration (17 files)

## Summary Statistics

### Before First Cleanup (December 23, 2025)
- **Root directory**: 172 markdown files
- **Total**: 413 markdown files
- **Status**: Cluttered, many duplicates and obsolete files

### After First Cleanup (December 24, 2025)
- **Root directory**: 11 essential files (reduced by 93%)
- **Organized docs/**: 78 files in structured folders
- **Removed**: 80+ obsolete files
- **Total**: ~330 markdown files
- **Status**: Clean, organized, easy to navigate

### After Second Cleanup (January 13, 2026)
- **Build logs**: Excluded from git tracking (added to .gitignore)
- **Reports archived**: 2 generated reports → `Archive/Reports/`
- **Scripts archived**: 4 one-time scripts → `Archive/Scripts/`
- **Documentation archived**: 1 summary document → `Archive/Documentation/`
- **Status**: Root directory further cleaned of temporary files

### After Third Cleanup (January 14, 2026)
- **Docs/ directory**: Consolidated to `docs/archive/legacy-Docs/` (49 files)
- **Critical review**: Archived to `docs/archive/critical-review-2025/` (6 files)
- **Root markdown files**: Reduced from 16 to 11 (31% reduction)
- **Documentation directories**: Unified - only `docs/` remains (no more Docs/ confusion)
- **Created**: `GETTING_STARTED.md` - Modern MVP-focused entry point
- **Status**: Single documentation location, MVP-focused navigation, historical context preserved

### After Fourth Cleanup (January 21, 2026)
- **Assets/ directory**: Archived 44 non-MVP system guides → `docs/archive/pre-mvp-systems/assets-guides/`
- **Root directory**: Archived `MASTER_DOCUMENTATION.md` → `docs/archive/pre-mvp-systems/`
- **Root markdown files**: Reduced from 11 to 10 (9% further reduction)
- **Assets/ focus**: Now contains only MVP-relevant guides (trading, docking, stations, flight)
- **Status**: All documentation aligned with Trade Simulator MVP focus

## Using This Document

### For Contributors
- All obsolete documents have been removed
- Use the organized `docs/` structure for current documentation
- See [docs/README.md](docs/README.md) for navigation

### For Maintainers
- This document tracks what was removed and why
- Review periodically to ensure documentation stays clean
- Update when marking new files obsolete
- Consider annual documentation audits

## Documentation Lifecycle

```
Active Document
    ↓
Superseded by better documentation
    ↓
Marked for removal (this audit process)
    ↓
Removed or archived
    ↓
Tracked in this file
```

## Why These Cleanups Were Needed

### December 24, 2025 Cleanup
The project had accumulated:
- Multiple overlapping guides for the same topics
- Temporary status documents that were never removed
- Build fix summaries that belonged in Git history
- Code review documents for completed PRs
- Outdated implementation guides

**Achieved**:
- ✅ 93% reduction in root directory files
- ✅ Clear documentation hierarchy
- ✅ Easy navigation for new developers
- ✅ Eliminated duplicate information
- ✅ Preserved all important content

### January 14, 2026 Cleanup
The repository still had:
- Duplicate documentation directories (`Docs/` vs `docs/`)
- Historical critical review documents in root (completed Dec 2025)
- No modern MVP-focused entry point
- Confusion about which documentation location to use

**Achieved**:
- ✅ Single documentation directory (`docs/` only)
- ✅ Critical review documents preserved with context
- ✅ Modern `GETTING_STARTED.md` with MVP focus
- ✅ Root directory reduced to 11 essential files
- ✅ Historical turning point (Dec 2025 review) properly archived

### January 21, 2026 Cleanup
The repository still had:
- Non-MVP system documentation in Assets/ (44 guides for deferred systems)
- Pre-pivot master documentation in root (MASTER_DOCUMENTATION.md)
- No clear separation between MVP and post-MVP documentation
- Potential confusion about which systems are in scope

**Achieved**:
- ✅ All non-MVP system docs archived to `docs/archive/pre-mvp-systems/`
- ✅ Clear focus on Trade Simulator MVP only
- ✅ Pre-pivot documentation preserved for future reference
- ✅ Root directory reduced to 10 essential files
- ✅ Assets/ directory focused on MVP systems only (trading, docking, stations, flight)

## Future Maintenance

To keep documentation clean:
1. Remove temporary documents (reviews, summaries) after their purpose is served
2. Consolidate duplicate guides when discovered
3. Use `Archive/` for historical scripts, reports, and code
4. Use `docs/archive/` for historical documents with lasting value
5. Keep root directory limited to essential project files (target: 10-12 files)
6. Always add build artifacts to .gitignore
7. Use lowercase `docs/` as the only documentation directory
8. Update `GETTING_STARTED.md` to reflect current project state
9. Conduct annual documentation audits

---

## Current Active Documentation

For current, maintained documentation, see:

- **[GETTING_STARTED.md](GETTING_STARTED.md)** - ⭐ Modern MVP-focused entry point
- **[docs/README.md](docs/README.md)** - ⭐ Documentation navigation guide
- **[docs/mvp/README.md](docs/mvp/README.md)** - ⭐ Trade Simulator MVP documentation
- **[README.md](README.md)** - Project overview
- **[DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)** - Complete documentation index
- **[wiki/Home.md](wiki/Home.md)** - Wiki homepage

### Trade Simulator MVP (Current Focus)
Active development documentation:
- **[.github/instructions/trade-simulator-mvp.instructions.md](.github/instructions/trade-simulator-mvp.instructions.md)** - Complete MVP plan
- **[.github/instructions/anti-patterns.instructions.md](.github/instructions/anti-patterns.instructions.md)** - Lessons learned
- **[docs/mvp/](docs/mvp/)** - MVP implementation guides

### Historical Archives
Archived but important for understanding project evolution:
- **[docs/archive/critical-review-2025/README.md](docs/archive/critical-review-2025/README.md)** - Critical review (Dec 2025)
- **[docs/archive/pre-mvp-systems/README.md](docs/archive/pre-mvp-systems/README.md)** - ⭐ NEW: Pre-MVP systems (Jan 2026)
- **[docs/archive/legacy-Docs/README.md](docs/archive/legacy-Docs/README.md)** - Legacy documentation (Jan 2026)

---

**Status**: ✅ Cleanups complete - Repository is organized and MVP-focused

*This document was updated as part of the January 21, 2026 documentation review and cleanup.*
