#!/usr/bin/env python3
"""
Adastrea - Unreal Editor Asset Validation Script (Stub)

This is a stub/template for Unreal Editor Python-based asset and blueprint validation.
This script is designed to be run INSIDE the Unreal Editor's Python environment,
not from the command line.

To use this script:
1. Enable the "Python Editor Script Plugin" in Unreal Engine
2. Go to Edit -> Project Settings -> Python
3. Add this script to the startup scripts or run it manually from the Python console
4. Extend the validation functions below to check your specific Data Assets and Blueprints

Future enhancements could include:
- Validate all FactionDataAssets have valid properties
- Check SpaceshipDataAssets for completeness
- Verify PersonnelDataAssets have required fields
- Check Blueprint graphs for common errors
- Validate asset naming conventions
- Check for orphaned or unused assets
- Verify material instances are properly configured
"""

import unreal

class AdastreaAssetValidator:
    """
    Asset validation for Adastrea project.
    
    This class provides validation methods for various Data Assets and Blueprints
    used in the Adastrea project. Extend these methods to add custom validation logic.
    """
    
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info_messages = []
    
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
    
    def validate_faction_data_assets(self):
        """
        Validate all FactionDataAsset instances.
        
        Example checks:
        - Faction ID is not empty
        - Tech level is between 1-10
        - Colors are properly set
        - Relationships reference valid faction IDs
        """
        self.log_info("Starting Faction Data Asset validation...")
        
        # Get asset registry
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Find all FactionDataAsset instances
        # Note: Replace 'FactionDataAsset' with the actual class name if different
        faction_assets = asset_registry.get_assets_by_class('FactionDataAsset', True)
        
        self.log_info(f"Found {len(faction_assets)} Faction Data Assets")
        
        # TODO: Implement specific validation logic
        # Example:
        # for asset_data in faction_assets:
        #     asset = unreal.load_asset(asset_data.package_name)
        #     if asset:
        #         # Validate faction properties
        #         if not asset.faction_id:
        #             self.log_error(f"{asset.get_name()}: Faction ID is empty")
        #         if asset.tech_level < 1 or asset.tech_level > 10:
        #             self.log_error(f"{asset.get_name()}: Tech level {asset.tech_level} is out of range (1-10)")
        
        self.log_info("Faction Data Asset validation complete")
    
    def validate_spaceship_data_assets(self):
        """
        Validate all SpaceshipDataAsset instances.
        
        Example checks:
        - Ship name is not empty
        - Stats are within valid ranges
        - Required fields are filled
        """
        self.log_info("Starting Spaceship Data Asset validation...")
        
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        spaceship_assets = asset_registry.get_assets_by_class('SpaceshipDataAsset', True)
        
        self.log_info(f"Found {len(spaceship_assets)} Spaceship Data Assets")
        
        # TODO: Implement specific validation logic
        
        self.log_info("Spaceship Data Asset validation complete")
    
    def validate_personnel_data_assets(self):
        """
        Validate all PersonnelDataAsset instances.
        
        Example checks:
        - Personnel name is not empty
        - Role is valid
        - Skills are properly configured
        """
        self.log_info("Starting Personnel Data Asset validation...")
        
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        personnel_assets = asset_registry.get_assets_by_class('PersonnelDataAsset', True)
        
        self.log_info(f"Found {len(personnel_assets)} Personnel Data Assets")
        
        # TODO: Implement specific validation logic
        
        self.log_info("Personnel Data Asset validation complete")
    
    def validate_blueprint_assets(self):
        """
        Validate Blueprint assets.
        
        Example checks:
        - Blueprints compile without errors
        - No missing dependencies
        - Proper naming conventions
        """
        self.log_info("Starting Blueprint validation...")
        
        # TODO: Implement Blueprint validation logic
        
        self.log_info("Blueprint validation complete")
    
    def validate_all(self):
        """Run all validation checks"""
        self.log_info("=" * 60)
        self.log_info("Adastrea Asset Validation Starting")
        self.log_info("=" * 60)
        
        # Run all validation methods
        self.validate_faction_data_assets()
        self.validate_spaceship_data_assets()
        self.validate_personnel_data_assets()
        self.validate_blueprint_assets()
        
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
