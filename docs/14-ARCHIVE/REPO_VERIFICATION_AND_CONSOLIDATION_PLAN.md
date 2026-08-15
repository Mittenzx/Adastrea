# Adastrea - Repo Verification & MD Consolidation Plan

**Run this BEFORE connecting UE MCP** — cleans up the repo so we know exactly what we're working with.

---

## Phase 1: Repo Audit & Verification (Automated)

### 1.1 Code Health Checks
```bash
# Run existing test suite
python test_trading_system.py
python test_station_systems.py
python test_damage_calculations.py
python test_todo_verification.py

# Repository health
python repository_health_check.py
python repository_health_check_enhanced.py
```

### 1.2 Verify Critical Files Exist
```bash
# Check all DataAssets referenced in code actually exist
find Content/DataAssets/Trading/Items -name "*.uasset" | wc -l  # Should be 20+
find Content/Blueprints/Stations/Modules -name "*.uasset" | wc -l
find Content/Blueprints/Trading -name "*.uasset" | wc -l

# Verify C++ module structure
ls Source/Adastrea/Private/Trading/
ls Source/Adastrea/Private/Stations/
ls Source/Adastrea/Private/Ships/
```

### 1.3 Check Build Configuration
```bash
# Verify .uproject modules match Build.cs
cat Adastrea.uproject | grep -A 20 "Modules"
cat Source/Adastrea/Adastrea.Build.cs | grep -A 20 "PublicDependencyModuleNames"
```

### 1.4 Documentation Inventory
```bash
# List ALL .md files with sizes
find . -name "*.md" -type f -exec ls -lh {} \; | sort -k5 -h
```

---

## Phase 2: MD File Consolidation Strategy

### Current State (Jumbled)
```
Root (15+ files):
├── README.md (58 KB)           ← KEEP as main entry
├── ARCHITECTURE.md (11 KB)     ← KEEP as technical reference
├── ROADMAP.md (19 KB)          ← KEEP as timeline
├── CHANGELOG.md (?)            ← Check if exists
├── MVP_FORWARD_PLAN.md (16 KB) ← NEW - my plan
├── MVP_QUICK_REFERENCE.md (8 KB) ← NEW - my quick ref
├── MCP_EXECUTION_PLAN.md (10 KB) ← NEW - my execution plan
├── CRITICAL_REVIEW_SUMMARY.md
├── CRITICAL_REVIEW_ACTION_PLAN.md
├── TODO_TRACKING.md
├── TODO_TRACKING_WINDOWS.md
├── MASTER_DOCUMENTATION.md (74 KB)
├── CURRENT_STATUS.md
├── ... many more

docs/ (organized but duplicated):
├── mvp/README.md (9 KB)
├── mvp/TRADING_MVP_GUIDE.md
├── mvp/PHASE_1_CHECKLIST.md
├── mvp/GETTING_STARTED.md
├── mvp/SYSTEM_CLASSIFICATION.md
├── development/CONTRIBUTING.md
├── development/CODE_STYLE.md
├── development/FULL_ARCHITECTURE.md
├── reference/MVP_TELEMETRY_QUICK_REFERENCE.md
├── setup/BUILD_WITH_UE_TOOLS.md
├── ... 30+ more

.github/instructions/:
├── trade-simulator-mvp.instructions.md (30 KB)
├── anti-patterns.instructions.md
├── unreal-directive-best-practices.md
```

---

## Phase 3: Consolidation Plan (What I'll Do)

### 3.1 Create Single Source of Truth Structure

