"""Ship windows as separate geometry (user, 2026-09-28: windows are not part of the
tiling hull texture any more -- see HULL_WINDOWS in generate_adastrea_assets.py).

For each ship hull (SM_Ship_<X>_01_Assembled.fbx) this raycasts window strips onto
the flat, near-vertical side plating at human deck heights and exports them as
SM_Ship_<X>_01_Windows.fbx in the SAME object space and with the same FBX settings
as the hull, so in Unreal the window mesh attaches to the hull mesh at identity
(ASpaceship does this automatically when a matching _Windows mesh exists).

Each window = a glass quad plus a thin raised frame ring. Material slots (bound
by name on import, see Tools/ue_import_ship_windows.py):
  M_ShipWindow_LitCool, M_ShipWindow_LitWarm   emissive glass (lit cabins)
  M_ShipWindow_Dark                            dark glossy glass (unlit cabins)
  M_ShipWindow_Frame                           dark metal frame
Layout is port/starboard symmetric and grouped in runs with gaps, so it reads as
decks of cabins rather than a regular grid.

Sizes are real metres, converted per hull with the in-game length the ship's
Blueprint gives it (SHIPS below: hull mesh scale x mesh bounds, read from UE).

Run: blender -b --python Tools/build_ship_windows.py -- [--ships Battleship,Cruiser] [--ue-hulls DIR]
                                                        [--preview DIR] [--dry-run]
  --ue-hulls DIR  folder from `ue_import_ship_windows.py --export-hulls DIR` (needed for UE_SOURCE ships)
"""
import math
import os
import sys

import bmesh
import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen  # noqa: E402

# hull key -> in-game length in metres (BP ShipMesh scale x mesh bounds, 2026-09-28).
# A hull used by several Blueprints is sized for its main one (the Battleship mesh
# is also BP_Ship_Luxury at a third of the size).
SHIPS = {
    "Battleship": 28.0,      # BP_Battleship (unique-UV variant of this hull)
    "Cruiser": 22.6,
    # Freighter (17.8 m): the in-game hull is stacked cargo containers - no cabins
    # Destroyer (15.7 m): wedge hull; the only flat side plating is the keel fin
    # Corvette (13 m): its unique baked hull texture already carries lit windows
    "BulkCarrier": 9.5,
    "Miner": 9.5,
    "HeavyHauler": 9.5,
    "Trader": 9.5,
    "CargoFreighter": 9.5,
    "Smuggler": 9.5,
    "Escort": 9.5,
    "Courier": 9.5,
    # Fighter (7.4 m): cockpit canopy only, no cabin windows
}
MIN_LENGTH_M = 8.0

WIN_W, WIN_H, GAP = 1.0, 0.4, 0.45        # metres
DECK_H = 2.8
FRAME = 0.04                               # frame ring width
GLASS_OFF, FRAME_OFF = 0.012, 0.022        # offset out of the hull surface
SIDE_DOT = 0.80                            # |normal . side axis| for a clear line of sight hit
FACE_DOT = 0.85                            # plating must point this much sideways to take windows
STEP_TOL = 0.06                            # a depth jump > 6 cm between 5 cm cells = a step/greeble edge (panel-line grooves are smaller)
EDGE_MARGIN = 0.05                         # clear plating around the frame (m)
GRID_M = 0.05                              # side depth-grid resolution (m)
LIT_FRAC, WARM_FRAC, DROP_FRAC = 0.65, 0.30, 0.18
ROW_SEP = 1.2                              # min vertical spacing between window rows (m)
MIN_ROW = 3                                # a row needs at least this many windows (both sides)
MATS = ["M_ShipWindow_LitCool", "M_ShipWindow_LitWarm", "M_ShipWindow_Dark", "M_ShipWindow_Frame"]


def argv():
    a = sys.argv
    return a[a.index("--") + 1:] if "--" in a else []


# Hulls whose Unreal asset was imported from an older export of the same FBX path
# (the file on disk is a different design now). For these the windows are placed on
# the mesh exported back out of Unreal (ue_import_ship_windows.py --export-hulls),
# so they match what the game renders. Round trip: UE export -> Blender import is
# (x, -y, z) / 100 and gen.export_fbx -> UE import is the exact inverse.
UE_SOURCE = {
    "Cruiser": "SM_Ship_Cruiser_01_Assembled.fbx",
    "Destroyer": "SM_Ship_Destroyer_01_Assembled.fbx",
    "Freighter": "SM_Ship_Freighter_01_Assembled.fbx",
    "Corvette": "SM_Ship_Corvette_01_Assembled_UniqueUV.fbx",
}


