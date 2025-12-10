#!/usr/bin/env python3
"""
Adastrea - UI Widget Blueprint Generator

This script creates UI Widget Blueprints for the Adastrea project.

Features:
- Generate HUD widgets
- Generate menu widgets
- Generate gameplay UI widgets
- Automatic parent class assignment
- Proper folder structure

Usage:
    # In Unreal Editor Python Console
    import UIWidgetGenerator
    UIWidgetGenerator.generate_all_widgets()
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


class UIWidgetGenerator:
    """Generator for UI Widget Blueprints"""
    
    # UI Widgets to create
    WIDGET_BLUEPRINTS = [
        # HUD
        ("WBP_SpaceshipHUD", "AdastreaHUDWidget", "/Game/UI/HUD"),
        
        # Menus
        ("WBP_MainMenu", "UserWidget", "/Game/UI/Menus"),
        ("WBP_PauseMenu", "UserWidget", "/Game/UI/Menus"),
        ("WBP_SettingsMenu", "UserWidget", "/Game/UI/Menus"),
        
        # Trading
        ("WBP_TradingInterface", "TradingInterfaceWidget", "/Game/UI/Trading"),
        
        # Station
        ("WBP_StationManagement", "StationManagementWidget", "/Game/UI/Stations"),
        
        # Inventory
        ("WBP_InventoryWidget", "InventoryWidget", "/Game/UI/Inventory"),
        
        # Ship
        ("WBP_ShipCustomization", "ShipCustomizationWidget", "/Game/UI/Ships"),
        
        # Quest
        ("WBP_QuestLog", "UserWidget", "/Game/UI/Quests"),
        
        # Map
        ("WBP_SectorMap", "SectorMapWidget", "/Game/UI/Map"),
        
        # Dialogue
        ("WBP_DialogueBox", "UserWidget", "/Game/UI/Dialogue"),
        
        # Notifications
        ("WBP_Notification", "UserWidget", "/Game/UI/Notifications"),
    ]
    
    def __init__(self):
        """Initialize the UI widget generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.created_count = 0
        self.skipped_count = 0
        self.errors = []
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[UIWidgetGen] {message}")
        elif level == "warning":
            unreal.log_warning(f"[UIWidgetGen] {message}")
        else:
            unreal.log(f"[UIWidgetGen] {message}")
    
    def create_widget_blueprint(self, widget_name: str, parent_class_name: str,
                               folder_path: str) -> Optional[str]:
        """
        Create a Widget Blueprint
        
        Args:
            widget_name: Name of the widget
            parent_class_name: Parent class name
            folder_path: Unreal content folder path
            
        Returns:
            Path to created widget, or None if failed
        """
        asset_path = f"{folder_path}/{widget_name}"
        
        # Check if already exists
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.log(f"Widget already exists: {widget_name}", "info")
            self.skipped_count += 1
            return asset_path
        
        try:
            # Try to find the parent class
            parent_class = None
            
            # First try Adastrea module
            try:
                parent_class = unreal.load_class(None, f"/Script/Adastrea.{parent_class_name}")
            except:
                pass
            
            # Then try UMG module
            if not parent_class:
                try:
                    parent_class = unreal.load_class(None, f"/Script/UMG.{parent_class_name}")
                except:
                    pass
            
            # Finally try without module
            if not parent_class:
                try:
                    parent_class = unreal.load_class(None, parent_class_name)
                except:
                    pass
            
            if not parent_class:
                self.log(f"Could not find parent class: {parent_class_name}", "error")
                self.errors.append(f"Missing class: {parent_class_name}")
                return None
            
            # Create widget blueprint factory
            factory = unreal.WidgetBlueprintFactory()
            factory.set_editor_property("ParentClass", parent_class)
            
            # Create the widget blueprint
            widget = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path=folder_path,
                asset_class=unreal.WidgetBlueprint,
                factory=factory
            )
            
            if widget:
                self.log(f"✓ Created widget: {widget_name}")
                self.created_count += 1
                
                # Save the asset
                self.editor_asset_lib.save_loaded_asset(widget)
                
                return asset_path
            else:
                self.log(f"Failed to create widget: {widget_name}", "error")
                return None
                
        except Exception as e:
            self.log(f"Error creating widget {widget_name}: {str(e)}", "error")
            self.errors.append(f"{widget_name}: {str(e)}")
            return None
    
    def generate_all_widgets(self) -> int:
        """
        Generate all UI widget blueprints
        
        Returns:
            Number of widgets created
        """
        self.log("=" * 80)
        self.log("GENERATING UI WIDGET BLUEPRINTS")
        self.log("=" * 80)
        
        for widget_name, parent_class, folder in self.WIDGET_BLUEPRINTS:
            self.create_widget_blueprint(widget_name, parent_class, folder)
        
        self.log("")
        self.log("=" * 80)
        self.log("UI WIDGET GENERATION COMPLETE!")
        self.log(f"Created: {self.created_count}, Skipped: {self.skipped_count}")
        
        if self.errors:
            self.log(f"\nErrors encountered: {len(self.errors)}", "warning")
            for error in self.errors:
                self.log(f"  - {error}", "warning")
        
        self.log("")
        self.log("NEXT STEPS:")
        self.log("1. Open each widget in the UMG Designer")
        self.log("2. Add UI elements (buttons, text, images, etc.)")
        self.log("3. Bind events in the Graph")
        self.log("4. Test widgets in-game")
        self.log("=" * 80)
        
        return self.created_count


# Convenience functions
def generate_all_widgets() -> int:
    """Generate all widgets - convenience function"""
    generator = UIWidgetGenerator()
    return generator.generate_all_widgets()


def create_widget_blueprint(widget_name: str, parent_class: str, folder: str) -> Optional[str]:
    """Create single widget - convenience function"""
    generator = UIWidgetGenerator()
    return generator.create_widget_blueprint(widget_name, parent_class, folder)


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea UI Widget Generator - Create UI Widget Blueprints"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Generate all widgets")
    parser.add_argument("--custom", nargs=3, metavar=("NAME", "PARENT", "FOLDER"),
                       help="Create custom widget: NAME PARENT_CLASS FOLDER_PATH")
    
    args = parser.parse_args()
    
    generator = UIWidgetGenerator()
    
    if args.all:
        generator.generate_all_widgets()
    elif args.custom:
        name, parent, folder = args.custom
        generator.create_widget_blueprint(name, parent, folder)
    else:
        parser.print_help()
        print("\nTIP: Use --all to generate all widgets")


if __name__ == "__main__":
    main()
