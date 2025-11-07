# Adastrea Testing and Automation Guide

This guide provides comprehensive documentation for the automated testing and validation workflows in the Adastrea project.

## Table of Contents

1. [Overview](#overview)
2. [Testing Scripts](#testing-scripts)
3. [Smoke Testing](#smoke-testing)
4. [Screenshot Testing](#screenshot-testing)
5. [Automation Runner](#automation-runner)
6. [Configuration](#configuration)
7. [Scheduling Overnight Tests](#scheduling-overnight-tests)
8. [Integration with Editor](#integration-with-editor)
9. [Troubleshooting](#troubleshooting)

## Overview

The Adastrea project includes a comprehensive suite of automated testing tools designed to improve reliability and streamline validation workflows:

- **Smoke Tests**: Load all maps and detect missing assets, broken references, and errors
- **Screenshot Tests**: Capture screenshots and perform visual regression testing
- **Automation Runner**: CLI utility for running tests and scheduling overnight validation

All testing scripts are designed to run inside the Unreal Editor's Python environment and generate detailed HTML and JSON reports.

## Testing Scripts

### Available Scripts

| Script | Purpose | Runs In Editor | CLI Support |
|--------|---------|----------------|-------------|
| `SmokeTest.py` | Map loading and asset validation | ✓ | ✓ |
| `ScreenshotTester.py` | Visual regression testing | ✓ | ✓ |
| `AutomationRunner.py` | Orchestrates all tests | ✓ | ✓ |

### Prerequisites

1. **Unreal Engine 5.5** with Python Editor Script Plugin enabled
2. **Python 3.6+** for standalone script execution
3. **PIL/Pillow** (optional, for screenshot comparison):
   ```bash
   pip install Pillow
   ```

## Smoke Testing

### What It Does

The smoke test script (`SmokeTest.py`) performs the following checks:

- ✓ Discovers all maps in the project
- ✓ Loads each map and verifies successful loading
- ✓ Detects missing static meshes and skeletal meshes
- ✓ Identifies broken asset references
- ✓ Captures warnings and errors during map loading
- ✓ Takes screenshots of maps with errors
- ✓ Generates detailed HTML and JSON reports

### Running Smoke Tests

#### In Unreal Editor Python Console

```python
import SmokeTest

# Create runner
tester = SmokeTest.SmokeTestRunner()

# Run all tests
results = tester.run_all_tests()

# Generate report
tester.generate_report(results)
```

#### With Custom Configuration

```python
import SmokeTest

tester = SmokeTest.SmokeTestRunner("smoke_test_config.json")
results = tester.run_all_tests()
tester.generate_report(results)
```

#### From Command Line (Unreal Editor)

```bash
UnrealEditor-Cmd.exe "Adastrea.uproject" -run=pythonscript -script="SmokeTest.py"
```

### Configuration

Edit `smoke_test_config.json` to customize:

```json
{
  "test_settings": {
    "load_timeout_seconds": 30,
    "screenshot_on_error": true,
    "verbose_logging": true,
    "stop_on_first_error": false
  },
  "map_discovery": {
    "search_paths": [
      "/Game/Maps/",
      "/Game/Content/Maps/"
    ],
    "exclude_patterns": [
      "*_Backup",
      "*_OLD",
      "*/Developers/*"
    ]
  }
}
```

### Output

Smoke tests generate the following reports in `TestReports/SmokeTests/`:

- **HTML Report**: `smoke_test_report_[timestamp].html` - Visual report with screenshots
- **JSON Report**: `smoke_test_report_[timestamp].json` - Machine-readable results
- **Screenshots**: `Screenshots/[mapname]_[timestamp].png` - Error screenshots

## Screenshot Testing

### What It Does

The screenshot test script (`ScreenshotTester.py`) provides:

- ✓ Automated screenshot capture at configured camera positions
- ✓ Golden master comparison with pixel-based difference detection
- ✓ Visual regression reporting with difference highlighting
- ✓ Side-by-side comparison images
- ✓ Asset thumbnail generation (in development)
- ✓ Configurable difference thresholds

### Running Screenshot Tests

#### In Unreal Editor Python Console

```python
import ScreenshotTester

# Create runner
tester = ScreenshotTester.ScreenshotTestRunner()

# Run all tests
results = tester.run_all_tests()

# Generate report
tester.generate_report(results)
```

#### Creating Golden Masters

First run creates golden masters if `update_golden_masters` is enabled:

```python
import ScreenshotTester

tester = ScreenshotTester.ScreenshotTestRunner("screenshot_test_config.json")
# Set update flag in config or manually:
tester.config.config["test_settings"]["update_golden_masters"] = True

results = tester.run_all_tests()
tester.generate_report(results)
```

### Configuration

Edit `screenshot_test_config.json`:

```json
{
  "test_settings": {
    "screenshot_width": 1920,
    "screenshot_height": 1080,
    "comparison_threshold": 0.01,
    "pixel_tolerance": 5,
    "update_golden_masters": false
  },
  "capture_locations": [
    {
      "name": "Main_Menu_View",
      "map": "/Game/Maps/MainMenu",
      "camera_location": {"x": 0, "y": 0, "z": 200},
      "camera_rotation": {"pitch": -15, "yaw": 0, "roll": 0},
      "wait_time_seconds": 2.0
    }
  ]
}
```

### Output

Screenshot tests generate reports in `TestReports/Screenshots/`:

- **Current Screenshots**: `[testname]_[timestamp].png`
- **Golden Masters**: `TestReports/GoldenMasters/[testname].png`
- **Diff Images**: `TestReports/ScreenshotDiffs/[testname]_diff_[timestamp].png`
- **HTML Report**: `screenshot_test_report_[timestamp].html`
- **JSON Report**: `screenshot_test_report_[timestamp].json`

## Automation Runner

### What It Does

The Automation Runner (`AutomationRunner.py`) orchestrates all testing:

- ✓ Runs smoke tests and screenshot tests in sequence
- ✓ Generates consolidated reports
- ✓ Supports scheduled execution for overnight runs
- ✓ Command-line interface for CI/CD integration
- ✓ Comprehensive logging

### Running All Tests

```bash
# Run all tests
python AutomationRunner.py --all

# Run only smoke tests
python AutomationRunner.py --smoke-test

# Run only screenshot tests
python AutomationRunner.py --screenshot-test

# Use custom configuration
python AutomationRunner.py --all --config automation_config.json
```

### Scheduling Overnight Tests

```bash
# Schedule tests to run at 11:00 PM
python AutomationRunner.py --all --schedule "23:00"

# This will wait until the scheduled time and then run all tests
```

For repeated overnight runs, use system schedulers:

#### Windows (Task Scheduler)

Create a batch file `run_tests.bat`:
```batch
@echo off
cd /d C:\Projects\Adastrea
python AutomationRunner.py --all
```

Then schedule it in Task Scheduler.

#### Linux/Mac (cron)

```bash
# Edit crontab
crontab -e

# Add line to run tests at 11:00 PM daily
0 23 * * * cd /path/to/Adastrea && python3 AutomationRunner.py --all
```

### Configuration

Edit `automation_config.json`:

```json
{
  "test_execution": {
    "run_smoke_tests": true,
    "run_screenshot_tests": true,
    "stop_on_first_failure": false
  },
  "scheduling": {
    "enabled": false,
    "time": "23:00",
    "repeat_daily": false
  },
  "reporting": {
    "output_directory": "TestReports/Automation",
    "consolidate_reports": true,
    "generate_summary": true
  }
}
```

### Output

Automation runner generates in `TestReports/Automation/`:

- **Consolidated Report**: `consolidated_report_[timestamp].html`
- **JSON Report**: `consolidated_report_[timestamp].json`
- **Log File**: `automation.log`

## Configuration Files

### Configuration File Summary

| File | Purpose |
|------|---------|
| `smoke_test_config.json` | Smoke test settings |
| `screenshot_test_config.json` | Screenshot test settings |
| `automation_config.json` | Automation runner settings |

### Configuration Best Practices

1. **Start with defaults**: The scripts work with sensible defaults
2. **Customize gradually**: Add specific maps/locations as needed
3. **Version control configs**: Track configuration changes in git
4. **Document custom settings**: Add comments explaining why settings were changed

## Integration with Editor

### Running Tests in Unreal Editor

1. **Enable Python Plugin**:
   - Edit → Plugins → Search for "Python Editor Script Plugin"
   - Enable and restart Unreal Editor

2. **Run from Python Console**:
   - Window → Developer Tools → Output Log
   - Window → Developer Tools → Python Console
   - Execute test scripts as shown above

3. **Add to Editor Menu** (Optional):
   - Create Editor Utility Widget (see next section)

### Editor Utility Widget Integration

To add a UI for running tests in the editor, you can create an Editor Utility Widget:

1. Create a new Editor Utility Widget Blueprint
2. Add buttons for each test type
3. Call the Python scripts from Blueprint using Execute Python Script node

Example Blueprint logic:
```
Button_RunSmokeTests → Execute Python Script → "import SmokeTest; SmokeTest.main()"
```

## Troubleshooting

### Common Issues

#### "Module not found" errors

**Problem**: `ImportError: No module named 'SmokeTest'`

**Solution**: Ensure scripts are in the project root directory and Python can find them:
```python
import sys
from pathlib import Path
project_root = Path(unreal.SystemLibrary.get_project_directory())
sys.path.insert(0, str(project_root))
```

#### No maps found

**Problem**: Smoke tests report "No maps found to test"

**Solution**: 
1. Check that maps exist in Content/Maps/
2. Verify search paths in `smoke_test_config.json`
3. Ensure maps are saved as `.umap` files (not placeholder `.txt` files)

#### Screenshot comparison fails

**Problem**: "PIL not available, cannot compare screenshots"

**Solution**: Install Pillow:
```bash
pip install Pillow
```

#### Permission errors writing reports

**Problem**: Cannot write to TestReports directory

**Solution**:
1. Ensure directory exists and is writable
2. Check Unreal Editor has permissions to write to project directory
3. Run Unreal Editor with appropriate permissions

### Debug Mode

Enable verbose logging:

```python
# In Python scripts
tester.config.config["test_settings"]["verbose_logging"] = True

# Or via CLI
python AutomationRunner.py --all --verbose
```

### Getting Help

1. Check the script source code - all scripts are well-documented
2. Review the configuration files for available options
3. Check the Unreal Engine Python documentation
4. Review generated log files in TestReports/

## Best Practices

### When to Run Tests

- **Smoke Tests**: 
  - After adding or modifying maps
  - Before committing major changes
  - During overnight CI runs
  - Before releases

- **Screenshot Tests**:
  - After visual changes to levels
  - After lighting changes
  - After material updates
  - Weekly regression checks

### Maintaining Golden Masters

1. **Initial Creation**: Run screenshot tests with `update_golden_masters: true`
2. **Review Changes**: Always review diff images before updating golden masters
3. **Version Control**: Commit golden masters to git
4. **Regular Updates**: Update golden masters when intentional visual changes are made

### CI/CD Integration

Example GitHub Actions workflow:

```yaml
name: Nightly Tests

on:
  schedule:
    - cron: '0 23 * * *'  # Run at 11 PM daily

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run Automation Tests
        run: python AutomationRunner.py --all
      - name: Upload Reports
        uses: actions/upload-artifact@v2
        with:
          name: test-reports
          path: TestReports/
```

## Examples

### Example 1: Quick Smoke Test

```python
import SmokeTest

tester = SmokeTest.SmokeTestRunner()
results = tester.run_all_tests()
tester.generate_report(results)

# Print summary
print(f"Tested {len(results)} maps")
print(f"Passed: {sum(1 for r in results if r.success)}")
print(f"Failed: {sum(1 for r in results if not r.success)}")
```

### Example 2: Screenshot Test with Custom Config

```python
import ScreenshotTester
import json

# Load config
with open("screenshot_test_config.json", 'r') as f:
    config = json.load(f)

# Add custom location
config["capture_locations"].append({
    "name": "CustomView",
    "map": "/Game/Maps/MyMap",
    "camera_location": {"x": 100, "y": 200, "z": 300},
    "camera_rotation": {"pitch": -20, "yaw": 45, "roll": 0}
})

# Save modified config
with open("custom_screenshot_config.json", 'w') as f:
    json.dump(config, f, indent=2)

# Run with custom config
tester = ScreenshotTester.ScreenshotTestRunner("custom_screenshot_config.json")
results = tester.run_all_tests()
tester.generate_report(results)
```

### Example 3: Automated Overnight Validation

```bash
# Create a shell script: nightly_tests.sh
#!/bin/bash
cd /path/to/Adastrea
python3 AutomationRunner.py --all > nightly_test_output.log 2>&1

# Add to crontab for 11 PM daily
# 0 23 * * * /path/to/nightly_tests.sh
```

## Related Documentation

- [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md) - Manual testing checklist
- [SetupCheck.py](SetupCheck.py) - Environment validation
- [AdastreaAssetValidator.py](AdastreaAssetValidator.py) - Asset validation
- [EditorUtilities.py](EditorUtilities.py) - Editor utility functions

## Version History

- **1.0** (2024-11-07): Initial implementation
  - Smoke testing with map loading and asset validation
  - Screenshot testing with visual regression
  - Automation runner with scheduling support
  - Comprehensive reporting and logging

---

**Maintained by**: Adastrea Development Team  
**Last Updated**: 2024-11-07
