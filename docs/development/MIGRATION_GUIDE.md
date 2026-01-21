# Migration Guide - Repository Modernization (January 2026)

**Date**: January 14, 2026  
**Version**: 1.0  
**Impact**: Documentation paths, navigation, and structure

---

## Overview

This guide helps developers, contributors, and tools update references after the January 2026 repository modernization. The primary changes are:

1. **Documentation directory consolidation** - `Docs/` → `docs/archive/legacy-Docs/`
2. **Critical review archival** - Root critical review docs → `docs/archive/critical-review-2025/`
3. **New entry point** - `GETTING_STARTED.md` created with MVP focus

---

## What Changed

### 1. Docs/ Directory Removed

**Before:**
```
/Adastrea/
├── Docs/                    # Capital D, 49 files
│   ├── README.md
│   ├── PLAYER_CONTROLS_*.md
│   ├── HUD_*.md
│   └── ...
├── docs/                    # Lowercase d, 234 files
│   ├── README.md
│   ├── development/
│   └── ...
```

**After:**
```
/Adastrea/
├── docs/                    # Only one docs directory
│   ├── README.md
│   ├── development/
│   ├── archive/
│   │   └── legacy-Docs/     # Former Docs/ content
│   │       ├── README.md    # Explains archive
│   │       ├── PLAYER_CONTROLS_*.md
│   │       └── ...
```

**Action Required:**
- Update any references from `Docs/` to `docs/archive/legacy-Docs/`
- Most content has been superseded - check `docs/README.md` for current alternatives
- Use lowercase `docs/` for all new documentation

### 2. Critical Review Documents Archived

**Before:**
```
/Adastrea/
├── CRITICAL_REVIEW_SUMMARY.md
├── CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md
├── CRITICAL_REVIEW_TECHNICAL_ISSUES.md
├── CRITICAL_REVIEW_ACTION_PLAN.md
├── CRITICAL_REVIEW_README.md
├── CURRENT_STATUS.md
```

**After:**
```
/Adastrea/
├── docs/
│   └── archive/
│       └── critical-review-2025/
│           ├── README.md    # Context and outcomes
│           ├── CRITICAL_REVIEW_SUMMARY.md
│           ├── CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md
│           ├── CRITICAL_REVIEW_TECHNICAL_ISSUES.md
│           ├── CRITICAL_REVIEW_ACTION_PLAN.md
│           ├── CRITICAL_REVIEW_README.md
│           └── CURRENT_STATUS.md
```

**Action Required:**
- Update links to critical review docs: `docs/archive/critical-review-2025/CRITICAL_REVIEW_SUMMARY.md`
- For current status, see root `CHANGELOG.md` or `GETTING_STARTED.md`
- Critical review is historical (Dec 2025) - see archive README for context

### 3. New Entry Point Created

**Added:**
- `GETTING_STARTED.md` - Modern MVP-focused entry point in root

**Supersedes:**
- Multiple informal getting-started references
- Outdated QUICKSTART.md content (still exists but supplemented)

**Action Required:**
- Direct new users to `GETTING_STARTED.md` first
- Update README.md links if pointing to old entry points

---

## Path Migration Table

Use this table to update your references:

| Old Path | New Path | Notes |
|----------|----------|-------|
| `Docs/README.md` | `docs/archive/legacy-Docs/README.md` | Archived, see docs/README.md for current |
| `Docs/PLAYER_CONTROLS_*.md` | `docs/archive/legacy-Docs/PLAYER_CONTROLS_*.md` | Historical reference only |
| `Docs/HUD_*.md` | `docs/archive/legacy-Docs/HUD_*.md` | Historical reference only |
| `Docs/MapSystem*.md` | `docs/archive/legacy-Docs/MapSystem*.md` | Historical reference only |
| `CRITICAL_REVIEW_SUMMARY.md` | `docs/archive/critical-review-2025/CRITICAL_REVIEW_SUMMARY.md` | Archived with context |
| `CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md` | `docs/archive/critical-review-2025/CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md` | Archived |
| `CRITICAL_REVIEW_TECHNICAL_ISSUES.md` | `docs/archive/critical-review-2025/CRITICAL_REVIEW_TECHNICAL_ISSUES.md` | Archived |
| `CRITICAL_REVIEW_ACTION_PLAN.md` | `docs/archive/critical-review-2025/CRITICAL_REVIEW_ACTION_PLAN.md` | Archived |
| `CURRENT_STATUS.md` | `docs/archive/critical-review-2025/CURRENT_STATUS.md` | Dec 2025 snapshot |
| N/A | `GETTING_STARTED.md` | **NEW** - Start here |

