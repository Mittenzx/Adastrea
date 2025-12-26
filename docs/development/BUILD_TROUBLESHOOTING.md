# Build Troubleshooting Guide

This guide covers common build issues and their solutions for the Adastrea project.

## MSBuild SetEnv Task Failure

### Error Message

```
Error MSB4018: The "SetEnv" task failed unexpectedly.
System.ArgumentException: Environment variable name or value is too long.
   at System.Environment.SetEnvironmentVariable(String variable, String value)
```

### Cause

Windows has environment variable length limitations (~32KB for all environment variables combined). When Unreal Build Tool (UBT) generates Visual Studio project files, it creates INCLUDE paths that can exceed this limit.

### Solution

**Remove unnecessary include path declarations from `.Build.cs` files:**

❌ **WRONG** - Empty arrays still contribute to environment variable length:
```csharp
PublicIncludePaths.AddRange(
    new string[] {
        // ... add public include paths required here ...
    }
);

PrivateIncludePaths.AddRange(
    new string[] {
        // ... add other private include paths required here ...
    }
);
```

✅ **CORRECT** - Remove these declarations entirely:
```csharp
// Note: PublicIncludePaths and PrivateIncludePaths removed to fix command line length issues
// UE5 automatically includes Public/ and Private/ folders, making explicit paths redundant
// This prevents MSBuild SetEnv task from failing with "Environment variable name or value is too long"

PublicDependencyModuleNames.AddRange(
    new string[] {
        "Core",
        // ... other modules
    }
);
```

### Why This Works

- **Unreal Engine 5** automatically includes `Public/` and `Private/` subdirectories from:
  - Your module's directory
  - All dependency module directories
- Explicit include path declarations are **redundant** and waste environment variable space
- Removing them reduces command line length during project generation

### Verification

Check all `.Build.cs` files in your project:
```bash
find . -name "*.Build.cs" -exec grep -l "PublicIncludePaths\|PrivateIncludePaths" {} \;
```

All modules should follow the same pattern:
- `Source/Adastrea/Adastrea.Build.cs` ✅ Fixed
- `Source/StationEditor/StationEditor.Build.cs` ✅ Fixed
- `Source/PlayerMods/PlayerMods.Build.cs` ✅ Fixed
- `Plugins/UnrealMCP/Source/UnrealMCP/UnrealMCP.Build.cs` ✅ Fixed

### Prevention

When creating new modules or plugins:
1. **DO NOT** add `PublicIncludePaths` or `PrivateIncludePaths` declarations
2. Rely on UE5's automatic include path resolution
3. Use forward declarations in headers where possible
4. Include full headers only in `.cpp` files

### Related Issues

If you still encounter SetEnv errors after this fix:
1. **Check AdditionalCompilerArguments length** in `.Target.cs` files
2. **Verify bOverrideBuildEnvironment** setting
3. **Clean and regenerate project files**:
   ```bash
   # Windows
   Right-click .uproject → Generate Visual Studio project files
   
   # Linux/Mac
   <UnrealEngine>/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh <YourProject>.uproject
   ```

## Missing Include Files

### Error Message

```
fatal error C1083: Cannot open include file: 'SomeHeader.h': No such file or directory
```

### Solution

Since we removed explicit include paths, use the correct include syntax:

✅ **Cross-module includes** (from dependencies):
```cpp
#include "Ships/SpaceshipDataAsset.h"       // From Adastrea module
#include "Factions/FactionDataAsset.h"      // From Adastrea module
```

✅ **Same-module includes**:
```cpp
#include "Combat/WeaponComponent.h"         // Within same module
#include "UI/TradingWidget.h"               // Within same module
```

✅ **Forward declarations** (header files):
```cpp
// Forward declare to avoid includes in headers
class USpaceshipDataAsset;
class AFaction;
```

## Circular Module Dependencies

### Error Message

```
error: circular dependency detected between modules: ModuleA depends on ModuleB, which depends on ModuleA
```

### Solution

1. **Review module dependency graph** using the automated checker:
   ```bash
   python .github/scripts/check_module_dependencies.py
   ```

2. **Fix circular dependencies** by:
   - Using interfaces instead of direct class references
   - Moving shared code to a common base module
   - Using event-driven communication instead of direct dependencies

