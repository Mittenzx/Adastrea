"""Fix the fighter ship scale - imported assembled mesh is ~20x too large
(~57k units; a fighter should be ~2-3k). Scale the ShipMesh component down and
push the player camera behind the smaller ship.
"""
import unreal

BP = "/Game/Blueprints/Ships/BP_Ship_Fighter"
MESH_SCALE = 0.05  # 57k * 0.05 = ~2.85k units

bpc = unreal.EditorAssetLibrary.load_asset(BP)
gen = bpc.generated_class()
cdo = unreal.get_default_object(gen)

# Find the C++ ShipMeshComponent (a StaticMeshComponent subtype) on the CDO.
comp = None
if hasattr(cdo, 'get_components_by_class'):
    comps = [c for c in cdo.get_components_by_class(unreal.StaticMeshComponent) if c is not None]
    comp = comps[0] if comps else None

if comp:
    comp.set_editor_property("relative_scale3d", unreal.Vector(MESH_SCALE, MESH_SCALE, MESH_SCALE))
    comp.set_editor_property("relative_location", unreal.Vector(0, 0, 0))
    unreal.log("=== Set fighter ShipMesh scale to %s ===" % MESH_SCALE)
else:
    unreal.log_error("=== No StaticMeshComponent found to scale ===")

# Camera: sit behind the ~2850-unit ship so it doesn't obscure the view.
try:
    cdo.set_editor_property("camera_distance", 4500.0)
    unreal.log("=== Set camera_distance=4500 ===")
except Exception as e:
    unreal.log_error("camera prop set failed: %s" % e)

unreal.EditorAssetLibrary.save_asset(BP)
unreal.log("=== SAVED BP_Ship_Fighter ===")
unreal.log("=== fix_ship_scale.py complete ===")