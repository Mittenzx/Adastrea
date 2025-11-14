# Adastrea Build Files Package

This package contains build scripts and documentation that were removed from the main Adastrea repository in PR #120. These files are provided here for migration to a separate build tooling repository.

## Contents

This package includes all files that were removed from the main repository:

### Build Scripts
- `build_windows.bat` - Windows build script for Adastrea using UnrealBuildTool
- `build_unix.sh` - Unix/Linux/Mac build script for Adastrea using UnrealBuildTool

### Build Documentation
- `BUILD_ERROR_DEBUGGING.md` - Quick reference guide for debugging build errors
- `BUILD_TROUBLESHOOTING.md` - Comprehensive troubleshooting guide for build issues
- `UNREAL_BUILD_GUIDE.md` - Complete guide for building with UnrealBuildTool

### GitHub Workflows
- `.github/workflows/build-test.yml` - Manual build test workflow for CI/CD

## Package Information

**Created from**: PR #120 (commit 7ffc4368a4b45088c187608da9a66029fbbcde57)  
**Removal date**: 2025-11-14  
**Total size**: ~60.6 KB (documentation and scripts)  
**Purpose**: Migration to separate build tooling repository

## File Sizes

| File | Size | Description |
|------|------|-------------|
| build_windows.bat | ~9 KB | Windows build automation script |
| build_unix.sh | ~10 KB | Unix/Linux/Mac build automation script |
| BUILD_ERROR_DEBUGGING.md | ~15 KB | Build error debugging quick reference |
| BUILD_TROUBLESHOOTING.md | ~9 KB | Build troubleshooting guide |
| UNREAL_BUILD_GUIDE.md | ~35 KB | Complete UnrealBuildTool documentation |
| .github/workflows/build-test.yml | ~8 KB | GitHub Actions workflow |

## Usage

### For Build Repository Integration

1. **Copy to new repository**:
   ```bash
   # Create or navigate to your build tools repository
   cd /path/to/adastrea-build-tools
   
   # Copy all files from this package
   cp -r /path/to/build_package/* .
   ```

2. **Update paths if necessary**:
   - The scripts reference `Adastrea.uproject` in the parent directory
   - Adjust the `UE_ROOT` variable in scripts for your environment

3. **Make scripts executable** (Unix/Linux/Mac):
   ```bash
   chmod +x build_unix.sh
   ```

### Using the Build Scripts

**Windows**:
```cmd
REM Build editor
build_windows.bat

REM Clean build
build_windows.bat clean

REM Shipping build
build_windows.bat shipping
```

**Unix/Linux/Mac**:
```bash
# Build editor
./build_unix.sh

# Clean build
./build_unix.sh clean

# Shipping build
./build_unix.sh shipping
```

### GitHub Workflows

The `.github/workflows/build-test.yml` workflow can be integrated into a CI/CD pipeline:

1. Copy to `.github/workflows/` in your repository
2. Requires self-hosted runners with Unreal Engine 5.6 installed
3. Manually triggerable via GitHub Actions interface

## Documentation

### BUILD_ERROR_DEBUGGING.md
Quick reference for:
- Getting build errors without Visual Studio
- Reading and interpreting error messages
- Common error patterns and fixes
- Error code reference tables

### BUILD_TROUBLESHOOTING.md
Comprehensive guide for:
- Resolved build issues (historical reference)
- UnrealBuildTool .NET issues
- Cross-platform build notes
- Module loading issues

### UNREAL_BUILD_GUIDE.md
Complete documentation for:
- Building with UnrealBuildTool command-line
- Build configurations (Development, Debug, Shipping)
- Platform-specific build instructions
- CI/CD integration examples
- Advanced build options

## Why These Files Were Removed

The Adastrea repository maintainers decided to:
1. Keep the main repository focused on game content and core code
2. Reduce repository size and complexity
3. Separate build tooling into its own dedicated repository
4. Maintain cleaner git history for game development

See `BUILD_FILES_MOVED.md` in the main repository for the current build instructions.

## Integration Checklist

When setting up a new build repository with these files:

- [ ] Copy all files to new repository
- [ ] Update `UE_ROOT` paths in build scripts if needed
- [ ] Make `build_unix.sh` executable
- [ ] Test build scripts on target platforms
- [ ] Update documentation links if repository URL changes
- [ ] Set up GitHub Actions runners if using workflow
- [ ] Document new repository location in main Adastrea README
- [ ] Create repository-specific README.md

## Support

For questions about:
- **Build scripts**: Refer to UNREAL_BUILD_GUIDE.md in this package
- **Unreal Engine builds**: See Epic's official documentation
- **Adastrea project**: Visit the main repository at https://github.com/Mittenzx/Adastrea

## Version History

### Version 1.0 (2025-11-14)
- Initial package creation from PR #120
- Includes all build scripts and documentation removed from main repository
- Ready for migration to separate build tooling repository

## License

These scripts and documentation are part of the Adastrea project and follow the same license as the main repository (MIT License).

---

**Package Created**: 2025-11-14  
**Source**: Adastrea PR #120  
**Maintained by**: Adastrea Development Team
