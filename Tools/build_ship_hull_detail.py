"""Production hull detailing for one ship (techniques from Tools/hull_detail_showcase.py).

  hull    1-segment flat chamfer on the hard edges + armour plating (sliced, inset,
          raised/flush/recessed plates), applied to the in-game hull. UVs and the
          M_Assembled slot are kept, so the tiled class material (M_<Ship>_Hull)
          still binds. Faces under window geometry are paneled but stay flush so
          the separate _Windows mesh is never buried.
  detail  separate kit mesh <ship>_Detail: RCS quads, airlock hatches, radiator
          fins, sensor blisters, registration number, a light greeble scatter.
          Same pivot/frame as the hull, so it attaches at zero relative transform
          (ASpaceship::AttachShipDetail). Nav lights, antenna and dish are left to
          UExteriorDressingComponent, which fits them to the hull at runtime.

Source of truth is the redesign hull in Assets/FBX/concepts/redo (never modified);
outputs overwrite Assets/FBX/generated/<hull>.fbx and write <ship>_Detail.fbx plus
the hazard-stripe texture T_ShipDetail_Hazard_D.png.

Run: blender -b --python Tools/build_ship_hull_detail.py -- [--ship Corvette_01] [--seed 417] [--dry-run]
"""
import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree

_argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
sys.argv = sys.argv[:1]  # keep the showcase's own option parser away from our args
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hull_detail_showcase as hd  # noqa: E402  (also imports build_unique_hull_textures/gen)

gen = hd.gen


def opt(name, default=None):
    return _argv[_argv.index(name) + 1] if name in _argv else default


SHIP = opt("--ship", "Corvette_01")
SEED = int(opt("--seed", "417"))
DRY = "--dry-run" in _argv
REDO = os.path.normpath(os.path.join(gen.BASE, "..", "concepts", "redo"))
# in-game hull asset name per ship (the Corvette/Battleship hulls kept the _UniqueUV name)
HULL_OUT = {"Corvette_01": "SM_Ship_Corvette_01_Assembled_UniqueUV"}.get(SHIP, f"SM_Ship_{SHIP}_Assembled")
# game metres per Blender unit; detail sizes in the showcase are in game metres
M_PER_BU = hd.uniq.FLEET_M_PER_BU

DETAIL_SLOTS = ["M_ShipDetail_Trim", "M_ShipDetail_Paint", "M_ShipDetail_Hazard",
                "M_ShipDetail_GlowWhite", "M_ShipDetail_GlowRed", "M_ShipDetail_GlowGreen", "M_ShipDetail_Dark"]


def import_obj(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=path, axis_forward='-Y', axis_up='Z')
    obs = [o for o in bpy.data.objects if o not in before and o.type == 'MESH']
    assert len(obs) == 1, (path, [o.name for o in obs])
    ob = obs[0]
    # bake the node transform into the mesh: UE bakes it too, so world positions are what count
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return ob


def scale_mesh(ob, k):
    ob.data.transform(Matrix.Scale(k, 4))
    ob.data.update()


def glazed_faces(hull, win):
    """Hull faces that window geometry sits on (nearest face to each window vertex)."""
    bvh = BVHTree.FromObject(hull, bpy.context.evaluated_depsgraph_get())
    hit = set()
    for v in win.data.vertices:
        loc, nrm, idx, dist = bvh.find_nearest(v.co)
        if idx is not None and dist < 0.05:
            hit.add(idx)
    return hit


def hazard_texture(path, size=256, stripes=8):
    img = bpy.data.images.new("T_ShipDetail_Hazard_D", size, size, alpha=False)
    px = []
    for y in range(size):
        for x in range(size):
            band = int((x + y) * stripes / size) % 2
            px += (0.75, 0.50, 0.04, 1.0) if band else (0.03, 0.03, 0.03, 1.0)
    img.pixels = px
    img.filepath_raw = path
    img.file_format = 'PNG'
    img.save()


def export_fbx(ob, outname):
    """Like gen.export_fbx, but writes raw centimetre coordinates the way the redesign
    hulls were written (UE ignores the FBX unit header; a metre scene through
    FBX_SCALE_ALL would come in 100x too big), with face smoothing groups."""
    gen.sel_activate(ob)
    out = os.path.join(gen.BASE, outname + ".fbx")
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'}, global_scale=0.01,
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE')
    return out