def import_hull(key, ue_dir=None):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    gen.setup_scene()
    if key in UE_SOURCE:
        if not ue_dir:
            raise SystemExit(f"{key}: needs --ue-hulls DIR (hull exported from Unreal)")
        path = os.path.join(ue_dir, UE_SOURCE[key])
    else:
        path = os.path.join(gen.BASE, f"SM_Ship_{key}_01_Assembled.fbx")
    bpy.ops.import_scene.fbx(filepath=path, axis_forward='-Y', axis_up='Z')
    for o in list(bpy.data.objects):
        if o.name.startswith("UCX_"):           # collision hulls from a UE export
            bpy.data.objects.remove(o)
    return [o for o in bpy.data.objects if o.type == 'MESH'][0]


def local_bvh(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    tree = BVHTree.FromBMesh(bm)
    bm.free()
    return tree


def side_depth(tree, side, lo, hi, cell, reach):
    """Ray-cast the ship's visible side into a (z, y) grid: hit depth (x) and |n.x|."""
    ys = np.arange(lo.y + cell / 2, hi.y, cell)
    zs = np.arange(lo.z + cell / 2, hi.z, cell)
    depth = np.full((len(zs), len(ys)), np.nan, np.float32)
    facing = np.zeros((len(zs), len(ys)), np.float32)
    normal = np.zeros((len(zs), len(ys), 3), np.float32)
    d = Vector((-side, 0, 0))
    x_out = (hi.x if side > 0 else lo.x) + side * reach
    for iz, z in enumerate(zs):
        for iy, y in enumerate(ys):
            loc, n, _, _ = tree.ray_cast(Vector((x_out, y, z)), d)
            if loc is not None:
                depth[iz, iy] = loc.x
                facing[iz, iy] = n.x * side
                normal[iz, iy] = n
    return ys, zs, depth, facing, normal


def rect_ok(mask, kh, kw):
    """True at (z, y) where the kh x kw rectangle centred there is all True (integral image)."""
    ii = np.zeros((mask.shape[0] + 1, mask.shape[1] + 1), np.int32)
    ii[1:, 1:] = np.cumsum(np.cumsum(mask.astype(np.int32), 0), 1)
    out = np.zeros_like(mask)
    h2, w2 = kh // 2, kw // 2
    z0, z1 = h2, mask.shape[0] - (kh - h2)
    y0, y1 = w2, mask.shape[1] - (kw - w2)
    if z1 <= z0 or y1 <= y0:
        return out
    zz = np.arange(z0, z1)[:, None]
    yy = np.arange(y0, y1)[None, :]
    a, b = zz - h2, yy - w2
    tot = ii[a + kh, b + kw] - ii[a, b + kw] - ii[a + kh, b] + ii[a, b]
    out[z0:z1, y0:y1] = tot == kh * kw
    return out


def place_windows(ob, length_m, seed):
    """Return windows [(centre, normal, u, v, kind)] in hull-local BU.

    The hulls are kit-bashed from overlapping boxes, so exposed flat plating is
    patchy. Each side is ray-cast into a 5 cm depth grid; a window (plus frame
    and a margin) may sit where its whole footprint is side-facing and on one
    depth plane (nothing in front, no step under it). Deck rows are then the
    heights where the most windows fit, >= ROW_SEP apart, shared by both sides.
    """
    tree = local_bvh(ob)
    bb = [Vector(c) for c in ob.bound_box]
    lo = Vector((min(c.x for c in bb), min(c.y for c in bb), min(c.z for c in bb)))
    hi = Vector((max(c.x for c in bb), max(c.y for c in bb), max(c.z for c in bb)))
    bu_per_m = (hi.y - lo.y) / length_m
    k = min(1.0, max(0.6, length_m / 20.0))         # smaller windows on small ships
    w, h, gap, f = WIN_W * k * bu_per_m, WIN_H * k * bu_per_m, GAP * k * bu_per_m, FRAME * bu_per_m
    cell = GRID_M * bu_per_m
    margin = EDGE_MARGIN * k * bu_per_m
    kw = int(math.ceil((w + 2 * f + 2 * margin) / cell))
    kh = int(math.ceil((h + 2 * f + 2 * margin) / cell))
    pitch = int(math.ceil((w + gap) / cell))
    y_lo = lo.y + 0.10 * (hi.y - lo.y)
    y_hi = hi.y - 0.08 * (hi.y - lo.y)

    sides = {}
    step = STEP_TOL * bu_per_m
    for side in (1, -1):
        ys, zs, depth, facing, normal = side_depth(tree, side, lo, hi, cell, 10.0 * bu_per_m)
        # smooth plating: side-facing and no depth step to any neighbour (slabs are
        # slightly tilted, so compare neighbours rather than one constant depth)
        d = np.where(np.isnan(depth), np.inf if side > 0 else -np.inf, depth)
        jump = np.zeros(depth.shape, bool)
        dy = np.abs(np.diff(d, axis=1)) > step
        dz = np.abs(np.diff(d, axis=0)) > step
        jump[:, 1:] |= dy
        jump[:, :-1] |= dy
        jump[1:, :] |= dz
        jump[:-1, :] |= dz
        good = (facing > FACE_DOT) & ~np.isnan(depth) & ~jump
        ok = rect_ok(good, kh, kw)
        ok &= ((ys >= y_lo) & (ys <= y_hi))[None, :]
        sides[side] = (ys, zs, ok, depth, normal)

    def pack(side, iz):
        """Greedy left-to-right packing of windows along one grid row."""
        ok = sides[side][2]
        row, out, iy = ok[iz], [], 0
        while iy < len(row):
            if row[iy]:
                out.append(iy)
                iy += pitch
            else:
                iy += 1
        return out

    zs = sides[1][1]
    score = [(len(pack(1, iz)) + len(pack(-1, iz)), iz) for iz in range(len(zs))]
    score.sort(key=lambda t: -t[0])
    rows = []
    for cnt, iz in score:
        if cnt < MIN_ROW:
            break
        if all(abs(zs[iz] - zs[r]) >= ROW_SEP * bu_per_m for r in rows):
            rows.append(iz)

    out = []
    for iz in sorted(rows):
        for side in (1, -1):
            ys, zs_s, ok, depth, normal = sides[side]
            rr = np.random.default_rng(seed * 7919 + iz)   # same pattern per row on both sides
            run_kind, prev = None, None
            for iy in pack(side, iz):
                if prev is None or iy - prev > pitch * 1.5:
                    run_kind = None                   # a gap in the plating starts a new run
                prev = iy
                if rr.random() < DROP_FRAC:
                    run_kind = None
                    continue
                if run_kind is None or rr.random() < 0.18:   # new run: re-roll lit/warm
                    lit = rr.random() < LIT_FRAC
                    run_kind = (1 if rr.random() < WARM_FRAC else 0) if lit else 2
                # seat on the outermost point of the footprint: bridges panel grooves,
                # never sinks into a raised plate
                hz, hw = kh // 2, kw // 2
                patch = depth[max(0, iz - hz):iz + hz + 1, max(0, iy - hw):iy + hw + 1]
                seat = float(np.nanmax(patch) if side > 0 else np.nanmin(patch))
                c = Vector((seat, float(ys[iy]), float(zs_s[iz])))
                n = Vector(normal[iz, iy].tolist()).normalized()
                u = (Vector((0, 1, 0)) - n * n.y).normalized()
                v = n.cross(u).normalized()
                if v.z < 0:
                    v = -v
                out.append((c, n, u, v, run_kind))
    return out, bu_per_m, k


def build_mesh(ob, wins, bu_per_m, name, k=1.0):
    me = bpy.data.meshes.new(name)
    wo = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(wo)
    wo.matrix_world = ob.matrix_world.copy()
    for mn in MATS:
        m = bpy.data.materials.get(mn) or bpy.data.materials.new(mn)
        me.materials.append(m)
    bm = bmesh.new()
    w, h, f = WIN_W * k * bu_per_m, WIN_H * k * bu_per_m, FRAME * bu_per_m

    def quad(c, u, v, hw, hh, mat):
        vs = [bm.verts.new(c + u * sx * hw + v * sy * hh) for sx, sy in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
        face = bm.faces.new(vs)
        face.material_index = mat
        return face

    for c, n, u, v, kind in wins:
        g = c + n * GLASS_OFF * bu_per_m
        quad(g, u, v, w / 2, h / 2, kind)
        # frame ring: 4 thin quads around the glass, a touch further out
        fc = c + n * FRAME_OFF * bu_per_m
        quad(fc + v * (h / 2 + f / 2), u, v, w / 2 + f, f / 2, 3)
        quad(fc - v * (h / 2 + f / 2), u, v, w / 2 + f, f / 2, 3)
        quad(fc + u * (w / 2 + f / 2), u, v, f / 2, h / 2, 3)
        quad(fc - u * (w / 2 + f / 2), u, v, f / 2, h / 2, 3)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()
    # face normals must point out of the hull (recalc can't know "out" for open quads)
    for p, (c, n, u, v, kind) in zip(me.polygons, [x for x in wins for _ in range(5)]):
        if p.normal.dot(n) < 0:
            p.flip()
    uv = me.uv_layers.new(name="UVMap")
    for poly in me.polygons:
        for li, (s, t) in zip(poly.loop_indices, ((0, 0), (1, 0), (1, 1), (0, 1))):
            uv.data[li].uv = (s, t)
    me.update()
    return wo


def preview(ob, wo, path):
    sc = bpy.context.scene
    hull_mat = bpy.data.materials.new("PrevHull")
    hull_mat.use_nodes = True
    bs = hull_mat.node_tree.nodes["Principled BSDF"]
    bs.inputs["Base Color"].default_value = (0.25, 0.27, 0.30, 1)
    bs.inputs["Roughness"].default_value = 0.4
    ob.data.materials.clear()
    ob.data.materials.append(hull_mat)
    cols = {"M_ShipWindow_LitCool": (0.45, 0.75, 1.0), "M_ShipWindow_LitWarm": (1.0, 0.7, 0.35)}
    for m in wo.data.materials:
        m.use_nodes = True
        b = m.node_tree.nodes["Principled BSDF"]
        if m.name in cols:
            b.inputs["Emission Color"].default_value = (*cols[m.name], 1)
            b.inputs["Emission Strength"].default_value = 6
            b.inputs["Base Color"].default_value = (0.05, 0.05, 0.06, 1)
        elif m.name == "M_ShipWindow_Dark":
            b.inputs["Base Color"].default_value = (0.02, 0.025, 0.03, 1)
            b.inputs["Roughness"].default_value = 0.05
        else:
            b.inputs["Base Color"].default_value = (0.08, 0.08, 0.09, 1)
            b.inputs["Metallic"].default_value = 0.0
    bb = [ob.matrix_world @ Vector(c) for c in ob.bound_box]
    ctr = sum(bb, Vector()) / 8
    size = max((max(p[i] for p in bb) - min(p[i] for p in bb)) for i in range(3))
    cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam"))
    sc.collection.objects.link(cam)
    sc.camera = cam
    cam.location = ctr + Vector((size * 1.05, -size * 0.55, size * 0.35))
    cam.rotation_euler = (ctr - cam.location).to_track_quat('-Z', 'Y').to_euler()
    cam.data.lens = 50
    cam.data.clip_start = size * 0.001
    cam.data.clip_end = size * 20
    sun = bpy.data.objects.new("Sun", bpy.data.lights.new("Sun", "SUN"))
    sun.data.energy = 2.5
    sun.rotation_euler = (math.radians(55), 0, math.radians(35))
    sc.collection.objects.link(sun)
    w = bpy.data.worlds.new("W")
    sc.world = w
    w.use_nodes = True
    w.node_tree.nodes["Background"].inputs[0].default_value = (0.01, 0.012, 0.018, 1)
    sc.render.engine = "CYCLES"
    sc.cycles.samples = 24
    sc.cycles.use_denoising = True
    sc.render.resolution_x, sc.render.resolution_y = 900, 520
    sc.render.filepath = path
    bpy.ops.render.render(write_still=True)
    # straight side elevation (ortho) -- shows where the deck rows landed
    cam.data.type = 'ORTHO'
    cam.data.ortho_scale = size * 1.1
    cam.location = ctr + Vector((size * 2, 0, 0))
    cam.rotation_euler = (math.radians(90), 0, math.radians(90))
    sc.render.filepath = path.replace(".png", "_side.png")
    bpy.ops.render.render(write_still=True)


def main():
    a = argv()
    keys = a[a.index("--ships") + 1].split(",") if "--ships" in a else list(SHIPS)
    prev = a[a.index("--preview") + 1] if "--preview" in a else None
    dry = "--dry-run" in a
    ue_dir = a[a.index("--ue-hulls") + 1] if "--ue-hulls" in a else None
    for i, key in enumerate(keys):
        length = SHIPS[key]
        if length < MIN_LENGTH_M:
            print("WINDOWS skip", key)
            continue
        if key in UE_SOURCE and not ue_dir:
            print("WINDOWS skip", key, "(needs --ue-hulls)")
            continue
        ob = import_hull(key, ue_dir)
        wins, bu_per_m, k = place_windows(ob, length, seed=1000 + i)
        name = f"SM_Ship_{key}_01_Windows"
        print(f"WINDOWS {key}: {len(wins)} windows ({length} m, {bu_per_m:.1f} BU/m)", flush=True)
        if not wins:
            continue
        wo = build_mesh(ob, wins, bu_per_m, name, k)
        if prev:
            os.makedirs(prev, exist_ok=True)
            preview(ob, wo, os.path.join(prev, f"{key}.png"))
        if not dry:
            print("  ->", gen.export_fbx(wo, name))


main()
