# SpaceStation Module Configuration Guide

**Created**: January 10, 2026  
**Updated**: January 18, 2026  
**Related Classes**: `ASpaceStation`, `ASpaceStationModule`

> **⚠️ This guide is deprecated.** The `DefaultModuleClasses` array has been removed in favor of the **Child Actor Components** method, which provides visual, design-time editing of station modules.
> 
> **Please use the Child Actor Components method instead:**
> - See [Module Documentation Index](../modules/README.md) for individual module setup guides
> - See [TRADING_AND_DOCKING_COMPLETE_GUIDE.md](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md#step-2-add-components) for complete station setup

---

## Recommended Approach: Child Actor Components

**Why Child Actor Components?**
- ✅ Visual feedback at design-time (see modules in editor viewport)
- ✅ Easy positioning and layout
- ✅ No runtime spawning complexity
- ✅ Station automatically discovers modules in BeginPlay
- ✅ Full C++ support with aggregate query functions

### Quick Start

1. **Open SpaceStation Blueprint**
   - Navigate to `BP_SpaceStation` (or your custom station Blueprint)
   - Open in Blueprint editor

2. **Add Modules in Components Panel**
   - Click **+ Add** → **Child Actor Component**
   - Rename to module type (e.g., "DockingBay", "Marketplace")
   - Set **Child Actor Class** to your module Blueprint
   - Position in viewport

3. **Configure Module Properties**
   - Open module Blueprint Class Defaults
   - Configure module-specific properties
   - Compile and save

4. **Test**
   - Place station in level
   - Press Play
   - Modules are automatically discovered and registered

---

## Complete Module Setup Guides

For detailed instructions on setting up each module type:

- [Docking Bay Module](../modules/DOCKING_BAY_MODULE.md) - Ship docking facilities
- [Marketplace Module](../modules/MARKETPLACE_MODULE.md) - Trading and commerce
- [Cargo Bay Module](../modules/CARGO_BAY_MODULE.md) - Storage facilities
- [Module Index](../modules/README.md) - All module documentation

---

## Station-Level Module Query Functions

The station Blueprint provides functions to view aggregate module information:

**Docking Bay Functions:**
- `GetTotalDockingPoints()` - Total docking points across all docking bays
- `GetTotalDockingCapacity()` - Maximum simultaneous docking capacity
- `GetDockingBayModule()` / `GetDockingBayModules()` - Access docking bay modules

**Marketplace Functions:**
- `GetOpenMarketplaceCount()` - Count of open marketplaces
- `GetTotalMarketplaceCount()` - Total marketplace modules
- `GetMarketplaceNames()` - Array of marketplace display names

**Usage Example:**
```
Event Construction Script
  ↓
Get Total Docking Points → Set TotalDockingPointsDisplay
  ↓
Get Open Marketplace Count → Set OpenMarketplaceCountDisplay
```

---

## Migration from DefaultModuleClasses

If you have existing stations using `DefaultModuleClasses`:

1. **Open your station Blueprint**
2. **Note which modules are in DefaultModuleClasses array**
3. **Clear the array** (it no longer exists in C++ but may still show in old Blueprints)
4. **Add modules as Child Actor Components** (see Quick Start above)
5. **Compile and save**
6. **Test in Play mode** - modules should be discovered automatically

---

**Version History:**

- **v2.0** (2026-01-18) - Deprecated DefaultModuleClasses, guide redirects to Child Actor Components method
- **v1.0** (2026-01-10) - Initial guide for DefaultModuleClasses array

---

**For Support**: See main project [README.md](../../README.md) or module-specific guides
