"""
Adastrea hero-ship generator: Fighter / Corvette / Freighter / Cruiser / Destroyer
=================================================================================
Replaces the old "one blocky kit at five scales" ships with five genuinely
different silhouettes, each designed from its NAME:

  Fighter   - needle/dart fuselage, swept-back delta wings, bubble canopy,
              twin engine nacelles, wing-mounted cannons, V-tail.
  Corvette  - long slim spine, HAMMERHEAD bridge block, side sponsons with
              gun pods, three-engine cluster.
  Destroyer - flat, angular blade/arrowhead wedge, ventral + dorsal knife
              fins, forward-heavy gun rails along the nose, 5-nozzle bank.
              No tower.
  Freighter - long structural spine carrying a comb of cargo containers, tiny
              forward cockpit, huge rear engine block with 2x2 nozzles.
  Cruiser   - layered "wedding-cake" hull, tall command tower with wide
              bridge, two broadside batteries of 5 turrets, 7-nozzle stern.

Pipeline conventions are the same as Tools/generate_adastrea_assets.py, whose
helpers (setup_scene, smart_uv, emissive_mat, wire_pbr_material) are reused:
  - authored in Blender units exactly like the legacy ship meshes (the legacy
    generator's numbers are "cm-looking" but the scene has scale_length 1.0, so
    the FBX exporter multiplies by 100 -> Unreal uu; Blueprints compensate with
    a ShipMesh scale of ~0.015 (fighter) ... ~0.006 (corvette)),
  - nose along +Y (same orientation as the legacy meshes), Z up, symmetric in X,
  - transforms applied (loc only carries the part pivot), single UV0 (triplanar
    smart_uv), quad-dominant lofted / prism hard-surface,
  - per-part FBX (Carcass/Engine/Weapon/Sensor/Reactor/Cargo/Nav_*) plus
    <ship>_Assembled.fbx, plus <ship>_hardpoints.json.

Output goes to BASE (hard-coded by the shared generator to the MAIN checkout's
Assets/FBX/generated -- the folder the Unreal editor is bound to).

Usage (headless):
  blender -b --python Tools/generate_hero_ships.py -- [Fighter Corvette ...]
  (no names = all five)   add --dry-run to build but not write FBX.
"""
import os, sys, math, json, random, importlib.util
import bpy, bmesh
from mathutils import Vector, Euler

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "generate_adastrea_assets", os.path.join(HERE, "generate_adastrea_assets.py"))
G = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(G)          # defines helpers only; main() is not run

DRY_RUN = False


# ----------------------------------------------------------------------------
# Geometry kit: primitives return lists of polygons (coordinate tuples); a Kit
# collects them per part and turns them into one mesh.
# ----------------------------------------------------------------------------
def _xf(polys, pos=(0, 0, 0), rot=(0, 0, 0)):
    e = Euler([math.radians(a) for a in rot], 'XYZ')
    m = e.to_matrix()
    px, py, pz = pos
    out = []
    for p in polys:
        q = []
        for v in p:
            w = m @ Vector(v)
            q.append((w.x + px, w.y + py, w.z + pz))
        out.append(q)
    return out


def _ring(y, hw, hh, zc, xc, N, sq):
    pts = []
    for i in range(N):
        t = 2 * math.pi * (i + 0.5) / N
        c, s = math.cos(t), math.sin(t)
        x = hw * math.copysign(abs(c) ** (2.0 / sq), c)
        z = hh * math.copysign(abs(s) ** (2.0 / sq), s)
        pts.append((xc + x, y, zc + z))
    return pts


def loft(stations, N=12, sq=2.4, cap=True):
    """Fuselage loft along Y. stations: (y, halfwidth, halfheight, zc[, xc]).
    sq = superellipse exponent (2 round, 4 boxy)."""
    rings = []
    for s in stations:
        y, hw, hh, zc = s[:4]
        xc = s[4] if len(s) > 4 else 0.0
        rings.append(_ring(y, max(hw, 0.4), max(hh, 0.4), zc, xc, N, sq))
    polys = []
    for a, b in zip(rings[:-1], rings[1:]):
        for i in range(N):
            j = (i + 1) % N
            polys.append([a[i], a[j], b[j], b[i]])
    if cap:
        polys.append(list(reversed(rings[0])))
        polys.append(list(rings[-1]))
    return polys


