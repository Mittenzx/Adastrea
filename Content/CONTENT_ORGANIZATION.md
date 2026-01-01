# Content Organization Guide

Best practices for organizing game content in the Unreal Engine Content Browser.

## Directory Structure

```
Content/
├── Blueprints/           # Blueprint classes
│   ├── AI/              # AI-related Blueprints
│   ├── Characters/      # Character Blueprints
│   ├── Factions/        # Faction Blueprints
│   ├── Ships/           # Ship Blueprints
│   ├── Stations/        # Station and module Blueprints
│   ├── Trading/         # Trading system Blueprints
│   └── UI/              # UI Blueprint widgets
│
├── DataAssets/          # Data Asset instances
│   ├── Factions/        # Faction configurations
│   ├── Homeworlds/      # Player starting locations
│   ├── Materials/       # Material definitions
│   ├── Personnel/       # Crew member profiles
│   ├── Ships/           # Ship configurations
│   └── Trading/         # Trade items, markets, contracts
│
├── Maps/                # Level maps
│   ├── Test/            # Test levels
│   ├── Gameplay/        # Main gameplay levels
│   └── Menus/           # Menu scenes
│
├── Materials/           # Materials and material instances
│   ├── Master/          # Master materials
│   ├── Instances/       # Material instances
│   └── Functions/       # Material functions
│
├── Meshes/              # 3D models
│   ├── Ships/           # Ship models
│   ├── Stations/        # Station models
│   ├── Props/           # Prop models
│   └── Environment/     # Environmental models
│
├── Textures/            # Texture assets
│   ├── Ships/           # Ship textures
│   ├── Stations/        # Station textures
│   ├── UI/              # UI textures and icons
│   └── Environment/     # Environmental textures
│
├── Audio/               # Sound assets
│   ├── Music/           # Background music
│   ├── SFX/             # Sound effects
│   └── Voice/           # Voice acting
│
├── VFX/                 # Visual effects
│   ├── Particles/       # Particle systems
│   ├── Niagara/         # Niagara systems
│   └── Materials/       # VFX materials
│
├── Animations/          # Animation assets
│   ├── Characters/      # Character animations
│   └── Sequences/       # Level sequences
│
└── UI/                  # UI-specific assets
    ├── Widgets/         # Widget Blueprints
    ├── Textures/        # UI textures
    └── Fonts/           # Font assets
```

## Naming Conventions

### Blueprint Classes

Format: `BP_[System]_[Type]_[Variant]`

Examples:
- `BP_Ship_Fighter_Light`
- `BP_Station_Military_Small`
- `BP_UI_MainMenu`
- `BP_Character_Player`

### Data Assets

Format: `DA_[Type]_[Name]`

Examples:
- `DA_Faction_SolarisUnion`
- `DA_Ship_PathfinderScout`
- `DA_Personnel_Captain_Smith`
- `DA_TradeItem_Iron`

### Widget Blueprints

Format: `WBP_[Location]_[Purpose]`

Examples:
- `WBP_HUD_Main`
- `WBP_Menu_Settings`
- `WBP_Dialog_Confirmation`
- `WBP_Station_Editor`

### Materials

Format: `M_[Surface]_[Variant]` (Master) or `MI_[Surface]_[Variant]` (Instance)

Examples:
- `M_Metal_Base` (Master Material)
- `MI_Metal_Blue` (Material Instance)
- `M_Ship_Hull_Master`
- `MI_Ship_Hull_Fighter`

### Meshes

Format: `SM_[Category]_[Object]_[Variant]` (Static) or `SK_[Category]_[Object]` (Skeletal)

Examples:
- `SM_Ship_Fighter_01`
- `SM_Station_Module_Docking`
- `SK_Character_Human`
- `SM_Prop_Crate_Large`

### Textures

Format: `T_[Object]_[MapType]_[Variant]`

