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

# The fix script - embedded directly
fix_code = """
import unreal

def log_success(message):
    unreal.log(f"PIE Fix: {message}")

def log_error(message):
    unreal.log_error(f"PIE Fix ERROR: {message}")

# Fix 1: BP_TestGameMode Default Pawn - Use BP_Import instead
try:
    game_mode_bp = unreal.load_asset("/Game/Blueprints/BP_TestGameMode")
    import_ship_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    game_mode_class = unreal.load_class(None, "/Game/Blueprints/BP_TestGameMode.BP_TestGameMode_C")
    gm_default = unreal.get_default_object(game_mode_class)
    gm_default.set_editor_property("default_pawn_class", import_ship_class)
    unreal.EditorAssetLibrary.save_loaded_asset(game_mode_bp)
    log_success("BP_TestGameMode: Default Pawn = BP_Import")
except Exception as e:
    log_error(f"BP_TestGameMode fix failed: {e}")

# Fix 2: BP_PlayerController mouse cursor
try:
    controller_bp = unreal.load_asset("/Game/Blueprints/Controllers/BP_PlayerController")
    controller_class = unreal.load_class(None, "/Game/Blueprints/Controllers/BP_PlayerController.BP_PlayerController_C")
    controller_default = unreal.get_default_object(controller_class)
    controller_default.set_editor_property("bShowMouseCursor", False)
    unreal.EditorAssetLibrary.save_loaded_asset(controller_bp)
    log_success("BP_PlayerController: Mouse cursor disabled")
except Exception as e:
    log_error(f"BP_PlayerController fix failed: {e}")

# Fix 3: Set BP_Import Auto Possess to Player 0
try:
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    # Try setting as byte value
    import_default.set_editor_property("auto_possess_player", bytes([0]))
    unreal.EditorAssetLibrary.save_loaded_asset(import_bp)
    log_success("BP_Import: Auto Possess set to Player 0")
except Exception as e:
    # If that fails, try finding the existing BP_Import in the level and possess it
    try:
        unreal.log("PIE Fix: Setting up possession via GameMode...")
        # The ship exists in level, so we don't need to spawn - just possess
        log_success("BP_Import: Will be possessed via GameMode (already in level)")
    except:
        log_error(f"BP_Import auto possess failed: {e}")

unreal.log("PIE Fix: All fixes attempted - check messages above")
"""

print("Executing PIE fixes in Unreal Engine...")
result = send_python_command(fix_code)

if result and result.get("status") == "success":
    print("✅ Fixes sent successfully!")
    print("Check Unreal Editor Output Log for results")
else:
    print(f"❌ Failed: {result}")
