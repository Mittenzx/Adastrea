# Building Adastrea with Unreal Engine 5.6

This guide explains how to build Adastrea using your installed Unreal Engine 5.6.

> **🚨 IMPORTANT FOR WINDOWS USERS:** The `build_with_ue_tools.bat` script automatically detects your UE 5.6 installation and uses its build tools. This bypasses MSBuild SetEnv task failures with Visual Studio's environment variable limitations.

## Overview

### Windows Build Approach

The Windows build script (`build_with_ue_tools.bat`) automatically detects your Unreal Engine 5.6 installation by checking:

1. **Environment Variables** - `UE5_ROOT` or `UE_ROOT`
2. **Standard Installation Paths**:
   - `C:\Program Files\Epic Games\UE_5.6`
   - `C:\Program Files\Epic Games\UE5.6`
   - `C:\Program Files\Epic Games\UE_5.6.0`
3. **Windows Registry** - Custom installation paths registered by Epic Games Launcher

Once detected, the script uses the installed engine's UnrealBuildTool to compile your project.

### Linux/Mac Build Approach

Linux and Mac platforms use the minimal build tools approach (downloading only essential build tools from the UnrealEngine repository).

**Size Comparison:**
- Full UE installation: ~50-60GB (required for Windows)
- Build tools only: ~500MB-1GB (Linux/Mac)
- Adastrea project: ~1GB

## Prerequisites

### Windows Prerequisites

1. **Unreal Engine 5.6** - Install via Epic Games Launcher
   - Download from: https://www.unrealengine.com/download
   - Install Unreal Engine 5.6
   - The build script will auto-detect the installation

2. **.NET SDK 6.0+** - Required for UnrealBuildTool
   - Download from: https://dotnet.microsoft.com/download/dotnet/6.0
   - Verify with: `dotnet --version`

3. **Visual Studio 2022** - With C++ game development workload (optional, for IDE support)

### Linux/Mac Prerequisites

1. **Git** - For cloning build tools
   ```bash
   # Ubuntu/Debian
   sudo apt-get install git
   
   # Mac
   brew install git
   ```

2. **.NET SDK 6.0+** - Required for UnrealBuildTool
   ```bash
   # Ubuntu/Debian
   sudo apt-get install dotnet-sdk-6.0
   
   # Mac
   brew install dotnet-sdk
   
   # Verify installation
   dotnet --version
   ```

3. **Build Tools** - C++ compiler and standard libraries
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential clang mono-complete
   
   # Mac (Xcode Command Line Tools)
   xcode-select --install
   ```

### Epic Games Account Setup (Linux/Mac Only)

To access the UnrealEngine repository for build tools, you need to:

1. **Link GitHub to Epic Games Account**
   - Visit: https://www.epicgames.com/account/connections
   - Connect your GitHub account

2. **Accept Organization Invitation**
   - Check your GitHub email for invitation
   - Accept invitation to join Epic Games organization

3. **Verify Access**
   - Visit: https://github.com/EpicGames
   - Confirm you can see the organization

## Quick Start

### Windows

```batch
REM Build Adastrea (auto-detects UE 5.6 installation)
build_with_ue_tools.bat Development Win64

REM Alternative configurations
build_with_ue_tools.bat DebugGame Win64
build_with_ue_tools.bat Shipping Win64
```

**What the script does:**
1. ✅ Auto-detects your UE 5.6 installation
2. ✅ Uses the installed engine's UnrealBuildTool
3. ✅ Generates project files
4. ✅ Compiles the Adastrea project
5. ✅ Bypasses MSBuild SetEnv task limitations

**Setting a custom UE path (optional):**
```batch
REM Set UE5_ROOT environment variable
set UE5_ROOT=C:\MyCustomPath\UE_5.6
build_with_ue_tools.bat Development Win64
```

### Linux/Mac

```bash
# 1. Setup build tools (downloads ~500MB)
./setup_ue_build_tools.sh

# 2. Build Adastrea
./build_with_ue_tools.sh Development Linux
# or
./build_with_ue_tools.sh Development Mac
```

## Build Configurations

Available build configurations:

- **Development** - Default, includes debugging symbols, optimized performance
- **DebugGame** - Full debugging, slower performance
- **Shipping** - Maximum optimization, no debugging symbols

```bash
# Build with specific configuration
./build_with_ue_tools.sh Development Linux
./build_with_ue_tools.sh DebugGame Linux
./build_with_ue_tools.sh Shipping Linux
```

## Platform Support

Supported platforms:
- **Windows** - Native batch scripts (recommended for UE 5.6 SetEnv workaround)
- **Linux** - Primary platform for CI/CD
- **Mac** - Supported with Xcode Command Line Tools

```batch
REM Windows (native batch scripts)
build_with_ue_tools.bat Development Win64
```

```bash
# Linux
./build_with_ue_tools.sh Development Linux

