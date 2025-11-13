# Build Error Debugging Quick Reference

This is a quick reference guide for debugging build errors using Unreal Engine's command-line tools instead of Visual Studio. For complete documentation, see [UNREAL_BUILD_GUIDE.md](UNREAL_BUILD_GUIDE.md).

## Quick Start - Get Build Errors Without Visual Studio

### Windows

```batch
# Simple build with error output
build_windows.bat

# Verbose build for detailed errors
build_windows.bat verbose

# Clean build if you suspect cached issues
build_windows.bat clean verbose
```

### Mac/Linux

```bash
# Simple build with error output
./build_unix.sh

# Verbose build for detailed errors
./build_unix.sh verbose

# Clean build if you suspect cached issues
./build_unix.sh clean verbose
```

### Save Error Log to File

```bash
# Windows (PowerShell)
.\build_windows.bat verbose 2>&1 | Tee-Object -FilePath build_errors.log

# Windows (Command Prompt)
build_windows.bat verbose > build_errors.log 2>&1

# Mac/Linux
./build_unix.sh verbose 2>&1 | tee build_errors.log
```

## Reading Build Errors

### Error Message Format

```
Source/Adastrea/Ships/SpaceshipDataAsset.cpp(142): error C2065: 'undeclaredVariable' : undeclared identifier
│                                          │        │           │
│                                          │        │           └─ Error description
│                                          │        └─ Error code
│                                          └─ Line number
└─ File path
```

### Common Error Patterns

#### 1. Missing Include

**Error**:
```
fatal error C1083: Cannot open include file: 'MyHeader.h': No such file or directory
```

**Quick Fix**:
1. Check file exists: `find Source -name "MyHeader.h"`
2. Verify include path in `.Build.cs`
3. Check case sensitivity (important on Linux/Mac)

#### 2. Unresolved Symbol

**Error**:
```
error LNK2019: unresolved external symbol "void MyFunction(void)"
```

**Quick Fix**:
1. Ensure function is implemented in .cpp file
2. Check module dependencies in `.Build.cs`
3. Verify UFUNCTION/UPROPERTY macros

#### 3. Syntax Error

**Error**:
```
error C2143: syntax error: missing ';' before '}'
```

**Quick Fix**:
1. Go to the line number in the error
2. Check for missing semicolons
3. Check for mismatched braces/parentheses

## Debugging Workflow

### Step 1: Run Initial Build

```bash
# Get the error
build_windows.bat verbose
```

### Step 2: Find First Error

Scroll up in output to find the **first** error (ignore warnings initially):
- Look for `error C` (MSVC) or `error:` (GCC/Clang)
- Ignore subsequent errors - they may be cascading from the first

### Step 3: Fix and Rebuild

```bash
# Quick rebuild of just modified module
build_windows.bat
```

Don't clean unless necessary - incremental builds are much faster.

### Step 4: Iterate

Repeat until build succeeds.

## Advanced Debugging

### Build Specific Module Only

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Development ^
  -Project="%CD%\Adastrea.uproject" ^
  -Module=Adastrea ^
  -Verbose

# Mac/Linux
"/Users/Shared/Epic Games/UE_5.6/Engine/Build/BatchFiles/Mac/Build.sh" \
  AdastreaEditor Mac Development \
  -Project="$(pwd)/Adastrea.uproject" \
  -Module=Adastrea \
  -Verbose
```

**Adastrea Modules**:
- `Adastrea` - Main game module
- `PlayerMods` - Player modifications module
- `StationEditor` - Station editor module

### Generate Compile Commands for Better IDE Support

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Development ^
  -Project="%CD%\Adastrea.uproject" ^
  -Mode=GenerateClangDatabase

# Mac/Linux
"/Users/Shared/Epic Games/UE_5.6/Engine/Build/BatchFiles/Mac/Build.sh" \
  AdastreaEditor Mac Development \
  -Project="$(pwd)/Adastrea.uproject" \
  -Mode=GenerateClangDatabase
```

This creates `compile_commands.json` for better code completion in VS Code, CLion, etc.

### Disable Unity Build for Better Error Isolation

Unity Build combines multiple .cpp files which can sometimes hide the real error location:

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AdastreaEditor Win64 Development ^
  -Project="%CD%\Adastrea.uproject" ^
  -DisableUnity ^
  -Verbose
