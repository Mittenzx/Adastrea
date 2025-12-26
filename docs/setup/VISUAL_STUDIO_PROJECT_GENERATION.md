# Visual Studio Project Generation Guide

This guide helps you successfully generate Visual Studio project files for the Adastrea project and resolve common issues.

---

## Quick Start: Generate Project Files

### Method 1: Using Windows Explorer (Recommended)
1. Locate `Adastrea.uproject` in Windows Explorer
2. **Right-click** on the file
3. Select **"Generate Visual Studio project files"**
4. Wait for the process to complete
5. Open `Adastrea.sln` in Visual Studio

### Method 2: Using Command Line
```batch
:: Navigate to your Unreal Engine installation
cd "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles"

:: Generate project files
GenerateProjectFiles.bat "C:\Path\To\Your\Adastrea.uproject"
```

### Method 3: Using UnrealBuildTool Directly
```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="C:\Path\To\Your\Adastrea.uproject" -game -rocket -progress
```

---

## Common Issues and Solutions

### Issue 1: CS0101 - Duplicate Namespace/Type Error

**Symptoms:**
```
error CS0101: The namespace '<global namespace>' already contains a definition for 'AdastreaDirectorEditor'
```

**Cause:** Duplicate or conflicting plugin structures in your project.

**Solution:**

1. **Clean Build Artifacts:**
   ```batch
   :: Delete intermediate build files
   rmdir /s /q Intermediate
   rmdir /s /q Binaries
   rmdir /s /q .vs
   
   :: Clean plugin artifacts
   rmdir /s /q Plugins\AdastreaDirector\Binaries
   rmdir /s /q Plugins\AdastreaDirector\Intermediate
   ```

2. **Remove Duplicate Plugins:**
   - Check for folders ending in `_Built` (e.g., `AdastreaDirector_Built`)
   - Delete any `HostProject` folders inside plugin directories
   - Ensure only one copy of each plugin exists in `Plugins/` directory

3. **Regenerate Project Files:**
   - Right-click `Adastrea.uproject` → Generate Visual Studio project files

### Issue 2: "Bad IL Format" or "Invalid CLR Version"

**Symptoms:**
```
Compiled assembly file appears to be for a newer CLR version or is otherwise invalid
(Exception: Bad IL format)
```

**Cause:** Corrupted DLL files in the Intermediate build directory.

**Solution:**

1. **Delete Build Rules DLLs:**
   ```batch
   del /s /q Intermediate\Build\BuildRules\*.dll
   ```

2. **Clear All Intermediate Files:**
   ```batch
   rmdir /s /q Intermediate
   ```

3. **Regenerate Project Files**

### Issue 3: Missing or Corrupted .Build.cs Files

**Symptoms:**
```
error: Could not find module rules file for 'AdastreaDirector'
```

**Solution:**

1. **Verify Build.cs Files Exist:**
   - `Source/Adastrea/Adastrea.Build.cs`
   - `Source/PlayerMods/PlayerMods.Build.cs`
   - `Source/StationEditor/StationEditor.Build.cs`
   - `Plugins/AdastreaDirector/Source/AdastreaDirector/AdastreaDirector.Build.cs`
   - `Plugins/AdastreaDirector/Source/AdastreaDirectorEditor/AdastreaDirectorEditor.Build.cs`

2. **Check Build.cs Syntax:**
   - Must have unique class names matching the module name
   - Must inherit from `ModuleRules`
   - Must have proper constructor signature

3. **Example Correct Build.cs:**
   ```csharp
   using UnrealBuildTool;

   public class AdastreaDirector : ModuleRules
   {
       public AdastreaDirector(ReadOnlyTargetRules Target) : base(Target)
       {
           PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
           
           PublicDependencyModuleNames.AddRange(new string[] {
               "Core",
               "CoreUObject",
               "Engine"
           });
       }
   }
   ```

### Issue 4: Plugin Not Found or Not Loading

**Symptoms:**
- Plugin doesn't appear in the Plugins list
- Build errors about missing plugin modules

**Solution:**

1. **Verify Plugin Descriptor:**
   - Check `Plugins/AdastreaDirector/AdastreaDirector.uplugin` exists
   - Ensure it's valid JSON with proper module definitions

2. **Check .uproject Plugin References:**
   - If plugin is optional, it may need to be listed in `Adastrea.uproject`
   - Add to Plugins array if needed (see below)

3. **Verify Plugin Structure:**
   ```
   Plugins/AdastreaDirector/
   ├── AdastreaDirector.uplugin
   ├── Resources/
   │   └── Icon128.png
   ├── Source/
   │   ├── AdastreaDirector/
   │   │   ├── AdastreaDirector.Build.cs
   │   │   ├── Public/
   │   │   └── Private/
   │   └── AdastreaDirectorEditor/
   │       ├── AdastreaDirectorEditor.Build.cs
   │       ├── Public/
   │       └── Private/
   └── Content/
   ```

### Issue 5: Circular Module Dependencies

**Symptoms:**
```
error: Circular dependency detected between modules
```

**Solution:**

1. **Check Module Dependencies:**
   ```bash
   python .github/scripts/check_module_dependencies.py
   ```

2. **Review Build.cs Dependencies:**
   - Ensure no circular references in `PublicDependencyModuleNames`
   - Consider moving shared code to a separate module

