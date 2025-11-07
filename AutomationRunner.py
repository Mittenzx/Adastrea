#!/usr/bin/env python3
"""
Adastrea - Automation Runner CLI Utility

This script provides a command-line interface for running automated tests,
including smoke tests and screenshot tests. It supports scheduling for
overnight validation or testing runs.

Features:
- Run smoke tests, screenshot tests, or both
- Schedule tests for overnight execution
- Batch execution with comprehensive logging
- Integration with existing test frameworks
- Email notifications (optional)
- Export test results in multiple formats

Usage Examples:
    # Run all tests
    python AutomationRunner.py --all
    
    # Run only smoke tests
    python AutomationRunner.py --smoke-test
    
    # Run only screenshot tests
    python AutomationRunner.py --screenshot-test
    
    # Schedule overnight run
    python AutomationRunner.py --all --schedule "23:00"
    
    # Run with custom config
    python AutomationRunner.py --all --config automation_config.json
    
    # Run in Unreal Editor context
    UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=pythonscript -script="AutomationRunner.py --all"
"""

import argparse
import json
import os
import sys
import time
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List, Any, Optional
import traceback

# Try to import Unreal Engine modules (will be available in Editor)
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False


class AutomationConfig:
    """Configuration manager for automation runner"""
    
    DEFAULT_CONFIG = {
        "test_execution": {
            "run_smoke_tests": True,
            "run_screenshot_tests": True,
            "run_asset_validation": False,
            "stop_on_first_failure": False,
            "parallel_execution": False
        },
        "scheduling": {
            "enabled": False,
            "time": "23:00",
            "repeat_daily": False,
            "days_of_week": ["monday", "tuesday", "wednesday", "thursday", "friday"]
        },
        "reporting": {
            "output_directory": "TestReports/Automation",
            "consolidate_reports": True,
            "generate_summary": True,
            "email_notifications": False,
            "email_recipients": [],
            "timestamp_format": "%Y-%m-%d_%H-%M-%S"
        },
        "smoke_test_config": "smoke_test_config.json",
        "screenshot_test_config": "screenshot_test_config.json",
        "logging": {
            "log_level": "INFO",
            "log_file": "TestReports/Automation/automation.log",
            "console_output": True
        }
    }
    
    def __init__(self, config_path: Optional[str] = None):
        """Initialize configuration"""
        import copy
        self.config = copy.deepcopy(self.DEFAULT_CONFIG)
        if config_path and os.path.exists(config_path):
            self.load_config(config_path)
    
    def load_config(self, config_path: str):
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                user_config = json.load(f)
                self._merge_config(user_config)
        except Exception as e:
            print(f"Warning: Failed to load config from {config_path}: {e}")
    
    def _merge_config(self, user_config: Dict):
        """Merge user configuration with defaults"""
        for key, value in user_config.items():
            if key in self.config and isinstance(value, dict):
                self.config[key].update(value)
            else:
                self.config[key] = value
    
    def get(self, *keys):
        """Get configuration value by nested keys"""
        value = self.config
        for key in keys:
            if isinstance(value, dict):
                value = value.get(key, {})
            else:
                return None
        return value


