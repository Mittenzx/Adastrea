"""
Check Enhanced Input configuration for spaceship controls
"""
import unreal

def check_input_actions():
    """Check the configuration of Input Actions"""
    print("=== Checking Enhanced Input Configuration ===\n")
    
    # Check IA_Move
    ia_move = unreal.load_asset("/Game/Input/Actions/IA_Move")
    if ia_move:
        print(f"✓ IA_Move found: {ia_move.get_name()}")
        print(f"  Value Type: {ia_move.value_type}")
        print(f"  Expected: EInputActionValueType.AXIS3D for Vector3D")
    else:
        print("✗ IA_Move not found")
    
    print()
    
    # Check IA_Look
    ia_look = unreal.load_asset("/Game/Input/Actions/IA_Look")
    if ia_look:
        print(f"✓ IA_Look found: {ia_look.get_name()}")
        print(f"  Value Type: {ia_look.value_type}")
        print(f"  Expected: EInputActionValueType.AXIS2D for Vector2D")
    else:
        print("✗ IA_Look not found")
    
    print()
    
    # Check IMC_Spaceship
    imc = unreal.load_asset("/Game/Input/IMC_Spaceship")
    if imc:
        print(f"✓ IMC_Spaceship found: {imc.get_name()}")
        mappings = imc.get_mappings()
        print(f"  Number of mappings: {len(mappings)}")
        for i, mapping in enumerate(mappings):
            print(f"  Mapping {i}: {mapping.action.get_name() if mapping.action else 'None'}")
    else:
        print("✗ IMC_Spaceship not found")
    
    print()
    
    # Check BP_Import
    bp_import = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
    if bp_import:
        print(f"✓ BP_Import found")
        cdo = unreal.get_default_object(bp_import)
        if hasattr(cdo, 'move_action'):
            print(f"  Move Action: {cdo.move_action.get_name() if cdo.move_action else 'Not Set'}")
        if hasattr(cdo, 'look_action'):
            print(f"  Look Action: {cdo.look_action.get_name() if cdo.look_action else 'Not Set'}")
    else:
        print("✗ BP_Import not found")

if __name__ == "__main__":
    check_input_actions()