```
docs/
├── INDEX.md                    # Master navigation (NEW)
├── 01-GETTING_STARTED.md       # Setup, build, first run
├── 02-MVP_OVERVIEW.md          # What is the Trade Simulator MVP
├── 03-CURRENT_STATUS.md        # Real status: code vs content
├── 04-ARCHITECTURE.md          # Technical architecture (from ARCHITECTURE.md)
├── 05-ROADMAP.md               # Timeline (from ROADMAP.md)
├── 06-SYSTEM_REFERENCE.md      # All 22 systems summary
├── 07-MVP_EXECUTION_PLAN.md    # My 3-day MCP plan (consolidated)
├── 08-CONTRIBUTING.md          # How to contribute
├── 09-SETUP_GUIDES/
│   ├── BUILD_WITH_UE_TOOLS.md
│   ├── VISUAL_STUDIO_PROJECT_GENERATION.md
│   ├── PROJECT_GENERATION_QUICK_FIX.md
│   └── WINDOWS_BUILD_SCRIPTS.md
├── 10-MVP_PHASES/
│   ├── PHASE_1_HARDCODED_PROTOTYPE.md
│   ├── PHASE_2_STRUCTURED_VERSION.md
│   └── PHASE_3_POLISH_DEMO.md
├── 11-TECHNICAL_SPECS/
│   ├── TRADING_SYSTEM.md
│   ├── STATION_SYSTEM.md
│   ├── SHIP_SYSTEM.md
│   ├── ECONOMY_SYSTEM.md
│   └── DATA_ASSET_GUIDE.md
├── 12-ANTI_PATTERNS.md
├── 13-CRITICAL_REVIEW_SUMMARY.md
├── 14-ARCHIVE/                 # Everything else moved here
    ├── old_README_backup.md
    ├── MASTER_DOCUMENTATION.md
    ├── CURRENT_STATUS.md
    ├── endless-cycle-*.md
    ├── hourly_cycle_*.md
    ├── cycle_*_summary.md
    └── ...
```

### 3.2 Root Level: Keep Only 4 Files
```
Adastrea/
├── README.md              # Main entry - points to docs/INDEX.md
├── LICENSE
├── Adastrea.uproject
├── ROADMAP.md             # High-level timeline (keep)
└── docs/                  # ALL documentation here
```

### 3.3 What Gets Archived (Not Deleted)
- `MASTER_DOCUMENTATION.md` → `docs/14-ARCHIVE/`
- All `endless-cycle-*.md`, `hourly_cycle_*.md`, `cycle_*_summary.md` → `docs/14-ARCHIVE/`
- `CURRENT_STATUS.md` → `docs/14-ARCHIVE/` (superseded by `03-CURRENT_STATUS.md`)
- `TODO_TRACKING*.md` → `docs/14-ARCHIVE/`
- My 3 new plans → consolidated into `07-MVP_EXECUTION_PLAN.md`
- Duplicate MVP docs in root → consolidated into `docs/10-MVP_PHASES/`

---

## Phase 4: Execution (What I'll Run)

```bash
# 1. Run verification checks
python test_trading_system.py
python test_station_systems.py
python test_damage_calculations.py
python test_todo_verification.py
python repository_health_check_enhanced.py

# 2. Inventory all MD files
find . -name "*.md" -type f | head -50

# 3. Create new docs structure
mkdir -p docs/09-SETUP_GUIDES docs/10-MVP_PHASES docs/11-TECHNICAL_SPECS docs/14-ARCHIVE

# 4. Move/archive files (I'll do this carefully with git mv to preserve history)
# 5. Create new consolidated files
# 6. Update README.md to point to docs/INDEX.md
# 7. Verify no broken links
```

---

## Phase 5: Verification Report (What You'll Get)

After consolidation, I'll give you:

```
REPO VERIFICATION REPORT
========================
✅ Tests passing: X/Y
✅ DataAssets found: 20 trade items, 5 markets
✅ Blueprints found: X ships, Y stations, Z trading
✅ Build config: Consistent
✅ MD files: 45 → 14 organized files (31 archived)
✅ Root cleanup: 15 → 4 files
✅ No broken internal links

READY FOR UE MCP CONNECTION
```

---

## Shall I Run This Now?

**Say "run verification"** and I'll:
1. Execute all test scripts
2. Inventory every .md file
3. Create the new consolidated structure
4. Move/archive files with git history
5. Create the new master INDEX.md and consolidated docs
6. Update README.md

**Then** you install UE 5.6, enable Remote Control, and we run the MCP execution plan on a clean, verified repo.

**This takes ~10-15 minutes of automated work.** Ready?