def tube(p0, p1, r0, r1=None, N=12, cap=True):
    """Cylinder / cone / flared bell between two points."""
    r1 = r0 if r1 is None else r1
    p0, p1 = Vector(p0), Vector(p1)
    d = (p1 - p0)
    d.normalize()
    ref = Vector((0, 0, 1)) if abs(d.z) < 0.9 else Vector((1, 0, 0))
    u = d.cross(ref); u.normalize()
    v = d.cross(u); v.normalize()

    def ring(p, r):
        out = []
        for i in range(N):
            t = 2 * math.pi * (i + 0.5) / N
            w = p + u * (r * math.cos(t)) + v * (r * math.sin(t))
            out.append((w.x, w.y, w.z))
        return out
    a, b = ring(p0, r0), ring(p1, r1)
    polys = [[a[i], a[(i + 1) % N], b[(i + 1) % N], b[i]] for i in range(N)]
    if cap:
        polys.append(list(reversed(a)))
        polys.append(list(b))
    return polys


def prism(poly2d, z0, z1, s_top=1.0):
    """Planform (x,y) polygon extruded in Z; top face scaled about centroid."""
    n = len(poly2d)
    cx = sum(p[0] for p in poly2d) / n
    cy = sum(p[1] for p in poly2d) / n
    bot = [(x, y, z0) for x, y in poly2d]
    top = [(cx + (x - cx) * s_top, cy + (y - cy) * s_top, z1) for x, y in poly2d]
    polys = [[bot[i], bot[(i + 1) % n], top[(i + 1) % n], top[i]] for i in range(n)]
    polys.append(list(reversed(bot)))
    polys.append(list(top))
    return polys


def prism_yz(profile, x0, x1):
    """Side profile (y,z) polygon extruded along X (fins, keels, blades)."""
    n = len(profile)
    a = [(x0, y, z) for y, z in profile]
    b = [(x1, y, z) for y, z in profile]
    polys = [[a[i], a[(i + 1) % n], b[(i + 1) % n], b[i]] for i in range(n)]
    polys.append(list(reversed(a)))
    polys.append(list(b))
    return polys


def box(c, s, rot=(0, 0, 0)):
    cx, cy, cz = c
    sx, sy, sz = s
    hx, hy, hz = sx / 2, sy / 2, sz / 2
    polys = prism([(-hx, -hy), (hx, -hy), (hx, hy), (-hx, hy)], -hz, hz)
    return _xf(polys, pos=c, rot=rot)


def sphere_y(c, rx, ry, rz, N=12, rings=6):
    """Ellipsoid (blister / dome / bulb) elongated along Y."""
    st = []
    for i in range(rings + 1):
        t = math.pi * i / rings
        st.append((c[1] - ry * math.cos(t), rx * math.sin(t), rz * math.sin(t), c[2], c[0]))
    return loft(st, N=N, sq=2.0)


class Kit:
    def __init__(self):
        self.groups = []

    def add(self, polys, mirror=False):
        self.groups.append(polys)
        if mirror:
            self.groups.append([[(-x, y, z) for x, y, z in reversed(p)] for p in polys])
        return self

    def empty(self):
        return not self.groups

    def bounds(self):
        pts = [v for g in self.groups for p in g for v in p]
        mn = [min(v[i] for v in pts) for i in range(3)]
        mx = [max(v[i] for v in pts) for i in range(3)]
        return mn, mx


def kits_to_mesh(name, kits):
    bm = bmesh.new()
    for k in kits:
        for g in k.groups:
            vm = {}
            for p in g:
                vs = []
                for v in p:
                    key = (round(v[0], 3), round(v[1], 3), round(v[2], 3))
                    if key not in vm:
                        vm[key] = bm.verts.new(key)
                    vs.append(vm[key])
                # drop degenerate (repeated) verts
                uniq = []
                for x in vs:
                    if x not in uniq:
                        uniq.append(x)
                if len(uniq) < 3:
                    continue
                try:
                    bm.faces.new(uniq)
                except ValueError:
                    pass
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(ob)
    return ob


# ----------------------------------------------------------------------------
# Small shared detail helpers
# ----------------------------------------------------------------------------
def nozzle(kit, x, y, z, r, length):
    """Flared engine bell pointing -Y (exhaust), plus a dark inner throat."""
    kit.add(tube((x, y, z), (x, y - length, z), r, r * 1.35, N=14))
    kit.add(tube((x, y - length, z), (x, y - length - 2, z), r * 1.05, r * 1.05, N=14))


