# Unreal Engine Build Guide

This guide explains how to build Adastrea using Unreal Engine's command-line build tools instead of Visual Studio or other IDEs. This is particularly useful for:

- **Build error debugging**: Get detailed compiler output without IDE overhead
- **CI/CD pipelines**: Automate builds in continuous integration systems
- **Build scripts**: Create custom build automation
- **Cross-platform development**: Build on Linux/Mac without full IDE setup
- **Faster iteration**: Skip IDE load times for quick compilation checks

## Table of Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Build Commands Reference](#build-commands-reference)
- [Build Configurations](#build-configurations)
- [Build Targets](#build-targets)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Error Debugging](#error-debugging)
- [Build Automation Scripts](#build-automation-scripts)
- [Advanced Usage](#advanced-usage)
- [Troubleshooting](#troubleshooting)

## Overview

Unreal Engine provides several command-line tools for building projects:

- **UnrealBuildTool (UBT)**: The primary build system that compiles C++ code
- **UnrealEditor-Cmd**: Command-line editor for running automation and cooking
- **RunUAT**: Automation tool for packaging and deployment

This guide focuses on **UnrealBuildTool** for compilation and error debugging.

## Prerequisites

### Required Software

- **Unreal Engine 5.6** installed (check `Adastrea.uproject` for exact version)
- **C++ Compiler**:
  - Windows: Visual Studio 2022 with C++ workload (MSVC)
  - Linux: GCC or Clang
  - Mac: Xcode Command Line Tools

### Environment Setup

1. **Verify Unreal Engine Installation**:
   ```bash
   # Windows (default installation)
   dir "C:\Program Files\Epic Games\UE_5.6"
   
   # Mac
   ls "/Users/Shared/Epic Games/UE_5.6"
   
   # Linux
   ls "~/UnrealEngine/Engine"
   ```

2. **Set Environment Variables** (optional but recommended):
   ```bash
   # Windows (PowerShell)
   $env:UE_ROOT = "C:\Program Files\Epic Games\UE_5.6"
   
   # Linux/Mac (bash)
   export UE_ROOT="/Users/Shared/Epic Games/UE_5.6"
   ```

## Quick Start

### Windows

```batch
@echo off
REM Build the editor (Development configuration)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Development ^
  -Project="%CD%\Adastrea.uproject" ^
  -Verbose
```

### Mac

```bash
#!/bin/bash
# Build the editor (Development configuration)
"/Users/Shared/Epic Games/UE_5.6/Engine/Build/BatchFiles/Mac/Build.sh" \
  AdastreaEditor Mac Development \
  -Project="$(pwd)/Adastrea.uproject" \
  -Verbose
```

### Linux

```bash
#!/bin/bash
# Build the editor (Development configuration)
"~/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh" \
  AdastreaEditor Linux Development \
  -Project="$(pwd)/Adastrea.uproject" \
  -Verbose
```

## Build Commands Reference

### Basic Syntax

```
<Build.bat/sh> <Target> <Platform> <Configuration> -Project="<ProjectPath>" [Options]
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| **Target** | What to build (see Build Targets) | `AdastreaEditor`, `Adastrea` |
| **Platform** | Target platform | `Win64`, `Mac`, `Linux` |
| **Configuration** | Build configuration | `Development`, `Shipping`, `Debug` |
| **-Project** | Path to .uproject file | `-Project="C:\Adastrea\Adastrea.uproject"` |

### Common Options

| Option | Description | Use Case |
|--------|-------------|----------|
| `-Verbose` | Show detailed build output | Debugging build errors |
| `-VeryVerbose` | Show extremely detailed output | Deep debugging |
| `-IgnoreJunk` | Skip junk files during build | Faster clean builds |
| `-NoHotReload` | Disable hot reload | Fix hot reload issues |
| `-2019` | Use Visual Studio 2019 | Override compiler version |
| `-2022` | Use Visual Studio 2022 | Override compiler version |
| `-Clean` | Clean before building | Force full rebuild |
| `-Progress` | Show build progress | Monitor long builds |
| `-WaitMutex` | Wait for other builds | Avoid build conflicts |

## Build Configurations

Unreal Engine supports several build configurations:

### Development (Recommended for Testing)

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Development -Project="%CD%\Adastrea.uproject"
```

**Characteristics**:
- ✅ Reasonable performance
- ✅ Debug symbols included
- ✅ Editor features enabled
- ✅ Cheats and console commands available
- ⚠️ Some optimizations applied

### Debug (Full Debugging)

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Debug -Project="%CD%\Adastrea.uproject"
```

**Characteristics**:
- ✅ All debug symbols
- ✅ No optimizations
- ✅ Full debugging capabilities
- ❌ Very slow runtime performance
- ❌ Large binary size

### Shipping (Release Build)

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  Adastrea Win64 Shipping -Project="%CD%\Adastrea.uproject"
```

**Characteristics**:
- ✅ Maximum performance
- ✅ Full optimizations
- ✅ Smallest binary size
- ❌ No debug symbols
- ❌ No console commands
- ❌ No development cheats

### DebugGame (Hybrid)

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  Adastrea Win64 DebugGame -Project="%CD%\Adastrea.uproject"
```

**Characteristics**:
- ✅ Engine is optimized
- ✅ Game code has debug symbols
- ✅ Good compromise for debugging game logic
- ⚠️ Better performance than Debug

## Build Targets

Adastrea has multiple build targets defined in `Source/*.Target.cs`:

### AdastreaEditor (Editor Build)

```bash
# Build the editor
Build.bat AdastreaEditor Win64 Development -Project="..."
```

**Use for**:
- Working in Unreal Editor
- Content creation
- Blueprint editing
- Testing in PIE (Play In Editor)

**Modules included**:
- Adastrea (Runtime)
- PlayerMods (Runtime)
- StationEditor (Runtime)
- All editor-specific code

### Adastrea (Game Build)

```bash
# Build standalone game
Build.bat Adastrea Win64 Development -Project="..."
```

**Use for**:
- Standalone game executable
- Testing without editor
- Preparing for packaging
- Performance testing

**Modules included**:
- Adastrea (Runtime)
- PlayerMods (Runtime)
- StationEditor (Runtime)
- No editor code

## Platform-Specific Instructions

### Windows (Visual Studio)

#### Prerequisites
```batch
REM Check Visual Studio installation
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

#### Build Script (Windows)
Create `build_windows.bat`:
```batch
@echo off
setlocal

REM Configuration
set UE_ROOT=C:\Program Files\Epic Games\UE_5.6
set PROJECT_ROOT=%~dp0
set PROJECT_FILE=%PROJECT_ROOT%Adastrea.uproject
set BUILD_CONFIG=Development
set PLATFORM=Win64
set TARGET=AdastreaEditor

REM Clean previous build (optional)
if "%1"=="clean" (
    echo Cleaning build artifacts...
    rmdir /s /q "%PROJECT_ROOT%Binaries" 2>nul
    rmdir /s /q "%PROJECT_ROOT%Intermediate" 2>nul
    rmdir /s /q "%PROJECT_ROOT%Saved\Logs" 2>nul
)

REM Build
echo Building %TARGET% for %PLATFORM% in %BUILD_CONFIG% configuration...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" ^
    %TARGET% %PLATFORM% %BUILD_CONFIG% ^
    -Project="%PROJECT_FILE%" ^
    -Verbose ^
    -Progress

if %ERRORLEVEL% NEQ 0 (
    echo Build FAILED with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
) else (
    echo Build SUCCEEDED
)

endlocal
```

Usage:
```batch
# Normal build
build_windows.bat

# Clean build
build_windows.bat clean
```

### Mac (Xcode)

#### Prerequisites
```bash
# Check Xcode Command Line Tools
xcode-select --version
```

#### Build Script (Mac)
Create `build_mac.sh`:
```bash
#!/bin/bash
set -e

# Configuration
UE_ROOT="/Users/Shared/Epic Games/UE_5.6"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FILE="$PROJECT_ROOT/Adastrea.uproject"
BUILD_CONFIG="Development"
PLATFORM="Mac"
TARGET="AdastreaEditor"

# Clean previous build (optional)
if [ "$1" = "clean" ]; then
    echo "Cleaning build artifacts..."
    rm -rf "$PROJECT_ROOT/Binaries"
    rm -rf "$PROJECT_ROOT/Intermediate"
    rm -rf "$PROJECT_ROOT/Saved/Logs"
fi

# Build
echo "Building $TARGET for $PLATFORM in $BUILD_CONFIG configuration..."
"$UE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh" \
    $TARGET $PLATFORM $BUILD_CONFIG \
    -Project="$PROJECT_FILE" \
    -Verbose \
    -Progress

if [ $? -eq 0 ]; then
    echo "Build SUCCEEDED"
else
    echo "Build FAILED with error code $?"
    exit 1
fi
```

Make executable and run:
```bash
chmod +x build_mac.sh
./build_mac.sh
```

### Linux (GCC/Clang)

#### Prerequisites
```bash
# Check compiler
gcc --version
clang --version

# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential clang mono-runtime mono-devel
```

#### Build Script (Linux)
Create `build_linux.sh`:
```bash
#!/bin/bash
set -e

# Configuration
UE_ROOT="$HOME/UnrealEngine"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FILE="$PROJECT_ROOT/Adastrea.uproject"
BUILD_CONFIG="Development"
PLATFORM="Linux"
TARGET="AdastreaEditor"

# Clean previous build (optional)
if [ "$1" = "clean" ]; then
    echo "Cleaning build artifacts..."
    rm -rf "$PROJECT_ROOT/Binaries"
    rm -rf "$PROJECT_ROOT/Intermediate"
    rm -rf "$PROJECT_ROOT/Saved/Logs"
fi

# Build
echo "Building $TARGET for $PLATFORM in $BUILD_CONFIG configuration..."
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" \
    $TARGET $PLATFORM $BUILD_CONFIG \
    -Project="$PROJECT_FILE" \
    -Verbose \
    -Progress

if [ $? -eq 0 ]; then
    echo "Build SUCCEEDED"
else
    echo "Build FAILED with error code $?"
    exit 1
fi
```

Make executable and run:
```bash
chmod +x build_linux.sh
./build_linux.sh
```

## Error Debugging

### Reading Build Output

UnrealBuildTool provides detailed error messages in the console output:

```
Compiling game modules...
[1/245] Compile Module.Adastrea.cpp
[2/245] Compile SpaceshipDataAsset.cpp
Source/Adastrea/Ships/SpaceshipDataAsset.cpp(142): error C2065: 'undeclaredVariable' : undeclared identifier
```

**Error Message Components**:
1. **File path**: `Source/Adastrea/Ships/SpaceshipDataAsset.cpp`
2. **Line number**: `(142)`
3. **Error code**: `error C2065`
4. **Description**: `'undeclaredVariable' : undeclared identifier`

### Verbose Output for Debugging

For detailed compilation information:

```bash
# Windows
Build.bat AdastreaEditor Win64 Development -Project="..." -Verbose -VeryVerbose
```

**Output includes**:
- All compiler commands
- Include paths
- Preprocessor definitions
- Linker commands
- Detailed timing information

### Common Build Errors

#### 1. Missing Include Files

**Error**:
```
fatal error C1083: Cannot open include file: 'MyHeader.h': No such file or directory
```

**Solutions**:
1. Check include path in `.Build.cs`:
   ```csharp
   PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Ships"));
   ```
2. Verify file exists in correct location
3. Check file name casing (case-sensitive on Linux/Mac)

#### 2. Unresolved External Symbols

**Error**:
```
error LNK2019: unresolved external symbol "void MyFunction(void)"
```

**Solutions**:
1. Ensure function is implemented in .cpp file
2. Check module dependencies in `.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
   ```
3. Verify UFUNCTION macro is correct

#### 3. Module Not Found

**Error**:
```
ERROR: Unable to instantiate module 'PlayerMods'
```

**Solutions**:
1. Check module is listed in `.Target.cs`:
   ```csharp
   ExtraModuleNames.Add("PlayerMods");
   ```
2. Verify `.Build.cs` file exists
3. Check module directory structure

### Saving Build Logs

Redirect output to a file for analysis:

```bash
# Windows
Build.bat AdastreaEditor Win64 Development -Project="..." -Verbose > build.log 2>&1

# Linux/Mac
Build.sh AdastreaEditor Linux Development -Project="..." -Verbose > build.log 2>&1
```

## Build Automation Scripts

### Continuous Integration Build Script

Create `ci_build.sh` (cross-platform with some adjustments):

```bash
#!/bin/bash
# CI/CD Build Script for Adastrea
set -e

# Detect platform
case "$(uname -s)" in
    Linux*)     PLATFORM=Linux; BUILD_SCRIPT=Build.sh;;
    Darwin*)    PLATFORM=Mac; BUILD_SCRIPT=Build.sh;;
    MINGW*|MSYS*|CYGWIN*) PLATFORM=Win64; BUILD_SCRIPT=Build.bat;;
    *)          echo "Unsupported platform"; exit 1;;
esac

# Configuration
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FILE="$PROJECT_ROOT/Adastrea.uproject"
BUILD_CONFIG="${BUILD_CONFIG:-Development}"
TARGET="${TARGET:-AdastreaEditor}"

# Determine UE path
if [ "$PLATFORM" == "Win64" ]; then
    UE_ROOT="${UE_ROOT:-C:/Program Files/Epic Games/UE_5.6}"
elif [ "$PLATFORM" == "Mac" ]; then
    UE_ROOT="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.6}"
else
    UE_ROOT="${UE_ROOT:-$HOME/UnrealEngine}"
fi

# Clean build (if requested)
if [ "$CLEAN_BUILD" == "true" ]; then
    echo "Performing clean build..."
    rm -rf "$PROJECT_ROOT/Binaries"
    rm -rf "$PROJECT_ROOT/Intermediate"
    rm -rf "$PROJECT_ROOT/Saved/Logs"
fi

# Build
echo "=========================================="
echo "Building Adastrea"
echo "  Platform: $PLATFORM"
echo "  Target: $TARGET"
echo "  Configuration: $BUILD_CONFIG"
echo "  Unreal Engine: $UE_ROOT"
echo "=========================================="

if [ "$PLATFORM" == "Win64" ]; then
    "$UE_ROOT/Engine/Build/BatchFiles/$BUILD_SCRIPT" \
        $TARGET $PLATFORM $BUILD_CONFIG \
        -Project="$PROJECT_FILE" \
        -Verbose -Progress
else
    "$UE_ROOT/Engine/Build/BatchFiles/$PLATFORM/$BUILD_SCRIPT" \
        $TARGET $PLATFORM $BUILD_CONFIG \
        -Project="$PROJECT_FILE" \
        -Verbose -Progress
fi

BUILD_RESULT=$?

if [ $BUILD_RESULT -eq 0 ]; then
    echo "=========================================="
    echo "Build SUCCEEDED"
    echo "=========================================="
else
    echo "=========================================="
    echo "Build FAILED with error code $BUILD_RESULT"
    echo "=========================================="
    exit $BUILD_RESULT
fi
```

Usage:
```bash
# Default build
./ci_build.sh

# Clean build
CLEAN_BUILD=true ./ci_build.sh

# Shipping build
BUILD_CONFIG=Shipping TARGET=Adastrea ./ci_build.sh
```

### Parallel Module Build

For faster builds, UnrealBuildTool automatically uses parallel compilation. You can control this with:

```bash
# Windows - Use all CPU cores
Build.bat AdastreaEditor Win64 Development -Project="..." -MaxParallelActions=0

# Linux/Mac
Build.sh AdastreaEditor Linux Development -Project="..." -MaxParallelActions=0
```

**Options**:
- `0`: Use all available cores (default)
- `N`: Use N cores (e.g., `-MaxParallelActions=4`)

## Advanced Usage

### Building Specific Modules Only

To build only certain modules (faster iteration):

```bash
# Build only the Adastrea module
Build.bat AdastreaEditor Win64 Development -Project="..." -Module=Adastrea

# Build multiple modules
Build.bat AdastreaEditor Win64 Development -Project="..." -Module=Adastrea -Module=PlayerMods
```

### Incremental vs Full Rebuild

UnrealBuildTool automatically detects changes and performs incremental builds. For a full rebuild:

```bash
# Force full rebuild
Build.bat AdastreaEditor Win64 Development -Project="..." -Clean
```

### Generate Compile Commands (for IDEs)

For better IDE integration (VS Code, CLion, etc.):

```bash
# Generate compile_commands.json
Build.bat AdastreaEditor Win64 Development -Project="..." -Mode=GenerateClangDatabase
```

This creates `compile_commands.json` for better code completion and navigation.

### Build with Custom Defines

Add custom preprocessor definitions:

```bash
# Windows
Build.bat AdastreaEditor Win64 Development -Project="..." -Define=MY_CUSTOM_DEFINE=1
```

### Disable Unity Build (for debugging)

Unity Build combines multiple .cpp files for faster compilation but can hide include issues:

```bash
# Disable Unity Build
Build.bat AdastreaEditor Win64 Development -Project="..." -DisableUnity
```

## Troubleshooting

### Build is Slow

**Solutions**:
1. **Enable parallel builds**:
   ```bash
   Build.bat AdastreaEditor Win64 Development -Project="..." -MaxParallelActions=0
   ```

2. **Use precompiled headers**:
   - Already enabled by default in Unreal projects

3. **Incremental builds**:
   - Don't clean unless necessary
   - UBT automatically detects changes

4. **Exclude unnecessary modules**:
   - Build only modules you're working on with `-Module=`

### Build Fails with "Out of Memory"

**Solutions**:
1. **Reduce parallel actions**:
   ```bash
   Build.bat AdastreaEditor Win64 Development -Project="..." -MaxParallelActions=4
   ```

2. **Close other applications**

3. **Increase virtual memory (Windows)**

### "Access Denied" Errors

**Solutions**:
1. **Close Unreal Editor** - Editor locks binaries
2. **Close Visual Studio** - May lock files
3. **Run command prompt as Administrator** (Windows)
4. **Check antivirus** - May block compilation

### Compiler Not Found

**Windows**:
```batch
# Verify Visual Studio
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# Specify Visual Studio version
Build.bat AdastreaEditor Win64 Development -Project="..." -2022
```

**Mac**:
```bash
# Install Xcode Command Line Tools
xcode-select --install
```

**Linux**:
```bash
# Install compilers
sudo apt-get install build-essential clang
```

### .NET SDK Errors when Running UnrealBuildTool

If you see .NET-related errors when running UnrealBuildTool, these are usually issues with UBT's .NET SDK dependencies (not problems with UBT itself):

**Solutions**:
1. Verify .NET SDK version matches UE requirements
2. Clear NuGet cache:
   ```bash
   dotnet nuget locals all --clear
   ```
3. Reinstall Unreal Engine 5.6
4. Check [Epic's Known Issues](https://issues.unrealengine.com/)

See [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) for more details.

## Integration with CI/CD

### GitHub Actions Example

Create `.github/workflows/build.yml`:

```yaml
name: Build Adastrea

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-windows:
    runs-on: windows-latest
    
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
      
      - name: Setup Unreal Engine
        # Use a self-hosted runner with UE installed
        # or install UE in this step
        run: |
          echo "Verifying Unreal Engine installation..."
          dir "C:\Program Files\Epic Games\UE_5.6"
      
      - name: Build Project
        run: |
          & "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" `
            AdastreaEditor Win64 Development `
            -Project="$env:GITHUB_WORKSPACE\Adastrea.uproject" `
            -Verbose -Progress
        shell: pwsh
      
      - name: Upload Build Logs
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: build-logs
          path: Saved/Logs/

  build-linux:
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
      
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential clang mono-runtime mono-devel
      
      - name: Setup Unreal Engine
        # Use a self-hosted runner with UE installed
        run: |
          echo "Verifying Unreal Engine installation..."
          ls ~/UnrealEngine/Engine
      
      - name: Build Project
        run: |
          ~/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
            AdastreaEditor Linux Development \
            -Project="$GITHUB_WORKSPACE/Adastrea.uproject" \
            -Verbose -Progress
      
      - name: Upload Build Logs
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: build-logs-linux
          path: Saved/Logs/
```

**Note**: Building Unreal projects in GitHub Actions requires:
1. Self-hosted runner with Unreal Engine installed, or
2. Installing Unreal Engine in the workflow (time-consuming)

See [Epic's documentation](https://docs.unrealengine.com/5.6/en-US/using-continuous-integration-with-unreal-engine/) for CI/CD best practices.

## Best Practices

### Development Workflow

1. **Use Development Configuration** for daily work
2. **Use Verbose output** when debugging build errors
3. **Build incrementally** - don't clean unless necessary
4. **Test in editor** before standalone builds
5. **Use module-specific builds** when working on one system

### Error Debugging Workflow

1. **Build with `-Verbose` flag**
2. **Save build log to file**
3. **Search for first error** (ignore warnings initially)
4. **Fix error and rebuild**
5. **Use `-Module=` for faster iteration**

### Build Script Best Practices

1. **Parameterize paths** - don't hardcode UE installation
2. **Check return codes** - handle build failures
3. **Clean selectively** - only when necessary
4. **Log output** - save build logs for debugging
5. **Use environment variables** - make scripts flexible

## Quick Reference

### Build Commands

```bash
# Development Editor Build (most common)
Build.bat AdastreaEditor Win64 Development -Project="..." -Verbose

# Clean Build
Build.bat AdastreaEditor Win64 Development -Project="..." -Clean -Verbose

# Shipping Game Build
Build.bat Adastrea Win64 Shipping -Project="..." -Verbose

# Build Specific Module
Build.bat AdastreaEditor Win64 Development -Project="..." -Module=Adastrea

# Generate Compile Commands
Build.bat AdastreaEditor Win64 Development -Project="..." -Mode=GenerateClangDatabase
```

### Build Locations

After building, binaries are in:
- **Windows**: `Binaries/Win64/UnrealEditor-*.dll`
- **Mac**: `Binaries/Mac/UnrealEditor-*.dylib`
- **Linux**: `Binaries/Linux/UnrealEditor-*.so`

### Log Locations

Build logs and editor logs:
- `Saved/Logs/UnrealBuildTool.txt` - UBT build log
- `Saved/Logs/<ProjectName>.log` - Editor log

## Related Documentation

- [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) - Build issue solutions
- [CONTRIBUTING.md](CONTRIBUTING.md) - Development setup
- [CHEATSHEET.md](CHEATSHEET.md) - Quick reference
- [TESTING_AUTOMATION_GUIDE.md](TESTING_AUTOMATION_GUIDE.md) - Automated testing

## External Resources

- [Unreal Engine Build Configuration](https://docs.unrealengine.com/5.6/en-US/build-configuration-for-unreal-engine/)
- [UnrealBuildTool Documentation](https://docs.unrealengine.com/5.6/en-US/unreal-build-tool-in-unreal-engine/)
- [Building Unreal Engine from Source](https://docs.unrealengine.com/5.6/en-US/building-unreal-engine-from-source/)
- [Continuous Integration with Unreal Engine](https://docs.unrealengine.com/5.6/en-US/using-continuous-integration-with-unreal-engine/)

---

**Maintained by**: Adastrea Development Team  
**Last Updated**: 2025-11-13  
**Version**: 1.0
