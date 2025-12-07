#!/usr/bin/env python3
"""
PIE Setup Diagnostic Script
============================

This script checks all PIE setup requirements and reports what's missing or misconfigured.
Execute in Unreal Editor's Python console.
"""

import unreal

def print_header(title):
    """Print a section header"""
    unreal.log("=" * 60)
    unreal.log(f"  {title}")
    unreal.log("=" * 60)

def print_result(check_name, passed, details=""):
    """Print check result"""
    status = "✓ PASS" if passed else "✗ FAIL"
    unreal.log(f"{status}: {check_name}")
    if details:
        unreal.log(f"        {details}")

def check_project_settings():
    """Check project-level settings"""
    print_header("1. PROJECT SETTINGS")
    
    try:
        # Get project settings
        project_settings = unreal.get_default_object(unreal.GameMapsSettings)
        
        # Check default game mode
        default_game_mode = project_settings.get_editor_property("global_default_game_mode")
        game_mode_name = default_game_mode.get_name() if default_game_mode else "None"
        
        is_correct = "BP_TestGameMode" in game_mode_name
        print_result("Default GameMode", is_correct, f"Current: {game_mode_name}")
        
        if not is_correct:
            unreal.log("        → Should be: BP_TestGameMode_C")
            
        return is_correct
    except Exception as e:
        print_result("Project Settings", False, f"Error: {e}")
        return False

def check_game_mode():
    """Check BP_TestGameMode configuration"""
    print_header("2. BP_TESTGAMEMODE")
    
    all_passed = True
    
    try:
        # Load game mode
        game_mode_bp = unreal.load_asset("/Game/Blueprints/BP_TestGameMode")
        if not game_mode_bp:
            print_result("Blueprint exists", False, "BP_TestGameMode not found")
            return False
        
        print_result("Blueprint exists", True, "Found at /Game/Blueprints/BP_TestGameMode")
        
        # Get generated class
        game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
        default_obj = unreal.get_default_object(game_mode_class)
        
        # Check default pawn class
        default_pawn = default_obj.get_editor_property("default_pawn_class")
        pawn_name = default_pawn.get_name() if default_pawn else "None"
        pawn_correct = "BP_PlayerShip" in pawn_name
        print_result("Default Pawn Class", pawn_correct, f"Current: {pawn_name}")
        all_passed = all_passed and pawn_correct
        
        # Check player controller class
        controller_class = default_obj.get_editor_property("player_controller_class")
        controller_name = controller_class.get_name() if controller_class else "None"
        controller_correct = "BP_PlayerController" in controller_name
        print_result("Player Controller Class", controller_correct, f"Current: {controller_name}")
        all_passed = all_passed and controller_correct
        
        return all_passed
        
    except Exception as e:
        print_result("BP_TestGameMode", False, f"Error: {e}")
        return False

def check_player_controller():
    """Check BP_PlayerController configuration"""
    print_header("3. BP_PLAYERCONTROLLER")
    
    all_passed = True
    
    try:
        # Load controller
        controller_bp = unreal.load_asset("/Game/Blueprints/Controllers/BP_PlayerController")
        if not controller_bp:
            print_result("Blueprint exists", False, "BP_PlayerController not found")
            return False
        
        print_result("Blueprint exists", True, "Found at /Game/Blueprints/Controllers/BP_PlayerController")
        
        # Get generated class
        controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
        default_obj = unreal.get_default_object(controller_class)
        
        # Check mouse cursor
        show_mouse = default_obj.get_editor_property("bshow_mouse_cursor")
        mouse_correct = not show_mouse
        print_result("Mouse Cursor Disabled", mouse_correct, f"Show Mouse Cursor: {show_mouse}")
        all_passed = all_passed and mouse_correct
        
        # Check HUD widget class
        try:
            hud_widget = default_obj.get_editor_property("hud_widget_class")
            hud_name = hud_widget.get_name() if hud_widget else "None"
            hud_correct = "WBP_SpaceShipHUD" in hud_name or hud_name != "None"
            print_result("HUD Widget Class", hud_correct, f"Current: {hud_name}")
            all_passed = all_passed and hud_correct
        except:
            print_result("HUD Widget Class", False, "Property not accessible (may need manual check)")
            unreal.log("        → C++ property may not be visible in Python")
        
        return all_passed
        
    except Exception as e:
        print_result("BP_PlayerController", False, f"Error: {e}")
        return False

