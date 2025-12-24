# Adastrea Development Tools

## Overview

The `/Tools/` directory contains validation scripts and development utilities for the Adastrea project. These tools help maintain code quality, catch common errors, and enforce project standards.

## Tool Categories

### Core Validation Tools
- `check_uproperty.py` - UPROPERTY macro validation for GC tracking
- `check_null_safety.py` - Null pointer safety checks
- `validate_naming.py` - Naming convention enforcement
- `SchemaValidator.py` - YAML/JSON schema validation
- `AdastreaAssetValidator.py` - Unreal asset validation
- `ContentValidator.py` - Content validation framework

### Data Management Tools
- `YAMLtoDataAsset.py` - YAML to Unreal Data Asset importer
- `GoogleSheetsIntegration.py` - Export/import via Google Sheets
- `CSVIntegration.py` - CSV export/import functionality

### Testing & Analysis Tools
- `SmokeTest.py` - Comprehensive smoke testing
- `ScreenshotTester.py` - Visual regression testing
- `AutomationRunner.py` - Test automation runner
- `analyze_project.py` - Project structure analysis

### Setup & Configuration Tools
- `SetupCheck.py` / `SetupCheck.sh` - Development environment validation
- `CheckGeneratorEnvironment.py` - Generator environment diagnostics
- `check_input_config.py` - Input configuration checker
- `check_station_editor.py` - Station editor validation

### Content Organization Tools
- `organize_content.py` - Automated content organization

## Validation Scripts

### check_uproperty.py
**Purpose**: Validates that all UObject* pointers have the UPROPERTY() macro for garbage collection tracking.

**Why It's Important**:
- UObject pointers without UPROPERTY() are invisible to Unreal's garbage collector
- Can cause memory leaks and crashes
- Critical for game stability

**Usage**:
```bash
# Basic scan
python Tools/check_uproperty.py

# Verbose output with detailed analysis
python Tools/check_uproperty.py --verbose
```

**What It Checks**:
- UObject* pointers (UClass*, UComponent*, etc.)
- AActor* pointers
- Common pointer patterns that need UPROPERTY

**Example Output**:
```
=== UPROPERTY Validation Report ===
Found 3 potential issues in 2 files:

Source/Adastrea/Combat/WeaponComponent.h:45
  UProjectile* CurrentProjectile;
  ^ Missing UPROPERTY() macro

Total files scanned: 156
Files with issues: 2
```

**When to Run**:
- Before committing C++ changes
- After adding new UObject pointer members
- As part of CI/CD pipeline (automated)

---

### check_null_safety.py
**Purpose**: Checks for potential null pointer dereferences and unsafe pointer usage.

**Why It's Important**:
- Null pointer crashes are common in C++
- Prevents runtime crashes
- Improves code reliability

**Usage**:
```bash
# Scan all source files
python Tools/check_null_safety.py

# Check specific file
python Tools/check_null_safety.py Source/Adastrea/Combat/WeaponComponent.cpp
```

**What It Checks**:
- Pointer dereferences without null checks
- IsValid() usage for Actors
- Null checks before pointer operations

**Example Issues Detected**:
```cpp
// Bad: No null check
void UWeaponComponent::FireWeapon()
{
    CurrentTarget->TakeDamage(...);  // ❌ Crash if CurrentTarget is null
}

// Good: Proper null check
void UWeaponComponent::FireWeapon()
{
    if (IsValid(CurrentTarget))
    {
        CurrentTarget->TakeDamage(...);  // ✅ Safe
    }
}
```

**When to Run**:
- Before committing C++ code
- After modifying pointer-heavy code
- When debugging crashes

---

### validate_naming.py
**Purpose**: Enforces Unreal Engine and project naming conventions.

**Why It's Important**:
- Consistent naming improves code readability
- Follows Unreal Engine standards
- Makes code easier to maintain

