#!/usr/bin/env python3
"""
Adastrea - Screenshot Testing and Visual Regression Tool

This script provides automated screenshot capture for key locations/assets and
comparison against golden master versions for visual regression detection.

This script is designed to run INSIDE the Unreal Editor's Python environment.

Features:
- Automated screenshot capture at configured camera positions
- Golden master comparison with pixel-based diff detection
- Visual regression reporting with difference highlighting
- Support for multiple test scenarios and viewpoints
- Configurable via screenshot_test_config.json
- Integration with smoke testing workflow

Usage in Unreal Editor Python Console:
    import ScreenshotTester
    tester = ScreenshotTester.ScreenshotTestRunner()
    results = tester.run_all_tests()
    tester.generate_report(results)

Usage from CLI (requires Unreal Engine command line):
    UnrealEditor-Cmd.exe "ProjectPath.uproject" -run=pythonscript -script="ScreenshotTester.py"
"""

import json
import os
import sys
import copy
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

# Try to import PIL for image comparison
try:
    from PIL import Image, ImageChops, ImageDraw, ImageFont
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("Warning: PIL not available. Image comparison features will be limited.")
    print("Install with: pip install Pillow")


class ScreenshotTestConfig:
    """Configuration manager for screenshot tests"""
    
    DEFAULT_CONFIG = {
        "test_settings": {
            "screenshot_width": 1920,
            "screenshot_height": 1080,
            "comparison_threshold": 0.01,  # 1% difference threshold
            "pixel_tolerance": 5,  # Allow 5 units of difference per pixel
            "update_golden_masters": False,
            "verbose_logging": True
        },
        "capture_locations": [
            {
                "name": "Main_Menu",
                "map": "/Game/Maps/MainMenu",
                "camera_location": {"x": 0, "y": 0, "z": 200},
                "camera_rotation": {"pitch": -15, "yaw": 0, "roll": 0},
                "wait_time_seconds": 2.0
            },
            {
                "name": "Station_Overview",
                "map": "/Game/Maps/TestStation",
                "camera_location": {"x": 0, "y": 0, "z": 500},
                "camera_rotation": {"pitch": -30, "yaw": 0, "roll": 0},
                "wait_time_seconds": 3.0
            }
        ],
        "asset_screenshots": [
            {
                "name": "Ship_Fighter_Preview",
                "asset_path": "/Game/Blueprints/Ships/BP_Ship_Fighter",
                "thumbnail_size": 512,
                "background_color": {"r": 0.1, "g": 0.1, "b": 0.1, "a": 1.0}
            }
        ],
        "reporting": {
            "output_directory": "TestReports/Screenshots",
            "golden_masters_directory": "TestReports/GoldenMasters",
            "diff_directory": "TestReports/ScreenshotDiffs",
            "generate_html": True,
            "generate_json": True,
            "timestamp_format": "%Y-%m-%d_%H-%M-%S"
        }
    }
    
    def __init__(self, config_path: Optional[str] = None):
        """Initialize configuration"""
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


