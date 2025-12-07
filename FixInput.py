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

fix_input = """
import unreal

# Fix: Enable Auto Receive Input for BP_Import
try:
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    # Set Auto Receive Input to Player 0 (value 1)
    import_default.set_editor_property("auto_receive_input", 1)
    
    unreal.EditorAssetLibrary.save_loaded_asset(import_bp)
    unreal.log("✓ BP_Import: Auto Receive Input = Player 0")
except Exception as e:
    unreal.log_error(f"✗ BP_Import input fix failed: {e}")
"""

print("Enabling Auto Receive Input for BP_Import...")
result = send_python_command(fix_input)

if result and result.get("status") == "success":
    print("✅ Input enabled! Try Alt+P now - you should have controls!")
else:
    print(f"❌ Failed: {result}")
