"""
Redesigns of the ships the game currently flies. Fifteen of the sixteen
Blueprint ships share one procedural "stacked slab" hull; each gets its own
silhouette here, driven by the Blueprint's role. (The Fighter is already a
distinct design and is left alone.)

  Gunship_02      BP_Ship_Gunship            squat armoured wedge, shoulder cannon pods
  Escort_01       BP_Ship_Patrol             patrol cutter, twin tail booms, searchlight
  Courier_01      BP_Ship_Science            science ship, lab pods, sensor booms, dish
  Smuggler_01     BP_Ship_Frigate / Utility  faceted stealth wedge, oversized engines
  Trader_01       BP_Ship_Trading            bridge module, spine of cargo pods
  Freighter_01    BP_Ship_Freighter          hero freighter (container comb) + windows
  CargoFreighter  BP_Ship_Transport_Genesis  tanker: four long tanks round a spine
  Corvette_01     BP_Ship_Corvette           symmetric hammerhead, glazed hammer face, X fins
  Miner_01        BP_Ship_Mining             octagonal core, drill, + laser arms, X ore silos
  Destroyer_01    BP_Ship_Destroyer          diamond-section arrowhead, turrets top+bottom
  HeavyHauler_01  BP_Ship_Transport_Behemoth cab pushing a truss packed with containers
  BulkCarrier_01  BP_Ship_Carrier            core hull + two flight pods with hangar mouths
  Cruiser_01      BP_Ship_Cruiser            hex hull, + gun sponsons, blades top+bottom
  Battleship_01   BP_Battleship / Luxury     diamond hull, triple turrets top+bottom, spinal gun
  CommandXL_01    BP_CommandXL               glazed command globe, X fins with comm dishes

Every design is scaled to the CURRENT mesh's length (TARGET_LEN), so the
Blueprints' existing ShipMesh scales keep the ships the same size in game.
Outputs (nothing touches Assets/FBX/generated):
  Assets/FBX/concepts/redo/            per-part FBX + _Assembled + _hardpoints.json
  Assets/FBX/concepts/redo/renders/    clay sheets, before/after pairs, contact sheet

Usage: blender -b --python Tools/generate_ship_redesigns.py -- [Gunship_02 ...] [--no-fbx]
"""
import os, sys, math, random, json, glob, importlib.util
import bpy
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("generate_ship_concepts",
                                               os.path.join(HERE, "generate_ship_concepts.py"))
C = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(C)
H, G, Kit = C.H, C.G, C.Kit
loft, tube, prism, prism_yz, box, sphere_y, nozzle, turret, _xf = (
    C.loft, C.tube, C.prism, C.prism_yz, C.box, C.sphere_y, C.nozzle, H.turret, C._xf)
cbox, mirrored, torus_y, truss_y, parts, result = C.cbox, C.mirrored, C.torus_y, C.truss_y, C.parts, C.result

C.STD_MATS.update({"Drill": "M_Drill", "MiningLaser": "M_MiningLaser"})
C.CLAY.update({"Drill": ((0.55, 0.50, 0.42), 0.35, 0.8, 0), "MiningLaser": ((0.30, 0.34, 0.36), 0.35, 0.7, 0)})

OUT = os.path.normpath(os.path.join(HERE, "..", "Assets", "FBX", "concepts", "redo"))
RENDERS = os.path.join(OUT, "renders")
CURRENT = os.path.join(C.RENDERS, "current")

# current in-game mesh lengths (Y extent, Blender units) from render_game_ships
TARGET_LEN = {"Gunship_02": 608, "Escort_01": 608, "Courier_01": 608, "Smuggler_01": 608,
              "Trader_01": 1266, "Freighter_01": 1266, "CargoFreighter_01": 1266,
              "Corvette_01": 1547, "Miner_01": 1547, "Destroyer_01": 1547, "HeavyHauler_01": 1547,
              "BulkCarrier_01": 1726, "Battleship_01": 1726, "Cruiser_01": 1637, "CommandXL_01": 2026}


def fit_length(d, target):
    """Uniformly scale a design so its Y extent equals target."""
    ys = [v[1] for k in d["parts"].values() for g in k.groups for p in g for v in p]
    ys += [p[1] for p in d["nav"].values()]
    s = target / (max(ys) - min(ys))
    for k in d["parts"].values():
        k.groups = [[[(x * s, y * s, z * s) for x, y, z in p] for p in g] for g in k.groups]
    d["nav"] = {n: tuple(c * s for c in p) for n, p in d["nav"].items()}
    d["nav_r"] *= s
    return d


def add_windows(d, polys_list):
    d["parts"].setdefault("Windows", Kit())
    d["mats"]["Windows"] = C.STD_MATS["Windows"]
    for polys, mir in polys_list:
        d["parts"]["Windows"].add(polys, mirror=mir)
    return d


def triple(kit, x, y, z, r, fwd=1, blen=None):
    """Heavy triple turret: armoured drum + gun house + three barrels (fwd=+1/-1)."""
    blen = blen or r * 3.2
    kit.add(tube((x, y, z), (x, y, z + r * 0.35), r, r * 0.95, N=14))
    kit.add(cbox((x, y + fwd * r * 0.15, z + r * 0.6), (r * 1.6, r * 1.5, r * 0.55), 0.8))
    for dx in (-0.45, 0, 0.45):
        b0 = (x + dx * r, y + fwd * r * 0.7, z + r * 0.6)
        kit.add(tube(b0, (b0[0], b0[1] + fwd * blen, b0[2] + r * 0.05), r * 0.13, r * 0.11, N=8))


def ring_windows(win, y, rad, n, size=(16, 24, 4), start=0.0, arc=360.0):
    for k in range(n):
        win.add(_xf(box((0, y, rad), size), rot=(0, start + k * arc / n, 0)))


