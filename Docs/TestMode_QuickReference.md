# Test Mode Quick Reference

## What is Test Mode?

Test Mode is a minimal testing level with:
- **Floor**: Simple plane for spatial reference
- **Player Start**: Default spawn point at (0, 0, 100)
- **Spaceship Spawn Point**: Target point at (500, 0, 200) with tag "SpaceshipSpawnPoint"
- **Basic Lighting**: Directional light, sky light, and sky atmosphere

## Quick Setup (5-10 Minutes)

### 1. Create the Level
```
File → New Level → Empty Level
Save as: Content/Maps/TestMode.umap
```

### 2. Add Essential Actors
- **Floor**: Plane mesh at (0, 0, 0), scale (100, 100, 1)
- **Player Start**: At (0, 0, 100)
- **Target Point**: At (500, 0, 200), tag "SpaceshipSpawnPoint"
- **Directional Light**: Rotation Pitch=-45, Intensity 5.0
- **Sky Light**: Default settings, Intensity 1.0
- **Sky Atmosphere**: Default settings

### 3. Create BP_TestModeGameMode
```
Content/Blueprints/ → Right-click → Blueprint Class
Parent: AdastreaGameMode
Name: BP_TestModeGameMode

Class Defaults:
- DefaultSpaceshipClass: BP_PlayerShip
- bAutoSpawnPlayerShip: true
- bSpawnAtCenter: false
```

### 4. Create BP_PlayerShip (if needed)
```
Content/Blueprints/Ships/ → Right-click → Blueprint Class
Parent: Spaceship
Name: BP_PlayerShip

Add Components:
- Static Mesh (placeholder cube)
- Spring Arm (Target Arm Length: 800)
- Camera (attached to Spring Arm)
```

### 5. Configure Level
```
World Settings:
- GameMode Override: BP_TestModeGameMode
```

### 6. Test
```
Press Play (Alt+P)
Verify spaceship spawns and is controllable
```

## Detailed Documentation

For complete instructions, see:
- **Level Specification**: [Content/Maps/TestMode.umap.txt](../Content/Maps/TestMode.umap.txt)
- **Blueprint Setup**: [Content/Blueprints/BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt)

## Coordinate Reference

| Element | Location (X, Y, Z) | Rotation |
|---------|-------------------|----------|
| Floor | (0, 0, 0) | Default |
| Player Start | (0, 0, 100) | Yaw=0 (forward) |
| Spaceship Spawn | (500, 0, 200) | Yaw=0 (forward) |

## Common Issues

### Spaceship doesn't spawn
- Check BP_TestModeGameMode is set in World Settings
- Verify DefaultSpaceshipClass is set
- Ensure Target Point has tag "SpaceshipSpawnPoint"

### Can't control spaceship
- Configure input mappings (MoveForward, MoveRight, Turn, LookUp)
- See ENHANCED_INPUT_GUIDE.md for proper input setup

### Floor not visible
- Verify static mesh is assigned
- Check material is applied
- Ensure actor is not hidden

## File Locations

```
Content/
├── Maps/
│   ├── TestMode.umap              (Created in Unreal Editor)
│   └── TestMode.umap.txt          (Specification document)
└── Blueprints/
    ├── BP_TestModeGameMode        (Created in Unreal Editor)
    ├── BP_TestModeSetup_Guide.txt (Setup instructions)
    └── Ships/
        └── BP_PlayerShip          (Created in Unreal Editor)
```

## Project Settings

To set Test Mode as editor startup map:
```
Edit → Project Settings → Maps & Modes
Editor Startup Map: /Game/Maps/TestMode
```

## Testing Checklist

- [ ] Level loads without errors
- [ ] Floor visible with collision
- [ ] Lighting adequate (not too dark/bright)
- [ ] Player Start positioned correctly
- [ ] Spaceship spawns at spawn point
- [ ] Spaceship controllable with input
- [ ] Camera follows spaceship
- [ ] No console errors

## Next Steps

After basic setup works:
1. Add spaceship DataAsset configuration
2. Implement Enhanced Input system
3. Add visual effects (thrust, weapons)
4. Add test targets and obstacles
5. Configure UI/HUD elements

## Related Documentation

- [CONTENT_CREATION_QUICKSTART.md](../CONTENT_CREATION_QUICKSTART.md) - General content creation guide
- [ENHANCED_INPUT_GUIDE.md](../ENHANCED_INPUT_GUIDE.md) - Input system setup
- [Assets/SpaceshipSystemGuide.md](../Assets/SpaceshipSystemGuide.md) - Spaceship configuration
- [BLUEPRINT_CREATION_GUIDE.md](../BLUEPRINT_CREATION_GUIDE.md) - Blueprint best practices

---

**Purpose**: Quick reference for Test Mode level setup and usage
**Last Updated**: 2025-11-05
