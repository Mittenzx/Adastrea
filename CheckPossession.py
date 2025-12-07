import socket
import json

def send_python_command(code):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(("127.0.0.1", 55557))
        
        command = {
            "type": "execute_python",
            "params": {"code": code}
        }
        
        sock.sendall(json.dumps(command).encode('utf-8'))
        response = sock.recv(8192).decode('utf-8')
        sock.close()
        
        return json.loads(response)
    except Exception as e:
        print(f"Error: {e}")
        return None

# Check possession in PIE
check_code = """
import unreal

def log_info(message):
    unreal.log(f"[POSSESSION CHECK] {message}")

log_info("=" * 80)
log_info("Checking Possession Setup")
log_info("=" * 80)

try:
    # Check if PIE is running
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    is_playing = editor_subsystem.is_editor_playing()
    
    if is_playing:
        log_info("✓ PIE is running - checking possession...")
        
        # Get PIE world
        world = unreal.EditorLevelLibrary.get_game_world()
        if world:
            log_info(f"  World: {world.get_name()}")
            
            # Get player controller
            player_controller = unreal.GameplayStatics.get_player_controller(world, 0)
            if player_controller:
                log_info(f"  ✓ Player Controller found: {player_controller.get_name()}")
                
                # Get possessed pawn
                pawn = player_controller.get_controlled_pawn()
                if pawn:
                    pawn_class = pawn.get_class().get_name()
                    pawn_location = pawn.get_actor_location()
                    log_info(f"  ✓✓✓ POSSESSED PAWN: {pawn.get_name()} (class: {pawn_class})")
                    log_info(f"      Location: {pawn_location}")
                    
                    # Check if it's the ship
                    if 'BP_Import' in pawn_class or 'Spaceship' in pawn_class:
                        log_info("  ✓✓✓ SUCCESS! You are possessed into the ship!")
                    else:
                        log_info(f"  ⚠ WARNING: Possessed pawn is NOT BP_Import (it's {pawn_class})")
                else:
                    log_info("  ✗✗✗ NO PAWN POSSESSED - Player Controller has no pawn!")
                    log_info("  This means possession is NOT working")
            else:
                log_info("  ✗ No Player Controller found")
        else:
            log_info("  ✗ Cannot get game world")
    else:
        log_info("✗ PIE is NOT running - Start PIE first with Alt+P, then run this check")
        log_info("")
        log_info("PRE-PIE CHECK:")
        log_info("Checking BP_Import and BP_TestGameMode configuration...")
        
        # Check BP_Import settings
        import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
        import_default = unreal.get_default_object(import_class)
        auto_possess = import_default.get_editor_property('auto_possess_player')
        
        log_info(f"  BP_Import Auto Possess Player: {auto_possess}")
        if str(auto_possess) == 'AutoReceiveInput.PLAYER0' or auto_possess == 1:
            log_info("    ✓ Correct (PLAYER0)")
        else:
            log_info(f"    ✗ WRONG! Should be PLAYER0, is: {auto_possess}")
        
        # Check GameMode DefaultPawnClass
        game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
        gm_default = unreal.get_default_object(game_mode_class)
        default_pawn = gm_default.get_editor_property('default_pawn_class')
        
        log_info(f"  BP_TestGameMode Default Pawn Class: {default_pawn}")
        if default_pawn and 'BP_Import' in str(default_pawn):
            log_info("    ✓ Correct (BP_Import)")
        else:
            log_info(f"    ✗ WRONG! Should be BP_Import")
        
        # Check if BP_Import exists in level
        log_info("")
        log_info("Checking if BP_Import is in TestLevel...")
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        import_actors = [a for a in all_actors if 'BP_Import' in a.get_class().get_name()]
        
        if import_actors:
            log_info(f"  ✓ Found {len(import_actors)} BP_Import actor(s) in level")
            for actor in import_actors:
                log_info(f"    - {actor.get_name()} at {actor.get_actor_location()}")
        else:
            log_info("  ⚠ No BP_Import actors found in level (will spawn via DefaultPawnClass)")
    
    log_info("=" * 80)
    
except Exception as e:
    log_info(f"Check failed: {e}")
    import traceback
    log_info(traceback.format_exc())
"""

print("Checking possession setup...")
print("If PIE is running, this will show what you're possessed into")
print("If PIE is not running, this will check the configuration")
result = send_python_command(check_code)

if result and result.get("status") == "success":
    print("\n✅ Check complete - see Output Log for results")
    print("\nIf PIE is not running:")
    print("  1. Check the configuration in Output Log")
    print("  2. Start PIE with Alt+P")
    print("  3. Run this script again to verify possession")
else:
    print(f"❌ Failed: {result}")
