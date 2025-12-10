#!/usr/bin/env python3
"""
Adastrea - Master Content Generator

This is the one-stop script to generate ALL missing blueprints, data assets,
maps, UI widgets, and input configurations for the Adastrea project.

Features:
- Orchestrates all content generation scripts
- Generates Blueprints for all game systems
- Converts YAML templates to Data Assets
- Creates Enhanced Input configurations
- Generates basic test maps
- Creates UI widget blueprints
- Validates all generated content

Usage:
    # In Unreal Editor Python Console - Generate EVERYTHING
    import MasterContentGenerator
    MasterContentGenerator.generate_all_content()
    
    # Generate specific categories
    MasterContentGenerator.generate_blueprints()
    MasterContentGenerator.generate_data_assets()
    MasterContentGenerator.generate_input_system()
    MasterContentGenerator.generate_maps()
    MasterContentGenerator.generate_ui_widgets()
    
    # Command line - Interactive mode
    python MasterContentGenerator.py --interactive
    
    # Command line - Generate everything
    python MasterContentGenerator.py --all
    
    # Command line - Generate specific content
    python MasterContentGenerator.py --blueprints --data-assets
"""

import sys
from pathlib import Path
from typing import Dict, Any
from datetime import datetime

# Try to import Unreal Engine modules
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("WARNING: This script must be run inside Unreal Editor for full functionality!")
    print("Some features will be simulated or unavailable outside the editor.")


