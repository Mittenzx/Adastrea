# Content Organization Tool

A Python script to automatically organize Unreal Engine content files into proper folder structures according to Adastrea's naming conventions and organization guidelines.

## Features

- ✅ **Automatic Type Detection**: Identifies Materials, Meshes, Textures, Blueprints, and more
- ✅ **Smart Categorization**: Creates appropriate subfolders (e.g., Props, Splines, Imported)
- ✅ **Dry Run Mode**: Preview changes before applying them
- ✅ **Detailed Reports**: Generate comprehensive reports of all changes
- ✅ **Safe Operations**: Protects existing organized folders from reorganization
- ✅ **Handles Fab Imports**: Properly categorizes third-party assets
- ✅ **Unrecognized File Handling**: Option to move unknown files to Uncategorized folder

## Quick Start

### Preview Changes (Dry Run)

```bash
# Preview organization of Imported folder
python Tools/organize_content.py --dry-run --path Imported

# Preview organization of entire Content directory
python Tools/organize_content.py --dry-run
```

### Organize Files

```bash
# Organize Imported folder
python Tools/organize_content.py --path Imported

# Organize entire Content directory
python Tools/organize_content.py

# Organize including unrecognized files
python Tools/organize_content.py --path Imported --organize-unrecognized
```

### Generate Report Only

```bash
# Just generate a report without moving files
python Tools/organize_content.py --report-only --path Imported
```

## Usage Examples

### Example 1: Clean Up Fab Imports

When you import assets from Fab (Unreal Marketplace), they often end up mixed together:

```bash
# Preview what will be organized
python Tools/organize_content.py --dry-run --path Imported

# If the preview looks good, organize them
python Tools/organize_content.py --path Imported
```

### Example 2: Organize Entire Content Directory

```bash
# Always start with dry run
python Tools/organize_content.py --dry-run

# Review the report, then execute
python Tools/organize_content.py
```

### Example 3: Handle Unrecognized Files

Some files may not be automatically categorized:

```bash
# See what files are unrecognized
python Tools/organize_content.py --dry-run --path Imported

# Move them to Uncategorized folder for manual review
python Tools/organize_content.py --path Imported --organize-unrecognized
```

## Asset Type Detection

The script automatically detects asset types based on naming patterns:

### Materials
- Prefixes: `M_`, `MI_`, `Material`, `default`
- Example: `M_Metal_Base.uasset`, `Material_042.uasset`, `default_fire.uasset`
- Target: `Content/Materials/`

### Static Meshes
- Prefixes: `SM_`, `Cube`, `Cylinder`, `Sphere`, `Plane`, `Icosphere`, `mesh`, `BezierCurve`
- Example: `SM_Ship_Fighter.uasset`, `Cube_007.uasset`, `BezierCurve_001.uasset`
- Target: `Content/Meshes/Props/` or `Content/Meshes/Splines/`

### Textures
- Prefixes: `T_`, `TEX_`, `Texture`
- Suffixes: `_D`, `_N`, `_R`, `_M`, `_AO`, `_E`
- Example: `T_Ship_Hull_D.uasset`, `TEX_av2.uasset`
- Target: `Content/Textures/`

### Blueprints
- Prefixes: `BP_`, `WBP_`
- Example: `BP_Ship_Fighter.uasset`, `WBP_MainMenu.uasset`
- Target: `Content/Blueprints/`

### Data Assets
- Prefixes: `DA_`
- Example: `DA_Ship_Pathfinder.uasset`
- Target: `Content/DataAssets/`

### Particles/VFX
- Prefixes: `P_`, `PS_`, `NS_`
- Example: `NS_Engine_Thrust.uasset`
- Target: `Content/VFX/`

## Folder Structure Created

```
Content/
├── Materials/              # All materials (M_, MI_, Material)
│   ├── Master/            # (future: master materials)
│   └── Instances/         # (future: material instances)
│
├── Meshes/                # All static meshes
│   ├── Props/             # Basic shapes (Cubes, Cylinders, Spheres)
│   ├── Splines/           # BezierCurves and splines
│   └── Imported/          # Generic imported meshes
│
├── Textures/              # All textures
│   ├── Ships/             # (future: ship textures)
│   ├── Stations/          # (future: station textures)
│   └── UI/                # (future: UI textures)
│
├── Blueprints/            # Blueprint classes (protected)
├── DataAssets/            # Data Assets (protected)
├── UI/                    # UI widgets (protected)
├── VFX/                   # Visual effects (protected)
└── Uncategorized/         # Unrecognized files (optional)
```

## Protected Folders

The following folders are **protected** and will not be reorganized:
- `Blueprints/`
- `DataAssets/`
- `UI/`
- `VFX/`
- `Input/`
- `Maps/`
- `Python/`
- `Docs/`

Files in these folders are assumed to be already properly organized.

## Command Line Options

```
usage: organize_content.py [-h] [--content-dir CONTENT_DIR] [--path PATH]
                          [--dry-run] [--report-only]
                          [--organize-unrecognized]

options:
  -h, --help            Show this help message and exit
  
  --content-dir CONTENT_DIR
                        Path to Content directory (default: Content)
  
  --path PATH          Specific subfolder to organize (relative to Content)
                        Example: --path Imported
  
  --dry-run            Preview changes without actually moving files
                        Always use this first!
  
  --report-only        Generate report only, do not move files
                        Similar to --dry-run
  
  --organize-unrecognized
                        Move unrecognized files to Uncategorized/ folder
                        By default, unrecognized files are left in place
```

