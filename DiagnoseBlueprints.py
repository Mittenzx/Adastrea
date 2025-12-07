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

diagnostic_code = """
import unreal

unreal.log("=== BLUEPRINT DIAGNOSTICS ===")

# Check BP_TestGameMode
try:
    gm_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
    gm_default = unreal.get_default_object(gm_class)
    
    default_pawn = gm_default.get_editor_property("default_pawn_class")
    player_controller = gm_default.get_editor_property("player_controller_class")
    
    unreal.log(f"BP_TestGameMode:")
    unreal.log(f"  - Default Pawn Class: {default_pawn}")
    unreal.log(f"  - Player Controller Class: {player_controller}")
except Exception as e:
    unreal.log_error(f"BP_TestGameMode check failed: {e}")

# Check BP_Import
try:
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    auto_possess = import_default.get_editor_property("auto_possess_player")
    auto_receive = import_default.get_editor_property("auto_receive_input")
    
    unreal.log(f"BP_Import:")
    unreal.log(f"  - Auto Possess Player: {auto_possess}")
    unreal.log(f"  - Auto Receive Input: {auto_receive}")
    
    # Check for components
    components = import_default.get_editor_property("blueprint_created_components")
    unreal.log(f"  - Components: {len(components) if components else 0}")
    if components:
        for comp in components:
            unreal.log(f"    - {comp.get_name()}: {comp.get_class().get_name()}")
except Exception as e:
    unreal.log_error(f"BP_Import check failed: {e}")

# Check BP_PlayerController
try:
    pc_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
    pc_default = unreal.get_default_object(pc_class)
    
    show_mouse = pc_default.get_editor_property("bShowMouseCursor")
    
    unreal.log(f"BP_PlayerController:")
    unreal.log(f"  - Show Mouse Cursor: {show_mouse}")
except Exception as e:
    unreal.log_error(f"BP_PlayerController check failed: {e}")

# Check TestLevel
try:
    level_path = "/Game/Maps/TestLevel"
    world = unreal.EditorLevelLibrary.get_editor_world()
    
    # Get all actors in level
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    
    player_starts = [a for a in all_actors if a.get_class().get_name() == "PlayerStart"]
    import_ships = [a for a in all_actors if "BP_Import" in a.get_class().get_name()]
    
    unreal.log(f"TestLevel:")
    unreal.log(f"  - Total Actors: {len(all_actors)}")
    unreal.log(f"  - Player Starts: {len(player_starts)}")
    unreal.log(f"  - BP_Import instances: {len(import_ships)}")
    
    if import_ships:
        for ship in import_ships:
            location = ship.get_actor_location()
            unreal.log(f"    - {ship.get_name()} at {location}")
except Exception as e:
    unreal.log_error(f"TestLevel check failed: {e}")

unreal.log("=== DIAGNOSTICS COMPLETE ===")
"""

print("Running Blueprint diagnostics...")
result = send_python_command(diagnostic_code)

if result and result.get("status") == "success":
    print("✅ Diagnostic complete - check Unreal Output Log")
else:
    print(f"❌ Failed: {result}")
