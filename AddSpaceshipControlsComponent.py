#!/usr/bin/env python3
"""
Check SpaceshipControlsComponent on BP_Import Blueprint
"""

import unreal

def log_info(message):
    unreal.log(f"[COMPONENT CHECK] {message}")

def log_error(message):
    unreal.log_error(f"[COMPONENT CHECK ERROR] {message}")

def check_spaceship_controls_component():
    """Check if SpaceshipControlsComponent exists on BP_Import"""

    log_info("=" * 80)
    log_info("Checking SpaceshipControlsComponent on BP_Import")
    log_info("=" * 80)

    try:
        # Load BP_Import
        import_bp = unreal.load_asset("/Game/Blueprints/Ships/BP_Import")
        if not import_bp:
            log_error("Could not load BP_Import asset")
            return False

        import_class = unreal.load_class(None, "/Game/Blueprints/Ships/BP_Import.BP_Import_C")
        if not import_class:
            log_error("Could not load BP_Import class")
            return False

        log_info("✓ Loaded BP_Import successfully")

        # Get the default object
        import_default = unreal.get_default_object(import_class)

        # Get all components
        components = import_default.get_components_by_class(unreal.ActorComponent)
        log_info(f"\\nFound {len(components)} components:")

        has_controls_component = False
        for component in components:
            comp_name = component.get_class().get_name()
            comp_display_name = component.get_name()
            log_info(f"  - {comp_name}: {comp_display_name}")

            if comp_name == "SpaceshipControlsComponent":
                has_controls_component = True

        log_info("\\n" + "=" * 80)
        if has_controls_component:
            log_info("✅ SPACESHIPCONTROLSCOMPONENT FOUND!")
            log_info("Your BP_Import should work correctly now.")
        else:
            log_info("❌ SPACESHIPCONTROLSCOMPONENT MISSING!")
            log_info("\\nMANUAL STEPS REQUIRED:")
            log_info("1. Open BP_Import in Blueprint Editor")
            log_info("2. In Components panel, click '+ Add' button")
            log_info("3. Search for 'SpaceshipControlsComponent'")
            log_info("4. Select 'Spaceship Controls Component'")
            log_info("5. Compile and Save the Blueprint")
            log_info("6. Test PIE again")

        log_info("=" * 80)

        return has_controls_component

    except Exception as e:
        log_error(f"Check failed: {e}")
        import traceback
        log_error(traceback.format_exc())
        return False

if __name__ == "__main__":
    has_component = check_spaceship_controls_component()
    if has_component:
        print("✅ SpaceshipControlsComponent is present!")
    else:
        print("❌ SpaceshipControlsComponent is missing - follow manual steps above")