3. **Example fix**:
   ```csharp
   // WRONG - creates circular dependency
   PublicDependencyModuleNames.Add("ModuleB");  // in ModuleA
   PublicDependencyModuleNames.Add("ModuleA");  // in ModuleB
   
   // CORRECT - one-way dependency
   PublicDependencyModuleNames.Add("ModuleA");  // in ModuleB only
   ```

## Link Errors (Unresolved Externals)

### Error Message

```
error LNK2019: unresolved external symbol "function" referenced in...
```

### Common Causes

1. **Missing function implementation**
   - Declared in header but not implemented in `.cpp`
   
2. **Missing module dependency**
   - Using classes from a module not listed in `PublicDependencyModuleNames`
   
3. **Inline/Template issues**
   - Template/inline functions must be in header, not `.cpp`

4. **UFUNCTION implementation mismatch**
   - `BlueprintNativeEvent` functions need `_Implementation` suffix in `.cpp`

### Solutions

```cpp
// Header (.h)
UFUNCTION(BlueprintNativeEvent, Category="AI")
void DecideAction();

// Implementation (.cpp) - must use _Implementation suffix
void UMyAIClass::DecideAction_Implementation()
{
    // implementation here
}
```

## Compilation Warnings as Errors

If you see many warnings being treated as errors, check:

```csharp
// In .Build.cs
bWarningsAsErrors = true;  // Module level

// In .Target.cs
bWarningsAsErrors = false; // Target level (for third-party code)
```

**Best Practice:**
- Enable `bWarningsAsErrors = true` in your module `.Build.cs` files
- Disable in `.Target.cs` to allow third-party plugins to build
- Fix warnings in your code, don't just disable them (see Anti-Pattern #15)

## Performance: Slow Compilation

### Unity Build Issues

If compilation is slow:

```csharp
// In .Target.cs
bUseUnityBuild = false;  // Can slow builds but improves iteration time

// Alternative: Adaptive build (UE5 default)
// Automatically excludes recently modified files from unity builds
```

### Include What You Use (IWYU)

```csharp
// In .Build.cs
IWYUSupport = IWYUSupport.Full;  // Enforces minimal includes
```

Benefits:
- Faster compilation
- Clearer dependencies
- Better code organization

## Clean Build Steps

When all else fails, perform a clean rebuild:

### Windows

```powershell
# 1. Close Unreal Editor and Visual Studio
# 2. Delete generated files
Remove-Item -Recurse -Force Binaries
Remove-Item -Recurse -Force Intermediate
Remove-Item -Recurse -Force Saved
Remove-Item *.sln

# 3. Regenerate project files
Right-click Adastrea.uproject → Generate Visual Studio project files

# 4. Open solution and rebuild
```

### Linux/Mac

```bash
# 1. Close Unreal Editor
# 2. Delete generated files
rm -rf Binaries Intermediate Saved *.sln *.xcworkspace

# 3. Regenerate project files
<UnrealEngine>/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh Adastrea.uproject

# 4. Build
<UnrealEngine>/Engine/Build/BatchFiles/Mac/Build.sh AdastreaEditor Mac Development Adastrea.uproject
```

## Additional Resources

- [Unreal Build Tool Documentation](https://docs.unrealengine.com/5.0/en-US/unreal-build-tool-in-unreal-engine/)
- [Module Dependencies Guide](./MODULE_DEPENDENCY_GUIDE.md)
- [Simplified Build Guide](./SIMPLIFIED_BUILD_GUIDE.md)
- [Anti-Patterns Documentation](../../.github/instructions/anti-patterns.instructions.md) - See Anti-Pattern #15 (Build System Fragility)

## Reporting New Issues

If you encounter a build issue not covered here:

1. Check existing [GitHub Issues](https://github.com/Mittenzx/Adastrea/issues)
2. Include:
   - Full error message
   - Steps to reproduce
   - Your environment (OS, UE version, Visual Studio version)
   - Recent changes to `.Build.cs` or `.Target.cs` files
3. Tag with `build-system` label

---

**Last Updated**: 2025-12-26  
**Applies To**: Unreal Engine 5.6+  
**Related Fixes**: SetEnv task error handling improvements
