# Content Generation Tools - Complete Guide

## Overview

Adastrea includes multiple content generation tools for creating game assets at different scales and complexity levels. This guide explains when to use each tool.

## Quick Reference

| Tool | Purpose | Best For | Complexity |
|------|---------|----------|------------|
| `ContentGenerator.py` | Basic asset variants | Quick single-asset variations | Low |
| `QuickContentGenerator.py` | Fast batch creation | Creating 10-50 assets quickly | Low |
| `AdvancedContentGenerator.py` | Comprehensive assets | Full featured assets with all stats | Medium |
| `MassContentGenerator.py` | Bulk generation + scenes | Creating 100+ assets + populating levels | High |
| `UltimateContentGenerator.py` | Complete game world | Running all generators + validation | Very High |

## Content Generators Explained

### 1. ContentGenerator.py
**Location**: `/ContentGenerator.py`  
**Type**: Blueprint Function Library

**Purpose**: Basic content generation accessible from Blueprints

**Use When:**
- Creating simple asset variants with randomized stats
- Working primarily in Blueprints
- Need quick variations of existing assets
- Testing asset configurations

**Example:**
```python
ContentGenerator.generate_spaceship_variants(base_ship_asset, num_variants=10)
```

**Features:**
- Blueprint-callable functions
- Simple randomization
- Variant generation
- Quick iteration

---

### 2. QuickContentGenerator.py
**Location**: `/QuickContentGenerator.py`

**Purpose**: Fast batch creation of basic game content

**Use When:**
- Need to quickly create 10-50 assets
- Prototyping game content
- Creating placeholder assets
- Testing systems with sample data

**What It Creates:**
- Spaceship Data Assets (basic stats)
- Weapon Data Assets
- Personnel Data Assets
- Quest Data Assets

**Typical Output**: 10-50 assets in under a minute

---

### 3. AdvancedContentGenerator.py
**Location**: `/AdvancedContentGenerator.py`

**Purpose**: Creates comprehensive, production-quality game content with full feature sets

**Use When:**
- Creating production-ready assets
- Need assets with all stats properly configured
- Want variety and realism in generated content
- Creating content for actual gameplay

**What It Creates:**
- Spaceships (50+ with varied templates)
- Full stat configurations
- Balanced gameplay values
- Faction assignments
- Realistic naming

**Features:**
- Template-based generation
- Balanced stats
- Faction integration
- Professional quality output

**Typical Output**: 50+ fully-featured assets

---

### 4. MassContentGenerator.py
**Location**: `/MassContentGenerator.py`

**Purpose**: Large-scale content generation with level population

**Use When:**
- Creating 100+ game assets
- Populating entire sectors/levels
- Building complete game worlds
- Performance testing with realistic data

**What It Creates:**
- Hundreds of Data Assets
- Populated game levels
- Complete sectors with stations and ships
- NPCs and encounters

**Features:**
- Integrates with ScenePopulator
- Level population
- Placement patterns
- Large-scale generation

**Typical Output**: 100-500+ assets + populated levels

**Note**: This is the most resource-intensive generator. Use for final content creation or testing at scale.

---

### 5. UltimateContentGenerator.py + Launcher
**Location**: `/UltimateContentGenerator.py`, `/UltimateContentGeneratorLauncher.py`

**Purpose**: Orchestrates ALL content generation systems to create a complete game world

**Use When:**
- Building the complete game world from scratch
- Running comprehensive content generation pipeline
- Need validated, tested, and benchmarked content
- Creating demo or release builds

**What It Does:**
- Runs all generators in sequence
- Populates multiple sectors
- Validates all created assets
- Runs performance benchmarks
- Generates test reports

**Features:**
- Automated testing integration
- Data validation
- Performance profiling
- Complete pipeline execution

**Typical Output**: Complete game world with validation reports

**Warning**: This runs ALL generators and can take significant time (10+ minutes). Use for final builds or comprehensive testing.

---

## Content Generator Tools in /Content/Python/

These are in-engine Python scripts that run within Unreal Editor:

### MasterContentGenerator.py
**Location**: `/Content/Python/MasterContentGenerator.py`

**Purpose**: In-engine content generation with full Unreal Editor integration

**Use When:**
- Running generation from within Unreal Editor
- Need editor-specific functionality
- Creating content that requires editor context

### MasterContentGeneratorLauncher.py
**Location**: `/Content/Python/MasterContentGeneratorLauncher.py`