def turret(kit, x, y, z, r, barrel_dir, barrel_len, mirror=False, twin=True, br=None):
    """Turret drum + twin barrels. barrel_dir is a unit-ish (dx,dy,dz)."""
    br = br or r * 0.22
    kit.add(tube((x, y, z), (x, y, z + r * 0.7), r, r * 0.85, N=10), mirror=mirror)
    d = Vector(barrel_dir); d.normalize()
    side = Vector((0, 1, 0)).cross(d) if abs(d.y) < 0.9 else Vector((1, 0, 0))
    side.normalize()
    offs = (-1, 1) if twin else (0,)
    for o in offs:
        base = Vector((x, y, z + r * 0.45)) + side * (o * r * 0.38)
        kit.add(tube(base, base + d * barrel_len, br, br * 0.8, N=8), mirror=mirror)



def interp(pairs, y):
    """Piecewise-linear lookup; pairs = [(y, value)] in any order."""
    pairs = sorted(pairs)
    if y <= pairs[0][0]:
        return pairs[0][1]
    for (y0, v0), (y1, v1) in zip(pairs[:-1], pairs[1:]):
        if y <= y1:
            return v0 + (v1 - v0) * (y - y0) / (y1 - y0 or 1)
    return pairs[-1][1]


def greebles(kit, rng, n, y0, y1, hw_pairs, z_pairs, frac=0.7, skip=None,
             sx=(8, 26), sy=(12, 44), sz=(3, 9)):
    """Deterministic surface detail: small plates/vent boxes scattered on a deck,
    mirrored L/R. hw_pairs/z_pairs are (y, half-width) / (y, deck-height) samples."""
    for _ in range(n):
        y = rng.uniform(y0, y1)
        x = rng.uniform(8, max(9, interp(hw_pairs, y) * frac))
        if skip and skip(x, y):
            continue
        z = interp(z_pairs, y)
        d = (rng.uniform(*sx), rng.uniform(*sy), rng.uniform(*sz))
        kit.add(box((x, y, z + d[2] / 2 - 0.6), d), mirror=True)


# ----------------------------------------------------------------------------
# Ship designs. Each returns dict:
#   parts: {suffix: (Kit, material_name)}, nav: {(name): (x,y,z)}, nav_r: bulb radius
# Nose = +Y. Units = the legacy "Blender units" (see module docstring).
# ----------------------------------------------------------------------------
def design_fighter():
    P = {k: Kit() for k in ("Carcass", "Engine", "Weapon", "Sensor", "Cargo")}
    c = P["Carcass"]
    # needle fuselage, long pointed nose
    c.add(loft([(338, 1.2, 1.2, 0), (300, 6, 6, 1), (230, 18, 15, 2), (140, 32, 24, 4),
                (40, 42, 30, 5), (-90, 44, 30, 4), (-190, 38, 26, 2), (-232, 30, 20, 1)],
               N=12, sq=2.6))
    # bubble canopy
    c.add(sphere_y((0, 62, 30), 19, 60, 17, N=12, rings=7))
    c.add(tube((0, 118, 34), (0, 176, 26), 1.5, 1.5, N=6))          # pitot-ish sensor spike
    # swept-back delta wings (thin, tapered top)
    wing = [(30, 70), (205, -105), (205, -190), (30, -195)]
    c.add(prism(wing, -4, 4, s_top=0.72), mirror=True)
    # forward canards
    c.add(prism([(22, 225), (95, 172), (95, 150), (22, 165)], -2, 2, s_top=0.8), mirror=True)
    # canted V-tail fins
    c.add(_xf(prism_yz([(-128, 26), (-196, 26), (-236, 104), (-206, 104)], -2.5, 2.5), pos=(34, 0, 0), rot=(0, 24, 0)),
          mirror=True)
    # dorsal spine + flank intakes + wing root fairings
    c.add(box((0, -70, 33), (9, 160, 7)))
    c.add(box((40, -20, 2), (16, 80, 22)), mirror=True)
    c.add(box((70, -60, 5), (34, 90, 8)), mirror=True)
    # engines: twin nacelles flanking the tail
    e = P["Engine"]
    e.add(tube((54, -70, 4), (54, -215, 4), 24, 21, N=12), mirror=True)
    e.add(box((0, -195, 4), (100, 46, 22)))                            # cross-beam between nacelles
    for s in (1, -1):
        nozzle(e, 54 * s, -215, 4, 19, 52)
    # wing-mounted weapons: cannon pods with long barrels
    w = P["Weapon"]
    w.add(tube((150, -80, -3), (150, 40, -3), 8, 6.5, N=10), mirror=True)
    w.add(tube((150, 40, -3), (150, 120, -3), 2.8, 2.4, N=8), mirror=True)
    w.add(tube((108, -30, -3), (108, 30, -3), 5, 4, N=8), mirror=True)
    # sensor: dorsal whip + small blade array
    s = P["Sensor"]
    s.add(tube((0, -125, 30), (0, -125, 95), 1.8, 1.2, N=6))
    s.add(box((0, -125, 96), (34, 5, 3)))
    # cargo: small ventral pod
    P["Cargo"].add(loft([(60, 10, 8, -36), (30, 22, 16, -36), (-70, 24, 17, -36), (-100, 12, 9, -34)],
                        N=8, sq=3.0))
    return dict(parts=P, mats={"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_Block",
                               "Sensor": "M_Sensor_Block", "Cargo": "M_Cargo_Hold"},
                nav={"NavGreen": (-205, -110, 2), "NavRed": (205, -110, 2),
                     "NavWhite": (0, -276, 6), "NavBeacon": (0, -150, 78)}, nav_r=4.5)


