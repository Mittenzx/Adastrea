# Windows Build Scripts - Quick Reference

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

Build directly with UnrealBuildTool (UBT), bypassing Visual Studio's MSBuild entirely.

## Scripts

### `setup_ue_build_tools.bat`

Downloads only the Unreal Engine build tools (~500MB) instead of the full engine source (~50GB).

**Usage:**
```batch
setup_ue_build_tools.bat
```

**What it does:**
- Downloads UnrealBuildTool, UnrealHeaderTool, and build scripts
- Uses git sparse checkout (only ~500MB download)
- One-time setup

**Requirements:**
- Git for Windows
- .NET SDK 6.0+
- Epic Games GitHub account access

### `build_with_ue_tools.bat`

Builds Adastrea using UnrealBuildTool directly, bypassing Visual Studio.

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
- Finds or builds UnrealBuildTool
- Generates project files (if needed)
- Compiles Adastrea modules
- Validates build output

## Quick Start

```batch
REM 1. One-time setup (downloads ~500MB)
setup_ue_build_tools.bat

REM 2. Build Adastrea
build_with_ue_tools.bat Development Win64
```

## Benefits

✅ **Bypasses MSBuild SetEnv limitations** - No 32KB environment variable limits  
✅ **Faster compilation** - No Visual Studio overhead  
✅ **Works with large projects** - Handles 49KB+ include paths  
✅ **Same output** - Produces identical binaries to Visual Studio builds  
✅ **Still use VS for editing** - You can still use Visual Studio for code editing, just not building

## Prerequisites

### Required Software

1. **Git for Windows**
   - Download: https://git-scm.com/download/win
   - Verify: `git --version`

2. **.NET SDK 6.0+**
   - Download: https://dotnet.microsoft.com/download/dotnet/6.0
   - Verify: `dotnet --version`

3. **Epic Games GitHub Access**
   - Link GitHub at: https://www.epicgames.com/account/connections
   - Accept Epic Games organization invitation
   - Verify: https://github.com/EpicGames

### Optional (for development)

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
