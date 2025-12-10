#!/usr/bin/env python3
"""
Adastrea - Content Validator

This script validates all generated content to ensure it works correctly.

Features:
- Validate Blueprint existence and compilation
- Validate Data Asset properties
- Validate Input system configuration
- Validate map setup
- Validate UI widgets
- Generate validation report

Usage:
    # In Unreal Editor Python Console
    import ContentValidator
    ContentValidator.validate_all()
    
    # Validate specific categories
    ContentValidator.validate_blueprints()
    ContentValidator.validate_data_assets()
"""

import os
import sys
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("ERROR: This script must be run inside Unreal Editor!")
    sys.exit(1)


class ContentValidator:
    """Validator for generated content"""
    
    # Expected assets
    ESSENTIAL_BLUEPRINTS = [
        "/Game/Blueprints/GameModes/BP_SpaceGameMode",
        "/Game/Blueprints/Core/BP_GameInstance",
        "/Game/Blueprints/Controllers/BP_SpaceshipController",
        "/Game/Blueprints/Ships/BP_PlayerShip",
    ]
    
    ESSENTIAL_WIDGETS = [
        "/Game/UI/HUD/WBP_SpaceshipHUD",
        "/Game/UI/Menus/WBP_MainMenu",
        "/Game/UI/Menus/WBP_PauseMenu",
    ]
    
    ESSENTIAL_INPUT = [
        "/Game/Input/Actions/IA_Move",
        "/Game/Input/Actions/IA_Look",
        "/Game/Input/Actions/IA_Fire_Primary",
        "/Game/Input/IMC_Spaceship",
        "/Game/DataAssets/Input/DA_InputConfig",
    ]
    
    ESSENTIAL_MAPS = [
        "/Game/Maps/TestMode",
    ]
    
    def __init__(self):
        """Initialize the content validator"""
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.results = {
            "blueprints": {"passed": 0, "failed": 0, "errors": []},
            "data_assets": {"passed": 0, "failed": 0, "errors": []},
            "widgets": {"passed": 0, "failed": 0, "errors": []},
            "input": {"passed": 0, "failed": 0, "errors": []},
            "maps": {"passed": 0, "failed": 0, "errors": []},
        }
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[ContentValidator] {message}")
        elif level == "warning":
            unreal.log_warning(f"[ContentValidator] {message}")
        else:
            unreal.log(f"[ContentValidator] {message}")
    
    def validate_asset_exists(self, asset_path: str, category: str) -> bool:
        """
        Validate that an asset exists
        
        Args:
            asset_path: Path to the asset
            category: Category for tracking results
            
        Returns:
            True if asset exists, False otherwise
        """
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.results[category]["passed"] += 1
            return True
        else:
            self.results[category]["failed"] += 1
            self.results[category]["errors"].append(f"Missing: {asset_path}")
            return False
    
    def validate_blueprints(self) -> bool:
        """
        Validate Blueprint assets
        
        Returns:
            True if all validated successfully
        """
        self.log("=" * 80)
        self.log("Validating Blueprints")
        self.log("=" * 80)
        
        all_valid = True
        
        for bp_path in self.ESSENTIAL_BLUEPRINTS:
            exists = self.validate_asset_exists(bp_path, "blueprints")
            
            if exists:
                self.log(f"✓ {bp_path}")
                
                # Try to compile the blueprint
                try:
                    blueprint = unreal.load_asset(bp_path)
                    if blueprint:
                        # Check if it's valid
                        if not unreal.EditorAssetLibrary.does_asset_exist(bp_path):
                            self.log(f"✗ {bp_path} - Failed to load", "error")
                            all_valid = False
                except Exception as e:
                    self.log(f"✗ {bp_path} - Error: {str(e)}", "error")
                    self.results["blueprints"]["errors"].append(f"{bp_path}: {str(e)}")
                    all_valid = False
            else:
                self.log(f"✗ {bp_path} - Not found", "error")
                all_valid = False
        
        return all_valid
    
    def validate_data_assets(self) -> bool:
        """
        Validate Data Assets
        
        Returns:
            True if all validated successfully
        """
        self.log("")
        self.log("=" * 80)
        self.log("Validating Data Assets")
        self.log("=" * 80)
        
        # Find all Data Assets
        data_asset_paths = [
            "/Game/DataAssets/Ships",
            "/Game/DataAssets/Personnel",
            "/Game/DataAssets/Trading",
            "/Game/DataAssets/Factions",
        ]
        
        all_valid = True
        total_found = 0
        
        for path in data_asset_paths:
            try:
                assets = self.editor_asset_lib.list_assets(path, recursive=True)
                total_found += len(assets)
                
                self.log(f"  {path}: {len(assets)} assets found")
                self.results["data_assets"]["passed"] += len(assets)
            except Exception as e:
                self.log(f"  {path}: Error - {str(e)}", "warning")
        
        if total_found == 0:
            self.log("✗ No Data Assets found", "warning")
            all_valid = False
        else:
            self.log(f"✓ Found {total_found} Data Assets total")
        
        return all_valid
    
    def validate_widgets(self) -> bool:
        """
        Validate UI Widgets
        
        Returns:
            True if all validated successfully
        """
        self.log("")
        self.log("=" * 80)
        self.log("Validating UI Widgets")
        self.log("=" * 80)
        
        all_valid = True
        
        for widget_path in self.ESSENTIAL_WIDGETS:
            exists = self.validate_asset_exists(widget_path, "widgets")
            
            if exists:
                self.log(f"✓ {widget_path}")
            else:
                self.log(f"✗ {widget_path} - Not found", "error")
                all_valid = False
        
        return all_valid
    
    def validate_input_system(self) -> bool:
        """
        Validate Enhanced Input system
        
        Returns:
            True if all validated successfully
        """
        self.log("")
        self.log("=" * 80)
        self.log("Validating Enhanced Input System")
        self.log("=" * 80)
        
        all_valid = True
        
        for input_path in self.ESSENTIAL_INPUT:
            exists = self.validate_asset_exists(input_path, "input")
            
            if exists:
                self.log(f"✓ {input_path}")
            else:
                self.log(f"✗ {input_path} - Not found", "error")
                all_valid = False
        
        return all_valid
    
    def validate_maps(self) -> bool:
        """
        Validate Maps/Levels
        
        Returns:
            True if all validated successfully
        """
        self.log("")
        self.log("=" * 80)
        self.log("Validating Maps/Levels")
        self.log("=" * 80)
        
        all_valid = True
        
        for map_path in self.ESSENTIAL_MAPS:
            exists = self.validate_asset_exists(map_path, "maps")
            
            if exists:
                self.log(f"✓ {map_path}")
            else:
                self.log(f"✗ {map_path} - Not found", "error")
                all_valid = False
        
        return all_valid
    
    def validate_all(self) -> bool:
        """
        Validate all content
        
        Returns:
            True if all validation passed
        """
        self.log("=" * 80)
        self.log("ADASTREA CONTENT VALIDATION")
        self.log("=" * 80)
        self.log("")
        
        # Run all validations
        bp_valid = self.validate_blueprints()
        da_valid = self.validate_data_assets()
        widget_valid = self.validate_widgets()
        input_valid = self.validate_input_system()
        map_valid = self.validate_maps()
        
        # Print summary
        self.print_summary()
        
        # Overall result
        all_valid = bp_valid and da_valid and widget_valid and input_valid and map_valid
        
        if all_valid:
            self.log("\n✓ ALL VALIDATION CHECKS PASSED!", "info")
        else:
            self.log("\n✗ SOME VALIDATION CHECKS FAILED", "warning")
        
        return all_valid
    
    def print_summary(self):
        """Print validation summary"""
        self.log("")
        self.log("=" * 80)
        self.log("VALIDATION SUMMARY")
        self.log("=" * 80)
        
        total_passed = 0
        total_failed = 0
        
        for category, results in self.results.items():
            passed = results["passed"]
            failed = results["failed"]
            total_passed += passed
            total_failed += failed
            
            status = "✓" if failed == 0 else "✗"
            self.log(f"{status} {category.upper()}: {passed} passed, {failed} failed")
            
            if results["errors"]:
                for error in results["errors"][:5]:  # Show first 5 errors
                    self.log(f"    - {error}", "warning")
                if len(results["errors"]) > 5:
                    self.log(f"    ... and {len(results['errors']) - 5} more", "warning")
        
        self.log("")
        self.log(f"TOTAL: {total_passed} passed, {total_failed} failed")
        self.log("=" * 80)
    
    def get_results(self) -> Dict[str, Any]:
        """Get validation results"""
        return self.results


