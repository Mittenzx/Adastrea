"""UE editor-Python one-shot: import ALL Adastrea meshes and link each part to its
UE-native M_* material (built by ue_fleet_materials.py).

Imports every FBX in Assets/FBX/generated (ships, interiors, stations, objects,
props, combat) as a StaticMesh, then assigns the correct M_* material to its
material slot 0 based on the part name (hull/engine/weapon/cargo/sensor/reactor/...).

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Self-contained (boot -> run -> exit). Do NOT leave an interactive editor open.
"""
import unreal
import os, glob, json

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
DEST = "/Game/Assets/All"       # consolidated import folder (below /Game/Assets)
MAT_FOLDER = "/Game/Materials/"

def log(m):
    unreal.log("[mesh-link] " + str(m))

def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def slot_mat(name):
    """Map a part/asset name to the M_* material to assign (matches material_map)."""
    b = name.replace(".fbx", "")
    if b.startswith("SM_Ship_"):
        ship = b.split("_")[2] if len(b.split("_"))>2 else ""
        if b.endswith("_Carcass"):                       return "M_Hull"
        if b.endswith("_Assembled"):                     return "M_Hull"   # assembled uses carcass hull mat
        if b.endswith("_Engine"):        return "M_Engine_Block"
        if b.endswith("_Reactor"):       return "M_Reactor_Block"
        if b.endswith("_Weapon"):        return "M_Weapon_Block"
        if b.endswith("_Sensor"):        return "M_Sensor_Block"
        if b.endswith("_Cargo"):         return "M_Cargo_Hold"
        if b.endswith("_Drill"):         return "M_Drill"
        if b.endswith("_MiningLaser"):   return "M_MiningLaser"
        # cargo ships map hull by their faction set later; default Hull
        return "M_Hull"
    if b.startswith("SM_Int_"):
        if b.endswith("_Shell") or b.endswith("Airlock") or b.endswith("Corridor"): return "M_Interior_Hab"
        if b.endswith("_Deck") or b.endswith("_Gall") or b.endswith("_Floor"):       return "M_Interior_Deck"
        if b.endswith("_Bunks") or b.endswith("_Mess") or b.endswith("_Desks"):      return "M_Interior_Hab"
        if "Cockpit" in b:             return "M_Interior_Cockpit"
        if "AlienHold" in b or "Xenomorph" in b: return "M_Interior_Alien"
        return "M_Interior_Hab"
    if b.startswith("SM_Station_"):
        if b.endswith("_Carcass"):     return "M_Station_Hab"
        if b.endswith("_HabRing"):     return "M_Habitat_Ring"
        if b.endswith("_Sensor"):      return "M_Sensor_Block"
        if b.endswith("_Cargo"):       return "M_Cargo_Hold"
        if b.endswith("_Drill"):       return "M_Drill"
        if b.endswith("_MiningLaser"): return "M_MiningLaser"
        if b.endswith("_Reactor"):     return "M_Reactor_Block"
        if b.endswith("_AsteroidShell"): return "M_Asteroid_Shell"
        return "M_Station_Hab"
    if b.startswith("SM_Obj_"):
        if "Satellite" in b:    return "M_Satellite"
        if "Derelict" in b:     return "M_Derelict"
        if "FuelCell" in b:     return "M_FuelCell"
        if "CommsTower" in b:   return "M_CommsTower"
        return "M_Hull"
    if b.startswith("SM_Combat_"):
        if "Plasma" in b:       return "M_Combat_Plasma"
        if "Laser" in b:        return "M_Combat_Laser"
        if "Missile" in b:      return "M_Combat_Missile"
        return "M_Hull"
    if b.startswith("SM_Prop_"):
        if "Crate" in b or "Container" in b: return "M_Prop_Crate"
        if "Tank" in b or "Hazard" in b:     return "M_Prop_Tank"
        return "M_Hull"
    if b.startswith("SM_StarDome"): return "M_Hull"
    return "M_Hull"

def load_material(matname):
    path = MAT_FOLDER + matname
    return unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None

def main():
    ensure_folder(DEST)
    at = unreal.AssetToolsHelpers.get_asset_tools()

    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("import_materials", False)
    opts.static_mesh_import_data.set_editor_property("import_uniform_scale", 1.0)

    # Gather FBX EXCLUDING nav lights (already imported) and old single _Nav.fbx
    fbxs = []
    for f in glob.glob(os.path.join(GEN, "*.fbx")):
        b = os.path.basename(f)
        if "_Nav_Nav" in b or ("_Nav.fbx" in b):   # nav lights already done
            continue
        fbxs.append(f)

    log("found %d FBX to import" % len(fbxs))
    imported = linked = 0
    missing_mat = set()
    for fbx in fbxs:
        b = os.path.basename(fbx)
        dest_name = b[:-4]
        task = unreal.AssetImportTask()
        task.filename = fbx
        task.destination_path = DEST
        task.destination_name = dest_name
        task.replace_existing = True
        task.automated = True
        task.save = True
        task.options = opts
        at.import_asset_tasks([task])
        mesh_path = DEST + "/" + dest_name
        if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
            imported += 1
            mesh = unreal.load_asset(mesh_path)
            matname = slot_mat(dest_name)
            mat = load_material(matname)
            if mesh is not None:
                if mat is not None:
                    mesh.set_material(0, mat)
                    linked += 1
                else:
                    missing_mat.add(matname)
                unreal.EditorAssetLibrary.save_asset(mesh_path)
    log("imported=%d linked=%d  missing_materials=%s ALL_DONE" % (imported, linked, sorted(missing_mat)))

main()