def design_corvette():
    P = {k: Kit() for k in ("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo")}
    c = P["Carcass"]
    # long slim spine
    c.add(loft([(-560, 50, 44, 0), (-420, 62, 52, 0), (-100, 70, 58, 4), (150, 62, 50, 6),
                (330, 50, 42, 6), (470, 40, 34, 4)], N=12, sq=3.0))
    # HAMMERHEAD: wide flat forward bridge block
    ham = [(-105, 425), (105, 425), (190, 520), (190, 640), (62, 715), (-62, 715), (-190, 640), (-190, 520)]
    c.add(prism(ham, -26, 30, s_top=0.82))
    # command bridge on the hammerhead
    c.add(loft([(430, 44, 18, 56), (475, 64, 30, 60), (570, 58, 32, 64), (640, 40, 24, 62), (665, 18, 12, 60)],
               N=8, sq=3.5))
    c.add(box((0, 545, 100), (70, 90, 12)))                             # bridge roof
    # hammerhead armour strips
    for x in (-120, -60, 60, 120):
        c.add(box((x, 600, 33), (26, 96, 4)))
    rng = random.Random(1101)
    greebles(c, rng, 26, -420, 380, [(-560, 50), (-100, 70), (470, 40)],
             [(-560, 44), (-420, 52), (-100, 62), (150, 56), (330, 48), (470, 38)], frac=0.45,
             skip=lambda x, y: abs(y + 260) < 50, sx=(6, 16), sy=(10, 34), sz=(3, 7))
    # dorsal keel fin + ventral keel
    c.add(prism_yz([(300, 40), (120, 96), (-300, 100), (-520, 44)], -7, 7))
    c.add(prism_yz([(300, -32), (100, -66), (-350, -66), (-500, -32)], -6, 6))
    # sponsons: side pods with pylons
    c.add(loft([(310, 5, 5, -6, 152), (250, 26, 24, -6, 152), (110, 36, 28, -6, 152),
                (-70, 32, 26, -6, 152), (-110, 12, 11, -6, 152)], N=10, sq=2.6), mirror=True)
    c.add(box((108, 105, -4), (94, 130, 14)), mirror=True)
    c.add(box((108, -20, -4), (94, 60, 14)), mirror=True)
    # engine cluster (3 nacelles + bells)
    e = P["Engine"]
    e.add(tube((0, -480, 0), (0, -565, 0), 46, 46, N=14))
    nozzle(e, 0, -565, 0, 42, 62)
    for s in (-1, 1):
        e.add(tube((88 * s, -400, -4), (88 * s, -545, -4), 31, 31, N=12))
        nozzle(e, 88 * s, -545, -4, 27, 52)
    e.add(box((0, -500, 0), (176, 40, 30)))
    # weapons: sponson twin barrels + hammerhead forward guns
    w = P["Weapon"]
    for s in (-1, 1):
        turret(w, 152 * s, 190, 18, 26, (0, 1, 0.02), 130)
        turret(w, 152 * s, 20, 18, 22, (0, 1, 0.02), 110)
    w.add(tube((150, 650, 4), (150, 790, 4), 6, 5, N=8), mirror=True)
    w.add(tube((110, 650, 4), (110, 790, 4), 6, 5, N=8), mirror=True)
    # sensor: mast + array crossbar on the bridge
    s = P["Sensor"]
    s.add(tube((0, 520, 104), (0, 520, 190), 5, 3, N=8))
    s.add(box((0, 520, 192), (96, 10, 6)))
    s.add(sphere_y((0, 520, 200), 10, 10, 10, N=8, rings=4))
    # reactor: dorsal vented core aft
    r = P["Reactor"]
    r.add(tube((0, -260, 50), (0, -260, 92), 36, 30, N=14))
    r.add(tube((0, -260, 92), (0, -260, 100), 20, 20, N=14))
    # cargo: ventral pod
    P["Cargo"].add(box((0, 20, -58), (84, 250, 30)))
    return dict(parts=P, mats={"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_Block",
                               "Sensor": "M_Sensor_Block", "Reactor": "M_Reactor_Block", "Cargo": "M_Cargo_Hold"},
                nav={"NavGreen": (-192, 600, 6), "NavRed": (192, 600, 6),
                     "NavWhite": (0, -640, 0), "NavBeacon": (0, -160, 108)}, nav_r=9)


