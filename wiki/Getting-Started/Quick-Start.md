# Quick Start Guide

Get up and running with Adastrea in 10 minutes!

## Prerequisites

- **Unreal Engine 5.6** (check the .uproject file for exact version)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++ development
- **Git** for version control
- At least 50GB of free disk space

## Step 1: Clone the Repository

```bash
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea
```

## Step 2: Validate Your Environment (Recommended)

Run our automated setup validation to ensure everything is configured correctly:

### Option 1: Shell Script (Linux/Mac/WSL)
```bash
./SetupCheck.sh
```

### Option 2: Python Script (Cross-Platform)
```bash
python SetupCheck.py
```

The validation scripts check for:
- ✓ Unreal Engine version compatibility (5.6)
- ✓ C++ compiler availability
- ✓ Required project folders and files
- ✓ Essential documentation

## Step 3: Generate Project Files

1. Right-click on `Adastrea.uproject` file
2. Select "Generate Visual Studio project files"
3. Wait for the process to complete

## Step 4: Build the Project

### Option A: Using Unreal Engine
1. Double-click `Adastrea.uproject` to open in Unreal Engine
2. Wait for initial compilation to complete
3. The editor will open automatically when ready

### Option B: Using Visual Studio
1. Open `Adastrea.sln` in Visual Studio
2. Set build configuration to "Development Editor"
3. Build the solution (Ctrl+Shift+B)
4. Open `Adastrea.uproject` when build completes

## Step 5: Verify Installation

1. **Launch the editor** (if not already open)
2. **Check Data Assets**: Navigate to `Content/DataAssets/` and verify example assets load
3. **Run the game**: Click the Play button in the editor toolbar
4. **Check console**: Verify no critical errors in the Output Log

### Optional: Run Asset Validation
```python
# In Unreal Editor Python console
import unreal
exec(open('AdastreaAssetValidator.py').read())
```

## Next Steps

### For Designers
- Read the [Designer Onboarding Guide](Designer-Onboarding.md)
- Start with [Content Creation Quick Start](Content-Creation.md)
- Explore [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)

### For Programmers
- Review the [Architecture Overview](../Developer-Guides/Architecture.md)
- Read [Contributing Guidelines](../Developer-Guides/Contributing.md)
- Check [Code Style Guide](../Developer-Guides/Code-Style.md)

### For Everyone
- Explore the [Core Systems Overview](../Core-Systems/README.md)
- Try [Procedural Content Generation](../Automation-Testing/Procedural-Generation.md)
- Review [Testing Automation](../Automation-Testing/Testing-Guide.md)

## Common Issues

### "Unreal Engine not found"
- Ensure Unreal Engine 5.6 is installed via Epic Games Launcher
- Check that the engine path is correct in system PATH

### "Build Failed"
- Run `SetupCheck.py` to verify environment
- Check you have Visual Studio 2022 with C++ desktop development workload
- Ensure you have enough disk space (50GB+)

### "Missing modules or plugins"
- Delete `Intermediate` and `Binaries` folders
- Regenerate project files
- Rebuild solution

## Getting Help

- **Documentation**: Check the [Documentation Index](../Reference/Documentation-Index.md)
- **Issues**: Open an issue on [GitHub](https://github.com/Mittenzx/Adastrea/issues)
- **Discussions**: Join [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)

---

**Estimated Time**: 10-30 minutes depending on hardware and download speeds

**Next**: [Installation Guide](Installation.md) for detailed setup instructions

*For the complete README, see [Main Repository README](../../README.md)*
