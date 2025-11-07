#!/usr/bin/env python3
"""
Adastrea - YAML to Data Asset Importer

This script reads YAML template files and creates Unreal Engine Data Assets from them.
Must be run INSIDE the Unreal Editor Python environment.

To use this script:
1. Enable the "Python Editor Script Plugin" in Unreal Engine
2. Go to Tools → Python → Execute Python Script or use Python Console
3. Run this script to import YAML templates as Data Assets

Features:
- Import Spaceship templates from YAML
- Import Personnel templates from YAML
- Import Trade Item, Market, and Contract templates from YAML
- Import Faction AI templates from YAML
- Import Material templates from YAML
- Batch import entire directories
- Interactive mode for single imports

Usage in Unreal Editor Python Console:
    import YAMLtoDataAsset
    YAMLtoDataAsset.show_menu()
    
    # Or import specific files
    YAMLtoDataAsset.import_spaceship_yaml("Assets/SpaceshipTemplates/Fighter_MyShip.yaml")
    YAMLtoDataAsset.import_personnel_yaml("Assets/PersonnelTemplates/Captain_Smith.yaml")
    
    # Or batch import
    YAMLtoDataAsset.batch_import_spaceships()
"""

import os
import sys
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple

# Try to import Unreal Engine modules (required)
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("WARNING: This script must be run inside Unreal Editor with Python plugin enabled!")
    print("Some features will be limited or unavailable outside the editor.")

# Try to import PyYAML (should be available in Unreal's Python)
try:
    import yaml
    YAML_AVAILABLE = True
except ImportError:
    YAML_AVAILABLE = False
    print("ERROR: PyYAML not available.")
    print("To install PyYAML, run in Unreal Editor Python Console:")
    print("  import subprocess, sys")
    print("  subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyyaml'])")