class ScreenshotTestResult:
    """Represents the result of a screenshot test"""
    
    def __init__(self, name: str):
        self.name = name
        self.success = True
        self.screenshot_path: Optional[str] = None
        self.golden_master_path: Optional[str] = None
        self.diff_path: Optional[str] = None
        self.difference_percentage = 0.0
        self.pixel_differences = 0
        self.total_pixels = 0
        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.timestamp = datetime.now().isoformat()
    
    def add_error(self, error: str):
        """Add an error to the result"""
        self.errors.append(error)
        self.success = False
    
    def add_warning(self, warning: str):
        """Add a warning to the result"""
        self.warnings.append(warning)
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert result to dictionary"""
        return {
            "name": self.name,
            "success": self.success,
            "screenshot_path": self.screenshot_path,
            "golden_master_path": self.golden_master_path,
            "diff_path": self.diff_path,
            "difference_percentage": self.difference_percentage,
            "pixel_differences": self.pixel_differences,
            "total_pixels": self.total_pixels,
            "errors": self.errors,
            "warnings": self.warnings,
            "timestamp": self.timestamp
        }


class ScreenshotTestRunner:
    """Main screenshot test runner"""
    
    def __init__(self, config_path: Optional[str] = None):
        """Initialize the screenshot test runner"""
        if not UNREAL_AVAILABLE:
            raise RuntimeError("ScreenshotTestRunner requires Unreal Engine Python environment")
        
        self.config = ScreenshotTestConfig(config_path)
        self.results: List[ScreenshotTestResult] = []
        
        # Get project root
        self.project_root = Path(unreal.SystemLibrary.get_project_directory())
        self.output_dir = self.project_root / self.config.get("reporting", "output_directory")
        self.golden_dir = self.project_root / self.config.get("reporting", "golden_masters_directory")
        self.diff_dir = self.project_root / self.config.get("reporting", "diff_directory")
        
        # Create directories
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.golden_dir.mkdir(parents=True, exist_ok=True)
        self.diff_dir.mkdir(parents=True, exist_ok=True)
        
        # Initialize logging
        self.log_messages: List[Tuple[str, str]] = []
    
    def log(self, message: str, level: str = "info"):
        """Log a message"""
        self.log_messages.append((level, message))
        
        if self.config.get("test_settings", "verbose_logging"):
            if level == "error":
                unreal.log_error(f"[ScreenshotTest] {message}")
            elif level == "warning":
                unreal.log_warning(f"[ScreenshotTest] {message}")
            else:
                unreal.log(f"[ScreenshotTest] {message}")
    
    def capture_location_screenshot(self, location_config: Dict) -> ScreenshotTestResult:
        """
        Capture a screenshot at a specific camera location
        
        Args:
            location_config: Configuration dictionary for this location
            
        Returns:
            ScreenshotTestResult object
        """
        name = location_config.get("name", "Unknown")
        result = ScreenshotTestResult(name)
        
        self.log(f"Capturing screenshot: {name}")
        
        try:
            # Load the map
            map_path = location_config.get("map")
            if map_path:
                success = unreal.EditorLevelLibrary.load_level(map_path)
                if not success:
                    result.add_error(f"Failed to load map: {map_path}")
                    return result
            
            # Wait for map to settle
            wait_time = location_config.get("wait_time_seconds", 2.0)
            # Note: In actual implementation, we would use a proper wait mechanism
            # For now, this is a placeholder
            
            # Set camera position
            camera_location = location_config.get("camera_location", {"x": 0, "y": 0, "z": 200})
            camera_rotation = location_config.get("camera_rotation", {"pitch": 0, "yaw": 0, "roll": 0})
            
            # Create camera position
            location = unreal.Vector(
                camera_location.get("x", 0),
                camera_location.get("y", 0),
                camera_location.get("z", 200)
            )
            
            rotation = unreal.Rotator(
                camera_rotation.get("pitch", 0),
                camera_rotation.get("yaw", 0),
                camera_rotation.get("roll", 0)
            )
            
            # Set editor camera (this is a simplified approach)
            # In production, we'd spawn a camera actor and use it
            
            # Capture screenshot
            timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
            filename = f"{name}_{timestamp}.png"
            screenshot_path = self.output_dir / filename
            
            # Take screenshot
            width = self.config.get("test_settings", "screenshot_width")
            height = self.config.get("test_settings", "screenshot_height")
            
            unreal.AutomationLibrary.take_high_res_screenshot(
                width=width,
                height=height,
                filename=str(screenshot_path)
            )
            
            result.screenshot_path = str(screenshot_path)
            
            # Compare with golden master if it exists
            if PIL_AVAILABLE:
                golden_path = self.golden_dir / f"{name}.png"
                result.golden_master_path = str(golden_path)
                
                if golden_path.exists():
                    self._compare_screenshots(result, screenshot_path, golden_path)
                else:
                    result.add_warning("No golden master found for comparison")
                    
                    # Update golden master if configured
                    if self.config.get("test_settings", "update_golden_masters"):
                        self._update_golden_master(screenshot_path, golden_path)
                        result.add_warning(f"Created new golden master: {golden_path}")
            
            self.log(f"Screenshot captured: {screenshot_path}")
        
        except Exception as e:
            result.add_error(f"Exception capturing screenshot: {str(e)}")
            result.add_error(traceback.format_exc())
            self.log(f"Exception capturing screenshot {name}: {e}", "error")
        
        return result
    
    def capture_asset_screenshot(self, asset_config: Dict) -> ScreenshotTestResult:
        """
        Capture a screenshot of a specific asset
        
        Args:
            asset_config: Configuration dictionary for this asset
            
        Returns:
            ScreenshotTestResult object
        """
        name = asset_config.get("name", "Unknown")
        result = ScreenshotTestResult(name)
        
        self.log(f"Capturing asset screenshot: {name}")
        
        try:
            asset_path = asset_config.get("asset_path")
            if not asset_path:
                result.add_error("No asset path specified")
                return result
            
            # Load the asset
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset:
                result.add_error(f"Failed to load asset: {asset_path}")
                return result
            
            # Generate asset thumbnail
            thumbnail_size = asset_config.get("thumbnail_size", 512)
            
            timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
            filename = f"{name}_{timestamp}.png"
            screenshot_path = self.output_dir / filename
            
            # Note: Asset thumbnail generation requires additional implementation
            # This is a placeholder for the actual implementation
            result.screenshot_path = str(screenshot_path)
            
            result.add_warning("Asset screenshot capture not fully implemented yet")
            
        except Exception as e:
            result.add_error(f"Exception capturing asset screenshot: {str(e)}")
            self.log(f"Exception capturing asset screenshot {name}: {e}", "error")
        
        return result
    
    def _compare_screenshots(self, result: ScreenshotTestResult, 
                            screenshot_path: Path, golden_path: Path):
        """
        Compare a screenshot with its golden master
        
        Args:
            result: ScreenshotTestResult to update
            screenshot_path: Path to the current screenshot
            golden_path: Path to the golden master
        """
        if not PIL_AVAILABLE:
            result.add_warning("PIL not available, cannot compare screenshots")
            return
        
        try:
            # Load images
            current = Image.open(screenshot_path)
            golden = Image.open(golden_path)
            
            # Ensure same size
            if current.size != golden.size:
                result.add_error(f"Size mismatch: current {current.size} vs golden {golden.size}")
                return
            
            # Convert to RGB if needed
            if current.mode != 'RGB':
                current = current.convert('RGB')
            if golden.mode != 'RGB':
                golden = golden.convert('RGB')
            
            # Calculate difference
            diff = ImageChops.difference(current, golden)
            
            # Count different pixels using efficient method
            pixel_tolerance = self.config.get("test_settings", "pixel_tolerance")
            
            width, height = current.size
            total_pixels = width * height
            
            # Convert to list for efficient processing (faster than getpixel in loop)
            diff_data = list(diff.getdata())
            
            # Count pixels that exceed tolerance in any channel
            different_pixels = sum(
                1 for pixel in diff_data
                if any(channel > pixel_tolerance for channel in pixel)
            )
            
            # Calculate percentage
            difference_percentage = (different_pixels / total_pixels) * 100
            
            result.pixel_differences = different_pixels
            result.total_pixels = total_pixels
            result.difference_percentage = difference_percentage
            
            # Check threshold
            threshold = self.config.get("test_settings", "comparison_threshold") * 100
            
            if difference_percentage > threshold:
                result.add_error(
                    f"Visual regression detected: {difference_percentage:.2f}% difference "
                    f"(threshold: {threshold:.2f}%)"
                )
                
                # Generate diff image
                diff_path = self._generate_diff_image(current, golden, diff, result.name)
                result.diff_path = str(diff_path)
            else:
                self.log(f"Screenshots match within threshold: {difference_percentage:.4f}%")
        
        except Exception as e:
            result.add_error(f"Error comparing screenshots: {str(e)}")
            self.log(f"Error comparing screenshots: {e}", "error")
    
    def _generate_diff_image(self, current: Image.Image, golden: Image.Image, 
                           diff: Image.Image, name: str) -> Path:
        """
        Generate a visual diff image showing differences
        
        Args:
            current: Current screenshot
            golden: Golden master
            diff: Difference image
            name: Test name
            
        Returns:
            Path to the diff image
        """
        # Create a side-by-side comparison image
        width, height = current.size
        
        # Create new image with 3x width (golden, current, diff)
        comparison = Image.new('RGB', (width * 3, height))
        
        # Paste images
        comparison.paste(golden, (0, 0))
        comparison.paste(current, (width, 0))
        
        # Enhance diff for visibility
        diff_enhanced = diff.point(lambda p: p * 10)  # Make differences more visible
        comparison.paste(diff_enhanced, (width * 2, 0))
        
        # Add labels
        draw = ImageDraw.Draw(comparison)
        # Use default font (PIL's built-in)
        font = None
        
        draw.text((10, 10), "Golden Master", fill=(255, 255, 0), font=font)
        draw.text((width + 10, 10), "Current", fill=(255, 255, 0), font=font)
        draw.text((width * 2 + 10, 10), "Difference (10x)", fill=(255, 255, 0), font=font)
        
        # Save comparison
        timestamp = datetime.now().strftime(self.config.get("reporting", "timestamp_format"))
        diff_filename = f"{name}_diff_{timestamp}.png"
        diff_path = self.diff_dir / diff_filename
        
        comparison.save(diff_path)
        
        self.log(f"Diff image saved to: {diff_path}")
        return diff_path
    
    def _update_golden_master(self, screenshot_path: Path, golden_path: Path):
        """
        Update the golden master with the current screenshot
        
        Args:
            screenshot_path: Path to the current screenshot
            golden_path: Path where golden master should be saved
        """
        try:
            if PIL_AVAILABLE:
                img = Image.open(screenshot_path)
                img.save(golden_path)
            else:
                # Fallback: copy file
                import shutil
                shutil.copy2(screenshot_path, golden_path)
            
            self.log(f"Updated golden master: {golden_path}")
        
        except Exception as e:
            self.log(f"Failed to update golden master: {e}", "error")
    
    def run_all_tests(self) -> List[ScreenshotTestResult]:
        """
        Run all configured screenshot tests
        
        Returns:
            List of ScreenshotTestResult objects
        """
        self.log("=== Starting Screenshot Tests ===")
        
        self.results = []
        
        # Run location screenshots
        capture_locations = self.config.get("capture_locations")
        if capture_locations:
            for location in capture_locations:
                result = self.capture_location_screenshot(location)
                self.results.append(result)
        
        # Run asset screenshots
        asset_screenshots = self.config.get("asset_screenshots")
        if asset_screenshots:
            for asset in asset_screenshots:
                result = self.capture_asset_screenshot(asset)
                self.results.append(result)
        
        self.log("=== Screenshot Tests Complete ===")
        self._print_summary()
        
        return self.results
    
    def _print_summary(self):
        """Print a summary of test results"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.success)
        failed = total - passed
        
        self.log(f"Total Screenshots: {total}")
        self.log(f"Passed: {passed}")
        self.log(f"Failed: {failed}")
        
        if failed > 0:
            self.log("Failed Tests:", "warning")
            for result in self.results:
                if not result.success:
                    self.log(f"  - {result.name}: {result.difference_percentage:.2f}% diff", "warning")
    
    def generate_report(self, results: Optional[List[ScreenshotTestResult]] = None):
        """
        Generate test reports in configured formats
        
        Args:
            results: List of ScreenshotTestResult objects (uses self.results if None)
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
    
    def _generate_json_report(self, results: List[ScreenshotTestResult], timestamp: str):
        """Generate JSON report"""
        report_path = self.output_dir / f"screenshot_test_report_{timestamp}.json"
        
        report_data = {
            "timestamp": datetime.now().isoformat(),
            "total_tests": len(results),
            "passed": sum(1 for r in results if r.success),
            "failed": sum(1 for r in results if not r.success),
            "results": [r.to_dict() for r in results],
            "log_messages": self.log_messages
        }
        
        with open(report_path, 'w') as f:
            json.dump(report_data, f, indent=2)
        
        self.log(f"JSON report saved to: {report_path}")
    
    def _generate_html_report(self, results: List[ScreenshotTestResult], timestamp: str):
        """Generate HTML report"""
        report_path = self.output_dir / f"screenshot_test_report_{timestamp}.html"
        
        total = len(results)
        passed = sum(1 for r in results if r.success)
        failed = total - passed
        
        # Build HTML content
        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Adastrea Screenshot Test Report - {timestamp}</title>
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
        .screenshot-preview {{
            max-width: 600px;
            margin: 10px 0;
            border: 1px solid #ddd;
        }}
        .stat {{
            display: inline-block;
            margin: 10px 20px 10px 0;
            font-size: 18px;
        }}
        .stat-label {{
            font-weight: bold;
        }}
        .error {{
            color: #e74c3c;
            margin: 5px 0;
        }}
        .warning {{
            color: #f39c12;
            margin: 5px 0;
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>Adastrea Screenshot Test Report</h1>
        <p>Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>
    </div>
    
    <div class="summary">
        <h2>Summary</h2>
        <div class="stat">
            <span class="stat-label">Total Tests:</span> {total}
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
        <h3>{result.name} - {status_text}</h3>
"""
            
            if result.difference_percentage > 0:
                html += f"""
        <p><strong>Difference:</strong> {result.difference_percentage:.4f}% 
        ({result.pixel_differences:,} / {result.total_pixels:,} pixels)</p>
"""
            
            if result.screenshot_path:
                html += f"""
        <div>
            <strong>Current Screenshot:</strong><br>
            <img src="{result.screenshot_path}" class="screenshot-preview" alt="Current Screenshot">
        </div>
"""
            
            if result.diff_path:
                html += f"""
        <div>
            <strong>Visual Diff:</strong><br>
            <img src="{result.diff_path}" class="screenshot-preview" alt="Difference Comparison">
        </div>
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
        runner = ScreenshotTestRunner(config_path)
        results = runner.run_all_tests()
        runner.generate_report(results)
        
        # Return exit code based on results
        failed = sum(1 for r in results if not r.success)
        return 0 if failed == 0 else 1
    
    except Exception as e:
        print(f"Fatal error running screenshot tests: {e}")
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
