# Editor-Placed Modules Guide

**Last Updated**: 2026-01-17  
**Status**: NEW FEATURE - Editor-time module placement now supported

---

## Overview

Space station modules can now be added **directly in the Unreal Editor** at design-time, making them visible and editable before pressing Play. This is an alternative to the runtime spawning approach using `DefaultModuleClasses`.

### Two Ways to Add Modules

| Method | When Visible | Best For | Requires C++ |
|--------|--------------|----------|--------------|
| **Editor-Placed** | Design-time (in viewport) | Visual layout, precise positioning | ❌ No |
| **Runtime-Spawned** | Only after Play starts | Procedural generation, testing | ❌ No |

**Both methods work together!** You can mix editor-placed and runtime-spawned modules on the same station.

---

## Method 1: Child Actor Components (Recommended)

### Step-by-Step Instructions

#### 1. Open Station Blueprint

1. Navigate to `Content/Blueprints/Stations/`
2. Open `BP_SpaceStation` (or your custom station Blueprint)
3. Open in **Blueprint Editor**

#### 2. Add Child Actor Component

1. In the **Components** panel (left side), click **+ Add** button
2. Search for "Child Actor Component"
3. Select **Child Actor Component** from the list
4. Name it descriptively (e.g., `DockingBay_1`, `Marketplace_Central`)

#### 3. Configure the Child Actor

1. With the Child Actor Component selected, look at the **Details** panel (right side)
2. Find **Child Actor Class** property
3. Click the dropdown and select your module Blueprint:
   - `BP_SpaceStationModule_DockingBay`
   - `BP_SpaceStationModule_Marketplace`
   - `BP_CargoBayModule`
   - etc.

#### 4. Position the Module in Viewport

1. With the Child Actor Component still selected, switch to the **Viewport** tab
2. The module will now be visible in 3D!
3. Use the transform tools to position it:
   - **W** key - Move tool
   - **E** key - Rotate tool
   - **R** key - Scale tool
4. Position relative to the station core

#### 5. Add More Modules (Optional)

Repeat steps 2-4 for each module you want to add:
- Add multiple docking bays
- Add marketplace
- Add cargo storage
- Add habitation modules

#### 6. Compile and Save

1. Click **Compile** button (top toolbar)
2. Click **Save** button
3. Test by dragging station into level and pressing Play

### Visual Example

```
Components Panel Hierarchy:
DefaultSceneRoot
├─ StaticMesh_StationCore (your station mesh)
├─ DockingBay_1 (Child Actor Component)
│   └─ BP_SpaceStationModule_DockingBay (auto-spawned child)
│       ├─ DockingPoint_1
│       ├─ DockingPoint_2
│       ├─ DockingPoint_3
│       └─ DockingPoint_4
├─ Marketplace_Central (Child Actor Component)
│   └─ BP_SpaceStationModule_Marketplace (auto-spawned child)
└─ CargoStorage_1 (Child Actor Component)
    └─ BP_CargoBayModule (auto-spawned child)
```

---

## Method 2: Direct Actor Placement (Advanced)

### When to Use This

Use this method when you want modules to be **separate actors in the level** rather than nested inside the station Blueprint. Useful for:
- Very large stations with complex module layouts
- Stations that need modules added/removed dynamically
- Multiple stations sharing the same modules

### Step-by-Step Instructions

#### 1. Place Station in Level

1. Drag `BP_SpaceStation` from Content Browser into level
2. Position where you want the station

#### 2. Place Module Actors

1. Drag module Blueprint into level (e.g., `BP_SpaceStationModule_DockingBay`)
2. Position it near the station
3. Repeat for each module

#### 3. Attach Modules to Station

1. Select a module actor in the **World Outliner**
2. In the **Details** panel, find **Transform** section
3. Click the dropdown arrow next to **Transform**
4. Select **Attach to** → Choose your station actor
5. Repeat for each module

#### 4. Test

1. Press **Play**
2. Modules should be automatically discovered and added to station's `Modules` array

### Visual Example

```
World Outliner:
Level
├─ BP_SpaceStation_TradeHub
│   ├─ BP_SpaceStationModule_DockingBay (attached)
│   ├─ BP_SpaceStationModule_Marketplace (attached)
│   └─ BP_CargoBayModule (attached)
├─ PlayerStart
└─ DirectionalLight
```

---

## How It Works (Technical Details)

### C++ Implementation

The `ASpaceStation::BeginPlay()` function now has two phases:

```cpp
void ASpaceStation::BeginPlay()
{
    // Phase 1: Discover editor-placed modules
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    
    for (AActor* AttachedActor : AttachedActors)
    {
        ASpaceStationModule* ExistingModule = Cast<ASpaceStationModule>(AttachedActor);
        if (ExistingModule && !Modules.Contains(ExistingModule))
        {
            Modules.Add(ExistingModule);
            // Module discovered!
        }
    }
    
    // Phase 2: Spawn runtime modules (if any)
    if (DefaultModuleClasses.Num() > 0)
    {
        // Spawn modules from DefaultModuleClasses array
    }
}
```

