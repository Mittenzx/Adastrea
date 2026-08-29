"""Set the assembled fighter mesh as the visual on BP_Ship_Fighter's ShipMesh
component (C++ ASpaceship::ShipMeshComponent). Resolves the mesh under the
AdastreaShips plugin content mount.
"""
import unreal

MESH = "/AdastreaShips/Meshes/Ships/SM_Ship_Fighter_01_Assembled"
BP = "/Game/Blueprints/Ships/BP_Ship_Fighter"

if not unreal.EditorAssetLibrary.does_asset_exist(MESH):
    unreal.log_error("=== Mesh asset not found at %s ===" % MESH)
    unreal.log_error("Available: %s" % unreal.EditorAssetLibrary.list_assets("/AdastreaShips/Meshes/Ships"))
else:
    sm = unreal.load_asset(MESH)
    unreal.log("=== Loaded mesh: %s (%s) ===" % (MESH, sm.get_class().get_name()))

    bpc = unreal.EditorAssetLibrary.load_asset(BP)
    gen = bpc.generated_class()
    cdo = unreal.get_default_object(gen)

    # The C++ component is ShipMeshComponent (a StaticMeshComponent subtype).
    comps = [c for c in cdo.get_components_by_class(unreal.StaticMeshComponent) if c is not None] if hasattr(cdo, 'get_components_by_class') else []
    if not comps and hasattr(cdo, 'get_components_by_tag'):
        comps = list(cdo.get_components_by_tag('ShipMesh'))
    if comps:
        comp = comps[0]
        comp.set_editor_property("static_mesh", sm)
        unreal.log("=== Set BP_Ship_Fighter.mesh -> %s ===" % MESH)
        unreal.EditorAssetLibrary.save_asset(BP)
        unreal.log("=== SAVED BP_Ship_Fighter ===")
    else:
        unreal.log_error("=== No StaticMeshComponent found on fighter CDO (comps=%d) ===" % len(comps))
unreal.log("=== set_fighter_mesh.py complete ===")