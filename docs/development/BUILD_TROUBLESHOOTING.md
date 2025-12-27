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

### Solution ✅ RESOLVED

**The Adastrea project has been restructured to follow standard UE5 module layout.**

As of December 2025, all modules use the standard Public/Private structure with no explicit include paths. This completely eliminates the SetEnv error.

### How It Was Fixed

1. **Moved all source files** to standard locations:
   - All header files → `Public/` subdirectories
   - All implementation files → `Private/` subdirectories

2. **Removed explicit include paths** from all `.Build.cs` files:
   ```csharp
   // NO LONGER NEEDED - these lines have been removed:
   // PublicIncludePaths.AddRange(new string[] { ... });
   ```

3. **UE5 automatic inclusion** now handles everything:
   - UE5 automatically includes `Public/` and `Private/` directories
   - No manual path configuration required

### If You Still Encounter This Error (Unreal Engine 5.6 Known Issue)

Even with correct Build.cs configuration, Unreal Engine 5.6's Visual Studio project generator can create extremely long include paths (49KB+) that exceed Windows environment variable limits.

**This is a known UE 5.6 issue with large projects.**

#### Workaround: Build with UnrealBuildTool Directly (Recommended)

Instead of building through Visual Studio, use UnrealBuildTool (UBT) directly to bypass the SetEnv limitation:

**Windows:**
```batch
# 1. Setup build tools (one-time, downloads ~500MB)
setup_ue_build_tools.bat

# 2. Build Adastrea directly with UBT (bypasses Visual Studio)
build_with_ue_tools.bat Development Win64

# Alternative: Use installed UE 5.6
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
  Adastrea Win64 DevelopmentEditor ^
  -Project="%CD%\Adastrea.uproject" ^
  -Progress -NoHotReloadFromIDE
```

**Benefits of UBT Direct Build:**
- ✅ Bypasses MSBuild SetEnv limitations
- ✅ Faster compilation (no Visual Studio overhead)
- ✅ Works with extremely large projects
- ✅ Same output as Visual Studio builds
- ✅ Can still use Visual Studio for code editing

**See Also:** [Build with UE Tools Guide](../setup/BUILD_WITH_UE_TOOLS.md) for detailed instructions.

#### If Building Through Visual Studio is Required

If you must use Visual Studio for building (not just editing):

1. **Check for non-standard directory structures**
   - Ensure all headers are in `Public/` subdirectories
   - Ensure all implementations are in `Private/` subdirectories

2. **Remove explicit include paths** from `.Build.cs`:
   ```csharp
   // Remove these if present:
   PublicIncludePaths.AddRange(new string[] { ... });
   PrivateIncludePaths.AddRange(new string[] { ... });
   ```

3. **Clean and regenerate project files**:
   ```batch
   # Windows
   Right-click .uproject → Generate Visual Studio project files
   ```

4. **If issue persists**, use UBT direct build (see above)

### Verification

Check all `.Build.cs` files should NOT have explicit include paths:
```bash
find . -name "*.Build.cs" -exec grep -l "PublicIncludePaths\|PrivateIncludePaths" {} \;
```

Expected result: No matches (or only comments mentioning why they're not needed)

All modules in Adastrea now follow the standard pattern:
- `Source/Adastrea/Adastrea.Build.cs` ✅ No explicit paths
- `Source/StationEditor/StationEditor.Build.cs` ✅ No explicit paths
- `Source/PlayerMods/PlayerMods.Build.cs` ✅ No explicit paths
- `Plugins/UnrealMCP/Source/UnrealMCP/UnrealMCP.Build.cs` ✅ No explicit paths

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

**Last Updated**: 2025-12-27  
**Applies To**: Unreal Engine 5.6+  
**Related Fixes**: SetEnv task error handling improvements, Windows UBT direct build workaround