# Mac
./build_with_ue_tools.sh Development Mac
```

## Build Details

### What Gets Downloaded (Linux/Mac Only)

The `setup_ue_build_tools.sh` script uses **git sparse-checkout** to download only:

```
UnrealBuildTools/
├── Engine/
│   ├── Build/
│   │   ├── BatchFiles/         # Build automation scripts
│   │   └── Build.version        # Version info
│   ├── Binaries/
│   │   └── DotNET/             # .NET binaries for UBT
│   ├── Source/
│   │   └── Programs/
│   │       ├── UnrealBuildTool/  # UBT source
│   │       └── Shared/           # Shared utilities
│   └── Config/
│       ├── BaseEngine.ini      # Minimal engine config
│       └── BasePlatforms.ini   # Platform configs
```

**Total size: ~500MB** (instead of 50GB for full engine)

### Windows Build Details

The Windows script automatically uses your installed Unreal Engine 5.6:
- **No download required** - Uses your existing UE installation
- **Automatic detection** - Finds UE via environment variables, standard paths, or registry
- **Full UE tools** - Uses the complete UE installation's build tools

## Build Output

After building, you'll find:

```
Adastrea/
├── Binaries/
│   └── Linux/
│       ├── libUnrealEditor-Adastrea.so
│       ├── libUnrealEditor-PlayerMods.so
│       └── libUnrealEditor-StationEditor.so
├── Intermediate/
│   └── Build/
│       └── Linux/
│           └── [Compilation artifacts]
└── UnrealBuildTools/
    └── [Build tools - not committed]
