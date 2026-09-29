"""
Adastrea ship CONCEPT generator: extra base hulls in deliberately different
styles, built with the same primitive kit as Tools/generate_hero_ships.py.

  Manta    - stealthy flying-wing / lifting body, engines buried in the
             trailing edge, split elevons, winglets.
  Halo     - hard-sci-fi explorer: lattice spine, ribbed hab ring on spokes,
             tank cluster, radiators, shadow shield, big dish.
  Mule     - industrial tug: boxy cab, roll cage, articulated clamp arms,
             four corner thruster pods.
  Monolith - brutalist warship: stacked terraces, vertical ribs, split bow
             jaws around a spinal gun, engine wall.
  Trident  - forked frigate: three prongs off a rear block, prong-tip cannons.

Same conventions as the hero ships (nose +Y, Z up, legacy "Blender units",
per-part FBX + _Assembled + _hardpoints.json), but output goes to
Assets/FBX/concepts/ so nothing lands in the folder the editor is bound to
until a design is picked. Each ship also gets a render sheet (3/4 hero shot +
top + side orthographic) in Assets/FBX/concepts/renders/, plus a contact sheet.

Usage (headless):
  blender -b --python Tools/generate_ship_concepts.py -- [Manta Halo ...]
  (no names = all)   --no-fbx  render only    --no-render  export only
"""
import os, sys, math, random, importlib.util
import bpy
import numpy as np
from mathutils import Vector, Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "generate_hero_ships", os.path.join(HERE, "generate_hero_ships.py"))
H = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(H)          # helpers + hero designs; main() is not run
G = H.G

Kit = H.Kit
loft, tube, prism, prism_yz, box, sphere_y, nozzle, turret, _xf = (
    H.loft, H.tube, H.prism, H.prism_yz, H.box, H.sphere_y, H.nozzle, H.turret, H._xf)

OUT = os.path.normpath(os.path.join(HERE, "..", "Assets", "FBX", "concepts"))
RENDERS = os.path.join(OUT, "renders")


def torus_y(c, R, r, N=32, M=10):
    """Ring around the Y axis (hab rings, collars)."""
    cx, cy, cz = c
    grid = []
    for i in range(N):
        a = 2 * math.pi * i / N
        row = []
        for j in range(M):
            b = 2 * math.pi * j / M
            rr = R + r * math.cos(b)
            row.append((cx + rr * math.cos(a), cy + r * math.sin(b), cz + rr * math.sin(a)))
        grid.append(row)
    return [[grid[i][j], grid[(i + 1) % N][j], grid[(i + 1) % N][(j + 1) % M], grid[i][(j + 1) % M]]
            for i in range(N) for j in range(M)]


STD_MATS = {"Carcass": "M_Hull", "Engine": "M_Engine_Block", "Weapon": "M_Weapon_Block",
            "Sensor": "M_Sensor_Block", "Reactor": "M_Reactor_Block", "Cargo": "M_Cargo_Hold",
            "Windows": "M_ShipWindow_LitCool"}


def parts(*names):
    return {k: Kit() for k in names}


def result(P, nav, nav_r):
    return dict(parts=P, mats={k: STD_MATS[k] for k in P}, nav=nav, nav_r=nav_r)


# ----------------------------------------------------------------------------
# Designs (nose +Y)
# ----------------------------------------------------------------------------
def cbox(c, s, t=0.9):
    """Box with its top face shrunk by t: reads as a chamfered armour block."""
    x, y, z = c
    sx, sy, sz = s
    return prism([(x - sx / 2, y - sy / 2), (x + sx / 2, y - sy / 2), (x + sx / 2, y + sy / 2),
                  (x - sx / 2, y + sy / 2)], z - sz / 2, z + sz / 2, s_top=t)


def mirrored(half):
    """Close a +X half planform (nose first, ending on the centreline) into a full one."""
    return half + [(-x, y) for x, y in reversed(half[1:-1])]


def truss_y(kit, y0, y1, h, step, r=4):
    """Square open lattice along -Y from y0 to y1: 4 longerons, frames, diagonals."""
    for sx in (-1, 1):
        for sz in (-1, 1):
            kit.add(tube((sx * h, y0, sz * h), (sx * h, y1, sz * h), r, r, N=6))
    n = max(1, int(round((y0 - y1) / step)))
    for i in range(n + 1):
        y = y0 - (y0 - y1) * i / n
        for sz in (-1, 1):
            kit.add(box((0, y, sz * h), (2 * h, r * 1.6, r * 1.6)))
            kit.add(box((sz * h, y, 0), (r * 1.6, r * 1.6, 2 * h)))
        if i < n:
            yb = y0 - (y0 - y1) * (i + 1) / n
            f = 1 if i % 2 else -1
            for sx in (-1, 1):
                kit.add(tube((sx * h, y, -f * h), (sx * h, yb, f * h), r * 0.7, r * 0.7, N=5))
                kit.add(tube((-f * h, y, sx * h), (f * h, yb, sx * h), r * 0.7, r * 0.7, N=5))


