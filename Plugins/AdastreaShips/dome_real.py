"""Assign the realistic starfield material to the ACTUAL giant dome (SMA_3).
The previous run matched a wrong small sphere (SMA_1). Target by the huge
scale-100000 Sphere at origin.
"""
import unreal

mat = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_Starfield_Real")
if not mat:
    unreal.log_error("=== M_Starfield_Real not found ===")
else:
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    done = False
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StaticMeshActor):
        try:
            scale = a.get_actor_scale3d()
        except Exception:
            continue
        label = a.get_actor_label()
        # The real dome is the huge sphere (scale ~100000).
        if scale.x > 1000:
            comp = a.static_mesh_component
            if comp:
                comp.set_material(0, mat)
                unreal.log("=== Assigned M_Starfield_Real to dome %s (scale %s) ===" % (label, scale))
                done = True
                break
    if not done:
        unreal.log_error("=== No huge-scale dome found ===")
    else:
        try:
            from unreal import LevelEditorSubsystem
            LevelEditorSubsystem().save_current_level()
            unreal.log("=== Level saved ===")
        except Exception as e:
            unreal.log_error("Level save failed (non-fatal): %s" % e)
unreal.log("=== dome_real.py complete ===")