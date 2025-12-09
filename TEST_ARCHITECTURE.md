# Test Suite Architecture

This document provides a visual overview of the comprehensive test suite architecture for the Adastrea project.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     Adastrea Test Suite                         │
│                   (run_all_tests.py)                            │
└─────────────────────────┬───────────────────────────────────────┘
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
        ▼                 ▼                 ▼
┌───────────────┐ ┌──────────────┐ ┌──────────────────┐
│   Build       │ │  Python      │ │   Code Quality   │
│Configuration  │ │  Tests       │ │   Validation     │
└───────────────┘ └──────────────┘ └──────────────────┘
        │                 │                 │
        │                 │                 │
        ▼                 ▼                 ▼
┌───────────────┐ ┌──────────────┐ ┌──────────────────┐
│ .uproject     │ │test_build_   │ │check_uproperty.py│
│ Build.cs      │ │  errors.py   │ │validate_naming.py│
│ Documentation │ │              │ │check_null_safety │
└───────────────┘ │test_compre-  │ └──────────────────┘
                  │ hensive.py   │
                  │              │
                  │test_procedur-│
                  │ al_generat...│
                  │              │
                  │test_schema_  │
                  │  validator.py│
                  └──────────────┘
```

## Test Suite Components

### 1. Unified Test Runner (`run_all_tests.py`)

**Purpose**: Central orchestrator for all tests

```
tests/run_all_tests.py
├── Build Configuration Check
│   ├── Validate .uproject file
│   ├── Check Build.cs files
│   └── Verify module structure
│
├── Documentation Check
│   ├── README.md
│   ├── ARCHITECTURE.md
│   ├── CODE_STYLE.md
│   └── CONTRIBUTING.md
│
├── Python Tests
│   ├── Standalone Tests (no dependencies)
│   │   └── test_build_errors.py
│   │
│   └── Pytest Tests (requires pytest)
│       ├── test_comprehensive.py
│       ├── test_procedural_generators.py
│       └── test_schema_validator.py
│
├── Code Quality Validation
│   ├── UPROPERTY Usage Check
│   ├── Asset Naming Convention Check
│   └── Null Safety Check
│
└── C++ Automation Tests Info
    └── AdastreaAutomationTests.cpp
```

### 2. Build Error Detection (`test_build_errors.py`)

**Type**: Standalone (no dependencies required)

```
test_build_errors.py
├── check_build_cs_dependencies()
│   ├── Validate Build.cs syntax
│   ├── Extract module dependencies
│   ├── Detect circular dependencies
│   └── Check for self-dependencies
│
├── check_filename_conventions()
│   ├── No spaces in filenames
│   └── Valid file extensions
│
├── check_header_guards()
│   ├── #pragma once
│   └── #ifndef guards
│
├── check_generated_h_includes()
│   └── .generated.h is last include
│
├── check_include_files()
│   └── Detect missing includes
│
└── check_deprecated_code()
    ├── UE_DEPRECATED
    ├── DEPRECATED_FORGAME
    └── Old Unreal APIs
```

### 3. Comprehensive Test Suite (`test_comprehensive.py`)

**Type**: Pytest-based (requires pytest)

```
test_comprehensive.py
├── TestRepositoryStructure
│   ├── test_uproject_exists
│   ├── test_source_directory_exists
│   ├── test_content_directory_exists
│   └── test_build_cs_files_exist
│
├── TestBuildConfiguration
│   ├── test_all_build_cs_files_valid_syntax
│   ├── test_no_circular_dependencies
│   └── test_core_modules_present
│
├── TestDeprecatedCode
│   ├── test_scan_for_deprecated_apis
│   └── test_check_for_old_unreal_apis
│
├── TestFileNaming
│   ├── test_source_files_proper_extensions
│   ├── test_no_spaces_in_source_filenames
│   └── test_header_cpp_pairs_exist
│
├── TestIncludeDirectives
│   ├── test_generated_h_include_last
│   └── test_no_absolute_includes
│
├── TestCodeQuality
│   ├── test_no_hardcoded_secrets
│   └── test_proper_uproperty_usage
│
├── TestAssetNaming
│   └── test_blueprint_naming_conventions
│
├── TestModuleStructure
│   ├── test_all_modules_have_public_private_dirs
│   └── test_headers_in_public_implementations_in_private
│
├── TestDocumentation
│   ├── test_readme_exists
│   ├── test_architecture_doc_exists
│   └── test_all_systems_have_guides
│
└── TestExistingTests
    ├── test_run_procedural_generator_tests
    └── test_run_schema_validator_tests
```

### 4. Code Quality Validation Tools

```
Tools/
├── check_uproperty.py
│   ├── Scan for UObject* pointers
│   ├── Validate UPROPERTY usage
│   └── Report missing macros
│
├── validate_naming.py
│   ├── Check Blueprint prefixes (BP_)
│   ├── Check Data Asset prefixes (DA_)
│   ├── Check Widget prefixes (WBP_)
│   └── Validate folder structure
│
└── check_null_safety.py
    ├── Detect pointer dereferences
    ├── Check for null checks
    └── Validate IsValid() usage
```

## Test Execution Flow

```
┌─────────────────────┐
│  User runs:         │
│  tests/run_all_tests.py   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Parse Arguments     │
│ --verbose           │
│ --quick             │
│ --timeout N         │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Initialize          │
│ TestRunner          │
└──────────┬──────────┘
           │
           ├──────────────────────────────────┐
           │                                  │
           ▼                                  ▼