def design_manta():
    """Stealth flying wing: stepped (chamfered) wing layers, buried nacelles,
    split elevons, dorsal spine ridge, wing gun pods."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    half = [(0, 430), (90, 400), (190, 320), (300, 190), (410, 40), (470, -90), (455, -150),
            (360, -120), (250, -180), (140, -250), (0, -220)]
    c.add(prism(mirrored(half), -5, 6, s_top=0.96))                                    # wing skin
    c.add(prism(mirrored([(x * 0.8, y * 0.88 + 12) for x, y in half]), 6, 11, s_top=0.95))   # upper step
    c.add(prism(mirrored([(x * 0.76, y * 0.84 + 12) for x, y in half]), -10, -5, s_top=1.05))  # lower step
    c.add(loft([(420, 8, 5, 2), (330, 60, 20, 6), (160, 120, 40, 10), (-60, 130, 42, 10),
                (-200, 90, 30, 6), (-235, 50, 16, 4)], N=16, sq=2.2))                # centre body
    c.add(prism_yz([(200, 44), (120, 56), (-150, 55), (-225, 30), (-225, 20), (200, 20)], -4, 4))  # spine
    c.add(prism([(130, 110), (150, 110), (410, -20), (395, -36)], 10, 14), mirror=True)            # strakes
    c.add(prism([(150, -238), (245, -186), (252, -200), (157, -252)], -3, 4), mirror=True)         # elevons
    c.add(prism([(262, -176), (350, -128), (356, -142), (268, -190)], -3, 4), mirror=True)
    c.add(prism_yz([(-60, 8), (-150, 64), (-176, 64), (-150, 8)], 452, 460), mirror=True)          # winglets
    c.add(tube((463, -40, 0), (463, -175, 0), 7, 5, N=8), mirror=True)                            # tip pods
    for sx in (-1, 1):                                                        # buried nacelles + scoops
        c.add(loft([(40, 12, 10, 12, 72 * sx), (-40, 32, 24, 10, 72 * sx), (-200, 34, 26, 7, 72 * sx),
                    (-228, 30, 24, 6, 72 * sx)], N=12, sq=3.0))
    c.add(loft([(40, 4, 4, 30, 72), (10, 22, 10, 32, 72), (-110, 26, 14, 28, 72)], N=8, sq=3), mirror=True)
    c.add(box((52, 110, -12), (56, 150, 3)), mirror=True)                                         # bay doors
    c.add(box((0, 280, 50), (4, 116, 3)))                                                         # canopy frame
    for y in (250, 312):
        c.add(box((0, y, 46), (48, 4, 5)))
    rng = random.Random(4401)
    H.greebles(c, rng, 60, -190, 300, [(y, x * 0.8) for x, y in half], [(-300, 11), (430, 11)], frac=0.9,
               skip=lambda x, y: x < 150 or abs(x - 250) < 24, sx=(8, 22), sy=(10, 34), sz=(1.5, 3))
    P["Windows"].add(sphere_y((0, 280, 34), 26, 70, 16, N=12, rings=6))
    e = P["Engine"]
    for sx in (-1, 1):
        e.add(tube((72 * sx, -222, 7), (72 * sx, -234, 7), 30, 30, N=12))
        nozzle(e, 72 * sx, -234, 7, 22, 30)
    w = P["Weapon"]
    w.add(loft([(150, 4, 4, 0, 250), (175, 11, 9, 0, 250), (250, 12, 9, 0, 250), (275, 6, 5, 0, 250)],
               N=8, sq=2.5), mirror=True)
    w.add(tube((250, 270, 0), (250, 345, 0), 4, 3.2, N=8), mirror=True)
    P["Sensor"].add(sphere_y((0, 60, -14), 34, 40, 14, N=12, rings=6))
    r = P["Reactor"]
    for k in range(4):
        r.add(box((24, -60 - k * 28, 51), (16, 14, 4)), mirror=True)
    return result(P, {"NavGreen": (-466, -110, 1), "NavRed": (466, -110, 1),
                      "NavWhite": (0, -272, 8), "NavBeacon": (0, -40, 60)}, 5)


def design_halo():
    """Hard-sci-fi explorer: open lattice spine, ribbed hab ring on spokes with
    tension cables, strapped tank cluster, segmented radiators, shadow shield,
    gimballed main drive, dish on the command module."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    truss_y(c, 470, -600, 26, 90)
    c.add(loft([(760, 10, 10, 0), (720, 52, 52, 0), (640, 76, 76, 0), (520, 78, 78, 0), (480, 56, 56, 0)],
               N=16, sq=2.0))                                                # command module
    c.add(tube((0, 482, 0), (0, 440, 0), 56, 36, N=16))                     # adapter to truss
    c.add(torus_y((0, 764, 0), 14, 4, N=16, M=6))                           # docking ring
    for k in range(4):                                                      # RCS quads
        c.add(_xf(box((0, 700, 60), (16, 16, 12)), rot=(0, 45 + 90 * k, 0)))
    c.add(tube((0, 60, 0), (0, -60, 0), 90, 90, N=20))                      # ring hub + bearings
    for y in (74, -62):
        c.add(tube((0, y, 0), (0, y - 12, 0), 102, 102, N=20))
    c.add(torus_y((0, 0, 0), 420, 34, N=48, M=10))                          # hab ring
    for k in range(12):                                                     # ring ribs
        c.add(_xf(box((0, 0, 420), (12, 84, 84)), rot=(0, k * 30, 0)))
    for ang in (45, 135, 225, 315):                                         # spokes + lift collars
        c.add(_xf(tube((0, 0, 100), (0, 0, 388), 13, 13, N=8), rot=(0, ang, 0)))
        c.add(_xf(tube((0, 0, 230), (0, 0, 262), 22, 22, N=8), rot=(0, ang, 0)))
    for ang in (0, 90, 180, 270):                                           # tension cables
        c.add(_xf(tube((0, 40, 100), (0, 0, 388), 2.5, 2.5, N=4), rot=(0, ang, 0)))
        c.add(_xf(tube((0, -40, 100), (0, 0, 388), 2.5, 2.5, N=4), rot=(0, ang, 0)))
    for sx in (-1, 1):                                                      # segmented radiators
        c.add(tube((30 * sx, -310, 0), (385 * sx, -310, 0), 6, 5, N=8))
        for x in (110, 215, 320):
            c.add(box((x * sx, -310, 0), (98, 210, 3)))
            c.add(box((x * sx, -205, 0), (98, 5, 7)))
            c.add(box((x * sx, -415, 0), (98, 5, 7)))
    c.add(tube((0, -436, 0), (0, -452, 0), 46, 132, N=24))                  # shadow shield
    c.add(tube((0, -470, 0), (0, -560, 0), 62, 62, N=16))                   # reactor casing
    win = P["Windows"]
    for i in range(24):
        win.add(_xf(box((0, 0, 455), (34, 26, 4)), rot=(0, 7.5 + i * 15, 0)))
    for k in range(16):
        win.add(_xf(box((0, 600, 77), (16, 30, 4)), rot=(0, k * 22.5, 0)))
    win.add(box((0, 745, 22), (40, 10, 12)))
    k = P["Cargo"]
    for i in range(6):
        a = math.radians(60 * i + 30)
        x, z = 105 * math.cos(a), 105 * math.sin(a)
        k.add(sphere_y((x, 300, z), 55, 70, 55, N=14, rings=7))
        k.add(torus_y((x, 300, z), 56, 3, N=16, M=4))
        c.add(tube((x * 0.28, 300, z * 0.28), (x * 0.55, 300, z * 0.55), 5, 5, N=6))
    for y in (245, 355):
        c.add(torus_y((0, y, 0), 105, 5, N=24, M=5))
    r = P["Reactor"]
    for y in (-482, -512, -542):
        r.add(tube((0, y, 0), (0, y - 7, 0), 66, 66, N=16))
    e = P["Engine"]
    e.add(cbox((0, -630, 0), (170, 70, 170), 0.88))
    nozzle(e, 0, -665, 0, 70, 120)
    for sx in (-1, 1):
        for sz in (-1, 1):
            e.add(tube((66 * sx, -640, 66 * sz), (52 * sx, -712, 52 * sz), 6, 6, N=6))   # gimbal struts
            nozzle(e, 78 * sx, -660, 78 * sz, 14, 34)
    s = P["Sensor"]
    s.add(tube((0, 600, 76), (0, 600, 170), 6, 5, N=8))
    s.add(tube((0, 600, 170), (0, 632, 215), 12, 95, N=28))                # dish
    s.add(_xf(torus_y((0, 0, 0), 95, 3, N=28, M=4), pos=(0, 632, 215), rot=(55, 0, 0)))  # dish rim
    s.add(tube((0, 632, 215), (0, 690, 290), 3, 2, N=6))                   # feed horn
    s.add(sphere_y((0, 690, 290), 8, 8, 8, N=8, rings=4))
    s.add(tube((0, -30, -100), (0, -30, -170), 3, 2, N=6))                 # ventral whip
    w = P["Weapon"]
    H.turret(w, 0, 540, 76, 16, (0, 1, 0.2), 40, twin=True, br=3)
    H.turret(w, 0, -150, 30, 12, (0, -1, 0.2), 32, twin=True, br=2.5)
    return result(P, {"NavGreen": (-456, 0, 0), "NavRed": (456, 0, 0),
                      "NavWhite": (0, -800, 0), "NavBeacon": (0, 520, 84)}, 9)


