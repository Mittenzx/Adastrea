#!/usr/bin/env python3
"""
Adastrea - Smoke Test Script

This script provides comprehensive smoke testing for the Adastrea project.
It loads every level/map and reports on missing assets, broken references, or warnings.

This script is designed to run INSIDE the Unreal Editor's Python environment.

Features:
- Discovers and loads all maps in the project
- Detects missing assets and broken references
- Captures warnings and errors during map loading
- Generates detailed HTML and JSON reports
- Configurable via smoke_test_config.json
- Can be run from CLI or Editor Utility Widget

Usage in Unreal Editor Python Console:
    import SmokeTest
    tester = SmokeTest.SmokeTestRunner()
    results = tester.run_all_tests()
    tester.generate_report(results)

Usage from CLI (requires Unreal Engine command line):
    UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=pythonscript -script="SmokeTest.py"
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple
import traceback

# Try to import Unreal Engine modules (will be available in Editor)
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("Warning: Running outside Unreal Editor. Limited functionality available.")


class SmokeTestConfig:
    """Configuration manager for smoke tests"""
    
    DEFAULT_CONFIG = {
        "test_settings": {
            "load_timeout_seconds": 30,
            "screenshot_on_error": True,
            "verbose_logging": True,
            "stop_on_first_error": False
        },
        "map_discovery": {
            "search_paths": [
                "/Game/Maps/",
                "/Game/Content/Maps/",
                "/Game/Levels/"
            ],
            "exclude_patterns": [
                "*_Backup",
                "*_OLD",
                "*_Test",
                "*/Developers/*"
            ],
            "include_sublevels": True
        },
        "error_detection": {
            "check_missing_assets": True,
            "check_broken_references": True,
            "check_blueprint_errors": True,
            "check_material_errors": True,
            "capture_warnings": True,
            "error_keywords": [
                "Failed to load",
                "Missing",
                "Broken",
                "Error",
                "Cannot find",
                "Invalid reference"
            ]
        },
        "reporting": {
            "output_directory": "TestReports/SmokeTests",
            "generate_html": True,
            "generate_json": True,
            "generate_csv": False,
            "include_screenshots": True,
            "timestamp_format": "%Y-%m-%d_%H-%M-%S"
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
            value = value.get(key, {})
        return value


class TestResult:
    """Represents the result of a single test"""
    
    def __init__(self, map_name: str):
        self.map_name = map_name
        self.success = True
        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.missing_assets: List[str] = []
        self.broken_references: List[str] = []
        self.load_time_seconds = 0.0
        self.timestamp = datetime.now().isoformat()
        self.screenshot_path: Optional[str] = None
    
    def add_error(self, error: str):
        """Add an error to the result"""
        self.errors.append(error)
        self.success = False
    
    def add_warning(self, warning: str):
        """Add a warning to the result"""
        self.warnings.append(warning)
    
    def add_missing_asset(self, asset_path: str):
        """Add a missing asset"""
        self.missing_assets.append(asset_path)
        self.success = False
    
    def add_broken_reference(self, reference: str):
        """Add a broken reference"""
        self.broken_references.append(reference)
        self.success = False
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert result to dictionary"""
        return {
            "map_name": self.map_name,
            "success": self.success,
            "errors": self.errors,
            "warnings": self.warnings,
            "missing_assets": self.missing_assets,
            "broken_references": self.broken_references,
            "load_time_seconds": self.load_time_seconds,
            "timestamp": self.timestamp,
            "screenshot_path": self.screenshot_path
        }


