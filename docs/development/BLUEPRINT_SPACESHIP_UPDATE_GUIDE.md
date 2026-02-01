# Quick Guide: Updating Blueprint Spaceships After Pivot Fix

## What Changed?

All spaceships now have a `ShipRoot` component that provides a proper pivot point for rotation. This fixes the issue where ships rotated around world origin (0,0,0) instead of their center.

## Update Steps (5 minutes per Blueprint)

### 1. Open Your Blueprint

Open your spaceship Blueprint in the Unreal Editor (e.g., `BP_PlayerShip`, `BP_Ship_Fighter`, etc.)

### 2. Check Component Hierarchy

In the Components panel, you should see:
```
└─ ShipRoot (SceneComponent) [ROOT] ← NEW!
   └─ ... other components ...
```

If you don't see `ShipRoot`, try:
- Click "Compile" button
- Close and reopen the Blueprint
- File → Refresh All Nodes

### 3. Attach Your Mesh to ShipRoot

If you have a ship mesh component:

1. Select your mesh component in the hierarchy
2. Drag it onto `ShipRoot` to make it a child
3. Or use the dropdown next to the mesh and select `ShipRoot` as parent

### 4. Center the Mesh

With your mesh selected:

1. Go to the Details panel
2. Find Transform → Location
3. Set values so the ship's visual center is at `ShipRoot`'s origin:
   - X: 0.0 (or adjust if ship is front-heavy)
   - Y: 0.0
   - Z: 0.0 (or adjust if ship needs vertical offset)

**Tip**: Look at the viewport - the colored arrows (red/green/blue) show the pivot point. Your ship model should be centered on these arrows.

### 5. Test in PIE (Play In Editor)

1. Click "Play" (Alt+P)
2. Try rotating the ship
3. Verify it rotates around its center, not the world origin

### 6. Save

Click "Save" or press Ctrl+S

## Visual Example

### Before (WRONG ❌)
```
Ship mesh far from pivot point:
         Pivot (0,0,0)
            ↓
            •
            
            
     🚀 Ship Model
     (rotates around distant point)
```

### After (CORRECT ✅)
```
Ship mesh centered on pivot:
         Pivot (0,0,0)
            ↓
         🚀 •
     (ship model centered)
     (rotates around center)
```

## Common Issues

### "I don't see ShipRoot in my Blueprint"

**Solution**: 
1. Compile the Blueprint (click Compile button)
2. Close and reopen
3. Make sure your Blueprint parent class is `ASpaceship` (check Class Settings)

### "My ship still rotates wrong"

**Solution**:
1. Check your mesh is attached to `ShipRoot`, not floating
2. Adjust mesh Transform → Location to center it
3. Make sure you're testing in Play mode, not just viewing in editor

### "My camera is weird now"

**Solution**:
1. `CameraSpringArm` should be attached to `ShipRoot` (this is automatic)
2. Check `CameraSpringArm` properties:
   - Target Arm Length: 800.0 (or your preference)
   - Enable Camera Lag: true
   - Camera Lag Speed: 10.0

## Need Help?

See the full documentation: `docs/development/SPACESHIP_PIVOT_FIX.md`

## Quick Checklist

- [ ] Opened Blueprint in editor
- [ ] Verified `ShipRoot` exists as root component
- [ ] Attached ship mesh to `ShipRoot`
- [ ] Centered mesh at (0,0,0) relative to `ShipRoot`
- [ ] Tested in PIE - ship rotates around center
- [ ] Saved Blueprint

---

**Time Required**: 5 minutes per Blueprint  
**Difficulty**: Easy  
**When**: Do this next time you edit each Blueprint (not urgent, but recommended)