# ----------------------------------------------------------------------------
# Small ships (~608 long)
# ----------------------------------------------------------------------------
def design_gunship():
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(loft([(300, 20, 14, 0), (240, 70, 40, 0), (100, 110, 58, 4), (-150, 120, 60, 4),
                (-270, 100, 50, 2), (-295, 84, 42, 2)], N=8, sq=4.0))
    c.add(cbox((120, 40, 2), (16, 280, 92), 0.85), mirror=True)            # cheek armour
    c.add(loft([(262, 6, 4, 34), (222, 30, 16, 44), (150, 36, 22, 54), (96, 26, 14, 52)], N=8, sq=3.0))
    c.add(box((150, -30, 52), (80, 100, 26)), mirror=True)                  # shoulder pylons
    c.add(loft([(200, 10, 10, 52, 205), (160, 30, 28, 52, 205), (-110, 32, 30, 52, 205),
                (-150, 22, 22, 52, 205)], N=8, sq=3.6), mirror=True)       # cannon pods
    for y in (-60, 20, 100):
        c.add(loft([(y, 35, 33, 52, 205), (y - 10, 35, 33, 52, 205)], N=8, sq=3.6), mirror=True)
    rng = random.Random(8101)
    H.greebles(c, rng, 22, -250, 80, [(-295, 70), (-150, 100), (100, 92)], [(-295, 44), (-150, 64), (100, 62)],
               frac=0.7, skip=lambda x, y: x < 30 and y > 40, sx=(6, 16), sy=(8, 24), sz=(2, 5))
    P["Windows"].add(box((0, 232, 48), (46, 34, 4), rot=(-28, 0, 0)))
    P["Windows"].add(box((34, 160, 56), (4, 60, 8)), mirror=True)
    w = P["Weapon"]
    w.add(tube((205, 200, 52), (205, 300, 52), 9, 8, N=10), mirror=True)
    w.add(box((205, 296, 52), (26, 22, 26)), mirror=True)
    w.add(tube((0, 180, -58), (0, 180, -40), 24, 22, N=12))                 # chin turret
    for sx in (-1, 1):
        w.add(tube((8 * sx, 190, -50), (8 * sx, 290, -54), 4, 3.5, N=8))
    e = P["Engine"]
    e.add(cbox((0, -300, 4), (170, 60, 90), 0.9))
    for sx in (-1, 1):
        nozzle(e, 50 * sx, -330, 4, 26, 36)
        nozzle(e, 205 * sx, -150, 52, 14, 20)
    P["Reactor"].add(tube((0, -160, 62), (0, -160, 70), 30, 26, N=14))
    s = P["Sensor"]
    s.add(tube((0, -60, 64), (0, -60, 110), 3, 2, N=6))
    s.add(sphere_y((120, 180, 40), 10, 14, 8, N=8, rings=4), mirror=True)
    return result(P, {"NavGreen": (-240, -40, 52), "NavRed": (240, -40, 52),
                      "NavWhite": (0, -380, 4), "NavBeacon": (0, -60, 116)}, 5)


def design_escort():
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(loft([(310, 4, 4, 0), (250, 30, 20, 0), (120, 56, 34, 2), (-80, 60, 36, 2), (-200, 46, 30, 2),
                (-215, 40, 26, 2)], N=12, sq=2.6))
    c.add(prism([(40, 40), (40, -120), (150, -150), (150, -100)], -3, 4, s_top=0.9), mirror=True)
    c.add(loft([(-30, 6, 6, 0, 150), (-60, 18, 16, 0, 150), (-300, 18, 16, 0, 150), (-320, 12, 12, 0, 150)],
               N=10, sq=2.4), mirror=True)                                  # tail booms
    c.add(prism_yz([(-230, 14), (-310, 72), (-332, 72), (-322, 14)], 147, 153), mirror=True)
    c.add(box((0, -305, 0), (300, 36, 5)))                                  # stabiliser
    c.add(box((0, 170, 46), (4, 110, 3)))
    rng = random.Random(8201)
    H.greebles(c, rng, 18, -190, 110, [(-200, 40), (-80, 54), (120, 50)], [(-200, 32), (-80, 38), (120, 36)],
               frac=0.6, skip=lambda x, y: y > 100 or abs(y - 40) < 26 or abs(y + 60) < 14,
               sx=(5, 12), sy=(8, 20), sz=(2, 4))
    win = P["Windows"]
    win.add(sphere_y((0, 170, 30), 20, 60, 16, N=12, rings=6))
    for y in (60, 20, -20):
        win.add(box((58, y, 4), (4, 22, 8)), mirror=True)
    e = P["Engine"]
    e.add(tube((0, -200, 2), (0, -232, 2), 40, 38, N=14))
    nozzle(e, 0, -232, 2, 32, 34)
    for sx in (-1, 1):
        nozzle(e, 150 * sx, -320, 0, 12, 20)
    w = P["Weapon"]
    turret(w, 0, 40, 36, 16, (0, 1, 0.05), 60, br=3.5)
    w.add(tube((110, -20, -4), (110, 130, -4), 4, 3, N=8), mirror=True)
    s = P["Sensor"]
    s.add(sphere_y((0, 210, -26), 14, 16, 12, N=10, rings=5))              # searchlight/scanner
    s.add(tube((0, -60, 36), (0, -60, 70), 3, 2, N=6))
    s.add(box((0, -60, 72), (60, 6, 4)))
    P["Reactor"].add(box((0, -140, 35), (30, 50, 4)))
    return result(P, {"NavGreen": (-168, -200, 0), "NavRed": (168, -200, 0),
                      "NavWhite": (0, -332, 6), "NavBeacon": (0, -60, 80)}, 4.5)


def design_courier():
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(loft([(300, 6, 6, 0), (260, 34, 34, 0), (120, 44, 44, 0), (-150, 44, 44, 0), (-240, 34, 34, 0)],
               N=14, sq=2.0))
    c.add(torus_y((0, 200, 0), 44, 6, N=24, M=6))                           # sensor collar
    c.add(loft([(120, 6, 6, 0, 120), (100, 26, 26, 0, 120), (-80, 26, 26, 0, 120), (-100, 6, 6, 0, 120)],
               N=12, sq=2.0), mirror=True)                                  # lab pods
    c.add(box((80, 10, 0), (80, 30, 10)), mirror=True)
    c.add(box((80, -60, 0), (80, 20, 8)), mirror=True)
    win = P["Windows"]
    for y in (-50, -10, 30, 70):
        win.add(box((146, y, 6), (4, 16, 10)), mirror=True)
    ring_windows(win, 240, 30, 10, size=(10, 16, 3))
    s = P["Sensor"]
    s.add(tube((120, 100, 0), (120, 330, 0), 3, 2, N=6), mirror=True)       # sensor booms
    s.add(sphere_y((120, 330, 0), 6, 6, 6, N=8, rings=4), mirror=True)
    s.add(tube((0, -40, 44), (0, -40, 90), 4, 3, N=8))
    s.add(tube((0, -40, 90), (0, -12, 124), 6, 50, N=24))                   # dish
    s.add(_xf(torus_y((0, 0, 0), 50, 2.5, N=24, M=4), pos=(0, -12, 124), rot=(50, 0, 0)))
    s.add(box((0, 60, -50), (40, 120, 10)))                                 # ventral array
    r = P["Reactor"]
    for y in (-160, -190):
        r.add(tube((0, y, 0), (0, y - 8, 0), 46, 46, N=14))
    e = P["Engine"]
    e.add(tube((0, -240, 0), (0, -268, 0), 36, 34, N=14))
    nozzle(e, 0, -268, 0, 28, 36)
    for sx in (-1, 1):
        nozzle(e, 120 * sx, -100, 0, 10, 18)
    turret(P["Weapon"], 0, 120, 44, 10, (0, 1, 0.1), 30, br=2.4)
    return result(P, {"NavGreen": (-148, 0, 0), "NavRed": (148, 0, 0),
                      "NavWhite": (0, -230, 38), "NavBeacon": (0, 80, 50)}, 4.5)


