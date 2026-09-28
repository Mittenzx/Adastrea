"""Reimport regenerated texture PNGs over their existing Texture2D assets, in place.

Finds every Texture2D named T_<Set>_<Map> under the given content roots and, when a
matching PNG exists in the generated-texture folders, imports that PNG over it. The
asset's own settings (sRGB, compression, flip green, LOD group, address modes) are
read before the import and written back after, so materials see exactly the same
texture setup with new pixels.

Run headless (editor closed):
  UnrealEditor-Cmd.exe Adastrea.uproject -run=pythonscript -script="Tools/reimport_generated_textures.py [--dry-run] [--only Set1,Set2]" -unattended -nosplash -nullrhi
"""
import os
import sys

import unreal

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PNG_DIRS = [os.path.join(ROOT, "Assets", "FBX", "generated", "Textures"),
            os.path.join(ROOT, "Assets", "Textures", "generated", "interiors")]
CONTENT_ROOTS = ["/Game/Textures", "/AdastreaShips/Textures"]
KEEP = ["srgb", "compression_settings", "flip_green_channel", "lod_group",
        "address_x", "address_y", "mip_gen_settings"]


def log(m):
    unreal.log("[reimport-tex] " + str(m))


def parse_args():
    argv = sys.argv[1:]
    dry = "--dry-run" in argv
    only = None
    if "--only" in argv:
        only = set(argv[argv.index("--only") + 1].split(","))
    return dry, only


def png_for(asset_name):
    for d in PNG_DIRS:
        p = os.path.join(d, asset_name + ".png")
        if os.path.exists(p):
            return p
    return None


def main():
    dry, only = parse_args()
    reg = unreal.AssetRegistryHelpers.get_asset_registry()
    # commandlets don't wait for the background scan (plugin content would be missing)
    reg.scan_paths_synchronous(CONTENT_ROOTS, True)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    done = skipped = 0
    for root in CONTENT_ROOTS:
        for ad in reg.get_assets_by_path(root, recursive=True):
            if str(ad.asset_class_path.asset_name) != "Texture2D":
                continue
            name = str(ad.asset_name)
            if not name.startswith("T_"):
                continue
            set_name = name[2:].rsplit("_", 1)[0]
            if only and set_name not in only:
                continue
            png = png_for(name)
            if not png:
                skipped += 1
                continue
            path = str(ad.package_name)
            folder = str(ad.package_path)
            if dry:
                log("would reimport %s <- %s" % (path, png))
                done += 1
                continue
            tex = unreal.load_asset(path)
            saved = {}
            for k in KEEP:
                try:
                    saved[k] = tex.get_editor_property(k)
                except Exception:
                    pass
            task = unreal.AssetImportTask()
            task.filename = png
            task.destination_path = folder
            task.destination_name = name
            task.replace_existing = True
            task.replace_existing_settings = False
            task.automated = True
            task.save = False
            tools.import_asset_tasks([task])
            tex = unreal.load_asset(path)
            for k, v in saved.items():
                try:
                    tex.set_editor_property(k, v)
                except Exception as e:
                    log("could not restore %s on %s: %s" % (k, name, e))
            unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
            done += 1
    log("%s %d textures, %d without a generated PNG" % ("would reimport" if dry else "reimported", done, skipped))


main()
