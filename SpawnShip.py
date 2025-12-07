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

spawn_ship = """
import unreal

unreal.log("=== SPAWNING BP_IMPORT IN LEVEL ===")

try:
    # Load the BP_Import class
    import_class = unreal.load_object(None, "/Game/Blueprints/Ships/BP_Import.BP_Import")
    
    if not import_class:
        unreal.log_error("Could not load BP_Import blueprint")
    else:
        # Spawn it in the level at a good location
        actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        
        # Spawn at origin with good visibility
        location = unreal.Vector(0, 0, 200)
        rotation = unreal.Rotator(0, 0, 0)
        
        spawned_actor = actor_subsystem.spawn_actor_from_object(
            import_class,
            location,
            rotation
        )
        
        if spawned_actor:
            unreal.log(f"✓ Spawned BP_Import at {location}")
            unreal.log(f"  Actor name: {spawned_actor.get_name()}")
            
            # Set Auto Possess Player on the spawned instance
            spawned_actor.set_editor_property("auto_possess_player", 1)
            spawned_actor.set_editor_property("auto_receive_input", 1)
            
            unreal.log("✓ Set Auto Possess Player = Player 0")
            unreal.log("✓ Set Auto Receive Input = Player 0")
            
            # Save the level
            unreal.EditorLevelLibrary.save_current_level()
            unreal.log("✓ Level saved")
            
            unreal.log("")
            unreal.log("SUCCESS! Try Alt+P now to test PIE")
        else:
            unreal.log_error("Spawn failed - actor is None")
            
except Exception as e:
    unreal.log_error(f"Spawn failed: {e}")
    import traceback
    unreal.log_error(traceback.format_exc())
"""

print("Spawning BP_Import ship in TestLevel...")
result = send_python_command(spawn_ship)

if result:
    print("✅ Ship spawned! Try Alt+P now")
else:
    print("❌ Failed")