```

## GitHub Actions CI/CD

The repository includes a GitHub Actions workflow for automated building:

**Workflow:** `.github/workflows/test-build-with-ue-tools.yml`

### Manual Trigger

1. Go to **Actions** tab in GitHub
2. Select **Test Build with UE Build Tools**
3. Click **Run workflow**
4. Choose build configuration (Development, DebugGame, Shipping)

### Automatic Trigger

The workflow runs automatically on:
- Push to branches: `build/**`, `ci/ue-build-tools`
- Changes to: `Source/**`, `*.uproject`, `Config/**`

### CI Build Steps

1. ✓ Install .NET SDK and build dependencies
2. ✓ Validate project structure
3. ✓ Download and setup UE build tools (~500MB)
4. ✓ Build UnrealBuildTool from source
5. ✓ Compile Adastrea project
6. ✓ Upload build artifacts

## Troubleshooting

### Windows-Specific Issues

#### MSBuild SetEnv Task Failure (Primary Use Case)

**Error:** `Error MSB4018: The "SetEnv" task failed unexpectedly`

**This is the exact problem these scripts solve!**

**Solution:**
1. Use `build_with_ue_tools.bat` instead of Visual Studio
2. Bypasses MSBuild entirely
3. Works with UE 5.6 large projects (49KB+ include paths)

**Note:** You can still use Visual Studio for code editing, just not for building.

#### Git Not Found

**Error:** `git is not installed or not in PATH`

**Solution:**
Install Git for Windows from: https://git-scm.com/download/win

#### .NET SDK Not Found (Windows)

**Error:** `.NET SDK not found`

**Solution:**
```batch
REM Download and install .NET SDK 6.0 from:
REM https://dotnet.microsoft.com/download/dotnet/6.0

REM Verify installation
dotnet --version
```

### Authentication Failed

**Error:** `Failed to fetch from UnrealEngine repository`

**Solution:**
1. Link GitHub account at https://www.epicgames.com/account/connections
2. Accept Epic Games organization invitation
3. Verify membership at https://github.com/EpicGames
4. Try again after 5 minutes

### .NET SDK Not Found (Linux/Mac)

**Error:** `.NET SDK not found`

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install dotnet-sdk-6.0

# Mac
brew install dotnet-sdk

# Verify
dotnet --version
```

### UnrealBuildTool Build Failed

**Error:** `Failed to build UnrealBuildTool`

**Solution:**
1. Check .NET SDK version: `dotnet --version` (need 6.0+)
2. Clear NuGet cache: `dotnet nuget locals all --clear`
3. Retry: 
   - Windows: `setup_ue_build_tools.bat`
   - Linux/Mac: `./setup_ue_build_tools.sh`

### Insufficient Disk Space

**Error:** `Insufficient disk space`

**Solution:**
- Build tools need: ~2GB total (500MB download + 1.5GB build)
- Free up space or use different mount point
- Check: `df -h` (Linux/Mac) or `dir` (Windows)

### Build Compilation Errors

**Error:** Compilation errors in Adastrea modules

**Solution:**
1. Ensure all source files are present
2. Check for missing dependencies in `.Build.cs` files
3. Clean and rebuild:
   ```bash
   rm -rf Binaries/ Intermediate/
   ./build_with_ue_tools.sh Development Linux
   ```

## Limitations

### What This Includes

✓ UnrealBuildTool (UBT) for compilation  
✓ Build scripts and automation  
✓ Minimal engine configuration  
✓ Ability to compile C++ modules  

### What This Does NOT Include

✗ Unreal Editor executable  
✗ Content creation tools  
✗ Asset cooking capabilities  
✗ Runtime engine binaries  
✗ Blueprint editing  

### For Development/Editing

To actually **run** and **edit** the project, you still need:
- Full Unreal Engine 5.6 installation
- Download from: https://www.unrealengine.com/download

**Use this build tool approach for:**
- ✓ CI/CD pipelines
- ✓ Automated testing
- ✓ Code validation
- ✓ Build verification
- ✓ Lightweight compilation

## Advanced Usage

### Clean Build

**Windows:**
```batch
REM Remove build artifacts
rmdir /s /q Binaries Intermediate

REM Rebuild
build_with_ue_tools.bat Development Win64
```

**Linux/Mac:**
```bash
# Remove build artifacts
rm -rf Binaries/ Intermediate/ UnrealBuildTools/

# Setup and rebuild
./setup_ue_build_tools.sh
./build_with_ue_tools.sh Development Linux
```

### Clone Only (No Build)

```bash
# Just download build tools, don't build anything
./setup_ue_build_tools.sh --clone-only
```

### Custom Build Options

Edit `build_with_ue_tools.sh` to add custom UnrealBuildTool arguments:

```bash
dotnet "${UBT_PATH}" Adastrea Linux DevelopmentEditor \
    -Project="${PROJECT_FILE}" \
    -Progress \
    -NoHotReloadFromIDE \
    -Verbose  # Add for detailed output
```

## Performance Benchmarks

Typical build times (GitHub Actions runners):

- **Setup build tools:** 3-5 minutes
- **Build UnrealBuildTool:** 2-3 minutes  
- **Compile Adastrea (clean):** 10-15 minutes
- **Compile Adastrea (incremental):** 2-5 minutes

**Total CI time:** ~20-25 minutes for full clean build

## FAQ

**Q: Can I use this for game development?**  
A: Windows users have full UE installed. Linux/Mac users need full UE 5.6 for editor work; build tools are for compilation only.

**Q: Do I need to download build tools on Windows?**  
A: No, Windows script uses your installed UE 5.6. Linux/Mac download minimal build tools (~500MB).

**Q: Do I need to download build tools every time (Linux/Mac)?**  
A: No, `UnrealBuildTools/` is cached locally and in CI.

**Q: How much disk space do I need?**  
A: ~5GB total (500MB build tools + build artifacts + cache)

**Q: Can I build for multiple platforms?**  
A: Yes, but you need the respective platform SDKs installed.

**Q: Does this work with custom engine modifications?**  
A: No, this uses Epic's official UE 5.6 build tools.

**Q: Can I contribute code changes using this?**  
A: Yes! This is perfect for CI validation of code changes.

## Related Documentation

- [README.md](README.md) - Project overview
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [.github/workflows/test-build-with-ue-tools.yml](.github/workflows/test-build-with-ue-tools.yml) - CI workflow

## Support

For issues with:
- **Build tools setup:** Check [Troubleshooting](#troubleshooting) section
- **Epic Games access:** Visit https://www.epicgames.com/help
- **Adastrea project:** Open an issue on GitHub

---

**Last Updated:** 2025-12-27  
**Unreal Engine Version:** 5.6  
**Build Tools Size:** ~500MB  
**Windows Support:** Native batch scripts for SetEnv workaround
