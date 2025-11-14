# Build Files Moved

The build scripts and build-related documentation have been removed from the main Adastrea repository to keep it focused on game content and core code.

## What Was Removed

The following files have been packaged and removed from this repository:

### Build Scripts
- `build_windows.bat` - Windows build script
- `build_unix.sh` - Unix/Linux/Mac build script

### Build Documentation
- `BUILD_ERROR_DEBUGGING.md` - Build error debugging guide
- `BUILD_TROUBLESHOOTING.md` - Build troubleshooting guide
- `UNREAL_BUILD_GUIDE.md` - Complete Unreal build documentation

### Workflow
- `.github/workflows/build-test.yml` - GitHub Actions workflow that used the build scripts

## Where to Find the Files

A complete package of all removed files is available in `build_package/` directory (locally generated, not in git) and as `adastrea_build_files.tar.gz`.

These files can be moved to a separate repository dedicated to build tooling and CI/CD infrastructure.

## How to Build Adastrea Now

### Using Unreal Editor
1. Open `Adastrea.uproject` in Unreal Engine 5.6
2. The editor will automatically compile the project

### Using Visual Studio (Windows)
1. Right-click `Adastrea.uproject` and select "Generate Visual Studio project files"
2. Open the generated `.sln` file
3. Build in "Development Editor" configuration

### Using Xcode (Mac)
1. Right-click `Adastrea.uproject` and select "Generate Xcode project"
2. Open the generated project
3. Build the "AdastreaEditor" target

### Using Command Line

**Windows:**
```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AdastreaEditor Win64 Development -Project="%CD%\Adastrea.uproject"
```

**Mac/Linux:**
```bash
"/path/to/UE_5.6/Engine/Build/BatchFiles/Mac/Build.sh" AdastreaEditor Mac Development -Project="$PWD/Adastrea.uproject"
```

## Setup Validation

Before building, it's recommended to validate your environment:

```bash
# Cross-platform
python SetupCheck.py

# Unix/Linux/Mac
./SetupCheck.sh
```

## Integration with Build Repository

If you need to use the removed build scripts:

1. Clone or download them from the separate build repository
2. Copy the scripts to the Adastrea root directory
3. Run them as documented in their README

## Questions?

For build-related questions, refer to:
- Unreal Engine's official documentation
- The separate build repository (if one has been created)
- The project's setup validation scripts (`SetupCheck.py`, `SetupCheck.sh`)