## Report Format

The script generates a detailed report saved to `content_organization_report.txt`:

```
================================================================================
CONTENT ORGANIZATION REPORT
================================================================================

STATISTICS:
--------------------------------------------------------------------------------
  Material             :   60 files
  StaticMesh           :  159 files
  Texture              :    6 files
  Unrecognized         :    1 files
  Total                :  227 files

PLANNED MOVES: 226
--------------------------------------------------------------------------------

  → Materials/ (60 files)
      • Material_042.uasset
      • default_fire.uasset
      ... and 58 more

  → Meshes/Props/ (123 files)
      • Cube_007.uasset
      • Cylinder_079.uasset
      ... and 121 more

UNRECOGNIZED FILES: 1
--------------------------------------------------------------------------------
These files could not be automatically categorized:
  ? unknown_asset.uasset

Tip: Use --organize-unrecognized to move these to Uncategorized/ folder
```

## Best Practices

### 1. Always Dry Run First

```bash
# WRONG: Don't organize without previewing
python Tools/organize_content.py

# RIGHT: Always preview first
python Tools/organize_content.py --dry-run
# Review the report, then:
python Tools/organize_content.py
```

### 2. Start with Specific Folders

```bash
# Organize one folder at a time
python Tools/organize_content.py --dry-run --path Imported
python Tools/organize_content.py --path Imported

python Tools/organize_content.py --dry-run --path SomeOtherFolder
python Tools/organize_content.py --path SomeOtherFolder
```

### 3. Review Unrecognized Files

Unrecognized files should be reviewed manually:

```bash
# Check what's unrecognized
python Tools/organize_content.py --dry-run --path Imported

# Option 1: Leave them in place for manual review
# (default behavior)

# Option 2: Move to Uncategorized for batch processing
python Tools/organize_content.py --path Imported --organize-unrecognized
```

### 4. Check Unreal Editor After Organization

After organizing files:
1. Open Unreal Editor
2. Right-click in Content Browser
3. Select "Fix Up Redirectors"
4. Save all assets
5. Test that everything still works

## Troubleshooting

### Files Not Being Moved

**Problem**: Script runs but files aren't moved

**Solution**: Check if you're using `--dry-run` or `--report-only` mode. Remove these flags to actually move files.

### "Target Already Exists" Error

**Problem**: Script reports "Target already exists" errors

**Solution**: Files with the same name already exist at the destination. You'll need to manually rename or merge them.

### Unrecognized Files

**Problem**: Many files are marked as "Unrecognized"

**Solution**: 
1. Review the file names in the report
2. If they follow a pattern, you can add detection rules in the script
3. Or manually categorize them in Unreal Editor
4. Or use `--organize-unrecognized` to move them to Uncategorized folder

### Script Can't Find Content Directory

**Problem**: "ERROR: Content directory not found"

**Solution**: Run the script from the project root directory, or use `--content-dir` to specify the path.

## Integration with Unreal Editor

### Before Running the Script

1. **Backup your project** (commit to Git or create a backup)
2. **Close Unreal Editor** (to avoid conflicts)
3. Run the script

### After Running the Script

1. **Open Unreal Editor**
2. **Fix Up Redirectors**: Right-click in Content Browser → "Fix Up Redirectors"
3. **Save All**: File → Save All
4. **Test**: Make sure everything works correctly
5. **Commit**: Commit the organized files to Git

## Advanced Usage

### Custom Organization Rules

Edit the script's `ASSET_PATTERNS` dictionary to add custom rules:

```python
ASSET_PATTERNS = {
    'CustomType': {
        'prefixes': ['Custom_', 'CT_'],
        'folder': 'CustomFolder'
    },
    # ... other patterns
}
```

### Subcategorization

Edit the `MESH_CATEGORIES` dictionary for custom mesh subcategories:

```python
MESH_CATEGORIES = {
    'Cube': 'Props',
    'Ship': 'Ships',
    'Station': 'Stations',
    # ... other categories
}
```

## Safety Features

The script includes several safety features:

1. **Dry Run Mode**: Always preview changes before applying
2. **Protected Folders**: Won't reorganize already-organized folders
3. **Duplicate Detection**: Won't overwrite existing files
4. **Error Reporting**: Logs all errors for review
5. **Detailed Reports**: Complete record of all changes

## Related Documentation

- [Content Organization Guide](../Content/CONTENT_ORGANIZATION.md) - Full organization guidelines
- [Naming Conventions](../CODE_STYLE.md) - Asset naming standards
- [Content Browser](https://docs.unrealengine.com/5.6/en-US/content-browser-in-unreal-engine/) - Official UE documentation

## Support

If you encounter issues:
1. Check the generated report in `content_organization_report.txt`
2. Review the console output for errors
3. Check this README for troubleshooting steps
4. Consult the [Content Organization Guide](../Content/CONTENT_ORGANIZATION.md)

---

**Always use `--dry-run` first to preview changes!** 🔍✨
