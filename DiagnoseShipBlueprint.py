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

# Comprehensive ship Blueprint diagnostics
diagnose_code = """
import unreal

def log_info(message):
    unreal.log(f"[SHIP DIAGNOSTIC] {message}")

def log_error(message):
    unreal.log_error(f"[SHIP DIAGNOSTIC ERROR] {message}")

log_info("=" * 80)
log_info("BP_Import Ship Blueprint Diagnostic")
log_info("=" * 80)

try:
    # Load BP_Import
    import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
    import_default = unreal.get_default_object(import_class)
    
    log_info("\\n1. CLASS DEFAULTS CHECK:")
    log_info(f"   Auto Possess Player: {import_default.get_editor_property('auto_possess_player')}")
    log_info(f"   Auto Receive Input: {import_default.get_editor_property('auto_receive_input')}")
    
    # Check components
    log_info("\\n2. COMPONENT CHECK:")
    try:
        components = import_default.get_editor_property('blueprint_created_components')
        if components:
            log_info(f"   Found {len(components)} components:")
            for comp in components:
                comp_name = comp.get_name()
                comp_class = comp.get_class().get_name()
                log_info(f"      - {comp_name} ({comp_class})")
                
                # Check for Camera component
                if 'Camera' in comp_class:
                    log_info(f"        ✓ Camera component found!")
                
                # Check for Movement component
                if 'Movement' in comp_class or 'Floating' in comp_class:
                    log_info(f"        ✓ Movement component found!")
                    if hasattr(comp, 'max_speed'):
                        log_info(f"          Max Speed: {comp.max_speed}")
        else:
            log_error("   NO COMPONENTS FOUND - Ship will be invisible and immobile!")
    except Exception as e:
        log_error(f"   Failed to check components: {e}")
    
    # Check parent class
    log_info("\\n3. PARENT CLASS CHECK:")
    parent = import_class.get_super_class()
    log_info(f"   Parent class: {parent.get_name()}")
    
    # Check if it's ASpaceship
    if 'Spaceship' in parent.get_name():
        log_info("   ✓ Correctly inherits from ASpaceship")
    else:
        log_error(f"   ✗ Does NOT inherit from ASpaceship (parent: {parent.get_name()})")
    
    # Check for mesh
    log_info("\\n4. MESH CHECK:")
    try:
        root_comp = import_default.get_editor_property('root_component')
        if root_comp:
            log_info(f"   Root Component: {root_comp.get_name()} ({root_comp.get_class().get_name()})")
        else:
            log_error("   NO ROOT COMPONENT!")
        
        # Try to find static mesh component
        mesh_comp = import_default.get_editor_property('static_mesh_component')
        if mesh_comp:
            log_info(f"   Static Mesh Component found: {mesh_comp.get_name()}")
            mesh = mesh_comp.get_editor_property('static_mesh')
            if mesh:
                log_info(f"   ✓ Mesh assigned: {mesh.get_name()}")
            else:
                log_error("   ✗ No mesh assigned to component!")
        else:
            log_info("   No StaticMeshComponent property (might use different component type)")
    except Exception as e:
        log_error(f"   Mesh check failed: {e}")
    
    # Check Blueprint interfaces
    log_info("\\n5. BLUEPRINT INTERFACES:")
    try:
        interfaces = import_class.get_interfaces()
        if interfaces:
            log_info(f"   Implements {len(interfaces)} interfaces:")
            for iface in interfaces:
                log_info(f"      - {iface.get_name()}")
        else:
            log_info("   No interfaces implemented")
    except Exception as e:
        log_error(f"   Interface check failed: {e}")
    
    log_info("\\n6. INPUT SETUP CHECK:")
    log_info("   Note: Blueprint input logic can only be verified by opening Blueprint editor")
    log_info("   Check for these common issues:")
    log_info("      - SetupPlayerInputComponent event implemented?")
    log_info("      - Enhanced Input actions bound correctly?")
    log_info("      - Input actions calling movement functions?")
    
    log_info("\\n" + "=" * 80)
    log_info("DIAGNOSTIC COMPLETE")
    log_info("=" * 80)
    
except Exception as e:
    log_error(f"Diagnostic failed: {e}")
    import traceback
    log_error(traceback.format_exc())
"""

print("Running BP_Import Blueprint diagnostics...")
result = send_python_command(diagnose_code)

if result and result.get("status") == "success":
    print("✅ Diagnostic sent successfully!")
    print("Check Unreal Editor Output Log for detailed results")
else:
    print(f"❌ Failed: {result}")
