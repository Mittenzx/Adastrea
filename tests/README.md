# Adastrea Test Suite

This directory contains all automated tests for the Adastrea project.

## Quick Start

```bash
# Run all tests (from project root)
python run_all_tests.py

# Run specific test suite
python tests/test_build_errors.py
pytest tests/test_comprehensive.py -v
```

## Test Files

### Core Test Suites

- **`test_build_errors.py`** - Standalone build error detection (no dependencies)
  - Checks Build.cs configuration
  - Validates filename conventions
  - Detects deprecated code usage
  - Verifies include directives
  - Checks header guards

- **`test_comprehensive.py`** - Comprehensive test suite (requires pytest)
  - Repository structure validation
  - Build configuration checks
  - Code quality analysis
  - Asset naming conventions
  - Module structure validation
  - Documentation checks
  - Integrates all existing tests

- **`test_procedural_generators.py`** - Tests for procedural generation
  - Name generation tests
  - Star/planet name generation
  - Validation of generator data

- **`test_schema_validator.py`** - Schema validation tests
  - JSON schema file validation
  - Schema structure checks
  - Import/export validation

### C++ Tests

- **`AdastreaAutomationTests.cpp`** - Unreal Engine automation tests
  - Data Asset validation
  - System component tests
  - Achievement/Quest system tests
  - Audio/Inventory system tests
  - Must be run in Unreal Editor

### Configuration Files

- **`requirements-test.txt`** - Python dependencies for testing
  ```
  pytest>=7.0.0
  pytest-cov>=4.0.0
  jsonschema>=4.0.0
  pyyaml>=6.0.0
  ```

## Running Tests

### All Tests

```bash
# From project root
python run_all_tests.py [--verbose] [--quick]
```

Options:
- `--verbose` - Show detailed output
- `--quick` - Skip slow validation checks
- `--python-only` - Run only Python tests
- `--validation-only` - Run only validation scripts

### Individual Test Suites

```bash
# Build error detection (standalone)
python tests/test_build_errors.py

# Comprehensive tests (requires pytest)
pytest tests/test_comprehensive.py -v

# Specific test file
pytest tests/test_procedural_generators.py -v

# Specific test class
pytest tests/test_comprehensive.py::TestBuildConfiguration -v

# Specific test function
pytest tests/test_comprehensive.py::TestCodeQuality::test_no_hardcoded_secrets -v
```

### C++ Automation Tests

**In Unreal Editor:**
1. Open Adastrea.uproject
2. Window > Developer Tools > Session Frontend
3. Automation tab
4. Filter for "Adastrea"
5. Run Tests

**Command Line:**
```bash
UnrealEditor-Cmd.exe Adastrea.uproject -ExecCmds="Automation RunTests Adastrea" -unattended
```

## Test Categories

### Build Validation
- Build.cs syntax and configuration
- Module dependencies
- Include directives
- Header guards
- Filename conventions

### Code Quality
- UPROPERTY usage validation
- Null safety checks
- Asset naming conventions
- Deprecated API detection
- Hardcoded secrets check

### Functionality
- Procedural generation
- Schema validation
- Data Asset validation
- System component validation

## CI/CD Integration

Tests run automatically via GitHub Actions on:
- Pull requests to main/develop
- Pushes to main/develop

Workflow: `.github/workflows/code-quality.yml`

## Adding New Tests

### Python Tests

1. Create `test_your_feature.py` in `tests/` directory
2. Import pytest: `import pytest`
3. Create test class: `class TestYourFeature:`
4. Add test methods: `def test_something(self):`

Example:
```python
import pytest

class TestMyFeature:
    def test_basic_functionality(self):
        """Test that feature works."""
        assert True
```

### C++ Automation Tests

1. Add to `AdastreaAutomationTests.cpp`
2. Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` macro
3. Follow naming: `FYourFeatureTest`
4. Category: `"Adastrea.Category.TestName"`

Example:
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyTest,
    "Adastrea.Systems.MySystem.BasicValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMyTest::RunTest(const FString& Parameters)
{
    // Test implementation
    return true;
}
```

## Troubleshooting

### pytest not found
```bash
pip install -r tests/requirements-test.txt
```

### Tests failing in CI
- Check GitHub Actions log
- Run locally: `python run_all_tests.py --verbose`
- Fix errors before pushing

### UPROPERTY warnings
- Review findings manually
- Function parameters don't need UPROPERTY
- Member variables DO need UPROPERTY

## Test Output

### Exit Codes
- `0` - All tests passed
- `1` - Some tests failed

### Symbols
- ✅ `✓ PASS` - Test passed
- ❌ `✗ FAIL` - Test failed  
- ⚠️ `⚠ WARNING` - Non-critical issue
- ℹ️ `ℹ INFO` - Informational message

## Documentation

For comprehensive testing documentation, see:
- **`TESTING_COMPREHENSIVE.md`** - Complete testing guide
- **`CONTRIBUTING.md`** - Contribution guidelines
- **`README.md`** - Project overview

## Support

Questions or issues:
1. Review test output for details
2. Check documentation
3. Search GitHub Issues
4. Create new issue with test output

---

**Note**: Some tests require pytest to be installed. Install with:
```bash
pip install -r tests/requirements-test.txt
```