class MasterContentGenerator:
    """Master orchestrator for generating all Adastrea content"""
    
    def __init__(self):
        """Initialize the master generator"""
        if UNREAL_AVAILABLE:
            self.project_dir = Path(unreal.SystemLibrary.get_project_directory())
        else:
            self.project_dir = Path.cwd()
        
        self.content_dir = self.project_dir / "Content"
        self.assets_dir = self.project_dir / "Assets"
        
        # Statistics tracking
        self.stats = {
            "blueprints_created": 0,
            "data_assets_created": 0,
            "input_actions_created": 0,
            "maps_created": 0,
            "ui_widgets_created": 0,
            "niagara_systems_created": 0,
            "errors": []
        }
    
    def log(self, message: str, level: str = "info"):
        """Log message to console and Unreal if available"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        formatted_msg = f"[{timestamp}] [MasterGen] {message}"
        
        if UNREAL_AVAILABLE:
            if level == "error":
                unreal.log_error(formatted_msg)
            elif level == "warning":
                unreal.log_warning(formatted_msg)
            else:
                unreal.log(formatted_msg)
        else:
            prefix = "ERROR: " if level == "error" else "WARNING: " if level == "warning" else ""
            print(f"{prefix}{formatted_msg}")
    
    def generate_all_content(self, validate: bool = True) -> Dict[str, Any]:
        """
        Generate all missing content for the Adastrea project
        
        Args:
            validate: Whether to validate content after generation
            
        Returns:
            Dictionary with generation statistics
        """
        self.log("=" * 80)
        self.log("ADASTREA MASTER CONTENT GENERATOR")
        self.log("=" * 80)
        self.log("")
        self.log("Starting comprehensive content generation...")
        self.log("")
        
        # Phase 1: Essential Blueprints
        self.log("PHASE 1: Generating Essential Blueprints...")
        self.generate_blueprints(essential_only=True)
        
        # Phase 2: Data Assets from YAML
        self.log("")
        self.log("PHASE 2: Converting YAML Templates to Data Assets...")
        self.generate_data_assets()
        
        # Phase 3: Enhanced Input System
        self.log("")
        self.log("PHASE 3: Setting up Enhanced Input System...")
        self.generate_input_system()
        
        # Phase 4: Test Maps
        self.log("")
        self.log("PHASE 4: Creating Test Maps...")
        self.generate_maps()
        
        # Phase 5: UI Widgets
        self.log("")
        self.log("PHASE 5: Generating UI Widgets...")
        self.generate_ui_widgets()
        
        # Phase 6: Additional Blueprints
        self.log("")
        self.log("PHASE 6: Generating Additional Blueprints...")
        self.generate_blueprints(essential_only=False)
        
        # Phase 7: Niagara Particle Systems
        self.log("")
        self.log("PHASE 7: Generating Niagara Particle Systems...")
        self.generate_niagara_systems()
        
        # Phase 8: Validation
        if validate:
            self.log("")
            self.log("PHASE 8: Validating Generated Content...")
            self.validate_content()
        
        # Print summary
        self.log("")
        self.log("=" * 80)
        self.log("GENERATION COMPLETE!")
        self.log("=" * 80)
        self.print_summary()
        
        return self.stats
    
    def generate_blueprints(self, essential_only: bool = False) -> int:
        """
        Generate Blueprint assets
        
        Args:
            essential_only: If True, only generate critical blueprints
            
        Returns:
            Number of blueprints created
        """
        try:
            from BlueprintGenerator import BlueprintGenerator
            
            generator = BlueprintGenerator()
            
            if essential_only:
                self.log("Generating essential blueprints only...")
                count = generator.generate_essential_blueprints()
            else:
                self.log("Generating all blueprints...")
                count = generator.generate_all_blueprints()
            
            self.stats["blueprints_created"] += count
            self.log(f"✓ Created {count} blueprints", "info")
            return count
            
        except ImportError:
            self.log("BlueprintGenerator.py not found! Skipping blueprint generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating blueprints: {str(e)}", "error")
            self.stats["errors"].append(f"Blueprint generation: {str(e)}")
            return 0
    
    def generate_data_assets(self) -> int:
        """
        Generate Data Assets from YAML templates
        
        Returns:
            Number of data assets created
        """
        try:
            from DataAssetBatchCreator import DataAssetBatchCreator
            
            generator = DataAssetBatchCreator()
            count = generator.batch_convert_all()
            
            self.stats["data_assets_created"] += count
            self.log(f"✓ Created {count} data assets from YAML", "info")
            return count
            
        except ImportError:
            self.log("DataAssetBatchCreator.py not found! Skipping data asset generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating data assets: {str(e)}", "error")
            self.stats["errors"].append(f"Data asset generation: {str(e)}")
            return 0
    
    def generate_input_system(self) -> int:
        """
        Generate Enhanced Input system configuration
        
        Returns:
            Number of input assets created
        """
        try:
            from InputSystemGenerator import InputSystemGenerator
            
            generator = InputSystemGenerator()
            count = generator.generate_complete_input_system()
            
            self.stats["input_actions_created"] += count
            self.log(f"✓ Created {count} input assets", "info")
            return count
            
        except ImportError:
            self.log("InputSystemGenerator.py not found! Skipping input generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating input system: {str(e)}", "error")
            self.stats["errors"].append(f"Input generation: {str(e)}")
            return 0
    
    def generate_maps(self) -> int:
        """
        Generate test maps/levels
        
        Returns:
            Number of maps created
        """
        try:
            from MapGenerator import MapGenerator
            
            generator = MapGenerator()
            count = generator.generate_test_maps()
            
            self.stats["maps_created"] += count
            self.log(f"✓ Created {count} maps", "info")
            return count
            
        except ImportError:
            self.log("MapGenerator.py not found! Skipping map generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating maps: {str(e)}", "error")
            self.stats["errors"].append(f"Map generation: {str(e)}")
            return 0
    
    def generate_ui_widgets(self) -> int:
        """
        Generate UI widget blueprints
        
        Returns:
            Number of widgets created
        """
        try:
            from UIWidgetGenerator import UIWidgetGenerator
            
            generator = UIWidgetGenerator()
            count = generator.generate_all_widgets()
            
            self.stats["ui_widgets_created"] += count
            self.log(f"✓ Created {count} UI widgets", "info")
            return count
            
        except ImportError:
            self.log("UIWidgetGenerator.py not found! Skipping UI generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating UI widgets: {str(e)}", "error")
            self.stats["errors"].append(f"UI widget generation: {str(e)}")
            return 0
    
    def generate_niagara_systems(self) -> int:
        """
        Generate Niagara particle systems
        
        Returns:
            Number of systems created
        """
        try:
            from NiagaraGenerator import NiagaraGenerator
            
            generator = NiagaraGenerator()
            count = generator.generate_all_effects()
            
            self.stats["niagara_systems_created"] += count
            self.log(f"✓ Created {count} Niagara systems", "info")
            return count
            
        except ImportError:
            self.log("NiagaraGenerator.py not found! Skipping Niagara generation.", "warning")
            return 0
        except Exception as e:
            self.log(f"Error generating Niagara systems: {str(e)}", "error")
            self.stats["errors"].append(f"Niagara generation: {str(e)}")
            return 0
    
    def validate_content(self) -> bool:
        """
        Validate all generated content
        
        Returns:
            True if validation passed, False otherwise
        """
        try:
            from ContentValidator import ContentValidator
            
            validator = ContentValidator()
            is_valid = validator.validate_all()
            
            if is_valid:
                self.log("✓ All content validated successfully!", "info")
            else:
                self.log("✗ Some content failed validation. Check logs for details.", "warning")
            
            return is_valid
            
        except ImportError:
            self.log("ContentValidator.py not found! Skipping validation.", "warning")
            return False
        except Exception as e:
            self.log(f"Error validating content: {str(e)}", "error")
            self.stats["errors"].append(f"Validation: {str(e)}")
            return False
    
    def print_summary(self):
        """Print generation statistics summary"""
        self.log("")
        self.log("Generation Statistics:")
        self.log(f"  Blueprints Created:       {self.stats['blueprints_created']}")
        self.log(f"  Data Assets Created:      {self.stats['data_assets_created']}")
        self.log(f"  Input Actions Created:    {self.stats['input_actions_created']}")
        self.log(f"  Maps Created:             {self.stats['maps_created']}")
        self.log(f"  UI Widgets Created:       {self.stats['ui_widgets_created']}")
        self.log(f"  Niagara Systems Created:  {self.stats['niagara_systems_created']}")
        self.log("")
        
        total = (self.stats['blueprints_created'] + 
                self.stats['data_assets_created'] + 
                self.stats['input_actions_created'] + 
                self.stats['maps_created'] + 
                self.stats['ui_widgets_created'] + 
                self.stats['niagara_systems_created'])
        
        self.log(f"TOTAL ASSETS CREATED: {total}")
        
        if self.stats['errors']:
            self.log("")
            self.log(f"Errors Encountered: {len(self.stats['errors'])}", "warning")
            for error in self.stats['errors']:
                self.log(f"  - {error}", "warning")


# Convenience functions for direct import usage
def generate_all_content(validate: bool = True) -> Dict[str, Any]:
    """Generate all content - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_all_content(validate=validate)


