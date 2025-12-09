# Post-Merge Build Validation

This guide describes how to run comprehensive build error checks after merging to the main branch.

## Quick Start

Run the comprehensive build error detection after any merge to main:

```bash
# From project root
python tests/test_all_build_errors.py

# With verbose output
python tests/test_all_build_errors.py --verbose
```

## What It Checks

The comprehensive test (`test_all_build_errors.py`) performs the following checks:

### 1. Missing Include Files (Comprehensive)
- Searches multiple include paths
- Tests relative paths
- Checks module Public/Private directories
- Validates all #include directives

### 2. Circular Dependencies
- Analyzes Build.cs files
- Detects circular module dependencies
- Reports dependency chains

### 3. UPROPERTY on UObject Pointers
- Ensures all UObject*/AActor* member variables have UPROPERTY()
- Critical for garbage collection
- Prevents memory leaks and crashes

### 4. Hardcoded Secrets
- Scans for passwords, API keys, tokens
- Detects sensitive data in source files
- Security vulnerability prevention

### 5. CoreMinimal.h Includes
- Verifies .cpp files include CoreMinimal.h
- Unreal Engine requirement
- Ensures precompiled headers work correctly

### 6. API Export Macros
- Checks classes have proper MODULE_API macros
- Required for cross-module visibility
- Ensures proper DLL exports

### 7. Relative Include Path Validation
- Validates ../ paths actually resolve
- Catches broken relative includes
- Prevents "file not found" errors

## Usage Scenarios

### After Merging PR to Main

```bash
# 1. Checkout main branch
git checkout main
git pull origin main

# 2. Run comprehensive checks
python tests/test_all_build_errors.py

# 3. If errors found, create hotfix
git checkout -b hotfix/build-errors
# Fix the issues
# Commit and create PR
```

### Before Creating Release

```bash
# Run with verbose output for detailed diagnostics
python tests/test_all_build_errors.py --verbose

# Review all warnings even if tests pass
# Warnings may indicate potential future issues
```

### Regular Health Check

Schedule weekly or after major feature merges:

```bash
# Can be run as part of CI/CD pipeline
python tests/test_all_build_errors.py || exit 1
```

## Understanding Results

### Exit Codes
- `0` - All critical checks passed (warnings are OK)
- `1` - Critical errors found (must fix before release)

### Output Symbols
- ✅ `✓` - Check passed
- ❌ `✗ ERROR` - Critical issue (blocks release)
- ⚠️ `⚠ WARNING` - Non-critical issue (review recommended)
- ℹ️ `ℹ INFO` - Additional details

### Example Output

```
======================================================================
COMPREHENSIVE BUILD ERROR DETECTION
======================================================================
Project Root: /path/to/Adastrea
Verbose Mode: False

🔍 Checking include files (comprehensive)...
  ✓ All includes found

🔄 Checking for circular dependencies...
  ✓ No circular dependencies found

📦 Checking UPROPERTY on UObject pointers...
  ✓ All UObject pointers have UPROPERTY

🔐 Checking for hardcoded secrets...
  ✓ No hardcoded secrets found

📚 Checking CoreMinimal.h includes...
  ✓ All .cpp files include CoreMinimal.h

🔧 Checking API export macros...
  ✓ All classes have proper API macros

🔀 Checking relative include paths...
  ✓ All relative include paths are valid

======================================================================
RESULTS
======================================================================

✅ All critical checks passed!
```

## Comparison with Other Tests

### test_build_errors.py (Basic)
- Faster, less comprehensive
- Good for quick checks during development
- May have false positives

### test_all_build_errors.py (Comprehensive)
- More thorough searching
- Better at finding actual issues
- Fewer false positives
- Recommended for post-merge validation

### test_comprehensive.py (Full Suite)
- Includes all build checks plus more
- Checks documentation, assets, etc.
- Takes longer to run
- Best for pre-release validation

## Fixing Common Issues

### Missing Include Files

**Error:** `Source/Module/File.cpp:10 - missing 'SomeHeader.h'`

**Fix:**
1. Check if header actually exists
2. Verify include path is correct
3. Add module to Build.cs if needed
4. Use relative path (../) if in subdirectory

### Circular Dependencies

**Error:** `ModuleA <-> ModuleB`

**Fix:**
1. Review module dependencies in Build.cs
2. Move shared code to a common module
3. Use forward declarations instead of includes
4. Refactor to break dependency cycle

### Missing UPROPERTY

**Error:** `File.h:50 - UMyObject* MyPointer;`

**Fix:**
```cpp
// Before
UMyObject* MyPointer;

// After
UPROPERTY()
UMyObject* MyPointer;
```

### Hardcoded Secrets

**Error:** `File.cpp:20 - potential api key`

**Fix:**
1. Move sensitive data to config files
2. Use environment variables
3. Load from encrypted storage
4. Never commit secrets to git

### Missing CoreMinimal.h

**Error:** `File.cpp - missing CoreMinimal.h`

**Fix:**
```cpp
// Add as first include
#include "CoreMinimal.h"
#include "OtherHeaders.h"
```

### Broken Relative Path

**Error:** `File.h:5 - ../NonExistent.h`

**Fix:**
1. Verify file exists at that path
2. Check for typos in filename
3. Update path to correct location
4. Consider using module-relative path instead

## Automation

### GitHub Actions

Add to `.github/workflows/post-merge.yml`:

```yaml
name: Post-Merge Validation

on:
  push:
    branches: [ main ]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
        with:
          python-version: '3.11'
      - name: Run comprehensive build checks
        run: python tests/test_all_build_errors.py
```

### Pre-Release Script

Create `scripts/pre-release-check.sh`:

```bash
#!/bin/bash
set -e

echo "Running comprehensive build validation..."
python tests/test_all_build_errors.py --verbose

echo "Running full test suite..."
python tests/run_all_tests.py

echo "All checks passed! Ready for release."
```

## Troubleshooting

### Too Many False Positives

If you see many "missing includes" for engine headers:

1. Update the engine include prefix list in the script
2. Add new engine modules to the skip list
3. Report the issue with examples

### Test Takes Too Long

For large projects:

1. Use `--verbose` only when needed
2. Run on CI instead of locally
3. Split into module-specific tests

### Inconsistent Results

If results vary between runs:

1. Ensure clean git state
2. Check for IDE-generated files
3. Verify all files are committed
4. Clear Intermediate/Binaries folders

## Best Practices

1. **Run after every merge to main** - Catch issues immediately
2. **Fix errors before warnings** - Prioritize critical issues
3. **Review warnings regularly** - Prevent future problems
4. **Keep test updated** - Add checks for new patterns
5. **Document false positives** - Help improve the test

## Support

If you encounter issues:

1. Check this documentation
2. Run with `--verbose` for details
3. Review test output carefully
4. Search existing GitHub issues
5. Create new issue with test output

## Related Documentation

- `tests/README.md` - Overview of all tests
- `TESTING_COMPREHENSIVE.md` - Complete testing guide
- `CONTRIBUTING.md` - Contribution guidelines
- `BUILD_FIX_SUMMARY.md` - Recent build fixes

---

**Last Updated:** 2025-12-09  
**Test Version:** 1.0  
**Maintained by:** Adastrea Development Team