Map Type Suffixes:
- `_D` - Diffuse/Base Color
- `_N` - Normal
- `_R` - Roughness
- `_M` - Metallic
- `_AO` - Ambient Occlusion
- `_E` - Emissive

Examples:
- `T_Ship_Hull_D_01`
- `T_Ship_Hull_N_01`
- `T_Metal_R_Scratched`
- `T_UI_Icon_Ship`

## Organization Best Practices

### 1. Keep Related Assets Together

Good:
```
Content/Ships/
├── Blueprints/
│   └── BP_Ship_Fighter
├── DataAssets/
│   └── DA_Ship_Fighter
├── Meshes/
│   └── SM_Ship_Fighter
└── Textures/
    ├── T_Ship_Fighter_D
    └── T_Ship_Fighter_N
```

### 2. Use Folders for Variants

```
Content/DataAssets/Factions/
├── Player/
│   ├── DA_Faction_PlayerAlliance
│   └── DA_Faction_PlayerTraders
├── Enemy/
│   ├── DA_Faction_Pirates
│   └── DA_Faction_Hostiles
└── Neutral/
    ├── DA_Faction_Merchants
    └── DA_Faction_Scientists
```

### 3. Separate Test Assets

```
Content/
├── Test/
│   ├── TestMaps/
│   ├── TestBlueprints/
│   └── DebugAssets/
└── [Regular Content]/
```

### 4. Use Prefixes for Quick Filtering

In Content Browser search:
- `BP_` - Show only Blueprints
- `DA_` - Show only Data Assets
- `WBP_` - Show only Widgets
- `M_` - Show only Materials

## Special Directories

### Developers/

Each developer can have their own folder for experimental work:
```
Content/Developers/
├── JohnDoe/
│   └── [Experimental assets]
└── JaneDoe/
    └── [Work in progress]
```

**Note**: These should not be committed unless intentional.

### Collections/

Use Collections (in Content Browser) for:
- Assets being worked on
- Assets for specific features
- Assets needing review
- Assets to be optimized

### Showcases/

```
Content/Showcases/
├── Ships/           # Ship showcase level
├── Stations/        # Station examples
└── Features/        # Feature demonstrations
```

## Migration Best Practices

When moving assets between projects:

1. **Select Root Asset**: Select the main asset to migrate
2. **Check Dependencies**: Review what will be migrated
3. **Organize First**: Ensure destination has proper structure
4. **Test After**: Verify all references are intact

## Content Browser Filters

### Custom Filters

Create filters for common searches:
- "Ships" → All ship-related assets
- "In Development" → Assets being worked on
- "Needs Optimization" → Assets to optimize
- "Ready for Release" → Approved assets

### Source Control

In Content Browser settings:
- Show source control status
- Use color coding for modified/checked-out files
- Check in frequently with clear messages

## Documentation in Content

### Asset Descriptions

Every asset should have:
1. **Purpose**: What is this asset for?
2. **Usage**: How should it be used?
3. **Dependencies**: What does it require?
4. **Notes**: Special considerations

Example (edit in Details panel):
```
Purpose: Main menu widget for game start screen
Usage: Spawned by BP_GameMode_MainMenu
Dependencies: WBP_Button_Standard, T_UI_Background_Main
Notes: Uses custom font "MainFont"
```

### README Files

Add README files in major folders:
- Right-click folder > Create Advanced Asset > Note
- Name it `_README` (underscore makes it appear first)
- Explain folder contents and organization

## Asset Duplication Warning

⚠️ **Avoid Duplicates**:
- Don't duplicate Data Assets (reference the original)
- Don't duplicate Blueprints unless creating variants
- Use Material Instances instead of duplicating Materials
- Use Child Blueprints for variants

## Cleanup Guidelines

### Automated Organization Tool

Adastrea includes an automated content organization tool that can help you clean up disorganized files:

```bash
# Preview what will be organized
python Tools/organize_content.py --dry-run --path Imported

# Organize files
python Tools/organize_content.py --path Imported
```

