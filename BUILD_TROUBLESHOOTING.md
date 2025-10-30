# Build Troubleshooting Guide

This document helps resolve common build issues for the Adastrea project.

## Unreal C++ Build Issues - RESOLVED ✓

The following issues have been fixed in the latest version:

### 1. Duplicate Log Category Declarations
**Issue**: `FLogCategoryLogAdastrea*` redefinition errors  
**Status**: ✅ FIXED  
**Solution**: Removed duplicate `Source/Adastrea/AdastreaLog.h`. Log categories are now declared only in `Source/Adastrea/Public/AdastreaLog.h`.

### 2. Missing Include Files
**Issue**: Cannot find `Characters/PersonnelDataAsset.h`, `Ships/SpaceshipDataAsset.h`, `Stations/SpaceStation.h`  
**Status**: ✅ FIXED  
**Solution**: Added `Adastrea/Characters`, `Adastrea/Ships`, `Adastrea/Stations` to PublicIncludePaths in `Adastrea.Build.cs`.

### 3. Deprecated Include Order Version
**Issue**: `EngineIncludeOrderVersion.Unreal5_3` is obsolete for UE 5.5  
**Status**: ✅ FIXED  
**Solution**: Updated to `EngineIncludeOrderVersion.Latest` and `BuildSettingsVersion.V5` in both Target.cs files.

## Unreal Build Tool (UBT) .NET Issues

If you encounter .NET-related errors during build, these are typically issues with Unreal Build Tool (UBT) itself, not your project code.

### Common UBT .NET Errors

#### 1. NuGet Package Compatibility Warnings
```
Microsoft.IO.Redist 6.1.0 restored for .NETFramework instead of target 'net8.0'
Microsoft.Build 17.14.28 restored for .NETFramework instead of target 'net8.0'
```

**Cause**: UBT (Epic's tool) may have NuGet package version conflicts on your system.

**Solutions**:
- Ensure you have the correct .NET SDK installed (check UE documentation for required version)
- Clear NuGet cache: `dotnet nuget locals all --clear`
- Re-install Unreal Engine 5.5
- Check Epic Games' Known Issues for your UE version

#### 2. Assembly Version Conflicts
```
Assembly version conflicts: System.Text.Json, System.Diagnostics.DiagnosticSource, etc.
```

**Cause**: Multiple .NET versions or UBT dependency conflicts.

**Solutions**:
- Ensure you only have the .NET versions required by UE 5.5 installed
- Clear intermediate build files: Delete `Intermediate/`, `Binaries/`, `Saved/` folders
- Regenerate project files: Right-click .uproject → Generate Visual Studio project files

#### 3. EpicGames.* Assembly References
```
References to EpicGames.Build.dll, EpicGames.Core.dll causing conflicts
```

**Cause**: This is internal to UBT.

**Solutions**:
- These assemblies are part of Epic's Unreal Build Tool
- You cannot modify them
- Ensure you're using the correct UE version (5.5 for this project)
- Verify your UE installation is not corrupted

### Verifying Your Build Environment

1. **Check .NET SDK Version**:
   ```bash
   dotnet --version
   ```
   Should match the version required by UE 5.5 (consult Epic's documentation)

2. **Clean Build**:
   ```bash
   # Delete generated files
   rm -rf Intermediate/ Binaries/ Saved/
   rm -f *.sln
   
   # Regenerate project (on Windows)
   # Right-click Adastrea.uproject → Generate Visual Studio project files
   
   # Or use UnrealBuildTool directly
   # <UE_INSTALL>/Engine/Build/BatchFiles/Build.bat AdastreaEditor Win64 Development
   ```

3. **Verify UE Installation**:
   - Open Epic Games Launcher
   - Verify Unreal Engine 5.5 installation
   - Click "Verify" on the engine version
   - Repair if necessary

## C# / .NET / WCF / System.Drawing Issues - NOT APPLICABLE

**Important**: This project does NOT contain:
- ❌ C# managed projects (.csproj files)
- ❌ NuGet package references
- ❌ WCF ServiceModel code
- ❌ System.Drawing usage
- ❌ Custom .NET applications

The only C# files in this project are:
- ✅ `*.Build.cs` - Unreal Build Tool module configurations
- ✅ `*.Target.cs` - Unreal Build Tool target configurations

These are not compiled as separate .NET projects. They are processed by UBT when building the Unreal project.

## Cross-Platform Build Notes

### Windows
- Requires Visual Studio 2022 with C++ workload
- UBT automatically handles .NET requirements

### Linux
- Requires Clang or GCC
- May need to install .NET SDK manually for UBT
- Follow Epic's Linux development setup guide

### macOS
- Requires Xcode
- UBT handles .NET requirements via Mono or .NET

## Getting Help

If you continue to experience build issues:

1. **Check Unreal Engine Version**: This project requires UE 5.5
   ```bash
   # Verify in Adastrea.uproject
   grep "EngineAssociation" Adastrea.uproject
   # Should show: "EngineAssociation": "5.5"
   ```

2. **Run Setup Validation**:
   ```bash
   python SetupCheck.py --verbose
   ```

3. **Check Epic's Documentation**:
   - [Unreal Engine Build Configuration](https://docs.unrealengine.com/5.5/en-US/build-configuration-for-unreal-engine/)
   - [Building Unreal Engine from Source](https://docs.unrealengine.com/5.5/en-US/building-unreal-engine-from-source/)

4. **Search Epic's Known Issues**:
   - Check [Unreal Engine Issues & Bug Tracker](https://issues.unrealengine.com/)
   - Search for your specific error message

5. **Project-Specific Issues**:
   - Open an issue on the [Adastrea GitHub repository](https://github.com/Mittenzx/Adastrea/issues)
   - Include full build log
   - Specify your OS, UE version, and .NET SDK version

## Summary

**For this project**:
- ✅ All C++ build issues have been fixed
- ✅ Include paths are correct
- ✅ Log categories have no duplicates
- ✅ Target configurations are up-to-date for UE 5.5

**If you see .NET errors**:
- These are UBT (Epic's tool) issues, not project issues
- Follow the UBT troubleshooting steps above
- Ensure your UE 5.5 installation is correct
- Check Epic's documentation and known issues

**This project does NOT have**:
- Custom .NET/C# applications
- NuGet packages to manage
- WCF or System.Drawing code

The project is a pure Unreal Engine C++ project with standard Blueprint Data Asset workflows.
