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

# Verify component setup
verify_code = """
import unreal

def log_info(message):
    unreal.log(f"[VERIFY] {message}")

log_info("=" * 80)
log_info("Verifying BP_Import Setup")
log_info("=" * 80)

try:
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    log_info("\\nSettings:")
    log_info(f"  ✓ Auto Possess: {import_default.get_editor_property('auto_possess_player')}")
    log_info(f"  ✓ Auto Receive Input: {import_default.get_editor_property('auto_receive_input')}")
    
    log_info("\\nNOTE: Component check requires Blueprint to be saved after adding them")
    log_info("After adding components, run this script again to verify")
    log_info("=" * 80)
    
except Exception as e:
    log_info(f"Verification failed: {e}")
"""

print("Verifying BP_Import configuration...")
result = send_python_command(verify_code)

if result and result.get("status") == "success":
    print("✅ Verification complete - check Output Log")
else:
    print(f"❌ Failed: {result}")