**Purpose**: Launches MasterContentGenerator with UI and progress tracking

---

## Decision Flow Chart

```
Need content? 
  │
  ├─> Just testing one feature? 
  │   └─> Use ContentGenerator.py (Blueprint callable)
  │
  ├─> Quick prototyping (10-50 assets)?
  │   └─> Use QuickContentGenerator.py
  │
  ├─> Production quality assets?
  │   └─> Use AdvancedContentGenerator.py
  │
  ├─> Populate entire sectors (100+)?
  │   └─> Use MassContentGenerator.py
  │
  └─> Build complete game world?
      └─> Use UltimateContentGenerator.py
```

## Usage Examples

### Example 1: Quick Prototyping
```bash
# Create 20 test spaceships quickly
python QuickContentGenerator.py
```

### Example 2: Production Content
```bash
# Create 50 production-quality spaceships with full stats
python AdvancedContentGenerator.py
```

### Example 3: Populate Game World
```bash
# Create hundreds of assets and populate levels
python MassContentGenerator.py
```

### Example 4: Complete Game Build
```bash
# Run all generators and create validated game world
python UltimateContentGeneratorLauncher.py
```

## Integration with Other Tools

### ScenePopulator
Used by: `MassContentGenerator.py`, `UltimateContentGenerator.py`

Places generated assets in levels with various patterns:
- Grid placement
- Random scatter
- Orbital patterns
- Cluster formations

### Validation Systems
Used by: `UltimateContentGenerator.py`

- `DataValidationLibrary` - Validates created assets
- `AutomatedTestLibrary` - Runs automated tests
- `PerformanceBenchmarkLibrary` - Profiles performance

## File Organization

```
/                                   # Project root
├── ContentGenerator.py             # Basic Blueprint-callable generator
├── QuickContentGenerator.py        # Fast batch creation
├── AdvancedContentGenerator.py     # Production-quality generator
├── MassContentGenerator.py         # Large-scale generation
├── UltimateContentGenerator.py     # Complete world generation
├── UltimateContentGeneratorLauncher.py  # Launcher with UI
│
└── Content/
    └── Python/                     # In-engine scripts
        ├── MasterContentGenerator.py
        └── MasterContentGeneratorLauncher.py
```

## Performance Considerations

| Generator | Asset Count | Time | Memory | Recommended Use |
|-----------|-------------|------|--------|-----------------|
| ContentGenerator | 1-10 | <1 min | Low | Always safe |
| QuickContentGenerator | 10-50 | 1-2 min | Low | Safe for frequent use |
| AdvancedContentGenerator | 50-100 | 2-5 min | Medium | Use as needed |
| MassContentGenerator | 100-500 | 5-10 min | High | Use for builds |
| UltimateContentGenerator | 500+ | 10-30 min | Very High | Use for final builds only |

## Troubleshooting

### Generator Won't Run
1. Make sure you're in the Unreal Editor Python console
2. Check that required modules are imported
3. Verify asset paths exist

### Out of Memory
- Use a smaller-scale generator
- Close other applications
- Generate in batches

### Assets Not Appearing
- Check the output paths in the generator
- Verify assets were created: Check `/Game/Adastrea/DataAssets/`
- Refresh the Content Browser

### Slow Performance
- Use appropriate generator for your needs
- Close unnecessary editor tabs
- Consider running overnight for large batches

## Best Practices

1. **Start Small**: Begin with QuickContentGenerator for testing
2. **Iterate**: Use AdvancedContentGenerator for production
3. **Scale Up**: Use MassContentGenerator only when needed
4. **Validate**: Always run UltimateContentGenerator before releases
5. **Clean Up**: Delete test assets before generating production content
6. **Backup**: Save your project before running large generators

## Migration Guide

If you were using old generators:

| Old Tool | New Recommendation |
|----------|-------------------|
| Generic scripts | Use QuickContentGenerator.py |
| Custom generators | Migrate to AdvancedContentGenerator.py |
| Bulk generation | Use MassContentGenerator.py |
| One-off tests | Use ContentGenerator.py |

## Support and Documentation

- **Code Examples**: See each generator's docstrings
- **Integration**: Check `COMPLETE_REVIEW_SUMMARY.md`
- **Architecture**: See `CONTENT_GENERATION_GUIDE.md` (if available)
- **Issues**: Open GitHub issue with `[Content Generation]` tag

---

**Last Updated**: 2025-12-20  
**Maintained By**: Adastrea Development Team  
**Status**: Active - Ready for use