**Usage**:
```bash
# Validate all source files
python Tools/validate_naming.py

# Check specific directory
python Tools/validate_naming.py Source/Adastrea/Combat/
```

**What It Checks**:
- Class prefixes (U for UObject, A for AActor, F for structs)
- Variable naming (PascalCase for members, bPrefix for booleans)
- Function naming (PascalCase, verb-noun patterns)
- File naming (matches class name)

**Naming Rules Enforced**:

| Type | Prefix | Example | Notes |
|------|--------|---------|-------|
| UObject class | U | UWeaponComponent | Subclass of UObject |
| AActor class | A | ASpaceship | Subclass of AActor |
| Struct | F | FWeaponStats | Plain structs |
| Enum | E | EWeaponType | Enumerations |
| Interface (class) | I | ITargetable | Interface implementations |
| Boolean property | b | bIsActive | Always prefix with 'b' |
| Member variable | - | MaxAmmo | PascalCase |
| Function | - | FireWeapon() | PascalCase, verb-noun |

**Example Issues**:
```cpp
// ❌ Bad naming
class WeaponComponent : public UActorComponent { };  // Missing U prefix
bool isActive;  // Should be bIsActive
void fire();    // Should be Fire() or FireWeapon()

// ✅ Good naming
class UWeaponComponent : public UActorComponent { };
bool bIsActive;
void FireWeapon();
```

**When to Run**:
- Before committing new classes
- After refactoring
- During code review

---

### organize_content.py
**Purpose**: Organizes Unreal content files and maintains directory structure.

**Why It's Important**:
- Keeps Content Browser organized
- Enforces project structure
- Prevents asset clutter

**Usage**:
```bash
# Organize all content
python Tools/organize_content.py

# Dry run (show what would change)
python Tools/organize_content.py --dry-run

# Organize specific directory
python Tools/organize_content.py Content/DataAssets/
```

**What It Does**:
- Moves misplaced assets to correct directories
- Validates asset naming conventions
- Reports duplicate assets
- Checks for unused assets

**Directory Structure**:
```
Content/
├── Blueprints/
│   ├── GameModes/
│   ├── Ships/
│   └── UI/
├── DataAssets/
│   ├── Ships/
│   ├── Personnel/
│   ├── Weapons/
│   └── Quests/
├── Materials/
├── Textures/
└── Fab/
    └── [Marketplace assets]
```

**When to Run**:
- After importing new assets
- Before committing Content changes
- Periodically to maintain organization

---

### SchemaValidator.py
**Purpose**: Validates YAML/JSON data files against schema definitions.

**Why It's Important**:
- Ensures data integrity before import
- Catches configuration errors early
- Validates game content structure

**Usage**:
```bash
# Validate single file
python Tools/SchemaValidator.py --schema spaceship --data Assets/SpaceshipTemplates/Scout_Pathfinder.yaml

# Batch validation
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates

# Generate report
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation_report.txt
```

**When to Run**:
- Before importing YAML templates
- After editing data templates
- As part of content validation pipeline

---

### YAMLtoDataAsset.py
**Purpose**: Import YAML templates as Unreal Engine Data Assets.

**Why It's Important**:
- Enables data-driven content creation
- Batch imports game content
- Works within Unreal Editor Python environment

**Usage** (in Unreal Editor):
```python
# Run in Unreal Editor Python Console
import YAMLtoDataAsset
YAMLtoDataAsset.import_spaceship("Assets/SpaceshipTemplates/Scout_Pathfinder.yaml")
```

**When to Run**:
- After validating YAML files with SchemaValidator
- When importing new game content
- For batch content creation

---

### GoogleSheetsIntegration.py
**Purpose**: Export/import game data via Google Sheets for collaborative editing.

**Usage**:
```bash
# Export spaceships
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# Import from Google Sheets
python Tools/GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
```

**When to Run**:
- When collaborating on game data with non-technical team members
- For bulk editing of game content

---

### CSVIntegration.py
**Purpose**: Export/import game data via CSV files.

