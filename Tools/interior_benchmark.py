"""Realism benchmark for a walkable ship interior.

Scores an interior 0-100 from two sources, so iterations can be compared by
number (like Tools/texture_benchmark.py does for texture sets):

  geometry  measured in Blender on the EXPORTED FBX parts (what Unreal gets):
            ray casts and nearest-surface queries per zone.
  image     statistics of the in-engine screenshots from
            Tools/pie_capture_interior.py (Lumen, real materials, the C++ lights).

Benchmarks and where the targets come from
------------------------------------------
Space (a ship's volume is expensive: every cubic metre is pressurised, shielded
and hauled). Reference interiors: US Navy passageways ~0.8-1.2 m wide,
submarine passageways 0.6-0.9 m, ISS module cross-section ~2.1 x 2.1 m, naval
deck clear height ~2.1-2.4 m. Games need some slack for a first-person camera
and the r=42 cm avatar capsule, so targets are set somewhat above real ships
(the Alien: Isolation / Dead Space range):
  corridor_width   clear width at hip height          good <= 2.2 m, bad >= 4.0 m
  ceiling          median clear height over the floor good <= 2.8 m, bad >= 4.2 m
                   (command spaces: <= 3.2 / >= 6.5; bays exempt)
  free_span        median free eye-height sightline    good <= 2.5 m, bad >= 7 m
                   (how far you can see before hitting something)
  dead_air         share of the air volume more than  good <= 8 %,  bad >= 45 %
                   1 m from any surface ("empty floor")
Form
  wall_clutter     share of eye-height rays stopped by  good >= 60 %, bad <= 15 %
                   equipment before the room's bounding wall (bare-box walls)
  boxiness         area share of faces aligned to an axis good <= 70 %, bad >= 95 %
                   (sloped hull walls, chamfers, curved pipe runs break it up)
  detail           triangles per m2 of surface          good >= 60,  bad <= 8
Lighting and surfaces (in-engine screenshots)
  dynamic_range    log2(p99.5 / p2) of linear luminance good >= 6,   bad <= 3 stops
  flat_shading     16 px blocks with ~no variation      good <= 10 %, bad >= 45 %
                   (untextured, unlit or unshadowed surfaces)
  clipping         pixels with a channel at 255         good <= 0.5 %, bad >= 5 %
  flat_emissive    bright blocks that are one flat colour good <= 15 %, bad >= 60 %
                   (screens and panels with no content)
  spectrum         1/f^beta slope of the image (natural  |beta-2| good <= 0.25, bad >= 0.9
                   scenes sit near beta = 2; steeper = missing fine detail)
  repetition       strongest off-centre peak of the     good <= 0.15, bad >= 0.45
                   high-passed image autocorrelation
                   (visibly repeating tiles/panels)

Run:
  python Tools/interior_benchmark.py [Battleship] [--regen-geometry]
    -> Saved/InteriorBenchmark/<prefix>_benchmark.{md,json}
  (the geometry pass runs itself: blender -b --python Tools/interior_benchmark.py -- --geometry <Ship>)
"""
import glob
import json
import math
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GEN = os.path.join(ROOT, "Assets", "FBX", "generated")
OUT = os.path.join(ROOT, "Saved", "InteriorBenchmark")
BLENDER = r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"

# Zones in BLENDER design cm (the build script's own space; UE y = -Blender y):
# (name, kind, x0, x1, y0, y1, ceiling_z), from the layout constants in
# Tools/build_battleship_decks.py.
INTERIORS = {
    "Battleship": {
        "prefix": "SM_Int_Battleship_Decks",
        "zones": [
            ("Spine corridor", "corridor", -1100, 1600, -110, 110, 250),
            ("Berths", "room", -1100, -200, 110, 710, 270),
            ("Ready room", "room", -1100, -200, -710, -110, 270),
            ("Mess", "room", -200, 700, 110, 710, 270),
            ("Medbay", "room", -200, 700, -710, -110, 270),
            ("Briefing", "room", 700, 1600, 110, 710, 270),
            ("Armory", "room", 700, 1600, -710, -110, 270),
            ("CIC", "command", 1600, 2800, -500, 500, 330),
            ("Hangar", "bay", -3100, -1100, -1150, 1150, 950),
            ("Engineering", "machinery", -4300, -3100, -800, 800, 900),
        ],
    },
}