def design_smuggler():
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    half = [(0, 320), (60, 250), (170, 40), (200, -200), (160, -280), (0, -250)]
    c.add(prism(mirrored(half), -18, 24, s_top=0.7))
    c.add(prism(mirrored([(x * 0.8, y * 0.9) for x, y in half]), -26, -18, s_top=1.2))
    c.add(prism([(-60, 120), (60, 120), (80, -160), (-80, -160)], 24, 40, s_top=0.8))
    for sx in (-1, 1):
        c.add(loft([(-60, 10, 8, 0, 120 * sx), (-120, 40, 30, 0, 120 * sx), (-300, 42, 32, 0, 120 * sx),
                    (-320, 36, 28, 0, 120 * sx)], N=8, sq=3.2))
    fin = prism_yz([(-120, 20), (-230, 80), (-255, 80), (-240, 20)], -3, 3)
    c.add(_xf(fin, pos=(90, 0, 0), rot=(0, 25, 0)), mirror=True)
    P["Windows"].add(prism([(-22, 240), (22, 240), (34, 160), (-34, 160)], 20, 32, s_top=0.6))
    k = P["Cargo"]
    k.add(box((34, -20, 41), (60, 210, 3)), mirror=True)                    # bay doors
    e = P["Engine"]
    for sx in (-1, 1):
        nozzle(e, 120 * sx, -320, 0, 30, 36)
    w = P["Weapon"]
    w.add(tube((50, 200, -6), (50, 300, -6), 4, 3, N=8), mirror=True)
    P["Sensor"].add(sphere_y((0, -120, 40), 10, 14, 6, N=8, rings=4))
    P["Reactor"].add(box((150, -60, 2), (4, 120, 5)), mirror=True)
    return result(P, {"NavGreen": (-196, -150, 0), "NavRed": (196, -150, 0),
                      "NavWhite": (0, -360, 0), "NavBeacon": (0, -60, 46)}, 4.5)


# ----------------------------------------------------------------------------
# Medium (~1266)
# ----------------------------------------------------------------------------
def design_trader():
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    c.add(loft([(640, 10, 10, 0), (590, 60, 46, 0), (470, 100, 70, 0), (330, 100, 70, 0), (290, 70, 50, 0)],
               N=12, sq=2.8))
    c.add(box((0, -10, 0), (70, 620, 60)))                                  # spine
    c.add(loft([(560, 20, 10, 76), (530, 50, 22, 80), (430, 56, 24, 80), (400, 40, 16, 76)], N=8, sq=3.0))
    c.add(loft([(-290, 70, 50, 0), (-340, 120, 80, 0), (-520, 130, 85, 0), (-560, 110, 70, 0)], N=10, sq=3.5))
    k = P["Cargo"]
    for y in (200, 0, -200):
        k.add(cbox((110, y, 0), (140, 180, 120), 0.92), mirror=True)
        k.add(cbox((0, y, 94), (110, 180, 70), 0.9))
        for dy in (-60, 60):
            k.add(box((110, y + dy, 0), (148, 8, 128)), mirror=True)
        c.add(box((55, y, 0), (40, 60, 40)), mirror=True)                  # clamps
    rng = random.Random(8501)
    H.greebles(c, rng, 22, 310, 560, [(300, 70), (470, 90), (590, 50)], [(300, 50), (470, 70), (590, 44)],
               frac=0.8, skip=lambda x, y: x < 60 and y > 390, sx=(8, 18), sy=(10, 26), sz=(3, 6))
    H.greebles(c, rng, 16, -540, -330, [(-540, 110), (-330, 110)], [(-540, 84), (-330, 84)], frac=0.7,
               skip=lambda x, y: x < 40 and abs(y + 440) < 40)
    win = P["Windows"]
    win.add(box((0, 537, 88), (80, 6, 12)))
    for y in range(360, 580, 40):
        win.add(box((100, y, 10), (4, 20, 12)), mirror=True)
    e = P["Engine"]
    for sx in (-1, 1):
        for sz in (-1, 1):
            nozzle(e, 60 * sx, -560, 40 * sz, 34, 50)
    w = P["Weapon"]
    turret(w, 0, 450, 70, 16, (0, 1, 0.1), 50, br=3.5)
    turret(w, 0, -470, 86, 16, (0, -1, 0.1), 50, br=3.5)
    P["Reactor"].add(tube((0, -440, 84), (0, -440, 96), 34, 28, N=14))
    s = P["Sensor"]
    s.add(tube((0, 470, 102), (0, 470, 170), 4, 2.5, N=6))
    s.add(sphere_y((0, 470, 176), 12, 12, 12, N=8, rings=4))
    return result(P, {"NavGreen": (-182, 0, 0), "NavRed": (182, 0, 0),
                      "NavWhite": (0, -640, 0), "NavBeacon": (0, 470, 192)}, 8)


def design_freighter():
    d = H.design_freighter()
    return add_windows(d, [(box((0, 998, 70), (34, 6, 12)), False), (box((44, 930, 70), (4, 50, 10)), True)])


