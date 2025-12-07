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

# Add components to BP_Import
add_components_code = """
import unreal

unreal.log("=" * 80)
unreal.log("[ADD COMPONENTS] Adding Camera and Movement to BP_Import")
unreal.log("=" * 80)

try:
    # Load BP_Import Blueprint
    bp_asset = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    
    if not bp_asset:
        unreal.log_error("[ADD COMPONENTS] Failed to load BP_Import")
    else:
        # Get the Blueprint's generated class
        bp_class = bp_asset.generated_class()
        
        # Unfortunately, we cannot add components via Python to Blueprint classes
        # We can only modify existing component properties
        
        unreal.log("[ADD COMPONENTS] ⚠ Python API Limitation:")
        unreal.log("[ADD COMPONENTS]   Cannot add new components to Blueprints via Python")
        unreal.log("[ADD COMPONENTS]   You must add components manually in the Blueprint Editor")
        unreal.log("")
        unreal.log("[ADD COMPONENTS] MANUAL STEPS REQUIRED:")
        unreal.log("[ADD COMPONENTS] 1. Open BP_Import in Blueprint Editor")
        unreal.log("[ADD COMPONENTS] 2. In Components panel, click 'Add Component'")
        unreal.log("[ADD COMPONENTS] 3. Add 'Camera' component:")
        unreal.log("[ADD COMPONENTS]    - Search for 'Camera'")
        unreal.log("[ADD COMPONENTS]    - Set Location: X=-300, Y=0, Z=100 (third-person view)")
        unreal.log("[ADD COMPONENTS] 4. Add 'FloatingPawnMovement' component:")
        unreal.log("[ADD COMPONENTS]    - Search for 'FloatingPawnMovement'")
        unreal.log("[ADD COMPONENTS]    - Set Max Speed: 1000.0")
        unreal.log("[ADD COMPONENTS]    - Set Acceleration: 2000.0")
        unreal.log("[ADD COMPONENTS] 5. (Optional) Add 'Static Mesh' component:")
        unreal.log("[ADD COMPONENTS]    - Search for 'Static Mesh'")
        unreal.log("[ADD COMPONENTS]    - Choose a ship mesh")
        unreal.log("[ADD COMPONENTS] 6. Click 'Compile' button")
        unreal.log("[ADD COMPONENTS] 7. Click 'Save' button")
        unreal.log("[ADD COMPONENTS] 8. Test PIE again")
        unreal.log("")
        unreal.log("[ADD COMPONENTS] Alternative: We can configure the C++ ASpaceship class")
        unreal.log("[ADD COMPONENTS] to automatically add these components in the constructor")
        
except Exception as e:
    unreal.log_error(f"[ADD COMPONENTS] Error: {e}")

unreal.log("=" * 80)
"""

print("Checking Blueprint component options...")
result = send_python_command(add_components_code)

if result and result.get("status") == "success":
    print("✅ Check complete!")
    print("\nCheck Unreal Editor Output Log for manual steps")
    print("\nYou have two options:")
    print("  1. Add components manually in BP_Import Blueprint Editor")
    print("  2. I can modify the C++ ASpaceship class to add components automatically")
    print("\nWhich would you prefer?")
else:
    print(f"❌ Failed: {result}")