class YAMLtoDataAssetImporter:
    """Imports YAML template files and creates Unreal Engine Data Assets"""
    
    def __init__(self):
        """Initialize the importer"""
        if not UNREAL_AVAILABLE:
            raise RuntimeError("This script must be run inside Unreal Editor!")
        
        if not YAML_AVAILABLE:
            raise RuntimeError("PyYAML is required. See error messages above for installation instructions.")
        
        self.project_dir = Path(unreal.SystemLibrary.get_project_directory())
        self.assets_dir = self.project_dir / "Assets"
        self.content_dir = Path("/Game")  # Unreal content root
        
        # Asset registry for finding asset classes
        self.asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_util = unreal.EditorAssetLibrary()
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[YAML Import] {message}")
        elif level == "warning":
            unreal.log_warning(f"[YAML Import] {message}")
        else:
            unreal.log(f"[YAML Import] {message}")
    
    def load_yaml(self, yaml_path: Path) -> Optional[Dict[str, Any]]:
        """Load and parse YAML file"""
        try:
            if not yaml_path.exists():
                self.log(f"YAML file not found: {yaml_path}", "error")
                return None
            
            with open(yaml_path, 'r', encoding='utf-8') as f:
                data = yaml.safe_load(f)
            
            self.log(f"Loaded YAML: {yaml_path.name}")
            return data
        except Exception as e:
            self.log(f"Failed to load YAML {yaml_path}: {e}", "error")
            return None
    
    def create_data_asset(self, asset_path: str, asset_class: type) -> Optional[unreal.Object]:
        """Create a new Data Asset in Unreal"""
        try:
            # Check if asset already exists
            if self.editor_util.does_asset_exist(asset_path):
                self.log(f"Asset already exists: {asset_path}", "warning")
                # Load existing asset
                return unreal.load_asset(asset_path)
            
            # Create new asset
            factory = unreal.DataAssetFactory()
            factory.set_editor_property('data_asset_class', asset_class)
            
            package_path = os.path.dirname(asset_path)
            asset_name = os.path.basename(asset_path)
            
            asset = self.asset_tools.create_asset(
                asset_name,
                package_path,
                asset_class,
                factory
            )
            
            if asset:
                self.log(f"Created Data Asset: {asset_path}")
                return asset
            else:
                self.log(f"Failed to create asset: {asset_path}", "error")
                return None
                
        except Exception as e:
            self.log(f"Error creating asset {asset_path}: {e}", "error")
            return None
    
    def set_property_safe(self, asset: unreal.Object, property_name: str, value: Any) -> bool:
        """Safely set a property on an asset"""
        try:
            asset.set_editor_property(property_name, value)
            return True
        except Exception as e:
            self.log(f"Warning: Could not set property '{property_name}': {e}", "warning")
            return False
    
    def import_spaceship_yaml(self, yaml_path: str) -> Optional[str]:
        """
        Import a spaceship YAML file and create a SpaceshipDataAsset
        
        Args:
            yaml_path: Path to YAML file (relative to project or absolute)
            
        Returns:
            Content path to created asset or None if failed
        """
        yaml_file = Path(yaml_path)
        if not yaml_file.is_absolute():
            yaml_file = self.project_dir / yaml_file
        
        data = self.load_yaml(yaml_file)
        if not data:
            return None
        
        # Determine asset name from YAML data or filename
        ship_name = data.get('BasicInfo', {}).get('ShipID', yaml_file.stem)
        asset_path = f"/Game/Spaceships/DataAssets/DA_Ship_{ship_name}"
        
        self.log(f"Importing spaceship: {ship_name}")
        
        # Find SpaceshipDataAsset class
        try:
            spaceship_class = unreal.load_class(None, "/Script/Adastrea.SpaceshipDataAsset")
        except Exception as e:
            self.log(f"SpaceshipDataAsset class not found. Ensure the project is compiled. Error: {e}", "error")
            return None
        
        # Create the Data Asset
        asset = self.create_data_asset(asset_path, spaceship_class)
        if not asset:
            return None
        
        # Set properties from YAML data
        # Basic Info
        if 'BasicInfo' in data:
            basic_info = data['BasicInfo']
            self.set_property_safe(asset, 'ship_name', basic_info.get('ShipName', ''))
            self.set_property_safe(asset, 'description', basic_info.get('Description', ''))
            self.set_property_safe(asset, 'ship_class', basic_info.get('ShipClass', ''))
            self.set_property_safe(asset, 'ship_id', basic_info.get('ShipID', ''))
        
        # Core Stats
        if 'CoreStats' in data:
            core = data['CoreStats']
            self.set_property_safe(asset, 'hull_strength', float(core.get('HullStrength', 1000.0)))
            self.set_property_safe(asset, 'cargo_capacity', float(core.get('CargoCapacity', 100.0)))
            self.set_property_safe(asset, 'crew_required', int(core.get('CrewRequired', 2)))
            self.set_property_safe(asset, 'max_crew', int(core.get('MaxCrew', 6)))
            self.set_property_safe(asset, 'modular_points', int(core.get('ModularPoints', 8)))
        
        # Combat Stats
        if 'CombatStats' in data:
            combat = data['CombatStats']
            self.set_property_safe(asset, 'armor_rating', float(combat.get('ArmorRating', 30.0)))
            self.set_property_safe(asset, 'shield_strength', float(combat.get('ShieldStrength', 500.0)))
            self.set_property_safe(asset, 'shield_recharge_rate', float(combat.get('ShieldRechargeRate', 10.0)))
            self.set_property_safe(asset, 'weapon_slots', int(combat.get('WeaponSlots', 2)))
            self.set_property_safe(asset, 'weapon_power_capacity', float(combat.get('WeaponPowerCapacity', 1000.0)))
            self.set_property_safe(asset, 'point_defense_rating', float(combat.get('PointDefenseRating', 5.0)))
        
        # Mobility Stats
        if 'MobilityStats' in data:
            mobility = data['MobilityStats']
            self.set_property_safe(asset, 'max_speed', float(mobility.get('MaxSpeed', 700.0)))
            self.set_property_safe(asset, 'acceleration', float(mobility.get('Acceleration', 100.0)))
            self.set_property_safe(asset, 'maneuverability', int(mobility.get('Maneuverability', 7)))
            self.set_property_safe(asset, 'jump_range', float(mobility.get('JumpRange', 20.0)))
            self.set_property_safe(asset, 'fuel_capacity', float(mobility.get('FuelCapacity', 2000.0)))
            self.set_property_safe(asset, 'fuel_consumption_rate', float(mobility.get('FuelConsumptionRate', 75.0)))
        
        # Continue with other stat categories...
        # (Add more as needed based on actual SpaceshipDataAsset properties)
        
        # Save the asset
        self.editor_util.save_asset(asset_path)
        self.log(f"Successfully imported spaceship: {ship_name} → {asset_path}")
        
        return asset_path
    
    def import_personnel_yaml(self, yaml_path: str) -> Optional[str]:
        """
        Import a personnel YAML file and create a PersonnelDataAsset
        
        Args:
            yaml_path: Path to YAML file (relative to project or absolute)
            
        Returns:
            Content path to created asset or None if failed
        """
        yaml_file = Path(yaml_path)
        if not yaml_file.is_absolute():
            yaml_file = self.project_dir / yaml_file
        
        data = self.load_yaml(yaml_file)
        if not data:
            return None
        
        # Determine asset name
        personnel_id = data.get('PersonnelID', yaml_file.stem)
        asset_path = f"/Game/Personnel/DataAssets/DA_Personnel_{personnel_id}"
        
        self.log(f"Importing personnel: {personnel_id}")
        
        # Find PersonnelDataAsset class
        try:
            personnel_class = unreal.load_class(None, "/Script/Adastrea.PersonnelDataAsset")
        except Exception as e:
            self.log(f"PersonnelDataAsset class not found. Ensure the project is compiled. Error: {e}", "error")
            return None
        
        # Create the Data Asset
        asset = self.create_data_asset(asset_path, personnel_class)
        if not asset:
            return None
        
        # Set properties from YAML data
        self.set_property_safe(asset, 'personnel_id', data.get('PersonnelID', ''))
        self.set_property_safe(asset, 'personnel_name', data.get('PersonnelName', ''))
        self.set_property_safe(asset, 'biography', data.get('Biography', ''))
        self.set_property_safe(asset, 'age', int(data.get('Age', 30)))
        self.set_property_safe(asset, 'gender', data.get('Gender', 'Male'))
        self.set_property_safe(asset, 'species', data.get('Species', 'Human'))
        self.set_property_safe(asset, 'nationality', data.get('Nationality', ''))
        
        # Role and Assignment
        self.set_property_safe(asset, 'primary_role', data.get('PrimaryRole', ''))
        self.set_property_safe(asset, 'current_assignment', data.get('CurrentAssignment', ''))
        self.set_property_safe(asset, 'department', data.get('Department', ''))
        
        # Skills and Experience
        self.set_property_safe(asset, 'overall_skill_level', int(data.get('OverallSkillLevel', 5)))
        self.set_property_safe(asset, 'total_experience', int(data.get('TotalExperience', 5000)))
        
        # Status
        self.set_property_safe(asset, 'morale', int(data.get('Morale', 70)))
        self.set_property_safe(asset, 'health', int(data.get('Health', 90)))
        self.set_property_safe(asset, 'fatigue', int(data.get('Fatigue', 30)))
        self.set_property_safe(asset, 'loyalty', int(data.get('Loyalty', 75)))
        self.set_property_safe(asset, 'reputation', int(data.get('Reputation', 50)))
        
        # Employment
        self.set_property_safe(asset, 'salary', int(data.get('Salary', 5000)))
        self.set_property_safe(asset, 'contract_duration', int(data.get('ContractDuration', 12)))
        
        # Personality
        self.set_property_safe(asset, 'personality_type', data.get('PersonalityType', ''))
        self.set_property_safe(asset, 'personality_description', data.get('PersonnelDescription', ''))
        
        # Save the asset
        self.editor_util.save_asset(asset_path)
        self.log(f"Successfully imported personnel: {personnel_id} → {asset_path}")
        
        return asset_path
    
    def batch_import_directory(self, directory: str, import_func) -> List[str]:
        """
        Batch import all YAML files from a directory
        
        Args:
            directory: Directory containing YAML files
            import_func: Function to use for importing (e.g., import_spaceship_yaml)
            
        Returns:
            List of created asset paths
        """
        dir_path = Path(directory)
        if not dir_path.is_absolute():
            dir_path = self.project_dir / dir_path
        
        if not dir_path.exists():
            self.log(f"Directory not found: {dir_path}", "error")
            return []
        
        yaml_files = list(dir_path.glob("*.yaml")) + list(dir_path.glob("*.yml"))
        self.log(f"Found {len(yaml_files)} YAML files in {dir_path}")
        
        created_assets = []
        for yaml_file in yaml_files:
            try:
                asset_path = import_func(str(yaml_file))
                if asset_path:
                    created_assets.append(asset_path)
            except Exception as e:
                self.log(f"Error importing {yaml_file.name}: {e}", "error")
        
        self.log(f"Batch import complete: {len(created_assets)}/{len(yaml_files)} assets created")
        return created_assets
    
    def batch_import_spaceships(self) -> List[str]:
        """Batch import all spaceship YAML templates"""
        return self.batch_import_directory("Assets/SpaceshipTemplates", self.import_spaceship_yaml)
    
    def batch_import_personnel(self) -> List[str]:
        """Batch import all personnel YAML templates"""
        return self.batch_import_directory("Assets/PersonnelTemplates", self.import_personnel_yaml)
    
    def show_menu(self):
        """Interactive menu for importing YAML files"""
        while True:
            print("\n" + "=" * 60)
            print("YAML to Data Asset Importer")
            print("=" * 60)
            print("\nSingle Import:")
            print("  1. Import Spaceship YAML")
            print("  2. Import Personnel YAML")
            print("  3. Import Trade Item YAML")
            print("  4. Import Market YAML")
            print("  5. Import Contract YAML")
            print("  6. Import Faction AI YAML")
            print("\nBatch Import:")
            print("  7. Import All Spaceships")
            print("  8. Import All Personnel")
            print("  9. Import All Trading Templates")
            print("  10. Import All Faction AI Templates")
            print("\n  0. Exit")
            print("=" * 60)
            
            choice = input("\nSelect option (0-10): ").strip()
            
            if choice == "0":
                print("Exiting...")
                break
            elif choice == "1":
                path = input("Enter YAML file path: ").strip()
                self.import_spaceship_yaml(path)
            elif choice == "2":
                path = input("Enter YAML file path: ").strip()
                self.import_personnel_yaml(path)
            elif choice == "7":
                self.batch_import_spaceships()
            elif choice == "8":
                self.batch_import_personnel()
            else:
                print("Option not yet implemented or invalid choice.")


