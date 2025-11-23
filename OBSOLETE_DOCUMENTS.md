# Obsolete Documents - Marked with ZZ_ Prefix

**Last Updated**: November 23, 2025  
**Purpose**: Track documents that are no longer actively used but retained for reference

## Overview

Documents marked with `ZZ_` prefix are obsolete or superseded by other documentation. They are kept in the repository for historical reference but should not be used for new work.

## Why Mark as Obsolete?

Documents are marked obsolete when they:
- Contain temporary status information
- Have been superseded by better documentation
- Are historical logs or reports
- Contain outdated or redundant information
- Are subsets of more comprehensive guides

## Obsolete Documents List

### Build Status and Error Reports

**ZZ_BUILD_STATUS.md**
- **Marked**: November 23, 2025
- **Reason**: One-time status report from November 2025
- **Superseded By**: Current build status via GitHub Actions
- **Type**: Historical status document

**ZZ_BUILD_ERRORS_FIXED.md**
- **Marked**: November 23, 2025
- **Reason**: Historical error log
- **Superseded By**: Git commit history, resolved issues
- **Type**: Historical log

**ZZ_BUILD_ANALYSIS_REPORT.md**
- **Marked**: November 23, 2025
- **Reason**: One-time analysis report
- **Superseded By**: Current code quality is validated via CI/CD
- **Type**: Historical analysis

### Project Status Documents

**ZZ_WHATS_STILL_NEEDED.md**
- **Marked**: November 23, 2025
- **Reason**: Temporary status/TODO document
- **Superseded By**: GitHub Issues, Project Roadmap (ROADMAP.md)
- **Type**: Status document

### Blueprint Display Documentation

The following five documents were part of an earlier Blueprint documentation system that has been replaced by the integrated Blueprint guides:

**ZZ_BLUEPRINT_DISPLAY_CHEATSHEET.md**
- **Marked**: November 23, 2025
- **Reason**: Superseded by BLUEPRINT_QUICK_REFERENCE.md
- **Superseded By**: BLUEPRINT_QUICK_REFERENCE.md, BLUEPRINT_GUIDE_FOR_BEGINNERS.md
- **Type**: Redundant documentation

**ZZ_BLUEPRINT_DISPLAY_COMPARISON.md**
- **Marked**: November 23, 2025
- **Reason**: Superseded by integrated Blueprint guides
- **Superseded By**: BLUEPRINT_GUIDE_FOR_BEGINNERS.md
- **Type**: Redundant documentation

**ZZ_BLUEPRINT_DISPLAY_EXAMPLES.md**
- **Marked**: November 23, 2025
- **Reason**: Examples integrated into main Blueprint guides
- **Superseded By**: BLUEPRINT_GUIDE_FOR_BEGINNERS.md, Assets/BlueprintWorkflowTemplates.md
- **Type**: Redundant documentation

**ZZ_BLUEPRINT_DISPLAY_GUIDE.md**
- **Marked**: November 23, 2025
- **Reason**: Superseded by comprehensive Blueprint guide
- **Superseded By**: BLUEPRINT_GUIDE_FOR_BEGINNERS.md (1146 lines)
- **Type**: Redundant documentation

**ZZ_BLUEPRINT_DISPLAY_INDEX.md**
- **Marked**: November 23, 2025
- **Reason**: Superseded by integrated Blueprint documentation index
- **Superseded By**: BLUEPRINT_REQUIREMENTS_LIST.md, wiki documentation
- **Type**: Redundant documentation

**ZZ_BLUEPRINT_DOCS_README.md**
- **Marked**: November 23, 2025
- **Reason**: README for obsolete Blueprint display system
- **Superseded By**: BLUEPRINT_GUIDE_FOR_BEGINNERS.md, wiki/Blueprint-Development/
- **Type**: Redundant documentation

### Focused Setup Guides

**ZZ_CPP_FOCUSED_SETUP.md**
- **Marked**: November 23, 2025
- **Reason**: Subset of information available in other guides
- **Superseded By**: README.md, CONTRIBUTING.md, ARCHITECTURE.md
- **Type**: Redundant subset

**ZZ_ESSENTIAL_BLUEPRINTS_ONLY.md**
- **Marked**: November 23, 2025
- **Reason**: Subset of BLUEPRINT_REQUIREMENTS_LIST.md
- **Superseded By**: BLUEPRINT_REQUIREMENTS_LIST.md (882 lines, comprehensive)
- **Type**: Redundant subset

## Summary by Category

### Status Reports (4 documents)
- Build status
- Error logs  
- Analysis reports
- TODO lists

### Blueprint Documentation (6 documents)
- Display system guides
- Documentation index
- README for old system

### Setup Guides (2 documents)
- C++ focused setup
- Essential Blueprints only

## Total Obsolete Documents

**Count**: 12 files marked with `ZZ_` prefix

## Using This Document

### For Contributors
- **Don't use ZZ_ files** for reference
- Use the "Superseded By" links to find current documentation
- If you need historical context, ZZ_ files are available

### For Maintainers
- Review ZZ_ files periodically (every 6-12 months)
- Consider deletion after sufficient time (1+ year)
- Update this list when marking new files obsolete
- Document the superseding documentation

## Obsolete Document Lifecycle

```
Active Document
    ↓
Marked Obsolete (ZZ_ prefix)
    ↓
Historical Reference (6-12 months)
    ↓
Consider for Deletion (1+ years)
    ↓
Archive or Delete
```

## Why Keep Them?

We keep obsolete documents temporarily for:
1. **Historical Reference** - Understanding past decisions
2. **Link Compatibility** - Old links may still reference them
3. **Knowledge Preservation** - Context for why things changed
4. **Gradual Transition** - Allow time for users to adapt

## When to Delete

Documents can be deleted after:
- 12+ months as obsolete
- No external links reference them
- Information is fully preserved elsewhere
- No historical value remains

## Alternative: Archive Directory

Future consideration: Move ZZ_ files to `Docs/archive/` instead of using prefix.

**Benefits**:
- Cleaner root directory
- Clear separation
- Easier to ignore in searches

**Implementation**:
```bash
mkdir -p Docs/archive/
mv ZZ_*.md Docs/archive/
```

## Current Active Documentation

For current, maintained documentation, see:

- **[README.md](README.md)** - Project overview
- **[DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)** - Complete documentation index
- **[wiki/Home.md](wiki/Home.md)** - Wiki homepage
- **[wiki/Reference/Documentation-Index.md](wiki/Reference/Documentation-Index.md)** - Wiki documentation index

## Questions?

- **Why was a document marked obsolete?** Check this list for reasoning
- **Where do I find current docs?** See "Superseded By" links
- **Can I still use ZZ_ files?** Not recommended - use current docs
- **Should we delete ZZ_ files?** Not yet - allow transition period

---

**All obsolete documents are clearly marked with ZZ_ prefix for easy identification.**

*This document will be updated as new documents become obsolete.*