**Usage**:
```bash
# Export to CSV
python Tools/CSVIntegration.py export --type spaceship --output spaceships.csv

# Import from CSV
python Tools/CSVIntegration.py import --type spaceship --input spaceships.csv
```

**When to Run**:
- For data manipulation in spreadsheet tools
- When bulk editing game content

---

### AdastreaAssetValidator.py
**Purpose**: Comprehensive Unreal Editor asset validation.

**Usage** (in Unreal Editor):
```python
# Run in Unreal Editor Python Console
import AdastreaAssetValidator
AdastreaAssetValidator.validate_all_assets()
```

**What It Checks**:
- Asset naming conventions
- Data Asset required fields
- Blueprint complexity
- Material and texture specifications

**When to Run**:
- Before committing Content changes
- After creating new assets
- During quality assurance

---

### ContentValidator.py
**Purpose**: Validates generated game content for correctness.

**Usage** (in Unreal Editor):
```python
import ContentValidator
ContentValidator.validate_all()
```

**What It Checks**:
- Blueprint existence and compilation
- Data Asset properties
- Input system configuration
- UI widgets

**When to Run**:
- After content generation
- Before playtesting
- During QA passes

---

### SmokeTest.py
**Purpose**: Comprehensive smoke testing for all maps and levels.

**Usage** (in Unreal Editor):
```python
import SmokeTest
tester = SmokeTest.SmokeTestRunner()
tester.run_all_tests()
```

**What It Checks**:
- Loads every map/level
- Detects missing assets
- Captures warnings and errors
- Generates detailed reports

**When to Run**:
- Before major releases
- After significant Content changes
- During QA testing

---

### ScreenshotTester.py
**Purpose**: Automated screenshot capture and visual regression testing.

**Usage** (in Unreal Editor):
```python
import ScreenshotTester
tester = ScreenshotTester.ScreenshotTestRunner()
tester.capture_screenshots()
```

**What It Checks**:
- Visual regressions
- Screenshot comparison against golden masters
- Pixel-based difference detection

**When to Run**:
- Before visual updates
- During QA testing
- To establish visual baselines

---

### AutomationRunner.py
**Purpose**: Test automation orchestration.

**Usage**:
```bash
python Tools/AutomationRunner.py --suite full
```

**When to Run**:
- In CI/CD pipelines
- For comprehensive testing
- Before releases

---

### SetupCheck.py / SetupCheck.sh
**Purpose**: Validates development environment setup.

**Usage**:
```bash
# Python version (cross-platform)
python Tools/SetupCheck.py

# Shell version (Linux/Mac)
./Tools/SetupCheck.sh

# With analysis
python Tools/SetupCheck.py --analyze
```

**What It Checks**:
- Unreal Engine installation
- C++ compiler availability
- Project structure
- Required dependencies

**When to Run**:
- Initial project setup
- After environment changes
- When troubleshooting build issues

---

### CheckGeneratorEnvironment.py
**Purpose**: Verifies content generator environment readiness.

**Usage**:
```bash
python Tools/CheckGeneratorEnvironment.py
```

**What It Checks**:
- Python dependencies
- Unreal Python plugin
- Generator prerequisites

**When to Run**:
- Before running content generators
- When debugging generator issues

---

### analyze_project.py
**Purpose**: Comprehensive project structure and compilation analysis.

**Usage**:
```bash
python Tools/analyze_project.py
```

**What It Provides**:
- Project structure overview
- Compilation status
- Improvement recommendations

**When to Run**:
- Project health checks
- Architecture reviews
- Planning refactoring

---

### check_input_config.py
**Purpose**: Validates Enhanced Input system configuration.

**Usage**:
```bash
python Tools/check_input_config.py
```

**When to Run**:
- After modifying input mappings
- When debugging input issues

---

### check_station_editor.py
**Purpose**: Validates Station Editor system.

