# Comprehensive Testing Guide for Adastrea

This guide documents the comprehensive test suite that checks for build errors, deprecated code, filename issues, and dependency problems across the entire repository.

## Overview

The Adastrea project includes multiple testing layers:

1. **Build Error Detection** - Checks for potential compilation issues
2. **Code Quality Validation** - UPROPERTY usage, naming conventions, null safety
3. **Python Tests** - Unit tests for Python automation scripts
4. **C++ Automation Tests** - Unreal Engine automation framework tests

## Quick Start

### Run All Tests

```bash
# Run all tests (requires pytest to be installed)
python run_all_tests.py --verbose

# Run only fast checks (skips slow validation)
python run_all_tests.py --quick

# Run only Python tests
python run_all_tests.py --python-only

# Run only validation scripts
python run_all_tests.py --validation-only
```

### Run Individual Test Suites

```bash
# Build error detection (standalone, no dependencies)
python tests/test_build_errors.py

# Comprehensive test suite (requires pytest)
pytest tests/test_comprehensive.py -v

# Specific test modules
pytest tests/test_procedural_generators.py -v
pytest tests/test_schema_validator.py -v
```

## Test Suites

### 1. Build Error Detection (`test_build_errors.py`)

**Purpose**: Detect potential build errors before compilation

**Checks**:
- ✅ Build.cs file syntax and structure
- ✅ Module dependency configuration
- ✅ Circular dependencies between modules
- ✅ Filename conventions (no spaces, valid extensions)
- ✅ Header include guards (#pragma once or #ifndef)
- ✅ .generated.h include order (must be last)
- ✅ Missing include files
- ✅ Deprecated API usage

**Run standalone**:
```bash
python tests/test_build_errors.py
```

**Exit codes**:
- `0` - All critical checks passed
- `1` - Errors found (build may fail)

### 2. Comprehensive Test Suite (`test_comprehensive.py`)

**Purpose**: Extensive validation of repository structure and code quality

**Test Classes**:

#### `TestRepositoryStructure`
- Validates .uproject file exists
- Checks Source and Content directories
- Verifies Build.cs files for all modules

#### `TestBuildConfiguration`
- Validates Build.cs syntax
- Checks for circular dependencies
- Ensures core UE modules are included

#### `TestDeprecatedCode`
- Scans for deprecated API usage
- Checks for old Unreal Engine APIs
- Reports deprecated patterns

#### `TestFileNaming`
- Validates file extensions
- Checks for spaces in filenames
- Verifies header/implementation pairs

#### `TestIncludeDirectives`
- Validates .generated.h is last include
- Checks for absolute path includes
- Ensures proper include order

#### `TestCodeQuality`
- Scans for hardcoded secrets
- Validates UPROPERTY usage
- Checks null safety patterns

#### `TestAssetNaming`
- Validates Blueprint naming (BP_ prefix)
- Checks Data Asset naming (DA_ prefix)
- Verifies widget naming (WBP_ prefix)

#### `TestModuleStructure`
- Checks Public/Private directory structure
- Validates header locations
- Ensures proper organization

#### `TestDocumentation`
- Verifies README.md exists
- Checks ARCHITECTURE.md
- Validates system documentation

#### `TestExistingTests`
- Runs procedural generator tests
- Runs schema validator tests
- Integrates existing test suites

**Run with pytest**:
```bash
pytest tests/test_comprehensive.py -v
```

### 3. Code Quality Validation Scripts

#### UPROPERTY Usage Check (`Tools/check_uproperty.py`)

Validates that all UObject* pointers have the UPROPERTY macro for garbage collection tracking.

```bash
python Tools/check_uproperty.py [--verbose]
```

**What it checks**:
- UClass*, AActor*, UObject* pointers in headers
- Missing UPROPERTY macros
- Proper garbage collection patterns

**Example output**:
```
⚠️  Potential Issues Found:
  Source/Adastrea/AI/NPCLogicBase.h
    Line 42: UDataAsset* CachedData;
```

#### Asset Naming Validation (`Tools/validate_naming.py`)

Validates asset naming conventions across the project.

```bash
python Tools/validate_naming.py
```

**Naming conventions enforced**:
- Blueprints: `BP_*`
- Data Assets: `DA_*`
- Materials: `M_*`, `MI_*`, `MF_*`
- Textures: `T_*`
- Static Meshes: `SM_*`
- Skeletal Meshes: `SK_*`
- Widgets: `WBP_*`, `W_*`

#### Null Safety Check (`Tools/check_null_safety.py`)

Scans C++ implementation files for pointer dereferences without null checks.

```bash
python Tools/check_null_safety.py [--verbose] [--export baseline.txt]
```

**What it checks**:
- Pointer dereferences (->)
- Missing IsValid() or null checks
- Unsafe pointer operations

### 4. C++ Automation Tests (`tests/AdastreaAutomationTests.cpp`)

Unreal Engine's built-in automation testing framework.

**Test categories**:
- Data Asset validation (Spaceship, Faction, Personnel)
- System validation (Achievement, Quest, Audio)
- Component validation (Inventory, Combat, Player Unlock)

**Run in Unreal Editor**:
1. Open Adastrea.uproject
2. Window > Developer Tools > Session Frontend
3. Go to Automation tab
4. Filter for "Adastrea"
5. Click "Run Tests"

**Run from command line**:
```bash
UnrealEditor-Cmd.exe Adastrea.uproject -ExecCmds="Automation RunTests Adastrea" -unattended
```

## CI/CD Integration

### GitHub Actions

The comprehensive test suite runs automatically on pull requests and pushes to main/develop branches.

**Workflow**: `.github/workflows/code-quality.yml`

**Steps**:
1. Checkout code
2. Setup Python 3.11
3. Install dependencies from `tests/requirements-test.txt`
4. Run UPROPERTY check
5. Run naming convention validation
6. Check for hardcoded secrets
7. Run comprehensive test suite

**Trigger events**:
- Pull requests to main/develop
- Pushes to main/develop

**View results**:
- Go to repository "Actions" tab
- Check latest workflow run
- Review test output and failures

## Test Configuration

### pytest Configuration (`pytest.ini`)

```ini
[pytest]
testpaths = tests
python_files = test_*.py
python_classes = Test*
python_functions = test_*
addopts = -v --tb=short
filterwarnings =
    ignore::DeprecationWarning
```

### Test Requirements (`tests/requirements-test.txt`)

```
pytest>=7.0.0
pytest-cov>=4.0.0
jsonschema>=4.0.0
pyyaml>=6.0.0
```

**Install dependencies**:
```bash
pip install -r tests/requirements-test.txt
```

## Adding New Tests

### Python Unit Tests

1. Create test file in `tests/` directory
2. Name file with `test_` prefix (e.g., `test_my_feature.py`)
3. Create test classes with `Test` prefix
4. Create test functions with `test_` prefix

**Example**:
```python
#!/usr/bin/env python3
"""Tests for my feature."""
import pytest

class TestMyFeature:
    """Tests for MyFeature class."""
    
    def test_basic_functionality(self):
        """Test that basic feature works."""
        assert True
    
    def test_edge_case(self):
        """Test edge case handling."""
        # Test implementation
        pass
```

### C++ Automation Tests

1. Add test to `tests/AdastreaAutomationTests.cpp`
2. Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` macro
3. Follow naming pattern: `F{FeatureName}Test`
4. Category: `"Adastrea.{Category}.{TestName}"`

**Example**:
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyFeatureTest,
    "Adastrea.Systems.MyFeature.BasicValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMyFeatureTest::RunTest(const FString& Parameters)
{
    // Test implementation
    UClass* MyClass = FindObject<UClass>(nullptr, TEXT("/Script/Adastrea.MyFeature"));
    TestNotNull(TEXT("MyFeature class should exist"), MyClass);
    
    return true;
}
```

### Code Quality Checks

1. Add new check function to `tests/test_build_errors.py`
2. Follow pattern of existing check functions
3. Use `add_error()`, `add_warning()`, `add_info()` for reporting

**Example**:
```python
def check_my_quality_metric(self):
    """Check for my quality metric."""
    print("\nChecking my quality metric...")
    source_dir = self.project_root / "Source"
    
    # Implementation
    issues_found = []
    
    if issues_found:
        self.add_warning(f"Found {len(issues_found)} issues")
    else:
        print("  ✓ No issues found")
```

## Troubleshooting

### pytest not found

**Error**: `No module named 'pytest'`

**Solution**:
```bash
pip install -r tests/requirements-test.txt
```

### Tests timing out

**Issue**: Tests take too long in CI

**Solution**: Use `--quick` flag to skip slow checks
```bash
python run_all_tests.py --quick
```

### False positives in include checks

**Issue**: Test reports missing includes that actually exist

**Reason**: Complex include path resolution

**Solution**: Review warnings manually - many are informational only

### UPROPERTY false positives

**Issue**: UPROPERTY checker reports false positives

**Reason**: Function parameters and forward declarations

**Solution**: Review findings manually - not all require UPROPERTY

## Best Practices

1. **Run tests before committing**
   ```bash
   python run_all_tests.py
   ```

2. **Fix critical errors first**
   - Address all "ERROR" messages
   - Review "WARNING" messages
   - "INFO" messages are informational

3. **Use quick mode for rapid iteration**
   ```bash
   python run_all_tests.py --quick
   ```

4. **Run specific test suites for focused work**
   ```bash
   python tests/test_build_errors.py  # Build checks only
   pytest tests/test_comprehensive.py::TestCodeQuality -v  # Specific class
   ```

5. **Review test output in CI**
   - Check GitHub Actions for automated results
   - Fix issues before merging PRs

## Continuous Improvement

### Adding New Checks

When adding new validation checks:

1. Determine if it should be an error or warning
2. Add to appropriate test suite
3. Update documentation
4. Test with existing codebase
5. Adjust thresholds to avoid false positives

### Updating Test Coverage

Regularly review:

- Code coverage metrics
- Areas without tests
- New features needing validation
- Deprecated checks to remove

### Performance Optimization

For slow tests:

1. Profile test execution time
2. Cache expensive operations
3. Add to "slow" test category
4. Skip in `--quick` mode

## Reference

### Exit Codes

- `0` - All tests passed
- `1` - Some tests failed
- `2` - Critical error in test runner

### Test Result Symbols

- ✅ `✓ PASS` - Test passed
- ❌ `✗ FAIL` - Test failed
- ⚠️ `⚠ WARNING` - Issue found (not critical)
- ℹ️ `ℹ INFO` - Informational message
- 🔍 `...` - Additional details available

### Common Issues and Fixes

| Issue | Cause | Fix |
|-------|-------|-----|
| Module depends on itself | Incorrect dependency parsing | Check Build.cs syntax |
| Missing include | Complex include paths | Review manually |
| Deprecated API | Old Unreal API usage | Update to modern API |
| No header guard | Missing #pragma once | Add to header file |
| Hardcoded secret | Password in source | Move to config |

## Support

For issues with the test suite:

1. Check this documentation first
2. Review test output for details
3. Check GitHub Issues for known problems
4. Create new issue with test output attached

---

**Last Updated**: 2025-12-09  
**Version**: 1.0  
**Maintained by**: Adastrea Development Team