┌─────────────────────┐           ┌─────────────────────┐
│ Run Config Checks   │           │ Run Python Tests    │
│ - .uproject        │           │ - Standalone        │
│ - Build.cs         │           │ - Pytest (if avail.)│
│ - Documentation    │           └──────────┬──────────┘
└──────────┬──────────┘                     │
           │                                 │
           │                                 ▼
           │                      ┌─────────────────────┐
           │                      │ Run Validation      │
           │                      │ Scripts             │
           │                      │ - UPROPERTY         │
           │                      │ - Naming            │
           │                      │ - Null Safety       │
           │                      └──────────┬──────────┘
           │                                 │
           └─────────┬───────────────────────┘
                     │
                     ▼
           ┌─────────────────────┐
           │ Collect Results     │
           │ - Pass/Fail counts  │
           │ - Detailed output   │
           └──────────┬──────────┘
                      │
                      ▼
           ┌─────────────────────┐
           │ Print Summary       │
           │ - Test results      │
           │ - Exit code         │
           └─────────────────────┘
```

## CI/CD Integration

```
┌──────────────────────────────────────────────────────────┐
│             GitHub Actions Workflow                       │
│         (.github/workflows/code-quality.yml)             │
└───────────────────────┬──────────────────────────────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Trigger on:           │
            │ - Pull Requests       │
            │ - Push to main/develop│
            └───────────┬───────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Setup Environment     │
            │ - Checkout code       │
            │ - Setup Python 3.11   │
            └───────────┬───────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Install Dependencies  │
            │ pip install -r tests/ │
            │   requirements-test.txt│
            └───────────┬───────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Run Individual Checks │
            │ - UPROPERTY           │
            │ - Naming              │
            │ - Hardcoded secrets   │
            └───────────┬───────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Run Comprehensive     │
            │ Test Suite            │
            │ python tests/run_all_tests.py│
            └───────────┬───────────┘
                        │
                        ▼
            ┌───────────────────────┐
            │ Report Results        │
            │ - Pass/Fail status    │
            │ - Detailed logs       │
            └───────────────────────┘
```

## Test Categories

### Category 1: Critical (Must Pass)
- Build configuration validity
- No circular dependencies
- No hardcoded secrets
- Documentation exists

### Category 2: Important (Should Pass)
- UPROPERTY usage (some false positives OK)
- Naming conventions
- Header guards
- Include order

### Category 3: Informational (Advisory)
- Deprecated code detection
- Null safety warnings
- Module structure suggestions
- Missing includes (many false positives)

## File Organization

```
Adastrea/
│
├── tests/
│   ├── run_all_tests.py          # Main test runner
│   ├── README.md                 # Quick reference
│   ├── requirements-test.txt     # Dependencies
│   ├── test_build_errors.py      # Standalone tests
│   ├── test_comprehensive.py     # Pytest tests
│   ├── test_procedural_generators.py
│   ├── test_schema_validator.py
│   └── AdastreaAutomationTests.cpp
│
├── Tools/
│   ├── check_uproperty.py
│   ├── validate_naming.py
│   └── check_null_safety.py
│
├── TESTING_COMPREHENSIVE.md      # Complete guide
├── TEST_SUITE_SUMMARY.md         # Implementation summary
└── TEST_ARCHITECTURE.md          # This file
```

## Execution Modes

### 1. Full Mode (Default)
```bash
python tests/run_all_tests.py
```
- All checks enabled
- All test suites
- Full validation
- ~60 seconds

### 2. Quick Mode
```bash
python tests/run_all_tests.py --quick
```
- Fast checks only
- Skips slow validation
- Core tests only
- ~20 seconds

### 3. Python Only
```bash
python tests/run_all_tests.py --python-only
```
- Only Python test suites
- No validation scripts
- Good for dev iteration

### 4. Validation Only
```bash
python tests/run_all_tests.py --validation-only
```
- Only validation scripts
- No pytest tests
- Good for code quality checks

### 5. Standalone
```bash
python tests/test_build_errors.py
```
- No dependencies required
- Core build checks only
- Fastest option
- ~5 seconds

## Test Output Format

```
======================================================================
ADASTREA COMPREHENSIVE TEST SUITE
======================================================================
Project Root: /path/to/Adastrea
Python Version: 3.x.x

[Build Configuration Checks]
[Documentation Checks]
[Python Tests]
[Code Quality Validation]
[C++ Automation Tests Info]

======================================================================
TEST RESULTS SUMMARY
======================================================================

Total Test Suites: N
  ✓ Passed: X
  ✗ Failed: Y
  ℹ Info Only: Z

Detailed Results:
  ✓ PASS: Test Name 1
  ✗ FAIL: Test Name 2
  ℹ INFO: Test Name 3
```

## Integration Points

### With Existing Tests
- Runs existing pytest tests
- Integrates with C++ automation tests
- Calls validation scripts
- Preserves existing test configurations

### With CI/CD
- GitHub Actions compatible
- Configurable timeout
- Clear exit codes
- Detailed logging

### With Development Workflow
- Pre-commit hook compatible
- IDE integration ready
- Quick iteration support
- Detailed error messages

## Extension Points

To add new test categories:

1. **Add to `test_comprehensive.py`**:
   ```python
   class TestNewCategory:
       def test_new_check(self):
           # Implementation
   ```

2. **Add to `test_build_errors.py`**:
   ```python
   def check_new_validation(self):
       # Implementation
   ```

3. **Add to `run_all_tests.py`**:
   ```python
   def run_new_category(self):
       # Integration
   ```

4. **Update documentation**:
   - TESTING_COMPREHENSIVE.md
   - tests/README.md
   - This file

---

**Version**: 1.0  
**Last Updated**: 2025-12-09  
**Status**: Production Ready ✅