For detailed usage instructions, see [Tools/README_ORGANIZE_CONTENT.md](../Tools/README_ORGANIZE_CONTENT.md).

### Regular Maintenance

Monthly tasks:
- [ ] Remove unused assets (Fix Up Redirectors)
- [ ] Check for duplicate assets
- [ ] Update asset descriptions
- [ ] Fix broken references
- [ ] Optimize large textures/meshes
- [ ] Review Developer folders

### Before Commits

- [ ] Only include necessary assets
- [ ] Verify no temp/test files
- [ ] Check file sizes (avoid huge assets)
- [ ] Validate all references
- [ ] Test in clean project if possible

## Performance Considerations

### Texture Sizes

- UI Elements: 512x512 max (often 256x256)
- Ship Textures: 2048x2048 max
- Station Textures: 4096x4096 max
- Environment: Variable (optimize per use case)

### Mesh Complexity

- Props: < 1,000 triangles
- Ships (small): < 10,000 triangles
- Ships (large): < 50,000 triangles
- Stations: < 100,000 triangles per module

### Blueprint Optimization

- Avoid complex Tick events
- Use Event Dispatchers over Tick checks
- Cache component references
- Use object pooling for frequent spawns

## Integration with Documentation

### Link Assets to Docs

In asset descriptions, reference:
- System Guide: `See Assets/SpaceshipSystemGuide.md`
- Templates: `Based on Assets/SpaceshipTemplates/PathfinderScout.yaml`
- Workflow: `Follow Assets/SpaceshipDesignerWorkflow.md`

### Document New Asset Types

When creating new asset categories:
1. Add to this guide
2. Create templates in Assets/
3. Update ARCHITECTURE.md
4. Add to README.md

## Collaboration Tips

### Team Workflow

1. **Claim Assets**: Use checkout system
2. **Regular Sync**: Pull before starting work
3. **Small Commits**: Commit working changes frequently
4. **Clear Messages**: Describe what changed
5. **Review Changes**: Check before committing

### Conflict Resolution

If conflicts occur:
- Coordinate with team member
- Decide whose version to keep
- Document the decision
- Prevent future conflicts with better communication

## Tools and Utilities

### Content Browser Tools

- **Filter**: Quick asset filtering
- **Collections**: Organize asset sets
- **Search**: Powerful search syntax
- **Source Control**: Git integration
- **Migration**: Move assets between projects

### Reference Viewer

- Right-click asset > Reference Viewer
- Shows dependencies and references
- Useful for cleanup and understanding connections

### Size Map

- Tools > Audit > Size Map
- Visualize asset sizes
- Identify optimization opportunities

## Quick Reference

### Common Paths
- Blueprints: `/Game/Blueprints/`
- Data Assets: `/Game/DataAssets/`
- Materials: `/Game/Materials/`
- Content root: `/Game/`

### Common Searches
- `Type=Blueprint` - All Blueprints
- `Type=Material` - All Materials
- `Size>10MB` - Large assets
- `Modified>Today` - Today's changes

## Recent Organization Updates

### UI File Structure Cleanup (2026-01-01)

**Status**: ✅ Completed

The UI file structure has been cleaned up to follow these standards:
- **All UI Widgets**: Located in `Content/UI/[System]/`
- **Naming Convention**: `WBP_[System][Purpose]` format only
- **Removed**: Legacy `BP_UI_*` files from `Content/Blueprints/UI/`
- **Removed**: 11 duplicate widget files with inconsistent naming

For complete details, see:
- [UI_FILE_STRUCTURE_FINAL.md](../docs/development/UI_FILE_STRUCTURE_FINAL.md) - What was changed
- [UI_FILE_STRUCTURE_CLEANUP.md](../docs/development/UI_FILE_STRUCTURE_CLEANUP.md) - Original analysis

**Key Takeaway**: Always create UI widgets in `Content/UI/[System]/` with `WBP_*` prefix.

---

**Keep your Content organized and your team will thank you!** 🎨✨