**Key Points:**
1. **Editor-placed modules are discovered first** - ensures they're in the `Modules` array
2. **Runtime modules are spawned second** - allows both methods to coexist
3. **No conflicts** - both types of modules work together seamlessly

### Modules Array Population

The `Modules` array is automatically populated with:
- ✅ Child Actor Components (editor-placed)
- ✅ Direct level placement (editor-placed)
- ✅ DefaultModuleClasses spawned modules (runtime)
- ✅ Manually added via `AddModule()` (runtime)

All station functions (`HasMarketplace()`, `GetMarketplaceModule()`, etc.) work with ALL module types!

---

## Comparison: Editor vs Runtime

### Editor-Placed Modules

**Advantages:**
- ✅ **Visible in editor** - see layout before Play
- ✅ **Precise positioning** - use viewport transform tools
- ✅ **Visual feedback** - immediate results
- ✅ **Per-instance configuration** - each station can be unique
- ✅ **Artist-friendly** - no Class Defaults arrays
- ✅ **Easier iteration** - change layout without recompiling

**Disadvantages:**
- ⚠️ **More Blueprint work** - each station needs manual setup
- ⚠️ **Not procedural** - can't randomize easily
- ⚠️ **Harder to template** - each station is custom

### Runtime-Spawned Modules (DefaultModuleClasses)

**Advantages:**
- ✅ **Procedural generation** - can spawn different modules each time
- ✅ **Template-based** - configure once in Class Defaults, reuse everywhere
- ✅ **Easy testing** - quickly add/remove module types
- ✅ **Dynamic** - change module list without editing Blueprints

**Disadvantages:**
- ⚠️ **Not visible in editor** - can't see until Play
- ⚠️ **Less precise positioning** - modules spawn at station origin
- ⚠️ **Harder to visualize** - need to Play to see result

### Recommendation

**For MVP Trading Simulator:**
- Use **Editor-Placed Modules** (Child Actor Components)
- Stations are handcrafted, not procedural
- Visual layout is important for design
- Makes iteration faster

**For Post-MVP Features:**
- Mix both methods as needed
- Use runtime spawning for procedural content
- Use editor placement for hero stations

---

## Best Practices

### ✅ DO

- **Use Child Actor Components** for most cases (easiest workflow)
- **Name components clearly** (e.g., `DockingBay_North`, `Marketplace_1`)
- **Position modules visually** in the viewport
- **Test both in editor and in-game** to verify placement
- **Keep DefaultModuleClasses empty** if using editor placement (avoid duplication)

### ❌ DON'T

- **Don't mix both methods carelessly** - can lead to duplicate modules
- **Don't forget to attach** if using direct level placement
- **Don't position modules too close** - they can overlap visually
- **Don't rely on runtime spawning** if visual layout matters

---

## Common Issues and Solutions

### Issue: Modules Not Appearing

**Problem:** Added Child Actor Component but module doesn't appear.

**Solutions:**
1. ✅ Ensure **Child Actor Class** is set (not None)
2. ✅ Check module Blueprint compiles successfully
3. ✅ Verify module has a visible Static Mesh component
4. ✅ Check module isn't hidden in editor (eye icon in outliner)

### Issue: Modules Floating Away

**Problem:** Modules appear in wrong location or drift away from station.

**Solutions:**
1. ✅ Ensure Child Actor Component is **child of station root** in hierarchy
2. ✅ Check **Attachment Rules** are set to Keep Relative
3. ✅ Verify transform is relative to parent, not world space
4. ✅ Use **Snap to** tools for precise alignment

### Issue: Duplicate Modules

**Problem:** Same module appears twice or multiple times.

**Solutions:**
1. ✅ If using editor placement, **clear DefaultModuleClasses array**
2. ✅ Check you didn't add same module multiple times
3. ✅ Verify no duplicate Child Actor Components
4. ✅ Check Output Log for duplicate warnings

### Issue: Can't Find Module at Runtime

**Problem:** `HasMarketplace()` returns false even though module is in editor.

**Solutions:**
1. ✅ Ensure module is **attached to station** in hierarchy
2. ✅ Check module Blueprint inherits from correct C++ class
3. ✅ Verify module's `ModuleType` property is set correctly
4. ✅ Check Output Log for discovery messages in BeginPlay

---

## Testing Checklist

### Editor Testing

- [ ] Child Actor Component added to station Blueprint
- [ ] Child Actor Class set to module Blueprint
- [ ] Module visible in viewport at design-time
- [ ] Module positioned correctly relative to station
- [ ] Components panel shows correct hierarchy
- [ ] Blueprint compiles without errors

### Runtime Testing

- [ ] Place station in level
- [ ] Press Play
- [ ] Check Output Log for "Discovered editor-placed module" messages
- [ ] Verify `Modules` array populated (check with Print String)
- [ ] Test station functions: `HasMarketplace()`, `HasDockingCapability()`
- [ ] Dock at station and verify trading works
- [ ] No duplicate modules appear