# metric: (label, group, good, bad, weight). Linear between bad (0) and good (100).
METRICS = {
    "corridor_width": ("Corridor width (m)", "Space", 2.2, 4.0, 6),
    "ceiling": ("Ceiling height (m)", "Space", None, None, 8),      # per-kind targets below
    "free_span": ("Free sightline at eye height (m)", "Space", 2.5, 7.0, 8),
    "dead_air": ("Dead air >1 m from any surface (%)", "Space", 8.0, 45.0, 10),
    "wall_clutter": ("Eye rays stopped before bare wall (%)", "Form", 60.0, 15.0, 6),
    "boxiness": ("Axis-aligned surface area (%)", "Form", 70.0, 95.0, 5),
    "detail": ("Triangles per m2 of surface", "Form", 60.0, 8.0, 4),
    "dynamic_range": ("Luminance range (stops)", "Light", 6.0, 3.0, 8),
    "flat_shading": ("Flat-shaded image blocks (%)", "Light", 10.0, 45.0, 10),
    "clipping": ("Clipped pixels (%)", "Light", 0.5, 5.0, 5),
    "flat_emissive": ("Flat-colour bright blocks (%)", "Light", 15.0, 60.0, 7),
    "spectrum": ("|1/f slope - 2|", "Light", 0.25, 0.9, 5),
    "repetition": ("Tiling repetition (autocorr. peak)", "Light", 0.15, 0.45, 7),
}
CEILING = {"corridor": (2.8, 4.2), "room": (2.8, 4.2), "command": (3.2, 6.5)}   # bays exempt
DEAD_AIR = {"bay": (35.0, 75.0), "machinery": (15.0, 50.0)}                    # bays may be open


def ship_cfg(ship):
    """Zones for a ship: the table above, else the ones the deck builder wrote
    (Tools/build_ship_decks.py: contract "zones", or <prefix>_zones.json from a dry run)."""
    if ship in INTERIORS:
        return INTERIORS[ship]
    pre = "SM_Int_%s_Decks" % ship
    for path in (os.path.join(OUT, pre + "_zones.json"), os.path.join(GEN, pre + "_contract.json")):
        if os.path.exists(path):
            d = json.load(open(path))
            zones = d if isinstance(d, list) else d.get("zones")
            if zones:
                return {"prefix": pre, "zones": [tuple(z) for z in zones]}
    raise SystemExit("no zones for %s: run blender -b --python Tools/build_ship_decks.py -- %s --dry-run"
                     % (ship, ship))


def lin(v, good, bad):
    if v is None:
        return None
    t = (v - bad) / (good - bad)
    return max(0.0, min(100.0, 100.0 * t))


