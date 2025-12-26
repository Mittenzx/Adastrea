# Module Directory Structure Guide

## Overview

This document explains the non-standard directory structure used in the Adastrea project and how it affects the build configuration.

## Standard UE5 Module Structure

A typical Unreal Engine 5 module follows this structure:

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

## Adastrea's Non-Standard Structure

The Adastrea project uses a different structure:

```
Source/Adastrea/
├── Adastrea.Build.cs
├── Public/
│   ├── AI/
│   ├── Combat/
│   └── ... (headers organized by system)
├── Private/
│   └── AdastreaFunctionLibrary.cpp
├── AI/                    # ⚠️ Non-standard location
│   ├── FactionLogic.cpp
│   ├── NPCLogicBase.cpp
│   └── PersonnelLogic.cpp
├── Combat/                # ⚠️ Non-standard location
│   ├── WeaponComponent.cpp
│   └── ...
├── Ships/                 # ⚠️ Non-standard location
├── Stations/              # ⚠️ Non-standard location
└── ... (19 more subdirectories at root level)
```

### Why This is Problematic

When subdirectories exist at the module root level (not inside Public/ or Private/):
1. UE5 does NOT automatically include them in the include path
2. The compiler cannot find headers or source files in these directories
3. Build failures occur with "file not found" errors
4. This requires explicit `PublicIncludePaths` declarations

## The SetEnv Error Issue

### Original Problem (Dec 2025)

The project had explicit include paths declared, which caused the MSBuild SetEnv task to fail:
```
System.ArgumentException: Environment variable name or value is too long
```

This happened because:
1. Include paths were being added to environment variables
2. Windows has a ~32KB limit on environment variable length
3. Long absolute paths multiplied across many directories exceeded this limit

### First Fix Attempt (PR #357)

The first fix removed ALL `PublicIncludePaths` declarations:
```csharp
// Removed this:
PublicIncludePaths.AddRange(new string[] {
    "Source/Adastrea/AI",
    "Source/Adastrea/Combat",
    // ... etc (long absolute paths)
});
```

**Problem**: This fixed the SetEnv error but broke the build because:
- The non-standard subdirectories were no longer in the include path
- Compiler couldn't find headers in AI/, Combat/, Ships/, etc.

### Complete Fix (This PR)

The complete fix adds back the include paths but uses **relative paths** via `ModuleDirectory`:

```csharp
PublicIncludePaths.AddRange(new string[] {
    System.IO.Path.Combine(ModuleDirectory, "AI"),
    System.IO.Path.Combine(ModuleDirectory, "Combat"),
    System.IO.Path.Combine(ModuleDirectory, "Ships"),
    // ... etc (21 total)
});
```

**Why this works**:
1. `ModuleDirectory` is a relative path, not absolute
2. Relative paths are much shorter than absolute paths
3. Stays well under the environment variable length limit
4. Compiler can find headers in non-standard directories
5. Build system works correctly

## Affected Modules

### Adastrea Module

Has 21 non-standard subdirectories:
- AI
- Audio
- Characters
- Combat
- Exploration
- Factions
- Input
- Materials
- Navigation
- Performance
- Planets
- Player
- Procedural
- Quest
- Rivals
- Ships
- Stations
- Trading
- Tutorial
- UI
- Way

### StationEditor Module

Has 1 non-standard subdirectory:
- UI

### PlayerMods Module

✅ Uses standard structure (no explicit include paths needed)

### UnrealMCP Plugin

✅ Uses standard structure (no explicit include paths needed)

## Best Practice Going Forward

### For New Modules

Use the standard UE5 structure:
```
Source/NewModule/
├── NewModule.Build.cs
├── Public/
│   └── [organize headers here]
└── Private/
    └── [organize source files here]
```

No explicit include paths needed!

### For Existing Non-Standard Modules

If you must add new subdirectories at the root level:

1. Add the directory to the module
2. Update the `.Build.cs` file:
```csharp
PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "NewDirectory"));
```
3. Always use `ModuleDirectory` for relative paths
4. Never use absolute paths

## Testing Include Paths

To verify include paths are working:

1. Generate project files (right-click .uproject → Generate Visual Studio project files)
2. Open in IDE and check for errors
3. Build the project
4. No "file not found" errors = success

## Related Documentation

- [Anti-Patterns Guide](.github/instructions/anti-patterns.instructions.md) - See #13 for file organization
- [CHANGELOG.md](CHANGELOG.md) - Complete fix history
- [Unreal Engine Module Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)

## Summary

| Structure Type | Include Paths Needed | Risk of SetEnv Error |
|---------------|---------------------|---------------------|
| Standard (Public/Private only) | ❌ No | ✅ None |
| Non-standard (root subdirs) | ✅ Yes (relative) | ⚠️ Low (if using ModuleDirectory) |
| Non-standard (root subdirs) | ✅ Yes (absolute) | ❌ High |

**Recommendation**: Gradually migrate to standard structure to simplify build configuration.

---

**Last Updated**: 2025-12-26  
**Related Issues**: SetEnv environment variable length error  
**Related PRs**: #357 (partial fix), #TBD (complete fix)
