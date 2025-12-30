#!/usr/bin/env python3
"""
Adastrea - Unreal Editor Utilities Script

This script provides utilities that can be run inside the Unreal Editor's Python environment.
It combines template generation, guide generation, and scene population into a unified tool.

To use this script in Unreal Editor:
1. Enable the "Python Editor Script Plugin" in Unreal Engine
2. Go to Tools → Python → Execute Python Script
3. Select this script to run
4. Or import it and use the functions directly

Features:
- Generate templates (YAML) and guides (Markdown)
- List existing templates and guides
- Validate template structure
- Interactive menu system
- Command-line compatible
- Scene and interior population

Usage in Unreal Editor Python Console:
    import EditorUtilities
    EditorUtilities.show_menu()
    
    # Or call specific functions
    EditorUtilities.generate_spaceship("MyShip", "Fighter")
    EditorUtilities.generate_system_guide("MySystem")
    EditorUtilities.populate_from_config("scene_config.yaml")
"""

import os
import sys
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime

# Try to import Unreal Engine modules (will be available in Editor)
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("Note: Running outside Unreal Editor. Some features may be limited.")

# Import our generator modules
try:
    from TemplateGenerator import TemplateGenerator
    from GuideGenerator import GuideGenerator
    GENERATORS_AVAILABLE = True
except ImportError:
    GENERATORS_AVAILABLE = False
    print("Warning: Could not import generator modules. They should be in the same directory.")

# Import scene population modules
try:
    from ScenePopulator import ScenePopulator
    from InteriorLayoutPopulator import InteriorLayoutPopulator
    POPULATION_AVAILABLE = True
except ImportError:
    POPULATION_AVAILABLE = False
    print("Warning: Could not import population modules.")


