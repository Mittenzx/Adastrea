#!/usr/bin/env python3
"""
Configure PIE Setup via Unreal MCP
===================================

This script configures Blueprints for PIE functionality using Unreal's Python API.
Designed to be executed via Unreal MCP (Model Context Protocol) remote execution.

USAGE VIA UNREAL MCP:
Run this script through your MCP client that has access to Unreal Engine.

USAGE IN UNREAL EDITOR:
1. Window → Developer Tools → Output Log
2. Change console to "Python"
3. Paste the code and press Enter

OR:
1. Tools → Execute Python Script
2. Select this file
"""

import unreal

def log_success(message):
    """Log success message"""
    unreal.log(f"[PIE Setup] ✓ {message}")

def log_error(message):
    """Log error message"""
    unreal.log_error(f"[PIE Setup] ✗ {message}")

def log_info(message):
    """Log info message"""
    unreal.log(f"[PIE Setup] {message}")

def configure_game_mode():
    """Configure BP_TestGameMode with default pawn and controller classes"""
    log_info("Configuring BP_TestGameMode...")
    
    try:
        # Load Blueprint assets
        game_mode_bp = unreal.load_asset("/Game/Blueprints/BP_TestGameMode")
        player_ship_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_PlayerShip")
        player_controller_bp = unreal.load_asset("/Game/Blueprints/Controllers/BP_PlayerController")
        
        if not game_mode_bp:
            log_error("Failed to load BP_TestGameMode")
            return False
        
        if not player_ship_bp:
            log_error("Failed to load BP_PlayerShip")
            return False
        
        if not player_controller_bp:
            log_error("Failed to load BP_PlayerController")
            return False
        
        # Get the generated classes from the Blueprints
        game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
        player_ship_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C")
        player_controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
        
        # Get default object and set properties
        gm_default = unreal.get_default_object(game_mode_class)
        gm_default.set_editor_property("default_pawn_class", player_ship_class)
        gm_default.set_editor_property("player_controller_class", player_controller_class)
        
        # Save the asset
        unreal.EditorAssetLibrary.save_loaded_asset(game_mode_bp)
        
        log_success("BP_TestGameMode configured")
        log_info("  - Default Pawn Class: BP_PlayerShip")
        log_info("  - Player Controller Class: BP_PlayerController")
        
        return True
        
    except Exception as e:
        log_error(f"Failed to configure BP_TestGameMode: {e}")
        return False

def configure_player_controller():
    """Configure BP_PlayerController with HUD widget and input settings"""
    log_info("Configuring BP_PlayerController...")
    log_info("Note: HUD Widget Class must be set manually in Blueprint")
    log_info("The C++ property may not be visible until Blueprint is reopened")
    
    try:
        # Load Blueprint assets
        player_controller_bp = unreal.load_asset("/Game/Blueprints/Controllers/BP_PlayerController")
        
        if not player_controller_bp:
            log_error("Failed to load BP_PlayerController")
            return False
        
        # Get the generated classes
        player_controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
        
        # Get default object and set properties
        pc_default = unreal.get_default_object(player_controller_class)
        
        # Set show mouse cursor (this property should always work)
        pc_default.set_editor_property("show_mouse_cursor", False)
        log_info("  - Show Mouse Cursor: False")
        
        # Save the asset
        unreal.EditorAssetLibrary.save_loaded_asset(player_controller_bp)
        
        log_success("BP_PlayerController configured (mouse cursor disabled)")
        log_info("  MANUAL STEP REQUIRED:")
        log_info("  1. Open BP_PlayerController Blueprint")
        log_info("  2. In Class Defaults, set 'HUD Widget Class' to WBP_SpaceShipHUD")
        log_info("  3. Compile and Save")
        
        return True
        
    except Exception as e:
        log_error(f"Failed to configure BP_PlayerController: {e}")
        return False

def verify_test_level():
    """Verify TestLevel has required actors"""
    log_info("Verifying TestLevel...")
    
    try:
        # Load the level using subsystem (EditorLevelLibrary is deprecated)
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        success = level_subsystem.load_level(asset_path="/Game/Maps/TestLevel")
        
        if not success:
            log_error(f"Failed to load /Game/Maps/TestLevel")
            return False
        
        # Get all actors using subsystem
        actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        all_actors = actor_subsystem.get_all_level_actors()
        
        has_light = False
        has_player_start = False
        
        for actor in all_actors:
            actor_class = actor.get_class().get_name()
            if "Light" in actor_class:
                has_light = True
            if "PlayerStart" in actor_class:
                has_player_start = True
        
        if has_light:
            log_success("Lighting found in level")
        else:
            log_error("No lighting found - add Directional Light")
        
        if has_player_start:
            log_success("Player Start found in level")
        else:
            log_error("No Player Start found - add Player Start actor")
        
        return has_light and has_player_start
        
    except Exception as e:
        log_error(f"Failed to verify TestLevel: {e}")
        return False

def main():
    """Main configuration function"""
    log_info("=" * 70)
    log_info("PIE Auto-Configuration for Adastrea")
    log_info("=" * 70)
    log_info("")
    
    success_count = 0
    total_steps = 3
    
    # Step 1: Configure Game Mode
    if configure_game_mode():
        success_count += 1
    log_info("")
    
    # Step 2: Configure Player Controller
    if configure_player_controller():
        success_count += 1
    log_info("")
    
    # Step 3: Verify Test Level
    if verify_test_level():
        success_count += 1
    log_info("")
    
    # Summary
    log_info("=" * 70)
    if success_count == total_steps:
        log_success(f"Auto-configuration completed! ({success_count}/{total_steps} steps)")
        log_info("")
        log_info("NEXT STEPS:")
        log_info("1. Open BP_PlayerShip Blueprint")
        log_info("2. Add components:")
        log_info("   - Static Mesh Component (visible mesh)")
        log_info("   - Camera Component (Location: X=-500, Y=0, Z=200)")
        log_info("   - SpaceshipControlsComponent")
        log_info("3. Set 'Auto Possess Player' to 'Player 0'")
        log_info("4. Compile and Save")
        log_info("5. Press Alt+P to test PIE!")
    else:
        log_error(f"Configuration completed with issues ({success_count}/{total_steps} steps succeeded)")
        log_info("Check errors above and configure manually as needed")
    
    log_info("=" * 70)
    log_info("")
    log_info("For manual setup: See PIE_FIX_CHECKLIST.md")
    log_info("For troubleshooting: See QUICK_PIE_FIX.md")

# Execute configuration
if __name__ == "__main__":
    main()
