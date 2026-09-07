"""UE one-shot: import the new empty-room interior FBX into the plugin path the
fighter's interior resolves to (and the test expects):
  /AdastreaShips/Meshes/Interiors/SM_Int_Fighter_EmptyRoom
Run via: UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Self-contained (boot->run->exit). Close interactive editor first.
"""
import unreal, os

FBX = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\SM_Int_Fighter_EmptyRoom.fbx"
DEST = "/AdastreaShips/Meshes/Interiors"
NAME = "SM_Int_Fighter_EmptyRoom"

def log(m): unreal.log("[er-import] " + str(m))

def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(DEST):
        unreal.EditorAssetLibrary.make_directory(DEST)
    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("import_materials", False)
    opts.static_mesh_import_data.set_editor_property("import_uniform_scale", 1.0)
    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = DEST
    task.destination_name = NAME
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    p = DEST + "/" + NAME
    log("imported; exists=" + str(unreal.EditorAssetLibrary.does_asset_exist(p + "." + NAME)))
    unreal.log("[er-import] ER_IMPORT_DONE")

main()