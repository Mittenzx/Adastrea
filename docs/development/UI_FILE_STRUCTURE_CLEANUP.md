# UI File Structure Cleanup Guide

**Date**: 2026-01-01  
**Status**: Proposed Cleanup Plan  
**Issue**: Repository has duplicate UI files and inconsistent organization

---

## Problem Summary

The Adastrea repository has accumulated UI file duplication and organizational issues:

1. **Two parallel UI directories** with different naming conventions
2. **Duplicate widget files** with naming variations
3. **Inconsistent structure** violating CONTENT_ORGANIZATION.md standards

---

## Current State Analysis

### Content/Blueprints/UI/ (Legacy Structure)

Contains 5 files using old `BP_UI_*` naming convention:
- `BP_UI_HUD.uasset`
- `BP_UI_Inventory.uasset`
- `BP_UI_Quest_Log.uasset`
- `BP_UI_Ship_Customization.uasset`
- `BP_UI_Trading.uasset`

**Status**: ❌ Wrong location and naming convention

### Content/UI/ (Current Structure)

Contains 31 widget files with proper `WBP_*` naming, but has duplicates:

#### Trading (2 files - DUPLICATE)
- `Trading/WBP_TradingInterface.uasset`
- `Trading/WBP_Trading_Interface.uasset` ← Duplicate with underscore

#### Menus (6 files - 3 duplicates)
- `Menus/WBP_MainMenu.uasset` ✓
- `Menus/WBP_Main_Menu.uasset` ← Duplicate with underscore
- `Menus/WBP_PauseMenu.uasset` ✓
- `Menus/WBP_Pause_Menu.uasset` ← Duplicate with underscore
- `Menus/WBP_SettingsMenu.uasset` ✓
- `Menus/WBP_Settings_Menu.uasset` ← Duplicate with underscore
- `Menus/WBP_Save_Load_Menu.uasset` ✓

#### Quests (2 files - DUPLICATE)
- `Quests/WBP_QuestLog.uasset` ✓
- `Quests/WBP_Quest_Log.uasset` ← Duplicate with underscore

#### Inventory (2 files - DUPLICATE)
- `Inventory/WBP_Inventory.uasset` ✓
- `Inventory/WBP_InventoryWidget.uasset` ← Possible duplicate or variant

#### Ship (2 files in different locations - DUPLICATE)
- `Ship/WBP_Ship_Customization.uasset` ✓ (in UI/Ship/)
- `Ships/WBP_ShipCustomization.uasset` ← Duplicate in wrong folder
- `Ships/WBP_ShipStatus.uasset` ✓

#### Notifications (2 files - POSSIBLE DUPLICATE)
- `Notifications/WBP_Notification.uasset` ✓
- `Notifications/WBP_Notification_Popup.uasset` ← May be different type

---

## Official Standard (CONTENT_ORGANIZATION.md)

**Widget Blueprint Naming**: `WBP_[Location]_[Purpose]`  
**Location**: `Content/UI/` with subdirectories by system  
**Example**: `WBP_Menu_Settings`, `WBP_HUD_Main`, `WBP_Trading_Interface`

---

## Cleanup Actions

### Priority 1: Remove Obvious Duplicates

**Trading Interface:**
- **KEEP**: `Content/UI/Trading/WBP_TradingInterface.uasset` (no underscore)
- **DELETE**: `Content/UI/Trading/WBP_Trading_Interface.uasset` (with underscore)
- **Rationale**: Consistent with standard naming (no underscores between parts)

**Main Menu:**
- **KEEP**: `Content/UI/Menus/WBP_MainMenu.uasset`
- **DELETE**: `Content/UI/Menus/WBP_Main_Menu.uasset`

**Pause Menu:**
- **KEEP**: `Content/UI/Menus/WBP_PauseMenu.uasset`
- **DELETE**: `Content/UI/Menus/WBP_Pause_Menu.uasset`

**Settings Menu:**
- **KEEP**: `Content/UI/Menus/WBP_SettingsMenu.uasset`
- **DELETE**: `Content/UI/Menus/WBP_Settings_Menu.uasset`