def design_mule():
    """Industrial tug: chamfered body + cab with a real windshield, roll cage with
    work lights, articulated clamp arms (elbow drum, twin rams, jaw teeth),
    winch drum, landing skids."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Cargo", "Windows")
    c = P["Carcass"]
    c.add(prism([(-100, -220), (100, -220), (100, 200), (-100, 200)], -55, 60, s_top=0.88))
    c.add(prism([(-115, 190), (115, 190), (115, 320), (80, 380), (-80, 380), (-115, 320)], -70, 90, s_top=0.86))
    c.add(box((0, 362, 94), (150, 34, 6)))                                  # visor
    for y in (-170, -80, 10, 100):                                          # side armour plates
        c.add(box((97, y, 0), (6, 80, 76)), mirror=True)
    for sx in (-1, 1):                                                      # roll cage
        c.add(tube((95 * sx, -200, 76), (95 * sx, 170, 76), 6, 6, N=6))
        for y in (-200, 20, 170):
            c.add(tube((95 * sx, y, 50), (95 * sx, y, 76), 6, 6, N=6))
    for y in (-200, 20, 170):
        c.add(tube((-95, y, 76), (95, y, 76), 6, 6, N=6))
    c.add(tube((-60, -160, 76), (60, -160, 76), 16, 16, N=12))             # winch drum
    for x in (-66, 66):
        c.add(box((x, -160, 68), (8, 36, 28)))
    c.add(cbox((150, 230, -10), (76, 96, 76), 0.85), mirror=True)           # clamp shoulders
    c.add(tube((132, 300, -10), (196, 300, -10), 30, 30, N=12), mirror=True)  # elbow drum
    c.add(cbox((165, 400, -10), (44, 180, 44), 0.9), mirror=True)           # forearm
    c.add(prism([(143, 490), (187, 490), (172, 565), (118, 592), (108, 572)], -34, 14), mirror=True)
    for x, y in ((136, 505), (127, 530), (117, 556)):                       # jaw teeth
        c.add(box((x, y, -10), (14, 10, 22)), mirror=True)
    for z0, z1 in ((40, 20), (-58, -38)):                                   # twin hydraulic rams
        c.add(tube((125, 250, z0), (142, 360, (z0 + z1) / 2), 10, 10, N=8), mirror=True)
        c.add(tube((142, 360, (z0 + z1) / 2), (160, 470, z1), 6, 5, N=8), mirror=True)
    for y in (-120, 100):                                                   # skids
        c.add(tube((80, y, -56), (80, y, -114), 5, 5, N=6), mirror=True)
    c.add(box((80, -10, -118), (12, 300, 8)), mirror=True)
    rng = random.Random(5501)
    H.greebles(c, rng, 16, -190, 180, [(-220, 80), (200, 80)], [(-220, 60), (200, 60)], frac=0.95,
               skip=lambda x, y: -100 < y < -20 or abs(y + 160) < 26 or abs(y - 20) < 10,
               sx=(8, 18), sy=(8, 22), sz=(3, 7))
    win = P["Windows"]
    win.add(box((0, 371, 42), (128, 4, 56), rot=(4.6, 0, 0)))              # windshield
    c.add(box((44, 373, 42), (6, 6, 58), rot=(4.6, 0, 0)), mirror=True)    # mullions
    win.add(box((104, 290, 56), (6, 70, 28)), mirror=True)
    e = P["Engine"]
    for sx in (-1, 1):
        for sz in (-1, 1):
            e.add(cbox((120 * sx, -170, 50 * sz), (56, 110, 46), 0.85))
            e.add(box((120 * sx, -114, 50 * sz), (44, 4, 30)))             # intake grille
            nozzle(e, 120 * sx, -225, 50 * sz, 17, 28)
    e.add(cbox((0, -240, 0), (120, 50, 80), 0.88))
    nozzle(e, -35, -265, 0, 26, 40)
    nozzle(e, 35, -265, 0, 26, 40)
    c.add(tube((-80, -60, 64), (80, -60, 64), 30, 30, N=14))              # reactor tank
    for x in (-54, -3, 48):
        P["Reactor"].add(tube((x, -60, 64), (x + 6, -60, 64), 32, 32, N=14))   # glow bands
    k = P["Cargo"]
    k.add(cbox((0, -10, -82), (150, 260, 50), 1.06))
    for y in (-100, -10, 80):
        k.add(box((0, y, -82), (158, 10, 54)))
    s = P["Sensor"]
    s.add(box((70, 330, 100), (24, 14, 14)), mirror=True)
    s.add(box((95, 170, 88), (14, 14, 10)), mirror=True)                    # cage work lights
    s.add(tube((0, 215, 86), (0, 215, 150), 3, 2, N=6))
    H.turret(P["Weapon"], 0, 290, 88, 14, (0, 1, 0.05), 40, twin=False, br=3)
    return result(P, {"NavGreen": (-116, 594, -10), "NavRed": (116, 594, -10),
                      "NavWhite": (0, -310, 0), "NavBeacon": (0, 215, 158)}, 6)


def design_monolith():
    """Brutalist warship: stacked terraces with surface detail, vertical ribs and
    an armour belt, hangar doors, chamfered bow jaws around a spinal gun with
    glowing coils, secondary broadside batteries, cowled engine wall."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(prism([(-150, 640), (150, 640), (150, -850), (-150, -850)], -170, 70, s_top=0.98))
    c.add(prism([(40, 640), (150, 640), (110, 960), (40, 980)], -150, 50, s_top=0.94), mirror=True)
    c.add(prism([(-115, 500), (115, 500), (115, -800), (-115, -800)], 70, 150, s_top=0.97))
    for y, sx in ((700, 70), (800, 64), (900, 56)):                        # jaw armour plates
        c.add(box((95, y, 51), (sx, 70, 4)), mirror=True)
        c.add(box((95, y + 40, -151), (sx, 10, 4)), mirror=True)
    c.add(prism([(-80, 150), (80, 150), (80, -720), (-80, -720)], 150, 230, s_top=0.96))
    c.add(prism([(-55, -330), (55, -330), (55, -560), (-55, -560)], 230, 300, s_top=0.9))   # bridge
    c.add(box((0, -420, 262), (170, 50, 20)))                              # bridge wings
    c.add(prism([(-92, 400), (92, 400), (92, -680), (-92, -680)], -250, -170, s_top=1.08))  # ventral
    for y in range(-700, 500, 150):
        c.add(cbox((158, y, -50), (18, 40, 220), 0.8), mirror=True)       # vertical ribs
    c.add(box((153, -100, -150), (8, 1500, 14)), mirror=True)             # armour belt
    rng = random.Random(6601)
    H.greebles(c, rng, 44, -780, 480, [(-800, 112), (500, 112)], [(-800, 150), (500, 150)], frac=1.0,
               skip=lambda x, y: (x < 88 and y < 160) or (abs(x - 96) < 30 and abs(y + 250) < 30)
               or (x < 44 and y > 180), sx=(8, 20), sy=(12, 40), sz=(3, 8))
    H.greebles(c, rng, 26, -700, 130, [(-720, 76), (150, 76)], [(-720, 230), (150, 230)], frac=1.0,
               skip=lambda x, y: (x < 62 and -580 < y < -310) or (x < 56 and abs(y + 620) < 26),
               sx=(8, 18), sy=(12, 36), sz=(3, 7))
    H.greebles(c, rng, 30, -830, 620, [(-850, 147), (640, 147)], [(-850, 70), (640, 70)], frac=1.0,
               skip=lambda x, y: x < 120, sx=(6, 12), sy=(14, 40), sz=(3, 6))
    win = P["Windows"]
    win.add(box((0, -337, 270), (90, 6, 12)))
    for y in range(-600, 400, 100):
        win.add(box((150, y, 30), (4, 50, 8)), mirror=True)
    w = P["Weapon"]
    w.add(tube((0, 600, -50), (0, 1060, -50), 26, 22, N=14))              # spinal gun
    w.add(tube((0, 1060, -50), (0, 1100, -50), 34, 34, N=14))
    H.turret(w, 0, 380, 150, 34, (0, 1, 0.03), 170, br=8)
    H.turret(w, 0, 230, 150, 30, (0, 1, 0.03), 150, br=7)
    H.turret(w, 96, -250, 150, 20, (0, 1, 0.03), 90, br=5, mirror=True)
    for y in (380, 60, -260, -560):                                       # broadside batteries
        H.turret(w, 132, y, 70, 12, (1, 0.35, 0.05), 50, br=3, mirror=True)
    e = P["Engine"]
    e.add(box((0, -880, -40), (340, 80, 380)))
    for x in (-100, 0, 100):
        for z in (-140, 40):
            e.add(box((x, -925, z), (92, 14, 92)))                         # nozzle cowls
            nozzle(e, x, -932, z, 38, 64)
    e.add(box((151, 300, -80), (4, 170, 80)), mirror=True)                # hangar doors
    c.add(box((153, 300, -80), (6, 186, 8)), mirror=True)
    r = P["Reactor"]
    for y in (700, 780, 860, 940):                                        # gun coils (glow)
        r.add(tube((0, y, -50), (0, y + 20, -50), 34, 34, N=14))
    r.add(box((0, -620, 232), (100, 40, 10)))
    r.add(box((152, -200, -120), (6, 900, 10)), mirror=True)
    s = P["Sensor"]
    s.add(box((0, -445, 300), (40, 40, 10)))
    s.add(box((0, -445, 380), (10, 60, 160)))
    s.add(sphere_y((0, -520, 300), 20, 20, 14, N=10, rings=5))
    for z in (340, 400):
        s.add(box((0, -445, z), (70, 8, 6)))
    return result(P, {"NavGreen": (-162, 640, -40), "NavRed": (162, 640, -40),
                      "NavWhite": (0, -960, 0), "NavBeacon": (0, -445, 472)}, 12)


