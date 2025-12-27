# Windows Build Scripts - Quick Reference

> **⚠️ IMPORTANT UPDATE (Dec 2025):** The `build_with_ue_tools.bat` script has been updated to use your **installed Unreal Engine** instead of downloading UnrealBuildTools. The `setup_ue_build_tools.bat` step is **no longer required**!

## Purpose

These scripts provide a workaround for the **Unreal Engine 5.6 MSBuild SetEnv task failure** on Windows, which occurs when Visual Studio project files have excessively long include paths (49KB+).

## Problem

When building large UE 5.6 projects through Visual Studio, you may encounter:

```
Error MSB4018: The "SetEnv" task failed unexpectedly.
System.ArgumentException: Environment variable name or value is too long.
```

This is a **known UE 5.6 issue** where the project generator includes all engine paths in the Visual Studio project, exceeding Windows environment variable limits (~32KB).

## Solution

Build using the standard Unreal Engine Build.bat from your installed UE, bypassing Visual Studio's MSBuild entirely.

## Scripts

### `build_with_ue_tools.bat` (UPDATED)

> **NEW:** Now uses your installed Unreal Engine. No setup required!

Builds Adastrea using standard UE Build.bat, bypassing Visual Studio.

**Usage:**
```batch
build_with_ue_tools.bat [Configuration] [Platform]

REM Examples:
build_with_ue_tools.bat                    REM Defaults: Development Win64
build_with_ue_tools.bat Development Win64
build_with_ue_tools.bat DebugGame Win64
build_with_ue_tools.bat Shipping Win64
```

**What it does:**
- Auto-detects installed Unreal Engine 5.6 or 5.5
- Generates project files (if needed)
- Compiles Adastrea modules using standard UE Build.bat
- Validates build output

**Requirements:**
- Unreal Engine 5.6 (or 5.5) installed via Epic Games Launcher
- Visual Studio 2022 with C++ tools
- Windows SDK

**Installation Paths Checked:**
1. `C:\Program Files\Epic Games\UE_5.6`
2. `C:\Program Files\Epic Games\UE_5.5` (fallback)
3. `C:\Program Files (x86)\Epic Games\UE_5.6`
4. Manual input if not found

### `setup_ue_build_tools.bat` (DEPRECATED FOR WINDOWS)

> **⚠️ DEPRECATED:** This script is no longer needed for Windows builds. The updated `build_with_ue_tools.bat` uses your installed UE instead.

This script is still used for Linux/Mac builds. For Windows, just run `build_with_ue_tools.bat` directly.

## Quick Start

```batch
REM Just build! (no setup needed)
build_with_ue_tools.bat Development Win64
```

That's it! The script will auto-detect your UE installation and build the project.

## Benefits

✅ **No setup required** - Just run the script with UE installed  
✅ **Auto-detection** - Finds UE 5.6 or 5.5 automatically  
✅ **Bypasses MSBuild SetEnv limitations** - No 32KB environment variable limits  
✅ **Faster compilation** - Uses standard UE build tools  
✅ **Works with large projects** - Handles 49KB+ include paths  
✅ **Same output** - Produces identical binaries to Visual Studio builds  
✅ **Still use VS for editing** - You can still use Visual Studio for code editing, just not building

## Prerequisites

### Required Software

1. **Unreal Engine 5.6 (or 5.5)**
   - Download via Epic Games Launcher
   - Install to default location (auto-detected)
   - Verify: Check that UE is installed in `C:\Program Files\Epic Games\UE_5.6`

2. **Visual Studio 2022**
   - Download: https://visualstudio.microsoft.com/vs/
   - Install C++ game development workload
   - Required for compiler and build tools

3. **Windows SDK**
   - Usually installed with Visual Studio
   - Verify in Visual Studio Installer

### Optional (No Longer Required)
- ❌ ~~Git for Windows~~ (not needed for this script)
- ❌ ~~.NET SDK~~ (warning only if missing)
- ❌ ~~Epic Games GitHub access~~ (not needed for this approach)

- Visual Studio 2022 (for code editing only)
- Unreal Engine 5.6 (for running editor and content creation)

## Troubleshooting

### "Git not found"

Install Git for Windows: https://git-scm.com/download/win

### ".NET SDK not found"

Install .NET SDK 6.0: https://dotnet.microsoft.com/download/dotnet/6.0

### "Failed to fetch from UnrealEngine repository"

1. Link GitHub account at https://www.epicgames.com/account/connections
2. Accept Epic Games organization invitation (check email)
3. Wait 5 minutes and try again

### "UnrealBuildTool failed to build"

1. Check .NET SDK version: `dotnet --version` (need 6.0+)
2. Clear NuGet cache: `dotnet nuget locals all --clear`
3. Retry: `setup_ue_build_tools.bat`

## Alternative: Use Installed UE 5.6

If you have Unreal Engine 5.6 installed, you can use its UnrealBuildTool directly:

```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
  Adastrea Win64 DevelopmentEditor ^
  -Project="%CD%\Adastrea.uproject" ^
  -Progress -NoHotReloadFromIDE
```

## Full Documentation

- [Build with UE Tools Guide](../setup/BUILD_WITH_UE_TOOLS.md) - Complete guide
- [Build Troubleshooting](../development/BUILD_TROUBLESHOOTING.md) - Common issues

## Support

For issues or questions:
- Check [BUILD_TROUBLESHOOTING.md](../development/BUILD_TROUBLESHOOTING.md)
- Open an issue on GitHub: https://github.com/Mittenzx/Adastrea/issues
- Tag with `build-system` label

---

**Last Updated:** 2025-12-27  
**For:** Unreal Engine 5.6 SetEnv workaround  
**Platform:** Windows 10/11
