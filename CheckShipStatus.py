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

check_and_fix = """
import unreal

unreal.log("=== CHECKING BP_IMPORT IN LEVEL ===")

# Get the BP_Import actor instance
try:
    all_actors = unreal.EditorActorSubsystem().get_all_level_actors()
    import_ships = [a for a in all_actors if "BP_Import" in str(type(a))]
    
    if import_ships:
        ship = import_ships[0]
        unreal.log(f"Found: {ship.get_name()}")
        unreal.log(f"  Type: {type(ship)}")
        unreal.log(f"  Class: {ship.get_class().get_name()}")
        
        # Check current settings
        try:
            auto_poss = ship.get_editor_property("auto_possess_player")
            unreal.log(f"  Current Auto Possess: {auto_poss}")
        except:
            unreal.log("  Cannot read auto_possess_player from instance")
            
        try:
            auto_input = ship.get_editor_property("auto_receive_input")
            unreal.log(f"  Current Auto Receive Input: {auto_input}")
        except:
            unreal.log("  Cannot read auto_receive_input from instance")
            
        # The issue: Instance properties don't override class defaults in level
        # Solution: Select the actor and manually set it, OR modify the blueprint class
        unreal.log("")
        unreal.log("SOLUTION: The BP_Import Blueprint class needs fixing, not the instance")
        unreal.log("The Auto Possess/Input settings we changed should work")
        unreal.log("BUT: There might be a camera issue - let me check...")
        
    else:
        unreal.log("ERROR: No BP_Import found in level!")
        
except Exception as e:
    unreal.log_error(f"Check failed: {e}")
    import traceback
    unreal.log_error(traceback.format_exc())
"""

print("Checking BP_Import status...")
result = send_python_command(check_and_fix)

if result:
    print("✅ Check complete - see Output Log")
else:
    print("❌ Failed")
