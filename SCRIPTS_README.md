# Adastrea Setup and Validation Scripts

This directory contains automated scripts to validate your development environment, project setup, and generate templates and documentation for the Adastrea Unreal Engine project.

## Overview

### Validation Scripts

Three scripts are provided to ensure your development environment is properly configured:

1. **SetupCheck.sh** - Shell script for Linux/Mac/WSL
2. **SetupCheck.py** - Cross-platform Python script
3. **AdastreaAssetValidator.py** - Unreal Editor Python script for asset validation

### Generator Scripts

Three powerful scripts for creating templates and documentation:

4. **TemplateGenerator.py** - Generate YAML templates for game systems
5. **GuideGenerator.py** - Generate Markdown documentation guides
6. **EditorUtilities.py** - Unified interface for Unreal Editor

> **Note**: For detailed documentation on the generator scripts, see [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md)

## SetupCheck.sh (Shell Script)

### Purpose
Validates the development environment on Linux, Mac, and Windows Subsystem for Linux (WSL).

### Requirements
- Bash shell
- Standard Unix tools (grep, find, etc.)

### Usage

```bash
# Basic validation
./SetupCheck.sh

# With build folder cleanup
./SetupCheck.sh --clean

# With static analysis (requires cppcheck)
./SetupCheck.sh --analyze

# Display help
./SetupCheck.sh --help
```

### What It Checks

1. **Unreal Engine Version** - Verifies .uproject file specifies Unreal Engine 5.5
2. **C++ Compiler** - Checks for g++ or clang++ availability
3. **Required Folders** - Validates existence of Source, Config, Content
4. **Required Files** - Checks for .uproject, module headers, config files
5. **Documentation** - Verifies key documentation files exist
6. **Clean Build** (optional) - Removes Intermediate, Saved, Binaries folders with confirmation
7. **Static Analysis** (optional) - Runs cppcheck if available

### Exit Codes

- `0` - All checks passed or only warnings
- `1` - Critical errors found (missing prerequisites)

### Example Output

```
========================================
Adastrea Pre-Build Setup Check
========================================
Checking development environment...

========================================
1. Checking Unreal Engine Version
========================================
✓ Required Unreal Engine version: 5.5

========================================
2. Checking C++ Compiler
========================================
✓ g++ compiler found: g++ (Ubuntu 13.3.0) 13.3.0

...

========================================
Setup Check Summary
========================================
✓ All checks passed! Your environment is ready for building.
```

## SetupCheck.py (Python Script)

### Purpose
Cross-platform validation script that works on Windows, Linux, and Mac.

### Requirements
- Python 3.6 or higher
- No external dependencies (uses only standard library)

### Usage

```bash
# Basic validation
python SetupCheck.py

# With static analysis (requires cppcheck)
python SetupCheck.py --analyze

# Verbose output
python SetupCheck.py --verbose

# Display help
python SetupCheck.py --help
```

### What It Checks

1. **Platform Detection** - Identifies OS and Python version
2. **Unreal Engine Version** - Parses .uproject JSON for engine version
3. **C++ Compilers** - Platform-specific compiler detection (MSVC/GCC/Clang)
4. **Project Structure** - Validates folders and files
5. **Documentation** - Checks for essential documentation
6. **Static Analysis** (optional) - Runs cppcheck/clang-tidy if available

### Features

- **Colored Output** - Uses ANSI colors on supported terminals
- **Cross-Platform** - Works on Windows, Linux, Mac
- **JSON Parsing** - Reads .uproject file safely
- **Graceful Degradation** - Handles missing tools without errors

### Example Output

```
============================================================
Adastrea Pre-Build Setup Check (Python)
Platform: Linux 6.11.0-1018-azure
Python: 3.12.3
============================================================

============================================================
1. Checking Unreal Engine Version
============================================================
✓ Required Unreal Engine version: 5.5

...

============================================================
Setup Check Summary
============================================================
✓ All checks passed! Your environment is ready for building.
```

## AdastreaAssetValidator.py (Unreal Editor Script)

### Purpose
Validates Data Assets and Blueprints from within the Unreal Editor using the Python Editor Script Plugin.

### Requirements
- Unreal Engine 5.5 with Python Editor Script Plugin enabled
- Must be run inside Unreal Editor, not from command line

### Setup

1. **Enable Python Plugin**:
   - Edit → Plugins
   - Search for "Python Editor Script Plugin"
   - Enable and restart Unreal Editor

2. **Run Script**:
   - Open Output Log (Window → Developer Tools → Output Log)
   - Open Python Console (Tools → Python → Execute Python Script)
   - Load and run AdastreaAssetValidator.py

### Usage

```python
# From Unreal Editor Python Console
import AdastreaAssetValidator
AdastreaAssetValidator.run_asset_validation()
```

### What It Validates (Stub Implementation)

The current version is a **stub/template** with placeholder validation methods. Extend these for your needs:

- `validate_faction_data_assets()` - Validate FactionDataAsset instances
- `validate_spaceship_data_assets()` - Validate SpaceshipDataAsset instances
- `validate_personnel_data_assets()` - Validate PersonnelDataAsset instances
- `validate_blueprint_assets()` - Validate Blueprint classes