def design_destroyer():
    P = {k: Kit() for k in ("Carcass", "Engine", "Weapon", "Sensor", "Reactor")}
    c = P["Carcass"]
    half = [(0, 950), (48, 790), (140, 300), (205, -300), (216, -620), (150, -800)]
    poly = half + [(-x, y) for x, y in reversed(half[1:])]
    c.add(prism(poly, -18, 32, s_top=0.66))                              # flat angular wedge blade
    # knife fins: ventral keel + low dorsal ridge (no tower)
    c.add(prism_yz([(860, -6), (240, -70), (-420, -92), (-700, -40), (-700, -8)], -6, 6))
    c.add(prism_yz([(700, 24), (300, 70), (-200, 84), (-620, 62), (-680, 24)], -7, 7))
    # angular armour facets on the deck
    c.add(prism([(-80, 420), (80, 420), (105, 250), (-105, 250)], 30, 44, s_top=0.7))
    c.add(prism([(-110, -80), (110, -80), (150, -420), (-150, -420)], 30, 46, s_top=0.75))
    c.add(box((0, -150, 52), (70, 130, 20)))                             # low sensor block
    rng = random.Random(2202)
    hwD = [(950, 0), (790, 48), (300, 140), (-300, 205), (-620, 216), (-800, 150)]
    def d_skip(x, y):
        for (px, py, rr) in ((0, 640, 44), (0, 430, 48), (0, -150, 70), (0, -450, 60), (0, -520, 40),
                             (105, 200, 40), (0, 250, 0), (0, -250, 0)):
            if (x - px) ** 2 + (y - py) ** 2 < rr * rr:
                return True
        return abs(x) < 4
    greebles(c, rng, 90, -560, 640, hwD, [(-800, 32), (900, 32)], frac=0.36, skip=d_skip,
             sx=(8, 22), sy=(14, 46), sz=(3, 8))
    # swept rear blade-fins
    c.add(prism([(200, -520), (330, -790), (300, -830), (150, -700)], -6, 6, s_top=0.6), mirror=True)
    # gun rails hugging the nose edges, forward-heavy
    w = P["Weapon"]
    for s in (-1, 1):
        w.add(tube((150 * s, -100, 30), (60 * s, 780, 30), 15, 11, N=10))
        w.add(tube((60 * s, 780, 30), (46 * s, 972, 30), 6, 5, N=8))
    turret(w, 0, 640, 44, 30, (0, 1, 0.03), 210, br=7)
    turret(w, 0, 430, 48, 34, (0, 1, 0.03), 210, br=8)
    for x in (-1, 1):
        turret(w, 105 * x, 200, 34, 22, (0, 1, 0.03), 150, br=6)
    # engine: bank of five nozzles on the trailing edge
    e = P["Engine"]
    e.add(box((0, -775, 4), (330, 70, 44)))
    for x, r in ((-120, 22), (-60, 27), (0, 32), (60, 27), (120, 22)):
        nozzle(e, x, -805, 4, r, 62)
    # reactor: dorsal glow vent aft-mid
    r = P["Reactor"]
    r.add(tube((0, -450, 44), (0, -450, 70), 44, 38, N=14))
    r.add(tube((0, -450, 70), (0, -450, 76), 26, 26, N=14))
    # sensor: slim blade mast on the dorsal ridge
    s = P["Sensor"]
    s.add(tube((0, -520, 80), (0, -520, 150), 4, 2.4, N=8))
    s.add(box((0, -520, 150), (60, 9, 4)))
    return dict(parts=P, mats={"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_TriLaser",
                               "Sensor": "M_Sensor_Block", "Reactor": "M_Reactor_Block"},
                nav={"NavGreen": (-332, -815, 0), "NavRed": (332, -815, 0),
                     "NavWhite": (0, -860, 8), "NavBeacon": (0, -240, 92)}, nav_r=9)


