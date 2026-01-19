# Docking Point Migration Guide

**Date**: 2026-01-19  
**Affects**: DockingBayModule and DockingPortModule  
**Breaking Change**: No (backward compatible)

---

## Overview

The docking point setup workflow has been simplified to eliminate an Unreal Engine UI limitation. Component arrays are now populated automatically using tags instead of manual Class Defaults configuration.

**Key Change**: Scene Components must now be tagged with `"DockingPoint"` instead of manually added to the `DockingPoints` array.

---

## What Changed?

### Before (Old Workflow)

```
1. Add Scene Components in Components panel
2. Open Class Defaults
3. Try to populate DockingPoints array
4. ❌ PROBLEM: Can only create new components, not select existing ones
5. Array remains empty → ships can't dock
```

### After (New Workflow)

```
1. Add Scene Components in Components panel
2. Tag each component with "DockingPoint"
3. ✅ DONE: Array populates automatically at BeginPlay
```

---

## Why the Change?

**Unreal Engine Limitation**: `TArray<USceneComponent*>` with `EditAnywhere` in Class Defaults only allows creating new component instances, not selecting existing components from the Blueprint hierarchy. This is an engine-level design decision related to component ownership.

**Solution**: Use component tags for discovery. The C++ code now automatically finds and populates components at runtime using `GetComponentsByTag()`.

---

## Migration Steps (Existing Projects)

If you have existing docking bay or docking port modules using the old workflow, follow these steps:

### Step 1: Open Your Docking Module Blueprint

Open `BP_SpaceStationModule_DockingBay` or `BP_SpaceStationModule_DockingPort` in the Unreal Editor.

### Step 2: Add Tags to Scene Components

For each docking point Scene Component:

1. **Select the Scene Component** in the Components panel
   - Example: `DockingPoint_1`, `DockingPoint_2`, etc.

2. **Find the "Tags" Section** in Details panel
   - Look for **Component Tags** (usually near the bottom)
   - You'll see: `▼ Component Tags [0]` (empty array)

3. **Add "DockingPoint" Tag**:
   - Click **+ Add Element** button
   - In the text field, type: `DockingPoint`
   - Must be exact spelling, case-sensitive!

4. **Repeat for All Docking Points**:
   - Tag every Scene Component you want to use as a docking point

### Step 3: Clear Old Array (Optional)

The old `DockingPoints` array in Class Defaults will be ignored, but you can clear it for cleanliness:

1. Open **Class Defaults**
2. Find **Docking → Docking Points** array
3. Delete all elements (the array will be repopulated automatically)

**Note**: This step is optional. The new code clears and repopulates the array automatically at `BeginPlay`, so old entries won't cause issues.

### Step 4: Compile and Test

1. **Compile the Blueprint**
2. **Save all changes**
3. **Test in-game**:
   - Place station in level
   - Play in editor
   - Check for debug messages: `[DOCKING] Found X components tagged as 'DockingPoint'`
   - Verify ships can dock successfully

---

## Verification Checklist

After migration, verify:

- [ ] Each docking point Scene Component has "DockingPoint" tag
- [ ] No spelling errors in tags (case-sensitive!)
- [ ] Number of tagged components ≥ MaxDockedShips
- [ ] Blueprint compiles without errors
- [ ] Debug messages confirm components found (if DOCKING_DEBUG_ENABLED)
- [ ] Ships can dock successfully in-game

---

## Troubleshooting

### Issue: "No docking points found" Error

**Symptoms**: 
- Log message: `[DOCKING] WARNING: No docking points found!`
- Ships can't dock

**Solution**:
1. Check each Scene Component has "DockingPoint" tag (exact spelling)
2. Verify tags are in **Component Tags**, not **Actor Tags**
3. Ensure Scene Components are children of the docking module actor

### Issue: Fewer Points Than Expected

**Symptoms**:
- Log message: `[DOCKING] WARNING: Only X docking points for capacity of Y ships`
- Some ships can't dock

**Solution**:
1. Verify all docking point Scene Components are tagged
2. Check for typos in tag names
3. Ensure `MaxDockedShips` matches number of tagged components

### Issue: Wrong Components Detected

**Symptoms**:
- Ships dock at incorrect locations
- Unexpected components in array

**Solution**:
1. Only tag Scene Components meant for docking
2. Remove "DockingPoint" tag from any incorrect components
3. Verify Scene Component positions are correct

---

## Technical Details

### C++ Changes

**New Functions**:
- `BeginPlay()` - Automatically calls `PopulateDockingPointsFromTags()`
- `PopulateDockingPointsFromTags()` - Discovers and populates array

**How It Works**:
```cpp
void ADockingBayModule::BeginPlay()
{
    Super::BeginPlay();
    PopulateDockingPointsFromTags();
}

void ADockingBayModule::PopulateDockingPointsFromTags()
{
    DockingPoints.Empty(); // Clear old entries
    
    // Find all components tagged with "DockingPoint"
    TArray<UActorComponent*> TaggedComponents;
    GetComponentsByTag(USceneComponent::StaticClass(), 
                      FName("DockingPoint"), 
                      TaggedComponents);
    
    // Cast and add to array
    for (UActorComponent* Component : TaggedComponents)
    {
        if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
        {
            DockingPoints.Add(SceneComp);
        }
    }
    
    // Warn if issues detected
    // ... (validation code)
}
```

**Property Changes**:
- `DockingPoints` UPROPERTY changed from `EditAnywhere` to `VisibleAnywhere`
- Array is now read-only in editor (populated automatically)

---

## Benefits of New System

1. **Eliminates UI Limitation**: No more confusing "can only create new" dropdown
2. **Designer-Friendly**: Simpler workflow with clear tagging
3. **Runtime Validation**: Debug messages confirm correct setup
4. **Automatic Discovery**: No manual array population needed
5. **Error Detection**: Warns about missing or insufficient docking points

---

## Backward Compatibility

**Safe to Update**: This change is backward compatible.

- Old Blueprints will continue to work (just add tags)
- No data loss or breaking changes
- Old array entries are ignored and replaced
- Migration is non-destructive

---

## Related Documentation

- [DOCKING_BAY_MODULE.md](../modules/DOCKING_BAY_MODULE.md) - Complete setup guide
- [TRADING_AND_DOCKING_COMPLETE_GUIDE.md](../mvp/TRADING_AND_DOCKING_COMPLETE_GUIDE.md) - Full trading system guide
- [Visual Comparison](images/blueprints/docking_point_tag_workflow.svg) - Old vs New workflow diagram

---

## Support

If you encounter issues during migration:

1. Check debug messages in log (filter for "[DOCKING]")
2. Verify tag spelling: `DockingPoint` (case-sensitive)
3. Ensure components are Scene Components, not other types
4. Review troubleshooting section above
5. See visual guide: `docking_point_tag_workflow.svg`

---

**Migration Difficulty**: Easy (5-10 minutes per module)  
**Recommended**: Update all docking modules at once for consistency

---

**Last Updated**: 2026-01-19  
**Maintained by**: Adastrea Development Team