### Extending the Validator

To add custom validation:

1. Modify the validation methods in AdastreaAssetValidator.py
2. Use `unreal.AssetRegistryHelpers.get_asset_registry()` to find assets
3. Use `unreal.load_asset(asset_path)` to load Data Assets
4. Access properties and validate values
5. Use `self.log_error()`, `self.log_warning()`, `self.log_info()` for output

### Example Custom Validation

```python
def validate_faction_data_assets(self):
    """Validate all FactionDataAsset instances"""
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    faction_assets = asset_registry.get_assets_by_class('FactionDataAsset', True)
    
    for asset_data in faction_assets:
        try:
            asset = unreal.load_asset(asset_data.package_name)
            if asset:
                # Validate Faction ID
                if not asset.faction_id:
                    self.log_error(f"{asset.get_name()}: Faction ID is empty")
                
                # Validate Tech Level
                if asset.tech_level < 1 or asset.tech_level > 10:
                    self.log_error(f"{asset.get_name()}: Tech level out of range")
        except Exception as e:
            self.log_error(f"Failed to load asset {asset_data.package_name}: {e}")
```

## Integration with Workflows

### For Designers

Run the Python script before opening Unreal Engine:

```bash
python SetupCheck.py
```

This ensures your environment is ready and identifies missing prerequisites early.

### For Programmers

Use the shell script with all options before building:

```bash
./SetupCheck.sh --clean --analyze
```

This validates the environment, cleans build artifacts, and runs static analysis.

### For CI/CD

Add to your continuous integration pipeline:

```bash
# In GitHub Actions, Jenkins, etc.
python SetupCheck.py --analyze || exit 1
```

This ensures all builds start from a validated environment.

### For Team Onboarding

New team members should run the setup check as their first step:

1. Clone repository
2. Run `python SetupCheck.py`
3. Fix any reported issues
4. Proceed with build

## Troubleshooting

### "No C++ compiler found"

**Linux/Mac:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# Mac
xcode-select --install
```

**Windows:**
- Install Visual Studio 2022 with "Desktop development with C++" workload

### "Unreal Engine version mismatch"

Check the .uproject file and ensure you have Unreal Engine 5.5 installed. Update the engine association if needed.

### "cppcheck not found" (when using --analyze)

**Linux:**
```bash
sudo apt-get install cppcheck
```

**Mac:**
```bash
brew install cppcheck
```

**Windows:**
Install via package manager or download from https://github.com/danmar/cppcheck/releases

### Script won't run - "Permission denied"

Make scripts executable:

```bash
chmod +x SetupCheck.sh
chmod +x SetupCheck.py
```

### Python script fails on Windows with color output

This is normal for older Windows versions. Colors will be disabled automatically. Functionality is not affected.

## Best Practices

### For Validation Scripts

1. **Run Before Every Build** - Validate environment before major builds
2. **Use --clean Periodically** - Clean build folders to avoid stale artifacts
3. **Run --analyze Before PR** - Catch issues before code review
4. **Keep Scripts Updated** - Update scripts when adding new project requirements
5. **Extend for Your Needs** - Add custom checks for project-specific requirements

### For Generator Scripts

1. **Generate Early** - Create templates when starting new content
2. **Customize Freely** - Templates are starting points, not final products
3. **Use Meaningful Names** - Follow naming conventions for consistency
4. **Document Systems** - Generate complete documentation sets for new systems
5. **Share Templates** - Commit useful templates for team use

## Template and Guide Generation

For creating templates and documentation, use the generator scripts:

```bash
# Generate a spaceship template
python TemplateGenerator.py --type spaceship --name "MyShip" --class Fighter

# Generate system documentation
python GuideGenerator.py --type system --name "MySystem"

# Use unified interface in Unreal Editor
python EditorUtilities.py --menu
```

See [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md) for complete documentation.

## Contributing

When adding new features or systems to Adastrea:

### For Validation
1. Update SetupCheck.sh and SetupCheck.py to validate new requirements
2. Add new validation methods to AdastreaAssetValidator.py for new Data Assets
3. Update this README with new checks and usage examples
4. Test scripts on multiple platforms (Windows, Linux, Mac)

### For Generators
1. Create templates using TemplateGenerator.py for new content types
2. Generate documentation using GuideGenerator.py for new systems
3. Update generator scripts if adding new template types
4. Share useful templates with the team

## Additional Resources

### Validation Scripts
- [README.md](README.md#robust-setup-validation) - Main project documentation
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md) - Manual testing checklist
- [Unreal Engine Python API](https://docs.unrealengine.com/en-US/ProductionPipelines/ScriptingAndAutomation/Python/) - For extending AdastreaAssetValidator.py

### Generator Scripts
- [GENERATOR_SCRIPTS_README.md](GENERATOR_SCRIPTS_README.md) - Complete generator documentation
- [TemplateGenerator.py](TemplateGenerator.py) - YAML template generation
- [GuideGenerator.py](GuideGenerator.py) - Markdown guide generation
- [EditorUtilities.py](EditorUtilities.py) - Unified Unreal Editor interface

---

**Last Updated**: 2024-10-28  
**Maintained by**: Adastrea Development Team