**Quest Log:**
- **KEEP**: `Content/UI/Quests/WBP_QuestLog.uasset`
- **DELETE**: `Content/UI/Quests/WBP_Quest_Log.uasset`

**Ship Customization:**
- **KEEP**: `Content/UI/Ship/WBP_Ship_Customization.uasset` (proper folder)
- **DELETE**: `Content/UI/Ships/WBP_ShipCustomization.uasset` (wrong folder)

### Priority 2: Move BP_UI_* Files to Correct Location

These files are in wrong location with wrong naming:

**From `Content/Blueprints/UI/`** → **To `Content/UI/`**:

1. `BP_UI_HUD.uasset` → Consider deprecating if `WBP_HUD_Main.uasset` exists
2. `BP_UI_Inventory.uasset` → Consider deprecating if `WBP_Inventory.uasset` exists
3. `BP_UI_Quest_Log.uasset` → Consider deprecating if `WBP_QuestLog.uasset` exists
4. `BP_UI_Ship_Customization.uasset` → Consider deprecating if `WBP_Ship_Customization.uasset` exists
5. `BP_UI_Trading.uasset` → Consider deprecating if `WBP_TradingInterface.uasset` exists

**Action**: Mark as deprecated, verify no references, then delete

### Priority 3: Investigate Potential Duplicates

**Inventory:**
- `WBP_Inventory.uasset` vs `WBP_InventoryWidget.uasset`
- **Action**: Need to check in Unreal Editor if these are actually different
- **Hypothesis**: One might be parent class, other might be implementation

**Notifications:**
- `WBP_Notification.uasset` vs `WBP_Notification_Popup.uasset`
- **Action**: These might be legitimately different (inline vs popup)
- **Decision**: Keep both unless proven duplicate

**Ship Status:**
- `WBP_ShipStatus.uasset` - appears unique, keep

### Priority 4: Standardize Directory Structure

**Final Target Structure:**

```
Content/UI/
├── HUD/
│   ├── WBP_HUD_Main.uasset
│   ├── WBP_Mini_Map.uasset
│   ├── WBP_Damage_Indicator.uasset
│   ├── WBP_Objective_Tracker.uasset
│   └── WBP_Spaceship.uasset
│
├── Menus/
│   ├── WBP_MainMenu.uasset
│   ├── WBP_PauseMenu.uasset
│   ├── WBP_SettingsMenu.uasset
│   └── WBP_Save_Load_Menu.uasset
│
├── Trading/
│   └── WBP_TradingInterface.uasset
│
├── Inventory/
│   ├── WBP_Inventory.uasset
│   └── WBP_InventoryWidget.uasset (if different)
│
├── Ship/
│   ├── WBP_Ship_Customization.uasset
│   └── WBP_ShipStatus.uasset
│
├── Stations/
│   ├── WBP_StationEditor.uasset
│   ├── WBP_StationManagement.uasset
│   ├── WBP_ModuleListItem.uasset
│   └── WBP_ConstructionQueueItem.uasset
│
├── Map/
│   ├── WBP_SectorMap.uasset
│   └── WBP_UniverseMap.uasset
│
├── Communication/
│   └── WBP_Chat_Window.uasset
│
├── Dialogue/
│   └── WBP_DialogueBox.uasset
│
├── Notifications/
│   ├── WBP_Notification.uasset
│   └── WBP_Notification_Popup.uasset
│
└── Quests/
    └── WBP_QuestLog.uasset
```

---

## Implementation Steps

### Step 1: Backup
```bash
# Create backup before any deletions
cd /home/runner/work/Adastrea/Adastrea
git add -A
git commit -m "Backup before UI cleanup"
```

### Step 2: Delete Obvious Duplicates (Git)
```bash
# Remove underscore-variation duplicates
git rm Content/UI/Trading/WBP_Trading_Interface.uasset
git rm Content/UI/Menus/WBP_Main_Menu.uasset
git rm Content/UI/Menus/WBP_Pause_Menu.uasset
git rm Content/UI/Menus/WBP_Settings_Menu.uasset
git rm Content/UI/Quests/WBP_Quest_Log.uasset
git rm Content/UI/Ships/WBP_ShipCustomization.uasset

# Commit removal
git commit -m "Remove duplicate UI widgets with underscore naming variations"
```

