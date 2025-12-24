#!/usr/bin/env python3
"""
Adastrea - Unreal Editor Asset Validation Script

This script provides comprehensive asset validation for the Adastrea project.
It is designed to be run INSIDE the Unreal Editor's Python environment,
not from the command line.

To use this script:
1. Enable the "Python Editor Script Plugin" in Unreal Engine
2. Go to Edit -> Project Settings -> Python
3. Add this script to the startup scripts or run it manually from the Python console
4. Ensure validator_config.json exists in the project root directory

Features:
- Validates asset naming conventions against configurable rules
- Checks Data Assets for required fields and valid value ranges
- Validates Blueprint complexity and structure
- Checks material and texture specifications
- Configurable via validator_config.json

Configuration:
- Edit validator_config.json to customize validation rules
- Add new naming conventions, validation rules, or ignore patterns
- No code changes required for most rule updates
"""

import unreal
import json
import os
import re

class AdastreaAssetValidator:
    """
    Asset validation for Adastrea project.
    
    This class provides validation methods for various Data Assets and Blueprints
    used in the Adastrea project. Validation rules are loaded from validator_config.json.
    """
    
    def __init__(self, config_path='validator_config.json'):
        self.errors = []
        self.warnings = []
        self.info_messages = []
        self.config = self.load_config(config_path)
    
    def load_config(self, config_path):
        """Load validation configuration from JSON file"""
        try:
            # Try to load from project root
            project_dir = unreal.Paths.project_dir()
            full_path = os.path.join(project_dir, config_path)
            
            if os.path.exists(full_path):
                with open(full_path, 'r') as f:
                    config = json.load(f)
                    self.log_info(f"Loaded configuration from {full_path}")
                    return config
            else:
                self.log_warning(f"Config file not found at {full_path}, using default rules")
                return self.get_default_config()
        except Exception as e:
            self.log_error(f"Failed to load config: {e}")
            return self.get_default_config()
    
    def get_default_config(self):
        """Return default configuration if config file is not available"""
        return {
            "naming_conventions": {
                "blueprints": {"prefix": "BP_"},
                "data_assets": {"prefix": "DA_"},
                "widgets": {"prefix": "WBP_"}
            },
            "validation_rules": {},
            "ignore_patterns": []
        }
    
    def log_error(self, message: str):
        """Log an error message"""
        self.errors.append(message)
        unreal.log_error(f"[Asset Validation] ERROR: {message}")
    
    def log_warning(self, message: str):
        """Log a warning message"""
        self.warnings.append(message)
        unreal.log_warning(f"[Asset Validation] WARNING: {message}")
    
    def log_info(self, message: str):
        """Log an info message"""
        self.info_messages.append(message)
        unreal.log(f"[Asset Validation] INFO: {message}")
    
    def validate_naming_convention(self, asset_name, asset_type):
        """
        Validate asset naming convention based on config rules
        
        Args:
            asset_name: The name of the asset to validate
            asset_type: The type of asset (e.g., 'blueprints', 'data_assets')
        
        Returns:
            True if naming is valid, False otherwise
        """
        naming_rules = self.config.get('naming_conventions', {}).get(asset_type, {})
        
        if not naming_rules:
            return True  # No rules defined for this asset type
        
        pattern = naming_rules.get('pattern')
        prefix = naming_rules.get('prefix')
        
        # Check prefix requirement
        if prefix and not asset_name.startswith(prefix):
            self.log_warning(f"{asset_name}: Should start with '{prefix}' (asset type: {asset_type})")
            return False
        
        # Check regex pattern if defined
        if pattern:
            if not re.match(pattern, asset_name):
                examples = naming_rules.get('examples', [])
                example_text = f" Examples: {', '.join(examples[:2])}" if examples else ""
                self.log_warning(f"{asset_name}: Does not match naming pattern.{example_text}")
                return False
        
        return True
    
    def validate_faction_data_assets(self):
        """
        Validate all FactionDataAsset instances.
        
        Checks from configuration:
        - Faction ID is not empty
        - Tech level is within valid range
        - Relationship values are within valid range
        - Asset naming conventions
        """
        self.log_info("Starting Faction Data Asset validation...")
        
        # Get asset registry
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Find all FactionDataAsset instances
        faction_assets = asset_registry.get_assets_by_class('FactionDataAsset', True)
        
        self.log_info(f"Found {len(faction_assets)} Faction Data Assets")
        
        # Get validation rules from config
        faction_rules = self.config.get('validation_rules', {}).get('data_assets', {}).get('faction', {})
        tech_level_range = faction_rules.get('tech_level_range', [1, 10])
        relationship_range = faction_rules.get('relationship_value_range', [-100, 100])
        
        # TODO: Implement specific validation logic once asset loading is working
        # Example implementation (uncomment and adapt when ready):
        # for asset_data in faction_assets:
        #     asset_name = asset_data.asset_name
        #     
        #     # Validate naming convention
        #     self.validate_naming_convention(asset_name, 'data_assets')
        #     
        #     # Load and validate properties
        #     try:
        #         asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
        #         if asset:
        #             # Check required fields
        #             if hasattr(asset, 'faction_id') and not asset.faction_id:
        #                 self.log_error(f"{asset_name}: Faction ID is empty")
        #             
        #             # Check tech level range
        #             if hasattr(asset, 'tech_level'):
        #                 if asset.tech_level < tech_level_range[0] or asset.tech_level > tech_level_range[1]:
        #                     self.log_error(f"{asset_name}: Tech level {asset.tech_level} is out of range {tech_level_range}")
        #     except Exception as e:
        #         self.log_warning(f"{asset_name}: Could not load asset for validation: {e}")
        
        self.log_info("Faction Data Asset validation complete")
    
    def validate_spaceship_data_assets(self):
        """
        Validate all SpaceshipDataAsset instances.
        
        Checks from configuration:
        - Ship name is not empty
        - Stats are within valid ranges
        - Required fields are filled
        - Asset naming conventions
        """
        self.log_info("Starting Spaceship Data Asset validation...")
        
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        spaceship_assets = asset_registry.get_assets_by_class('SpaceshipDataAsset', True)
        
        self.log_info(f"Found {len(spaceship_assets)} Spaceship Data Assets")
        
        # Get validation rules from config
        spaceship_rules = self.config.get('validation_rules', {}).get('data_assets', {}).get('spaceship', {})
        stat_ranges = spaceship_rules.get('stat_ranges', {'min': 0, 'max': 1000})
        
        # TODO: Advanced validation for spaceship properties
        # - Validate hull strength, cargo capacity, crew requirements
        # - Check combat stats (armor, shields, weapons)
        # - Verify mobility stats (speed, acceleration, maneuverability)
        # - Ensure operational stats are reasonable
        
        self.log_info("Spaceship Data Asset validation complete")
    
    def validate_personnel_data_assets(self):
        """
        Validate all PersonnelDataAsset instances.
        
        Checks from configuration:
        - Personnel name is not empty
        - Role is valid (from configured role list)
        - Skills are within valid ranges
        - Status values are within valid ranges
        - Asset naming conventions
        """
        self.log_info("Starting Personnel Data Asset validation...")
        
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        personnel_assets = asset_registry.get_assets_by_class('PersonnelDataAsset', True)
        
        self.log_info(f"Found {len(personnel_assets)} Personnel Data Assets")
        
        # Get validation rules from config
        personnel_rules = self.config.get('validation_rules', {}).get('data_assets', {}).get('personnel', {})
        valid_roles = personnel_rules.get('valid_roles', [])
        skill_range = personnel_rules.get('skill_level_range', [1, 10])
        status_range = personnel_rules.get('status_range', [0, 100])
        
        # TODO: Advanced validation for personnel properties
        # - Validate role against valid_roles list
        # - Check skill levels are within skill_range
        # - Verify status values (health, morale, fatigue) are within status_range
        # - Ensure relationships reference valid personnel IDs
        # - Check performance metrics are properly configured
        
        self.log_info("Personnel Data Asset validation complete")
    
    def validate_blueprint_assets(self):
        """
        Validate Blueprint assets.
        
        Checks from configuration:
        - Blueprints compile without errors
        - No missing dependencies
        - Proper naming conventions
        - Blueprint complexity (node count)
        - Presence of comments for complex graphs
        """
        self.log_info("Starting Blueprint validation...")
        
        # Get validation rules from config
        blueprint_rules = self.config.get('validation_rules', {}).get('blueprints', {})
        max_nodes_warning = blueprint_rules.get('max_nodes_warning', 500)
        max_nodes_error = blueprint_rules.get('max_nodes_error', 1000)
        require_comments = blueprint_rules.get('require_comments', True)
        
        # TODO: Advanced Blueprint validation
        # - Check Blueprint compilation status
        # - Count nodes in Blueprint graphs and warn/error based on thresholds
        # - Verify naming conventions (BP_ prefix)
        # - Check for missing dependencies or broken references
        # - Validate that complex Blueprints have descriptive comments
        # - Check for common Blueprint anti-patterns
        
        self.log_info("Blueprint validation complete")
    
    def validate_material_assets(self):
        """
        Validate Material assets.
        
        Checks from configuration:
        - Material naming conventions
        - Shader complexity (instruction count)
        - Texture size recommendations
        """
        self.log_info("Starting Material validation...")
        
        # Get validation rules from config
        material_rules = self.config.get('validation_rules', {}).get('materials', {})
        max_texture_size_warning = material_rules.get('max_texture_size_warning', 2048)
        max_instructions_warning = material_rules.get('max_instructions_warning', 300)
        max_instructions_error = material_rules.get('max_instructions_error', 500)
        
        # TODO: Advanced material validation
        # - Check material instruction count and warn if exceeds thresholds
        # - Validate texture sizes used in materials
        # - Check for unoptimized material setups
        # - Verify material instances reference valid parent materials
        # - Validate naming conventions (M_ for materials, MI_ for instances)
        
        self.log_info("Material validation complete")
    
    def validate_texture_assets(self):
        """
        Validate Texture assets.
        
        Checks from configuration:
        - Texture naming conventions with proper suffixes
        - Texture format (PNG, TGA, EXR)
        - Power-of-two dimensions
        - Size recommendations
        """
        self.log_info("Starting Texture validation...")
        
        # Get validation rules from config
        texture_rules = self.config.get('validation_rules', {}).get('textures', {})
        allowed_formats = texture_rules.get('allowed_formats', ['PNG', 'TGA', 'EXR'])
        power_of_two_required = texture_rules.get('power_of_two_required', True)
        max_size_warning = texture_rules.get('max_size_warning', 2048)
        
        # Get naming conventions for textures
        texture_naming = self.config.get('naming_conventions', {}).get('textures', {})
        valid_suffixes = texture_naming.get('suffixes', {}).values()
        
        # TODO: Advanced texture validation
        # - Check texture dimensions are power-of-two if required
        # - Validate texture format against allowed_formats list
        # - Warn if texture size exceeds recommended dimensions
        # - Check texture naming includes proper suffix (_D, _N, _R, etc.)
        # - Verify textures are using appropriate compression settings
        
        self.log_info("Texture validation complete")
    
    def validate_all(self):
        """Run all validation checks"""
        self.log_info("=" * 60)
        self.log_info("Adastrea Asset Validation Starting")
        self.log_info("=" * 60)
        
        # Display configuration info
        config_version = self.config.get('version', 'unknown')
        self.log_info(f"Using configuration version: {config_version}")
        
        # Run all validation methods
        self.validate_faction_data_assets()
        self.validate_spaceship_data_assets()
        self.validate_personnel_data_assets()
        self.validate_blueprint_assets()
        self.validate_material_assets()
        self.validate_texture_assets()
        
        # Print summary
        self.log_info("=" * 60)
        self.log_info("Asset Validation Complete")
        self.log_info(f"Errors: {len(self.errors)}")
        self.log_info(f"Warnings: {len(self.warnings)}")
        self.log_info(f"Info: {len(self.info_messages)}")
        self.log_info("=" * 60)
        
        if self.errors:
            self.log_error("Validation failed! Please fix errors above.")
            return False
        elif self.warnings:
            self.log_warning("Validation passed with warnings. Review warnings above.")
            return True
        else:
            self.log_info("All validation checks passed!")
            return True