def design_cargofreighter():
    """Tanker: four long capped tanks around a spine, forward cab, engine block."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    truss_y(c, 420, -420, 30, 105, r=6)
    c.add(loft([(640, 10, 10, 0), (600, 60, 50, 0), (500, 90, 76, 0), (440, 90, 76, 0), (420, 60, 50, 0)],
               N=12, sq=3.0))
    c.add(loft([(560, 20, 10, 70), (530, 50, 22, 74), (470, 50, 22, 74), (450, 30, 12, 70)], N=8, sq=3.0))
    k = P["Cargo"]
    for sx in (-1, 1):
        for sz in (-1, 1):
            x, z = 95 * sx, 95 * sz
            k.add(loft([(430, 10, 10, z, x), (400, 58, 58, z, x), (360, 72, 72, z, x), (-360, 72, 72, z, x),
                        (-400, 58, 58, z, x), (-430, 10, 10, z, x)], N=16, sq=2.0))
            for y in (-280, -140, 0, 140, 280):
                c.add(torus_y((x, y, z), 73, 4, N=20, M=4))
    for y in (-300, 0, 300):                                                # tank cradles
        c.add(box((0, y, 0), (340, 16, 30)))
        c.add(box((0, y, 0), (30, 16, 340)))
    for sz in (-1, 1):                                                      # manifold pipes
        c.add(tube((0, 420, 170 * sz), (0, -420, 170 * sz), 7, 7, N=6))
    c.add(loft([(-420, 70, 70, 0), (-470, 130, 110, 0), (-600, 140, 120, 0), (-630, 120, 100, 0)], N=10, sq=3.5))
    win = P["Windows"]
    win.add(box((0, 546, 80), (70, 6, 10)))
    for y in range(460, 600, 35):
        win.add(box((90, y, 10), (4, 18, 10)), mirror=True)
    e = P["Engine"]
    nozzle(e, 0, -630, 0, 56, 70)
    for sx in (-1, 1):
        nozzle(e, 90 * sx, -630, 60, 30, 44)
        nozzle(e, 90 * sx, -630, -60, 30, 44)
    P["Reactor"].add(tube((0, -520, 118), (0, -520, 130), 40, 34, N=14))
    turret(P["Weapon"], 0, 480, 76, 14, (0, 1, 0.1), 40, br=3)
    s = P["Sensor"]
    s.add(tube((0, 500, 96), (0, 500, 160), 4, 2.5, N=6))
    s.add(box((0, 500, 150), (60, 6, 4)))
    return result(P, {"NavGreen": (-170, 0, 0), "NavRed": (170, 0, 0),
                      "NavWhite": (0, -720, 0), "NavBeacon": (0, 500, 172)}, 8)


# ----------------------------------------------------------------------------
# Large (~1550-1730)
# ----------------------------------------------------------------------------
# Space-ship rules for the big hulls (they read as boats otherwise): no flat top
# decks or deck towers; mass above AND below the centreline; bridges are window
# bands in the nose; turrets top and bottom; fins/arms in X or + layouts; the
# engine cluster is the dominant feature.
def flipz(polys):
    return [[(x, y, -z) for x, y, z in reversed(p)] for p in polys]


def add_sym(kit, polys, mirror=False):
    """Add polys and their mirror below the centreline (z -> -z)."""
    kit.add(polys, mirror=mirror)
    kit.add(flipz(polys), mirror=mirror)


def add_roll(kit, polys, rolls, pos=(0, 0, 0)):
    """Add copies of polys (authored pointing +Z) rolled about the Y axis."""
    for a in rolls:
        kit.add(_xf(_xf(polys, rot=(0, a, 0)), pos=pos))


def rolled_turret(kit, y, rad, roll, r, barrel_len, twin=True, br=None, fwd=1):
    """Turret standing on a surface rad from the axis, rolled round Y (0 = top, 180 = bottom)."""
    tk = Kit()
    turret(tk, 0, y, rad, r, (0, fwd, 0.04), barrel_len, twin=twin, br=br)
    for g in tk.groups:
        kit.add(_xf(g, rot=(0, roll, 0)))


def rolled_triple(kit, y, rad, roll, r, fwd=1):
    tk = Kit()
    triple(tk, 0, y, rad, r, fwd=fwd)
    for g in tk.groups:
        kit.add(_xf(g, rot=(0, roll, 0)))


def greebles_sym(kit, rng, *a, **kw):
    tk = Kit()
    H.greebles(tk, rng, *a, **kw)
    for g in tk.groups:
        add_sym(kit, g)


def surf_greebles(kit, rng, n, y0, y1, hw_pairs, hh_pairs, sq, rolls=(0, 180), xfrac=0.55, pos=(0, 0, 0),
                  skip=None, sx=(8, 20), sy=(12, 36), sz=(2, 6)):
    """Mirrored plates sitting ON a superellipse loft (|x/hw|^sq + |z/hh|^sq = 1),
    placed on the faces given by rolls (0 top, 180 bottom, 90/270 sides)."""
    for roll in rolls:
        side = roll % 180 != 0
        for _ in range(n):
            y = rng.uniform(y0, y1)
            A = H.interp(hh_pairs if side else hw_pairs, y)
            B = H.interp(hw_pairs if side else hh_pairs, y)
            u = rng.uniform(6, max(7, A * xfrac))
            if skip and skip(u, y, roll):
                continue
            z = B * max(0.0, 1 - (u / A) ** sq) ** (1 / sq)
            d = (rng.uniform(*sx), rng.uniform(*sy), rng.uniform(*sz))
            for uu in (u, -u):
                kit.add(_xf(_xf(box((uu, y, z + d[2] / 2 - 1.2), d), rot=(0, roll, 0)), pos=pos))


def collar(kit, y, hw, hh, N=8, sq=2.0, t=14, grow=8):
    kit.add(loft([(y, hw + grow, hh + grow, 0), (y - t, hw + grow, hh + grow, 0)], N=N, sq=sq))


def design_corvette():
    """Hammerhead corvette: symmetric hammer with the bridge glazed into its
    face, slim spine, sponsons with turrets above and below, X radiator fins."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    c.add(loft([(-560, 50, 50, 0), (-420, 62, 60, 0), (-100, 70, 64, 0), (150, 62, 56, 0),
                (330, 50, 46, 0), (470, 40, 38, 0)], N=8, sq=2.2))
    ham = [(-105, 425), (105, 425), (190, 520), (190, 640), (62, 715), (-62, 715), (-190, 640), (-190, 520)]
    add_sym(c, prism(ham, 0, 34, s_top=0.8))
    for y in (-380, -140, 100, 300):
        H_ = H.interp([(-560, 50), (-420, 62), (-100, 70), (150, 62), (330, 50), (470, 40)], y)
        collar(c, y, H_, H_ - 4, N=8, sq=2.2, t=16, grow=7)
    for sx in (-1, 1):
        c.add(loft([(310, 5, 5, 0, 152 * sx), (250, 26, 24, 0, 152 * sx), (110, 36, 30, 0, 152 * sx),
                    (-70, 32, 28, 0, 152 * sx), (-110, 12, 11, 0, 152 * sx)], N=8, sq=2.4))
    c.add(box((108, 105, 0), (94, 130, 14)), mirror=True)
    c.add(box((108, -20, 0), (94, 60, 14)), mirror=True)
    add_roll(c, prism_yz([(-300, 60), (-440, 170), (-520, 170), (-500, 60)], -5, 5), (45, 135, 225, 315))
    rng = random.Random(9201)
    greebles_sym(c, rng, 26, 440, 690, [(425, 120), (640, 160), (715, 62)], [(425, 34), (715, 34)], frac=0.8,
                 sx=(8, 20), sy=(10, 30), sz=(2, 5))
    win = P["Windows"]
    win.add(box((0, 716, 0), (110, 4, 22)))                                 # bridge glazing in hammer face
    win.add(box((150, 690, 0), (4, 60, 16), rot=(0, 0, -58)), mirror=True)
    for y in range(-300, 320, 60):
        win.add(box((64, y, 0), (4, 24, 10)), mirror=True)
    e = P["Engine"]
    e.add(tube((0, -555, 0), (0, -600, 0), 52, 52, N=14))
    nozzle(e, 0, -600, 0, 46, 60)
    for sx in (-1, 1):
        e.add(tube((152 * sx, -110, 0), (152 * sx, -180, 0), 24, 24, N=12))
        nozzle(e, 152 * sx, -180, 0, 20, 36)
        for sz in (-1, 1):
            nozzle(e, 58 * sx, -560, 58 * sz, 18, 34)
    w = P["Weapon"]
    for sx in (-1, 1):
        for roll in (0, 180):
            tk = Kit()
            turret(tk, 152 * sx, 150, 30, 24, (0, 1, 0.02), 120)
            for g in tk.groups:
                w.add(g if roll == 0 else flipz(g))
    w.add(tube((150, 650, 0), (150, 790, 0), 6, 5, N=8), mirror=True)
    s = P["Sensor"]
    add_sym(s, tube((0, 600, 34), (0, 640, 90), 4, 2, N=6))
    P["Reactor"].add(torus_y((0, -260, 0), 74, 8, N=24, M=6))
    P["Cargo"].add(box((0, 20, -70), (70, 250, 20)))
    return result(P, {"NavGreen": (-192, 600, 0), "NavRed": (192, 600, 0),
                      "NavWhite": (0, -680, 0), "NavBeacon": (0, 640, 96)}, 9)


