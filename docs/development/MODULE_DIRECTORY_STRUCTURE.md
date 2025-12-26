# Module Directory Structure Guide

## Overview

This document explains the directory structure used in the Adastrea project and how it follows Unreal Engine 5 best practices.

## Standard UE5 Module Structure

Unreal Engine 5 modules follow this structure:

```
Source/ModuleName/
├── ModuleName.Build.cs
├── Public/
│   └── *.h (public headers)
└── Private/
    └── *.cpp (implementation files)
    └── *.h (private headers)
```

In this standard structure:
- UE5 automatically includes `ModuleName/Public/` in the include path
- UE5 automatically includes `ModuleName/Private/` in the include path
- No explicit `PublicIncludePaths` or `PrivateIncludePaths` needed
- Better IDE support and navigation
- Clearer separation of public APIs and private implementation

## Adastrea's Current Structure (✅ Standard)

**As of December 2025**, the Adastrea project now follows the standard UE5 structure:

```
Source/Adastrea/
├── Adastrea.Build.cs
├── Public/
│   ├── Adastrea.h
│   ├── AdastreaGameMode.h
│   ├── SpaceSectorMap.h
│   ├── AI/
│   │   ├── FactionLogic.h
│   │   ├── NPCLogicBase.h
│   │   └── PersonnelLogic.h
│   ├── Audio/
│   ├── Characters/
│   ├── Combat/
│   ├── Exploration/
│   ├── Factions/
│   ├── Input/
│   ├── Interfaces/
│   ├── Materials/
│   ├── Navigation/
│   ├── Performance/
│   ├── Planets/
│   ├── Player/
│   ├── Procedural/
│   ├── Quest/
│   ├── Rivals/
│   ├── Ships/
│   ├── Stations/
│   ├── Trading/
│   ├── Tutorial/
│   ├── UI/
│   └── Way/
└── Private/
    ├── Adastrea.cpp
    ├── AdastreaGameMode.cpp
    ├── AdastreaLog.cpp
    ├── SpaceSectorMap.cpp
    ├── AI/
    │   ├── FactionLogic.cpp
    │   ├── NPCLogicBase.cpp
    │   └── PersonnelLogic.cpp
    ├── Audio/
    ├── Characters/
    └── ... (mirrors Public/ structure)
```

### Benefits of Standard Structure

✅ **No explicit include paths needed** - Build.cs files are simpler  
✅ **Fixes SetEnv error** - No environment variable length issues  
✅ **Better IDE support** - IntelliSense and autocomplete work better  
✅ **Clear organization** - Public APIs separated from private implementation  
✅ **Future-proof** - Won't encounter similar issues as project grows

## Historical Context: The SetEnv Error Issue

### Original Problem (Oct-Dec 2025)

The project previously had a non-standard structure with subdirectories at the module root level (AI/, Combat/, Ships/, etc. outside of Public/Private/). This required 21 explicit `PublicIncludePaths` declarations, which caused the MSBuild SetEnv task to fail:

```
System.ArgumentException: Environment variable name or value is too long
```

This happened because:
1. Include paths were being added to environment variables
2. Windows has a ~32KB limit on environment variable length
3. Long paths multiplied across many directories exceeded this limit

### Resolution (December 2025)

**Complete restructuring** to standard UE5 layout:
- Moved all 140 header files to `Public/` subdirectories
- Moved all 128 implementation files to `Private/` subdirectories
- Removed all 21 explicit `PublicIncludePaths` declarations
- Updated StationEditor module similarly

**Result**: Build system works correctly with no environment variable issues

## Affected Modules

### Adastrea Module ✅

**Status**: Now follows standard structure  
**Structure**: Public/ and Private/ subdirectories with 23 system folders each  
**Include Paths**: None needed (automatic)

### StationEditor Module ✅

**Status**: Now follows standard structure  
**Structure**: Public/ and Private/ subdirectories  
**Include Paths**: None needed (automatic)

### PlayerMods Module ✅

**Status**: Uses standard structure  
**Include Paths**: None needed (automatic)

### UnrealMCP Plugin ✅

**Status**: Uses standard structure  
**Include Paths**: None needed (automatic)

## Best Practice for All Modules

### Required Structure

Always use the standard UE5 structure:
```
Source/NewModule/
├── NewModule.Build.cs
├── Public/
│   └── [organize headers here]
└── Private/
    └── [organize source files here]
```

### Benefits

- ✅ No explicit include paths needed
- ✅ Simpler Build.cs files
- ✅ No environment variable issues
- ✅ Better IDE integration
- ✅ Industry standard approach

### Organizing Large Modules

For modules with many files, organize by system within Public/ and Private/:

```
Public/
├── Core/           # Core functionality
├── Audio/          # Audio system
├── Combat/         # Combat system
├── UI/             # User interface
└── ...

Private/
├── Core/
├── Audio/
├── Combat/
├── UI/
└── ...
```

Each system's headers go in `Public/[System]/` and implementations in `Private/[System]/`.

## Include Statements

### Within Same Module

```cpp
// Include from Public/Combat/WeaponComponent.h
#include "Combat/WeaponComponent.h"

// Include from Public/Ships/Spaceship.h
#include "Ships/Spaceship.h"
```

### From Other Modules

```cpp
// Include from Adastrea module (in StationEditor)
#include "Stations/SpaceStationModule.h"
#include "Factions/FactionDataAsset.h"
```

### Best Practices

1. **Use forward declarations in headers** when possible
2. **Include full headers in .cpp files** only
3. **Use relative paths** from Public/ or Private/ directories
4. **Keep include paths short** and readable

## Related Documentation

- [Build Troubleshooting Guide](./BUILD_TROUBLESHOOTING.md) - Build issues and solutions
- [Anti-Patterns Guide](../../.github/instructions/anti-patterns.instructions.md) - See #13 for file organization
- [CHANGELOG.md](../../CHANGELOG.md) - Complete change history
- [Unreal Engine Module Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)

## Summary

| Module | Structure | Include Paths | Status |
|--------|-----------|---------------|--------|
| Adastrea | ✅ Standard (Public/Private) | ❌ None needed | ✅ Fixed |
| StationEditor | ✅ Standard (Public/Private) | ❌ None needed | ✅ Fixed |
| PlayerMods | ✅ Standard (Public/Private) | ❌ None needed | ✅ Compliant |
| UnrealMCP | ✅ Standard (Public/Private) | ❌ None needed | ✅ Compliant |

**All modules now follow UE5 best practices with no explicit include paths required.**

---

**Last Updated**: 2025-12-26  
**Related Issues**: SetEnv environment variable length error (resolved)  
**Related PRs**: Module restructuring to standard UE5 layout