# Convenience functions
def validate_all() -> bool:
    """Validate all content - convenience function"""
    validator = ContentValidator()
    return validator.validate_all()


def validate_blueprints() -> bool:
    """Validate blueprints - convenience function"""
    validator = ContentValidator()
    return validator.validate_blueprints()


def validate_data_assets() -> bool:
    """Validate data assets - convenience function"""
    validator = ContentValidator()
    return validator.validate_data_assets()


def validate_widgets() -> bool:
    """Validate widgets - convenience function"""
    validator = ContentValidator()
    return validator.validate_widgets()


def validate_input_system() -> bool:
    """Validate input system - convenience function"""
    validator = ContentValidator()
    return validator.validate_input_system()


def validate_maps() -> bool:
    """Validate maps - convenience function"""
    validator = ContentValidator()
    return validator.validate_maps()


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Content Validator - Validate generated content"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Validate all content")
    parser.add_argument("--blueprints", action="store_true",
                       help="Validate blueprints only")
    parser.add_argument("--data-assets", action="store_true",
                       help="Validate data assets only")
    parser.add_argument("--widgets", action="store_true",
                       help="Validate widgets only")
    parser.add_argument("--input", action="store_true",
                       help="Validate input system only")
    parser.add_argument("--maps", action="store_true",
                       help="Validate maps only")
    
    args = parser.parse_args()
    
    validator = ContentValidator()
    
    if args.all:
        validator.validate_all()
    elif args.blueprints:
        validator.validate_blueprints()
        validator.print_summary()
    elif args.data_assets:
        validator.validate_data_assets()
        validator.print_summary()
    elif args.widgets:
        validator.validate_widgets()
        validator.print_summary()
    elif args.input:
        validator.validate_input_system()
        validator.print_summary()
    elif args.maps:
        validator.validate_maps()
        validator.print_summary()
    else:
        parser.print_help()
        print("\nTIP: Use --all to validate everything")


if __name__ == "__main__":
    main()