def design_destroyer():
    """Double-wedge (diamond-section) arrowhead: facets above and below, gun rails
    on the knife edges, turrets top and bottom, two-row engine bank."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    half = [(0, 950), (48, 790), (140, 300), (205, -300), (216, -620), (150, -800)]
    poly = half + [(-x, y) for x, y in reversed(half[1:])]
    add_sym(c, prism(poly, 0, 52, s_top=0.58))
    add_sym(c, prism([(-80, 420), (80, 420), (105, 250), (-105, 250)], 44, 60, s_top=0.7))
    add_sym(c, prism([(-110, -80), (110, -80), (150, -420), (-150, -420)], 40, 58, s_top=0.75))
    c.add(prism([(-200, -520), (-150, -700), (-300, -830), (-330, -790)], -6, 6, s_top=0.6))
    c.add(prism([(200, -520), (330, -790), (300, -830), (150, -700)], -6, 6, s_top=0.6))
    rng = random.Random(9301)
    greebles_sym(c, rng, 50, -560, 620, [(950, 0), (790, 30), (300, 80), (-300, 118), (-620, 124)],
                 [(-800, 50), (900, 50)], frac=0.8,
                 skip=lambda x, y: (x < 110 and 240 < y < 440) or (x < 150 and -440 < y < -60)
                 or (x < 40 and abs(y - 640) < 40) or (x < 50 and abs(y + 560) < 50),
                 sx=(8, 20), sy=(14, 40), sz=(2, 6))
    w = P["Weapon"]
    for s in (-1, 1):
        w.add(tube((196 * s, -250, 0), (46 * s, 790, 0), 13, 9, N=10))
        w.add(tube((46 * s, 790, 0), (30 * s, 990, 0), 5, 4, N=8))
    for roll in (0, 180):
        rolled_turret(w, 640, 44, roll, 28, 190, br=6)
        rolled_turret(w, 150, 60, roll, 24, 150, br=5)
    win = P["Windows"]
    add_sym(win, box((0, 419, 52), (110, 4, 8)))
    for y in range(-400, 200, 70):
        win.add(box((170 + (y + 400) * -0.05, y, 12), (4, 30, 6)), mirror=True)
    e = P["Engine"]
    e.add(cbox((0, -800, 0), (300, 50, 90), 1.0))
    for x, r in ((-100, 24), (-34, 28), (34, 28), (100, 24)):
        for z in (-24, 24):
            nozzle(e, x, -825, z, r * 0.8, 50)
    r = P["Reactor"]
    add_sym(r, box((0, -250, 59), (40, 300, 3)))
    s = P["Sensor"]
    add_sym(s, tube((0, -560, 50), (0, -620, 120), 4, 2, N=6))
    return result(P, {"NavGreen": (-332, -815, 0), "NavRed": (332, -815, 0),
                      "NavWhite": (0, -880, 0), "NavBeacon": (0, 950, 0)}, 9)


def design_miner():
    """Mining ship: octagonal core, stepped drill, four laser arms in a + layout,
    four ore silos in an X layout, cab ring, engine cluster."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows", "Drill", "MiningLaser")
    c = P["Carcass"]
    c.add(loft([(560, 90, 90, 0), (520, 110, 110, 0), (-420, 110, 110, 0), (-470, 90, 90, 0)], N=8, sq=2.0))
    c.add(tube((0, 540, 0), (0, 600, 0), 116, 96, N=16))
    for y in (420, 200, -20, -240):
        collar(c, y, 110, 110, N=8, t=18, grow=10)
    k = P["Cargo"]
    for ang in (45, 135, 225, 315):
        silo = tube((0, 380, 175), (0, -300, 175), 62, 62, N=14)
        silo += sphere_y((0, 380, 175), 62, 30, 62, N=14, rings=4)
        silo += sphere_y((0, -300, 175), 62, 30, 62, N=14, rings=4)
        k.add(_xf(silo, rot=(0, ang, 0)))
        for y in (280, 40, -200):
            c.add(_xf(torus_y((0, y, 175), 63, 4, N=20, M=4), rot=(0, ang, 0)))
            c.add(_xf(box((0, y, 128), (20, 20, 40)), rot=(0, ang, 0)))
    d = P["Drill"]
    d.add(tube((0, 600, 0), (0, 860, 0), 92, 8, N=18))
    for y in (640, 695, 750, 805):
        rr = 92 - (y - 600) * 84 / 260
        d.add(tube((0, y, 0), (0, y + 16, 0), rr + 10, rr + 6, N=18))
    m = P["MiningLaser"]
    arm = (tube((0, 360, 100), (0, 520, 190), 13, 11, N=10) + tube((0, 520, 190), (0, 620, 190), 24, 16, N=12)
           + box((0, 360, 110), (50, 60, 40)))
    add_roll(m, arm, (0, 90, 180, 270))
    add_roll(P["Reactor"], sphere_y((0, 628, 190), 12, 10, 12, N=10, rings=5), (0, 90, 180, 270))
    win = P["Windows"]
    ring_windows(win, -330, 121, 16, size=(18, 30, 4))
    e = P["Engine"]
    e.add(tube((0, -470, 0), (0, -520, 0), 100, 100, N=16))
    nozzle(e, 0, -520, 0, 50, 60)
    for ang in (45, 135, 225, 315):
        a = math.radians(ang)
        nozzle(e, 78 * math.sin(a), -520, 78 * math.cos(a), 26, 44)
    s = P["Sensor"]
    add_sym(s, tube((0, -380, 120), (0, -380, 190), 4, 2.5, N=6))
    rolled_turret(P["Weapon"], -150, 120, 90, 16, 50, br=3.5)
    rolled_turret(P["Weapon"], -150, 120, -90, 16, 50, br=3.5)
    return result(P, {"NavGreen": (-124, 416, -124), "NavRed": (124, 416, -124),
                      "NavWhite": (0, -600, 0), "NavBeacon": (0, -380, 198)}, 10)


