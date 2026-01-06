# Station Module Blueprints Quick Reference

**Quick access guide for creating BP_SpaceStationModule_* blueprints**

---

## MVP-Critical Modules (Create First)

| Blueprint Name | C++ Parent | Power | Group | Purpose |
|----------------|------------|-------|-------|---------|
| `BP_SpaceStationModule_DockingBay` | `ADockingBayModule` | 50 | Docking | Large ship docking |
| `BP_SpaceStationModule_DockingPort` | `ADockingPortModule` | 10 | Docking | Small ship docking |
| `BP_SpaceStationModule_Marketplace` | `AMarketplaceModule` | 40 | Public | Trading interface |
| `BP_SpaceStationModule_CargoBay` | `ACargoBayModule` | 5 | Storage | Cargo storage (EXISTS) |
| `BP_SpaceStationModule_Habitation` | `AHabitationModule` | 30 | Habitation | Living quarters |
| `BP_SpaceStationModule_Corridor` | `ACorridorModule` | 2 | Connection | Passageways |

---

## Quick Creation Steps

1. **Navigate**: `Content/Blueprints/Stations/Modules/`
2. **Create**: Right-click → Blueprint Class → Search parent class → Select
3. **Name**: `BP_SpaceStationModule_[ModuleName]`
4. **Configure**: Add mesh to `MeshComponent`
5. **Save**: Compile and save

---

## All Module Types Reference

### Docking (MVP)
- **DockingBay**: 50 power, large ships
- **DockingPort**: 10 power, small ships

### Storage (MVP)
- **CargoBay**: 5 power, general storage
- **FuelDepot**: 15 power, fuel storage (POST-MVP)

### Public (MVP)
- **Marketplace**: 40 power, trading

### Habitation (MVP)
- **Habitation**: 30 power, living quarters
- **Barracks**: 20 power, military quarters (POST-MVP)

### Connection (MVP)
- **Corridor**: 2 power, passageways

### Power (POST-MVP)
- **Reactor**: -500 power (generates)
- **SolarArray**: -100 power (generates)

### Processing (POST-MVP)
- **Fabrication**: 150 power, manufacturing
- **Processing**: 100 power, refinement
- **ScienceLab**: 75 power, research

### Defence (POST-MVP)
- **ShieldGenerator**: 200 power, shields
- **Turret**: 25 power, weapons

---

## File Locations

- **Guide**: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`
- **C++ Headers**: `Source/Adastrea/Public/Stations/*Module.h`
- **C++ Implementation**: `Source/Adastrea/Private/Stations/*Module.cpp`
- **Blueprint Folder**: `Content/Blueprints/Stations/Modules/`

---

## Common Parent Classes

All modules inherit from `ASpaceStationModule` which provides:
- `ModuleType` - Display name (auto-set)
- `ModulePower` - Power usage (auto-set)
- `ModuleGroup` - Category (auto-set)
- `ModuleFaction` - Owner faction (optional)
- `MeshComponent` - Visual mesh (configure in Blueprint)

---

## Testing Checklist

- [ ] Blueprint compiles without errors
- [ ] Mesh appears when placed in level
- [ ] ModuleType displays correctly
- [ ] ModulePower shows correct value
- [ ] ModuleGroup set to correct category
- [ ] Can attach to BP_SpaceStation (if exists)

---

## Quick Troubleshooting

**Can't find parent class?**
→ Compile C++ code and restart editor

**No mesh showing?**
→ Assign Static Mesh to MeshComponent

**Wrong power value?**
→ Set in C++ parent constructor, not Blueprint

**Module not attaching?**
→ Check BP_SpaceStation has attachment logic

---

For detailed instructions, see: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`
