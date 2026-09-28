"""Import the separate ship window meshes (Tools/build_ship_windows.py) and their materials.

Builds /Game/Materials/M_ShipWindow (lit glass master: BaseColor, Roughness, Emissive
colour x strength) plus MI_ShipWindow_{LitCool,LitWarm,Dark,Frame}, then imports every
Assets/FBX/generated/SM_Ship_*_01_Windows.fbx through import_art_gap_assets (which binds
slot M_ShipWindow_X to MI_ShipWindow_X by name). ASpaceship::AttachShipWindows puts the
mesh on the hull at runtime.

Pipeline: (1) this script --export-hulls DIR, (2) blender build_ship_windows.py -- --ue-hulls DIR,
(3) this script (imports every SM_Ship_*_01_Windows.fbx).
Run headless (editor closed):
  UnrealEditor-Cmd.exe Adastrea.uproject -run=pythonscript -script="<abs>/Tools/ue_import_ship_windows.py [--dry-run]" -unattended -nosplash -nullrhi
"""
import glob
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_art_gap_assets as ia  # noqa: E402

MAT_DIR = "/Game/Materials"
MASTER = MAT_DIR + "/M_ShipWindow"
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

# instance -> (base colour, roughness, emissive colour, emissive strength)
INSTANCES = {
    "MI_ShipWindow_LitCool": ((0.02, 0.025, 0.03), 0.06, (0.55, 0.80, 1.00), 4.0),
    "MI_ShipWindow_LitWarm": ((0.02, 0.02, 0.02), 0.06, (1.00, 0.72, 0.42), 4.0),
    "MI_ShipWindow_Dark":    ((0.015, 0.018, 0.022), 0.04, (0.0, 0.0, 0.0), 0.0),
    "MI_ShipWindow_Frame":   ((0.09, 0.09, 0.10), 0.45, (0.0, 0.0, 0.0), 0.0),
}


def log(m):
    unreal.log("[ship-windows] " + str(m))


def build_master():
    if EAL.does_asset_exist(MASTER):
        mat = unreal.load_asset(MASTER)
        MEL.delete_all_material_expressions(mat)
    else:
        mat = AT.create_asset("M_ShipWindow", MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)

    def vparam(name, val, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
        n.set_editor_property("parameter_name", name)
        n.set_editor_property("default_value", unreal.LinearColor(val[0], val[1], val[2], 1.0))
        return n

    def sparam(name, val, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
        n.set_editor_property("parameter_name", name)
        n.set_editor_property("default_value", val)
        return n

    base = vparam("BaseColor", (0.02, 0.025, 0.03), -600, -200)
    rough = sparam("Roughness", 0.06, -600, 0)
    emis = vparam("EmissiveColor", (0.55, 0.8, 1.0), -600, 150)
    strength = sparam("EmissiveStrength", 4.0, -600, 320)
    mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 200)
    MEL.connect_material_expressions(emis, "", mul, "A")
    MEL.connect_material_expressions(strength, "", mul, "B")
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.recompile_material(mat)
    EAL.save_asset(MASTER, only_if_is_dirty=False)
    log("built " + MASTER)
    return mat


def build_instances(master):
    for name, (base, rough, emis, strength) in INSTANCES.items():
        path = MAT_DIR + "/" + name
        if EAL.does_asset_exist(path):
            mi = unreal.load_asset(path)
        else:
            mi = AT.create_asset(name, MAT_DIR, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
        MEL.set_material_instance_parent(mi, master)
        MEL.set_material_instance_vector_parameter_value(mi, "BaseColor", unreal.LinearColor(*base, 1.0))
        MEL.set_material_instance_scalar_parameter_value(mi, "Roughness", rough)
        MEL.set_material_instance_vector_parameter_value(mi, "EmissiveColor", unreal.LinearColor(*emis, 1.0))
        MEL.set_material_instance_scalar_parameter_value(mi, "EmissiveStrength", strength)
        EAL.save_asset(path, only_if_is_dirty=False)
        log("instance " + name)


# Hull assets whose geometry differs from today's FBX on disk (imported from an older
# export); build_ship_windows.py places their windows on these exported meshes.
UE_HULLS = ["/Game/Assets/Ships/SM_Ship_Cruiser_01_Assembled",
            "/AdastreaShips/Meshes/Ships/SM_Ship_Destroyer_01_Assembled",
            "/AdastreaShips/Meshes/Ships/SM_Ship_Freighter_01_Assembled",
            "/Game/Assets/Ships/SM_Ship_Corvette_01_Assembled_UniqueUV"]


def export_hulls(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    for p in UE_HULLS:
        t = unreal.AssetExportTask()
        t.object = unreal.load_asset(p)
        t.filename = os.path.join(out_dir, p.split("/")[-1] + ".fbx")
        t.automated = True
        t.prompt = False
        t.replace_identical = True
        t.exporter = unreal.StaticMeshExporterFBX()
        log("export %s -> %s: %s" % (p, t.filename, unreal.Exporter.run_asset_export_task(t)))


def main(argv):
    if "--export-hulls" in argv:
        export_hulls(argv[argv.index("--export-hulls") + 1])
        return
    dry = "--dry-run" in argv
    fbxs = sorted(glob.glob(os.path.join(ia.GEN_DIR, "SM_Ship_*_01_Windows.fbx")))
    names = [os.path.basename(f)[:-4] for f in fbxs]
    log("%d window meshes: %s" % (len(names), ", ".join(names)))
    if dry:
        ia.DRY_RUN = True
    else:
        build_instances(build_master())
    ok = sum(1 for n in names if ia.import_mesh(n))
    log("imported %d/%d RESULT_%s" % (ok, len(names), "OK" if ok == len(names) else "FAIL"))


main(sys.argv[1:])
