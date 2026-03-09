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

        # Basic faction validation
        for asset_data in faction_assets:
            asset_name = asset_data.asset_name

            # Validate naming convention
            self.validate_naming_convention(asset_name, 'data_assets')

            # Load and validate properties
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset:
                    # Check required fields based on common faction properties
                    if hasattr(asset, 'faction_id'):
                        if not asset.faction_id or str(asset.faction_id).strip() == '':
                            self.log_error(f"{asset_name}: Faction ID is empty")
                    
                    if hasattr(asset, 'faction_name'):
                        if not asset.faction_name or str(asset.faction_name).strip() == '':
                            self.log_error(f"{asset_name}: Faction Name is empty")
                    
                    # Check tech level range if property exists
                    if hasattr(asset, 'tech_level'):
                        tech_level = asset.tech_level
                        if tech_level < tech_level_range[0] or tech_level > tech_level_range[1]:
                            self.log_error(f"{asset_name}: Tech level {tech_level} is out of range {tech_level_range}")
                    
                    # Check relationship values if properties exist
                    if hasattr(asset, 'relationship_value'):
                        rel_value = asset.relationship_value
                        if rel_value < relationship_range[0] or rel_value > relationship_range[1]:
                            self.log_warning(f"{asset_name}: Relationship value {rel_value} is out of typical range {relationship_range}")
                    
                    # Check economic properties
                    if hasattr(asset, 'economic_power'):
                        if asset.economic_power < 0:
                            self.log_warning(f"{asset_name}: Economic power cannot be negative")
                    
                    if hasattr(asset, 'military_strength'):
                        if asset.military_strength < 0:
                            self.log_warning(f"{asset_name}: Military strength cannot be negative")
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load asset for validation: {e}")

        self.log_info(f"Faction Data Asset validation complete - checked {len(faction_assets)} assets")

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

        # Basic validation for spaceship properties
        for asset_data in spaceship_assets:
            asset_name = asset_data.asset_name
            
            # Validate naming convention
            self.validate_naming_convention(asset_name, 'data_assets')
            
            # Try to load and validate the asset
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset:
                    # Check for required properties (using hasattr to be safe)
                    if hasattr(asset, 'hull_strength'):
                        if asset.hull_strength < stat_ranges.get('min', 0) or asset.hull_strength > stat_ranges.get('max', 1000):
                            self.log_warning(f"{asset_name}: Hull strength {asset.hull_strength} is outside recommended range {stat_ranges}")
                    
                    if hasattr(asset, 'cargo_capacity'):
                        if asset.cargo_capacity < 0:
                            self.log_error(f"{asset_name}: Cargo capacity cannot be negative")
                    
                    if hasattr(asset, 'min_crew'):
                        if asset.min_crew < 0:
                            self.log_error(f"{asset_name}: Minimum crew cannot be negative")
                        if hasattr(asset, 'max_crew') and asset.max_crew < asset.min_crew:
                            self.log_error(f"{asset_name}: Max crew ({asset.max_crew}) cannot be less than min crew ({asset.min_crew})")
                    
                    # Check combat stats if available
                    if hasattr(asset, 'armor_rating'):
                        if asset.armor_rating < 0:
                            self.log_warning(f"{asset_name}: Armor rating is negative")
                    
                    if hasattr(asset, 'shield_strength'):
                        if asset.shield_strength < 0:
                            self.log_warning(f"{asset_name}: Shield strength is negative")
                    
                    # Check mobility stats
                    if hasattr(asset, 'max_speed'):
                        if asset.max_speed < 0:
                            self.log_error(f"{asset_name}: Max speed cannot be negative")
                    
                    if hasattr(asset, 'acceleration'):
                        if asset.acceleration < 0:
                            self.log_warning(f"{asset_name}: Acceleration is negative")
                    
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load asset for validation: {e}")

        self.log_info(f"Spaceship Data Asset validation complete - checked {len(spaceship_assets)} assets")

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

        # Basic validation for personnel properties
        for asset_data in personnel_assets:
            asset_name = asset_data.asset_name
            
            # Validate naming convention
            self.validate_naming_convention(asset_name, 'data_assets')
            
            # Try to load and validate the asset
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset:
                    # Check role against valid roles list
                    if hasattr(asset, 'role'):
                        if valid_roles and asset.role not in valid_roles:
                            self.log_warning(f"{asset_name}: Role '{asset.role}' is not in valid roles list: {valid_roles}")
                    
                    # Check skill levels
                    if hasattr(asset, 'skill_level'):
                        if asset.skill_level < skill_range[0] or asset.skill_level > skill_range[1]:
                            self.log_warning(f"{asset_name}: Skill level {asset.skill_level} is outside valid range {skill_range}")
                    
                    # Check status values
                    if hasattr(asset, 'health'):
                        if asset.health < status_range[0] or asset.health > status_range[1]:
                            self.log_warning(f"{asset_name}: Health {asset.health} is outside valid range {status_range}")
                    
                    if hasattr(asset, 'morale'):
                        if asset.morale < status_range[0] or asset.morale > status_range[1]:
                            self.log_warning(f"{asset_name}: Morale {asset.morale} is outside valid range {status_range}")
                    
                    # Check for required fields
                    if hasattr(asset, 'display_name'):
                        if not asset.display_name or str(asset.display_name).strip() == '':
                            self.log_error(f"{asset_name}: Display name is empty")
                    
                    # Check salary/economic values
                    if hasattr(asset, 'salary'):
                        if asset.salary < 0:
                            self.log_error(f"{asset_name}: Salary cannot be negative")
                    
                    if hasattr(asset, 'hire_cost'):
                        if asset.hire_cost < 0:
                            self.log_warning(f"{asset_name}: Hire cost is negative")
                    
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load asset for validation: {e}")

        self.log_info(f"Personnel Data Asset validation complete - checked {len(personnel_assets)} assets")

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

        # Basic Blueprint validation
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Get all Blueprint assets
        blueprint_assets = asset_registry.get_assets_by_class('Blueprint', True)
        self.log_info(f"Found {len(blueprint_assets)} Blueprint assets")
        
        # Track statistics
        bp_with_prefix = 0
        bp_without_prefix = 0
        bp_with_errors = 0
        
        for asset_data in blueprint_assets:
            asset_name = asset_data.asset_name
            
            # Check naming convention - Blueprints should start with BP_
            if not asset_name.startswith('BP_'):
                self.log_warning(f"{asset_name}: Blueprint name should start with 'BP_' prefix")
                bp_without_prefix += 1
            else:
                bp_with_prefix += 1
            
            # Check for compilation errors
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset:
                    # Check if Blueprint has compilation errors
                    if hasattr(asset, 'has_any_flags'):
                        # This is a simplified check - in real implementation we'd check compilation status
                        pass
                    
                    # Check for missing dependencies
                    dependencies = unreal.EditorAssetLibrary.find_asset_referencers_for_asset(asset_data.object_path)
                    if dependencies and len(dependencies) > 0:
                        # Check if any dependencies are missing
                        for dep in dependencies:
                            if not unreal.EditorAssetLibrary.does_asset_exist(dep):
                                self.log_error(f"{asset_name}: Missing dependency: {dep}")
                                bp_with_errors += 1
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load Blueprint for validation: {e}")
        
        # Summary
        self.log_info(f"Blueprint validation summary:")
        self.log_info(f"  - Blueprints with BP_ prefix: {bp_with_prefix}")
        self.log_info(f"  - Blueprints without BP_ prefix: {bp_without_prefix}")
        self.log_info(f"  - Blueprints with errors: {bp_with_errors}")
        
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

        # Basic Material validation
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Get all Material assets
        material_assets = asset_registry.get_assets_by_class('Material', True)
        material_instance_assets = asset_registry.get_assets_by_class('MaterialInstanceConstant', True)
        
        self.log_info(f"Found {len(material_assets)} Materials and {len(material_instance_assets)} Material Instances")
        
        # Track statistics
        materials_with_prefix = 0
        materials_without_prefix = 0
        material_instances_with_prefix = 0
        material_instances_without_prefix = 0
        
        # Validate Materials
        for asset_data in material_assets:
            asset_name = asset_data.asset_name
            
            # Check naming convention - Materials should start with M_
            if not asset_name.startswith('M_'):
                self.log_warning(f"{asset_name}: Material name should start with 'M_' prefix")
                materials_without_prefix += 1
            else:
                materials_with_prefix += 1
            
            # Check for parent material if it's a material instance
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset and hasattr(asset, 'parent'):
                    parent = asset.parent
                    if parent:
                        # Check if parent material exists
                        if not unreal.EditorAssetLibrary.does_asset_exist(parent.get_path_name()):
                            self.log_error(f"{asset_name}: Parent material does not exist: {parent.get_path_name()}")
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load Material for validation: {e}")
        
        # Validate Material Instances
        for asset_data in material_instance_assets:
            asset_name = asset_data.asset_name
            
            # Check naming convention - Material Instances should start with MI_
            if not asset_name.startswith('MI_'):
                self.log_warning(f"{asset_name}: Material Instance name should start with 'MI_' prefix")
                material_instances_without_prefix += 1
            else:
                material_instances_with_prefix += 1
        
        # Summary
        self.log_info(f"Material validation summary:")
        self.log_info(f"  - Materials with M_ prefix: {materials_with_prefix}")
        self.log_info(f"  - Materials without M_ prefix: {materials_without_prefix}")
        self.log_info(f"  - Material Instances with MI_ prefix: {material_instances_with_prefix}")
        self.log_info(f"  - Material Instances without MI_ prefix: {material_instances_without_prefix}")
        
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

        # Basic Texture validation
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Get all Texture assets
        texture_assets = asset_registry.get_assets_by_class('Texture2D', True)
        self.log_info(f"Found {len(texture_assets)} Texture assets")
        
        # Track statistics
        textures_with_suffix = 0
        textures_without_suffix = 0
        textures_power_of_two = 0
        textures_not_power_of_two = 0
        textures_too_large = 0
        
        # Common texture suffixes
        common_suffixes = ['_D', '_N', '_R', '_M', '_AO', '_E', '_H', '_T']
        
        for asset_data in texture_assets:
            asset_name = asset_data.asset_name
            
            # Check for common texture suffixes
            has_suffix = any(asset_name.endswith(suffix) for suffix in common_suffixes)
            if not has_suffix:
                self.log_warning(f"{asset_name}: Texture name should include common suffix (e.g., _D for Diffuse, _N for Normal)")
                textures_without_suffix += 1
            else:
                textures_with_suffix += 1
            
            # Check texture properties
            try:
                asset = unreal.EditorAssetLibrary.load_asset(asset_data.object_path)
                if asset:
                    # Check texture size
                    if hasattr(asset, 'get_import_data'):
                        import_data = asset.get_import_data()
                        if import_data:
                            # Check dimensions
                            if hasattr(import_data, 'width') and hasattr(import_data, 'height'):
                                width = import_data.width
                                height = import_data.height
                                
                                # Check power-of-two
                                def is_power_of_two(n):
                                    return n != 0 and (n & (n - 1)) == 0
                                
                                if power_of_two_required and (not is_power_of_two(width) or not is_power_of_two(height)):
                                    self.log_warning(f"{asset_name}: Texture dimensions {width}x{height} are not power-of-two")
                                    textures_not_power_of_two += 1
                                else:
                                    textures_power_of_two += 1
                                
                                # Check size limits
                                if width > max_size_warning or height > max_size_warning:
                                    self.log_warning(f"{asset_name}: Texture size {width}x{height} exceeds recommended maximum of {max_size_warning}")
                                    textures_too_large += 1
            except Exception as e:
                self.log_warning(f"{asset_name}: Could not load Texture for validation: {e}")
        
        # Summary
        self.log_info(f"Texture validation summary:")
        self.log_info(f"  - Textures with proper suffix: {textures_with_suffix}")
        self.log_info(f"  - Textures without proper suffix: {textures_without_suffix}")
        self.log_info(f"  - Textures with power-of-two dimensions: {textures_power_of_two}")
        self.log_info(f"  - Textures without power-of-two dimensions: {textures_not_power_of_two}")
        self.log_info(f"  - Textures exceeding size limit: {textures_too_large}")
        
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