def design_heavyhauler():
    """Behemoth: a cab module pushing an open truss packed with containers on
    all four sides, clamp frames, pusher engine block with X radiators."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    truss_y(c, 520, -560, 42, 108, r=7)
    c.add(loft([(780, 10, 10, 0), (720, 100, 100, 0), (620, 160, 160, 0), (560, 160, 160, 0), (530, 120, 120, 0)],
               N=12, sq=2.4))
    for y in (520, -560):                                                   # clamp frames
        for sz in (-1, 1):
            c.add(box((0, y, 172 * sz), (360, 22, 22)))
            c.add(box((172 * sz, y, 0), (22, 22, 360)))
    k = P["Cargo"]
    rng = random.Random(9401)
    for y in range(-490, 470, 106):
        for cx in (-112, 0, 112):
            for cz in (-112, 0, 112):
                if (cx, cz) == (0, 0) or rng.random() < 0.1:
                    continue
                k.add(box((cx, y, cz), (104, 98, 104)))
    win = P["Windows"]
    ring_windows(win, 690, 128, 18, size=(20, 26, 4))
    e = P["Engine"]
    e.add(cbox((0, -620, 0), (300, 100, 300), 1.0))
    for sx in (-1, 1):
        for sz in (-1, 1):
            nozzle(e, 80 * sx, -670, 80 * sz, 48, 70)
    add_roll(c, prism_yz([(-560, 150), (-620, 330), (-700, 330), (-690, 150)], -5, 5), (45, 135, 225, 315))
    w = P["Weapon"]
    for roll in (0, 90, 180, 270):
        rolled_turret(w, 600, 160, roll, 16, 50, br=3.5)
    P["Reactor"].add(torus_y((0, -560, 0), 175, 8, N=24, M=6))
    s = P["Sensor"]
    add_sym(s, tube((0, 740, 60), (0, 800, 140), 4, 2.5, N=6))
    return result(P, {"NavGreen": (-236, -690, -236), "NavRed": (236, -690, -236),
                      "NavWhite": (0, -760, 0), "NavBeacon": (0, 800, 146)}, 10)


def design_bulkcarrier():
    """Space carrier: armoured core hull flanked by two flight pods with open
    hangar mouths fore and aft, outrigger pylons, turrets above and below."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(loft([(840, 10, 10, 0), (720, 80, 70, 0), (300, 130, 110, 0), (-600, 140, 120, 0), (-800, 120, 100, 0)],
               N=8, sq=2.4))
    for sx in (-1, 1):
        c.add(loft([(740, 100, 70, 0, 320 * sx), (720, 112, 86, 0, 320 * sx), (-660, 112, 86, 0, 320 * sx),
                    (-690, 100, 70, 0, 320 * sx)], N=8, sq=4.0))           # flight pods
        for y in (500, 150, -200, -500):
            c.add(loft([(y, 118, 92, 0, 320 * sx), (y - 16, 118, 92, 0, 320 * sx)], N=8, sq=4.0))
    for y, l in ((300, 140), (-350, 180)):
        add_sym(c, box((190, y, 20), (180, l, 16)), mirror=True)            # outrigger pylons
    for y in (400, -100, -500):
        collar(c, y, H.interp([(720, 80), (300, 130), (-600, 140)], y), H.interp([(720, 70), (300, 110), (-600, 120)], y))
    rng = random.Random(9501)
    for sx in (-1, 1):
        surf_greebles(c, rng, 22, -640, 700, [(-690, 112), (740, 112)], [(-690, 86), (740, 86)], 4.0,
                      xfrac=0.75, pos=(320 * sx, 0, 0), sx=(8, 20), sy=(14, 40), sz=(2, 6),
                      skip=lambda u, y, r: any(abs(y - q) < 30 for q in (560, -40, -560)) and u < 20)
    surf_greebles(c, rng, 26, -760, 640, [(720, 80), (300, 130), (-600, 140), (-800, 120)],
                  [(720, 70), (300, 110), (-600, 120), (-800, 100)], 2.4, xfrac=0.45,
                  skip=lambda u, y, r: y > 360 and u < 20)
    e = P["Engine"]
    for sx in (-1, 1):                                                      # hangar mouths
        e.add(box((320 * sx, 742, 0), (180, 6, 120)))
        e.add(box((320 * sx, -692, 0), (180, 6, 120)))
        for sz in (-1, 1):
            nozzle(e, (320 + 60) * sx, -690, 50 * sz, 22, 36)
    e.add(tube((0, -795, 0), (0, -830, 0), 90, 90, N=16))
    nozzle(e, 0, -830, 0, 60, 80)
    for ang in (0, 90, 180, 270):
        a = math.radians(ang + 45)
        nozzle(e, 92 * math.sin(a), -810, 92 * math.cos(a), 26, 44)
    r = P["Reactor"]
    for sx in (-1, 1):                                                      # launch-lane lights
        for sz in (-1, 1):
            r.add(box((320 * sx, 745, 64 * sz), (190, 4, 4)))
    win = P["Windows"]
    ring_windows(win, 620, 96, 14, size=(18, 28, 4))
    for sx in (-1, 1):
        for y in range(-600, 700, 70):
            win.add(box((432 * sx, y, 0), (4, 28, 12)))
    w = P["Weapon"]
    for sx in (-1, 1):
        for y in (560, -40, -560):
            for roll in (0, 180):
                tk = Kit()
                turret(tk, 320 * sx, y, 92, 14, (0, 1, 0.05), 44, br=3)
                for g in tk.groups:
                    w.add(g if roll == 0 else flipz(g))
    s = P["Sensor"]
    add_sym(s, tube((0, 400, 110), (0, 440, 190), 5, 3, N=6))
    return result(P, {"NavGreen": (-434, 700, 0), "NavRed": (434, 700, 0),
                      "NavWhite": (0, -930, 0), "NavBeacon": (0, 440, 196)}, 11)


