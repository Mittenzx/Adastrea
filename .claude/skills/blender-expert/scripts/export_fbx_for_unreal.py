"""Export FBX files with Unreal-friendly settings. Does not modify or save the .blend.

Each top-level object is written to its own FBX, named after that object, together with its
children (e.g. SOCKET_ empties, parented UCX_ collision) and any unparented collision meshes
named UCX_/UBX_/USP_/UCP_ + the mesh name (optionally with a _01 style suffix).

Headless (all scene objects unless you narrow it down):
    blender -b file.blend --python export_fbx_for_unreal.py -- --out D:/Game/SourceArt/Exports --dry-run
    blender -b file.blend --python export_fbx_for_unreal.py -- --out D:/Game/SourceArt/Exports --collection Props
Options: --names A B | --collection NAME | --selected | --all, plus --anim, --deform-only, --dry-run
Inside Blender's Text Editor with no arguments: exports the current selection to //exports.

Settings used: FBX Units Scale, Face smoothing, modifiers applied, no leaf bones.
Verify with a test import in Unreal before exporting a whole library.
"""

import argparse
import os
import re
import sys

import bpy

COLLISION_PREFIXES = ("UCX_", "UBX_", "USP_", "UCP_")
SOCKET_PREFIX = "SOCKET_"
EXPORTABLE = {"MESH", "ARMATURE", "EMPTY"}


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(prog="export_fbx_for_unreal", description="FBX export for Unreal Engine")
    parser.add_argument("--out", default="//exports", help="output folder ('//' means next to the .blend)")
    scope = parser.add_mutually_exclusive_group()
    scope.add_argument("--names", nargs="+", help="export only these objects")
    scope.add_argument("--collection", help="export objects in this collection, including child collections")
    scope.add_argument("--selected", action="store_true", help="export the current selection")
    scope.add_argument("--all", action="store_true", help="export every object in the scene")
    parser.add_argument("--anim", action="store_true", help="include baked animation")
    parser.add_argument("--deform-only", action="store_true", help="export deform bones only")
    parser.add_argument("--dry-run", action="store_true", help="list what would be exported, write nothing")
    return parser.parse_args(argv)


def descendants(ob):
    found = []
    for child in ob.children:
        found.append(child)
        found.extend(descendants(child))
    return found


def is_helper(name):
    return name.startswith(COLLISION_PREFIXES) or name.startswith(SOCKET_PREFIX)


def collision_target(name, root_names):
    """UCX_SM_Crate_01 -> SM_Crate_01 or SM_Crate, whichever is being exported."""
    base = name.split("_", 1)[1] if "_" in name else name
    if base in root_names:
        return base
    trimmed = re.sub(r"_\d+$", "", base)
    return trimmed if trimmed in root_names else None


def pick_objects(args, scene):
    if args.names:
        missing = [n for n in args.names if n not in bpy.data.objects]
        if missing:
            print(f"[export] not found: {', '.join(missing)}")
        return [bpy.data.objects[n] for n in args.names if n in bpy.data.objects]
    if args.collection:
        coll = bpy.data.collections.get(args.collection)
        if coll is None:
            raise SystemExit(f"[export] collection not found: {args.collection}")
        return list(coll.all_objects)
    use_selection = args.selected or (not args.all and not bpy.app.background)
    if use_selection:
        return list(bpy.context.selected_objects)
    return list(scene.objects)


def build_groups(candidates, scene_objects):
    chosen = set(candidates)
    roots = [o for o in candidates if o.parent not in chosen and not is_helper(o.name)]
    groups = {}
    for root in roots:
        members = [root] + [d for d in descendants(root) if d.type in EXPORTABLE]
        if root.type == "EMPTY" and len(members) == 1:
            continue  # a lone empty has nothing to export
        groups[root.name] = members

    grouped = {o for members in groups.values() for o in members}

    # Loose collision meshes: search the whole scene, so --names SM_Crate still brings UCX_SM_Crate_01.
    for ob in scene_objects:
        if ob in grouped or ob.type not in EXPORTABLE or not ob.name.startswith(COLLISION_PREFIXES):
            continue
        target = collision_target(ob.name, groups.keys())
        if target:
            groups[target].append(ob)
            grouped.add(ob)

    for ob in candidates:
        if ob not in grouped and is_helper(ob.name):
            print(f"[export] skipped {ob.name}: parent it to its mesh, or name it like UCX_<MeshName>_01")
    return groups


def select_only(view_layer, objects):
    for ob in view_layer.objects:
        ob.select_set(False)
    selected = []
    for ob in objects:
        try:
            ob.select_set(True)
            selected.append(ob)
        except RuntimeError:
            print(f"[export]   {ob.name} is not in the active view layer; skipped")
    return selected


def main():
    args = parse_args()
    scene = bpy.context.scene
    view_layer = bpy.context.view_layer
    out_dir = bpy.path.abspath(args.out)

    candidates = [o for o in pick_objects(args, scene) if o.type in EXPORTABLE]
    groups = build_groups(candidates, list(scene.objects))
    if not groups:
        print("[export] nothing to export (check your selection, --names or --collection)")
        return

    if not args.dry_run:
        os.makedirs(out_dir, exist_ok=True)

    previous_selection = list(bpy.context.selected_objects)
    previous_active = view_layer.objects.active
    written = 0
    try:
        for name, members in groups.items():
            path = os.path.join(out_dir, f"{bpy.path.clean_name(name)}.fbx")
            print(f"[export] {name}: {', '.join(o.name for o in members)} -> {path}")
            if args.dry_run:
                continue
            selected = select_only(view_layer, members)
            if not selected:
                continue
            view_layer.objects.active = selected[0]
            bpy.ops.export_scene.fbx(
                filepath=path,
                use_selection=True,
                object_types=EXPORTABLE,
                apply_unit_scale=True,
                apply_scale_options="FBX_SCALE_UNITS",
                use_mesh_modifiers=True,
                mesh_smooth_type="FACE",
                add_leaf_bones=False,
                use_armature_deform_only=args.deform_only,
                bake_anim=args.anim,
            )
            written += 1
    finally:
        for ob in view_layer.objects:
            ob.select_set(False)
        for ob in previous_selection:
            try:
                ob.select_set(True)
            except RuntimeError:
                pass
        view_layer.objects.active = previous_active

    verb = "would write" if args.dry_run else "wrote"
    count = len(groups) if args.dry_run else written
    print(f"[export] {verb} {count} file(s) in {out_dir}")


main()
