# File Structure Standard - Trade Simulator MVP

**Status**: ✅ OFFICIAL STANDARD  
**Effective Date**: 2026-01-02  
**Authority**: Resolves Issue #[file structure inconsistencies]  
**Supersedes**: All conflicting file structure references in documentation

---

## 🎯 Purpose

This document establishes the **single, authoritative file structure standard** for the Adastrea Trade Simulator MVP. All documentation, guides, and implementation must follow these conventions.

---

## 📁 Official Directory Structure

### Complete Content Organization

```
Content/
├── Blueprints/           # Non-UI Blueprint classes
│   ├── Trading/          # Trading system Blueprints
│   │   ├── Stations/     # BP_TradingStation, BP_Station_*
│   │   └── Ships/        # BP_TradingShip, BP_SimpleTradingShip
│   ├── GameModes/        # BP_TradingGameMode
│   └── [Other Systems]/
│
├── UI/                   # ALL UI widgets (no exceptions)
│   └── Trading/          # Trading interface widgets
│       ├── WBP_TradingUI.uasset         (main interface)
│       └── WBP_TradeItemRow.uasset      (item widget)
│
├── DataAssets/           # Data Asset instances
│   └── Trading/
│       ├── Items/        # DA_TradeItem_*
│       └── Markets/      # DA_Market_*
│
└── Maps/                 # Level maps
    └── Trading/          # L_TradingTest, L_TradingDemo
```

### Key Principles

1. **NO `Content/Blueprints/MVP/` folder** - MVP assets integrated into main structure
2. **ALL UI widgets in `Content/UI/[System]/`** - No UI in Blueprints folder
3. **System-based organization** - Group by game system, not development phase
4. **Consistent naming** - Follow established prefixes and conventions

---

## 🏷️ Official Naming Conventions

### Trading UI Widgets

**✅ CORRECT NAMING:**
- `WBP_TradingUI` - Main trading interface widget
- `WBP_TradeItemRow` - Individual item row widget
- `WBP_SimpleHUD` - Simple HUD for MVP testing

**❌ DEPRECATED NAMING:**
- ~~`WBP_TradingUI`~~ - Old name, replaced by WBP_TradingUI
- ~~`WBP_Trading_Interface`~~ - Incorrect underscore
- ~~`BP_UI_Trading`~~ - Legacy naming convention

### Trading Blueprints

**✅ CORRECT NAMING:**
- `BP_TradingStation` - Base trading station actor
- `BP_Station_Agricultural` - Specific station variant
- `BP_Station_Industrial` - Specific station variant
- `BP_TradingShip` - Player trading ship
- `BP_SimpleTradingShip` - Simplified MVP ship
- `BP_TradingGameMode` - Trading game mode

**Format**: `BP_[System][Type]_[Variant]`

### Data Assets

**✅ CORRECT NAMING:**
- `DA_TradeItem_Water` - Trade item data asset
- `DA_TradeItem_Food` - Trade item data asset
- `DA_Market_Agricultural` - Market configuration
- `DA_Market_Industrial` - Market configuration

**Format**: `DA_[Type]_[Name]`

### Maps

**✅ CORRECT NAMING:**
- `L_TradingTest` - Trading test level
- `L_TradingDemo` - Trading demo level

**Format**: `L_[Purpose][Variant]`

---

## 📍 Official File Locations

### Widget Blueprints

| Widget | Location | Purpose |
|--------|----------|---------|
| `WBP_TradingUI` | `Content/UI/Trading/` | Main trading interface |
| `WBP_TradeItemRow` | `Content/UI/Trading/` | Item list row widget |
| `WBP_SimpleHUD` | `Content/UI/HUD/` | MVP testing HUD |

### Actor Blueprints

| Blueprint | Location | Purpose |
|-----------|----------|---------|
| `BP_TradingStation` | `Content/Blueprints/Trading/Stations/` | Base station class |
| `BP_Station_Agricultural` | `Content/Blueprints/Trading/Stations/` | Agricultural station |
| `BP_Station_Industrial` | `Content/Blueprints/Trading/Stations/` | Industrial station |
| `BP_TradingShip` | `Content/Blueprints/Trading/Ships/` | Player trading ship |
| `BP_SimpleTradingShip` | `Content/Blueprints/Trading/Ships/` | Simplified MVP ship |
| `BP_TradingGameMode` | `Content/Blueprints/GameModes/` | Trading game mode |

### Data Assets

| Data Asset | Location | Purpose |
|------------|----------|---------|
| `DA_TradeItem_*` | `Content/DataAssets/Trading/Items/` | Trade item definitions |
| `DA_Market_*` | `Content/DataAssets/Trading/Markets/` | Market configurations |

### Maps

| Map | Location | Purpose |
|-----|----------|---------|
| `L_TradingTest` | `Content/Maps/Trading/` | MVP testing level |
| `L_TradingDemo` | `Content/Maps/Trading/` | Demo presentation |

---

## 🚫 What NOT to Do

### ❌ Don't Create These Folders

- `Content/Blueprints/MVP/` - Use main structure instead
- `Content/Blueprints/UI/` - UI goes in `Content/UI/`
- `Content/MVP/` - No separate MVP folder needed
- `Content/UI/MVP/` - Use `Content/UI/Trading/` instead

### ❌ Don't Use These Naming Patterns

- `WBP_TradingUI` - Use `WBP_TradingUI`
- `BP_UI_*` - Legacy pattern, use `WBP_*`
- `BP_MVP_*` - No MVP prefix needed
- Extra underscores: `WBP_Trading_Interface`