def check_player_ship():
    """Check BP_PlayerShip configuration"""
    print_header("4. BP_PLAYERSHIP")
    
    all_passed = True
    
    try:
        # Load ship
        ship_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_PlayerShip")
        if not ship_bp:
            print_result("Blueprint exists", False, "BP_PlayerShip not found")
            return False
        
        print_result("Blueprint exists", True, "Found at /Game/Blueprints/Ships/BP_PlayerShip")
        
        # Get generated class
        ship_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_PlayerShip.BP_PlayerShip_C")
        default_obj = unreal.get_default_object(ship_class)
        
        # Check auto possess
        auto_possess = default_obj.get_editor_property("auto_possess_player")
        possess_correct = str(auto_possess) == "AutoPossessPlayer.PLAYER0"
        print_result("Auto Possess Player", possess_correct, f"Current: {auto_possess}")
        all_passed = all_passed and possess_correct
        
        # Try to check components (this may not work fully in Python)
        unreal.log("")
        unreal.log("    Component Check (manual verification recommended):")
        
        # Get Blueprint asset
        bp_asset = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Blueprints/Ships/BP_PlayerShip")
        if bp_asset:
            try:
                # This is a basic check - full component inspection may require manual verification
                unreal.log("    → Blueprint loaded successfully")
                unreal.log("    → Please manually verify in editor:")
                unreal.log("      • Camera Component exists")
                unreal.log("      • Static/Skeletal Mesh Component exists (for visibility)")
                unreal.log("      • FloatingPawnMovement or similar movement component")
            except:
                pass
        
        return all_passed
        
    except Exception as e:
        print_result("BP_PlayerShip", False, f"Error: {e}")
        return False

def check_input_system():
    """Check Enhanced Input configuration"""
    print_header("5. INPUT SYSTEM")
    
    try:
        # Check if IMC_Spaceship exists
        imc = unreal.load_asset("/Game/Input/IMC_Spaceship")
        imc_exists = imc is not None
        print_result("IMC_Spaceship exists", imc_exists, 
                    "Found at /Game/Input/IMC_Spaceship" if imc_exists else "Not found")
        
        # Note: Checking project settings input is complex, mention C++ auto-load
        unreal.log("")
        unreal.log("    Note: AdastreaPlayerController.cpp auto-loads IMC_Spaceship")
        unreal.log("    This happens in C++ BeginPlay(), no Blueprint setup needed")
        
        return imc_exists
        
    except Exception as e:
        print_result("Input System", False, f"Error: {e}")
        return False

def check_test_level():
    """Check TestLevel setup"""
    print_header("6. TEST LEVEL")
    
    all_passed = True
    
    try:
        # Get current level
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        current_level = level_subsystem.get_current_level()
        
        if not current_level:
            print_result("Level loaded", False, "No level currently open")
            return False
        
        level_name = current_level.get_outer().get_name()
        print_result("Level loaded", True, f"Current level: {level_name}")
        
        # Get all actors
        actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        all_actors = actor_subsystem.get_all_level_actors()
        
        # Check for Player Start
        has_player_start = any("PlayerStart" in actor.get_class().get_name() for actor in all_actors)
        print_result("Player Start exists", has_player_start, 
                    "Found" if has_player_start else "Missing - add PlayerStart actor")
        all_passed = all_passed and has_player_start
        
        # Check for lighting
        has_light = any("Light" in actor.get_class().get_name() for actor in all_actors)
        print_result("Lighting exists", has_light,
                    "Found" if has_light else "Missing - add DirectionalLight or PointLight")
        all_passed = all_passed and has_light
        
        return all_passed
        
    except Exception as e:
        print_result("Test Level", False, f"Error: {e}")
        return False

def main():
    """Run all diagnostic checks"""
    unreal.log("")
    unreal.log("╔" + "=" * 58 + "╗")
    unreal.log("║" + " " * 15 + "PIE SETUP DIAGNOSTIC" + " " * 23 + "║")
    unreal.log("╚" + "=" * 58 + "╝")
    unreal.log("")
    
    results = []
    
    results.append(("Project Settings", check_project_settings()))
    results.append(("BP_TestGameMode", check_game_mode()))
    results.append(("BP_PlayerController", check_player_controller()))
    results.append(("BP_PlayerShip", check_player_ship()))
    results.append(("Input System", check_input_system()))
    results.append(("Test Level", check_test_level()))
    
    # Summary
    print_header("SUMMARY")
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for name, result in results:
        status = "✓" if result else "✗"
        unreal.log(f"  {status} {name}")
    
    unreal.log("")
    unreal.log(f"  Result: {passed}/{total} checks passed")
    unreal.log("")
    
    if passed == total:
        unreal.log("  🎉 All checks passed! You should be able to run PIE.")
        unreal.log("  Press Alt+P to test Play In Editor")
    else:
        unreal.log("  ⚠️  Some issues found. Fix the failed checks above.")
        unreal.log("  Run ConfigurePIE_UnrealMCP.py to auto-fix some issues.")
    
    unreal.log("")
    unreal.log("=" * 60)

# Run diagnostic
if __name__ == "__main__":
    main()