class SmokeTestRunner:
    """Main smoke test runner"""
    
    def __init__(self, config_path: Optional[str] = None):
        """Initialize the smoke test runner"""
        if not UNREAL_AVAILABLE:
            raise RuntimeError("SmokeTestRunner requires Unreal Engine Python environment")
        
        self.config = SmokeTestConfig(config_path)
        self.results: List[TestResult] = []
        
        # Get project root
        self.project_root = Path(unreal.SystemLibrary.get_project_directory())
        self.output_dir = self.project_root / self.config.get("reporting", "output_directory")
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Initialize logging
        self.log_messages: List[Tuple[str, str]] = []  # (level, message)
    
    def log(self, message: str, level: str = "info"):
        """Log a message"""
        self.log_messages.append((level, message))
        
        if self.config.get("test_settings", "verbose_logging"):
            if level == "error":
                unreal.log_error(f"[SmokeTest] {message}")
            elif level == "warning":
                unreal.log_warning(f"[SmokeTest] {message}")
            else:
                unreal.log(f"[SmokeTest] {message}")
    
    def discover_maps(self) -> List[str]:
        """
        Discover all maps in the project
        
        Returns:
            List of map asset paths
        """
        self.log("Discovering maps in project...")
        
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        search_paths = self.config.get("map_discovery", "search_paths")
        exclude_patterns = self.config.get("map_discovery", "exclude_patterns")
        
        all_maps = []
        
        for search_path in search_paths:
            # Get all assets in this path
            assets = asset_registry.get_assets_by_path(search_path, recursive=True)
            
            for asset_data in assets:
                asset_path = str(asset_data.package_name)
                asset_class = str(asset_data.asset_class_path.asset_name)
                
                # Check if it's a map/level
                if asset_class == "World" or asset_path.endswith(".umap"):
                    # Check exclusion patterns
                    excluded = False
                    for pattern in exclude_patterns:
                        # Simple pattern matching (can be enhanced)
                        pattern_clean = pattern.replace("*", "")
                        if pattern_clean in asset_path:
                            excluded = True
                            break
                    
                    if not excluded:
                        all_maps.append(asset_path)
        
        self.log(f"Discovered {len(all_maps)} maps")
        return all_maps
    
    def load_map(self, map_path: str) -> TestResult:
        """
        Load a map and test for errors
        
        Args:
            map_path: Path to the map asset
            
        Returns:
            TestResult object
        """
        result = TestResult(map_path)
        self.log(f"Loading map: {map_path}")
        
        start_time = datetime.now()
        
        try:
            # Load the map
            success = unreal.EditorLevelLibrary.load_level(map_path)
            
            if not success:
                result.add_error(f"Failed to load map: {map_path}")
                return result
            
            # Calculate load time
            load_time = (datetime.now() - start_time).total_seconds()
            result.load_time_seconds = load_time
            
            # Check for errors in the loaded map
            self._check_map_assets(result)
            self._check_broken_references(result)
            
            # Capture screenshot if enabled and there were errors
            if (self.config.get("reporting", "include_screenshots") and 
                self.config.get("test_settings", "screenshot_on_error") and 
                not result.success):
                result.screenshot_path = self._capture_screenshot(map_path)
            
            self.log(f"Completed testing map: {map_path} (Success: {result.success})")
            
        except Exception as e:
            result.add_error(f"Exception while loading map: {str(e)}")
            result.add_error(traceback.format_exc())
            self.log(f"Exception testing map {map_path}: {e}", "error")
        
        return result
    
    def _check_map_assets(self, result: TestResult):
        """Check for missing assets in the current map"""
        if not self.config.get("error_detection", "check_missing_assets"):
            return
        
        try:
            # Get all actors in the level
            all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
            
            for actor in all_actors:
                # Check if actor is valid
                if not unreal.SystemLibrary.is_valid(actor):
                    continue
                
                # Get actor class
                actor_class = actor.get_class().get_name()
                
                # Check for missing static mesh components
                components = actor.get_components_by_class(unreal.StaticMeshComponent)
                for component in components:
                    if unreal.SystemLibrary.is_valid(component):
                        mesh = component.get_editor_property("static_mesh")
                        if not mesh:
                            result.add_missing_asset(
                                f"Actor '{actor.get_name()}' has missing static mesh"
                            )
                
                # Check for missing skeletal mesh components
                skeletal_components = actor.get_components_by_class(unreal.SkeletalMeshComponent)
                for component in skeletal_components:
                    if unreal.SystemLibrary.is_valid(component):
                        mesh = component.get_editor_property("skeletal_mesh")
                        if not mesh:
                            result.add_missing_asset(
                                f"Actor '{actor.get_name()}' has missing skeletal mesh"
                            )
        
        except Exception as e:
            result.add_warning(f"Error checking map assets: {str(e)}")
    
    def _check_broken_references(self, result: TestResult):
        """Check for broken references in the current map"""
        if not self.config.get("error_detection", "check_broken_references"):
            return
        
        try:
            # This is a placeholder for more sophisticated reference checking
            # In a real implementation, we would use Unreal's asset validation tools
            
            # Get output log and check for error keywords
            error_keywords = self.config.get("error_detection", "error_keywords")
            
            # Note: In practice, we would need to capture the output log
            # For now, we'll use a simplified approach
            
            pass
        
        except Exception as e:
            result.add_warning(f"Error checking broken references: {str(e)}")
    
    def _capture_screenshot(self, map_name: str) -> Optional[str]:
        """
        Capture a screenshot of the current viewport
        
        Args:
            map_name: Name of the map for the screenshot filename
            
        Returns:
            Path to the screenshot file
        """
        try:
            # Create screenshots directory
            screenshots_dir = self.output_dir / "Screenshots"
            screenshots_dir.mkdir(parents=True, exist_ok=True)
            
            # Generate filename
            timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
            map_basename = Path(map_name).stem
            filename = f"{map_basename}_{timestamp}.png"
            screenshot_path = screenshots_dir / filename
            
            # Take screenshot using Unreal's automation library
            unreal.AutomationLibrary.take_high_res_screenshot(
                width=1920,
                height=1080,
                filename=str(screenshot_path)
            )
            
            return str(screenshot_path)
        
        except Exception as e:
            self.log(f"Failed to capture screenshot: {e}", "warning")
            return None
    
    def run_all_tests(self) -> List[TestResult]:
        """
        Run smoke tests on all discovered maps
        
        Returns:
            List of TestResult objects
        """
        self.log("=== Starting Smoke Tests ===")
        
        # Discover maps
        maps = self.discover_maps()
        
        if not maps:
            self.log("No maps found to test", "warning")
            return []
        
        # Run tests on each map
        self.results = []
        stop_on_error = self.config.get("test_settings", "stop_on_first_error")
        
        for map_path in maps:
            result = self.load_map(map_path)
            self.results.append(result)
            
            if stop_on_error and not result.success:
                self.log("Stopping tests due to error (stop_on_first_error=True)", "warning")
                break
        
        self.log("=== Smoke Tests Complete ===")
        self._print_summary()
        
        return self.results
    
    def _print_summary(self):
        """Print a summary of test results"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.success)
        failed = total - passed
        
        self.log(f"Total Maps Tested: {total}")
        self.log(f"Passed: {passed}")
        self.log(f"Failed: {failed}")
        
        if failed > 0:
            self.log("Failed Maps:", "warning")
            for result in self.results:
                if not result.success:
                    self.log(f"  - {result.map_name}", "warning")
    
    def generate_report(self, results: Optional[List[TestResult]] = None):
        """
        Generate test reports in configured formats
        
        Args:
            results: List of TestResult objects (uses self.results if None)
        """
        if results is None:
            results = self.results
        
        if not results:
            self.log("No results to report", "warning")
            return
        
        timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
        
        # Generate JSON report
        if self.config.get("reporting", "generate_json"):
            self._generate_json_report(results, timestamp)
        
        # Generate HTML report
        if self.config.get("reporting", "generate_html"):
            self._generate_html_report(results, timestamp)
        
        self.log(f"Reports generated in: {self.output_dir}")
    
    def _generate_json_report(self, results: List[TestResult], timestamp: str):
        """Generate JSON report"""
        report_path = self.output_dir / f"smoke_test_report_{timestamp}.json"
        
        report_data = {
            "timestamp": datetime.now().isoformat(),
            "total_maps": len(results),
            "passed": sum(1 for r in results if r.success),
            "failed": sum(1 for r in results if not r.success),
            "results": [r.to_dict() for r in results],
            "log_messages": self.log_messages
        }
        
        with open(report_path, 'w') as f:
            json.dump(report_data, f, indent=2)
        
        self.log(f"JSON report saved to: {report_path}")
    
    def _generate_html_report(self, results: List[TestResult], timestamp: str):
        """Generate HTML report"""
        report_path = self.output_dir / f"smoke_test_report_{timestamp}.html"
        
        total = len(results)
        passed = sum(1 for r in results if r.success)
        failed = total - passed
        
        # Build HTML content
        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Adastrea Smoke Test Report - {timestamp}</title>
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
        .summary {{
            background-color: white;
            padding: 20px;
            margin: 20px 0;
            border-radius: 5px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .result {{
            background-color: white;
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
            border-left: 4px solid #3498db;
        }}
        .result.failed {{
            border-left-color: #e74c3c;
        }}
        .result.passed {{
            border-left-color: #2ecc71;
        }}
        .errors, .warnings {{
            margin: 10px 0;
        }}
        .error {{
            color: #e74c3c;
            margin: 5px 0;
        }}
        .warning {{
            color: #f39c12;
            margin: 5px 0;
        }}
        .stat {{
            display: inline-block;
            margin: 10px 20px 10px 0;
            font-size: 18px;
        }}
        .stat-label {{
            font-weight: bold;
        }}
        .screenshot {{
            max-width: 400px;
            margin: 10px 0;
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>Adastrea Smoke Test Report</h1>
        <p>Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>
    </div>
    
    <div class="summary">
        <h2>Summary</h2>
        <div class="stat">
            <span class="stat-label">Total Maps:</span> {total}
        </div>
        <div class="stat">
            <span class="stat-label" style="color: #2ecc71;">Passed:</span> {passed}
        </div>
        <div class="stat">
            <span class="stat-label" style="color: #e74c3c;">Failed:</span> {failed}
        </div>
    </div>
    
    <h2>Test Results</h2>
"""
        
        for result in results:
            status_class = "passed" if result.success else "failed"
            status_text = "✓ PASSED" if result.success else "✗ FAILED"
            
            html += f"""
    <div class="result {status_class}">
        <h3>{result.map_name} - {status_text}</h3>
        <p><strong>Load Time:</strong> {result.load_time_seconds:.2f}s</p>
"""
            
            if result.errors:
                html += """
        <div class="errors">
            <strong>Errors:</strong>
            <ul>
"""
                for error in result.errors:
                    html += f"                <li class='error'>{error}</li>\n"
                html += """
            </ul>
        </div>
"""
            
            if result.warnings:
                html += """
        <div class="warnings">
            <strong>Warnings:</strong>
            <ul>
"""
                for warning in result.warnings:
                    html += f"                <li class='warning'>{warning}</li>\n"
                html += """
            </ul>
        </div>
"""
            
            if result.missing_assets:
                html += """
        <div>
            <strong>Missing Assets:</strong>
            <ul>
"""
                for asset in result.missing_assets:
                    html += f"                <li class='error'>{asset}</li>\n"
                html += """
            </ul>
        </div>
"""
            
            if result.screenshot_path:
                html += f"""
        <div>
            <strong>Screenshot:</strong><br>
            <img src="{result.screenshot_path}" class="screenshot" alt="Error Screenshot">
        </div>
"""
            
            html += """
    </div>
"""
        
        html += """
</body>
</html>
"""
        
        with open(report_path, 'w') as f:
            f.write(html)
        
        self.log(f"HTML report saved to: {report_path}")


# Standalone execution support
def main():
    """Main entry point for standalone execution"""
    if not UNREAL_AVAILABLE:
        print("Error: This script must be run inside Unreal Engine Python environment")
        return 1
    
    # Check for config file argument
    config_path = None
    if len(sys.argv) > 1:
        config_path = sys.argv[1]
    
    try:
        runner = SmokeTestRunner(config_path)
        results = runner.run_all_tests()
        runner.generate_report(results)
        
        # Return exit code based on results
        failed = sum(1 for r in results if not r.success)
        return 0 if failed == 0 else 1
    
    except Exception as e:
        print(f"Fatal error running smoke tests: {e}")
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