class AutomationRunner:
    """Main automation runner"""
    
    def __init__(self, config_path: Optional[str] = None):
        """Initialize the automation runner"""
        self.config = AutomationConfig(config_path)
        
        # Determine project root
        if UNREAL_AVAILABLE:
            self.project_root = Path(unreal.SystemLibrary.get_project_directory())
        else:
            self.project_root = Path.cwd()
        
        # Setup output directory
        self.output_dir = self.project_root / self.config.get("reporting", "output_directory")
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Setup logging
        self.log_file = self.project_root / self.config.get("logging", "log_file")
        self.log_file.parent.mkdir(parents=True, exist_ok=True)
        
        # Initialize results storage
        self.results: Dict[str, Any] = {
            "smoke_tests": None,
            "screenshot_tests": None,
            "start_time": None,
            "end_time": None,
            "duration_seconds": 0,
            "overall_success": True
        }
    
    def log(self, message: str, level: str = "INFO"):
        """
        Log a message to file and optionally console
        
        Args:
            message: Message to log
            level: Log level (INFO, WARNING, ERROR)
        """
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] [{level}] {message}\n"
        
        # Write to log file
        with open(self.log_file, 'a') as f:
            f.write(log_entry)
        
        # Console output if enabled
        if self.config.get("logging", "console_output"):
            if UNREAL_AVAILABLE:
                if level == "ERROR":
                    unreal.log_error(message)
                elif level == "WARNING":
                    unreal.log_warning(message)
                else:
                    unreal.log(message)
            else:
                print(f"[{level}] {message}")
    
    def run_smoke_tests(self) -> bool:
        """
        Run smoke tests
        
        Returns:
            True if tests passed, False otherwise
        """
        self.log("Starting smoke tests...")
        
        try:
            if UNREAL_AVAILABLE:
                # Import and run smoke tests
                import SmokeTest
                
                config_path = self.project_root / self.config.get("smoke_test_config")
                runner = SmokeTest.SmokeTestRunner(str(config_path) if config_path.exists() else None)
                
                results = runner.run_all_tests()
                runner.generate_report(results)
                
                # Store results
                self.results["smoke_tests"] = {
                    "total": len(results),
                    "passed": sum(1 for r in results if r.success),
                    "failed": sum(1 for r in results if not r.success),
                    "details": [r.to_dict() for r in results]
                }
                
                success = all(r.success for r in results)
                self.log(f"Smoke tests completed: {self.results['smoke_tests']['passed']}/{self.results['smoke_tests']['total']} passed")
                
                return success
            else:
                self.log("Cannot run smoke tests outside Unreal Editor", "ERROR")
                return False
        
        except Exception as e:
            self.log(f"Error running smoke tests: {e}", "ERROR")
            self.log(traceback.format_exc(), "ERROR")
            return False
    
    def run_screenshot_tests(self) -> bool:
        """
        Run screenshot tests
        
        Returns:
            True if tests passed, False otherwise
        """
        self.log("Starting screenshot tests...")
        
        try:
            if UNREAL_AVAILABLE:
                # Import and run screenshot tests
                import ScreenshotTester
                
                config_path = self.project_root / self.config.get("screenshot_test_config")
                runner = ScreenshotTester.ScreenshotTestRunner(
                    str(config_path) if config_path.exists() else None
                )
                
                results = runner.run_all_tests()
                runner.generate_report(results)
                
                # Store results
                self.results["screenshot_tests"] = {
                    "total": len(results),
                    "passed": sum(1 for r in results if r.success),
                    "failed": sum(1 for r in results if not r.success),
                    "details": [r.to_dict() for r in results]
                }
                
                success = all(r.success for r in results)
                self.log(f"Screenshot tests completed: {self.results['screenshot_tests']['passed']}/{self.results['screenshot_tests']['total']} passed")
                
                return success
            else:
                self.log("Cannot run screenshot tests outside Unreal Editor", "ERROR")
                return False
        
        except Exception as e:
            self.log(f"Error running screenshot tests: {e}", "ERROR")
            self.log(traceback.format_exc(), "ERROR")
            return False
    
    def run_all_tests(self) -> bool:
        """
        Run all configured tests
        
        Returns:
            True if all tests passed, False otherwise
        """
        self.log("=" * 80)
        self.log("Starting Automated Test Suite")
        self.log("=" * 80)
        
        self.results["start_time"] = datetime.now().isoformat()
        start = datetime.now()
        
        overall_success = True
        
        # Run smoke tests if configured
        if self.config.get("test_execution", "run_smoke_tests"):
            success = self.run_smoke_tests()
            overall_success = overall_success and success
            
            if not success and self.config.get("test_execution", "stop_on_first_failure"):
                self.log("Stopping test execution due to smoke test failure", "WARNING")
                self.results["overall_success"] = False
                return False
        
        # Run screenshot tests if configured
        if self.config.get("test_execution", "run_screenshot_tests"):
            success = self.run_screenshot_tests()
            overall_success = overall_success and success
            
            if not success and self.config.get("test_execution", "stop_on_first_failure"):
                self.log("Stopping test execution due to screenshot test failure", "WARNING")
                self.results["overall_success"] = False
                return False
        
        # Calculate duration
        end = datetime.now()
        self.results["end_time"] = end.isoformat()
        self.results["duration_seconds"] = (end - start).total_seconds()
        self.results["overall_success"] = overall_success
        
        self.log("=" * 80)
        self.log(f"Test Suite Completed in {self.results['duration_seconds']:.2f} seconds")
        self.log(f"Overall Result: {'PASSED' if overall_success else 'FAILED'}")
        self.log("=" * 80)
        
        # Generate consolidated report
        if self.config.get("reporting", "consolidate_reports"):
            self.generate_consolidated_report()
        
        return overall_success
    
    def generate_consolidated_report(self):
        """Generate a consolidated report of all test results"""
        self.log("Generating consolidated report...")
        
        timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
        
        # Generate JSON report
        json_path = self.output_dir / f"consolidated_report_{timestamp}.json"
        with open(json_path, 'w') as f:
            json.dump(self.results, f, indent=2)
        
        self.log(f"JSON report saved to: {json_path}")
        
        # Generate HTML summary
        if self.config.get("reporting", "generate_summary"):
            html_path = self.output_dir / f"consolidated_report_{timestamp}.html"
            self._generate_html_summary(html_path)
            self.log(f"HTML report saved to: {html_path}")
    
    def _generate_html_summary(self, output_path: Path):
        """Generate HTML summary report"""
        
        # Extract statistics
        smoke_total = self.results.get("smoke_tests", {}).get("total", 0) if self.results.get("smoke_tests") else 0
        smoke_passed = self.results.get("smoke_tests", {}).get("passed", 0) if self.results.get("smoke_tests") else 0
        smoke_failed = self.results.get("smoke_tests", {}).get("failed", 0) if self.results.get("smoke_tests") else 0
        
        screenshot_total = self.results.get("screenshot_tests", {}).get("total", 0) if self.results.get("screenshot_tests") else 0
        screenshot_passed = self.results.get("screenshot_tests", {}).get("passed", 0) if self.results.get("screenshot_tests") else 0
        screenshot_failed = self.results.get("screenshot_tests", {}).get("failed", 0) if self.results.get("screenshot_tests") else 0
        
        overall_status = "PASSED" if self.results.get("overall_success") else "FAILED"
        status_color = "#2ecc71" if self.results.get("overall_success") else "#e74c3c"
        
        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Adastrea Automation Test Report</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }}
        .header {{
            background-color: #2c3e50;
            color: white;
            padding: 20px;
            border-radius: 5px;
        }}
        .overall-status {{
            background-color: {status_color};
            color: white;
            padding: 30px;
            margin: 20px 0;
            border-radius: 5px;
            text-align: center;
            font-size: 24px;
            font-weight: bold;
        }}
        .test-section {{
            background-color: white;
            padding: 20px;
            margin: 20px 0;
            border-radius: 5px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .stat {{
            display: inline-block;
            margin: 10px 30px 10px 0;
            font-size: 18px;
        }}
        .stat-value {{
            font-weight: bold;
            font-size: 24px;
        }}
        .passed {{ color: #2ecc71; }}
        .failed {{ color: #e74c3c; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>Adastrea Automated Test Suite Report</h1>
        <p>Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>
        <p>Duration: {self.results.get('duration_seconds', 0):.2f} seconds</p>
    </div>
    
    <div class="overall-status">
        Overall Status: {overall_status}
    </div>
    
    <div class="test-section">
        <h2>Smoke Tests</h2>
        <div class="stat">
            <div>Total Tests</div>
            <div class="stat-value">{smoke_total}</div>
        </div>
        <div class="stat">
            <div>Passed</div>
            <div class="stat-value passed">{smoke_passed}</div>
        </div>
        <div class="stat">
            <div>Failed</div>
            <div class="stat-value failed">{smoke_failed}</div>
        </div>
    </div>
    
    <div class="test-section">
        <h2>Screenshot Tests</h2>
        <div class="stat">
            <div>Total Tests</div>
            <div class="stat-value">{screenshot_total}</div>
        </div>
        <div class="stat">
            <div>Passed</div>
            <div class="stat-value passed">{screenshot_passed}</div>
        </div>
        <div class="stat">
            <div>Failed</div>
            <div class="stat-value failed">{screenshot_failed}</div>
        </div>
    </div>
    
    <div class="test-section">
        <h2>Test Execution Details</h2>
        <p><strong>Start Time:</strong> {self.results.get('start_time', 'N/A')}</p>
        <p><strong>End Time:</strong> {self.results.get('end_time', 'N/A')}</p>
        <p><strong>Log File:</strong> {self.log_file}</p>
    </div>
</body>
</html>
"""
        
        with open(output_path, 'w') as f:
            f.write(html)
    
    def schedule_tests(self, schedule_time: str):
        """
        Schedule tests to run at a specific time
        
        Args:
            schedule_time: Time to run tests in HH:MM format
        """
        self.log(f"Scheduling tests for {schedule_time}")
        
        try:
            # Parse schedule time
            target_hour, target_minute = map(int, schedule_time.split(':'))
            
            # Calculate time until scheduled run
            now = datetime.now()
            target = now.replace(hour=target_hour, minute=target_minute, second=0, microsecond=0)
            
            # If target time has passed today, schedule for tomorrow
            if target < now:
                target += timedelta(days=1)
            
            wait_seconds = (target - now).total_seconds()
            
            self.log(f"Tests will run at {target.strftime('%Y-%m-%d %H:%M:%S')}")
            self.log(f"Waiting {wait_seconds:.0f} seconds...")
            
            # Wait until scheduled time with interruptible sleep (check every minute)
            # This allows for graceful shutdown if needed
            check_interval = 60  # Check every minute
            while wait_seconds > 0:
                sleep_time = min(check_interval, wait_seconds)
                time.sleep(sleep_time)
                wait_seconds -= sleep_time
                
                # Log progress every 10 minutes
                if wait_seconds > 0 and int(wait_seconds) % 600 == 0:
                    self.log(f"Still waiting... {wait_seconds:.0f} seconds until test execution")
            
            # Run tests
            self.run_all_tests()
            
        except KeyboardInterrupt:
            self.log("Scheduled test execution interrupted by user", "WARNING")
            raise
        except Exception as e:
            self.log(f"Error scheduling tests: {e}", "ERROR")
            self.log(traceback.format_exc(), "ERROR")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Adastrea Automation Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Run all tests
  python AutomationRunner.py --all
  
  # Run only smoke tests
  python AutomationRunner.py --smoke-test
  
  # Schedule overnight run
  python AutomationRunner.py --all --schedule "23:00"
  
  # Use custom configuration
  python AutomationRunner.py --all --config automation_config.json
        """
    )
    
    parser.add_argument('--all', action='store_true',
                       help='Run all configured tests')
    parser.add_argument('--smoke-test', action='store_true',
                       help='Run smoke tests only')
    parser.add_argument('--screenshot-test', action='store_true',
                       help='Run screenshot tests only')
    parser.add_argument('--schedule', type=str, metavar='HH:MM',
                       help='Schedule tests to run at specific time (24-hour format)')
    parser.add_argument('--config', type=str, metavar='PATH',
                       help='Path to automation configuration file')
    parser.add_argument('--verbose', action='store_true',
                       help='Enable verbose logging')
    
    args = parser.parse_args()
    
    # Validate arguments
    if not any([args.all, args.smoke_test, args.screenshot_test]):
        parser.print_help()
        print("\nError: At least one test type must be specified")
        return 1
    
    try:
        # Initialize runner
        runner = AutomationRunner(args.config)
        
        # Override config with command line arguments
        if args.smoke_test and not args.all:
            runner.config.config["test_execution"]["run_smoke_tests"] = True
            runner.config.config["test_execution"]["run_screenshot_tests"] = False
        
        if args.screenshot_test and not args.all:
            runner.config.config["test_execution"]["run_smoke_tests"] = False
            runner.config.config["test_execution"]["run_screenshot_tests"] = True
        
        if args.verbose:
            runner.config.config["logging"]["console_output"] = True
            runner.config.config["logging"]["log_level"] = "DEBUG"
        
        # Run tests (scheduled or immediate)
        if args.schedule:
            runner.schedule_tests(args.schedule)
        else:
            success = runner.run_all_tests()
            return 0 if success else 1
    
    except KeyboardInterrupt:
        print("\nTest execution interrupted by user")
        return 130
    except Exception as e:
        print(f"Fatal error: {e}")
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