def design_freighter():
    P = {k: Kit() for k in ("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo")}
    c = P["Carcass"]
    # structural spine truss + gantry frames
    c.add(box((0, 140, 0), (86, 1540, 110)))
    for y in (-560, -260, 40, 340, 640):
        c.add(box((0, y, 0), (560, 22, 24)))                              # cross-frames
        c.add(box((0, y, 0), (24, 22, 320)))
    # small forward cockpit, tapered nose
    c.add(loft([(1110, 12, 10, 8), (1060, 42, 34, 10), (960, 70, 58, 12), (860, 80, 66, 8), (760, 80, 66, 0)],
               N=10, sq=3.2))
    c.add(loft([(1000, 20, 14, 62), (950, 44, 26, 66), (880, 48, 24, 62), (840, 36, 16, 58)], N=8, sq=2.4))
    c.add(box((0, 850, 118), (40, 60, 4)))
    # rear engine block (large)
    e = P["Engine"]
    e.add(loft([(-720, 120, 105, 0), (-800, 165, 150, 0), (-960, 190, 165, 0), (-1000, 190, 165, 0)],
               N=8, sq=4.5))
    for x in (-95, 95):
        for z in (-85, 85):
            nozzle(e, x, -1000, z, 58, 90)
    # reactor block between engines and cargo
    r = P["Reactor"]
    r.add(tube((0, -700, 0), (0, -830, 0), 70, 76, N=14))
    # cargo containers: comb of boxes on both sides of the spine (ribbed)
    k = P["Cargo"]
    inner = (-640, -340, -40, 260, 560)
    outer = (-640, -340, -40, 260)
    def container(x, y, z, sx, sy, sz):
        k.add(box((x, y, z), (sx, sy, sz)), mirror=(x != 0))
        for dy in (-sy * 0.36, 0, sy * 0.36):
            k.add(box((x, y + dy, z), (sx + 8, 9, sz + 8)), mirror=(x != 0))
    for y in inner:
        for z in (-82, 82):
            container(148, y, z, 190, 268, 156)
    for y in outer:
        container(346, y, 0, 190, 268, 156)
    # minimal defensive turrets
    w = P["Weapon"]
    for x in (-1, 1):
        turret(w, 40 * x, 780, 56, 15, (0, 1, 0.1), 40, twin=False, br=3)
        turret(w, 40 * x, -620, 56, 15, (0, -1, 0.1), 40, twin=False, br=3)
    # sensor
    s = P["Sensor"]
    s.add(tube((0, 900, 66), (0, 900, 150), 3.5, 2, N=8))
    s.add(sphere_y((0, 900, 160), 22, 9, 22, N=12, rings=6))
    return dict(parts=P, mats={"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_Block",
                               "Sensor": "M_Sensor_Block", "Reactor": "M_Reactor_Block", "Cargo": "M_Cargo_Hold"},
                nav={"NavGreen": (-448, 260, 0), "NavRed": (448, 260, 0),
                     "NavWhite": (0, -1100, 0), "NavBeacon": (0, 920, 100)}, nav_r=10)


