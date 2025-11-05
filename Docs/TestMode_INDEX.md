# Test Mode Documentation - Complete Index

## Overview

Test Mode is a minimal test level for rapid gameplay testing and development in Adastrea. This index provides a complete guide to all Test Mode documentation.

## Purpose

Test Mode provides a controlled environment for:
- **Quick Testing**: Rapid iteration on spaceship controls and mechanics
- **Development**: Testing new features in isolation
- **Debugging**: Controlled environment for bug reproduction
- **Training**: Learning Unreal Editor and Adastrea systems
- **Prototyping**: Trying out new gameplay ideas quickly

## Documentation Structure

### Quick Start (5-10 minutes)
**→ [TestMode_QuickReference.md](TestMode_QuickReference.md)**
- Minimal setup checklist
- Coordinate reference table
- Common issues and solutions
- Testing checklist

**Best for:** Getting started quickly, or as a reminder for setup steps

### Complete Level Specification
**→ [../Content/Maps/TestMode.umap.txt](../Content/Maps/TestMode.umap.txt)**
- Detailed level requirements
- Environment setup (lighting, post-processing)
- Game mode configuration
- Step-by-step creation guide in Unreal Editor
- Comprehensive troubleshooting

**Best for:** Complete understanding of the level structure and requirements

### Blueprint Setup Guide
**→ [../Content/Blueprints/BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt)**
- BP_TestModeGameMode creation and configuration
- BP_PlayerShip setup with components
- Input system configuration
- Debugging Blueprint issues
- Blueprint organization best practices

**Best for:** Creating and configuring Blueprints for Test Mode

### Visual Layout Reference
**→ [TestMode_Layout_Diagram.txt](TestMode_Layout_Diagram.txt)**
- ASCII art diagrams (top-down, side, 3D views)
- Coordinate reference with distances
- Actor hierarchy
- Visual debugging markers
- Material setup visualization

**Best for:** Understanding spatial relationships and visual reference

## File Structure

```
Adastrea/
├── Content/
│   ├── Blueprints/
│   │   ├── BP_TestModeGameMode        (Created in UE Editor)
│   │   ├── BP_TestModeSetup_Guide.txt (Setup documentation)
│   │   └── Ships/
│   │       └── BP_PlayerShip          (Created in UE Editor)
│   └── Maps/
│       ├── TestMode.umap              (Created in UE Editor)
│       └── TestMode.umap.txt          (Level specification)
└── Docs/
    ├── TestMode_INDEX.md              (This file)
    ├── TestMode_QuickReference.md     (Quick start guide)
    └── TestMode_Layout_Diagram.txt    (Visual reference)
```

## Quick Access Guide

### What to Read When...

**I want to create Test Mode for the first time:**
1. Start with [TestMode_QuickReference.md](TestMode_QuickReference.md)
2. Follow steps in order
3. Refer to [TestMode.umap.txt](../Content/Maps/TestMode.umap.txt) for details
4. Use [BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt) for Blueprints

