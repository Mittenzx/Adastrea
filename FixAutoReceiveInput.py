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

# Fix Auto Receive Input on BP_Import
fix_code = """
import unreal

def log_success(message):
    unreal.log(f"[INPUT FIX] {message}")

def log_error(message):
    unreal.log_error(f"[INPUT FIX ERROR] {message}")

log_success("=" * 80)
log_success("Fixing BP_Import Auto Receive Input")
log_success("=" * 80)

try:
    # Load BP_Import
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    # Check current value
    current_auto_receive = import_default.get_editor_property('auto_receive_input')
    log_success(f"Current Auto Receive Input: {current_auto_receive}")
    
    # Set to PLAYER0 (value 1)
    import_default.set_editor_property('auto_receive_input', 1)
    
    # Save the Blueprint
    unreal.EditorAssetLibrary.save_loaded_asset(import_bp)
    
    # Verify the change
    new_value = import_default.get_editor_property('auto_receive_input')
    log_success(f"New Auto Receive Input: {new_value}")
    
    if new_value == 1 or str(new_value) == 'AutoReceiveInput.PLAYER0':
        log_success("✓ Auto Receive Input successfully set to PLAYER0!")
        log_success("✓ BP_Import should now receive input in PIE")
    else:
        log_error(f"✗ Auto Receive Input is still: {new_value}")
    
    log_success("=" * 80)
    log_success("Fix complete - Test PIE now with Alt+P")
    log_success("=" * 80)
    
except Exception as e:
    log_error(f"Fix failed: {e}")
    import traceback
    log_error(traceback.format_exc())
"""

print("Fixing Auto Receive Input on BP_Import...")
result = send_python_command(fix_code)

if result and result.get("status") == "success":
    print("✅ Fix sent successfully!")
    print("\n🎮 CONTROLS SHOULD NOW WORK!")
    print("Test PIE again with Alt+P - you should have WASD controls now")
    print("\nCheck Unreal Editor Output Log for confirmation")
else:
    print(f"❌ Failed: {result}")
