# Testing Automation Quick Reference

Quick reference for Adastrea's automated testing tools.

## Scripts Overview

| Script | Purpose | Usage |
|--------|---------|-------|
| `SmokeTest.py` | Load maps, detect errors | In Editor or CLI |
| `ScreenshotTester.py` | Visual regression testing | In Editor or CLI |
| `AutomationRunner.py` | Run all tests | CLI only |

## Quick Commands

### Run All Tests
```bash
python AutomationRunner.py --all
```

### Run Specific Tests
```bash
# Smoke tests only
python AutomationRunner.py --smoke-test

# Screenshot tests only
python AutomationRunner.py --screenshot-test
```

### Schedule Overnight Run
```bash
python AutomationRunner.py --all --schedule "23:00"
```

## In Unreal Editor

### Smoke Test
```python
import SmokeTest
tester = SmokeTest.SmokeTestRunner()
results = tester.run_all_tests()
tester.generate_report(results)
```

### Screenshot Test
```python
import ScreenshotTester
tester = ScreenshotTester.ScreenshotTestRunner()
results = tester.run_all_tests()
tester.generate_report(results)
```

## Configuration Files

| File | Purpose |
|------|---------|
| `smoke_test_config.json` | Smoke test settings |
| `screenshot_test_config.json` | Screenshot test settings |
| `automation_config.json` | Automation runner settings |

## Output Locations

| Report Type | Location |
|-------------|----------|
| Smoke Tests | `TestReports/SmokeTests/` |
| Screenshots | `TestReports/Screenshots/` |
| Golden Masters | `TestReports/GoldenMasters/` |
| Automation | `TestReports/Automation/` |
| Logs | `TestReports/Automation/automation.log` |

## Common Tasks

### Update Golden Masters
Edit `screenshot_test_config.json`:
```json
{
  "test_settings": {
    "update_golden_masters": true
  }
}
```

### Add New Map to Smoke Test
Maps are auto-discovered from paths in `smoke_test_config.json`:
```json
{
  "map_discovery": {
    "search_paths": [
      "/Game/Maps/",
      "/Game/Content/Maps/"
    ]
  }
}
```

### Add New Screenshot Location
Edit `screenshot_test_config.json`:
```json
{
  "capture_locations": [
    {
      "name": "MyView",
      "map": "/Game/Maps/MyMap",
      "camera_location": {"x": 0, "y": 0, "z": 200},
      "camera_rotation": {"pitch": -15, "yaw": 0, "roll": 0}
    }
  ]
}
```

## Troubleshooting

### "Module not found"
Ensure scripts are in project root and add to sys.path:
```python
import sys
from pathlib import Path
project_root = Path(unreal.SystemLibrary.get_project_directory())
sys.path.insert(0, str(project_root))
```

### "No maps found"
Check:
1. Maps exist in Content/Maps/
2. Search paths in config are correct
3. Maps are .umap files (not .txt placeholders)

### PIL not available
Install Pillow for screenshot comparison:
```bash
pip install Pillow
```

## Best Practices

✓ Run smoke tests after adding/modifying maps  
✓ Run screenshot tests after visual changes  
✓ Review diff images before updating golden masters  
✓ Commit golden masters to version control  
✓ Schedule overnight runs for comprehensive validation  
✓ Keep configuration files in version control  

## More Information

📖 **Full Documentation**: [TESTING_AUTOMATION_GUIDE.md](TESTING_AUTOMATION_GUIDE.md)  
📋 **Examples**: Run `python examples_testing_framework.py`  
🔍 **Script Source**: Check script files for detailed docstrings

---

**Quick Start**: `python AutomationRunner.py --all`
