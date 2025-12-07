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

final_fix = """
import unreal

unreal.log("=== FINAL PIE FIX ===")

# Step 1: Fix BP_Import Blueprint class defaults
try:
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    # Set Auto Possess and Auto Receive Input using numeric values
    import_default.set_editor_property("auto_possess_player", 1)  # PLAYER0 = 1
    import_default.set_editor_property("auto_receive_input", 1)   # PLAYER0 = 1
    
    unreal.EditorAssetLibrary.save_loaded_asset(import_bp)
    unreal.log("✓ BP_Import class: Auto Possess = Player 0")
    unreal.log("✓ BP_Import class: Auto Receive Input = Player 0")
    
except Exception as e:
    unreal.log_error(f"BP_Import class fix failed: {e}")

# Step 2: Check if ship exists in level, if not spawn one
try:
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    all_actors = actor_subsystem.get_all_level_actors()
    
    # Look for BP_Import (be more flexible in search)
    import_ships = [a for a in all_actors if "BP_Import" in a.get_name() or "Import" in str(a.get_class())]
    
    unreal.log(f"Found {len(import_ships)} BP_Import-like actors in level")
    
    if not import_ships:
        unreal.log("No BP_Import found - spawning new one...")
        
        import_class = unreal.load_object(None, "/Game/Blueprints/Ships/BP_Import.BP_Import")
        location = unreal.Vector(0, 0, 200)
        rotation = unreal.Rotator(0, 0, 0)
        
        spawned = actor_subsystem.spawn_actor_from_object(import_class, location, rotation)
        
        if spawned:
            unreal.log(f"✓ Spawned BP_Import: {spawned.get_name()}")
            unreal.EditorLevelLibrary.save_current_level()
            unreal.log("✓ Level saved")
    else:
        unreal.log(f"✓ BP_Import already in level: {import_ships[0].get_name()}")
        
except Exception as e:
    unreal.log_error(f"Level check/spawn failed: {e}")

unreal.log("=== FIX COMPLETE - Try Alt+P ===")
"""

print("Applying final PIE fixes...")
result = send_python_command(final_fix)

if result:
    print("✅ Complete! Try Alt+P now")
    print("You should possess the ship and have WASD controls")
else:
    print("❌ Failed")
