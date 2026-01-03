# File Structure Issue Resolution Guide

**Related**: FILE_STRUCTURE_STANDARD.md, FILE_STRUCTURE_MIGRATION_SUMMARY.md  
**Last Updated**: 2026-01-02

---

## Overview

This guide helps resolve common issues related to the Adastrea file structure standard. Use this when encountering problems with file locations, naming, or organization.

---

## 🔍 Common Issues and Solutions

### Issue #1: "I can't find the documentation file I need"

**Symptoms**:
- Documentation referenced but not found
- Broken links in markdown files
- Old file paths in documentation

**Solution**:

1. **Check the standard structure** in `FILE_STRUCTURE_STANDARD.md`
2. **Search the docs/ directory**:
   ```bash
   find docs -name "*[keyword]*" -type f
   ```
3. **Check if file was archived**:
   ```bash
   ls docs/archive/
   ```
4. **Check DOCUMENTATION_INDEX.md** for file locations

**Prevention**:
- Always use relative paths from repository root
- Update all cross-references when moving files
- Check links work before committing

---

### Issue #2: "Widget name doesn't match documentation"

**Symptoms**:
- Documentation refers to `WBP_TradingUI`
- Asset is named `WBP_TradingInterface`
- Confusion about which name is correct

**Solution**:

**Correct Name**: `WBP_TradingUI`

**Steps to Fix**:

1. **In Unreal Editor**:
   - Open Content Browser
   - Navigate to `Content/UI/Trading/`
   - Right-click `WBP_TradingInterface` → Rename
   - Enter new name: `WBP_TradingUI`
   - Right-click asset → Fix Up Redirectors
   - Save all

2. **Verify References**:
   ```bash
   # Search for old name in C++ code
   grep -r "TradingInterface" Source/
   
   # Search for old name in Blueprint files (if text-based)
   grep -r "TradingInterface" Content/
   ```

3. **Test**:
   - Open trading UI in game
   - Verify no errors in Output Log
   - Test buy/sell functionality

**Why WBP_TradingUI is Correct**:
- Follows `WBP_[Category]_[Name]` convention
- "UI" is standard widget terminology
- Matches documentation standard
- Consistent with other widgets (WBP_HUD_Main, etc.)

---

### Issue #3: "Where should I put my new documentation file?"

**Decision Tree**:

```
Is it Blueprint documentation?
├─ Yes → docs/BP/[BlueprintName].md
└─ No ↓

Is it a quick reference/cheat sheet?
├─ Yes → docs/reference/[TOPIC]_QUICK_REFERENCE.md
└─ No ↓

Is it MVP/Trade Simulator related?
├─ Yes → docs/mvp/[TOPIC].md
└─ No ↓

Is it a setup/installation guide?
├─ Yes → docs/setup/[TOPIC]_SETUP.md
└─ No ↓

Is it system-specific documentation?
├─ Yes → docs/systems/[SYSTEM]_GUIDE.md
└─ No ↓

Is it development workflow/process?
├─ Yes → docs/development/[TOPIC].md
└─ No ↓

Is it testing documentation?
├─ Yes → docs/testing/[TOPIC].md
└─ No ↓

Is it outdated but potentially useful?
└─ Yes → docs/archive/[TOPIC].md
```

**Examples**:

- **New widget guide**: `docs/BP/WBP_NewWidget.md`
- **Trading system quick ref**: `docs/reference/TRADING_QUICK_REFERENCE.md`
- **MVP implementation steps**: `docs/mvp/IMPLEMENTATION_STEPS.md`
- **Docker setup guide**: `docs/setup/DOCKER_SETUP.md`
- **AI system guide**: `docs/systems/AI_SYSTEM_GUIDE.md`
- **Build workflow**: `docs/development/BUILD_WORKFLOW.md`
- **Test plan**: `docs/testing/TEST_PLAN.md`

---

### Issue #4: "My C++ includes are breaking"

**Symptoms**:
- Cannot find header file
- Compile error: "No such file or directory"
- Include path incorrect

**Solution**:

1. **Verify Header Location**:
   - Headers should be in `Source/Adastrea/Public/[System]/`
   - Implementation in `Source/Adastrea/Private/[System]/`

2. **Use Correct Include Path**:
   ```cpp
   // Correct (relative to Public/)
   #include "Trading/EconomyManager.h"
   #include "Ships/SpaceshipDataAsset.h"
   
   // Incorrect (absolute path)
   #include "Source/Adastrea/Public/Trading/EconomyManager.h"
   ```

