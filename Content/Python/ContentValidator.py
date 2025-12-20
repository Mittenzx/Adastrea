#!/usr/bin/env python3
"""
Content Validator for Adastrea

Validates generated content for consistency, completeness, and correctness.
Checks Blueprints, Data Assets, and other generated content for issues.

Usage:
    from ContentValidator import ContentValidator
    validator = ContentValidator()
    validator.validate_all_content()
"""

import unreal
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class ContentValidator:
    """
    Validates generated content for Adastrea
    """

    def __init__(self):
        """Initialize the content validator"""
        self.asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.validation_errors = []
        self.validation_warnings = []

    def validate_all_content(self) -> bool:
        """
        Validate all generated content

        Returns:
            True if validation passes, False if there are errors
        """
        print("[ContentValidator] Starting content validation...")

        self.validation_errors = []
        self.validation_warnings = []

        # Validate different types of content
        self._validate_blueprints()
        self._validate_data_assets()
        self._validate_ui_widgets()
        self._validate_niagara_systems()

        # Report results
        self._report_validation_results()

        # Return True if no errors
        return len(self.validation_errors) == 0

    def _validate_blueprints(self):
        """Validate Blueprint assets"""
        print("[ContentValidator] Validating Blueprints...")

        try:
            # Find all Blueprint assets
            blueprint_filter = unreal.ARFilter(
                class_names=["Blueprint"],
                recursive_paths=True
            )

            blueprint_assets = self.asset_registry.get_assets(blueprint_filter)

            for asset_data in blueprint_assets:
                asset_path = asset_data.asset_name
                if "/Game/Blueprints" in asset_path:
                    self._validate_single_blueprint(asset_path)

        except Exception as e:
            self.validation_errors.append(f"Blueprint validation failed: {e}")

    def _validate_data_assets(self):
        """Validate Data Asset files"""
        print("[ContentValidator] Validating Data Assets...")

        try:
            # Find all Data Assets
            data_asset_filter = unreal.ARFilter(
                class_names=["DataAsset"],
                recursive_paths=True
            )

            data_assets = self.asset_registry.get_assets(data_asset_filter)

            for asset_data in data_assets:
                asset_path = asset_data.asset_name
                if "/Game/DataAssets" in asset_path:
                    self._validate_single_data_asset(asset_path)

        except Exception as e:
            self.validation_errors.append(f"Data Asset validation failed: {e}")

    def _validate_ui_widgets(self):
        """Validate UI widget blueprints"""
        print("[ContentValidator] Validating UI Widgets...")

        try:
            # Find all Widget Blueprints
            widget_filter = unreal.ARFilter(
                class_names=["WidgetBlueprint"],
                recursive_paths=True
            )

            widgets = self.asset_registry.get_assets(widget_filter)

            for asset_data in widgets:
                asset_path = asset_data.asset_name
                if "/Game/UI" in asset_path:
                    self._validate_single_ui_widget(asset_path)

        except Exception as e:
            self.validation_errors.append(f"UI Widget validation failed: {e}")

    def _validate_niagara_systems(self):
        """Validate Niagara particle systems"""
        print("[ContentValidator] Validating Niagara Systems...")

        try:
            # Find all Niagara systems
            niagara_filter = unreal.ARFilter(
                class_names=["NiagaraSystem"],
                recursive_paths=True
            )

            systems = self.asset_registry.get_assets(niagara_filter)

            for asset_data in systems:
                asset_path = asset_data.asset_name
                if "/Game/Particles" in asset_path:
                    self._validate_single_niagara_system(asset_path)

        except Exception as e:
            self.validation_errors.append(f"Niagara validation failed: {e}")

    def _validate_single_blueprint(self, asset_path: str):
        """Validate a single Blueprint"""
        try:
            # Load the blueprint
            blueprint = self.editor_asset_lib.load_asset(asset_path)
            if not blueprint:
                self.validation_errors.append(f"Cannot load Blueprint: {asset_path}")
                return

            # Check if it has compilation errors
            # Note: This is a simplified check - full validation would require more complex logic

            # Check for basic properties
            if hasattr(blueprint, 'get_name'):
                name = blueprint.get_name()
                if not name or len(name) < 3:
                    self.validation_warnings.append(f"Blueprint has short name: {asset_path}")

        except Exception as e:
            self.validation_errors.append(f"Error validating Blueprint {asset_path}: {e}")

    def _validate_single_data_asset(self, asset_path: str):
        """Validate a single Data Asset"""
        try:
            # Load the data asset
            data_asset = self.editor_asset_lib.load_asset(asset_path)
            if not data_asset:
                self.validation_errors.append(f"Cannot load Data Asset: {asset_path}")
                return

            # Check if it has required properties
            # This would be customized based on the specific Data Asset types

        except Exception as e:
            self.validation_errors.append(f"Error validating Data Asset {asset_path}: {e}")

    def _validate_single_ui_widget(self, asset_path: str):
        """Validate a single UI widget"""
        try:
            # Load the widget
            widget = self.editor_asset_lib.load_asset(asset_path)
            if not widget:
                self.validation_errors.append(f"Cannot load UI Widget: {asset_path}")
                return

            # Basic validation - check if it's a valid widget class

        except Exception as e:
            self.validation_errors.append(f"Error validating UI Widget {asset_path}: {e}")

    def _validate_single_niagara_system(self, asset_path: str):
        """Validate a single Niagara system"""
        try:
            # Load the system
            system = self.editor_asset_lib.load_asset(asset_path)
            if not system:
                self.validation_errors.append(f"Cannot load Niagara System: {asset_path}")
                return

            # Basic validation - check if it's a valid Niagara system

        except Exception as e:
            self.validation_errors.append(f"Error validating Niagara System {asset_path}: {e}")

    def _report_validation_results(self):
        """Report validation results"""
        print("\n" + "="*60)
        print("CONTENT VALIDATION RESULTS")
        print("="*60)

        if not self.validation_errors and not self.validation_warnings:
            print("✅ All content validation passed!")
            print("No errors or warnings found.")
        else:
            if self.validation_errors:
                print(f"❌ {len(self.validation_errors)} ERRORS found:")
                for error in self.validation_errors:
                    print(f"  • {error}")

            if self.validation_warnings:
                print(f"⚠️  {len(self.validation_warnings)} WARNINGS found:")
                for warning in self.validation_warnings:
                    print(f"  • {warning}")

        print("="*60)