def design_trident():
    """Forked frigate: three prongs with armour collars and X-braced bracing, a
    raised command deck with turrets, side engine pods, prong-tip cannons."""
    P = parts("Carcass", "Engine", "Weapon", "Sensor", "Reactor", "Windows")
    c = P["Carcass"]
    c.add(loft([(-700, 120, 70, 0), (-620, 165, 92, 0), (-320, 180, 95, 0), (-160, 150, 72, 0),
                (-90, 110, 50, 0)], N=12, sq=3.5))
    cp = [(-200, 62, 50, 0), (250, 56, 46, 0), (650, 40, 32, 0), (900, 12, 10, 0)]
    sp = [(-260, 50, 42, 0, 200), (150, 46, 36, 0, 220), (480, 34, 26, 0, 210), (640, 10, 8, 0, 190)]
    c.add(loft(cp, N=10, sq=3.0))
    c.add(loft(sp, N=10, sq=3.0), mirror=True)
    for y in (80, 300, 520):                                              # prong armour collars
        hw, hh = H.interp([(s[0], s[1]) for s in cp], y), H.interp([(s[0], s[2]) for s in cp], y)
        c.add(loft([(y, hw + 5, hh + 5, 0), (y + 18, hw + 5, hh + 5, 0)], N=10, sq=3.0))
    for y in (40, 260, 440):
        hw, hh = H.interp([(s[0], s[1]) for s in sp], y), H.interp([(s[0], s[2]) for s in sp], y)
        xc = H.interp([(s[0], s[4]) for s in sp], y)
        c.add(loft([(y, hw + 5, hh + 5, 0, xc), (y + 18, hw + 5, hh + 5, 0, xc)], N=10, sq=3.0), mirror=True)
    c.add(box((130, 0, 0), (150, 50, 24)), mirror=True)                   # braces
    c.add(box((130, 330, 0), (150, 36, 18)), mirror=True)
    c.add(tube((55, 20, 10), (178, 310, 10), 6, 6, N=6), mirror=True)
    c.add(tube((178, 20, -10), (55, 310, -10), 6, 6, N=6), mirror=True)
    c.add(prism([(-100, -230), (100, -230), (140, -620), (-140, -620)], 80, 108, s_top=0.9))   # command deck
    c.add(loft([(-190, 20, 8, 114), (-230, 58, 22, 118), (-380, 60, 24, 118), (-440, 30, 12, 114)], N=8, sq=3.2))
    c.add(prism_yz([(-380, 90), (-600, 190), (-680, 190), (-640, 90)], -6, 6))
    c.add(prism_yz([(-380, -80), (-600, -170), (-680, -170), (-640, -80)], -6, 6))
    for sx in (-1, 1):                                                    # side engine pods
        c.add(loft([(-400, 8, 8, 0, 190 * sx), (-450, 34, 34, 0, 190 * sx), (-690, 36, 36, 0, 190 * sx),
                    (-712, 30, 30, 0, 190 * sx)], N=12, sq=2.4))
    rng = random.Random(7701)
    H.greebles(c, rng, 34, -600, -250, [(-620, 120), (-230, 86)], [(-620, 108), (-230, 108)], frac=0.9,
               skip=lambda x, y: (x < 66 and y > -460) or (x * x + (y + 520) ** 2 < 50 ** 2)
               or (abs(x - 95) < 26 and abs(y + 330) < 26), sx=(8, 18), sy=(10, 30), sz=(3, 7))
    P["Windows"].add(box((0, -212, 120), (70, 10, 10)))
    for sx in (-1, 1):
        P["Windows"].add(box((60 * sx, -310, 118), (4, 90, 8)))
    w = P["Weapon"]
    w.add(loft([(560, 6, 6, 0, 190), (590, 16, 14, 0, 190), (660, 16, 14, 0, 190), (680, 10, 10, 0, 190)],
               N=8, sq=2.6), mirror=True)
    w.add(tube((190, 660, 0), (190, 790, 0), 10, 8, N=10), mirror=True)
    H.turret(w, 0, 520, 30, 22, (0, 1, 0.03), 120, br=5)
    H.turret(w, 0, 250, 44, 26, (0, 1, 0.03), 140, br=6)
    H.turret(w, 215, 200, 34, 18, (0, 1, 0.03), 90, br=4, mirror=True)
    H.turret(w, 95, -330, 108, 18, (0, 1, 0.03), 80, br=4, mirror=True)
    e = P["Engine"]
    e.add(cbox((0, -710, 0), (300, 40, 120), 0.9))
    nozzle(e, 0, -730, 0, 48, 70)
    for sx in (-1, 1):
        nozzle(e, 100 * sx, -730, 0, 34, 60)
        nozzle(e, 190 * sx, -712, 0, 26, 48)
    r = P["Reactor"]
    r.add(tube((0, -520, 108), (0, -520, 132), 40, 34, N=14))
    for sx in (-1, 1):
        r.add(box((190 * sx, -560, 36), (20, 160, 4)))                    # pod heat vents
    s = P["Sensor"]
    s.add(tube((0, -320, 140), (0, -320, 220), 4, 2.5, N=8))
    s.add(box((0, -320, 205), (70, 8, 5)))
    return result(P, {"NavGreen": (-245, 150, 0), "NavRed": (245, 150, 0),
                      "NavWhite": (0, -820, 0), "NavBeacon": (0, -320, 228)}, 8)