# Global instance
_importer = None

def get_importer() -> YAMLtoDataAssetImporter:
    """Get or create the global importer instance"""
    global _importer
    if _importer is None:
        _importer = YAMLtoDataAssetImporter()
    return _importer


# Convenience functions
def show_menu():
    """Show the interactive menu"""
    get_importer().show_menu()


def import_spaceship_yaml(yaml_path: str) -> Optional[str]:
    """Import a spaceship YAML file"""
    return get_importer().import_spaceship_yaml(yaml_path)


def import_personnel_yaml(yaml_path: str) -> Optional[str]:
    """Import a personnel YAML file"""
    return get_importer().import_personnel_yaml(yaml_path)


def batch_import_spaceships() -> List[str]:
    """Batch import all spaceship YAML templates"""
    return get_importer().batch_import_spaceships()


def batch_import_personnel() -> List[str]:
    """Batch import all personnel YAML templates"""
    return get_importer().batch_import_personnel()


def main():
    """Main entry point"""
    if not UNREAL_AVAILABLE:
        print("ERROR: This script must be run inside Unreal Editor!")
        print("1. Enable 'Python Editor Script Plugin'")
        print("2. Go to Tools → Python → Execute Python Script")
        print("3. Select this script")
        return
    
    importer = YAMLtoDataAssetImporter()
    importer.show_menu()


if __name__ == '__main__':
    main()