class EditorUtilities:
    """Unified utilities for template and guide generation in Unreal Editor"""
    
    def __init__(self):
        """Initialize the editor utilities"""
        # Determine project root
        if UNREAL_AVAILABLE:
            # In Unreal Editor, get project directory
            self.project_root = Path(unreal.SystemLibrary.get_project_directory())
        else:
            # Outside editor, use current directory
            self.project_root = Path.cwd()
        
        # Initialize generators if available
        if GENERATORS_AVAILABLE:
            self.template_gen = TemplateGenerator(self.project_root)
            self.guide_gen = GuideGenerator(self.project_root)
        
        self.assets_dir = self.project_root / "Assets"
        self.content_dir = self.project_root / "Content"
    
    def log_message(self, message: str, level: str = "info"):
        """
        Log a message to Unreal Editor log or console
        
        Args:
            message: Message to log
            level: Log level (info, warning, error)
        """
        if UNREAL_AVAILABLE:
            if level == "error":
                unreal.log_error(message)
            elif level == "warning":
                unreal.log_warning(message)
            else:
                unreal.log(message)
        else:
            prefix = {"error": "ERROR", "warning": "WARNING", "info": "INFO"}
            print(f"[{prefix.get(level, 'INFO')}] {message}")
    
    # ==================== Template Generation ====================
    
    def generate_spaceship(self, name: str, ship_class: str = "Fighter") -> Optional[Path]:
        """Generate a spaceship template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_spaceship_template(name, ship_class)
            self.log_message(f"Successfully created spaceship template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create spaceship template: {e}", "error")
            return None
    
    def generate_personnel(self, name: str, role: str = "CrewMember") -> Optional[Path]:
        """Generate a personnel template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_personnel_template(name, role)
            self.log_message(f"Successfully created personnel template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create personnel template: {e}", "error")
            return None
    
    def generate_trade_item(self, name: str, category: str = "RawMaterials") -> Optional[Path]:
        """Generate a trade item template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_trade_item_template(name, category)
            self.log_message(f"Successfully created trade item template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create trade item template: {e}", "error")
            return None
    
    def generate_market(self, name: str, market_type: str = "Standard") -> Optional[Path]:
        """Generate a market template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_market_template(name, market_type)
            self.log_message(f"Successfully created market template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create market template: {e}", "error")
            return None
    
    def generate_contract(self, name: str, contract_type: str = "Delivery") -> Optional[Path]:
        """Generate a contract template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_contract_template(name, contract_type)
            self.log_message(f"Successfully created contract template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create contract template: {e}", "error")
            return None
    
    def generate_faction_ai(self, name: str) -> Optional[Path]:
        """Generate a faction AI template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_faction_ai_template(name)
            self.log_message(f"Successfully created faction AI template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create faction AI template: {e}", "error")
            return None
    
    def generate_material(self, name: str) -> Optional[Path]:
        """Generate a material template"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.template_gen.generate_material_template(name)
            self.log_message(f"Successfully created material template: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create material template: {e}", "error")
            return None
    
    # ==================== Guide Generation ====================
    
    def generate_system_guide(self, system_name: str) -> Optional[Path]:
        """Generate a complete system guide"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.guide_gen.generate_system_guide(system_name)
            self.log_message(f"Successfully created system guide: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create system guide: {e}", "error")
            return None
    
    def generate_workflow_guide(self, system_name: str) -> Optional[Path]:
        """Generate a designer workflow guide"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.guide_gen.generate_workflow_guide(system_name)
            self.log_message(f"Successfully created workflow guide: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create workflow guide: {e}", "error")
            return None
    
    def generate_quick_reference(self, system_name: str) -> Optional[Path]:
        """Generate a quick reference guide"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.guide_gen.generate_quick_reference(system_name)
            self.log_message(f"Successfully created quick reference: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create quick reference: {e}", "error")
            return None
    
    def generate_api_reference(self, system_name: str) -> Optional[Path]:
        """Generate an API reference guide"""
        if not GENERATORS_AVAILABLE:
            self.log_message("Generator modules not available", "error")
            return None
        
        try:
            filepath = self.guide_gen.generate_api_reference(system_name)
            self.log_message(f"Successfully created API reference: {filepath}")
            return filepath
        except Exception as e:
            self.log_message(f"Failed to create API reference: {e}", "error")
            return None
    
    # ==================== Listing and Info ====================
    
    def list_templates(self, template_type: Optional[str] = None) -> Dict[str, List[Path]]:
        """
        List existing template files
        
        Args:
            template_type: Filter by type (spaceship, personnel, trading, etc.) or None for all
            
        Returns:
            Dictionary mapping template types to lists of file paths
        """
        templates = {
            "Spaceship": [],
            "Personnel": [],
            "Trading": [],
            "FactionAI": [],
            "Material": []
        }
        
        # Spaceship templates
        spaceship_dir = self.assets_dir / "SpaceshipTemplates"
        if spaceship_dir.exists():
            templates["Spaceship"] = sorted(spaceship_dir.glob("*.yaml"))
        
        # Personnel templates
        personnel_dir = self.assets_dir / "PersonnelTemplates"
        if personnel_dir.exists():
            templates["Personnel"] = sorted(personnel_dir.glob("*.yaml"))
        
        # Trading templates
        trading_dir = self.assets_dir / "TradingTemplates"
        if trading_dir.exists():
            templates["Trading"] = sorted(trading_dir.glob("*.yaml"))
        
        # Faction AI templates
        faction_ai_dir = self.assets_dir / "FactionAITemplates"
        if faction_ai_dir.exists():
            templates["FactionAI"] = sorted(faction_ai_dir.glob("*.yaml"))
        
        # Material templates
        material_files = list(self.assets_dir.glob("Material*.yaml"))
        templates["Material"] = sorted(material_files)
        
        return templates
    
    def list_guides(self) -> List[Path]:
        """List existing guide files"""
        if self.assets_dir.exists():
            guides = sorted(self.assets_dir.glob("*Guide.md"))
            guides.extend(sorted(self.assets_dir.glob("*Workflow.md")))
            guides.extend(sorted(self.assets_dir.glob("*Reference.md")))
            return guides
        return []
    
    def print_templates_summary(self):
        """Print a summary of existing templates"""
        templates = self.list_templates()
        
        print("\n" + "=" * 60)
        print("Existing Templates Summary")
        print("=" * 60)
        
        total = 0
        for category, files in templates.items():
            count = len(files)
            total += count
            print(f"\n{category} Templates: {count}")
            for f in files[:5]:  # Show first 5
                print(f"  - {f.name}")
            if count > 5:
                print(f"  ... and {count - 5} more")
        
        print(f"\nTotal Templates: {total}")
        print("=" * 60)
    
    def print_guides_summary(self):
        """Print a summary of existing guides"""
        guides = self.list_guides()
        
        print("\n" + "=" * 60)
        print("Existing Guides Summary")
        print("=" * 60)
        
        print(f"\nTotal Guides: {len(guides)}")
        for guide in guides[:10]:  # Show first 10
            print(f"  - {guide.name}")
        if len(guides) > 10:
            print(f"  ... and {len(guides) - 10} more")
        
        print("=" * 60)
    
    # ==================== Interactive Menu ====================
    
    def show_menu(self):
        """Show interactive menu for utilities"""
        while True:
            print("\n" + "=" * 60)
            print("Adastrea Editor Utilities")
            print("=" * 60)
            print("\nTemplate Generation:")
            print("  1. Generate Spaceship Template")
            print("  2. Generate Personnel Template")
            print("  3. Generate Trade Item Template")
            print("  4. Generate Market Template")
            print("  5. Generate Contract Template")
            print("  6. Generate Faction AI Template")
            print("  7. Generate Material Template")
            print("\nGuide Generation:")
            print("  8. Generate System Guide")
            print("  9. Generate Workflow Guide")
            print("  10. Generate Quick Reference")
            print("  11. Generate API Reference")
            print("\nInformation:")
            print("  12. List All Templates")
            print("  13. List All Guides")
            print("\nBatch Operations:")
            print("  14. Generate Complete System Documentation Set")
            print("\nScene Population:")
            print("  15. Populate Level from Config")
            print("  16. Spawn Actors (Grid Pattern)")
            print("  17. Spawn Actors (Orbit Pattern)")
            print("  18. Populate Station Interior")
            print("  19. Clear Spawned Actors")
            print("\n  0. Exit")
            print("=" * 60)
            
            choice = input("\nSelect option (0-19): ").strip()
            
            if choice == "0":
                print("Exiting...")
                break
            elif choice == "1":
                name = input("Ship name: ").strip()
                ship_class = input("Ship class (Fighter/Scout/Trader/etc.): ").strip() or "Fighter"
                self.generate_spaceship(name, ship_class)
            elif choice == "2":
                name = input("Personnel name: ").strip()
                role = input("Role (Captain/Pilot/Engineer/etc.): ").strip() or "CrewMember"
                self.generate_personnel(name, role)
            elif choice == "3":
                name = input("Item name: ").strip()
                category = input("Category (RawMaterials/Manufactured/etc.): ").strip() or "RawMaterials"
                self.generate_trade_item(name, category)
            elif choice == "4":
                name = input("Market name: ").strip()
                market_type = input("Market type (Standard/FreePort/BlackMarket): ").strip() or "Standard"
                self.generate_market(name, market_type)
            elif choice == "5":
                name = input("Contract name: ").strip()
                contract_type = input("Contract type (Delivery/Combat/etc.): ").strip() or "Delivery"
                self.generate_contract(name, contract_type)
            elif choice == "6":
                name = input("Faction name: ").strip()
                self.generate_faction_ai(name)
            elif choice == "7":
                name = input("Material name: ").strip()
                self.generate_material(name)
            elif choice == "8":
                name = input("System name: ").strip()
                self.generate_system_guide(name)
            elif choice == "9":
                name = input("System name: ").strip()
                self.generate_workflow_guide(name)
            elif choice == "10":
                name = input("System name: ").strip()
                self.generate_quick_reference(name)
            elif choice == "11":
                name = input("System name: ").strip()
                self.generate_api_reference(name)
            elif choice == "12":
                self.print_templates_summary()
            elif choice == "13":
                self.print_guides_summary()
            elif choice == "14":
                name = input("System name: ").strip()
                self.generate_complete_documentation_set(name)
            elif choice == "15":
                config_path = input("Config file path (YAML/CSV): ").strip()
                self.populate_from_config(config_path)
            elif choice == "16":
                asset_path = input("Asset path (e.g., /Game/Ships/BP_Fighter): ").strip()
                count = int(input("Number of actors: ").strip() or "10")
                spacing = float(input("Grid spacing: ").strip() or "5000")
                self.spawn_actors_grid(asset_path, count, spacing)
            elif choice == "17":
                asset_path = input("Asset path (e.g., /Game/Ships/BP_Fighter): ").strip()
                count = int(input("Number of actors: ").strip() or "8")
                radius = float(input("Orbit radius: ").strip() or "10000")
                self.spawn_actors_orbit(asset_path, count, radius)
            elif choice == "18":
                theme = input("Theme (military/luxury/industrial/civilian): ").strip() or "military"
                self.populate_station_interior(theme)
            elif choice == "19":
                self.clear_spawned_actors()
            else:
                print("Invalid choice. Please select 0-19.")
    
    def generate_complete_documentation_set(self, system_name: str):
        """Generate a complete set of documentation for a system"""
        print(f"\nGenerating complete documentation set for {system_name}...")
        print("This will create:")
        print("  - System Guide")
        print("  - Workflow Guide")
        print("  - Quick Reference")
        print("  - API Reference")
        print()
        
        confirm = input("Continue? (y/n): ").strip().lower()
        if confirm != 'y':
            print("Cancelled.")
            return
        
        results = []
        
        # Generate all guides
        result = self.generate_system_guide(system_name)
        results.append(("System Guide", result))
        
        result = self.generate_workflow_guide(system_name)
        results.append(("Workflow Guide", result))
        
        result = self.generate_quick_reference(system_name)
        results.append(("Quick Reference", result))
        
        result = self.generate_api_reference(system_name)
        results.append(("API Reference", result))
        
        # Summary
        print("\n" + "=" * 60)
        print("Generation Complete")
        print("=" * 60)
        for name, path in results:
            status = "✓ Success" if path else "✗ Failed"
            print(f"{status}: {name}")
            if path:
                print(f"  → {path}")
        print("=" * 60)


    # ==================== Scene Population Functions ====================
    
    def populate_from_config(self, config_path: str) -> bool:
        """
        Populate level from YAML/CSV configuration file
        
        Args:
            config_path: Path to configuration file
            
        Returns:
            True if successful
        """
        if not POPULATION_AVAILABLE:
            self.log_message("Scene population modules not available", "error")
            return False
        
        populator = ScenePopulator()
        return populator.populate_from_config(config_path)
    
    def spawn_actors_grid(self, asset_path: str, count: int = 10, spacing: float = 5000) -> List[Any]:
        """
        Spawn actors in grid pattern
        
        Args:
            asset_path: Asset path
            count: Number of actors
            spacing: Grid spacing
            
        Returns:
            List of spawned actors
        """
        if not POPULATION_AVAILABLE:
            self.log_message("Scene population modules not available", "error")
            return []
        
        populator = ScenePopulator()
        return populator.spawn_actors_pattern(asset_path, 'grid', count, {'spacing': spacing})
    
    def spawn_actors_orbit(self, asset_path: str, count: int = 8, radius: float = 10000) -> List[Any]:
        """
        Spawn actors in orbit pattern
        
        Args:
            asset_path: Asset path
            count: Number of actors
            radius: Orbit radius
            
        Returns:
            List of spawned actors
        """
        if not POPULATION_AVAILABLE:
            self.log_message("Scene population modules not available", "error")
            return []
        
        populator = ScenePopulator()
        return populator.spawn_actors_pattern(asset_path, 'orbit', count, {'radius': radius})
    
    def populate_station_interior(self, theme: str = 'military') -> bool:
        """
        Populate a basic station interior
        
        Args:
            theme: Theme name (military, luxury, industrial, civilian)
            
        Returns:
            True if successful
        """
        if not POPULATION_AVAILABLE:
            self.log_message("Interior population modules not available", "error")
            return False
        
        populator = InteriorLayoutPopulator()
        
        # Create basic station layout
        populator.create_room('bridge', (0, 0, 100), (3000, 3000, 400), theme)
        populator.create_corridor((1500, 0, 100), (5000, 0, 100), theme=theme)
        populator.create_room('cargo_bay', (5000, 0, 100), (4000, 4000, 600), theme)
        
        self.log_message(f"Created basic station interior with {theme} theme")
        return True
    
    def clear_spawned_actors(self):
        """Clear all spawned actors"""
        if not POPULATION_AVAILABLE:
            self.log_message("Scene population modules not available", "error")
            return
        
        populator = ScenePopulator()
        populator.clear_spawned_actors()
        self.log_message("Cleared spawned actors")


