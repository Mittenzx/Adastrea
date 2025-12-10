"""
Create BP_ImportAI - An AI-controlled ship that flies to random coordinates

This script creates:
1. BP_SimpleShipAI - A simple AI controller for autonomous ship movement
2. BP_ImportAI - A Blueprint based on BP_Import with AI control
3. Adds SimpleAIMovementComponent for autonomous navigation

Usage:
    Execute this script in Unreal Editor via Python console or MCP
    Note: Requires SimpleAIMovementComponent C++ class to be compiled first
"""

import unreal


def create_ai_ship():
    """Create BP_ImportAI with autonomous random movement"""
    
    unreal.log("=== Creating AI-Controlled Ship ===")
    
    editor_asset_lib = unreal.EditorAssetLibrary()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    try:
        # Step 1: Create AI Controller Blueprint
        unreal.log("\n--- Step 1: Creating AI Controller ---")
        
        ai_controller_path = "/Game/Blueprints/AI"
        ai_controller_name = "BP_SimpleShipAI"
        ai_controller_full_path = f"{ai_controller_path}/{ai_controller_name}"
        
        # Create AI directory if it doesn't exist
        if not editor_asset_lib.does_directory_exist(ai_controller_path):
            editor_asset_lib.make_directory(ai_controller_path)
            unreal.log(f"✓ Created directory: {ai_controller_path}")
        
        # Use default AIController - no need to create custom one for this simple case
        ai_controller_class = unreal.AIController
        unreal.log(f"✓ Using default AIController class")
        
        # Step 2: Load BP_Import as template
        unreal.log("\n--- Step 2: Loading BP_Import template ---")
        
        import_blueprint_path = "/Game/Blueprints/Ships/BP_Import"
        import_blueprint = unreal.load_object(None, f"{import_blueprint_path}.BP_Import_C")
        
        if not import_blueprint:
            unreal.log_error(f"✗ Could not load BP_Import from {import_blueprint_path}")
            return False
        
        unreal.log(f"✓ Loaded BP_Import template")
        
        # Step 3: Create BP_ImportAI Blueprint
        unreal.log("\n--- Step 3: Creating BP_ImportAI ---")
        
        ship_path = "/Game/Blueprints/Ships"
        ship_name = "BP_ImportAI"
        ship_full_path = f"{ship_path}/{ship_name}"
        
        # Check if BP_ImportAI already exists, delete it to recreate
        if editor_asset_lib.does_asset_exist(ship_full_path):
            unreal.log(f"⚠ BP_ImportAI already exists, will reconfigure it")
            bp_import_ai = unreal.load_object(None, ship_full_path)
        else:
            # Duplicate BP_Import to create BP_ImportAI
            success = editor_asset_lib.duplicate_asset(import_blueprint_path, ship_full_path)
            
            if not success:
                unreal.log_error(f"✗ Failed to duplicate BP_Import to {ship_full_path}")
                return False
            
            unreal.log(f"✓ Created BP_ImportAI by duplicating BP_Import")
            bp_import_ai = unreal.load_object(None, ship_full_path)
        
        # Step 4: Configure BP_ImportAI
        unreal.log("\n--- Step 4: Configuring BP_ImportAI ---")
        
        # Load the Blueprint for editing
        generated_class = unreal.load_object(None, f"{ship_full_path}.{ship_name}_C")
        
        if not generated_class:
            unreal.log_error("✗ Could not load BP_ImportAI class")
            return False
        
        # Get the Class Default Object (CDO)
        cdo = unreal.get_default_object(generated_class)
        
        if cdo:
            # Set Auto Possess AI to "Placed in World or Spawned"
            cdo.set_editor_property('auto_possess_ai', unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)
            cdo.set_editor_property('ai_controller_class', ai_controller_class)
            unreal.log(f"✓ Set AI Controller Class to AIController")
            unreal.log(f"✓ Set Auto Possess AI to 'Placed in World or Spawned'")
        
        # Step 5: Add SimpleAIMovementComponent
        unreal.log("\n--- Step 5: Adding SimpleAIMovementComponent ---")
        
        # Note: Adding components to Blueprints via Python is complex
        # The component needs to be added in the Blueprint editor
        unreal.log("⚠ Component must be added manually in Blueprint Editor:")
        unreal.log("  1. Open BP_ImportAI")
        unreal.log("  2. Click 'Add Component' → Search 'SimpleAIMovement'")
        unreal.log("  3. Add 'SimpleAIMovementComponent'")
        unreal.log("  4. Configure component properties as needed")
        unreal.log("  5. Compile and Save")
        
        # Save the asset
        editor_asset_lib.save_asset(ship_full_path)
        unreal.log(f"✓ Saved BP_ImportAI configuration")
        
        # Step 6: Summary
        unreal.log("\n--- Step 6: Summary ---")
        unreal.log("\n=== SUMMARY ===")
        unreal.log(f"✓ Created: {ship_full_path}")
        unreal.log(f"✓ AI Controller configured (default AIController)")
        unreal.log(f"✓ Auto Possess AI set to 'Placed in World or Spawned'")
        unreal.log(f"")
        unreal.log(f"⚠ REQUIRED MANUAL STEPS:")
        unreal.log(f"")
        unreal.log(f"1. Compile C++ code:")
        unreal.log(f"   - SimpleAIMovementComponent.h")
        unreal.log(f"   - SimpleAIMovementComponent.cpp")
        unreal.log(f"")
        unreal.log(f"2. Open BP_ImportAI in Blueprint Editor:")
        unreal.log(f"   - Click 'Add Component'")
        unreal.log(f"   - Search for 'SimpleAIMovementComponent'")
        unreal.log(f"   - Add the component")
        unreal.log(f"   - Configure properties:")
        unreal.log(f"     • MinDistance: 10000.0")
        unreal.log(f"     • MaxDistance: 50000.0")
        unreal.log(f"     • MoveSpeed: 2000.0")
        unreal.log(f"     • ArrivalThreshold: 500.0")
        unreal.log(f"     • TurnRate: 90.0")
        unreal.log(f"     • bShowDebug: true (for testing)")
        unreal.log(f"   - Compile and Save")
        unreal.log(f"")
        unreal.log(f"3. Test in Editor:")
        unreal.log(f"   - Drag BP_ImportAI into any level")
        unreal.log(f"   - Press Alt+P to Play")
        unreal.log(f"   - Ship should fly autonomously!")
        unreal.log(f"")
        unreal.log(f"✅ Blueprint base created successfully!")
        
        return True
        
    except Exception as e:
        unreal.log_error(f"✗ Error creating AI ship: {e}")
        import traceback
        unreal.log_error(traceback.format_exc())
        return False


if __name__ == "__main__":
    create_ai_ship()