CONCEPTS = {"Manta": design_manta, "Halo": design_halo, "Mule": design_mule,
            "Monolith": design_monolith, "Trident": design_trident}


# ----------------------------------------------------------------------------
# Render sheet: clay materials, 3/4 hero + top + side ortho, stitched
# ----------------------------------------------------------------------------
CLAY = {"Carcass": ((0.60, 0.62, 0.65), 0.45, 0.35, 0), "Engine": ((0.16, 0.17, 0.19), 0.35, 0.85, 0),
        "Weapon": ((0.26, 0.27, 0.29), 0.4, 0.7, 0), "Sensor": ((0.78, 0.74, 0.60), 0.5, 0.2, 0),
        "Cargo": ((0.70, 0.42, 0.16), 0.55, 0.1, 0), "Reactor": ((1.0, 0.45, 0.12), 0.3, 0.0, 6),
        "Windows": ((0.45, 0.75, 1.0), 0.1, 0.0, 3)}


def clay_mat(name, col, rough, metal, emit):
    m = bpy.data.materials.new("Clay_" + name)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*col, 1)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    if emit:
        b.inputs["Emission Color"].default_value = (*col, 1)
        b.inputs["Emission Strength"].default_value = emit
    return m


def look_at(ob, target):
    ob.rotation_euler = (Vector(target) - ob.location).to_track_quat('-Z', 'Y').to_euler()