3. **Check Module Dependencies**:
   - Verify module listed in `.Build.cs` if cross-module include
   - Ensure no circular dependencies

4. **Regenerate Project Files**:
   ```bash
   # Windows
   GenerateProjectFiles.bat
   
   # Linux/Mac
   ./GenerateProjectFiles.sh
   ```

**Prevention**:
- Always use system subdirectories (AI/, Trading/, Ships/, etc.)
- Keep headers in Public/, implementations in Private/
- Use forward declarations when possible

---

### Issue #5: "Root directory is cluttered with too many files"

**Symptoms**:
- 100+ files in root directory
- Hard to find important files
- Multiple README/summary files

**Solution**:

1. **Move Documentation**:
   ```bash
   # Move summary/guide files to docs/
   mv MY_GUIDE.md docs/development/
   mv SUMMARY.md docs/reference/
   ```

2. **Use CHANGELOG.md**:
   - Don't create new summary files for every change
   - Update CHANGELOG.md instead
   - Only create dedicated docs when substantial

3. **Archive Old Files**:
   ```bash
   # Move deprecated docs to archive
   mv OLD_GUIDE.md docs/archive/
   ```

4. **Organize Scripts**:
   ```bash
   # Move utility scripts to Tools/
   mv my_script.py Tools/
   ```

**Target**:
- Root directory: &lt;100 files
- Only essential config files in root (README.md, CHANGELOG.md, .gitignore, etc.)
- All documentation in docs/ subdirectories

**See**: `.github/instructions/anti-patterns.instructions.md` - Anti-pattern #13

---

### Issue #6: "I have duplicate files in different locations"

**Symptoms**:
- Same file exists in multiple places
- Unclear which version is authoritative
- Inconsistent content between copies

**Solution**:

1. **Identify Authoritative Version**:
   - Check git history for most recent updates
   - Check file modification dates
   - Review content quality

2. **Remove Duplicates**:
   ```bash
   # Keep the correct location, remove others
   git rm path/to/duplicate/file.md
   ```

3. **Add Deprecation Notice** (if needed):
   ```markdown
   # DEPRECATED
   
   This file has been moved to: [correct/path/to/file.md](correct/path/to/file.md)
   
   Please update your bookmarks.
   ```

4. **Update Cross-References**:
   - Search for links to old location
   - Update to point to new location
   ```bash
   grep -r "old/path/to/file" docs/
   ```

**Prevention**:
- Follow FILE_STRUCTURE_STANDARD.md for placement
- Check for existing files before creating new ones
- Use symbolic links if file truly needs to be in multiple places

---

### Issue #7: "My Blueprint references are broken after rename"

**Symptoms**:
- "Failed to load" errors in Unreal Editor
- Red/missing references in Blueprints
- Widgets don't show up in UI

**Solution**:

1. **Fix Up Redirectors**:
   - Content Browser → Right-click asset
   - "Fix Up Redirectors on Folder"
   - Save all modified files

2. **Manual Reference Update**:
   - Open each Blueprint with broken reference
   - Replace with new asset reference
   - Compile and save

3. **Check Generated Files**:
   - Delete `Saved/` and `Intermediate/` folders
   - Regenerate project files
   - Reopen editor

4. **Check C++ References**:
   ```cpp
   // Update LoadObject calls
   // Old:
   UClass* WidgetClass = LoadObject<UClass>(nullptr, TEXT("/Game/UI/WBP_TradingInterface.WBP_TradingInterface_C"));
   
   // New:
   UClass* WidgetClass = LoadObject<UClass>(nullptr, TEXT("/Game/UI/Trading/WBP_TradingUI.WBP_TradingUI_C"));
   ```

**Prevention**:
- Always use "Fix Up Redirectors" after rename
- Test in editor after any asset rename
- Use Blueprint interfaces instead of direct references when possible

---

### Issue #8: "Tool script can't find files after reorganization"

**Symptoms**:
- Python script fails with "File not found"
- Paths in script are outdated
- Script references old file locations

**Solution**:

1. **Update Script Paths**:
   ```python
   # Old
   DOC_PATH = "Docs/guide.md"
   
   # New
   DOC_PATH = "docs/reference/guide.md"
   ```

