"""Second half of the ship-redesign import; needs the full editor (StaticMeshEditorSubsystem
is None under -run=pythonscript). Regenerates hull collision from the new shapes and points
BP_Ship_Corvette / BP_Battleship HullMaterialOverride at the tiled class hull (their old
baked unique-UV materials don't fit the redesign geometry).

Run at editor startup:
  UnrealEditor.exe Adastrea.uproject -ExecutePythonScript="<abs>/Tools/ue_ship_redesign_finish.py"
"""
import unreal

HULLS = ["/AdastreaShips/Meshes/Ships/SM_Ship_%s_Assembled" % s for s in (
    "Gunship_02", "Escort_01", "Courier_01", "Smuggler_01", "Trader_01", "Freighter_01", "CargoFreighter_01",
    "Corvette_01", "Miner_01", "Destroyer_01", "HeavyHauler_01", "BulkCarrier_01", "Battleship_01")] + [
    "/Game/Assets/Ships/SM_Ship_Cruiser_01_Assembled", "/Game/Assets/Ships/SM_Ship_CommandXL_01_Assembled",
    "/Game/Assets/Ships/SM_Ship_Corvette_01_Assembled_UniqueUV",
    "/Game/Assets/Ships/SM_Ship_Battleship_01_Assembled_UniqueUV"]
BP_OVERRIDES = {"/Game/Blueprints/Ships/BP_Ship_Corvette": "/Game/Materials/M_Corvette_Hull",
                "/Game/Blueprints/Ships/BP_Battleship": "/Game/Materials/M_Battleship_Hull"}
EAL = unreal.EditorAssetLibrary


def log(m):
    unreal.log_warning("[ship-redo-finish] " + str(m))


def main():
    smes = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    for path in HULLS:
        mesh = unreal.load_asset(path)
        if not mesh:
            log("! missing " + path)
            continue
        before = (smes.get_convex_collision_count(mesh), smes.get_simple_collision_count(mesh))
        smes.remove_collisions(mesh)
        smes.set_convex_decomposition_collisions(mesh, 12, 24, 200000)
        EAL.save_asset(path, only_if_is_dirty=False)
        log("%s collision convex/simple %s -> %s" % (path.split("/")[-1], before,
            (smes.get_convex_collision_count(mesh), smes.get_simple_collision_count(mesh))))
    for bp_path, mat_path in BP_OVERRIDES.items():
        bp = unreal.load_asset(bp_path)
        cdo = unreal.get_default_object(bp.generated_class())
        cdo.set_editor_property("hull_material_override", unreal.load_asset(mat_path))
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        EAL.save_asset(bp_path, only_if_is_dirty=False)
        log("%s HullMaterialOverride -> %s" % (bp_path.split("/")[-1],
            unreal.get_default_object(bp.generated_class()).get_editor_property("hull_material_override")))
    log("DONE")


main()
