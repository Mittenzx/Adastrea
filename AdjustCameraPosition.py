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

# Adjust camera position - try different values
adjust_camera_code = """
import unreal

unreal.log("=" * 80)
unreal.log("[CAMERA ADJUST] Adjusting camera position")
unreal.log("=" * 80)

try:
    # Get all BP_Import instances in the level
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    ship_actors = [actor for actor in actors if actor.get_class().get_name() == "BP_Import_C"]
    
    if not ship_actors:
        unreal.log_warning("[CAMERA ADJUST] No BP_Import instances found in level")
        unreal.log("[CAMERA ADJUST] Camera position is set in C++ constructor")
        unreal.log("[CAMERA ADJUST] Current position: X=-2800, Y=0, Z=200")
        unreal.log("[CAMERA ADJUST] To change it, we need to rebuild C++ with new values")
        unreal.log("")
        unreal.log("[CAMERA ADJUST] Recommended positions to try:")
        unreal.log("[CAMERA ADJUST]   Close:  X=-1500, Y=0, Z=250")
        unreal.log("[CAMERA ADJUST]   Medium: X=-3000, Y=0, Z=300")
        unreal.log("[CAMERA ADJUST]   Far:    X=-5000, Y=0, Z=500")
    else:
        for ship in ship_actors:
            unreal.log(f"[CAMERA ADJUST] Found ship: {ship.get_name()}")
            
            # Try to get the camera component
            components = ship.get_components_by_class(unreal.CameraComponent)
            
            if components:
                camera = components[0]
                current_loc = camera.get_relative_location()
                unreal.log(f"[CAMERA ADJUST] Current camera location: X={current_loc.x}, Y={current_loc.y}, Z={current_loc.z}")
                
                # Set new camera position - further back
                new_location = unreal.Vector(-5000.0, 0.0, 500.0)
                camera.set_relative_location(new_location, False, False)
                
                unreal.log(f"[CAMERA ADJUST] ✓ New camera location: X={new_location.x}, Y={new_location.y}, Z={new_location.z}")
                unreal.log("[CAMERA ADJUST] Test PIE now to see the change!")
            else:
                unreal.log_warning("[CAMERA ADJUST] No camera component found on ship")
                
except Exception as e:
    unreal.log_error(f"[CAMERA ADJUST] Error: {e}")

unreal.log("=" * 80)
"""

print("Adjusting camera position to X=-5000, Y=0, Z=500 (far third-person view)...")
result = send_python_command(adjust_camera_code)

if result and result.get("status") == "success":
    print("✅ Camera adjustment sent!")
    print("\nCheck the Unreal Editor Output Log for results")
    print("If it worked, test PIE to see the new camera position")
    print("\nIf you want a different position, I can change the values:")
    print("  - Closer: X=-1500")
    print("  - Medium: X=-3000")
    print("  - Farther: X=-7000")
else:
    print(f"❌ Failed: {result}")