---

## 🔄 Migration Guide

### If You Have Existing Files

**Old Location** → **New Location**:
```
Content/Blueprints/MVP/UI/WBP_TradingUI
→ Content/UI/Trading/WBP_TradingUI

Content/Blueprints/MVP/Stations/BP_Station_Agricultural
→ Content/Blueprints/Trading/Stations/BP_Station_Agricultural

Content/Blueprints/MVP/Ships/BP_SimpleTradingShip
→ Content/Blueprints/Trading/Ships/BP_SimpleTradingShip
```

**Old Name** → **New Name**:
```
WBP_TradingUI → WBP_TradingUI
WBP_Trading_Interface → WBP_TradingUI
BP_UI_Trading → WBP_TradingUI
```

### Migration Steps

1. **In Unreal Editor**:
   - Use "Rename" (not move) to update asset names
   - Use "Move" to relocate to correct folder
   - Fix up redirectors after moving

2. **In Documentation**:
   - Update all file path references
   - Update all asset name references
   - Verify screenshots match new names

3. **Verify No Broken References**:
   - Check Reference Viewer for each moved asset
   - Test in PIE (Play In Editor)
   - Verify all Blueprints compile

---

## 📚 Quick Reference

### Common Paths (Copy-Paste Ready)

**Widget Blueprint Creation**:
```
Content/UI/Trading/
Name: WBP_TradingUI
Parent: TradingInterfaceWidget (C++ class)
```

**Station Blueprint Creation**:
```
Content/Blueprints/Trading/Stations/
Name: BP_Station_Agricultural
Parent: ATradingStation (or Actor)
```

**Data Asset Creation**:
```
Content/DataAssets/Trading/Items/
Name: DA_TradeItem_Water
Parent: TradeItemDataAsset
```

### Decision Tree

**"Where do I put this file?"**

```
Is it a Widget Blueprint (UI)?
├─ YES → Content/UI/[System]/
└─ NO → Is it a Blueprint class?
    ├─ YES → Content/Blueprints/[System]/
    └─ NO → Is it a Data Asset?
        ├─ YES → Content/DataAssets/[System]/
        └─ NO → Content/[AssetType]/[System]/
```

**"What do I name this widget?"**

```
Trading interface widget → WBP_TradingUI
Trading item row widget → WBP_TradeItemRow
HUD widget → WBP_SimpleHUD
```

**"What do I name this Blueprint?"**

```
Format: BP_[System][Type]_[Variant]

Examples:
- Trading station → BP_TradingStation
- Agricultural station → BP_Station_Agricultural
- Trading ship → BP_TradingShip
```

---

## ✅ Validation Checklist

Before creating new files:

- [ ] Check this document for official location
- [ ] Verify naming follows established convention
- [ ] Confirm folder exists (create if needed)
- [ ] No MVP-specific folders used
- [ ] UI widgets in `Content/UI/` only
- [ ] Blueprints in `Content/Blueprints/[System]/`

Before updating documentation:

- [ ] Use official file paths from this document
- [ ] Use official asset names from this document
- [ ] Check no deprecated names used
- [ ] Verify screenshots match current structure
- [ ] Cross-reference with CONTENT_ORGANIZATION.md

---

## 📖 Related Documentation

**Primary References**:
- [CONTENT_ORGANIZATION.md](/home/runner/work/Adastrea/Adastrea/Content/CONTENT_ORGANIZATION.md) - General organization rules
- [CODE_STYLE.md](/home/runner/work/Adastrea/Adastrea/CODE_STYLE.md) - Naming conventions
- [UI_FILE_STRUCTURE_FINAL.md](/home/runner/work/Adastrea/Adastrea/docs/development/UI_FILE_STRUCTURE_FINAL.md) - UI cleanup details

**Implementation Guides**:
- [TRADING_MVP_GUIDE.md](/home/runner/work/Adastrea/Adastrea/docs/mvp/TRADING_MVP_GUIDE.md) - MVP implementation
- [BLUEPRINT_IMPLEMENTATION_GUIDE.md](/home/runner/work/Adastrea/Adastrea/docs/mvp/BLUEPRINT_IMPLEMENTATION_GUIDE.md) - Blueprint creation
- [TRADING_UI_QUICKSTART.md](/home/runner/work/Adastrea/Adastrea/docs/mvp/TRADING_UI_QUICKSTART.md) - UI setup

---

## 🎯 Standard Enforcement

**This document is AUTHORITATIVE.**

All conflicting references in other documentation are superseded by this standard.

When you encounter conflicting information:
1. ✅ Follow THIS document
2. 📝 Report the conflict (open issue or update doc)
3. 🔄 Help update the conflicting documentation

**Common Conflicts to Ignore**:
- Older guides referencing `Content/Blueprints/MVP/`
- References to `WBP_TradingUI` instead of `WBP_TradingUI`
- UI widgets in `Content/Blueprints/UI/`

**Correct Approach**: Use the standards defined here.

---

## 🔄 Document Maintenance

**Last Updated**: 2026-01-02  
**Version**: 1.0  
**Status**: Official Standard  
**Review Frequency**: When major structure changes occur  
**Owner**: Project Architecture Team

**Change Log**:
- 2026-01-02: Initial standard established (resolves file structure inconsistencies)

---

**When in doubt, refer to this document. This is the source of truth for file structure.**
