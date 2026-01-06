# Station Module Blueprints

This folder contains Blueprint assets for all space station module types.

## Overview

Each module is a Blueprint that inherits from a C++ parent class (`ASpaceStationModule` or one of its subclasses). The C++ parent provides core functionality, while the Blueprint configures visual appearance and module-specific properties.

## Current Modules

### MVP-Critical (Trade Simulator)

- **BP_CargoBayModule.uasset** ✅ (Exists)
  - Parent: `ACargoBayModule`
  - Purpose: Cargo storage for traded goods
  - Power: 5 units

### Needed for MVP

Create these modules following the guide in `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`:

- **BP_SpaceStationModule_DockingBay** (NEEDED)
  - Parent: `ADockingBayModule`
  - Purpose: Large ship docking facilities
  - Power: 50 units

- **BP_SpaceStationModule_DockingPort** (NEEDED)
  - Parent: `ADockingPortModule`
  - Purpose: Small ship docking ports
  - Power: 10 units

- **BP_SpaceStationModule_Marketplace** (NEEDED)
  - Parent: `AMarketplaceModule`
  - Purpose: Trading interface location
  - Power: 40 units

- **BP_SpaceStationModule_Habitation** (NEEDED)
  - Parent: `AHabitationModule`
  - Purpose: Living quarters
  - Power: 30 units

- **BP_SpaceStationModule_Corridor** (NEEDED)
  - Parent: `ACorridorModule`
  - Purpose: Station passageways
  - Power: 2 units

## How to Create New Modules

See the comprehensive guide: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`

Quick steps:
1. Right-click in this folder → Blueprint Class
2. Search for parent class (e.g., `DockingBayModule`)
3. Select and name: `BP_SpaceStationModule_[ModuleName]`
4. Open, add mesh to `MeshComponent`
5. Compile and save

## Module Properties

All modules have these base properties (auto-set by C++ parent):

- **ModuleType**: Display name (e.g., "Docking Bay")
- **ModulePower**: Power consumption (+) or generation (-)
- **ModuleGroup**: Category (Docking, Storage, Public, etc.)
- **ModuleFaction**: Owner faction (optional)

## File Naming Convention

**Format**: `BP_SpaceStationModule_[ModuleName]`

**Examples**:
- ✅ `BP_SpaceStationModule_DockingBay`
- ✅ `BP_SpaceStationModule_Marketplace`
- ❌ `DockingBay` (missing prefix)
- ❌ `BP_DockingBay` (wrong format)

## Documentation

- **Full Guide**: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`
- **Quick Reference**: `docs/reference/STATION_MODULE_BLUEPRINTS_QUICK_REFERENCE.md`
- **C++ Headers**: `Source/Adastrea/Public/Stations/*Module.h`

## Related Assets

- **Parent Station**: `../BP_SpaceStation.uasset`
- **Generic Module**: `../BP_StationModule.uasset`
- **Module Catalog**: `Content/DataAssets/Stations/DA_StationModuleCatalog.uasset`

## Support

If you encounter issues creating modules:
1. Check the comprehensive guide
2. Verify C++ code compiled successfully
3. Ensure parent class is marked `Blueprintable`
4. Review console log for errors

---

**Last Updated**: 2026-01-06  
**Status**: BP_CargoBayModule exists, 5 MVP modules needed
