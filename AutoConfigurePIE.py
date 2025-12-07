"""
Auto-Configure PIE Setup for Adastrea
======================================

This script automatically configures Blueprints for PIE (Play In Editor) functionality.
It sets up the Game Mode, Player Ship, Player Controller, and HUD widget.

USAGE:
1. Open Unreal Editor
2. Go to: Tools → Execute Python Script
3. Select this file (AutoConfigurePIE.py)
4. Script will configure everything automatically

OR from Unreal Editor's Python console:
execfile(r"c:\Unreal Projects\Adastrea\AutoConfigurePIE.py")
"""

import unreal

def log(message, level="log"):
    """Log messages to Unreal's output log"""
    if level == "warning":
        unreal.log_warning(f"[PIE Setup] {message}")
    elif level == "error":
        unreal.log_error(f"[PIE Setup] {message}")
    else:
        unreal.log(f"[PIE Setup] {message}")

def configure_game_mode():
    """Configure BP_TestGameMode with default pawn and controller"""
    log("Configuring BP_TestGameMode...")
    
    # Load the game mode Blueprint
    game_mode_path = "/Game/Blueprints/BP_TestGameMode"
    game_mode = unreal.load_asset(game_mode_path)
    
    if not game_mode:
        log(f"Failed to load {game_mode_path}", "error")
        return False
    
    # Load the player ship and controller Blueprints
    player_ship_path = "/Game/Blueprints/Ships/BP_PlayerShip"
    player_controller_path = "/Game/Blueprints/Controllers/BP_PlayerController"
    
    player_ship = unreal.load_asset(player_ship_path)
    player_controller = unreal.load_asset(player_controller_path)
    
    if not player_ship:
        log(f"Failed to load {player_ship_path}", "error")
        return False
    
    if not player_controller:
        log(f"Failed to load {player_controller_path}", "error")
        return False
    
    # Get the Blueprint's generated class
    generated_class = unreal.get_default_object(game_mode)
    
    # Set default pawn class
    generated_class.set_editor_property("default_pawn_class", player_ship.generated_class())
    log(f"  ✓ Set Default Pawn Class to BP_PlayerShip")
    
    # Set player controller class
    generated_class.set_editor_property("player_controller_class", player_controller.generated_class())
    log(f"  ✓ Set Player Controller Class to BP_PlayerController")
    
    # Save the Blueprint
    unreal.EditorAssetLibrary.save_loaded_asset(game_mode)
    log(f"  ✓ Saved BP_TestGameMode")
    
    return True

def configure_player_ship():
    """Configure BP_PlayerShip with required components"""
    log("Configuring BP_PlayerShip...")
    
    # Load the player ship Blueprint
    player_ship_path = "/Game/Blueprints/Ships/BP_PlayerShip"
    player_ship_bp = unreal.load_asset(player_ship_path)
    
    if not player_ship_bp:
        log(f"Failed to load {player_ship_path}", "error")
        return False
    
    log("  Note: Component configuration requires Blueprint editor access")
    log("  Please manually verify in Unreal Editor that BP_PlayerShip has:")
    log("    - Static Mesh Component (visible mesh)")
    log("    - Camera Component (X=-500, Y=0, Z=200)")
    log("    - SpaceshipControlsComponent")
    log("    - Auto Possess Player = Player 0")
    
    return True

def configure_player_controller():
    """Configure BP_PlayerController with HUD widget class"""
    log("Configuring BP_PlayerController...")
    
    # Load the player controller Blueprint
    controller_path = "/Game/Blueprints/Controllers/BP_PlayerController"
    controller_bp = unreal.load_asset(controller_path)
    
    if not controller_bp:
        log(f"Failed to load {controller_path}", "error")
        return False
    
    # Load the HUD widget
    hud_widget_path = "/Game/UI/HUD/WBP_SpaceShipHUD"
    hud_widget = unreal.load_asset(hud_widget_path)
    
    if not hud_widget:
        log(f"HUD widget not found at {hud_widget_path}", "warning")
        log("  You'll need to set HUD Widget Class manually in BP_PlayerController")
        return True  # Not a critical failure
    
    # Get the Blueprint's default object
    default_obj = unreal.get_default_object(controller_bp)
    
    # Set HUD widget class
    try:
        default_obj.set_editor_property("hud_widget_class", hud_widget.generated_class())
        log(f"  ✓ Set HUD Widget Class to WBP_SpaceShipHUD")
    except Exception as e:
        log(f"  Could not set HUD Widget Class automatically: {e}", "warning")
        log("  Please set 'HUD Widget Class' manually in BP_PlayerController Class Defaults")
    
    # Set input mode settings
    try:
        default_obj.set_editor_property("show_mouse_cursor", False)
        log(f"  ✓ Disabled mouse cursor")
    except:
        log("  Note: Set 'Show Mouse Cursor' to False manually", "warning")
    
    # Save the Blueprint
    unreal.EditorAssetLibrary.save_loaded_asset(controller_bp)
    log(f"  ✓ Saved BP_PlayerController")
    
    return True

def verify_test_level():
    """Verify TestLevel has required actors"""
    log("Verifying TestLevel...")
    
    level_path = "/Game/Maps/TestLevel"
    
    # Load the level
    success = unreal.EditorLevelLibrary.load_level(level_path)
    
    if not success:
        log(f"Failed to load {level_path}", "error")
        return False
    
    # Check for required actors
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    
    has_light = False
    has_player_start = False
    
    for actor in all_actors:
        actor_name = actor.get_class().get_name()
        if "DirectionalLight" in actor_name or "Light" in actor_name:
            has_light = True
        if "PlayerStart" in actor_name:
            has_player_start = True
    
    if not has_light:
        log("  ⚠ No Directional Light found in level", "warning")
        log("    Add one via: Place Actors → Lights → Directional Light")
    else:
        log("  ✓ Lighting found")
    
    if not has_player_start:
        log("  ⚠ No Player Start found in level", "warning")
        log("    Add one via: Place Actors → Basic → Player Start")
    else:
        log("  ✓ Player Start found")
    
    return True

def main():
    """Main configuration function"""
    log("=" * 60)
    log("Auto-Configure PIE Setup for Adastrea")
    log("=" * 60)
    log("")
    
    success = True
    
    # Step 1: Configure Game Mode
    if not configure_game_mode():
        success = False
    log("")
    
    # Step 2: Configure Player Ship (manual steps required)
    configure_player_ship()
    log("")
    
    # Step 3: Configure Player Controller
    if not configure_player_controller():
        success = False
    log("")
    
    # Step 4: Verify Test Level
    verify_test_level()
    log("")
    
    # Summary
    log("=" * 60)
    if success:
        log("✅ Auto-configuration completed!", "log")
        log("")
        log("NEXT STEPS:")
        log("1. Open BP_PlayerShip in Blueprint editor")
        log("2. Verify it has: Static Mesh, Camera, SpaceshipControlsComponent")
        log("3. Set 'Auto Possess Player' to 'Player 0'")
        log("4. Compile and save")
        log("5. Press Alt+P to test PIE!")
    else:
        log("⚠ Configuration completed with warnings", "warning")
        log("Check the messages above and configure manually as needed")
    
    log("=" * 60)
    log("")
    log("For manual setup instructions, see: PIE_FIX_CHECKLIST.md")
    log("For troubleshooting, see: QUICK_PIE_FIX.md")

# Run the configuration
if __name__ == "__main__":
    main()
