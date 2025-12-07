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

# Add basic components to BP_Import programmatically
setup_code = """
import unreal

def log_info(message):
    unreal.log(f"[SHIP SETUP] {message}")

def log_error(message):
    unreal.log_error(f"[SHIP SETUP ERROR] {message}")

log_info("=" * 80)
log_info("Setting up BP_Import components")
log_info("=" * 80)

try:
    # Load BP_Import
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    log_info("\\nCurrent configuration:")
    log_info(f"  Parent Class: {import_class.__class__.__name__}")
    log_info(f"  Auto Possess: {import_default.get_editor_property('auto_possess_player')}")
    log_info(f"  Auto Receive Input: {import_default.get_editor_property('auto_receive_input')}")
    
    # Ensure settings are correct
    log_info("\\nEnsuring correct settings...")
    import_default.set_editor_property('auto_possess_player', 1)  # PLAYER0
    import_default.set_editor_property('auto_receive_input', 1)   # PLAYER0
    
    # Save
    unreal.EditorAssetLibrary.save_loaded_asset(import_bp)
    log_info("✓ Settings saved")
    
    log_info("\\n" + "=" * 80)
    log_info("BASIC SETUP COMPLETE")
    log_info("=" * 80)
    log_info("\\nNow you need to manually add components in Blueprint Editor:")
    log_info("  1. Open BP_Import in Blueprint Editor")
    log_info("  2. In Components panel, add:")
    log_info("     - FloatingPawnMovement (for 6DOF movement)")
    log_info("     - Camera (for player view)")
    log_info("     - StaticMesh (optional, for visual representation)")
    log_info("\\nFor Event Graph (if using Enhanced Input from C++):")
    log_info("  - No graph nodes needed! C++ handles it via AdastreaPlayerController")
    log_info("\\nFor Event Graph (if using Blueprint input):")
    log_info("  1. Add 'SetupPlayerInputComponent' event")
    log_info("  2. Bind enhanced input actions to movement functions")
    log_info("=" * 80)
    
except Exception as e:
    log_error(f"Setup failed: {e}")
    import traceback
    log_error(traceback.format_exc())
"""

print("Configuring BP_Import basic settings...")
result = send_python_command(setup_code)

if result and result.get("status") == "success":
    print("✅ Basic settings configured!")
    print("\nCheck Output Log for manual component setup instructions")
else:
    print(f"❌ Failed: {result}")