def render_to(cam, path, w, h):
    sc = bpy.context.scene
    sc.camera = cam
    sc.render.resolution_x, sc.render.resolution_y = w, h
    sc.render.filepath = path
    bpy.ops.render.render(write_still=True)
    img = bpy.data.images.load(path)
    px = np.array(img.pixels[:], dtype=np.float32).reshape(h, w, 4)
    bpy.data.images.remove(img)
    return px


def render_sheet(name, d, base=None, outdir=None):
    G.setup_scene()
    sc = bpy.context.scene
    S = Matrix.Scale(0.01, 4)                  # render in metres-ish so lights/shadows behave
    objs = []
    for suf, kit in d["parts"].items():
        if kit.empty():
            continue
        ob = H.kits_to_mesh(f"{name}_{suf}", [kit])
        ob.data.transform(S)
        ob.data.materials.append(clay_mat(suf, *CLAY[suf]))
        objs.append(ob)
    r = d["nav_r"]
    for nm, pos in d["nav"].items():
        k = Kit(); k.add(sphere_y(pos, r, r, r, N=10, rings=5))
        ob = H.kits_to_mesh(f"{name}_{nm}", [k])
        ob.data.transform(S)
        ob.data.materials.append(clay_mat(nm, H.NAV_COL[nm][1], 0.3, 0, 12))
        objs.append(ob)
    return render_objs(base or f"SM_Ship_{name}_01", objs, outdir or RENDERS)