def design_cruiser():
    P = {k: Kit() for k in ("Carcass", "Engine", "Weapon", "Sensor", "Reactor")}
    c = P["Carcass"]
    # broad lower hull with raised prow
    c.add(loft([(1500, 12, 12, 0), (1420, 90, 45, 0), (1200, 240, 80, 0), (800, 320, 100, 6),
                (0, 360, 112, 10), (-800, 350, 112, 10), (-1240, 300, 100, 10), (-1300, 285, 96, 10)],
               N=14, sq=3.0))
    # layered decks
    d1 = [(0, 1250), (200, 900), (262, -500), (232, -1250), (-232, -1250), (-262, -500), (-200, 900)]
    d2 = [(0, 820), (135, 600), (172, -300), (142, -1000), (-142, -1000), (-172, -300), (-135, 600)]
    c.add(prism(d1, 88, 165, s_top=0.93))
    c.add(prism(d2, 160, 228, s_top=0.9))
    rng = random.Random(3303)
    hw1 = [(1250, 0), (900, 200), (-500, 262), (-1250, 232)]
    hw2 = [(820, 0), (600, 135), (-300, 172), (-1000, 142)]
    def c_skip1(x, y):     # keep clear of the raised upper deck
        return x < interp(hw2, y) * 0.93 + 12 and -1000 < y < 800
    def c_skip2(x, y):     # keep clear of tower + reactor
        return (x < 130 and -560 < y < -170) or ((x) ** 2 + (y + 840) ** 2 < 80 ** 2) or y > 700
    greebles(c, rng, 90, -1200, 1150, hw1, [(-1250, 165), (1250, 165)], frac=0.8, skip=c_skip1)
    greebles(c, rng, 50, -950, 650, hw2, [(-1000, 228), (820, 228)], frac=0.72, skip=c_skip2,
             sx=(8, 22), sy=(12, 38))
    # prow blade
    c.add(prism_yz([(1500, 6), (1330, 140), (900, 176), (880, 96)], -9, 9))
    # command tower: base, neck, wide bridge, antennae
    c.add(box((0, -360, 300), (200, 340, 150), rot=(0, 0, 0)))
    c.add(box((0, -360, 425), (110, 200, 110)))
    c.add(loft([(-230, 130, 26, 495), (-300, 140, 34, 492), (-420, 150, 36, 490), (-500, 120, 26, 486)], N=8, sq=3.4))
    c.add(prism([(-90, -220), (90, -220), (140, -300), (-140, -300)], 470, 520, s_top=0.8))
    c.add(box((0, -360, 585), (60, 60, 18)))
    for x in (-95, 95):
        c.add(box((x, -330, 350), (16, 200, 90)))                        # tower buttress fins
    # amidships hull plating
    for y in (700, 300, -100, -700, -1050):
        c.add(box((0, y, 232), (60, 120, 6)))
    # broadside gun batteries: platforms + turrets barrels pointing sideways
    w = P["Weapon"]
    for y in (1000, 500, 0, -500, -1000):
        for s in (-1, 1):
            c.add(box((s * 335, y, 48), (76, 210, 60)))                  # sponson platform
            base = (s * 338, y, 80)
            w.add(tube(base, (base[0], base[1], base[2] + 34), 46, 40, N=12))
            for dy in (-16, 16):
                w.add(tube((base[0], y + dy, 100), (base[0] + s * 190, y + dy, 106), 10, 8, N=8))
    # engine array: wide stern bar + 7 nozzles
    e = P["Engine"]
    e.add(box((0, -1335, 20), (640, 150, 178)))
    for x in (-270, -180, -90, 0, 90, 180, 270):
        r = 54 if x == 0 else 40
        nozzle(e, x, -1410, 20, r, 90)
    # reactor: big dorsal vent stack
    r = P["Reactor"]
    r.add(tube((0, -840, 228), (0, -840, 262), 62, 54, N=16))
    r.add(tube((0, -840, 262), (0, -840, 270), 36, 36, N=16))
    # sensor: mast + array on top of the bridge, dish near the bow
    s = P["Sensor"]
    s.add(tube((0, -360, 594), (0, -360, 690), 6, 3, N=8))
    s.add(box((0, -360, 660), (120, 8, 6)))
    s.add(sphere_y((0, 1000, 240), 34, 34, 22, N=12, rings=6))
    return dict(parts=P, mats={"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_Block",
                               "Sensor": "M_Sensor_Block", "Reactor": "M_Reactor_Block"},
                nav={"NavGreen": (-532, 0, 100), "NavRed": (532, 0, 100),
                     "NavWhite": (0, -1500, 30), "NavBeacon": (0, -360, 700)}, nav_r=14)


DESIGNS = {"Fighter": design_fighter, "Corvette": design_corvette, "Destroyer": design_destroyer,
           "Freighter": design_freighter, "Cruiser": design_cruiser}


# ----------------------------------------------------------------------------
# Build / export
# ----------------------------------------------------------------------------
NAV_COL = {"NavGreen": ("M_Nav_Green", (0.0, 1.0, 0.4)), "NavRed": ("M_Nav_Red", (1.0, 0.15, 0.15)),
           "NavWhite": ("M_Nav_White", (1.0, 1.0, 0.95)), "NavBeacon": ("M_Nav_Beacon", (1.0, 0.55, 0.1))}


def export_fbx(obj, outname):
    G.sel_activate(obj)
    out = os.path.join(G.BASE, outname + ".fbx")
    if DRY_RUN:
        return out
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE')
    return out


def finish(ob, outname, matname, origin):
    """Set pivot (volume centre for parts / bbox centre for assembled), UV0, material, export."""
    G.sel_activate(ob)
    if origin == 'BBOX':
        bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    else:
        bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_VOLUME')
    G.smart_uv(ob)
    ob.name = outname
    ob.data.name = outname
    mat = bpy.data.materials.get(matname) or bpy.data.materials.new(matname)
    mat.use_nodes = True
    if matname in ("M_Nav_Green", "M_Nav_Red", "M_Nav_White", "M_Nav_Beacon"):
        col = NAV_COL[[k for k, v in NAV_COL.items() if v[0] == matname][0]][1]
        mat = G.emissive_mat(matname, col, strength=10.0)
    else:
        try:
            G.wire_pbr_material(mat, G.resolve_texset(matname), emissive_strength=2.6)
        except Exception as ex:
            print("  [warn] wire fail", matname, ex)
    ob.data.materials.clear()
    ob.data.materials.append(mat)
    path = export_fbx(ob, outname)
    return path