# ==================== Module Functions ====================

# Create global instance
_utilities = None

def get_utilities() -> EditorUtilities:
    """Get or create the global utilities instance"""
    global _utilities
    if _utilities is None:
        _utilities = EditorUtilities()
    return _utilities


# Convenience functions for direct import
def show_menu():
    """Show the interactive menu"""
    get_utilities().show_menu()


def generate_spaceship(name: str, ship_class: str = "Fighter"):
    """Generate a spaceship template"""
    return get_utilities().generate_spaceship(name, ship_class)


def generate_personnel(name: str, role: str = "CrewMember"):
    """Generate a personnel template"""
    return get_utilities().generate_personnel(name, role)


def generate_trade_item(name: str, category: str = "RawMaterials"):
    """Generate a trade item template"""
    return get_utilities().generate_trade_item(name, category)


def generate_system_guide(system_name: str):
    """Generate a system guide"""
    return get_utilities().generate_system_guide(system_name)


def generate_workflow_guide(system_name: str):
    """Generate a workflow guide"""
    return get_utilities().generate_workflow_guide(system_name)


def list_templates():
    """List all templates"""
    get_utilities().print_templates_summary()


def list_guides():
    """List all guides"""
    get_utilities().print_guides_summary()


def generate_complete_docs(system_name: str):
    """Generate complete documentation set for a system"""
    get_utilities().generate_complete_documentation_set(system_name)


# ==================== Main Entry Point ====================

def main():
    """Main entry point"""
    utils = EditorUtilities()
    
    # If running with command line arguments
    if len(sys.argv) > 1:
        import argparse
        parser = argparse.ArgumentParser(description="Adastrea Editor Utilities")
        parser.add_argument('--menu', action='store_true', help='Show interactive menu')
        parser.add_argument('--list-templates', action='store_true', help='List all templates')
        parser.add_argument('--list-guides', action='store_true', help='List all guides')
        
        args = parser.parse_args()
        
        if args.list_templates:
            utils.print_templates_summary()
        elif args.list_guides:
            utils.print_guides_summary()
        elif args.menu:
            utils.show_menu()
        else:
            parser.print_help()
    else:
        # No arguments, show menu
        utils.show_menu()


if __name__ == '__main__':
    main()
