"""
Convert a downloaded BlenderKit .blend into an Adastrea-pipeline-ready FBX.

Appends mesh objects from a source .blend (fetched via blenderkit_fetch.py)
into a clean scene, bakes transforms, and exports using the exact same FBX
settings as Tools/generate_adastrea_assets.py's export_fbx() so the result
drops straight into the existing fbx2obj.py -> qa_assets.py -> ue_import_*.py
chain with no special-casing.

Scale note (empirically verified, don't trust the "meters" intuition here):
this pipeline's export_fbx() (apply_unit_scale=True against Blender's default
scale_length=1.0) round-trips a raw Blender unit to exactly 1 in the final
OBJ/QA/UE space — i.e. the project treats one raw Blender unit as ONE
CENTIMETER, not one meter, despite scale_length nominally meaning "meters
per unit". (Verified directly: a 1x1x1-unit test cube exported through this
exact FBX path and re-imported by fbx2obj.py measures 1x1x1 in the QA OBJ.)
BlenderKit assets are authored in real-world SI units where 1 Blender unit
really does equal 1 meter, so appended as-is they land 100x too small (a
real 10cm panel exports as a 0.1cm sliver — this is exactly what caught a
real SM_Prop_SciFiControlPanel import: QA's bounds check failed at
"0.1 x 0.1 x 0.1 cm" until the correction below was added). Default --scale
is therefore 100.0 (meters -> this-project's-cm convention). Override it
only if a specific source asset is already modeled at an unusual scale
(check --list-reported dimensions against the real object first).

Usage:
    blender -b --python Tools/blenderkit_import.py -- <source.blend> <SM_OutputName> [--filter substring] [--scale N] [--list]

    --list          just print object names in the source file and exit (no export)
    --filter TEXT   only import mesh objects whose name contains TEXT (case-insensitive)
    --scale N       uniform scale correction applied after import (default 100.0 — see note above)

Output:
    Assets/FBX/generated/kitbash/<SM_OutputName>.fbx
    (separate from the procedural generator's output dir so it's obvious at a
    glance which meshes are sourced vs. code-generated)
"""
import sys
import os
import argparse
import bpy

BASE = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\kitbash"


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    ap = argparse.ArgumentParser()
    ap.add_argument("source_blend")
    ap.add_argument("output_name", nargs="?")
    ap.add_argument("--filter", default=None)
    ap.add_argument("--scale", type=float, default=100.0,
                     help="meters -> this-project's-cm-per-unit convention (default 100.0, see module docstring)")
    ap.add_argument("--list", action="store_true")
    return ap.parse_args(argv)


def list_objects(source_blend):
    with bpy.data.libraries.load(source_blend, link=False) as (data_from, _):
        names = list(data_from.objects)
    print(f"{len(names)} object(s) in {source_blend}:")
    for n in names:
        print(f"  {n}")


def setup_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.length_unit = 'CENTIMETERS'
    return scene


def append_meshes(source_blend, name_filter=None):
    # Import the WHOLE object hierarchy, not just name-matching meshes.
    # BlenderKit/kitbash assets commonly model at an arbitrary local scale and
    # rely on a parent Empty for the real-world size — appending only the mesh
    # (by filter) silently drops that parent and exports at the wrong scale
    # (millimeter-tiny props). Bringing in every object lets Blender resolve
    # the real parent links, then we bake and drop the parent below.
    with bpy.data.libraries.load(source_blend, link=False) as (data_from, data_to):
        data_to.objects = list(data_from.objects)

    for obj in data_to.objects:
        if obj is not None:
            bpy.context.scene.collection.objects.link(obj)
    bpy.context.view_layer.update()

    targets = [o for o in data_to.objects
               if o is not None and o.type == 'MESH'
               and (name_filter is None or name_filter.lower() in o.name.lower())]
    if not targets:
        raise SystemExit(
            f"no mesh objects matched filter={name_filter!r} in {source_blend}; "
            f"run with --list to see available names"
        )

    # Bake each target's true world transform (parent chain included) into its
    # own local transform, then drop the parent — CLEAR_KEEP_TRANSFORM is the
    # operator that does this without moving/resizing the object.
    bpy.ops.object.select_all(action='DESELECT')
    for obj in targets:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = targets[0]
    bpy.ops.object.parent_clear(type='CLEAR_KEEP_TRANSFORM')

    return targets


def bake_transforms(objs, extra_scale=1.0):
    for obj in objs:
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        if extra_scale != 1.0:
            obj.scale = tuple(s * extra_scale for s in obj.scale)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)


def report_bounds(objs):
    import mathutils
    if not objs:
        return
    mn = mathutils.Vector((float('inf'),) * 3)
    mx = mathutils.Vector((float('-inf'),) * 3)
    for obj in objs:
        for corner in obj.bound_box:
            world = obj.matrix_world @ mathutils.Vector(corner)
            mn = mathutils.Vector(min(a, b) for a, b in zip(mn, world))
            mx = mathutils.Vector(max(a, b) for a, b in zip(mx, world))
    size = mx - mn
    print(f"Post-scale bounds: {size.x:.2f} x {size.y:.2f} x {size.z:.2f} "
          f"(this is what the exported FBX will measure as, in project cm)")
    if max(size) > 3000 or max(size) < 1.0:
        print("  [warn] size looks off for a prop/panel-scale asset (expect "
              "roughly 1-3000 cm) — check --scale, the source asset may not "
              "be modeled at real-world-meter scale")


def export_fbx(objs, outname):
    os.makedirs(BASE, exist_ok=True)
    out = os.path.join(BASE, outname + ".fbx")
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objs:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z',
    )
    return out


def main():
    args = parse_args()
    source_blend = os.path.abspath(args.source_blend)
    if not os.path.isfile(source_blend):
        raise SystemExit(f"not found: {source_blend}")

    setup_scene()

    if args.list:
        list_objects(source_blend)
        return

    if not args.output_name:
        raise SystemExit("output_name required unless --list is given")

    imported = append_meshes(source_blend, name_filter=args.filter)
    print(f"appended {len(imported)} mesh object(s)")
    bake_transforms(imported, extra_scale=args.scale)
    report_bounds(imported)
    out = export_fbx(imported, args.output_name)
    print(f"OK: {out} ({os.path.getsize(out)} bytes)")
    print("Next: run Tools/fbx2obj.py then Tools/qa_assets.py to QA-gate it "
          "like any generated part before importing into UE.")


if __name__ == "__main__":
    main()