def tri_count(ob):
    ob.data.calc_loop_triangles()
    return len(ob.data.loop_triangles)


def build_ship(cls):
    base = f"SM_Ship_{cls}_01"
    G.setup_scene()
    d = DESIGNS[cls]()
    P = d["parts"]
    report = {"ship": base, "parts": {}}

    # nav-light kits
    navk = {}
    r = d["nav_r"]
    for nm, pos in d["nav"].items():
        k = Kit()
        k.add(sphere_y(pos, r, r, r, N=10, rings=5))
        if nm == "NavBeacon":
            k.add(tube((pos[0], pos[1], pos[2] - r * 4), (pos[0], pos[1], pos[2] - r * 0.6), r * 0.35, r * 0.35, N=6))
        navk[nm] = k

    # ---- Assembled (single joined mesh in ship space, bbox-centre pivot)
    allk = list(P.values()) + list(navk.values())
    asm = kits_to_mesh(f"{base}_AssembledGeo", allk)
    mn = Vector((1e9,) * 3); mx = Vector((-1e9,) * 3)
    for v in asm.data.vertices:
        for i in range(3):
            mn[i] = min(mn[i], v.co[i]); mx[i] = max(mx[i], v.co[i])
    dims = [mx[i] - mn[i] for i in range(3)]
    path = finish(asm, f"{base}_Assembled", "M_Assembled", 'BBOX')
    # bbox of mesh in ship-space (pre-recentre) for hardpoint manifest
    ctr = Vector(((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, (mn.z + mx.z) / 2))
    report["assembled"] = dict(path=path, tris=tri_count(asm), dims_bu=[round(x, 1) for x in dims],
                               dims_uu_raw=[round(x * 100, 0) for x in dims])
    bpy.data.objects.remove(asm, do_unlink=True)

    # ---- parts
    hp = {}
    hp_key = {"Engine": ("HP_Engine", "engine"), "Cargo": ("HP_Cargo", "cargo"), "Weapon": ("HP_Nose", "weapon"),
              "Sensor": ("HP_Sensor", "sensor"), "Reactor": ("HP_Core", "reactor")}
    for suf, kit in P.items():
        if kit.empty():
            continue
        pmn, pmx = kit.bounds()
        center = [(pmn[i] + pmx[i]) / 2 for i in range(3)]
        ob = kits_to_mesh(f"{base}_{suf}_Geo", [kit])
        path = finish(ob, f"{base}_{suf}", d["mats"][suf], 'VOLUME')
        report["parts"][suf] = dict(path=path, tris=tri_count(ob), mat=d["mats"][suf])
        bpy.data.objects.remove(ob, do_unlink=True)
        if suf in hp_key:
            hp[hp_key[suf][0]] = dict(position=[round(v, 1) for v in center], mount_orientation="AXIS_FORWARD",
                                      allowed_types=[hp_key[suf][1]], occupied=True)
    for nm, kit in navk.items():
        ob = kits_to_mesh(f"{base}_Nav_{nm}_Geo", [kit])
        path = finish(ob, f"{base}_Nav_{nm}", NAV_COL[nm][0], 'VOLUME')
        report["parts"]["Nav_" + nm] = dict(path=path, tris=tri_count(ob), mat=NAV_COL[nm][0])
        bpy.data.objects.remove(ob, do_unlink=True)

    # hardpoint manifest (same schema as legacy; positions are ship-space, pivot-relative
    # to the Assembled bbox centre so they line up with the recentred Assembled mesh)
    for v in hp.values():
        v["position"] = [round(v["position"][i] - ctr[i], 1) for i in range(3)]
    man = dict(ship=base, size_class="hero_" + cls.lower(), scale=1.0,
               carcass_dims=[round(x, 1) for x in dims], hardpoints=hp)
    mp = os.path.join(G.BASE, f"{base}_hardpoints.json")
    if not DRY_RUN:
        with open(mp, "w") as f:
            json.dump(man, f, indent=2)
    return report


def main():
    global DRY_RUN
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    DRY_RUN = "--dry-run" in argv
    names = [a for a in argv if not a.startswith("--")] or list(DESIGNS)
    allrep = {}
    for n in names:
        rep = build_ship(n)
        allrep[n] = rep
        a = rep["assembled"]
        print(f"[hero] {n}: assembled tris={a['tris']} dims_bu={a['dims_bu']} -> {a['path']}")
        for suf, p in rep["parts"].items():
            print(f"         {suf}: tris={p['tris']} mat={p['mat']}")
    print("HERO_DONE" + (" (dry-run, nothing written)" if DRY_RUN else ""))
    return allrep


if __name__ == "__main__":
    main()