def design_cruiser():
    """Hex-section cruiser: dorsal+ventral blades, four gun sponsons in a + layout
    (turrets face outward), glazed prow band, ring engine cluster."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    hw = [(900, 10), (760, 70), (400, 120), (-500, 130), (-800, 120), (-860, 100)]
    c.add(loft([(y, v, v * 0.9, 0) for y, v in hw], N=6, sq=2.0))
    for y in (500, 200, -100, -600):
        v = H.interp(hw, y)
        collar(c, y, v, v * 0.9, N=6, t=20, grow=9)
    add_sym(c, prism_yz([(700, 50), (300, 170), (-400, 185), (-720, 110), (-720, 60)], -8, 8))
    arm = prism_yz([(250, 100), (120, 300), (-250, 300), (-360, 100)], -14, 14)
    pod = loft([(200, 8, 8, 300), (150, 32, 32, 300), (-290, 32, 32, 300), (-330, 20, 20, 300)], N=8, sq=3.0)
    add_roll(c, arm + pod, (90, 270))
    add_roll(c, _xf(arm + pod, pos=(0, -120, 0)), (0, 180))
    w = P["Weapon"]
    for roll in (90, 270):
        rolled_turret(w, 60, 332, roll, 26, 120, br=6)
        rolled_turret(w, -170, 332, roll, 26, 120, br=6)
    for roll in (0, 180):
        rolled_turret(w, -60, 332, roll, 26, 120, br=6)
        rolled_turret(w, 560, 90, roll, 20, 100, br=5)
    w.add(tube((0, 880, 0), (0, 1010, 0), 12, 10, N=10))
    rng = random.Random(9601)
    win = P["Windows"]
    for roll in (30, 150, 210, 330):                                        # upper/lower slanted faces
        for y in range(-560, 380, 80):
            win.add(_xf(box((0, y, 0.80 * H.interp(hw, y) + 1), (26, 28, 4)), rot=(0, roll - 3 if roll < 180 else roll + 3, 0)))
        for _ in range(12):
            y = rng.uniform(-700, 560)
            u = rng.uniform(22, 0.3 * H.interp(hw, y)) * rng.choice((-1, 1))
            d = (rng.uniform(8, 18), rng.uniform(14, 36), rng.uniform(2, 5))
            c.add(_xf(box((u, y, 0.80 * H.interp(hw, y) + d[2] / 2 - 1), d), rot=(0, roll, 0)))
    ring_windows(win, 690, 0.80 * H.interp(hw, 690) + 1, 6, size=(40, 22, 4), start=30)
    e = P["Engine"]
    e.add(tube((0, -855, 0), (0, -900, 0), 104, 104, N=6))
    nozzle(e, 0, -900, 0, 56, 90)
    for k in range(6):
        a = math.radians(60 * k + 30)
        nozzle(e, 76 * math.sin(a), -900, 76 * math.cos(a), 22, 50)
    v = H.interp(hw, -400)
    P["Reactor"].add(loft([(-392, v + 4, v * 0.9 + 4, 0), (-408, v + 4, v * 0.9 + 4, 0)], N=6, sq=2.0))  # glow band
    s = P["Sensor"]
    add_sym(s, tube((0, -780, 110), (0, -820, 200), 4, 2, N=6))
    return result(P, {"NavGreen": (-332, -130, 0), "NavRed": (332, -130, 0),
                      "NavWhite": (0, -1000, 0), "NavBeacon": (0, -820, 206)}, 11)


def design_battleship():
    """Armoured diamond-section battleship: triple turrets top AND bottom, spinal
    gun out of the prow, broadside sponsons, 3x3 engine grid."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    st = [(900, 10, 8), (780, 90, 60), (450, 200, 130), (-400, 220, 150), (-780, 200, 140), (-850, 170, 120)]
    c.add(loft([(y, a, b, 0) for y, a, b in st], N=8, sq=1.6))
    for y in (560, 300, 0, -300, -600):
        collar(c, y, H.interp([(y_, a) for y_, a, _ in st], y), H.interp([(y_, b) for y_, _, b in st], y),
               N=8, sq=1.6, t=22, grow=10)
    for sx in (-1, 1):
        c.add(cbox((222 * sx, -120, 0), (70, 520, 110), 1.0))               # broadside sponsons
    add_roll(c, prism_yz([(-560, 130), (-700, 260), (-800, 260), (-790, 130)], -7, 7), (45, 135, 225, 315))
    rng = random.Random(9701)
    surf_greebles(c, rng, 50, -760, 700, [(y, a) for y, a, _ in st], [(y, b) for y, _, b in st], 1.6,
                  xfrac=0.5, skip=lambda u, y, r: u < 70 and (abs(y - 500) < 70 or abs(y - 260) < 70
                                                               or abs(y + 520) < 70), sx=(8, 22), sy=(14, 40), sz=(3, 7))
    w = P["Weapon"]
    for roll in (0, 180):
        rolled_triple(w, 500, 120, roll, 46, fwd=1)
        rolled_triple(w, 260, 144, roll, 46, fwd=1)
        rolled_triple(w, -520, 146, roll, 46, fwd=-1)
    for sx in (-1, 1):
        for y in (60, -120, -300):
            tk = Kit()
            turret(tk, 0, y, 0, 18, (0, 1, 0.05), 60, br=4)
            for g in tk.groups:
                w.add(_xf(g, pos=(257 * sx, 0, 0), rot=(0, 90 * sx, 0)))
    w.add(tube((0, 820, 0), (0, 1060, 0), 26, 22, N=12))                   # spinal gun
    r = P["Reactor"]
    for y in (870, 920, 970):
        r.add(tube((0, y, 0), (0, y + 12, 0), 32, 32, N=12))
    win = P["Windows"]
    for sx in (-1, 1):                                                      # prow bridge glazing
        win.add(_xf(box((0, 0, 0), (4, 120, 16)), pos=(120 * sx, 640, 40), rot=(0, 0, -16 * sx)))
        win.add(_xf(box((0, 0, 0), (4, 120, 16)), pos=(120 * sx, 640, -40), rot=(0, 0, -16 * sx)))
        for y in range(-340, 120, 60):
            win.add(box((258 * sx, y, 36), (4, 30, 10)))
    e = P["Engine"]
    e.add(cbox((0, -880, 0), (330, 60, 260), 1.0))
    for x in (-100, 0, 100):
        for z in (-80, 0, 80):
            nozzle(e, x, -910, z, 34, 60)
    s = P["Sensor"]
    add_sym(s, tube((0, 700, 50), (0, 800, 130), 4, 2, N=6))
    return result(P, {"NavGreen": (-260, -380, 0), "NavRed": (260, -380, 0),
                      "NavWhite": (0, -990, 0), "NavBeacon": (0, 800, 136)}, 11)