def main():
    gen.setup_scene()
    hull = import_obj(os.path.join(REDO, f"SM_Ship_{SHIP}_Assembled.fbx"))
    hull.name = HULL_OUT
    win_path = os.path.join(REDO, f"SM_Ship_{SHIP}_Windows.fbx")
    win = import_obj(win_path) if os.path.exists(win_path) else None
    for o in (hull, win):
        if o:
            scale_mesh(o, M_PER_BU)

    rnd = random.Random(SEED)
    faces = hd.survey(hull)
    glazed = glazed_faces(hull, win) if win else set()
    for f in faces:
        f.glazed = f.idx in glazed
    ys = [v.co.y for v in hull.data.vertices]
    ymin, ymax = min(ys), max(ys)
    tris0 = sum(len(p.vertices) - 2 for p in hull.data.polygons)

    # ---- hull: chamfer the original hard edges, then plate ----
    # (bevel first: the plates' inset rims are already sloped, so beveling every
    # seam would only multiply triangles; bevel strips are too thin to be plated)
    hd.apply_bevel(hull)
    bpy.ops.object.select_all(action='DESELECT')
    hull.select_set(True)
    bpy.context.view_layer.objects.active = hull
    for m in list(hull.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)
    # face indices changed: find the window-host faces again on the beveled hull
    plates = hd.apply_plating(hull, rnd, flush=glazed_faces(hull, win) if win else set())

    # ---- detail kit ----
    kit_faces = [f for f in faces if not f.glazed]
    det = bpy.data.objects.new(f"SM_Ship_{SHIP}_Detail", bpy.data.meshes.new(f"SM_Ship_{SHIP}_Detail"))
    bpy.context.scene.collection.objects.link(det)
    for name in DETAIL_SLOTS:
        det.data.materials.append(bpy.data.materials.get(name) or bpy.data.materials.new(name))
    bm = bmesh.new()
    bm.loops.layers.uv.new("UVMap")
    # seat each kit site on the plated surface (plates sit at -1.5..+3.5 cm)
    bvh = BVHTree.FromObject(hull, bpy.context.evaluated_depsgraph_get())
    for f in kit_faces:
        hit = bvh.ray_cast(f.c + f.n * 0.1, -f.n, 0.2)[0]
        if hit is not None:
            f.c = hit + f.n * 0.001
    greebles = hd.apply_greebles(kit_faces, bm, rnd, 0.16)
    parts = hd.apply_functional(kit_faces, bm, bpy.context.scene.collection, ymin, ymax,
                                navlights=False, antenna=False)
    bm.to_mesh(det.data)
    bm.free()
    # registration text -> mesh, joined into the kit on the Paint slot
    texts = [o for o in bpy.data.objects if o.type == 'FONT']
    for t in texts:
        bpy.ops.object.select_all(action='DESELECT')
        t.select_set(True)
        bpy.context.view_layer.objects.active = t
        bpy.ops.object.convert(target='MESH')
        t.data.materials.clear()
        for name in DETAIL_SLOTS:
            t.data.materials.append(bpy.data.materials[name])
        for p in t.data.polygons:
            p.material_index = hd.PAINT
    bpy.ops.object.select_all(action='DESELECT')
    for t in texts:
        t.select_set(True)
    det.select_set(True)
    bpy.context.view_layer.objects.active = det
    if texts:
        bpy.ops.object.join()
    det = bpy.context.view_layer.objects.active
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    for p in det.data.polygons:
        p.use_smooth = False

    # ---- back to Blender units, clean, export ----
    for o in (hull, det):
        scale_mesh(o, 1.0 / M_PER_BU)
        gen.clean_mesh(o, eps=0.01)
        # flat shading: the merge in clean_mesh drops the bevel's hardened custom
        # normals (smooth blobs on big faces); a flat 45 deg chamfer still catches the light
        if o.data.has_custom_normals:
            gen.sel_activate(o)
            bpy.ops.mesh.customdata_custom_splitnormals_clear()
        for p in o.data.polygons:
            p.use_smooth = False
    tris = {o.name: sum(len(p.vertices) - 2 for p in o.data.polygons) for o in (hull, det)}
    print("HULLDETAIL", {"ship": SHIP, "plates": plates, "glazed_faces": len(glazed), "greebles": greebles,
                         "parts": parts, "hull_tris_before": tris0, "tris": tris,
                         "hull_slots": [m.name for m in hull.data.materials]}, flush=True)
    if DRY:
        return
    if win:
        bpy.data.objects.remove(win, do_unlink=True)
    print("EXPORT", export_fbx(hull, HULL_OUT), flush=True)
    print("EXPORT", export_fbx(det, f"SM_Ship_{SHIP}_Detail"), flush=True)
    hazard_texture(os.path.join(gen.TEXDIR, "T_ShipDetail_Hazard_D.png"))
    print("EXPORT T_ShipDetail_Hazard_D.png", flush=True)


main()