3. **Common Circular Dependencies in Adastrea:**
   - ❌ `Adastrea` depends on `StationEditor` AND `StationEditor` depends on `Adastrea`
   - ✅ Only `StationEditor` should depend on `Adastrea`

### Issue 6: Command Line Too Long / getenv Errors

**Symptoms:**
```
error: Command line is too long
error: 'getenv' is not a valid Win32 application
```

**Cause:** Excessive `PublicIncludePaths` entries in `.Build.cs` files causing command line length to exceed Windows limits during project generation.

**Solution:**

**FIXED (2025-12-26):** This issue has been resolved in the codebase. All redundant `PublicIncludePaths` have been removed from module Build.cs files.

1. **Understanding the Fix:**
   - Unreal Engine 5 automatically includes `Public/` and `Private/` folders for each module
   - Explicit `PublicIncludePaths` entries were redundant and caused excessively long command lines
   - All existing includes (e.g., `#include "Ships/SpaceshipDataAsset.h"`) continue to work

2. **If You're Still Experiencing This Issue:**
   - Ensure your local branch is up to date with the latest changes
   - Check that your Build.cs files don't have extra PublicIncludePaths entries
   - Run: `git pull origin main` to get the latest fixes

3. **Technical Details:**
   - **Removed from `Adastrea.Build.cs`**: 18 redundant include path entries
   - **Removed from `StationEditor.Build.cs`**: 2 redundant include path entries
   - **Removed from `PlayerMods.Build.cs`**: 2 redundant include path entries
   - UE5's automatic path resolution handles all includes correctly

4. **Verification:**
   - After updating, regenerate Visual Studio project files
   - Project generation should complete without command line errors
   - All includes should compile successfully

---

## Complete Clean Build Process

If you're experiencing persistent issues, follow this complete clean build process:

### Step 1: Close Everything
- Close Visual Studio
- Close Unreal Engine Editor
- Close any command prompts or PowerShell windows

### Step 2: Delete Generated Files
```batch
:: Main project
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved
rmdir /s /q .vs
del *.sln

:: All plugins
for /d %%D in (Plugins\*) do (
    if exist "%%D\Binaries" rmdir /s /q "%%D\Binaries"
    if exist "%%D\Intermediate" rmdir /s /q "%%D\Intermediate"
)

:: Delete build rules DLLs specifically
if exist "Intermediate\Build\BuildRules" rmdir /s /q "Intermediate\Build\BuildRules"
```

### Step 3: Verify Plugin Structure
- Ensure no `_Built` folders exist
- Ensure no `HostProject` folders exist
- Verify each plugin has a valid `.uplugin` file

### Step 4: Regenerate Project Files
```batch
:: Right-click Adastrea.uproject → Generate Visual Studio project files
:: Or use:
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "C:\Path\To\Adastrea.uproject"
```

### Step 5: Open and Build
1. Open `Adastrea.sln` in Visual Studio
2. Set build configuration to **Development Editor**
3. Build → Build Solution (Ctrl+Shift+B)

---

## Verification Checklist

After generating project files, verify the following:

- [ ] `Adastrea.sln` exists in project root
- [ ] Solution contains these projects:
  - [ ] Adastrea
  - [ ] PlayerMods
  - [ ] StationEditor
  - [ ] AdastreaDirector (plugin)
  - [ ] AdastreaDirectorEditor (plugin)
  - [ ] UnrealMCP (plugin)
- [ ] No build errors when opening solution
- [ ] Project builds successfully in **Development Editor** configuration

---

## Advanced: Manual Plugin Building

If you need to build a plugin separately (for distribution):

```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin ^
  -Plugin="C:\Path\To\Adastrea\Plugins\AdastreaDirector\AdastreaDirector.uplugin" ^
  -Package="C:\Output\AdastreaDirector" ^
  -Rocket
```

**Important:** Do NOT place built plugins back into the source tree. This will cause CS0101 errors.

---

## Platform-Specific Notes

### Windows
- Use Visual Studio 2022 or later
- Install "Game development with C++" workload
- Install ".NET desktop development" workload

### Mac
- Use Xcode 14.0 or later
- Generate project files creates `.xcworkspace` instead of `.sln`

### Linux
- Use QtCreator, CLion, or VS Code
- Install clang compiler toolchain
- Generate project files creates Makefile

---

## Getting Help

If you continue to experience issues:

1. **Check the build_errors.md file** for known compilation issues
2. **Search GitHub Issues** for similar problems
3. **Create a new issue** with:
   - Full error message
   - Unreal Engine version
   - Visual Studio version
   - Steps you've already tried
   - Output from `check_module_dependencies.py`

---

## Related Documentation

- [build_errors.md](build_errors.md) - Known compilation errors and solutions
- [MODULE_DEPENDENCY_GUIDE.md](MODULE_DEPENDENCY_GUIDE.md) - Module organization guide
- [PLUGIN_DEVELOPMENT_GUIDE.md](PLUGIN_DEVELOPMENT_GUIDE.md) - Plugin development guide
- [CONTRIBUTING.md](CONTRIBUTING.md) - General contribution guidelines

---

**Last Updated:** 2025-12-21  
**Unreal Engine Version:** 5.6  
**Visual Studio Version:** 2022