**I need to understand the level layout:**
- Read [TestMode_Layout_Diagram.txt](TestMode_Layout_Diagram.txt)
- Check coordinate reference in [TestMode_QuickReference.md](TestMode_QuickReference.md#coordinate-reference)

**I'm having issues with spaceship spawning:**
- See "Debugging Common Issues" in [BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt#debugging-common-issues)
- Check troubleshooting in [TestMode.umap.txt](../Content/Maps/TestMode.umap.txt#troubleshooting)

**I want to customize Test Mode:**
- Read "Optional Testing Elements" in [TestMode.umap.txt](../Content/Maps/TestMode.umap.txt#optional-testing-elements)
- See "Visual Debugging Markers" in [TestMode_Layout_Diagram.txt](TestMode_Layout_Diagram.txt#visual-debugging-markers-optional)

## Key Concepts

### Coordinate System

Test Mode uses Unreal Engine's standard coordinate system:
- **X-axis**: Forward (North)
- **Y-axis**: Right (East)
- **Z-axis**: Up

Key positions:
- Floor: (0, 0, 0)
- Player Start: (0, 0, 100)
- Spaceship Spawn: (500, 0, 200)

### Game Mode Configuration

Test Mode uses **BP_TestModeGameMode** (based on AdastreaGameMode) with:
- `DefaultSpaceshipClass`: BP_PlayerShip
- `bAutoSpawnPlayerShip`: true
- `bSpawnAtCenter`: false

### Spawning System

The spaceship spawn system uses:
1. Target Point actor at spawn location
2. Actor tag: "SpaceshipSpawnPoint"
3. Game Mode finds tagged actor and spawns spaceship there
4. Player controller possesses spawned spaceship

## Prerequisites

Before creating Test Mode, ensure you have:
- ✅ Unreal Engine 5.5 installed
- ✅ Adastrea project opened in UE Editor
- ✅ Basic familiarity with Unreal Editor
- ✅ Understanding of Blueprints (or willingness to learn)

## Setup Time Estimates

| Task | Time Required | Difficulty |
|------|---------------|------------|
| Create basic level | 5 minutes | Easy |
| Add lighting | 2 minutes | Easy |
| Create BP_TestModeGameMode | 3 minutes | Easy |
| Create BP_PlayerShip | 5 minutes | Medium |
| Configure input | 5 minutes | Medium |
| **Total (first time)** | **20 minutes** | **Easy-Medium** |
| **Total (experienced)** | **10 minutes** | **Easy** |

## Related Project Documentation

### Unreal Engine Setup
- [EmptyMapSetup.md](EmptyMapSetup.md) - Creating large sector maps (200km)
- [../CONTENT_CREATION_QUICKSTART.md](../CONTENT_CREATION_QUICKSTART.md) - General content creation

### Input System
- [../ENHANCED_INPUT_GUIDE.md](../ENHANCED_INPUT_GUIDE.md) - Enhanced Input setup

### Blueprint Development
- [../BLUEPRINT_CREATION_GUIDE.md](../BLUEPRINT_CREATION_GUIDE.md) - Blueprint best practices

### Spaceship System
- [../Assets/SpaceshipSystemGuide.md](../Assets/SpaceshipSystemGuide.md) - Complete spaceship system
- [../Assets/SpaceshipDesignerWorkflow.md](../Assets/SpaceshipDesignerWorkflow.md) - Designer workflow

### Project Architecture
- [../ARCHITECTURE.md](../ARCHITECTURE.md) - Overall architecture
- [../README.md](../README.md) - Project overview

## Differences: Test Mode vs Sector Maps

| Feature | Test Mode | Sector Maps |
|---------|-----------|-------------|
| **Purpose** | Quick testing | Full gameplay |
| **Size** | ~1km x 1km | 200km x 200km |
| **Complexity** | Minimal | Full featured |
| **Setup Time** | 10-20 minutes | Hours |
| **Content** | Floor + spawn points | Planets, stations, etc. |
| **Use Case** | Development/testing | Production gameplay |

## Workflow Integration

### Development Workflow
```
1. Create Test Mode (one time setup)
   ↓
2. Develop feature in C++
   ↓
3. Test in Test Mode level
   ↓
4. Iterate based on results
   ↓
5. Once stable, test in full sector map
   ↓
6. Deploy to production
```

### Designer Workflow
```
1. Idea for new ship/feature
   ↓
2. Create Data Asset
   ↓
3. Create Blueprint referencing Data Asset
   ↓
4. Test in Test Mode
   ↓
5. Refine based on testing
   ↓
6. Document in project
   ↓
7. Add to full gameplay sector
```

## Testing Checklist

### Initial Setup Verification
- [ ] Level opens without errors
- [ ] Floor visible and has collision
- [ ] Lighting adequate (can see clearly)
- [ ] Player Start positioned correctly (0, 0, 100)
- [ ] Spaceship Spawn Point marked with tag
- [ ] BP_TestModeGameMode created and compiles
- [ ] BP_PlayerShip created and compiles
- [ ] World Settings has correct GameMode

### Functional Testing
- [ ] Press Play - level loads quickly
- [ ] Spaceship spawns at correct location
- [ ] Camera follows spaceship
- [ ] Can control spaceship with input
- [ ] Spaceship moves forward/backward
- [ ] Spaceship turns left/right
- [ ] Camera orbits correctly
- [ ] No console errors or warnings

### Performance Testing
- [ ] Level loads in < 5 seconds
- [ ] Maintains 60+ FPS during play
- [ ] No hitching or stuttering
- [ ] Memory usage reasonable (< 1GB)

## Common Workflows

### Adding a New Test Ship

1. Create new Blueprint in `Content/Blueprints/Ships/`
2. Parent class: Spaceship
3. Configure visual mesh and components
4. Set as DefaultSpaceshipClass in BP_TestModeGameMode
5. Test in Test Mode

### Testing Ship Modifications

1. Open Test Mode level
2. Press Play to spawn ship
3. Test modifications
4. Stop play
5. Adjust Blueprint
6. Repeat until satisfied

### Debugging Spawn Issues

1. Check Output Log for errors
2. Verify SpaceshipSpawnPoint tag is correct
3. Verify GameMode is set in World Settings
4. Check that spaceship Blueprint compiles
5. Add debug Print String nodes to BP_TestModeGameMode

## Support and Troubleshooting

### Most Common Issues

1. **Spaceship doesn't spawn**
   - Solution in: [BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt#issue-spaceship-doesnt-spawn)

2. **Can't control spaceship**
   - Solution in: [BP_TestModeSetup_Guide.txt](../Content/Blueprints/BP_TestModeSetup_Guide.txt#issue-cant-control-spaceship)

3. **Floor not visible**
   - Solution in: [TestMode.umap.txt](../Content/Maps/TestMode.umap.txt#floor-not-visible)

4. **Dark level**
   - Solution in: [TestMode.umap.txt](../Content/Maps/TestMode.umap.txt#dark-level)

### Getting Help

If you encounter issues not covered in the documentation:
1. Check the Unreal Engine Output Log for errors
2. Review the troubleshooting sections in the documentation
3. Search the project's GitHub issues
4. Create a new GitHub issue with:
   - Steps to reproduce
   - Expected vs actual behavior
   - Relevant log output
   - Screenshots if applicable

## Version History

| Date | Version | Changes |
|------|---------|---------|
| 2025-11-05 | 1.0 | Initial Test Mode documentation release |

## Contributing

To improve Test Mode documentation:
1. Fork the repository
2. Make your changes
3. Test your changes in Unreal Editor
4. Submit a pull request
5. Update this index if adding new documentation

## License

Test Mode is part of the Adastrea project and follows the same MIT License. See [LICENSE](../LICENSE) for details.

---

**Document Maintainer**: Development Team  
**Last Updated**: 2025-11-05  
**Documentation Version**: 1.0  
**For**: Adastrea - Open World Space Flight Game