# ---------------------------------------------------------------------------
# Geometry pass (inside Blender)
# ---------------------------------------------------------------------------
def geometry_pass(ship):
    import bpy
    import bmesh
    from mathutils import Vector
    from mathutils.bvhtree import BVHTree

    cfg = ship_cfg(ship)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for fp in sorted(glob.glob(os.path.join(GEN, cfg["prefix"] + "_*.fbx"))):
        if fp.endswith("_Collision.fbx"):
            continue
        bpy.ops.import_scene.fbx(filepath=fp)
    bm = bmesh.new()
    for o in bpy.data.objects:
        if o.type != 'MESH':
            continue
        me = o.data.copy()
        me.transform(o.matrix_world)
        bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bm.faces.ensure_lookup_table()
    bvh = BVHTree.FromBMesh(bm)

    def V(x, y, z):
        return Vector((x, y, z))

    # loose islands (props, fixtures, panels) by flood fill over shared vertices
    island_of = {}
    centroids = []
    for f in bm.faces:
        if f.index in island_of:
            continue
        stack, n, acc = [f], 0, Vector()
        island_of[f.index] = len(centroids)
        while stack:
            g = stack.pop()
            acc += g.calc_center_median()
            n += 1
            for v in g.verts:
                for h in v.link_faces:
                    if h.index not in island_of:
                        island_of[h.index] = len(centroids)
                        stack.append(h)
        centroids.append(acc / n)

    dirs = [Vector((math.cos(a), math.sin(a), 0.0)) for a in (i * math.pi / 8 for i in range(16))]
    zones = []
    for name, kind, x0, x1, y0, y1, ceil in cfg["zones"]:
        area_m2 = (x1 - x0) * (y1 - y0) / 1e4
        step = 50.0 if area_m2 < 300 else 80.0
        heights, spans, clutter, width = [], [], [0, 0], []
        xs = [x0 + step / 2 + i * step for i in range(int((x1 - x0) // step))]
        ys = [y0 + step / 2 + j * step for j in range(int((y1 - y0) // step))]
        for x in xs:
            for y in ys:
                o = V(x, y, 100.0)
                near = bvh.find_nearest(o)
                if near[0] is None or near[3] < 20.0 or near[1].dot(o - near[0]) < 0:
                    continue   # inside furniture / a wall
                down = bvh.ray_cast(o, Vector((0, 0, -1)), 150.0)
                if down[0] is None:
                    continue   # not over a floor
                up = bvh.ray_cast(o, Vector((0, 0, 1)), 3000.0)
                if up[0] is not None:
                    heights.append((up[3] + 100.0 - (100.0 - down[3])) / 100.0)
                e = V(x, y, 160.0)
                for d in dirs:
                    hit = bvh.ray_cast(e, d, 5000.0)
                    dist = hit[3] if hit[0] is not None else 5000.0
                    spans.append(dist / 100.0)
                    # distance to the zone's bounding wall along d
                    dx, dy = d.x, d.y
                    tb = min([(x1 - x) / dx if dx > 1e-6 else (x0 - x) / dx if dx < -1e-6 else 1e9,
                              (y1 - y) / dy if dy > 1e-6 else (y0 - y) / dy if dy < -1e-6 else 1e9])
                    clutter[1] += 1
                    if dist < tb - 60.0:
                        clutter[0] += 1
                if kind == "corridor":
                    l = bvh.ray_cast(V(x, y, 100.0), Vector((0, -1, 0)), 3000.0)
                    r = bvh.ray_cast(V(x, y, 100.0), Vector((0, 1, 0)), 3000.0)
                    # skip samples looking out through a doorway (short spines are
                    # mostly doors, which pushed the median to the room depth)
                    if l[0] is not None and r[0] is not None and max(l[3], r[3]) < 350.0:
                        width.append((l[3] + r[3]) / 100.0)
        # dead air: voxel centres in open air more than 1 m from every surface
        vstep = 50.0 if area_m2 < 300 else 80.0
        air = dead = 0
        z = 30.0
        while z < ceil - 30.0:
            for x in [x0 + vstep / 2 + i * vstep for i in range(int((x1 - x0) // vstep))]:
                for y in [y0 + vstep / 2 + j * vstep for j in range(int((y1 - y0) // vstep))]:
                    p = V(x, y, z)
                    loc, nrm, _i, dist = bvh.find_nearest(p, 400.0)
                    if loc is None:
                        air += 1
                        dead += 1
                    elif nrm.dot(p - loc) > 0:
                        air += 1
                        dead += dist > 100.0
            z += vstep
        # surface stats of faces whose centre lies in the zone
        axis_a = tot_a = 0.0
        tris = 0
        isl = set()
        for f in bm.faces:
            c = f.calc_center_median()
            if not (x0 <= c.x <= x1 and y0 <= c.y <= y1 and -25.0 <= c.z <= ceil + 25.0):
                continue
            a = f.calc_area()
            n = f.normal
            tot_a += a
            tris += 1
            if max(abs(n.x), abs(n.y), abs(n.z)) > 0.999:
                axis_a += a
            isl.add(island_of[f.index])

        def med(v):
            v = sorted(v)
            return v[len(v) // 2] if v else None
        zones.append({
            "name": name, "kind": kind, "floor_m2": round(area_m2, 1),
            "footprint_m": [round((x1 - x0) / 100, 1), round((y1 - y0) / 100, 1)],
            "ceiling": med(heights), "free_span": med(spans),
            "corridor_width": med(width) if width else None,
            "dead_air": 100.0 * dead / max(1, air),
            "wall_clutter": 100.0 * clutter[0] / max(1, clutter[1]),
            "boxiness": 100.0 * axis_a / max(1e-6, tot_a),
            "detail": tris / max(1e-6, tot_a / 1e4),
        })
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, cfg["prefix"] + "_geometry.json"), "w") as fh:
        json.dump(zones, fh, indent=1)


# ---------------------------------------------------------------------------
# Image pass (plain python)
# ---------------------------------------------------------------------------
def image_metrics(path):
    import numpy as np
    from PIL import Image
    im = np.asarray(Image.open(path).convert("RGB")).astype(np.float32) / 255.0
    lum_s = im @ np.array([0.2126, 0.7152, 0.0722], np.float32)
    lin_rgb = np.where(im <= 0.04045, im / 12.92, ((im + 0.055) / 1.055) ** 2.4)
    lum = lin_rgb @ np.array([0.2126, 0.7152, 0.0722], np.float32)
    lo, hi = np.percentile(lum, 2), np.percentile(lum, 99.5)
    dr = math.log2(max(hi, 1e-4) / max(lo, 1.0 / 1024))
    B = 16
    h, w = (lum_s.shape[0] // B) * B, (lum_s.shape[1] // B) * B
    blk = lum_s[:h, :w].reshape(h // B, B, w // B, B).swapaxes(1, 2).reshape(h // B, w // B, -1)
    bstd, bmean = blk.std(-1), blk.mean(-1)
    lit = bmean > 0.04
    flat = float(((bstd < 0.008) & lit).sum() / max(1, lit.sum()))
    bright = bmean > 0.45
    flat_em = float(((bstd < 0.015) & bright).sum() / max(1, bright.sum())) if bright.sum() > 20 else 0.0
    clip = float((im.max(-1) >= 0.995).mean())
    # radially averaged power spectrum slope over mid frequencies
    g = lum_s[:h, :w] - lum_s[:h, :w].mean()
    g = g * np.outer(np.hanning(g.shape[0]), np.hanning(g.shape[1]))
    P = np.abs(np.fft.fftshift(np.fft.fft2(g))) ** 2
    cy, cx = P.shape[0] // 2, P.shape[1] // 2
    yy, xx = np.indices(P.shape)
    r = np.hypot(yy - cy, xx - cx).astype(int)
    radial = np.bincount(r.ravel(), P.ravel()) / np.maximum(1, np.bincount(r.ravel()))
    f = np.arange(len(radial))
    sel = (f >= 4) & (f <= min(cy, cx) // 2)
    beta = -np.polyfit(np.log(f[sel]), np.log(radial[sel] + 1e-12), 1)[0]
    # repetition: autocorrelation of the high-passed luminance, ignoring the
    # central lobe (a pixel always matches itself and its neighbours)
    from PIL import ImageFilter
    small = Image.fromarray((lum_s * 255).astype(np.uint8)).resize((lum_s.shape[1] // 2, lum_s.shape[0] // 2))
    hp = np.asarray(small, np.float32) - np.asarray(small.filter(ImageFilter.GaussianBlur(6)), np.float32)
    hp -= hp.mean()
    F = np.fft.fft2(hp, s=(hp.shape[0] * 2, hp.shape[1] * 2))
    ac = np.fft.fftshift(np.real(np.fft.ifft2(F * np.conj(F))))
    ac /= max(float(ac.max()), 1e-9)   # an all-black frame has no autocorrelation
    cy2, cx2 = ac.shape[0] // 2, ac.shape[1] // 2
    yy2, xx2 = np.indices(ac.shape)
    rr = np.hypot(yy2 - cy2, xx2 - cx2)
    rep = float(ac[(rr > 12) & (rr < min(cy2, cx2) * 0.5)].max())
    return {"repetition": rep, "dynamic_range": dr, "flat_shading": 100 * flat, "flat_emissive": 100 * flat_em,
            "clipping": 100 * clip, "spectrum": abs(beta - 2.0), "beta": float(beta)}


# ---------------------------------------------------------------------------
# Scoring + report
# ---------------------------------------------------------------------------
def score_zone(z):
    s = {}
    for k in ("free_span", "wall_clutter", "boxiness", "detail"):
        _l, _g, good, bad, _w = METRICS[k]
        s[k] = lin(z[k], good, bad)
    good, bad = DEAD_AIR.get(z["kind"], (METRICS["dead_air"][2], METRICS["dead_air"][3]))
    s["dead_air"] = lin(z["dead_air"], good, bad)
    if z["kind"] in CEILING and z["ceiling"]:
        s["ceiling"] = lin(z["ceiling"], *CEILING[z["kind"]])
    if z["kind"] in ("bay", "machinery"):
        s["free_span"] = None          # big open bays are legitimately long sightlines
    if z.get("corridor_width"):
        w = z["corridor_width"]
        s["corridor_width"] = 0.0 if w < 1.2 else lin(w, METRICS["corridor_width"][2], METRICS["corridor_width"][3])
    return s


def weighted(scores):
    num = den = 0.0
    for k, v in scores.items():
        if v is not None:
            num += METRICS[k][4] * v
            den += METRICS[k][4]
    return num / den if den else None


def grade(v):
    return "A" if v >= 85 else "B" if v >= 70 else "C" if v >= 55 else "D" if v >= 40 else "F"


def main():
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
    if "--geometry" in args:
        geometry_pass(args[args.index("--geometry") + 1])
        return
    if "--all" in args:
        return summary([a for a in args if not a.startswith("--")], "--regen-geometry" in args)
    ship = next((a for a in args if not a.startswith("--")), "Battleship")
    cfg = ship_cfg(ship)
    pre = cfg["prefix"]
    gpath = os.path.join(OUT, pre + "_geometry.json")
    if "--regen-geometry" in args or not os.path.exists(gpath):
        subprocess.run([BLENDER, "-b", "--factory-startup", "--python", os.path.abspath(__file__),
                        "--", "--geometry", ship], check=True, stdout=subprocess.DEVNULL)
    zones = json.load(open(gpath))
    for z in zones:
        z["scores"] = score_zone(z)
        z["score"] = weighted(z["scores"])

    shots = sorted(glob.glob(os.path.join(OUT, pre + "_ue_*.png")))
    views = {}
    for p in shots:
        m = image_metrics(p)
        tag = os.path.basename(p)[len(pre) + 4:-4]
        views[tag] = {"metrics": m, "scores": {k: lin(m[k], METRICS[k][2], METRICS[k][3])
                                                for k in ("dynamic_range", "flat_shading", "clipping",
                                                          "flat_emissive", "spectrum", "repetition")}}
        views[tag]["score"] = weighted(views[tag]["scores"])

    # overall: every metric averaged over zones (floor-area weighted) / views, then weighted
    overall = {}
    for k in METRICS:
        vals = [(z["scores"].get(k), z["floor_m2"]) for z in zones if z["scores"].get(k) is not None]
        vals += [(v["scores"][k], 1.0) for v in views.values() if k in v["scores"]]
        if vals:
            overall[k] = sum(a * b for a, b in vals) / sum(b for _a, b in vals)
    groups = {}
    for k, v in overall.items():
        groups.setdefault(METRICS[k][1], {})[k] = v
    total = weighted(overall)

    L = ["# Interior realism benchmark: %s" % pre, "",
         "**Overall %.0f / 100 (%s)**   " % (total, grade(total)) +
         "  ".join("%s %.0f" % (g, weighted(v)) for g, v in groups.items()), "",
         "Metric scores (0 = at the 'bad' reference, 100 = at the 'good' one; targets in the script docstring):", "",
         "| Metric | Group | Score |", "|---|---|---|"]
    for k, v in sorted(overall.items(), key=lambda kv: kv[1]):
        L.append("| %s | %s | %.0f |" % (METRICS[k][0], METRICS[k][1], v))
    L += ["", "## Zones (geometry)", "",
          "| Zone | Footprint (m) | Ceiling (m) | Corridor w (m) | Sightline (m) | Dead air % | Rays hit clutter % | Axis-aligned % | Tris/m2 | Score |",
          "|---|---|---|---|---|---|---|---|---|---|"]
    fmt = lambda v, f="%.1f": "-" if v is None else f % v
    for z in zones:
        L.append("| %s | %s x %s | %s | %s | %s | %s | %s | %s | %s | %s |" % (
            z["name"], z["footprint_m"][0], z["footprint_m"][1], fmt(z["ceiling"]), fmt(z["corridor_width"]),
            fmt(z["free_span"]), fmt(z["dead_air"], "%.0f"), fmt(z["wall_clutter"], "%.0f"),
            fmt(z["boxiness"], "%.0f"), fmt(z["detail"], "%.0f"), fmt(z["score"], "%.0f")))
    L += ["", "## Views (in-engine)", "",
          "| View | Range (stops) | Flat blocks % | Flat bright % | Clipped % | 1/f beta | Repetition | Score |",
          "|---|---|---|---|---|---|---|---|"]
    for t, v in views.items():
        m = v["metrics"]
        L.append("| %s | %.1f | %.0f | %.0f | %.2f | %.2f | %.2f | %.0f |" % (
            t, m["dynamic_range"], m["flat_shading"], m["flat_emissive"], m["clipping"], m["beta"],
            m["repetition"], v["score"]))
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, pre + "_benchmark.md"), "w") as fh:
        fh.write("\n".join(L) + "\n")
    with open(os.path.join(OUT, pre + "_benchmark.json"), "w") as fh:
        json.dump({"overall": total, "groups": {g: weighted(v) for g, v in groups.items()},
                   "metrics": overall, "zones": zones, "views": views}, fh, indent=1)
    print("\n".join(L))


def summary(ships, regen):
    """Geometry-only scores for many decks (no screenshots needed)."""
    if not ships:
        ships = ["Battleship"] + sorted(os.path.basename(p)[7:-len("_Decks_contract.json")]
                                        for p in glob.glob(os.path.join(GEN, "SM_Int_*_Decks_contract.json"))
                                        if "Battleship" not in p)
    rows = []
    for ship in ships:
        cfg = ship_cfg(ship)
        gpath = os.path.join(OUT, cfg["prefix"] + "_geometry.json")
        if regen or not os.path.exists(gpath):
            subprocess.run([BLENDER, "-b", "--factory-startup", "--python", os.path.abspath(__file__),
                            "--", "--geometry", ship], check=True, stdout=subprocess.DEVNULL)
        zones = json.load(open(gpath))
        sc = {}
        for z in zones:
            for k, v in score_zone(z).items():
                if v is not None:
                    sc.setdefault(k, []).append((v, z["floor_m2"]))
        m = {k: sum(a * b for a, b in v) / sum(b for _a, b in v) for k, v in sc.items()}
        grp = {}
        for k, v in m.items():
            grp.setdefault(METRICS[k][1], {})[k] = v
        rooms = [z for z in zones if z["kind"] in ("room", "corridor", "command")]
        worst = max(rooms, key=lambda z: z["dead_air"]) if rooms else None
        rows.append((ship, weighted(m), weighted(grp.get("Space", {})), weighted(grp.get("Form", {})),
                     max((z["ceiling"] or 0) for z in rooms) if rooms else 0,
                     max((z["corridor_width"] or 0) for z in zones),
                     "%s %.0f%%" % (worst["name"], worst["dead_air"]) if worst else "-"))
    L = ["| Ship | Geometry score | Space | Form | Max room ceiling (m) | Widest corridor (m) | Emptiest room (dead air) |",
         "|---|---|---|---|---|---|---|"]
    for r in rows:
        L.append("| %s | %.0f | %.0f | %.0f | %.1f | %.1f | %s |" % r)
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, "decks_summary.md"), "w") as fh:
        fh.write("\n".join(L) + "\n")
    print("\n".join(L))


if __name__ == "__main__":
    main()