**Usage**:
```bash
python Tools/check_station_editor.py
```

**When to Run**:
- After station system changes
- When debugging station issues

---

## Integration with CI/CD

These tools are automatically run in the CI/CD pipeline (see `.github/workflows/code-quality.yml`):

```yaml
# Automated checks on every PR
- check_uproperty.py
- check_null_safety.py
- validate_naming.py
```

If any tool reports errors, the CI build fails and requires fixes before merging.

## Running All Validation Tools

To run all validation tools at once:

```bash
# Run from your project root directory
cd <project_root>  # Change to your local Adastrea project root

# Run all tools
python Tools/check_uproperty.py --verbose && \
python Tools/check_null_safety.py && \
python Tools/validate_naming.py
```

Or use the provided script (if available):
```bash
./Tools/run_all_validations.sh
```

## Best Practices

1. **Run Before Committing**
   - Always run relevant validation scripts before committing
   - Fix all reported issues before pushing

2. **CI Integration**
   - Tools run automatically in CI
   - Don't ignore CI failures

3. **Regular Maintenance**
   - Run organize_content.py weekly
   - Run all validation tools before releases

4. **Clean Code**
   - Address warnings, not just errors
   - Use tools to learn best practices

## Troubleshooting

### Tool Reports False Positives

Some patterns may trigger false positives:

**For check_uproperty.py**:
- Forward declarations are excluded automatically
- Function parameters are excluded
- If you have a legitimate reason not to use UPROPERTY, add a comment explaining why

**For check_null_safety.py**:
- Some code paths guarantee non-null (e.g., in constructors)
- Add assertion or comment to document the guarantee

**For validate_naming.py**:
- Some third-party code may not follow conventions
- Exclude third-party directories if needed

### Tool Won't Run

1. **Check Python version**: Requires Python 3.7+
   ```bash
   python --version
   ```

2. **Check you're in project root**:
   ```bash
   pwd  # Should show /path/to/Adastrea
   ```

3. **Check file paths**:
   ```bash
   ls Tools/check_uproperty.py  # Should exist
   ```

### Understanding Results

Each tool provides:
- **File path**: Where the issue is
- **Line number**: Exact location
- **Issue description**: What's wrong
- **Suggestion**: How to fix it (when applicable)

## Contributing

### Adding New Validation Tools

1. Create script in `/Tools/` directory
2. Follow existing script structure:
   - Docstring at top
   - Command-line argument parsing
   - Clear error messages
   - Return non-zero exit code on errors

3. Add to CI pipeline (`.github/workflows/code-quality.yml`)
4. Document in this README

### Improving Existing Tools

1. Test changes thoroughly
2. Update documentation
3. Ensure backward compatibility
4. Add examples of new patterns detected

## Related Documentation

- `.github/instructions/` - Project coding standards
- `CODE_STYLE.md` - Detailed style guide
- `CONTRIBUTING.md` - Contribution guidelines
- `COMPLETE_REVIEW_SUMMARY.md` - Code review findings

---

## Quick Reference

| Tool | Primary Purpose | Run When |
|------|----------------|----------|
| check_uproperty.py | GC tracking validation | Before committing C++ |
| check_null_safety.py | Null pointer safety | Before committing C++ |
| validate_naming.py | Naming conventions | Before committing new classes |
| organize_content.py | Content organization | After importing assets |

---

**Location**: `/Tools/`  
**Maintained By**: Adastrea Development Team  
**Last Updated**: 2025-12-20  
**Status**: Active - All tools ready for use

## Migration Note

**Previous Duplicate Files Removed**:
- ❌ `/check_uproperty.py` (duplicate - removed)
- ❌ `/check_uproperty_final.py` (duplicate - removed)
- ❌ `/check_uproperty_fixed.py` (duplicate - removed)
- ✅ `/Tools/check_uproperty.py` (kept - most comprehensive)

All validation tools are now consolidated in the `/Tools/` directory for consistency.