### Step 3: Check for Legacy BP_UI_* References

Search codebase for any hardcoded references:
```bash
grep -r "BP_UI_Trading" Content/ Source/ --include="*.cpp" --include="*.h"
grep -r "BP_UI_HUD" Content/ Source/ --include="*.cpp" --include="*.h"
# etc...
```

### Step 4: Deprecate BP_UI_* Files

If no references found, delete:
```bash
git rm Content/Blueprints/UI/BP_UI_HUD.uasset
git rm Content/Blueprints/UI/BP_UI_Inventory.uasset
git rm Content/Blueprints/UI/BP_UI_Quest_Log.uasset
git rm Content/Blueprints/UI/BP_UI_Ship_Customization.uasset
git rm Content/Blueprints/UI/BP_UI_Trading.uasset

git commit -m "Remove deprecated BP_UI_* widgets (replaced by WBP_* in Content/UI/)"
```

### Step 5: Update Documentation

Update any documentation referencing deleted files:
- `docs/mvp/*.md` - Update WBP_Trading references
- `Blueprints/*.md` - Update Blueprint guides
- `Assets/*.md` - Update system guides

### Step 6: Clean Up Empty Directories

```bash
# If Blueprints/UI is now empty
rmdir Content/Blueprints/UI/
git commit -m "Remove empty Blueprints/UI directory"
```

---

## Verification Checklist

After cleanup:

- [ ] All duplicate files removed
- [ ] No broken references in code or blueprints
- [ ] Documentation updated
- [ ] CONTENT_ORGANIZATION.md followed
- [ ] Only one directory for UI widgets (Content/UI/)
- [ ] Consistent WBP_* naming convention
- [ ] Organized subdirectories by system
- [ ] No BP_UI_* legacy files remain
- [ ] Git history clean and documented

---

## Documentation Updates Required

### Files to Update:

1. **docs/mvp/TRADING_UI_BLUEPRINT_GUIDE.md**
   - Confirm it references `WBP_TradingInterface` (not `WBP_Trading_Interface`)

2. **docs/mvp/PHASE_1_CHECKLIST.md**
   - Update to correct widget names

3. **CONTENT_ORGANIZATION.md**
   - Add note about UI cleanup and single source structure

4. **Any Blueprint guides in Blueprints/**
   - Update references to use Content/UI/ paths

---

## Answer to Original Question

> "Why do I have two trading interface WBP? One is parent user widget one is trading interface widget?"

**Answer**: 
- You have **two duplicate files** with slightly different names:
  - `WBP_TradingInterface.uasset` (correct)
  - `WBP_Trading_Interface.uasset` (duplicate with extra underscore)

Both derive from `UTradingInterfaceWidget` C++ class (which inherits from `UUserWidget`).

The duplication is accidental - likely created during development when trying different naming conventions.

> "Should I be using the blueprints in content/blueprints/ui/ or content/ui/?"

**Answer**: Use **`Content/UI/`**

- `Content/UI/` is the correct location per CONTENT_ORGANIZATION.md
- `Content/Blueprints/UI/` contains legacy files with old naming convention
- The BP_UI_* files should be deprecated/removed
- All Widget Blueprints should be in `Content/UI/[System]/`

> "Can we design a set file structure and delete or move everything else?"

**Answer**: Yes. This document provides the cleanup plan.

**Recommended Action**:
1. Keep: `Content/UI/` with organized subdirectories
2. Delete: Duplicate widgets with underscore variations
3. Delete: Legacy `Content/Blueprints/UI/BP_UI_*` files
4. Result: Single, organized UI structure following official standards

---

## Notes

- This cleanup is safe because .uasset files are tracked in Git
- If a deleted file is needed, it can be recovered from Git history
- Always test in Unreal Editor after cleanup to verify no broken references
- Consider using Unreal's "Reference Viewer" tool to check dependencies

---

**Status**: Ready for implementation  
**Risk Level**: Low (Git provides safety net)  
**Estimated Time**: 30-60 minutes  
**Testing Required**: Yes (load project, verify no errors)