---

## Find and Replace Patterns

### For Markdown Files

Replace `Docs/` references:
```bash
# Linux/Mac
find . -name "*.md" -type f -exec sed -i 's|Docs/|docs/archive/legacy-Docs/|g' {} +

# Windows (PowerShell)
Get-ChildItem -Recurse -Filter *.md | ForEach-Object {
    (Get-Content $_.FullName) -replace 'Docs/', 'docs/archive/legacy-Docs/' | Set-Content $_.FullName
}
```

Replace critical review references:
```bash
# Linux/Mac
find . -name "*.md" -type f -exec sed -i 's|CRITICAL_REVIEW_\([A-Z_]*\)\.md|docs/archive/critical-review-2025/CRITICAL_REVIEW_\1.md|g' {} +

# Windows (PowerShell)
Get-ChildItem -Recurse -Filter *.md | ForEach-Object {
    (Get-Content $_.FullName) -replace 'CRITICAL_REVIEW_(.*)\.md', 'docs/archive/critical-review-2025/CRITICAL_REVIEW_$1.md' | Set-Content $_.FullName
}
```

### For Source Code

If your code references documentation paths (unlikely but possible):
```cpp
// Before
const FString DocsPath = TEXT("Docs/README.md");

// After
const FString DocsPath = TEXT("docs/archive/legacy-Docs/README.md");
// Or better, use current docs:
const FString DocsPath = TEXT("docs/README.md");
```

---

## Updating Your Local Clone

### If You Have Uncommitted Changes

1. **Stash your changes:**
   ```bash
   git stash
   ```

2. **Pull latest changes:**
   ```bash
   git checkout main
   git pull origin main
   ```

3. **Review path changes:**
   ```bash
   git log --oneline --graph -10
   # Look for: "chore: Phase 1 modernization..."
   ```

4. **Reapply your changes:**
   ```bash
   git stash pop
   ```

5. **Update any Docs/ references** in your changes using patterns above

### If You Have a Clean Working Directory

```bash
git checkout main
git pull origin main
```

That's it! Git handles the renames/moves automatically.

---

## Common Issues and Solutions

### Issue 1: "File not found: Docs/README.md"

**Solution:**
```bash
# File moved to:
docs/archive/legacy-Docs/README.md

# But you probably want the current docs index:
docs/README.md
```

### Issue 2: "Cannot find CRITICAL_REVIEW_SUMMARY.md"

**Solution:**
```bash
# File moved to:
docs/archive/critical-review-2025/CRITICAL_REVIEW_SUMMARY.md

# For current status, see:
GETTING_STARTED.md  # Project overview
CHANGELOG.md        # Recent changes
```

### Issue 3: "Which documentation directory do I use?"

**Solution:**
- **Always use lowercase `docs/`** (not `Docs/`)
- `Docs/` no longer exists (archived)
- Only one documentation location to eliminate confusion

### Issue 4: "Where is the project status?"

**Solution:**
- **Current status**: See `GETTING_STARTED.md` (MVP focus, Week 3-4 of 12)
- **Historical status**: See `docs/archive/critical-review-2025/CURRENT_STATUS.md` (Dec 2025 snapshot)
- **Changes**: See `CHANGELOG.md` (all updates)

### Issue 5: "I need the critical review documents"