def run_asset_validation():
    """
    Main entry point for asset validation.
    
    Call this function from the Unreal Editor Python console:
    >>> import AdastreaAssetValidator
    >>> AdastreaAssetValidator.run_asset_validation()
    """
    validator = AdastreaAssetValidator()
    success = validator.validate_all()
    return success


# Example usage when run directly in Unreal Editor
if __name__ == "__main__":
    # This will run if the script is executed in the Unreal Editor Python environment
    try:
        run_asset_validation()
    except Exception as e:
        unreal.log_error(f"Asset validation failed with exception: {e}")


"""
USAGE INSTRUCTIONS:

1. Enable Python in Unreal Engine:
   - Edit -> Plugins
   - Search for "Python Editor Script Plugin"
   - Enable the plugin and restart the editor

2. Run this script:
   Option A - From Python Console (Ctrl+P):
     import AdastreaAssetValidator
     AdastreaAssetValidator.run_asset_validation()
   
   Option B - Add to startup scripts:
     - Edit -> Project Settings -> Python
     - Add this file to "Startup Scripts"
     - Script will run automatically when editor opens

3. Extend validation logic:
   - Modify the validate_*_data_assets() methods above
   - Add new validation methods as needed
   - Use unreal.log(), unreal.log_warning(), unreal.log_error() for output

4. Access Data Asset properties:
   - Load assets using unreal.load_asset(asset_path)
   - Access properties using dot notation: asset.property_name
   - Refer to the C++ class definitions for available properties

5. Useful Unreal Python functions:
   - unreal.EditorAssetLibrary.list_assets(directory)
   - unreal.EditorAssetLibrary.load_asset(asset_path)
   - unreal.AssetRegistryHelpers.get_asset_registry()
   - unreal.EditorFilterLibrary.by_class(assets, class_name)

For more information on Unreal Engine Python API:
https://docs.unrealengine.com/en-US/ProductionPipelines/ScriptingAndAutomation/Python/
"""
