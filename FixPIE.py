#!/usr/bin/env python3
"""
Fix PIE Setup Issues
====================

This script fixes all the issues found by DiagnosePIE.py
"""

import unreal

def log_success(message):
    unreal.log(f"✓ {message}")

def log_error(message):
    unreal.log_error(f"✗ {message}")

def log_info(message):
    unreal.log(f"→ {message}")

def fix_game_mode():
    """Fix BP_TestGameMode - set correct Default Pawn Class"""
    log_info("Fixing BP_TestGameMode...")
    
    try:
        # Load assets
        game_mode_bp = unreal.load_asset("/Game/Blueprints/BP_TestGameMode")
        player_ship_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C")
        player_controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
        
        if not player_ship_class:
            log_error("Could not load BP_PlayerShip class")
            return False
        
        # Get game mode class and default object
        game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
        gm_default = unreal.get_default_object(game_mode_class)
        
        # Fix: Set correct Default Pawn Class to BP_PlayerShip
        gm_default.set_editor_property("default_pawn_class", player_ship_class)
        gm_default.set_editor_property("player_controller_class", player_controller_class)
        
        # Save
        unreal.EditorAssetLibrary.save_loaded_asset(game_mode_bp)
        
        log_success("BP_TestGameMode: Default Pawn Class set to BP_PlayerShip")
        log_success("BP_TestGameMode: Player Controller Class set to BP_PlayerController")
        return True
        
    except Exception as e:
        log_error(f"Failed to fix BP_TestGameMode: {e}")
        return False

def fix_player_controller():
    """Fix BP_PlayerController - disable mouse cursor"""
    log_info("Fixing BP_PlayerController...")
    
    try:
        controller_bp = unreal.load_asset("/Game/Blueprints/Controllers/BP_PlayerController")
        controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
        default_obj = unreal.get_default_object(controller_class)
        
        # Try different property names (Unreal property naming can vary)
        try:
            default_obj.set_editor_property("bshow_mouse_cursor", False)
            log_success("BP_PlayerController: Mouse cursor disabled")
        except:
            try:
                default_obj.set_editor_property("bShowMouseCursor", False)
                log_success("BP_PlayerController: Mouse cursor disabled (alternate property)")
            except:
                log_info("BP_PlayerController: Mouse cursor property not accessible via Python")
                log_info("  Please set manually: Class Defaults → Show Mouse Cursor = False")
        
        # Save
        unreal.EditorAssetLibrary.save_loaded_asset(controller_bp)
        return True
        
    except Exception as e:
        log_error(f"Failed to fix BP_PlayerController: {e}")
        return False

def fix_player_ship():
    """Fix BP_PlayerShip - set Auto Possess Player"""
    log_info("Fixing BP_PlayerShip...")
    
    try:
        ship_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_PlayerShip")
        ship_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C")
        default_obj = unreal.get_default_object(ship_class)
        
        # Fix: Set Auto Possess Player to Player 0
        # The property is auto_possess_player, value is AutoPossessPlayer.PLAYER0
        default_obj.set_editor_property("auto_possess_player", unreal.AutoPossessPlayer.PLAYER0)
        
        # Save
        unreal.EditorAssetLibrary.save_loaded_asset(ship_bp)
        
        log_success("BP_PlayerShip: Auto Possess Player set to Player 0")
        log_info("BP_PlayerShip: Components must be added manually:")
        log_info("  1. Open BP_PlayerShip in editor")
        log_info("  2. Add Camera Component (Location: X=-500, Y=0, Z=200)")
        log_info("  3. Add Static Mesh Component (any visible mesh)")
        log_info("  4. Compile and Save")
        
        return True
        
    except Exception as e:
        log_error(f"Failed to fix BP_PlayerShip: {e}")
        return False

def fix_project_settings():
    """Fix Project Settings - set Default GameMode"""
    log_info("Fixing Project Settings...")
    
    try:
        # Get project settings
        project_settings = unreal.get_default_object(unreal.GameMapsSettings)
        
        # Load BP_TestGameMode class
        game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
        
        if not game_mode_class:
            log_error("Could not load BP_TestGameMode class")
            return False
        
        # Set global default game mode
        project_settings.set_editor_property("global_default_game_mode", game_mode_class)
        
        log_success("Project Settings: Default GameMode set to BP_TestGameMode")
        return True
        
    except Exception as e:
        log_error(f"Failed to fix Project Settings: {e}")
        return False

def main():
    """Run all fixes"""
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  FIXING PIE SETUP ISSUES")
    unreal.log("=" * 60)
    unreal.log("")
    
    results = []
    
    results.append(("Project Settings", fix_project_settings()))
    results.append(("BP_TestGameMode", fix_game_mode()))
    results.append(("BP_PlayerController", fix_player_controller()))
    results.append(("BP_PlayerShip", fix_player_ship()))
    
    # Summary
    unreal.log("")
    unreal.log("=" * 60)
    unreal.log("  SUMMARY")
    unreal.log("=" * 60)
    
    for name, result in results:
        status = "✓" if result else "✗"
        unreal.log(f"  {status} {name}")
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    unreal.log("")
    unreal.log(f"  Fixed: {passed}/{total} issues")
    unreal.log("")
    
    if passed == total:
        unreal.log("  🎉 All automatic fixes applied!")
    else:
        unreal.log("  ⚠️  Some fixes require manual intervention (see above)")
    
    unreal.log("")
    unreal.log("  MANUAL STEPS REQUIRED:")
    unreal.log("  1. Open BP_PlayerController Blueprint")
    unreal.log("     → Class Defaults → Show Mouse Cursor = False (if not already)")
    unreal.log("     → Class Defaults → HUD Widget Class = WBP_SpaceShipHUD")
    unreal.log("")
    unreal.log("  2. Open BP_PlayerShip Blueprint")
    unreal.log("     → Add Camera Component (Location: X=-500, Y=0, Z=200)")
    unreal.log("     → Add Static Mesh Component (any visible mesh for testing)")
    unreal.log("     → Compile and Save")
    unreal.log("")
    unreal.log("  3. Press Alt+P to test PIE!")
    unreal.log("")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()