def render_objs(base, objs, outdir):
    """3/4 hero + top + side ortho sheet of already-built, clay-shaded mesh objects
    (mesh data in ship space, scaled 0.01). Returns the hero image pixels."""
    sc = bpy.context.scene
    name = base
    pts = np.array([v.co[:] for o in objs for v in o.data.vertices])
    mn, mx = pts.min(0), pts.max(0)
    ctr = (mn + mx) / 2
    dims = mx - mn
    rad = np.linalg.norm(dims) / 2

    world = bpy.data.worlds.new("W"); sc.world = world; world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs[0].default_value = (0.035, 0.04, 0.055, 1); bg.inputs[1].default_value = 1.0
    for nm, rot, en in (("Key", (50, 10, 35), 4.0), ("Fill", (60, 0, -140), 1.0), ("Rim", (-60, 0, 180), 2.0)):
        ld = bpy.data.lights.new(nm, 'SUN'); ld.energy = en; ld.angle = math.radians(3)
        lo = bpy.data.objects.new(nm, ld); sc.collection.objects.link(lo)
        lo.rotation_euler = [math.radians(a) for a in rot]
    sc.eevee.taa_render_samples = 32
    sc.render.film_transparent = False

    def cam(nm, loc, ortho=None):
        cd = bpy.data.cameras.new(nm); cd.clip_start = 0.05; cd.clip_end = rad * 20 + 100
        if ortho:
            cd.type = 'ORTHO'; cd.ortho_scale = ortho
        else:
            cd.lens = 50
        co = bpy.data.objects.new(nm, cd); sc.collection.objects.link(co)
        co.location = loc
        return co

    tmp = os.path.join(outdir, "_tmp")
    os.makedirs(tmp, exist_ok=True)
    v = Vector((0.85, 1.0, 0.55)).normalized()
    hero = cam("Hero", Vector(ctr) + v * rad * 2.9)
    look_at(hero, ctr)
    top = cam("Top", Vector((ctr[0], ctr[1], mx[2] + rad * 2)), ortho=max(dims[1], dims[0] * 1.5) * 1.12)
    top.rotation_euler = (0, 0, math.radians(90))       # nose points right
    side = cam("Side", Vector((mx[0] + rad * 2, ctr[1], ctr[2])), ortho=max(dims[1], dims[2] * 1.5) * 1.12)
    side.rotation_euler = (math.radians(90), 0, math.radians(90))

    a = render_to(hero, os.path.join(tmp, name + "_hero.png"), 1200, 800)
    b = render_to(top, os.path.join(tmp, name + "_top.png"), 600, 400)
    s = render_to(side, os.path.join(tmp, name + "_side.png"), 600, 400)
    sheet = np.ones((804, 1804, 4), np.float32)
    sheet[..., :3] = 0.12
    sheet[2:802, :1200] = a
    sheet[404:804, 1204:] = b       # pixel rows are bottom-up: top view in the upper half
    sheet[0:400, 1204:] = s
    out = os.path.join(outdir, f"{base}_sheet.png")
    save_px(sheet, out)
    print(f"[sheet] {base}: dims_bu={[round(float(x) * 100) for x in dims]} sheet -> {out}")
    return a


