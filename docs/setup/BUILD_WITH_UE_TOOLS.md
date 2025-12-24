# Building Adastrea with Unreal Engine Build Tools

This guide explains how to build Adastrea using only the Unreal Engine build tools, without requiring the full engine source code (~500MB vs ~50GB).

## Overview

Instead of cloning the entire UnrealEngine repository, we use **sparse checkout** to download only the essential build tools:

- **UnrealBuildTool (UBT)** - Compiles C++ code and manages dependencies
- **UnrealHeaderTool (UHT)** - Processes UE reflection macros
- **Build scripts** - Platform-specific build automation
- **Minimal dependencies** - Only what's needed for compilation

**Size Comparison:**
- Full UnrealEngine repo: ~50-60GB
- Build tools only: ~500MB-1GB
- Adastrea project: ~1GB

## Prerequisites

### Required Software

1. **Git** - For cloning repositories
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

### Epic Games Account Setup

To access the UnrealEngine repository, you need to:

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

### Option 1: Automated Setup (Recommended)

```bash
# 1. Setup build tools (downloads ~500MB)
./setup_ue_build_tools.sh

# 2. Build Adastrea
./build_with_ue_tools.sh Development Linux
```

### Option 2: Manual Setup

```bash
# 1. Make scripts executable
chmod +x setup_ue_build_tools.sh
chmod +x build_with_ue_tools.sh

# 2. Download build tools
./setup_ue_build_tools.sh

# 3. Build the project
./build_with_ue_tools.sh Development Linux
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
- **Linux** - Primary platform for CI/CD
- **Mac** - Supported with Xcode Command Line Tools
- **Windows** - Use Git Bash or WSL

```bash
# Linux
./build_with_ue_tools.sh Development Linux

# Mac
./build_with_ue_tools.sh Development Mac

# Windows (requires Git Bash or WSL)
bash build_with_ue_tools.sh Development Win64
```

## What Gets Downloaded?

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

### Authentication Failed

**Error:** `Failed to fetch from UnrealEngine repository`

**Solution:**
1. Link GitHub account at https://www.epicgames.com/account/connections
2. Accept Epic Games organization invitation
3. Verify membership at https://github.com/EpicGames
4. Try again after 5 minutes

### .NET SDK Not Found

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
3. Retry: `./setup_ue_build_tools.sh`

### Insufficient Disk Space

**Error:** `Insufficient disk space`

**Solution:**
- Build tools need: ~2GB total (500MB download + 1.5GB build)
- Free up space or use different mount point
- Check: `df -h`

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
A: No, this is only for compilation. You need full UE 5.6 for development.

**Q: Do I need to download this every time?**  
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

**Last Updated:** December 2025  
**Unreal Engine Version:** 5.6  
**Build Tools Size:** ~500MB
