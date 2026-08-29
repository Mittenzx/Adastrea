import unreal
BP = "/Game/Blueprints/Ships/BP_Ship_Fighter"
bpc = unreal.EditorAssetLibrary.load_asset(BP)
gen = bpc.generated_class()
cdo = unreal.get_default_object(gen)

SCALE = 0.015  # ~1,900x860 units after (was 126k wide raw). Small fighter ~19m.
comp = None
if hasattr(cdo, 'get_components_by_class'):
    comps = [c for c in cdo.get_components_by_class(unreal.StaticMeshComponent) if c is not None]
    comp = comps[0] if comps else None
if comp:
    comp.set_editor_property("relative_scale3d", unreal.Vector(SCALE, SCALE, SCALE))
    unreal.log("=== fighter mesh scale -> %s ===" % SCALE)
else:
    unreal.log_error("=== no mesh comp ===")

# Camera: ~2.5x ship length behind for a clean view., pull back from 4500 already set.
try:
    cdo.set_editor_property("camera_distance", 5000.0)
    unreal.log("=== camera_distance -> 5000 ===")
except Exception as e:
    unreal.log_error("cam: %s" % e)

unreal.EditorAssetLibrary.save_asset(BP)
unreal.log("=== SAVED ===")
unreal.log("=== ship_view_fix.py complete ===")