```

## Common Build Issues

### Issue: "Access Denied" or File Locked

**Cause**: Unreal Editor or Visual Studio has locked the binaries.

**Solution**:
1. Close Unreal Editor
2. Close Visual Studio
3. Try build again

### Issue: "Out of Memory"

**Cause**: Too many parallel compilation processes.

**Solution**:
```bash
build_windows.bat verbose
# Then manually add -MaxParallelActions=4 to Build.bat command
```

### Issue: Build Succeeds but Editor Fails to Load

**Cause**: Module not registered in Target.cs files.

**Solution**:
1. Check `.uproject` for module name
2. Verify it's in `Source/Adastrea.Target.cs` `ExtraModuleNames`
3. See [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) Section 4

### Issue: "Engine not found"

**Solution**:
1. Edit build script and set correct UE_ROOT path
2. Or set environment variable:
   ```bash
   # Windows
   set UE_ROOT=C:\Path\To\UE_5.6
   
   # Mac/Linux
   export UE_ROOT="/Path/To/UE_5.6"
   ```

## Error Code Quick Reference

### MSVC (Windows) Error Codes

| Code | Meaning | Common Cause |
|------|---------|--------------|
| C1083 | Cannot open include file | Missing or wrong include path |
| C2065 | Undeclared identifier | Typo, missing include, or missing declaration |
| C2143 | Syntax error | Missing semicolon, brace, or parenthesis |
| C2248 | Cannot access private member | Trying to access private/protected member |
| C2664 | Cannot convert argument | Wrong parameter type |
| C4100 | Unreferenced formal parameter | Unused function parameter (warning) |
| LNK2019 | Unresolved external symbol | Missing implementation or library |

### GCC/Clang (Mac/Linux) Error Types

| Error | Meaning | Common Cause |
|-------|---------|--------------|
| `no such file or directory` | Missing include | Wrong include path or filename |
| `was not declared in this scope` | Undeclared identifier | Missing include or forward declaration |
| `expected ';' before` | Syntax error | Missing semicolon |
| `cannot access private member` | Access violation | Trying to access private/protected member |
| `undefined reference to` | Linker error | Missing implementation |

## Debugging Checklist

When you get build errors, work through this checklist:

- [ ] **Read the first error carefully** - Don't skip to later errors
- [ ] **Check the line number** - Go to exact location in source file
- [ ] **Check for typos** - Most common cause of errors
- [ ] **Check includes** - Make sure all headers are included
- [ ] **Check module dependencies** - Verify `.Build.cs` dependencies
- [ ] **Try clean build** - `build_windows.bat clean`
- [ ] **Check recent changes** - What did you change since last successful build?
- [ ] **Search error code** - Google the error code + "Unreal Engine"
- [ ] **Check BUILD_TROUBLESHOOTING.md** - Known issues and solutions

## Getting More Help

### 1. Save Verbose Build Log

```bash
# Windows
build_windows.bat verbose > build_log.txt 2>&1

# Mac/Linux
./build_unix.sh verbose > build_log.txt 2>&1
```

### 2. Search the Log

```bash
# Find all errors
grep -i "error" build_log.txt

# Find specific error code
grep "C2065" build_log.txt

# Find warnings (after fixing errors)
grep -i "warning" build_log.txt
```

### 3. Check Documentation

- [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) - Known build issues
- [UNREAL_BUILD_GUIDE.md](UNREAL_BUILD_GUIDE.md) - Complete build documentation
- [CODE_STYLE.md](CODE_STYLE.md) - Coding standards to prevent errors

### 4. External Resources

- [Unreal Engine Documentation](https://docs.unrealengine.com/5.6/en-US/)
- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [Unreal Slackers Discord](https://unrealslackers.org/)

## Tips for Faster Debugging

1. **Build incrementally** - Don't clean unless necessary
2. **Build one module at a time** - Use `-Module=ModuleName`
3. **Fix errors from top to bottom** - Later errors may be cascading
4. **Use verbose mode sparingly** - Only when you need detailed info
5. **Save logs** - Easier to search than scrolling console
6. **Check git diff** - Review what changed since last working build

## Examples

### Example 1: Debug Missing Include

```bash
# Get error
build_windows.bat verbose

# Error shows:
# Source/Adastrea/Ships/MyShip.cpp(10): error C1083: Cannot open include file: 'SpaceshipDataAsset.h'

# Check if file exists
find Source -name "SpaceshipDataAsset.h"
# Output: Source/Adastrea/Ships/SpaceshipDataAsset.h

# Fix: Update include in MyShip.cpp
# Wrong: #include "SpaceshipDataAsset.h"
# Right: #include "Ships/SpaceshipDataAsset.h"

# Rebuild
build_windows.bat
```

### Example 2: Debug Linker Error

```bash
# Get error
build_windows.bat verbose

# Error shows:
# error LNK2019: unresolved external symbol "void UMyClass::MyFunction(void)"

# Check .cpp file has implementation
cat Source/Adastrea/MyModule/MyClass.cpp | grep "MyFunction"

# If missing, add implementation:
# void UMyClass::MyFunction()
# {
#     // Implementation
# }

# Rebuild
build_windows.bat
```

### Example 3: Clean Build After Dependency Change

```bash
# Changed .Build.cs dependencies
# Need clean build
build_windows.bat clean verbose

# This will:
# 1. Delete Binaries, Intermediate, Saved
# 2. Rebuild everything from scratch
```

---

**Quick Reference**: This document focuses on error debugging. For complete build documentation, see [UNREAL_BUILD_GUIDE.md](UNREAL_BUILD_GUIDE.md).

**Maintained by**: Adastrea Development Team  
**Last Updated**: 2025-11-13  
**Version**: 1.0
