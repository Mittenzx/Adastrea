# Quick Start: Organizing Content Files

This guide shows you how to quickly organize disorganized content files in your Adastrea project.

## Problem

You have files scattered in wrong folders, like materials mixed with textures and meshes in the `Imported` folder.

## Solution

Use the automated content organization tool.

## Steps

### 1. Preview What Will Change (Dry Run)

Always start by previewing what the script will do:

```bash
cd /path/to/Adastrea
python Tools/organize_content.py --dry-run --path Imported
```

This will show you:
- How many files will be moved
- Where they will be moved to
- Which files can't be automatically categorized

Example output:
```
STATISTICS:
  Material             :   60 files  → Content/Materials/
  StaticMesh           :  159 files  → Content/Meshes/Props/ or Meshes/Splines/
  Texture              :    6 files  → Content/Textures/
  Unrecognized         :    1 files  → (stays in place)
  Total                :  227 files
```

### 2. Review the Report

Check the generated report at `content_organization_report.txt` for details.

Key things to look for:
- ✅ Are files being moved to the right places?
- ✅ Are there any errors?
- ⚠️ Which files are unrecognized?

### 3. Organize the Files

If the preview looks good, run without `--dry-run`:

```bash
python Tools/organize_content.py --path Imported
```

The script will:
- Create necessary folders (Materials/, Meshes/, Textures/)
- Move files to appropriate locations
- Print progress for each file moved
- Generate a final report

### 4. Clean Up in Unreal Editor

After organizing files:

1. **Open Unreal Editor**
2. **Fix Redirectors**: Right-click in Content Browser → "Fix Up Redirectors"
3. **Save All**: File → Save All
4. **Verify**: Check that everything still works

### 5. Handle Unrecognized Files (Optional)

If some files couldn't be categorized:

```bash
# Move unrecognized files to Uncategorized folder
python Tools/organize_content.py --path Imported --organize-unrecognized
```

Then manually review them in `Content/Uncategorized/` and move them to the correct location.

## Command Summary

```bash
# Step 1: Preview (always do this first!)
python Tools/organize_content.py --dry-run --path Imported

# Step 2: Execute
python Tools/organize_content.py --path Imported

# Optional: Handle unrecognized files
python Tools/organize_content.py --path Imported --organize-unrecognized
```

## Full Project Organization

To organize the entire Content folder:

```bash
# Preview entire Content directory
python Tools/organize_content.py --dry-run

# Organize entire Content directory
python Tools/organize_content.py
```

⚠️ **Warning**: This will scan all subfolders. Protected folders (Blueprints, DataAssets, UI, VFX, etc.) are automatically skipped.

## What Gets Organized?

### Detected Automatically

| Asset Type | Naming Pattern | Destination |
|------------|---------------|-------------|
| Materials | `M_*`, `MI_*`, `Material*`, `default*` | `Materials/` |
| Static Meshes | `SM_*`, `Cube*`, `Cylinder*`, `Sphere*` | `Meshes/Props/` |
| Splines | `BezierCurve*` | `Meshes/Splines/` |
| Textures | `T_*`, `TEX_*`, `*_D`, `*_N`, `*_R` | `Textures/` |
| Blueprints | `BP_*`, `WBP_*` | `Blueprints/` |
| Data Assets | `DA_*` | `DataAssets/` |
| VFX | `NS_*`, `PS_*`, `P_*` | `VFX/` |

### Protected (Not Touched)

These folders are already organized and won't be scanned:
- `Blueprints/`
- `DataAssets/`
- `UI/`
- `VFX/`
- `Input/`
- `Maps/`

## Common Scenarios

### Scenario 1: Fab/Marketplace Import

You imported assets from Fab and they're all in one folder:

```bash
python Tools/organize_content.py --dry-run --path Imported
# Review output
python Tools/organize_content.py --path Imported
```

### Scenario 2: Multiple Folders to Organize

Organize one folder at a time:

```bash
python Tools/organize_content.py --path FolderA
python Tools/organize_content.py --path FolderB
python Tools/organize_content.py --path FolderC
```

### Scenario 3: Fresh Project Cleanup

Organize everything:

```bash
python Tools/organize_content.py --dry-run
# Review carefully!
python Tools/organize_content.py
```

## Troubleshooting

### "Target already exists"
Files with same name already exist at destination. Manually rename or merge them.

### Many unrecognized files
Either:
- Add detection rules to the script for your naming patterns
- Use `--organize-unrecognized` to move them to Uncategorized
- Manually organize them in Unreal Editor

### Script doesn't find Content folder
Run from project root directory, or use `--content-dir` to specify path.

## Need More Help?

See the full documentation:
- [Tools/README_ORGANIZE_CONTENT.md](README_ORGANIZE_CONTENT.md) - Complete documentation
- [Content/CONTENT_ORGANIZATION.md](../Content/CONTENT_ORGANIZATION.md) - Organization guidelines

---

**Remember: Always use `--dry-run` first!** 🔍
