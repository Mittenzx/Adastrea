#!/usr/bin/env python3
"""
Example Usage of Adastrea Testing Framework

This script demonstrates how to use the testing automation scripts
outside of the Unreal Editor environment for documentation purposes.

Note: The actual testing requires Unreal Engine Python environment.
This is for demonstration and documentation only.
"""

def example_smoke_test_usage():
    """Example of running smoke tests"""
    print("=" * 80)
    print("EXAMPLE: Running Smoke Tests")
    print("=" * 80)
    print()
    print("In Unreal Editor Python Console:")
    print()
    print("```python")
    print("import SmokeTest")
    print()
    print("# Create runner with default configuration")
    print("tester = SmokeTest.SmokeTestRunner()")
    print()
    print("# Run all tests")
    print("results = tester.run_all_tests()")
    print()
    print("# Generate HTML and JSON reports")
    print("tester.generate_report(results)")
    print()
    print("# Check results")
    print("passed = sum(1 for r in results if r.success)")
    print("failed = sum(1 for r in results if not r.success)")
    print("print(f'Passed: {passed}, Failed: {failed}')")
    print("```")
    print()


def example_screenshot_test_usage():
    """Example of running screenshot tests"""
    print("=" * 80)
    print("EXAMPLE: Running Screenshot Tests")
    print("=" * 80)
    print()
    print("In Unreal Editor Python Console:")
    print()
    print("```python")
    print("import ScreenshotTester")
    print()
    print("# Create runner with custom configuration")
    print("tester = ScreenshotTester.ScreenshotTestRunner('screenshot_test_config.json')")
    print()
    print("# Run all tests")
    print("results = tester.run_all_tests()")
    print()
    print("# Generate reports")
    print("tester.generate_report(results)")
    print()
    print("# Check for visual regressions")
    print("regressions = [r for r in results if not r.success]")
    print("if regressions:")
    print("    print(f'Visual regressions detected in {len(regressions)} tests')")
    print("    for r in regressions:")
    print("        print(f'  - {r.name}: {r.difference_percentage:.2f}% difference')")
    print("```")
    print()


def example_automation_runner_usage():
    """Example of using automation runner"""
    print("=" * 80)
    print("EXAMPLE: Using Automation Runner")
    print("=" * 80)
    print()
    print("From Command Line (outside Unreal Editor):")
    print()
    print("# Run all tests immediately")
    print("$ python AutomationRunner.py --all")
    print()
    print("# Run only smoke tests")
    print("$ python AutomationRunner.py --smoke-test")
    print()
    print("# Run only screenshot tests")
    print("$ python AutomationRunner.py --screenshot-test")
    print()
    print("# Schedule tests for 11 PM")
    print("$ python AutomationRunner.py --all --schedule '23:00'")
    print()
    print("# Use custom configuration")
    print("$ python AutomationRunner.py --all --config my_config.json")
    print()
    print("# Enable verbose output")
    print("$ python AutomationRunner.py --all --verbose")
    print()


def example_custom_configuration():
    """Example of creating custom configuration"""
    print("=" * 80)
    print("EXAMPLE: Custom Configuration")
    print("=" * 80)
    print()
    print("Create a custom screenshot test configuration:")
    print()
    print("```python")
    print("import json")
    print()
    print("config = {")
    print("    'capture_locations': [")
    print("        {")
    print("            'name': 'Station_Exterior',")
    print("            'map': '/Game/Maps/TestStation',")
    print("            'camera_location': {'x': 1000, 'y': 0, 'z': 500},")
    print("            'camera_rotation': {'pitch': -20, 'yaw': 180, 'roll': 0},")
    print("            'wait_time_seconds': 3.0")
    print("        }")
    print("    ],")
    print("    'test_settings': {")
    print("        'screenshot_width': 1920,")
    print("        'screenshot_height': 1080,")
    print("        'comparison_threshold': 0.02,  # 2% difference allowed")
    print("        'update_golden_masters': False")
    print("    }")
    print("}")
    print()
    print("# Save configuration")
    print("with open('my_screenshot_config.json', 'w') as f:")
    print("    json.dump(config, f, indent=2)")
    print()
    print("# Use configuration")
    print("tester = ScreenshotTester.ScreenshotTestRunner('my_screenshot_config.json')")
    print("```")
    print()


def example_ci_cd_integration():
    """Example of CI/CD integration"""
    print("=" * 80)
    print("EXAMPLE: CI/CD Integration")
    print("=" * 80)
    print()
    print("GitHub Actions workflow (.github/workflows/nightly-tests.yml):")
    print()
    print("```yaml")
    print("name: Nightly Tests")
    print()
    print("on:")
    print("  schedule:")
    print("    - cron: '0 23 * * *'  # Run at 11 PM daily")
    print("  workflow_dispatch:  # Allow manual trigger")
    print()
    print("jobs:")
    print("  test:")
    print("    runs-on: windows-latest")
    print("    steps:")
    print("      - uses: actions/checkout@v2")
    print("      ")
    print("      - name: Run Automation Tests")
    print("        run: python AutomationRunner.py --all")
    print("      ")
    print("      - name: Upload Test Reports")
    print("        if: always()")
    print("        uses: actions/upload-artifact@v2")
    print("        with:")
    print("          name: test-reports")
    print("          path: TestReports/")
    print("```")
    print()


def main():
    """Main demonstration"""
    print()
    print("╔" + "=" * 78 + "╗")
    print("║" + " " * 20 + "Adastrea Testing Framework Examples" + " " * 22 + "║")
    print("╚" + "=" * 78 + "╝")
    print()
    
    example_smoke_test_usage()
    example_screenshot_test_usage()
    example_automation_runner_usage()
    example_custom_configuration()
    example_ci_cd_integration()
    
    print("=" * 80)
    print("For complete documentation, see TESTING_AUTOMATION_GUIDE.md")
    print("=" * 80)
    print()


if __name__ == "__main__":
    main()
