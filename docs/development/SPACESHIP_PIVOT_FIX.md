# Spaceship Pivot Point Fix

**Date**: 2026-02-01  
**Issue**: Spaceships pivoting around world origin (0,0,0) instead of ship center  
**Status**: Fixed in commit bda29ab

## Problem Description

Previously, the `ASpaceship` class did not create an explicit root component. This caused several issues:

1. **Incorrect Rotation Pivot**: Ships would rotate around the world origin (0,0,0) instead of their own center
2. **Blueprint Mesh Issues**: When adding mesh components in Blueprint, they would not be properly centered
3. **Camera Behavior**: The camera spring arm was attached to the inherited root, leading to unexpected behavior

## Solution

Added a dedicated `USceneComponent` named `ShipRoot` as the root component of all spaceships:

```cpp
// In Spaceship.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
TObjectPtr<USceneComponent> ShipRoot;

// In Spaceship.cpp constructor
ShipRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ShipRoot"));
RootComponent = ShipRoot;

// Attach camera to ShipRoot
CameraSpringArm->SetupAttachment(ShipRoot);
```

## Benefits

1. **Proper Pivot Point**: Ships now rotate around their center (ShipRoot location)
2. **Blueprint Flexibility**: Designers can attach mesh components to ShipRoot and they will rotate correctly
3. **Consistent Behavior**: Follows Unreal Engine best practices for Pawn actors
4. **Camera Stability**: Camera spring arm is properly attached to the ship's center

## Impact on Existing Blueprints

### What Changed

- A new component `ShipRoot` is now the root of all Spaceship actors
- The camera spring arm is now a child of `ShipRoot` instead of the default root

### What You Need to Do

For existing Blueprint spaceships (e.g., `BP_PlayerShip`, `BP_Ship_Fighter`, etc.):

1. **Open the Blueprint** in Unreal Editor
2. **Check Component Hierarchy**: You should now see `ShipRoot` as the root component
3. **Attach Your Mesh**: If you have a mesh component, ensure it's attached to `ShipRoot`
4. **Set Mesh Location**: Position your mesh so that the ship's center of mass aligns with `ShipRoot`'s origin (0,0,0)
5. **Test Rotation**: Play the game and verify the ship rotates around its center

### Example Component Hierarchy

**Scene Component Tree** (hierarchical attachment):
```
└─ ShipRoot (SceneComponent) [ROOT]
   ├─ ShipMesh (StaticMeshComponent)
   └─ CameraSpringArm (SpringArmComponent)
      └─ Camera (CameraComponent)
```

**Owned Components** (not scene-attached):
```
- MovementComponent (FloatingPawnMovement)
- ParticleComponent (SpaceshipParticleComponent)
```

**Important**: Only scene components (those derived from `USceneComponent`) can be attached to ShipRoot. MovementComponent and ParticleComponent are `UActorComponent`-derived and don't require or support scene attachment.

## Technical Details

### Why USceneComponent?

- `USceneComponent` provides transformation (location, rotation, scale)
- Lightweight - no visual representation or collision
- Standard practice for actors that need a pivot point
- Allows child components to be attached and transform relative to it

### Alternative Solutions Considered

1. **Using Default Root**: Would require complex Blueprint setup for each ship
2. **Static Mesh as Root**: Would tie the root to a specific mesh, reducing flexibility
3. **Custom Component**: Unnecessary complexity for what's a standard pattern

### Performance Impact

- Negligible: `USceneComponent` is lightweight
- No additional processing in Tick
- Standard Unreal Engine practice

## Testing Recommendations

After updating Blueprints:

1. **Rotation Test**: Rotate the ship in place - it should spin around its center
2. **Movement Test**: Fly the ship around - camera should follow smoothly
3. **Docking Test**: Verify docking still works correctly with new pivot point
4. **Visual Test**: Ensure the ship mesh is visually centered on the pivot

## Related Files

- `Source/Adastrea/Public/Ships/Spaceship.h` - Header with ShipRoot declaration
- `Source/Adastrea/Private/Ships/Spaceship.cpp` - Implementation with component setup
- `Content/Blueprints/Ships/BP_PlayerShip.uasset` - Example Blueprint spaceship
- `Content/Blueprints/Ships/BP_Ship_Fighter.uasset` - Example Blueprint spaceship

## See Also

- [Spaceship System Documentation](../../Assets/SpaceshipSystemGuide.md)
- [Unreal Engine Pawn Documentation](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/GameFramework/APawn/)
- [Component Hierarchy Best Practices](https://docs.unrealengine.com/5.0/en-US/components-in-unreal-engine/)

## Troubleshooting

### Ship Still Rotates Around Wrong Point

**Problem**: After the fix, ship still rotates incorrectly.

**Solution**: 
1. Check that your ship's mesh component is attached to `ShipRoot`
2. Verify the mesh's relative location centers the ship's geometry at (0,0,0)
3. Use the Blueprint editor to visualize the pivot point (blue/red/green arrows)

### Camera Behaves Strangely

**Problem**: Camera doesn't follow the ship correctly.

**Solution**:
1. Verify `CameraSpringArm` is attached to `ShipRoot`
2. Check that `CameraSpringArm` has `bEnableCameraLag = true`
3. Adjust `CameraDistance` property if needed

### Old Blueprints Don't Work

**Problem**: Existing Blueprint ships don't show the new component.

**Solution**:
1. Close and reopen the Blueprint editor
2. If that doesn't work, recompile the Blueprint (click "Compile" button)
3. If still issues, create a new Blueprint based on `ASpaceship` and copy settings

## Questions?

If you encounter issues with this fix, please:
1. Check the troubleshooting section above
2. Review the example Blueprints in `Content/Blueprints/Ships/`
3. Open an issue on GitHub with details about the problem

---

**Last Updated**: 2026-02-01  
**Author**: GitHub Copilot  
**Reviewer**: Pending