def generate_blueprints(essential_only: bool = False) -> int:
    """Generate blueprints - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_blueprints(essential_only=essential_only)


def generate_data_assets() -> int:
    """Generate data assets - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_data_assets()


def generate_input_system() -> int:
    """Generate input system - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_input_system()


def generate_maps() -> int:
    """Generate maps - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_maps()


def generate_ui_widgets() -> int:
    """Generate UI widgets - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_ui_widgets()


def generate_niagara_systems() -> int:
    """Generate Niagara systems - convenience function"""
    generator = MasterContentGenerator()
    return generator.generate_niagara_systems()


def validate_content() -> bool:
    """Validate content - convenience function"""
    generator = MasterContentGenerator()
    return generator.validate_content()


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Master Content Generator - Generate all missing content"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Generate all content types")
    parser.add_argument("--blueprints", action="store_true",
                       help="Generate blueprints only")
    parser.add_argument("--data-assets", action="store_true",
                       help="Generate data assets only")
    parser.add_argument("--input", action="store_true",
                       help="Generate input system only")
    parser.add_argument("--maps", action="store_true",
                       help="Generate maps only")
    parser.add_argument("--ui", action="store_true",
                       help="Generate UI widgets only")
    parser.add_argument("--niagara", action="store_true",
                       help="Generate Niagara systems only")
    parser.add_argument("--validate", action="store_true",
                       help="Validate content after generation")
    parser.add_argument("--interactive", action="store_true",
                       help="Run in interactive mode")
    
    args = parser.parse_args()
    
    if not UNREAL_AVAILABLE:
        print("ERROR: This script must be run inside Unreal Editor!")
        print("Please run it from the Unreal Editor Python console.")
        sys.exit(1)
    
    generator = MasterContentGenerator()
    
    if args.interactive:
        # Interactive mode
        print("\n" + "=" * 80)
        print("ADASTREA MASTER CONTENT GENERATOR - Interactive Mode")
        print("=" * 80)
        print("\nWhat would you like to generate?")
        print("1. All content (recommended)")
        print("2. Essential blueprints only")
        print("3. Data assets from YAML")
        print("4. Enhanced input system")
        print("5. Test maps")
        print("6. UI widgets")
        print("7. Niagara particle systems")
        print("8. Validate existing content")
        print("0. Exit")
        
        choice = input("\nEnter your choice (0-8): ")
        
        if choice == "1":
            generator.generate_all_content(validate=True)
        elif choice == "2":
            generator.generate_blueprints(essential_only=True)
        elif choice == "3":
            generator.generate_data_assets()
        elif choice == "4":
            generator.generate_input_system()
        elif choice == "5":
            generator.generate_maps()
        elif choice == "6":
            generator.generate_ui_widgets()
        elif choice == "7":
            generator.generate_niagara_systems()
        elif choice == "8":
            generator.validate_content()
        elif choice == "0":
            print("Exiting...")
            return
        else:
            print("Invalid choice. Exiting...")
            return
    
    elif args.all:
        generator.generate_all_content(validate=args.validate)
    
    else:
        # Generate specific content types
        if args.blueprints:
            generator.generate_blueprints()
        if args.data_assets:
            generator.generate_data_assets()
        if args.input:
            generator.generate_input_system()
        if args.maps:
            generator.generate_maps()
        if args.ui:
            generator.generate_ui_widgets()
        if args.niagara:
            generator.generate_niagara_systems()
        if args.validate:
            generator.validate_content()
        
        # If no specific options, show help
        if not any([args.blueprints, args.data_assets, args.input, 
                   args.maps, args.ui, args.niagara, args.validate]):
            parser.print_help()
            print("\nTIP: Use --all to generate everything, or --interactive for guided mode")
    
    generator.print_summary()


if __name__ == "__main__":
    main()
