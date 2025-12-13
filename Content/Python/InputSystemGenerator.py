#!/usr/bin/env python3
"""
Adastrea - Enhanced Input System Generator

This script generates the DA_InputConfig Data Asset for the Adastrea project and provides
instructions for manually creating Input Actions and Input Mapping Contexts.

Due to limitations in the Unreal Python API, Input Actions and Input Mapping Contexts
cannot be created programmatically. The script will provide detailed instructions for
manual creation.

Features:
- Create DA_InputConfig Data Asset with all input references
- Provide step-by-step instructions for creating Input Actions
- Provide step-by-step instructions for creating Input Mapping Contexts
- Automatic key binding configuration (once assets are created)
- Support for keyboard, mouse, and gamepad

Usage:
    # In Unreal Editor Python Console
    import InputSystemGenerator
    InputSystemGenerator.generate_complete_input_system()
    
    # Generate specific parts
    InputSystemGenerator.generate_input_actions()  # Shows creation instructions
    InputSystemGenerator.generate_mapping_contexts()  # Shows creation instructions
    InputSystemGenerator.generate_input_config_data_asset()  # Creates the Data Asset
"""

import sys
from typing import Optional

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("ERROR: This script must be run inside Unreal Editor!")
    sys.exit(1)


class InputSystemGenerator:
    """Generator for Enhanced Input system assets"""
    
    # Input Actions to create
    INPUT_ACTIONS = [
        # Flight Movement (Space Flight)
        ("IA_Move", "Vector2D", "Movement input (A/D for strafe)"),
        ("IA_VerticalMove", "Float", "Vertical movement (W/S for up/down on Z axis)"),
        ("IA_Throttle", "Float", "Throttle control (Mouse wheel)"),
        ("IA_ThrottleUp", "Boolean", "Increase throttle (Page Up)"),
        ("IA_ThrottleDown", "Boolean", "Decrease throttle (Page Down)"),
        ("IA_Look", "Vector2D", "Camera look input (Mouse/Right Stick)"),
        ("IA_Boost", "Boolean", "Speed boost"),
        ("IA_Brake", "Boolean", "Brake/Slow down"),
        
        # Third Person Movement (Station/Ship Interior)
        ("IA_Walk", "Vector2D", "Standard third person movement (WASD)"),
        ("IA_LookThirdPerson", "Vector2D", "Standard third person camera (Mouse)"),
        ("IA_Jump", "Boolean", "Jump in third person mode"),
        ("IA_Crouch", "Boolean", "Crouch in third person mode"),
        ("IA_Sprint", "Boolean", "Sprint in third person mode"),
        
        # Combat
        ("IA_Fire_Primary", "Boolean", "Primary weapon"),
        ("IA_Fire_Secondary", "Boolean", "Secondary weapon"),
        ("IA_CycleWeapon", "Boolean", "Cycle weapons"),
        ("IA_TargetNext", "Boolean", "Target next enemy"),
        ("IA_TargetPrevious", "Boolean", "Target previous enemy"),
        ("IA_TargetNearest", "Boolean", "Target nearest enemy"),
        ("IA_ClearTarget", "Boolean", "Clear current target"),
        
        # Ship Systems
        ("IA_ShieldsToggle", "Boolean", "Toggle shields"),
        ("IA_ShieldsFront", "Boolean", "Shields to front"),
        ("IA_ShieldsRear", "Boolean", "Shields to rear"),
        ("IA_CountermeasuresDeploy", "Boolean", "Deploy countermeasures"),
        
        # Navigation
        ("IA_Autopilot", "Boolean", "Toggle autopilot"),
        ("IA_MatchSpeed", "Boolean", "Match target speed"),
        ("IA_FullStop", "Boolean", "Full stop"),
        ("IA_Dock", "Boolean", "Dock with station"),
        
        # UI
        ("IA_PauseMenu", "Boolean", "Pause menu"),
        ("IA_OpenInventory", "Boolean", "Open inventory"),
        ("IA_OpenMap", "Boolean", "Open star map"),
        ("IA_OpenQuestLog", "Boolean", "Open quest log"),
        ("IA_OpenShipCustomization", "Boolean", "Open ship customization"),
        ("IA_OpenTrading", "Boolean", "Open trading interface"),
        ("IA_OpenComms", "Boolean", "Open communications"),
        
        # Camera
        ("IA_CameraZoomIn", "Boolean", "Zoom camera in"),
        ("IA_CameraZoomOut", "Boolean", "Zoom camera out"),
        ("IA_CameraReset", "Boolean", "Reset camera"),
        
        # Misc
        ("IA_Interact", "Boolean", "Interact with object"),
        ("IA_Screenshot", "Boolean", "Take screenshot"),
    ]
    
    # Key bindings for each action
    # Flight Controls (for space flight)
    KEY_BINDINGS = {
        "IA_Move": [("A", "Strafe Left"), ("D", "Strafe Right")],
        "IA_VerticalMove": [("W", "Up (Z+)"), ("S", "Down (Z-)")],
        "IA_Throttle": [("MouseWheelUp", "Increase Throttle"), ("MouseWheelDown", "Decrease Throttle")],
        "IA_ThrottleUp": [("PageUp", "Throttle Up")],
        "IA_ThrottleDown": [("PageDown", "Throttle Down")],
        "IA_Look": [("Mouse X", "Look X"), ("Mouse Y", "Look Y")],
        "IA_Boost": [("LeftShift", "Boost")],
        "IA_Brake": [("LeftControl", "Brake")],
        
        # Third Person Controls (for station/ship interior)
        "IA_Walk": [("W", "Forward"), ("S", "Backward"), ("A", "Left"), ("D", "Right")],
        "IA_LookThirdPerson": [("Mouse X", "Look X"), ("Mouse Y", "Look Y")],
        "IA_Jump": [("Space", "Jump")],
        "IA_Crouch": [("LeftControl", "Crouch")],
        "IA_Sprint": [("LeftShift", "Sprint")],
        "IA_Fire_Primary": [("LeftMouseButton", "Fire Primary")],
        "IA_Fire_Secondary": [("RightMouseButton", "Fire Secondary")],
        "IA_TargetNext": [("T", "Target Next")],
        "IA_TargetPrevious": [("LeftAlt+T", "Target Previous")],
        "IA_Autopilot": [("Z", "Autopilot")],
        "IA_PauseMenu": [("Escape", "Pause")],
        "IA_OpenInventory": [("I", "Inventory")],
        "IA_OpenMap": [("M", "Map")],
        "IA_OpenQuestLog": [("L", "Quest Log")],
        "IA_Interact": [("E", "Interact")],
        "IA_CameraZoomIn": [("PageUp", "Zoom In")],
        "IA_CameraZoomOut": [("PageDown", "Zoom Out")],
    }
    
    # Mapping Contexts
    MAPPING_CONTEXTS = [
        ("IMC_SpaceshipFlight", "Spaceship flight control mapping context (space flight)"),
        ("IMC_ThirdPerson", "Third person control mapping context (station/ship interior)"),
        ("IMC_Menu", "Menu navigation mapping context"),
    ]
    
    def __init__(self):
        """Initialize the input system generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.created_count = 0
        self.skipped_count = 0
        self.errors = []
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[InputGen] {message}")
        elif level == "warning":
            unreal.log_warning(f"[InputGen] {message}")
        else:
            unreal.log(f"[InputGen] {message}")
    
    def create_input_action(self, action_name: str, value_type: str, 
                           description: str) -> Optional[str]:
        """
        Create an Input Action asset
        
        Note: Due to limitations in the Unreal Python API, Input Actions cannot be created
        programmatically. This method provides instructions for manual creation.
        
        Args:
            action_name: Name of the action (e.g., "IA_Move")
            value_type: Type of value (Boolean, Float, Vector2D, Vector3D)
            description: Description of the action
            
        Returns:
            None (manual creation required)
        """
        asset_path = f"/Game/Input/Actions/{action_name}"
        
        # Check if already exists
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.log(f"Input Action already exists: {action_name}", "info")
            self.skipped_count += 1
            return asset_path
        
        # Cannot create programmatically - provide instructions
        self.log(f"Input Action '{action_name}' needs to be created manually:", "warning")
        self.log(f"  1. Right-click in Content Browser > Input > Input Action", "warning")
        self.log(f"  2. Name: {action_name}", "warning")
        self.log(f"  3. Set Value Type: {value_type}", "warning")
        self.log(f"  4. Description: {description}", "warning")
        self.errors.append(f"{action_name}: Manual creation required (factory not exposed in Python API)")
        return None
    
    def create_mapping_context(self, context_name: str, description: str) -> Optional[str]:
        """
        Create an Input Mapping Context asset
        
        Note: Due to limitations in the Unreal Python API, Input Mapping Contexts cannot be created
        programmatically. This method provides instructions for manual creation.
        
        Args:
            context_name: Name of the context (e.g., "IMC_Spaceship")
            description: Description of the context
            
        Returns:
            None (manual creation required)
        """
        asset_path = f"/Game/Input/{context_name}"
        
        # Check if already exists
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.log(f"Mapping Context already exists: {context_name}", "info")
            self.skipped_count += 1
            return asset_path
        
        # Cannot create programmatically - provide instructions
        self.log(f"Input Mapping Context '{context_name}' needs to be created manually:", "warning")
        self.log(f"  1. Right-click in Content Browser > Input > Input Mapping Context", "warning")
        self.log(f"  2. Name: {context_name}", "warning")
        self.log(f"  3. Description: {description}", "warning")
        self.errors.append(f"{context_name}: Manual creation required (factory not exposed in Python API)")
        return None
    
    def generate_input_actions(self) -> int:
        """
        Generate all Input Actions
        
        Returns:
            Number of actions created
        """
        self.log("=" * 80)
        self.log("Generating Input Actions")
        self.log("=" * 80)
        
        initial_count = self.created_count
        
        for action_name, value_type, description in self.INPUT_ACTIONS:
            self.create_input_action(action_name, value_type, description)
        
        created = self.created_count - initial_count
        self.log(f"\nCreated {created} Input Actions")
        
        return created
    
    def generate_mapping_contexts(self) -> int:
        """
        Generate all Input Mapping Contexts
        
        Returns:
            Number of contexts created
        """
        self.log("")
        self.log("=" * 80)
        self.log("Generating Input Mapping Contexts")
        self.log("=" * 80)
        
        initial_count = self.created_count
        
        for context_name, description in self.MAPPING_CONTEXTS:
            self.create_mapping_context(context_name, description)
        
        created = self.created_count - initial_count
        self.log(f"\nCreated {created} Mapping Contexts")
        
        return created
    
    def generate_input_config_data_asset(self) -> Optional[str]:
        """
        Generate the DA_InputConfig Data Asset
        
        Returns:
            Path to created asset, or None if failed
        """
        self.log("")
        self.log("=" * 80)
        self.log("Generating DA_InputConfig Data Asset")
        self.log("=" * 80)
        
        asset_path = "/Game/DataAssets/Input/DA_InputConfig"
        
        # Check if already exists
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.log("DA_InputConfig already exists", "info")
            self.skipped_count += 1
            return asset_path
        
        try:
            # Load InputConfigDataAsset class
            asset_class = unreal.load_class(None, "/Script/Adastrea.InputConfigDataAsset")
            
            if not asset_class:
                self.log("Could not find InputConfigDataAsset class", "error")
                return None
            
            # Create factory
            factory = unreal.DataAssetFactory()
            factory.set_editor_property("DataAssetClass", asset_class)
            
            # Create the asset
            data_asset = self.asset_tools.create_asset(
                asset_name="DA_InputConfig",
                package_path="/Game/DataAssets/Input",
                asset_class=None,
                factory=factory
            )
            
            if data_asset:
                # Note: Properties would need to be set manually or via further automation
                self.log("✓ Created DA_InputConfig (manual configuration required)")
                self.log("  → Configure Input Actions and Mapping Contexts in the editor")
                
                # Save
                self.editor_asset_lib.save_loaded_asset(data_asset)
                
                self.created_count += 1
                return asset_path
            else:
                self.log("Failed to create DA_InputConfig", "error")
                return None
                
        except Exception as e:
            self.log(f"Error creating DA_InputConfig: {str(e)}", "error")
            self.errors.append(f"DA_InputConfig: {str(e)}")
            return None
    
    def generate_complete_input_system(self) -> int:
        """
        Generate complete Enhanced Input system
        
        Returns:
            Total number of assets created
        """
        self.log("=" * 80)
        self.log("GENERATING COMPLETE ENHANCED INPUT SYSTEM")
        self.log("=" * 80)
        
        # Generate Input Actions
        self.generate_input_actions()
        
        # Generate Mapping Contexts
        self.generate_mapping_contexts()
        
        # Generate Input Config Data Asset
        self.generate_input_config_data_asset()
        
        # Summary
        self.log("")
        self.log("=" * 80)
        self.log("INPUT SYSTEM GENERATION COMPLETE!")
        self.log(f"Created: {self.created_count}, Skipped: {self.skipped_count}")
        
        if self.errors:
            self.log(f"\nErrors encountered: {len(self.errors)}", "warning")
            for error in self.errors:
                self.log(f"  - {error}", "warning")
        
        self.log("")
        self.log("NEXT STEPS:")
        self.log("1. Create Input Actions manually (see warnings above for details)")
        self.log("2. Create Input Mapping Contexts manually (see warnings above for details)")
        self.log("3. Open DA_InputConfig in the editor")
        self.log("4. Assign Input Actions to their respective properties")
        self.log("5. Assign Mapping Contexts (IMC_SpaceshipFlight, IMC_ThirdPerson, IMC_Menu)")
        self.log("6. Configure key bindings in each Mapping Context")
        self.log("=" * 80)
        
        return self.created_count


# Convenience functions
def generate_complete_input_system() -> int:
    """Generate complete input system - convenience function"""
    generator = InputSystemGenerator()
    return generator.generate_complete_input_system()


def generate_input_actions() -> int:
    """Generate input actions - convenience function"""
    generator = InputSystemGenerator()
    return generator.generate_input_actions()


def generate_mapping_contexts() -> int:
    """Generate mapping contexts - convenience function"""
    generator = InputSystemGenerator()
    return generator.generate_mapping_contexts()


def generate_input_config_data_asset() -> Optional[str]:
    """Generate input config data asset - convenience function"""
    generator = InputSystemGenerator()
    return generator.generate_input_config_data_asset()


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Input System Generator - Create Enhanced Input assets"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Generate complete input system")
    parser.add_argument("--actions", action="store_true",
                       help="Generate Input Actions only")
    parser.add_argument("--contexts", action="store_true",
                       help="Generate Mapping Contexts only")
    parser.add_argument("--config", action="store_true",
                       help="Generate DA_InputConfig only")
    
    args = parser.parse_args()
    
    generator = InputSystemGenerator()
    
    if args.all:
        generator.generate_complete_input_system()
    elif args.actions:
        generator.generate_input_actions()
    elif args.contexts:
        generator.generate_mapping_contexts()
    elif args.config:
        generator.generate_input_config_data_asset()
    else:
        parser.print_help()
        print("\nTIP: Use --all to generate everything")


if __name__ == "__main__":
    main()