### Integration Testing

- [ ] Multiple modules work together
- [ ] Docking points on docking bay work correctly
- [ ] Marketplace module opens trading UI
- [ ] Station aggregate functions work (power, capabilities)
- [ ] No crashes or warnings in Output Log

---

## Example Workflow: Creating a Trade Station

### Goal
Create an agricultural trading station with:
- 1 docking bay (4 docking points)
- 1 marketplace (agriculture goods)
- 2 cargo storage modules
- 1 habitation module

### Steps

1. **Create Station Blueprint**
   - Content Browser → Blueprint Class → ASpaceStation
   - Name: `BP_TradingStation_Agricultural`

2. **Add Visual Mesh**
   - Add Static Mesh Component
   - Set mesh to station core model
   - Scale and position as needed

3. **Add Docking Bay (Child Actor Component)**
   - Add Component → Child Actor Component
   - Name: `DockingBay_Main`
   - Child Actor Class: `BP_SpaceStationModule_DockingBay`
   - Position: Front of station, facing outward

4. **Add Marketplace (Child Actor Component)**
   - Add Component → Child Actor Component
   - Name: `Marketplace_Agricultural`
   - Child Actor Class: `BP_SpaceStationModule_Marketplace`
   - Position: Center of station
   - Configure: In Class Defaults, set MarketDataAsset to `DA_Market_Agricultural`

5. **Add Cargo Storage (Child Actor Components)**
   - Add Component → Child Actor Component × 2
   - Names: `CargoStorage_1`, `CargoStorage_2`
   - Child Actor Class: `BP_CargoBayModule`
   - Position: Left and right sides of station

6. **Add Habitation (Child Actor Component)**
   - Add Component → Child Actor Component
   - Name: `Habitation_Crew`
   - Child Actor Class: `BP_SpaceStationModule_Habitation`
   - Position: Rear of station

7. **Configure Station Properties**
   - Class Defaults → StationName: "Agricultural Hub Alpha"
   - MaxStructuralIntegrity: 10000
   - Leave **DefaultModuleClasses empty** (using editor placement)

8. **Compile, Save, Test**
   - Compile Blueprint
   - Save
   - Drag into level
   - Press Play
   - Fly to station and dock
   - Open trading UI
   - Verify agricultural goods available

### Expected Result

```
Station visible in editor with:
- Visual station core mesh
- 5 visible module actors (docking bay, marketplace, 2 cargo, habitation)
- All positioned exactly where you placed them
- No runtime spawning needed
- All modules functional when game runs
```

---

## Migrating from Runtime Spawning

If you have existing stations using `DefaultModuleClasses`, you can convert them:

### Migration Steps

1. **Open Station Blueprint** that uses DefaultModuleClasses
2. **Note which modules are in array** (e.g., DockingBay, Marketplace)
3. **Clear DefaultModuleClasses array** (or keep for backup)
4. **Add Child Actor Components** for each module type
5. **Position modules in viewport** as desired
6. **Test** to verify everything works
7. **Delete backup** DefaultModuleClasses if satisfied

### Before (Runtime Spawning)

```
Class Defaults:
  DefaultModuleClasses:
    [0] BP_SpaceStationModule_DockingBay
    [1] BP_SpaceStationModule_Marketplace

Components:
  DefaultSceneRoot
  └─ StaticMesh_StationCore
```

### After (Editor Placement)

```
Class Defaults:
  DefaultModuleClasses: (empty)

Components:
  DefaultSceneRoot
  ├─ StaticMesh_StationCore
  ├─ DockingBay_1 (Child Actor)
  └─ Marketplace_Central (Child Actor)
```

**Result:** Modules now visible in editor, same functionality at runtime!

---

## Related Documentation

- **Station Architecture**: `docs/reference/STATION_MODULE_ARCHITECTURE.md`
- **Adding Modules (Runtime)**: `docs/reference/ADDING_MODULES_TO_STATIONS.md`
- **Docking System**: `docs/mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md`
- **Module Blueprints**: `docs/setup/STATION_MODULE_BLUEPRINTS_GUIDE.md`

---

## Summary

### Key Takeaways

1. ✅ **Modules can now be added in the editor** using Child Actor Components
2. ✅ **Visible at design-time** - no need to press Play to see layout
3. ✅ **Works alongside runtime spawning** - both methods supported
4. ✅ **No C++ knowledge required** - purely Blueprint workflow
5. ✅ **Better for MVP** - handcrafted stations with visual layout
6. ✅ **Automatic discovery** - station finds modules in BeginPlay

### Quick Start

**To add modules in editor:**
1. Open station Blueprint
2. Add → Child Actor Component
3. Set Child Actor Class to module Blueprint
4. Position in viewport
5. Compile, save, done!

**Modules are automatically discovered and tracked at runtime. All station functions work correctly!**

---

**Last Updated**: 2026-01-17  
**Feature Added**: PR #XXX - Support editor-placed modules via Child Actor Components  
**Status**: ✅ Ready to Use