def save_px(px, path):
    h, w = px.shape[:2]
    img = bpy.data.images.new(os.path.basename(path), w, h, alpha=True)
    img.pixels.foreach_set(px.ravel())
    img.filepath_raw = path
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)


def contact_sheet(heroes, path, cols=3):
    """Half-res hero shots in a grid, first row at the top."""
    rows = (len(heroes) + cols - 1) // cols
    cs = np.empty((rows * 400, cols * 600, 4), np.float32); cs[:] = heroes[0][0, 0]
    for i, hp in enumerate(heroes):
        rr, cc = divmod(i, cols)
        y0 = (rows - 1 - rr) * 400
        cs[y0:y0 + 400, cc * 600:(cc + 1) * 600] = hp[::2, ::2]
    save_px(cs, path)


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = [a for a in argv if not a.startswith("--")] or list(CONCEPTS)
    os.makedirs(RENDERS, exist_ok=True)
    G.BASE = OUT                                 # hero build_ship/export write here
    H.DESIGNS.update(CONCEPTS)
    heroes = []
    for n in names:
        if "--no-fbx" not in argv:
            rep = H.build_ship(n)
            print(f"[concept] {n}: assembled tris={rep['assembled']['tris']} parts={list(rep['parts'])}")
        if "--no-render" not in argv:
            heroes.append(render_sheet(n, CONCEPTS[n]()))
    if len(heroes) > 1:
        contact_sheet(heroes, os.path.join(RENDERS, "contact_sheet.png"))
    print("CONCEPTS_DONE")


if __name__ == "__main__":
    main()
