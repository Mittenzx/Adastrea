#!/usr/bin/env python3
"""
Adastrea - Blueprint Generator

This script automatically creates all required Blueprint assets for the Adastrea project
using Unreal Engine's Python API. Creates Blueprints for GameModes, PlayerControllers,
Ships, Stations, UI, and more.

Features:
- Generate essential blueprints (GameMode, PlayerController, PlayerShip, etc.)
- Generate ship variant blueprints
- Generate station module blueprints
- Generate NPC actor blueprints
- Generate component blueprints
- Automatic parent class assignment
- Proper folder structure creation

Usage:
    # In Unreal Editor Python Console
    import BlueprintGenerator
    BlueprintGenerator.generate_essential_blueprints()
    BlueprintGenerator.generate_all_blueprints()
    
    # Generate specific blueprint
    BlueprintGenerator.create_blueprint("BP_MyShip", "ASpaceship", "/Game/Blueprints/Ships")
"""

import sys
from typing import Dict, Any, Optional

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("ERROR: This script must be run inside Unreal Editor!")
    sys.exit(1)


class BlueprintGenerator:
    """Generator for creating Blueprint assets in Unreal Engine"""
    
    # Essential blueprints that MUST exist for game to run
    ESSENTIAL_BLUEPRINTS = [
        # Game Framework
        ("BP_SpaceGameMode", "AdastreaGameMode", "/Game/Blueprints/GameModes"),
        ("BP_GameInstance", "AdastreaGameInstance", "/Game/Blueprints/Core"),
        ("BP_SpaceshipController", "AdastreaPlayerController", "/Game/Blueprints/Controllers"),
        
        # Player Ship
        ("BP_PlayerShip", "ASpaceship", "/Game/Blueprints/Ships"),
    ]
    
    # Additional important blueprints
    SHIP_BLUEPRINTS = [
        ("BP_Ship_NPC_Fighter", "ASpaceship", "/Game/Blueprints/Ships/NPCs"),
        ("BP_Ship_NPC_Trader", "ASpaceship", "/Game/Blueprints/Ships/NPCs"),
        ("BP_Ship_NPC_Hauler", "ASpaceship", "/Game/Blueprints/Ships/NPCs"),
        ("BP_Ship_NPC_Scout", "ASpaceship", "/Game/Blueprints/Ships/NPCs"),
    ]
    
    STATION_BLUEPRINTS = [
        ("BP_SpaceStation_Main", "ASpaceStation", "/Game/Blueprints/Stations"),
    ]
    
    STATION_MODULE_BLUEPRINTS = [
        ("BP_Module_Docking", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Power", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Storage", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Defence", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Habitation", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Command", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Industrial", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Trade", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Research", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
        ("BP_Module_Entertainment", "ASpaceStationModule", "/Game/Blueprints/Stations/Modules"),
    ]
    
    ACTOR_BLUEPRINTS = [
        ("BP_Asteroid", "AActor", "/Game/Blueprints/Environment"),
        ("BP_Planet", "APlanet", "/Game/Blueprints/Environment"),
        ("BP_CargoContainer", "AActor", "/Game/Blueprints/Interactables"),
        ("BP_Waypoint", "AActor", "/Game/Blueprints/Navigation"),
    ]
    
    COMBAT_BLUEPRINTS = [
        ("BP_Projectile_Laser", "AActor", "/Game/Blueprints/Combat/Projectiles"),
        ("BP_Projectile_Cannon", "AActor", "/Game/Blueprints/Combat/Projectiles"),
        ("BP_Projectile_Missile", "AActor", "/Game/Blueprints/Combat/Projectiles"),
        ("BP_Explosion", "AActor", "/Game/Blueprints/Effects"),
    ]
    
    AI_BLUEPRINTS = [
        ("BP_NPCShipController", "AAIController", "/Game/Blueprints/AI"),
    ]
    
    def __init__(self):
        """Initialize the blueprint generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.created_count = 0
        self.skipped_count = 0
        self.errors = []
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[BlueprintGen] {message}")
        elif level == "warning":
            unreal.log_warning(f"[BlueprintGen] {message}")
        else:
            unreal.log(f"[BlueprintGen] {message}")
    
    def create_blueprint(self, blueprint_name: str, parent_class_name: str, 
                        folder_path: str, force: bool = False) -> Optional[str]:
        """
        Create a Blueprint asset
        
        Args:
            blueprint_name: Name of the blueprint (e.g., "BP_PlayerShip")
            parent_class_name: C++ parent class name (e.g., "ASpaceship")
            folder_path: Unreal content folder path (e.g., "/Game/Blueprints/Ships")
            force: If True, overwrite existing blueprint
            
        Returns:
            Path to created blueprint, or None if failed
        """
        asset_path = f"{folder_path}/{blueprint_name}"
        
        # Check if blueprint already exists
        if self.editor_asset_lib.does_asset_exist(asset_path) and not force:
            self.log(f"Blueprint already exists: {asset_path}", "info")
            self.skipped_count += 1
            return asset_path
        
        try:
            # Find the parent class
            parent_class = unreal.load_class(None, f"/Script/Adastrea.{parent_class_name}")
            
            if not parent_class:
                # Try without module prefix
                parent_class = unreal.load_class(None, parent_class_name)
            
            if not parent_class:
                self.log(f"Could not find parent class: {parent_class_name}", "error")
                self.errors.append(f"Missing class: {parent_class_name}")
                return None
            
            # Create blueprint factory
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("ParentClass", parent_class)
            
            # Create the blueprint
            blueprint = self.asset_tools.create_asset(
                asset_name=blueprint_name,
                package_path=folder_path,
                asset_class=unreal.Blueprint,
                factory=factory
            )
            
            if blueprint:
                self.log(f"✓ Created blueprint: {asset_path}")
                self.created_count += 1
                
                # Save the asset
                self.editor_asset_lib.save_loaded_asset(blueprint)
                
                return asset_path
            else:
                self.log(f"Failed to create blueprint: {asset_path}", "error")
                self.errors.append(f"Creation failed: {blueprint_name}")
                return None
                
        except Exception as e:
            self.log(f"Error creating blueprint {blueprint_name}: {str(e)}", "error")
            self.errors.append(f"{blueprint_name}: {str(e)}")
            return None
    
    def generate_essential_blueprints(self) -> int:
        """
        Generate only the essential blueprints required for game to run
        
        Returns:
            Number of blueprints created
        """
        self.log("=" * 80)
        self.log("Generating Essential Blueprints")
        self.log("=" * 80)
        
        for bp_name, parent_class, folder in self.ESSENTIAL_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        self.log("")
        self.log(f"Essential Blueprints: {self.created_count} created, {self.skipped_count} already existed")
        
        return self.created_count
    
    def generate_all_blueprints(self) -> int:
        """
        Generate all blueprints for complete game
        
        Returns:
            Number of blueprints created
        """
        self.log("=" * 80)
        self.log("Generating All Blueprints")
        self.log("=" * 80)
        
        # Essential blueprints
        self.log("\n--- Essential Blueprints ---")
        for bp_name, parent_class, folder in self.ESSENTIAL_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # Ship blueprints
        self.log("\n--- Ship Blueprints ---")
        for bp_name, parent_class, folder in self.SHIP_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # Station blueprints
        self.log("\n--- Station Blueprints ---")
        for bp_name, parent_class, folder in self.STATION_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # Station module blueprints
        self.log("\n--- Station Module Blueprints ---")
        for bp_name, parent_class, folder in self.STATION_MODULE_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # Actor blueprints
        self.log("\n--- Environmental Actor Blueprints ---")
        for bp_name, parent_class, folder in self.ACTOR_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # Combat blueprints
        self.log("\n--- Combat Blueprints ---")
        for bp_name, parent_class, folder in self.COMBAT_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        # AI blueprints
        self.log("\n--- AI Blueprints ---")
        for bp_name, parent_class, folder in self.AI_BLUEPRINTS:
            self.create_blueprint(bp_name, parent_class, folder)
        
        self.log("")
        self.log("=" * 80)
        self.log(f"Blueprint Generation Complete!")
        self.log(f"Created: {self.created_count}, Skipped: {self.skipped_count}")
        
        if self.errors:
            self.log(f"\nErrors encountered: {len(self.errors)}", "warning")
            for error in self.errors:
                self.log(f"  - {error}", "warning")
        
        self.log("=" * 80)
        
        return self.created_count
    
    def create_custom_blueprint(self, blueprint_name: str, parent_class_name: str,
                               folder_path: str = "/Game/Blueprints/Custom") -> Optional[str]:
        """
        Create a custom blueprint with specified parameters
        
        Args:
            blueprint_name: Name for the blueprint
            parent_class_name: Parent C++ class
            folder_path: Where to save the blueprint
            
        Returns:
            Path to created blueprint
        """
        return self.create_blueprint(blueprint_name, parent_class_name, folder_path, force=False)
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get generation statistics"""
        return {
            "created": self.created_count,
            "skipped": self.skipped_count,
            "errors": len(self.errors),
            "error_details": self.errors
        }


# Convenience functions for direct import
def generate_essential_blueprints() -> int:
    """Generate essential blueprints - convenience function"""
    generator = BlueprintGenerator()
    return generator.generate_essential_blueprints()


def generate_all_blueprints() -> int:
    """Generate all blueprints - convenience function"""
    generator = BlueprintGenerator()
    return generator.generate_all_blueprints()


def create_blueprint(blueprint_name: str, parent_class_name: str, folder_path: str) -> Optional[str]:
    """Create a single blueprint - convenience function"""
    generator = BlueprintGenerator()
    return generator.create_blueprint(blueprint_name, parent_class_name, folder_path)


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Blueprint Generator - Create Blueprint assets"
    )
    
    parser.add_argument("--essential", action="store_true",
                       help="Generate essential blueprints only")
    parser.add_argument("--all", action="store_true",
                       help="Generate all blueprints")
    parser.add_argument("--custom", nargs=3, metavar=("NAME", "PARENT", "FOLDER"),
                       help="Create custom blueprint: NAME PARENT_CLASS FOLDER_PATH")
    
    args = parser.parse_args()
    
    generator = BlueprintGenerator()
    
    if args.essential:
        generator.generate_essential_blueprints()
    elif args.all:
        generator.generate_all_blueprints()
    elif args.custom:
        name, parent, folder = args.custom
        generator.create_custom_blueprint(name, parent, folder)
    else:
        parser.print_help()
        print("\nTIP: Use --essential for minimal setup, --all for complete generation")


if __name__ == "__main__":
    main()