def design_commandxl():
    """Flagship: glazed command globe on the prow, diamond-section spine hull,
    four swept X fins carrying comm dishes, flank hangars, 7-nozzle cluster."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(sphere_y((0, 700, 0), 200, 220, 200, N=18, rings=9))             # command globe
    c.add(torus_y((0, 700, 0), 204, 10, N=36, M=6))
    st = [(560, 120, 110), (400, 200, 150), (-500, 260, 170), (-900, 230, 150), (-1000, 190, 130)]
    c.add(loft([(y, a, b, 0) for y, a, b in st], N=8, sq=1.5))
    for y in (300, 0, -300, -600):
        collar(c, y, H.interp([(y_, a) for y_, a, _ in st], y), H.interp([(y_, b) for y_, _, b in st], y),
               N=8, sq=1.5, t=24, grow=10)
    fin = prism_yz([(-240, 150), (-700, 520), (-860, 520), (-820, 150)], -10, 10)
    add_roll(c, fin, (45, 135, 225, 315))
    rng = random.Random(9801)
    surf_greebles(c, rng, 22, -900, 380, [(y, a) for y, a, _ in st], [(y, b) for y, _, b in st], 1.5,
                  rolls=(0, 90, 180, 270), xfrac=0.4, skip=lambda u, y, r: abs(y - 200) < 40 and u < 40,
                  sx=(10, 24), sy=(16, 44), sz=(3, 8))
    win = P["Windows"]
    for y, rad in ((700, 201), (640, 190), (760, 190), (590, 160), (810, 160)):
        ring_windows(win, y, rad, 20, size=(24, 16, 4))
    e = P["Engine"]
    e.add(box((262, -200, 0), (6, 300, 90)))                                # flank hangar mouths
    e.add(box((-262, -200, 0), (6, 300, 90)))
    e.add(tube((0, -995, 0), (0, -1040, 0), 150, 150, N=16))
    nozzle(e, 0, -1040, 0, 64, 100)
    for k in range(6):
        a = math.radians(60 * k)
        nozzle(e, 104 * math.sin(a), -1040, 104 * math.cos(a), 34, 70)
    s = P["Sensor"]
    for roll in (45, 135, 225, 315):
        dish = tube((0, -780, 520), (0, -780, 540), 10, 10, N=8) + tube((0, -780, 540), (0, -740, 590), 10, 70, N=24)
        s.add(_xf(dish, rot=(0, roll, 0)))
    s.add(tube((0, 920, 0), (0, 1060, 0), 5, 2, N=6))
    w = P["Weapon"]
    for roll in (0, 90, 180, 270):
        rolled_turret(w, 200, 156 if roll % 180 == 0 else 214, roll, 22, 80, br=5)
        rolled_turret(w, -400, 182, roll + 45, 18, 60, br=4)
    a_, b_ = H.interp([(y, a) for y, a, _ in st], -850), H.interp([(y, b) for y, _, b in st], -850)
    P["Reactor"].add(loft([(-842, a_ + 5, b_ + 5, 0), (-858, a_ + 5, b_ + 5, 0)], N=8, sq=1.5))   # glow band
    return result(P, {"NavGreen": (-373, -860, -373), "NavRed": (373, -860, -373),
                      "NavWhite": (0, -1150, 0), "NavBeacon": (0, 1070, 0)}, 13)


REDESIGNS = {"Gunship_02": design_gunship, "Escort_01": design_escort, "Courier_01": design_courier,
             "Smuggler_01": design_smuggler, "Trader_01": design_trader, "Freighter_01": design_freighter,
             "CargoFreighter_01": design_cargofreighter, "Corvette_01": design_corvette,
             "Miner_01": design_miner, "Destroyer_01": design_destroyer, "HeavyHauler_01": design_heavyhauler,
             "BulkCarrier_01": design_bulkcarrier, "Cruiser_01": design_cruiser,
             "Battleship_01": design_battleship, "CommandXL_01": design_commandxl}


def build(ship):
    """Export via the hero pipeline (base name SM_Ship_<cls>_01), then rename to the game name."""
    cls, num = ship.rsplit("_", 1)
    H.DESIGNS[cls] = lambda: fit_length(REDESIGNS[ship](), TARGET_LEN[ship])
    rep = H.build_ship(cls)
    if num != "01":
        for f in glob.glob(os.path.join(OUT, f"SM_Ship_{cls}_01_*")):
            os.replace(f, f.replace(f"SM_Ship_{cls}_01_", f"SM_Ship_{ship}_"))
        mp = os.path.join(OUT, f"SM_Ship_{ship}_hardpoints.json")
        if os.path.exists(mp):
            m = json.load(open(mp)); m["ship"] = f"SM_Ship_{ship}"; json.dump(m, open(mp, "w"), indent=2)
    return rep


def load_px(path):
    img = bpy.data.images.load(path)
    w, h = img.size
    px = np.array(img.pixels[:], dtype=np.float32).reshape(h, w, 4)
    bpy.data.images.remove(img)
    return px


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = [a for a in argv if not a.startswith("--")] or list(REDESIGNS)
    os.makedirs(os.path.join(RENDERS, "compare"), exist_ok=True)
    G.BASE = OUT
    heroes = []
    for ship in names:
        if "--no-fbx" not in argv:
            rep = build(ship)
            print(f"[redo] {ship}: tris={rep['assembled']['tris']} dims_bu={rep['assembled']['dims_bu']}")
        d = fit_length(REDESIGNS[ship](), TARGET_LEN[ship])
        new = C.render_sheet(ship, d, base=f"SM_Ship_{ship}", outdir=RENDERS)
        heroes.append(new)
        old_sheet = os.path.join(CURRENT, f"SM_Ship_{ship}_sheet.png")
        if os.path.exists(old_sheet):                  # before | after, hero shots
            old = load_px(old_sheet)[2:802, :1200]
            pair = np.empty((800, 2404, 4), np.float32); pair[:] = (0.12, 0.12, 0.12, 1)
            pair[:, :1200] = old
            pair[:, 1204:] = new
            C.save_px(pair, os.path.join(RENDERS, "compare", f"SM_Ship_{ship}_before_after.png"))
    if len(heroes) > 1:
        C.contact_sheet(heroes, os.path.join(RENDERS, "contact_sheet.png"), cols=4)
    for t in glob.glob(os.path.join(RENDERS, "_tmp", "*")):
        os.remove(t)
    print("REDO_DONE")


if __name__ == "__main__":
    main()