2. **Use Path.exists() Checks**:
   ```python
   from pathlib import Path
   
   doc_path = Path("docs/reference/guide.md")
   if not doc_path.exists():
       print(f"Error: File not found at {doc_path}")
       print("Have files been reorganized? Check FILE_STRUCTURE_STANDARD.md")
       sys.exit(1)
   ```

3. **Make Paths Configurable**:
   ```python
   # config.json
   {
     "docs_dir": "docs/reference",
     "tools_dir": "Tools"
   }
   ```

**Prevention**:
- Use configuration files for paths
- Add path validation in scripts
- Document expected file structure in script header

---

### Issue #9: "I'm unsure if my naming convention is correct"

**Quick Reference**:

| File Type | Convention | Example |
|-----------|-----------|---------|
| Widget Blueprint | `WBP_[Category]_[Name]` | `WBP_TradingUI` |
| Actor Blueprint | `BP_[Category]_[Name]` | `BP_Ship_NPC_Trader` |
| Data Asset | `DA_[Type]_[Name]` | `DA_Ship_Pathfinder` |
| UObject Class | `U[ClassName]` | `USpaceshipDataAsset` |
| Actor Class | `A[ClassName]` | `ASpaceStation` |
| Struct | `F[StructName]` | `FTradeTransaction` |
| Enum | `E[EnumName]` | `EWeaponType` |
| Interface | `I[InterfaceName]` | `IIDamageable` |
| Documentation | `[TOPIC]_[TYPE].md` | `TRADING_QUICK_REFERENCE.md` |

**Verification**:
```bash
# Check if your file follows naming convention
# Run this from repository root
python3 Tools/validate_naming.py
```

---

## 🛠️ Validation Tools

### Check File Structure Compliance

```bash
# Check for files in deprecated locations
find . -maxdepth 1 -name "*.md" -type f | wc -l  # Should be <20

# Check for duplicate documentation
find docs -name "README.md" -type f | wc -l  # Should be <5

# Check for widget naming
find Content/UI -name "WBP_*Interface*.uasset"  # Should be empty
```

### Automated Validation (Future)

Planned tools:
- `check_file_structure.py` - Validate against standard
- `find_duplicates.py` - Find duplicate files
- `fix_cross_references.py` - Update documentation links

---

## 📋 Pre-Commit Checklist

Before committing file structure changes:

- [ ] Files in correct `docs/` subdirectory
- [ ] Naming convention followed
- [ ] No duplicate files created
- [ ] Cross-references updated
- [ ] Deprecated locations removed or noted
- [ ] CHANGELOG.md updated (don't create new summary file)
- [ ] Tools/scripts updated if affected
- [ ] Tested in editor if Blueprints changed

---

## 🚨 Emergency Recovery

### "I moved a file and broke everything"

1. **Don't Panic** - Git has your back
2. **Check Git Status**:
   ```bash
   git status
   git diff
   ```
3. **Revert Changes**:
   ```bash
   git checkout -- path/to/file
   ```
4. **Or Reset**:
   ```bash
   git reset --hard HEAD
   ```
5. **Redo Carefully** following FILE_STRUCTURE_STANDARD.md

### "Editor won't load after asset rename"

1. **Close Editor**
2. **Delete Generated Files**:
   ```bash
   rm -rf Saved/ Intermediate/
   ```
3. **Regenerate Project**:
   ```bash
   # Windows
   GenerateProjectFiles.bat
   
   # Linux/Mac
   ./GenerateProjectFiles.sh
   ```
4. **Reopen Editor**
5. **Fix Up Redirectors**

---

## 📚 Additional Resources

- `FILE_STRUCTURE_STANDARD.md` - Official standard
- `FILE_STRUCTURE_MIGRATION_SUMMARY.md` - Migration tracking
- `.github/instructions/anti-patterns.instructions.md` - What to avoid
- `CODE_STYLE.md` - Coding conventions
- `CONTRIBUTING.md` - Contribution guidelines

---

## 📞 Getting Help

If this guide doesn't solve your issue:

1. Search existing GitHub issues for similar problems
2. Check `DOCUMENTATION_INDEX.md` for related documentation
3. Ask in team chat with `[File Structure]` tag
4. Open new GitHub issue with:
   - What you were trying to do
   - What went wrong
   - Steps you've tried
   - Relevant file paths

---

**Remember**: The file structure standard exists to make the project easier to navigate and maintain. When in doubt, follow the standard and ask for clarification.

---

**Last Updated**: 2026-01-02  
**Maintained by**: Adastrea Development Team
