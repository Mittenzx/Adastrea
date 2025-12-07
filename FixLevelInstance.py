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

fix_level_instance = """
import unreal

# Fix the BP_Import instance in the level
try:
    # Get the BP_Import actor instance in the level
    all_actors = unreal.EditorActorSubsystem().get_all_level_actors()
    import_ships = [a for a in all_actors if "BP_Import" in a.get_class().get_name()]
    
    if import_ships:
        ship = import_ships[0]
        unreal.log(f"Found BP_Import instance: {ship.get_name()}")
        
        # Set Auto Possess Player on the INSTANCE
        ship.set_actor_property("auto_possess_player", 1)
        
        # Also set Auto Receive Input
        ship.set_actor_property("auto_receive_input", 1)
        
        unreal.log("✓ BP_Import instance: Auto Possess = Player 0")
        unreal.log("✓ BP_Import instance: Auto Receive Input = Player 0")
        
        # Save the level
        unreal.EditorLevelLibrary.save_current_level()
        unreal.log("✓ Level saved")
    else:
        unreal.log_error("✗ No BP_Import found in level")
        
except Exception as e:
    unreal.log_error(f"✗ Level instance fix failed: {e}")
"""

print("Fixing BP_Import instance in level...")
result = send_python_command(fix_level_instance)

if result and result.get("status") == "success":
    print("✅ Level instance fixed! Try Alt+P now")
else:
    print(f"❌ Failed: {result}")