**Solution:**
All critical review docs are archived with context:
```bash
docs/archive/critical-review-2025/
├── README.md                          # Start here - full context
├── CRITICAL_REVIEW_SUMMARY.md         # Executive summary
├── CRITICAL_REVIEW_PUBLISHER_ASSESSMENT.md
├── CRITICAL_REVIEW_TECHNICAL_ISSUES.md
├── CRITICAL_REVIEW_ACTION_PLAN.md
└── CURRENT_STATUS.md                  # Dec 2025 snapshot
```

---

## Documentation Structure (Post-Migration)

### Root Directory (11 files)
Essential project documentation only:
```
├── README.md                    # Project overview
├── GETTING_STARTED.md           # ⭐ NEW - Start here
├── ARCHITECTURE.md              # System architecture
├── CODE_STYLE.md                # Coding standards
├── CONTRIBUTING.md              # Contribution guide
├── CHANGELOG.md                 # Version history
├── ROADMAP.md                   # Development roadmap
├── QUICKSTART.md                # Quick setup
├── MASTER_DOCUMENTATION.md      # All docs in one file
├── DOCUMENTATION_INDEX.md       # Complete doc index
└── OBSOLETE_DOCUMENTS.md        # Archive tracking
```

### docs/ Directory (Organized)
```
docs/
├── README.md                    # Documentation navigation
├── mvp/                         # Trade Simulator MVP docs
├── development/                 # Developer guides
├── reference/                   # Quick references
├── setup/                       # Installation guides
├── testing/                     # Testing docs
├── content-generation/          # Content tools
├── systems/                     # Game systems
└── archive/                     # Historical docs
    ├── critical-review-2025/    # ⭐ Dec 2025 review
    └── legacy-Docs/             # ⭐ Former Docs/ content
```

---

## For CI/CD and Automation

### GitHub Actions

If your workflows reference documentation paths:

```yaml
# Before
- name: Check docs
  run: test -f Docs/README.md

# After  
- name: Check docs
  run: test -f docs/README.md
```

### Scripts and Tools

Update any hardcoded paths:

```python
# Before
docs_path = "Docs/README.md"

# After
docs_path = "docs/README.md"
# Or for archived content:
docs_path = "docs/archive/legacy-Docs/README.md"
```

---

## Verification Checklist

After updating your references:

- [ ] All `Docs/` references updated to `docs/archive/legacy-Docs/` (if still needed)
- [ ] Critical review links updated to `docs/archive/critical-review-2025/`
- [ ] New documentation uses lowercase `docs/`
- [ ] CI/CD workflows tested with new paths
- [ ] Local builds work correctly
- [ ] Documentation links don't return 404

---

## Need Help?

### Documentation Questions
- **Current docs**: See `docs/README.md`
- **Entry point**: See `GETTING_STARTED.md`
- **Archive context**: See `docs/archive/*/README.md` files

### Technical Questions
- **Open an issue**: Tag with "documentation" label
- **Check CHANGELOG**: See `CHANGELOG.md` for recent changes
- **Architecture**: See `ARCHITECTURE.md`

---

## Timeline

- **December 24, 2025**: First documentation cleanup (80+ obsolete files removed)
- **January 13, 2026**: Build artifacts and scripts archived
- **January 14, 2026**: Documentation consolidation (Docs/ → docs/archive/), critical review archived
- **Future**: Ongoing maintenance following established patterns

---

## Related Documentation

- **CHANGELOG.md** - Detailed changes (2026-01-14 entry)
- **OBSOLETE_DOCUMENTS.md** - Archive tracking and history
- **docs/archive/critical-review-2025/README.md** - Critical review context
- **docs/archive/legacy-Docs/README.md** - Legacy Docs/ explanation
- **.github/instructions/anti-patterns.instructions.md** - Why these changes matter

---

**Questions?** Open a GitHub issue or discussion.

**Status**: ✅ Migration complete - Repository modernized for MVP focus

*Created: January 14, 2026 - Repository Modernization*
