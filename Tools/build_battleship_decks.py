"""Battleship full-deck walkable interior (Blender 5.x, headless).

    blender -b --python Tools/build_battleship_decks.py -- [--dry-run]

A 71 m x 23 m military deck for the Battleship, in the grimy 1990s
"marine transport" style (Aliens / Starship Troopers / Space: Above and
Beyond) rather than a clean Trek bridge: ribbed chamfered corridors, grating,
yellow-black hazard edging, exposed pipe runs, caged red emergency lamps,
green-phosphor CRT consoles, olive-drab bulkheads.

Ship scale (2026-09-30 rework): 1.9 m spine passage under a 2.5 m deckhead,
8.7 x 5.7 m compartments with 2.7 m deckheads, a tumblehome outboard wall with
hull frames, and pipe runs, ducts and junction boxes in every compartment.
The first version had 4 m corridors and 20 x 12 x 4.2 m rooms and scored
Space 23 on Tools/interior_benchmark.py.

Layout, bow (+X) to stern:

    CIC (command deck, raised plot-table dais, helm under armoured slit windows)
    | Briefing (CRT video wall, seat rows)  |  Armory (caged rifle racks)       |
    | Mess + galley                         |  Medbay + 8-pod cryo ring          |
    | Berths (3-high bunk racks)            |  Drop-crew ready room (suit racks) |
      ^ all six open off the central ribbed spine corridor
    Hangar (13 m high: dropship on a pad, launch door, gantry crane, catwalk,
            stairs up to the flight-control booth)
    Engineering (reactor column, catwalk ring, aft gallery, ramp, coolant tanks)

Contract with ASpaceshipInterior (EShipInteriorFamily::BattleshipDecks):
  * Parts are SM_Int_Battleship_Decks_<Part> for Part in PARTS. They are exported
    100x (like every SM_Int_* kit, so the M_IntSurface_Oriented MIs tile the same);
    the C++ mounts them at a FIXED runtime scale of 0.01, so 1 design cm = 1 uu.
    It does not do the 650/SphereRadius shrink the single-room kits use.
  * "Collision" is walk collision only, never rendered: floors, stair ramps,
    catwalks, walls, railings and furniture blockers. It is imported
    complex-as-simple. The avatar sweeps against it and snaps to its floors.
    Stairs are visual steps over a smooth ramp; the avatar also steps up
    <= 45 cm (ASpaceshipInterior::WalkStepHeight), so dais steps work too.
  * Sockets (written into the contract JSON; Tools/import_battleship_decks.py
    adds them to the Shell mesh) are in UE design-cm, actor-local:
      Entry  = avatar spawn (x, y, z, yaw)     Seat = helm exit trigger
      L_<C>_<RR>_<NNN> / LS_... = point light, colour C (W/A/R/G/B), radius RR m,
      LS = casts shadows.
  * Axes: Blender +X = UE +X (bow), UE y = -Blender y (same exporter as the
    other kits). The sockets are stored already flipped.
"""
import bpy, bmesh, math, os, sys, json
from mathutils import Vector, Matrix

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build_capital_interiors import Part, rect, circle, GEN  # noqa: E402

PREFIX = "SM_Int_Battleship_Decks"
PARTS = ["Shell", "Spine", "CIC", "Quarters", "Mess", "Medbay", "Briefing", "Armory",
         "Hangar", "Dropship", "Engineering", "Lights", "Collision"]

# Material slots. The first group are existing kit MIs; the rest are added by
# Tools/import_battleship_decks.py.
SH, DK, CO, ST, LW, VP, HT, PI = ("M_Int_Shell", "M_Int_Deck", "M_Int_Console", "M_Int_Stations",
                                  "M_Int_Lights", "M_Int_Viewport", "M_Int_Hatch", "M_Int_Vents")
BU, ME, GA, EN = "M_Int_Bunks", "M_Int_Mess", "M_Int_Galley", "M_Interior_Eng"
GR, HZ, BH = "M_Int_Grate", "M_Int_Hazard", "M_Int_Bulkhead"
LR, LA, LG, LB = "M_Int_LightsRed", "M_Int_LightsAmber", "M_Int_LightsGreen", "M_Int_LightsBlue"
CL = "M_Int_Collision"

# Heights (cm). Ship scale: real warship deckheads give ~2.1-2.4 m clear; these
# leave room for the first-person camera (Tools/interior_benchmark.py targets).
H_BLOCK = 270      # spine-block rooms
H_SPINE = 250      # corridor ceiling (walls run on to H_BLOCK above it)
H_CIC = 330
H_BAY = 950        # hangar (dropship 6.2 m tall, gantry crane above it)
H_ENG = 900        # engineering
WT = 30            # wall thickness (centred on the wall line)

K = {}
SOCKETS = {}


def reset():
    K.clear()
    SOCKETS.clear()
    for p in PARTS:
        K[p] = Part(f"{PREFIX}_{p}")


# ----------------------------------------------------------------------------
# Primitive helpers
# ----------------------------------------------------------------------------
def Mx(x, y, z=0.0, yaw=0.0):
    return Matrix.Translation(Vector((x, y, z))) @ Matrix.Rotation(math.radians(yaw), 4, 'Z')


def box(part, slot, x0, x1, y0, y1, z0, z1, M=None):
    K[part].box(slot, min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1), min(z0, z1), max(z0, z1), M)


def col(x0, x1, y0, y1, z0, z1, M=None):
    box("Collision", CL, x0, x1, y0, y1, z0, z1, M)


def solid(part, slot, x0, x1, y0, y1, z0, z1, M=None):
    box(part, slot, x0, x1, y0, y1, z0, z1, M)
    col(x0, x1, y0, y1, z0, z1, M)


def extrude(part, slot, prof, axis, a0, a1, M=None):
    """Extrude a 2D profile along `axis`. prof is (u, v): for axis 'x' that is
    (y, z), for 'y' it is (x, z), for 'z' it is (x, y)."""
    M = M or Matrix()
    n = len(prof)

    def P(a, u, v):
        if axis == 'x':
            return Vector((a, u, v))
        if axis == 'y':
            return Vector((u, a, v))
        return Vector((u, v, a))
    verts = [M @ P(a0, u, v) for u, v in prof] + [M @ P(a1, u, v) for u, v in prof]
    faces = [list(range(n))[::-1], list(range(n, 2 * n))]
    for i in range(n):
        j = (i + 1) % n
        faces.append([i, j, n + j, n + i])
    K[part]._faces(verts, faces, slot)


def cyl(part, slot, cx, cy, z0, z1, r, segs=16, M=None):
    K[part].prism_xy(slot, circle(cx, cy, r, segs), z0, z1, M)


def pipe(part, slot, p0, p1, r, segs=10):
    p0 = Vector(p0); p1 = Vector(p1)
    d = p1 - p0
    L = d.length
    if L < 1e-3:
        return
    z = d.normalized()
    a = Vector((0, 0, 1)) if abs(z.z) < 0.9 else Vector((1, 0, 0))
    x = a.cross(z).normalized()
    y = z.cross(x)
    M = Matrix(((x[0], y[0], z[0], p0[0]), (x[1], y[1], z[1], p0[1]),
                (x[2], y[2], z[2], p0[2]), (0, 0, 0, 1)))
    K[part].prism_xy(slot, circle(0, 0, r, segs), 0.0, L, M)


def seg_box(part, slot, p0, p1, t, z0, z1):
    """Plan box of thickness t centred on the segment p0->p1."""
    a = Vector((p0[0], p0[1])); b = Vector((p1[0], p1[1]))
    e = (b - a).normalized()
    n = Vector((-e.y, e.x)) * (t / 2)
    K[part].prism_xy(slot, [tuple(a - n), tuple(b - n), tuple(b + n), tuple(a + n)], z0, z1)


def light(color, radius_m, x, y, z, shadow=False):
    """Point-light socket (UE design cm; y flipped here)."""
    n = sum(1 for k in SOCKETS if k.startswith("L"))
    name = "%s_%s_%02d_%03d" % ("LS" if shadow else "L", color, radius_m, n)
    SOCKETS[name] = [round(x, 1), round(-y, 1), round(z, 1), 0.0]


# ----------------------------------------------------------------------------
# Architecture: walls with openings, door frames, windows, ribs
# ----------------------------------------------------------------------------
def wall(part, slot, p0, p1, z0, z1, openings=(), t=WT, frames=True, collide=True):
    """Wall along the line p0->p1. openings: (s_centre, width, zb, zt) with s
    measured from p0; the wall is split around each."""
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    e = (b - a).normalized()
    at = lambda s: tuple(a + e * s)
    cur = 0.0
    ops = sorted(openings)
    for (sc, w, zb, zt) in ops:
        s0, s1 = sc - w / 2, sc + w / 2
        if s0 > cur:
            seg_box(part, slot, at(cur), at(s0), t, z0, z1)
            if collide:
                seg_box("Collision", CL, at(cur), at(s0), t, z0, z1)
        if zb > z0:
            seg_box(part, slot, at(s0), at(s1), t, z0, zb)
            if collide:
                seg_box("Collision", CL, at(s0), at(s1), t, z0, zb)
        if zt < z1:
            seg_box(part, slot, at(s0), at(s1), t, zt, z1)
            if collide:
                seg_box("Collision", CL, at(s0), at(s1), t, zt, z1)
        cur = s1
    if cur < L:
        seg_box(part, slot, at(cur), at(L), t, z0, z1)
        if collide:
            seg_box("Collision", CL, at(cur), at(L), t, z0, z1)
    if frames:
        yaw = math.degrees(math.atan2(e.y, e.x))
        for (sc, w, zb, zt) in ops:
            c = a + e * sc
            door_frame(part, c.x, c.y, zb, yaw, w, zt - zb, t)


def door_frame(part, cx, cy, zb, yaw, w, h, t=WT):
    """Heavy blast-door frame: jambs + header proud of both faces, chamfered
    top corners, hazard-striped jamb faces, threshold plate, status lamp."""
    M = Mx(cx, cy, zb, yaw)
    d = t / 2 + 12            # frame depth either side of the wall line
    jw = 26
    hw = w / 2
    for s in (-1, 1):
        x0, x1 = (hw, hw + jw) if s > 0 else (-hw - jw, -hw)
        box(part, BH, x0, x1, -d, d, 0, h + 34, M)
        xi = hw if s > 0 else -hw
        box(part, HZ, xi - 1.5 * s, xi, -d + 2, d - 2, 0, h, M)   # striped inner jamb face
        # chamfer gusset in the top corner of the opening
        g = min(40.0, hw * 0.4)
        prof = [(xi, h), (xi - g * s, h), (xi, h - g)]
        extrude(part, BH, prof, 'y', -d + 3, d - 3, M)
    box(part, BH, -hw - jw, hw + jw, -d, d, h, h + 34, M)            # header
    box(part, HZ, -hw, hw, -d + 2, d - 2, h - 1.5, h, M)            # striped soffit
    box(part, HZ, -hw, hw, -d, d, 0, 0.8, M)                         # threshold plate
    # status lamps over the header on both faces
    for s in (-1, 1):
        box(part, HT, -22, 22, s * d, s * (d + 5), h + 8, h + 26, M)
        box("Lights", LG if s > 0 else LR, -16, 16, s * (d + 5), s * (d + 6.5), h + 12, h + 22, M)


def window_band(part, p0, p1, z0, z1, zw0, zw1, mull=140.0, t=WT):
    """Wall along p0->p1 with a continuous armoured slit window zw0..zw1."""
    seg_box(part, SH, p0, p1, t, z0, zw0)
    seg_box(part, SH, p0, p1, t, zw1, z1)
    seg_box("Collision", CL, p0, p1, t, z0, z1)
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    e = (b - a).normalized()
    seg_box(part, VP, p0, p1, 4, zw0, zw1)
    n = max(1, int(round(L / mull)))
    for k in range(n + 1):
        c = a + e * (L * k / n)
        seg_box(part, BH, tuple(c - e * 7), tuple(c + e * 7), t + 16, zw0, zw1)
    # heavy sill + header, proud of the inner face
    seg_box(part, BH, p0, p1, t + 24, zw0 - 14, zw0)
    seg_box(part, BH, p0, p1, t + 24, zw1, zw1 + 16)


def ribs(part, p0, p1, side, z1, step=400.0, depth=16.0, w=34.0, skip=(), knee=True, slot=BH):
    """Vertical pilasters on one face of the wall line p0->p1 (side=+1 is the
    left of the direction), with a diagonal knee brace under the ceiling.
    skip: (s0, s1) ranges along the wall (e.g. door openings) left bare."""
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    e = (b - a).normalized()
    nrm = Vector((-e.y, e.x)) * side
    yaw = math.degrees(math.atan2(e.y, e.x))
    n = int(L // step)
    for k in range(1, n + 1):
        s = k * L / (n + 1)
        c = a + e * s
        if any(s0 - w < s < s1 + w for (s0, s1) in skip):
            continue
        M = Mx(c.x, c.y, 0, yaw)
        y0 = side * WT / 2
        y1 = side * (WT / 2 + depth)
        box(part, slot, -w / 2, w / 2, y0, y1, 0, z1, M)
        box(part, HT, -w / 2 - 2, w / 2 + 2, y0, side * (WT / 2 + depth + 3), 0, 18, M)   # foot
        if knee:
            kd = min(90.0, z1 * 0.18)
            prof = [(y1, z1), (y1 + side * kd, z1), (y1, z1 - kd * 1.4)]
            extrude(part, slot, prof, 'x', -w / 2 + 6, w / 2 - 6, M)


def railing(part, p0, p1, z, h=105.0, post=160.0, collide=True, kick=True):
    a = Vector((p0[0], p0[1], z)); b = Vector((p1[0], p1[1], z))
    L = (b - a).length
    n = max(1, int(L // post))
    for k in range(n + 1):
        c = a + (b - a) * (k / n)
        pipe(part, HT, c, c + Vector((0, 0, h)), 3.2, 8)
    up = Vector((0, 0, h))
    pipe(part, HZ, a + up, b + up, 3.0, 8)                       # hazard-yellow top rail
    pipe(part, HT, a + up * 0.55, b + up * 0.55, 2.2, 6)
    if kick:
        seg_box(part, HT, p0, p1, 2.0, z, z + 12)
    if collide:
        seg_box("Collision", CL, p0, p1, 8.0, z, z + h + 20)


def stairs(part, x0, y0, run, width, z0, z1, yaw, rails=(True, True), collide=True):
    """Open grated stair climbing local +X from (x0, y0) at z0 to z1.
    Visual steps; the collision is a smooth wedge ramp under the nosings."""
    M = Mx(x0, y0, 0, yaw)
    rise = z1 - z0
    n = max(1, int(math.ceil(rise / 20.0)))
    g = run / n
    hw = width / 2
    for i in range(n):
        zt = z0 + rise * (i + 1) / n
        box(part, GR, i * g - 2, (i + 1) * g + 2, -hw + 6, hw - 6, zt - 4, zt, M)
        box(part, HZ, (i + 1) * g - 4, (i + 1) * g + 2, -hw + 6, hw - 6, zt - 3.5, zt + 0.3, M)   # nosing
    for s in (-1, 1):
        prof = [(0, z0 - 20), (run, z1 - 20), (run, z1 + 10), (0, z0 + 10)]
        extrude(part, HT, prof, 'y', s * hw - 6 * (s > 0), s * hw + 6 * (s < 0), M)
        if rails[0 if s < 0 else 1]:
            for k in range(0, 5):
                t = k / 4
                c = M @ Vector((run * t, s * (hw - 3), z0 + rise * t))
                pipe(part, HT, c, c + Vector((0, 0, 100)), 3.0, 8)
            a = M @ Vector((0, s * (hw - 3), z0 + 100)); b = M @ Vector((run, s * (hw - 3), z1 + 100))
            pipe(part, HZ, a, b, 3.0, 8)
    if collide:
        # ramp wedge: surface through the step nosings
        extrude("Collision", CL, [(0, z0), (run, z1), (run, z0)], 'y', -hw, hw, M)
        for s in (-1, 1):
            if rails[0 if s < 0 else 1]:
                extrude("Collision", CL, [(0, z0), (run, z1), (run, z1 + 125), (0, z0 + 125)], 'y',
                        s * hw - 4, s * hw + 4, M)


# ----------------------------------------------------------------------------
# Props
# ----------------------------------------------------------------------------
def crt(part, x, y, z, w, h, yaw, screen=LG, d=None):
    """Chunky CRT monitor facing local +X: tapered tube housing, inset bezel."""
    M = Mx(x, y, z, yaw)
    d = d or w * 0.8
    box(part, ST, -d * 0.45, 0, -w / 2, w / 2, 0, h, M)
    box(part, ST, -d, -d * 0.45, -w * 0.32, w * 0.32, h * 0.12, h * 0.85, M)        # tube back
    box(part, HT, 0, 2.5, -w / 2 + 3, w / 2 - 3, 3, h - 3, M)                         # bezel
    box("Lights", screen, 2.5, 3.3, -w / 2 + 7, w / 2 - 7, 8, h - 7, M)
    box(part, HT, 0, 3, w / 2 - 12, w / 2 - 4, 1, 4, M)                              # knob strip


def console(part, x, y, yaw, n_crt=2, chair_too=True, collide=True, screen=LG, z=0.0):
    """Sloped 90s operator desk. Desk occupies local x -70..0, operator at +x."""
    M = Mx(x, y, z, yaw)
    w = 70.0 * n_crt + 30
    prof = [(-70, 0), (0, 0), (0, 70), (-6, 76), (-44, 104), (-70, 106)]
    extrude(part, ST, prof, 'y', -w / 2, w / 2, M)
    extrude(part, HT, [(-2, 0), (4, 0), (4, 10), (-2, 10)], 'y', -w / 2, w / 2, M)    # kick
    # slope inlay: switch banks + small status screens
    for k in range(n_crt * 2):
        yc = -w / 2 + 20 + k * (w - 40) / max(1, n_crt * 2 - 1)
        pa = Vector((-8, 0, 80)); pb = Vector((-40, 0, 101))
        Mk = M @ Matrix.Translation(Vector((0, yc, 0)))
        prof_i = [(pa.x, pa.z), (pb.x, pb.z), (pb.x - 1.2, pb.z + 1.8), (pa.x - 1.2, pa.z + 1.8)]
        extrude("Lights", CO if k % 2 else LA if k % 3 == 0 else LR, prof_i, 'y', -6, 6, Mk)
    for k in range(n_crt):
        yc = -w / 2 + 15 + 35 + k * 70
        c = M @ Vector((-40, yc, 106))
        crt(part, c.x, c.y, c.z, 56, 44, yaw, screen=screen, d=38)
    if chair_too:
        c = M @ Vector((50, 0, 0))
        chair(part, c.x, c.y, c.z, yaw)
    if collide:
        box("Collision", CL, -72, 2, -w / 2, w / 2, 0, 120, M)


def chair(part, x, y, z, yaw, big=False):
    """Pedestal crew chair; seat faces local -X (operator looks at local -X)."""
    M = Mx(x, y, z, yaw)
    s = 1.25 if big else 1.0
    cyl(part, HT, 0, 0, 0, 5, 26 * s, 10, M)
    cyl(part, HT, 0, 0, 5, 40, 5 * s, 8, M)
    box(part, BH, -24 * s, 24 * s, -24 * s, 24 * s, 40, 50, M)
    box(part, BH, 18 * s, 26 * s, -23 * s, 23 * s, 50, 100 * s + 10, M)
    box(part, ST, 20 * s, 27 * s, -12, 12, 100 * s + 10, 100 * s + 30, M)          # headrest
    for sy in (-1, 1):
        box(part, HT, -16 * s, 14 * s, sy * 24 * s, sy * 30 * s, 60, 66, M)        # armrests
        box(part, HT, 6 * s, 12 * s, sy * 24 * s, sy * 28 * s, 50, 60, M)


def ceiling_panel(part, x, y, z, yaw=0.0, length=130.0, color=LW):
    """Recessed fluorescent tube fixture hanging under a ceiling at z."""
    M = Mx(x, y, z, yaw)
    box(part, ST, -length / 2, length / 2, -20, 20, -9, 0, M)
    box("Lights", color, -length / 2 + 5, length / 2 - 5, -14, 14, -10.5, -9, M)
    for sx in (-1, 1):
        box(part, HT, sx * length / 2 - 3, sx * length / 2 + 3, -22, 22, -12, 0, M)


def cage_lamp(part, x, y, z, yaw, color=LR):
    """Wall-mounted caged emergency lamp facing local +X."""
    M = Mx(x, y, z, yaw)
    box(part, HT, -1, 4, -9, 9, -9, 9, M)
    box("Lights", color, 4, 12, -6, 6, -6, 6, M)
    for k in (-6, 0, 6):
        box(part, HT, 4, 14, k - 0.8, k + 0.8, -8, 8, M)


def flood(part, x, y, z, color=LA):
    """Ceiling floodlamp housing hanging from z."""
    box(part, HT, x - 4, x + 4, y - 4, y + 4, z - 60, z, None)
    box(part, ST, x - 45, x + 45, y - 45, y + 45, z - 95, z - 60)
    box(part, HT, x - 50, x + 50, y - 50, y + 50, z - 100, z - 95)
    box("Lights", color, x - 38, x + 38, y - 38, y + 38, z - 102, z - 100)


def crate(part, x, y, z, sx, sy, sz, yaw=0.0, lid=HZ, collide=True):
    M = Mx(x, y, z, yaw)
    box(part, BH, -sx / 2, sx / 2, -sy / 2, sy / 2, 0, sz, M)
    for q in (-1, 1):
        box(part, HT, q * sx / 2 - 3, q * sx / 2 + 3, -sy / 2 - 1.5, sy / 2 + 1.5, 0, sz, M)   # corner straps
    box(part, lid, -sx / 2 + 4, sx / 2 - 4, -sy / 2 - 0.8, -sy / 2, sz * 0.3, sz * 0.55, M)   # stencil band
    if collide:
        box("Collision", CL, -sx / 2, sx / 2, -sy / 2, sy / 2, z * 0, sz, M)


def locker_bank(part, x0, x1, y_wall, depth, side, h=205.0, w=58.0, collide=True):
    """Row of tall lockers along X, backs on the wall face y_wall, doors facing
    side*Y (side=+1: the room is at larger y)."""
    n = max(1, int((x1 - x0) // w))
    wx = (x1 - x0) / n
    yf = y_wall + side * depth
    box(part, BH, x0, x1, y_wall, yf, 0, h)
    for k in range(n):
        xa = x0 + k * wx
        box(part, HT, xa, xa + 2, yf, yf + side * 1.5, 0, h)                           # door seam
        for v in range(4):
            box(part, HT, xa + 12, xa + wx - 12, yf, yf + side * 1.2, h - 30 - v * 7, h - 27 - v * 7)
        box(part, ST, xa + wx - 10, xa + wx - 6, yf, yf + side * 4, h * 0.45, h * 0.58)
    box(part, HT, x0, x1, y_wall, yf + side * 2, h, h + 6)
    if collide:
        col(x0, x1, y_wall, yf, 0, h)


def rifle(part, x, y, yaw):
    """Standing pulse-rifle silhouette (racked muzzle-up), facing local +X."""
    M = Mx(x, y, 0, yaw)
    box(part, BH, -6, 6, -3, 3, 88, 118, M)          # stock
    box(part, HT, -7, 9, -4, 4, 118, 172, M)         # receiver
    box(part, BH, 2, 11, -4.5, 4.5, 146, 186, M)     # shroud
    box(part, HT, 9, 17, -2.5, 2.5, 128, 146, M)     # magazine
    c0 = M @ Vector((5, 0, 186)); c1 = M @ Vector((5, 0, 204))
    pipe(part, HT, c0, c1, 1.6, 6)
    box(part, HT, -4, 2, -2, 2, 108, 118, M)         # grip


def bunk_stack(part, x0, x1, y0, y1, collide=True):
    """Three-high rack bunk (navy rack spacing, 75 cm tiers): tube frame, trays,
    mattresses, end panels. 235 cm tall so it clears the tumblehome under a
    270 cm deckhead."""
    top = 230.0
    for (x, y) in ((x0 + 3, y0 + 3), (x1 - 3, y0 + 3), (x0 + 3, y1 - 3), (x1 - 3, y1 - 3)):
        pipe(part, HT, (x, y, 0), (x, y, top), 3.0, 8)
    for z in (22.0, 97.0, 172.0):
        box(part, ST, x0, x1, y0, y1, z, z + 7)
        box(part, BU, x0 + 6, x1 - 6, y0 + 5, y1 - 5, z + 7, z + 20)
        box(part, BU, x1 - 40, x1 - 8, y0 + 12, y1 - 12, z + 20, z + 27)          # pillow
        box(part, HT, x0, x1, y0 - 1, y0 + 1, z + 7, z + 27)                        # rail lip
        box("Lights", LA, x1 - 30, x1 - 12, y1 - 2, y1 - 1, z + 48, z + 53)         # reading lamp
    box(part, BH, x0 - 2, x0 + 2, y0, y1, 0, top)
    box(part, BH, x1 - 2, x1 + 2, y0, y1, 0, top)
    box(part, ST, x0, x1, y0, y1, top, top + 5)
    if collide:
        col(x0, x1, y0, y1, 0, top + 5)


def room_services(part, x0, x1, ow, sw, h=None, duct=True, skip_x=()):
    """Ship-room dressing that no real compartment is without: overhead pipe and
    cable runs along the outboard wall, a ventilation duct down the middle of the
    deckhead, chamfered corners, junction boxes and a frame-number plate.
    ow / sw: inner face y of the outboard / spine wall. skip_x: (x0, x1) ranges
    along the walls kept clear (doors, tall furniture)."""
    h = h or H_BLOCK
    s = 1.0 if ow > sw else -1.0
    # overhead runs just inboard of the tumblehome (it takes ow-45..ow above 215)
    pipe(part, PI, (x0, ow - s * 58, h - 22), (x1, ow - s * 58, h - 22), 7, 10)
    pipe(part, HT, (x0, ow - s * 76, h - 16), (x1, ow - s * 76, h - 16), 4, 8)
    box(part, HT, x0, x1, ow - s * 104, ow - s * 88, h - 14, h - 10)                # cable tray
    for k in range(int((x1 - x0) // 150) + 1):
        xb = x0 + 40 + k * 150
        if xb < x1 - 20:
            box(part, BH, xb - 3, xb + 3, ow - s * 110, ow - s * 50, h - 34, h)     # pipe hangers
    if duct:
        yc = (ow + sw) / 2
        box(part, ST, x0 + 30, x1 - 30, yc - 24, yc + 24, h - 38, h - 6)
        box(part, HT, x0 + 30, x1 - 30, yc - 26, yc + 26, h - 6, h)
        for k in range(int((x1 - x0 - 60) // 180)):
            xg = x0 + 120 + k * 180
            box(part, GR, xg - 25, xg + 25, yc - 18, yc + 18, h - 39, h - 38)        # diffuser grille
    # chamfered vertical corners (break the box)
    for (cx, cy, dx, dy) in ((x0, ow, 1, -s), (x1, ow, -1, -s), (x0, sw, 1, s), (x1, sw, -1, s)):
        K[part].prism_xy(BH, [(cx, cy), (cx + dx * 34, cy), (cx, cy + dy * 34)], 0, h)
    # conduit drops + junction boxes on the outboard wall
    for k in range(int((x1 - x0) // 260)):
        xj = x0 + 130 + k * 260
        if any(a - 40 < xj < b + 40 for (a, b) in skip_x):
            continue
        pipe(part, HT, (xj, ow - s * 5, 150), (xj, ow - s * 5, h - 20), 2.5, 6)
        box(part, ST, xj - 16, xj + 16, ow - s * 12, ow, 128, 160)
        box("Lights", LG if k % 2 else LA, xj - 3, xj + 3, ow - s * 12.6, ow - s * 12, 150, 154)
    # frame-number plate (stencil band) mid-wall
    xm = (x0 + x1) / 2
    box(part, HZ, xm - 30, xm + 30, ow - s * 2, ow, 175, 190)


def extinguisher(part, x, y, wall_s):
    """Wall-bracketed extinguisher bottle (hazard-striped) + its placard."""
    cyl(part, HZ, x, y - wall_s * 12, 40, 95, 9, 10)
    cyl(part, HT, x, y - wall_s * 12, 95, 104, 4, 8)
    box(part, HT, x - 10, x + 10, y - wall_s * 3, y, 60, 70)
    box(part, HZ, x - 12, x + 12, y - wall_s * 1.5, y, 115, 140)


# ----------------------------------------------------------------------------
# Zones (ship scale: 2 m passages, 2.5-2.7 m deckheads, compartments of 50 m2)
# ----------------------------------------------------------------------------
X_ENG0, X_ENG1 = -4300.0, -3100.0
X_HGR0, X_HGR1 = -3100.0, -1100.0
X_BLK0, X_BLK1 = -1100.0, 1600.0
X_CICB, X_CIC1 = 2400.0, 2800.0     # bow chamfer starts at X_CICB
X_DIV = (-200.0, 700.0)             # room dividers in the spine block
Y_BLK = 710.0
Y_HGR = 1150.0
Y_ENG = 800.0
Y_CIC = 500.0
Y_BOW = 250.0
Y_SP = 110.0          # spine wall lines at +/- this (1.9 m clear)
DOOR_W, DOOR_H = 140.0, 210.0
SPINE_DOORS = {        # room door centres (x) on each spine wall
    +1: [-650.0, 250.0, 1150.0],     # berths, mess, briefing
    -1: [-650.0, 250.0, 1150.0],     # ready room, medbay, armory
}
PCX, PCY = -2150.0, -120.0           # dropship pad centre (nose to the launch door, -Y)
CAT_Z = 450.0                        # hangar catwalk + flight-control booth floor
BOOTH = (X_HGR1, -600.0, 150.0, 700.0)   # x0, x1, y0, y1 of the booth over the berths
RX, RY = -3700.0, 0.0                # reactor
ENG_RZ = 400.0                       # engineering catwalk ring + gallery
GAL_X = -4100.0                      # gallery front edge

# Walk-check spaces (Blender design cm): (name, x0, x1, y0, y1)
SPACES = [
    ("Spine", X_BLK0, X_BLK1, -Y_SP, Y_SP),
    ("Berths", X_BLK0, X_DIV[0], Y_SP, Y_BLK), ("ReadyRoom", X_BLK0, X_DIV[0], -Y_BLK, -Y_SP),
    ("Mess", X_DIV[0], X_DIV[1], Y_SP, Y_BLK), ("Medbay", X_DIV[0], X_DIV[1], -Y_BLK, -Y_SP),
    ("Briefing", X_DIV[1], X_BLK1, Y_SP, Y_BLK), ("Armory", X_DIV[1], X_BLK1, -Y_BLK, -Y_SP),
    ("CIC", X_BLK1, X_CIC1, -Y_CIC, Y_CIC),
    ("Hangar", X_HGR0, X_HGR1, -Y_HGR, Y_HGR),
    ("Engineering", X_ENG0, X_ENG1, -Y_ENG, Y_ENG),
]


def build_structure():
    sh = "Shell"
    # --- floors (visual) + one walk floor for everything ---------------------
    box(sh, DK, X_ENG0, X_ENG1, -Y_ENG, Y_ENG, -20, 0)
    box(sh, DK, X_HGR0, X_HGR1, -Y_HGR, Y_HGR, -20, 0)
    box(sh, DK, X_BLK0, X_BLK1, -Y_BLK, Y_BLK, -20, 0)
    cic_poly = [(X_BLK1, -Y_CIC), (X_CICB, -Y_CIC), (X_CIC1, -Y_BOW), (X_CIC1, Y_BOW),
                (X_CICB, Y_CIC), (X_BLK1, Y_CIC)]
    K[sh].prism_xy(DK, cic_poly, -20, 0)
    col(X_ENG0, X_CIC1, -Y_HGR, Y_HGR, -20, 0)

    # --- ceilings ---------------------------------------------------------
    box(sh, SH, X_ENG0, X_ENG1, -Y_ENG, Y_ENG, H_ENG, H_ENG + 20)
    box(sh, SH, X_HGR0, X_HGR1, -Y_HGR, Y_HGR, H_BAY, H_BAY + 20)
    box(sh, SH, X_BLK0, X_BLK1, -Y_BLK, Y_BLK, H_BLOCK, H_BLOCK + 20)
    K[sh].prism_xy(SH, cic_poly, H_CIC, H_CIC + 20)

    # --- engineering walls ------------------------------------------------
    wall(sh, EN, (X_ENG0, -Y_ENG), (X_ENG0, Y_ENG), 0, H_ENG, frames=False)
    wall(sh, EN, (X_ENG0, Y_ENG), (X_ENG1, Y_ENG), 0, H_ENG, frames=False)
    wall(sh, EN, (X_ENG1, -Y_ENG), (X_ENG0, -Y_ENG), 0, H_ENG, frames=False)
    # --- hangar walls -----------------------------------------------------
    # aft (shared with engineering): blast door into engineering
    wall(sh, SH, (X_HGR0, -Y_HGR), (X_HGR0, Y_HGR), 0, H_BAY, openings=[(Y_HGR, 300, 0, 350)])
    wall(sh, SH, (X_HGR0, Y_HGR), (X_HGR1, Y_HGR), 0, H_BAY, frames=False)
    # starboard: launch door opening, filled by the door leaves in build_hangar
    wall(sh, SH, (X_HGR1, -Y_HGR), (X_HGR0, -Y_HGR), 0, H_BAY,
         openings=[(X_HGR1 - PCX, 1400, 0, 800)], frames=False)
    # forward (hangar -> spine block): ready-room door, spine door, booth door
    # (catwalk level) + booth window
    bx0, bx1, by0, by1 = BOOTH
    wall(sh, SH, (X_HGR1, -Y_HGR), (X_HGR1, Y_HGR), 0, H_BAY,
         openings=[(Y_HGR - 450.0, DOOR_W, 0, DOOR_H),        # ready room (y = -450)
                   (Y_HGR + 0.0, 180, 0, 220),                 # spine
                   (Y_HGR + 250.0, 120, CAT_Z, CAT_Z + 205),   # booth door (y = 250)
                   (Y_HGR + 525.0, 250, CAT_Z + 70, CAT_Z + 190)],   # booth window
         frames=False)
    for (y, w, zb, zt) in ((-450.0, DOOR_W, 0, DOOR_H), (0.0, 180, 0, 220), (250.0, 120, CAT_Z, CAT_Z + 205)):
        door_frame(sh, X_HGR1, y, zb, 90.0, w, zt - zb)
    seg_box(sh, VP, (X_HGR1, 400), (X_HGR1, 650), 4, CAT_Z + 70, CAT_Z + 190)
    for y in (400, 525, 650):
        seg_box(sh, BH, (X_HGR1, y - 8), (X_HGR1, y + 8), WT + 20, CAT_Z + 70, CAT_Z + 190)
    seg_box(sh, BH, (X_HGR1, 390), (X_HGR1, 660), WT + 24, CAT_Z + 56, CAT_Z + 70)
    seg_box(sh, BH, (X_HGR1, 390), (X_HGR1, 660), WT + 24, CAT_Z + 190, CAT_Z + 206)

    # --- spine block ------------------------------------------------------
    wall(sh, SH, (X_BLK0, Y_BLK), (X_BLK1, Y_BLK), 0, H_BLOCK, frames=False)
    wall(sh, SH, (X_BLK1, -Y_BLK), (X_BLK0, -Y_BLK), 0, H_BLOCK, frames=False)
    for side in (1, -1):
        y = side * Y_SP
        ops = [(x - X_BLK0, DOOR_W, 0, DOOR_H) for x in SPINE_DOORS[side]]
        wall(sh, SH, (X_BLK0, y), (X_BLK1, y), 0, H_BLOCK, openings=ops)
        for xd in X_DIV:
            wall(sh, SH, (xd, y), (xd, side * Y_BLK), 0, H_BLOCK, frames=False)
        # tumblehome: the outboard wall leans in under the deckhead, following the hull
        yo = side * (Y_BLK - WT / 2)
        extrude(sh, SH, [(yo, 215), (yo, H_BLOCK), (yo - side * 45, H_BLOCK)], 'x', X_BLK0 + 15, X_BLK1 - 15)
        for xf in range(int(X_BLK0) + 300, int(X_BLK1), 300):                      # hull frames
            extrude(sh, BH, [(yo, 0), (yo - side * 14, 0), (yo - side * 14, 210),
                             (yo - side * 59, H_BLOCK), (yo - side * 45, H_BLOCK), (yo, 215)],
                    'x', xf - 9, xf + 9)
    # forward wall: spine block -> CIC (CIC side is taller)
    wall(sh, SH, (X_BLK1, -Y_BLK), (X_BLK1, Y_BLK), 0, H_CIC, openings=[(Y_BLK, 180, 0, 220)])

    # --- CIC walls: sides solid, bow + chamfers with armoured slit windows ----
    wall(sh, SH, (X_BLK1, Y_CIC), (X_CICB, Y_CIC), 0, H_CIC, frames=False)
    wall(sh, SH, (X_CICB, -Y_CIC), (X_BLK1, -Y_CIC), 0, H_CIC, frames=False)
    for p0, p1 in (((X_CICB, -Y_CIC), (X_CIC1, -Y_BOW)), ((X_CIC1, -Y_BOW), (X_CIC1, Y_BOW)),
                   ((X_CIC1, Y_BOW), (X_CICB, Y_CIC))):
        window_band(sh, p0, p1, 0, H_CIC, 140, 200, mull=110)

    # --- flight-control booth (above the berths, off the hangar catwalk) ----
    bz, bh = CAT_Z, CAT_Z + 250
    solid(sh, DK, bx0, bx1, by0, by1, bz - 20, bz)
    box(sh, SH, bx0, bx1, by0, by1, bh, bh + 20)
    wall(sh, SH, (bx1, by0), (bx1, by1), bz, bh, frames=False)
    wall(sh, SH, (bx0, by0), (bx1, by0), bz, bh, frames=False)
    wall(sh, SH, (bx1, by1), (bx0, by1), bz, bh, frames=False)


def build_spine():
    p = "Spine"
    x0, x1 = X_BLK0 + 15, X_BLK1 - 15
    wy = Y_SP - WT / 2           # inner wall face (95)
    doors = SPINE_DOORS
    # ceiling, chamfer wedges, floor grating + gutters
    box(p, SH, x0, x1, -wy, wy, H_SPINE, H_SPINE + 10)
    for s in (1, -1):
        prof = [(s * wy, 200), (s * wy, H_SPINE), (s * 55, H_SPINE)]
        extrude(p, BH, prof, 'x', x0, x1)
        box(p, GR, x0, x1, s * 58, s * 84, 0, 0.6)                                  # side gutter grille
        box(p, HT, x0, x1, s * 84, s * wy, 0, 12)                                   # kick trim
    box(p, GR, x0, x1, -34, 34, 0, 0.6)                                             # centre grating
    for s in (1, -1):
        box(p, HZ, x0, x1, s * 34, s * 40, 0, 0.7)                                  # hazard edge lines
    # ribs every 300, skipping doors; recessed ceiling fixture between each pair
    xs = []
    x = x0 + 150
    while x < x1 - 100:
        xs.append(x)
        x += 300
    for i, xr in enumerate(xs):
        near_door = [s for s in (1, -1) if any(abs(xr - d) < 125 for d in doors[s])]
        for s in (1, -1):
            if s in near_door:
                continue
            box(p, BH, xr - 12, xr + 12, s * (wy - 14), s * wy, 0, 200)
            box(p, HZ, xr - 13, xr + 13, s * (wy - 15), s * wy, 0, 18)                # striped foot
            A = Vector((s * wy, 200)); B = Vector((s * 55, H_SPINE))
            nrm = Vector((-s, -0.8)).normalized() * 14
            extrude(p, BH, [tuple(A), tuple(B), tuple(B + nrm), tuple(A + nrm)], 'x', xr - 12, xr + 12)
        box(p, BH, xr - 12, xr + 12, -55, 55, H_SPINE - 16, H_SPINE)
        if i < len(xs) - 1:
            xm = (xr + xs[i + 1]) / 2
            ceiling_panel(p, xm, 0, H_SPINE, 0.0, 120)
            if i % 2 == 0:
                light("W", 5, xm, 0, H_SPINE - 25)
    # pipe runs tucked into both upper chamfers, cable tray / conduit at knee height
    for s in (1, -1):
        for (yy, zz, r, slot) in ((80, 204, 7, PI), (70, 221, 5, HT), (59, 237, 4, PI)):
            pipe(p, slot, (x0, s * yy, zz), (x1, s * yy, zz), r, 10)
        if s > 0:
            box(p, HT, x0, x1, wy - 12, wy, 58, 62)
            box(p, HT, x0, x1, wy - 5, wy, 58, 74)
        else:
            pipe(p, HT, (x0, -wy + 6, 68), (x1, -wy + 6, 68), 4, 8)
    # between ribs: panels, vents, intercom terminals, caged red lamps
    for i in range(len(xs) - 1):
        xa, xb = xs[i] + 12, xs[i + 1] - 12
        xm = (xa + xb) / 2
        for s in (1, -1):
            if any(xa - 90 < d < xb + 90 for d in doors[s]):
                continue
            box(p, BH, xa + 10, xb - 10, s * (wy - 4), s * wy, 85, 190)
            if (i + (s > 0)) % 3 == 0:
                for v in range(5):
                    box(p, HT, xm - 40, xm + 40, s * (wy - 6), s * (wy - 4), 100 + v * 14, 106 + v * 14)
            elif (i + (s > 0)) % 3 == 1:
                box(p, ST, xm - 20, xm + 20, s * (wy - 12), s * (wy - 4), 115, 158)      # intercom
                box("Lights", LG, xm - 14, xm + 14, s * (wy - 13), s * (wy - 12), 132, 153)
                box(p, HT, xm - 18, xm + 18, s * (wy - 14), s * (wy - 12), 119, 127)
            else:
                cage_lamp(p, xm, s * (wy - 4), 180, -90 if s > 0 else 90)
    # stencilled deck-section plates either side of each door
    for s in (1, -1):
        for d in doors[s]:
            box(p, HZ, d - 118, d - 102, s * (wy - 5), s * wy, 30, 180)
            box(p, HZ, d + 102, d + 118, s * (wy - 5), s * wy, 30, 180)
    extinguisher(p, X_BLK0 + 120, wy, 1)
    extinguisher(p, X_BLK1 - 120, -wy, -1)
    # end bulkhead frames
    for xe in (x0, x1):
        for s in (1, -1):
            box(p, BH, xe - 16, xe + 16, s * 62, s * wy, 0, H_SPINE)


def build_cic():
    p = "CIC"
    cx, cy = 2000.0, 0.0
    # raised dais (two 18 cm steps), hazard edge, plot table
    for (z0, z1, r, slot) in ((0, 18, 260, DK), (18, 36, 220, GR)):
        pts = circle(cx, cy, r, 8, 22.5, 382.5)[:8]
        K[p].prism_xy(slot, pts, z0, z1)
        K["Collision"].prism_xy(CL, pts, z0, z1)
    K[p].annulus(HZ, cx, cy, 245, 260, 18, 18.6, 8, 22.5, 382.5)
    cyl(p, ST, cx, cy, 36, 84, 95, 6)
    K[p].prism_xy(ST, circle(cx, cy, 135, 6), 84, 104)
    K[p].prism_xy(HT, circle(cx, cy, 140, 6), 98, 103)
    K["Lights"].prism_xy(CO, circle(cx, cy, 120, 6), 104, 105.2)     # plot screen
    for k in range(-2, 3):
        box("Lights", LG, cx + k * 40 - 0.8, cx + k * 40 + 0.8, -95, 95, 140, 141.5)
        box("Lights", LG, cx - 95, cx + 95, k * 40 - 0.8, k * 40 + 0.8, 140, 141.5)
    for a in range(6):
        t = math.radians(a * 60 + 30)
        pipe(p, HT, (cx + 125 * math.cos(t), cy + 125 * math.sin(t), 104),
             (cx + 95 * math.cos(t), cy + 95 * math.sin(t), 142), 1.5, 6)
    cyl("Collision", CL, cx, cy, 36, 150, 150, 6)
    light("G", 4, cx, cy, 230)
    # overhead hood: octagonal light ring above the table
    K[p].annulus(ST, cx, cy, 190, 240, H_CIC - 55, H_CIC, 8, 22.5, 382.5)
    K["Lights"].annulus(LW, cx, cy, 196, 226, H_CIC - 57, H_CIC - 55, 8, 22.5, 382.5)
    # captain's chair on the dais, looking forward
    chair(p, cx - 190, cy, 36, 180.0, big=True)
    box("Collision", CL, cx - 230, cx - 150, -40, 40, 36, 150)
    # side-wall stations: operators shoulder to shoulder, backs to the dais
    for xs in (1790.0, 2110.0):
        console(p, xs, Y_CIC - 15 - 72, -90.0, n_crt=3)
        console(p, xs, -Y_CIC + 15 + 72, 90.0, n_crt=3, screen=LA if xs > 2000 else LG)
    # helm row under the bow windows: helm centre, nav + fire control
    console(p, 2620, 0, 180.0, n_crt=3)
    console(p, 2575, 225, 180.0, n_crt=2)
    console(p, 2575, -225, 180.0, n_crt=2, screen=LA)
    # main displays above the slit windows (bow + chamfers)
    for (p0, p1) in (((X_CICB, -Y_CIC), (X_CIC1, -Y_BOW)), ((X_CIC1, -Y_BOW), (X_CIC1, Y_BOW)),
                     ((X_CIC1, Y_BOW), (X_CICB, Y_CIC))):
        a = Vector(p0); b = Vector(p1)
        e = (b - a).normalized(); n = Vector((-e.y, e.x))       # inward (CCW outline)
        L = (b - a).length
        q0 = a + e * (L * 0.12) + n * 22; q1 = a + e * (L * 0.88) + n * 22
        seg_box(p, HT, tuple(q0), tuple(q1), 14, 215, 305)
        q0b = q0 + n * 8; q1b = q1 + n * 8
        seg_box(p, CO, tuple(q0b + e * 8), tuple(q1b - e * 8), 3, 224, 296)
    # aft wall CRT banks either side of the door
    for s in (1, -1):
        for col_i in range(3):
            for row in range(2):
                y = s * (165 + col_i * 115)
                crt(p, X_BLK1 + 15 + 45, y, 125 + row * 90, 100, 80, 0.0,
                    screen=LG if (col_i + row) % 3 else LA, d=45)
        box(p, ST, X_BLK1 + 15, X_BLK1 + 60, s * 110, s * (Y_CIC - 15), 0, 125)
        col(X_BLK1 + 15, X_BLK1 + 60, s * 110, s * (Y_CIC - 15), 0, 125)
    # structural pillars with cable bundles + hazard feet, between the side stations
    for (px, py) in ((1950, 425), (1950, -425), (2280, 425), (2280, -425)):   # between the side stations
        solid(p, BH, px - 28, px + 28, py - 28, py + 28, 0, H_CIC)
        box(p, HZ, px - 30, px + 30, py - 30, py + 30, 0, 45)
        for k in range(2):
            pipe(p, PI, (px - 12 + k * 24, py - s_sign(py) * 32, 0), (px - 12 + k * 24, py - s_sign(py) * 32, H_CIC), 4, 8)
    # deckhead beams, ducts + fixtures, red battle lamps
    for x in (1800.0, 2110.0, 2400.0):
        box(p, BH, x - 16, x + 16, -Y_CIC + 15, Y_CIC - 15, H_CIC - 32, H_CIC)
    for s in (1, -1):
        pipe(p, PI, (X_BLK1 + 20, s * 330, H_CIC - 45), (X_CICB, s * 330, H_CIC - 45), 8, 10)
        pipe(p, HT, (X_BLK1 + 20, s * 355, H_CIC - 40), (X_CICB, s * 355, H_CIC - 40), 5, 8)
    for (x, y) in ((1790, 290), (1790, -290), (2250, 290), (2250, -290), (2560, 0)):
        ceiling_panel(p, x, y, H_CIC - 32, 90.0 if x < 2500 else 0.0, 140)
        light("W", 6, x, y, H_CIC - 60)
    for (x, y) in ((1640, 90), (1640, -90)):
        cage_lamp(p, x, y, H_CIC - 50, 0.0)
    for s in (1, -1):
        cage_lamp(p, 2300, s * (Y_CIC - 15), H_CIC - 50, -90.0 * s)
    light("R", 4, 1680, 0, 280)
    # entry + seat
    SOCKETS["Entry"] = [2270.0, 0.0, 100.0, 180.0]
    SOCKETS["Seat"] = [2580.0, 0.0, 100.0, 0.0]


def s_sign(v):
    return 1.0 if v >= 0 else -1.0


def build_quarters():
    p = "Quarters"
    # --- berths (port, x -1100..-200, y 110..710) ---
    x0, x1, y0, y1 = X_BLK0 + 15, X_DIV[0] - 15, Y_SP + 15, Y_BLK - 15
    # outboard row + a back-to-back island, 80 cm aisles: 27 racks in 50 m2
    for k in range(3):
        xa = x0 + 60 + k * 240
        bunk_stack(p, xa, xa + 210, y1 - 110, y1 - 18)      # clear of the hull frames
        bunk_stack(p, xa, xa + 210, 339, 431)
        bunk_stack(p, xa, xa + 210, 431, 523)
    door = SPINE_DOORS[1][0]
    locker_bank(p, x0 + 40, door - 110, y0, 50, 1)
    locker_bank(p, door + 110, x1 - 40, y0, 50, 1)
    crate(p, x0 + 30, 431, 0, 40, 80, 45, lid=HT)                                   # footlocker
    room_services(p, x0, x1, y1, y0, duct=False, skip_x=[(x0, x1)])
    for x in (-900.0, -410.0):
        ceiling_panel(p, x, 260, H_BLOCK - 2, 0.0, 150)
        light("W", 5, x, 260, H_BLOCK - 30)
    light("A", 3, -650, 560, 150)
    cage_lamp(p, x1, 280, 200, 180.0)
    # --- drop-crew ready room (starboard, x -1100..-200, y -710..-110) ---
    y0, y1 = -Y_BLK + 15, -Y_SP - 15
    for k in range(4):
        xa = x0 + 35 + k * 200
        xm = xa + 80
        box(p, BH, xa, xa + 160, y0, y0 + 10, 0, 215)                               # back panel
        box(p, HT, xa, xa + 6, y0, y0 + 75, 0, 215)
        box(p, HT, xa + 154, xa + 160, y0, y0 + 75, 0, 215)
        box(p, HT, xa, xa + 160, y0, y0 + 75, 215, 222)
        box(p, ST, xa + 6, xa + 154, y0, y0 + 75, 0, 12)
        # armour suit on its hanger
        box(p, BH, xm - 26, xm + 26, y0 + 25, y0 + 58, 110, 172)                    # torso
        box(p, HT, xm - 20, xm + 20, y0 + 24, y0 + 60, 100, 112)                    # belt
        for s in (-1, 1):
            box(p, BH, xm + s * 26, xm + s * 42, y0 + 26, y0 + 56, 150, 176)       # pauldron
            box(p, HT, xm + s * 28, xm + s * 38, y0 + 32, y0 + 50, 95, 150)        # arm
            box(p, BH, xm + s * 6, xm + s * 22, y0 + 30, y0 + 52, 15, 100)         # leg
            box(p, HT, xm + s * 5, xm + s * 23, y0 + 28, y0 + 60, 0, 15)           # boot
        K[p].ico(BH, (xm, y0 + 42, 190), 16, 1)                                    # helmet
        box(p, VP, xm - 11, xm + 11, y0 + 57, y0 + 59, 184, 194)                   # visor
        box("Lights", LA, xm - 30, xm + 30, y0 + 70, y0 + 72, 208, 212)
    col(x0 + 35, x0 + 35 + 200 * 4, y0, y0 + 75, 0, 222)
    # bench down the middle (along x), kit bags + helmets on it
    box(p, HT, -1000, -420, -430, -390, 38, 45)
    box(p, BH, -998, -422, -428, -392, 45, 50)
    for xl in (-980, -710, -440):
        box(p, HT, xl - 5, xl + 5, -422, -398, 0, 38)
    for (xb, h) in ((-930, 30), (-760, 26), (-560, 34)):
        box(p, BU, xb - 30, xb + 30, -426, -394, 50, 50 + h)
    col(-1004, -416, -434, -386, 0, 60)
    door = SPINE_DOORS[-1][0]
    locker_bank(p, x0 + 40, door - 110, y1, 50, -1)
    locker_bank(p, door + 110, x1 - 40, y1, 50, -1)
    # status board on the divider wall
    box(p, HT, x1 - 12, x1, -560, -260, 100, 230)
    box(p, CO, x1 - 14, x1 - 12, -548, -272, 112, 218)
    room_services(p, x0, x1, y0, y1, skip_x=[(x0, x1)])
    for x in (-900.0, -410.0):
        ceiling_panel(p, x, -560, H_BLOCK - 2, 0.0, 150)
        light("W", 5, x, -560, H_BLOCK - 30)
    cage_lamp(p, x0, -300, 200, 0.0)


def build_mess():
    p = "Mess"
    x0, x1, y0, y1 = X_DIV[0] + 15, X_DIV[1] - 15, Y_SP + 15, Y_BLK - 15
    # three mess tables athwartships, bolted benches, 60 cm between bench backs
    for xt in (-80.0, 150.0, 380.0):
        box(p, ME, xt - 40, xt + 40, 265, 565, 72, 77)
        box(p, HT, xt - 34, xt + 34, 271, 559, 66, 72)
        for yl in (285.0, 545.0):
            box(p, HT, xt - 5, xt + 5, yl - 5, yl + 5, 0, 66)
            box(p, HT, xt - 32, xt + 32, yl - 4, yl + 4, 0, 5)
        for s in (-1, 1):
            box(p, ME, xt + s * 55, xt + s * 82, 265, 565, 42, 46)
            box(p, HT, xt + s * 64, xt + s * 72, 275, 555, 0, 42)
        # trays, mugs
        for k in range(4):
            yy = 290 + k * 72
            box(p, GA, xt - 32, xt - 6, yy, yy + 22, 77, 79)
            cyl(p, ST, xt + 18, yy + 11, 77, 88, 4, 8)
        col(xt - 84, xt + 84, 263, 567, 0, 90)
    # galley along the forward divider: counter, hood, ovens, fridge, urns
    gx = x1
    box(p, GA, gx - 70, gx, y0 + 95, y1, 0, 90)
    box(p, ST, gx - 75, gx, y0 + 95, y1, 90, 94)
    extrude(p, BH, [(gx - 80, 195), (gx, 195), (gx, 240), (gx - 55, 240)], 'y', y0 + 140, y1 - 120)
    box("Lights", LA, gx - 78, gx - 55, y0 + 150, y1 - 130, 192, 194)
    for k in range(3):
        yo = y0 + 130 + k * 130
        box(p, HT, gx - 72, gx - 70, yo, yo + 110, 12, 80)
        box(p, VP, gx - 73, gx - 72, yo + 15, yo + 95, 35, 70)
    box(p, GA, gx - 80, gx, y1 - 110, y1, 0, 205)                                    # fridge
    box(p, HT, gx - 81, gx - 80, y1 - 56, y1 - 54, 10, 200)
    for y in (y0 + 140, y0 + 200):
        cyl(p, ST, gx - 35, y, 94, 150, 16, 12)
        cyl(p, HT, gx - 35, y, 150, 156, 17, 12)
    col(gx - 82, gx, y0 + 95, y1, 0, 205)
    # menu board + extinguisher by the door
    box(p, HT, x0, x0 + 8, 300, 520, 110, 200)
    box(p, CO, x0 + 8, x0 + 9, 312, 508, 120, 190)
    extinguisher(p, SPINE_DOORS[1][1] + 110, y0, -1)
    room_services(p, x0, x1 - 90, y1, y0)
    for x in (-60.0, 380.0):
        ceiling_panel(p, x, 400, H_BLOCK - 40, 90.0, 180)
        light("W", 5, x, 400, H_BLOCK - 60)
    light("A", 3, gx - 120, 450, 190)


def build_medbay():
    p = "Medbay"
    x0, x1, y0, y1 = X_DIV[0] + 15, X_DIV[1] - 15, -Y_BLK + 15, -Y_SP - 15
    # cryo bay: four hypersleep pods, heads to the outboard wall
    for k in range(4):
        cx = 300.0 + k * 110
        M = Mx(cx, y0 + 10, 0, 90.0)          # local +X = inboard (+y)
        r0, r1 = 0.0, 210.0
        box(p, ST, r0, r1, -45, 45, 0, 55, M)
        box(p, HT, r0 - 4, r1 + 4, -48, 48, 0, 10, M)
        box(p, BU, r0 + 10, r1 - 10, -36, 36, 55, 62, M)
        lid = [(38 * math.cos(math.radians(t)), 62 + 34 * math.sin(math.radians(t))) for t in range(0, 181, 20)]
        extrude(p, VP, lid, 'x', r0 + 18, r1 - 14, M)
        for rr in (r0 + 14, r1 - 14):
            ring = [(44 * math.cos(math.radians(t)), 58 + 40 * math.sin(math.radians(t))) for t in range(0, 181, 20)]
            ring += [(38 * math.cos(math.radians(t)), 62 + 34 * math.sin(math.radians(t))) for t in range(180, -1, -20)]
            extrude(p, BH, ring, 'x', rr - 4, rr + 4, M)
        box(p, ST, r0, r0 + 20, -40, 40, 55, 115, M)                                 # head panel
        box("Lights", LG, r0 + 20, r0 + 21, -28, 28, 80, 108, M)
        box("Lights", LB, r1 + 4, r1 + 5, -30, 30, 20, 30, M)
        box("Collision", CL, r0 - 4, r1 + 6, -50, 50, 0, 110, M)
    # coolant manifold feeding the pods along the outboard wall
    pipe(p, PI, (250, y0 + 8, 30), (680, y0 + 8, 30), 5, 8)
    pipe(p, HT, (250, y0 + 20, 45), (680, y0 + 20, 45), 3, 6)
    light("B", 4, 465, y0 + 150, 200)
    # two med beds on the aft half, surgical lamps, curtain rails, monitors
    for mx in (-110.0, 70.0):
        box(p, HT, mx - 38, mx + 38, y0 + 30, y0 + 230, 0, 68)
        box(p, GA, mx - 42, mx + 42, y0 + 26, y0 + 234, 68, 80)
        box(p, BU, mx - 36, mx + 36, y0 + 190, y0 + 230, 80, 90)
        cyl(p, HT, mx, y0 + 130, H_BLOCK - 50, H_BLOCK, 4, 8)
        cyl(p, ST, mx, y0 + 130, H_BLOCK - 66, H_BLOCK - 50, 42, 16)
        cyl("Lights", LW, mx, y0 + 130, H_BLOCK - 68, H_BLOCK - 66, 34, 16)
        crt(p, mx + 62, y0 + 40, 115, 44, 36, 90.0, screen=LG, d=36)
        box(p, HT, mx + 44, mx + 84, y0 + 8, y0 + 38, 0, 115)
        col(mx - 46, mx + 46, y0, y0 + 240, 0, 100)
        light("W", 3, mx, y0 + 130, H_BLOCK - 90)
    for xr in (-20.0, 160.0):                                                        # curtain rails
        pipe(p, HT, (xr, y0 + 10, H_BLOCK - 30), (xr, y0 + 280, H_BLOCK - 30), 1.5, 6)
    # instrument cabinets on the aft divider wall
    for ya in (-430.0, -280.0):
        box(p, GA, x0, x0 + 50, ya, ya + 140, 0, 90)
        box(p, GA, x0, x0 + 32, ya, ya + 140, 140, 225)
        box(p, VP, x0 + 32, x0 + 33, ya + 10, ya + 130, 150, 215)
    col(x0, x0 + 50, -430, -140, 0, 100)
    # supply cart on the aisle
    box(p, ST, 480, 560, -330, -270, 20, 90)
    for z in (20.0, 55.0, 90.0):
        box(p, HT, 478, 562, -332, -268, z, z + 3)
    col(476, 564, -334, -266, 0, 95)
    extinguisher(p, SPINE_DOORS[-1][1] - 110, y1, 1)
    room_services(p, x0, x1, y0, y1, skip_x=[(x0, x1)])
    for (x, y) in ((300.0, -300.0), (600.0, -520.0)):
        ceiling_panel(p, x, y, H_BLOCK - 40, 0.0, 150)
        light("W", 5, x, y, H_BLOCK - 60)


def build_briefing():
    p = "Briefing"
    x0, x1, y0, y1 = X_DIV[1] + 15, X_BLK1 - 15, Y_SP + 15, Y_BLK - 15
    # CRT video wall on the forward bulkhead, 4 x 2
    yc = 440.0
    box(p, HT, x1 - 25, x1, yc - 250, yc + 250, 40, 250)
    for i in range(4):
        for j in range(2):
            y = yc - 180 + i * 120
            crt(p, x1 - 25, y, 60 + j * 95, 112, 88, 180.0, screen=CO if (i + j) % 2 else LG, d=10)
    col(x1 - 27, x1, yc - 252, yc + 252, 0, 250)
    # lectern + tactical board
    M = Mx(1460, 210, 0, 180.0)
    extrude(p, BH, [(-30, 0), (30, 0), (24, 110), (-36, 118)], 'y', -32, 32, M)
    box("Lights", LA, 1474, 1480, 185, 235, 112, 116)
    col(1425, 1495, 175, 245, 0, 120)
    box(p, HT, 760, 1260, y1 - 18, y1, 100, 205)
    box(p, CO, 772, 1248, y1 - 20, y1 - 18, 110, 195)
    # four seat rows facing the video wall, aisle down the middle
    for xr in (870.0, 1000.0, 1130.0, 1260.0):
        for block in ((285.0, 375.0), (545.0, 635.0)):
            for y in block:
                chair(p, xr, y, 0, 180.0)
            box(p, HT, xr + 20, xr + 30, block[0] - 30, block[1] + 30, 0, 6)
            col(xr - 32, xr + 32, block[0] - 30, block[1] + 30, 0, 100)
    box(p, ST, 1060, 1140, 400, 480, H_BLOCK - 40, H_BLOCK)                         # ceiling projector
    box("Lights", LW, 1138, 1140, 425, 455, H_BLOCK - 32, H_BLOCK - 12)
    room_services(p, x0, x1 - 40, y1, y0, duct=False, skip_x=[(760, 1260)])
    for x in (850.0, 1300.0):
        ceiling_panel(p, x, 260, H_BLOCK - 2, 90.0, 150)
        ceiling_panel(p, x, 600, H_BLOCK - 30, 90.0, 150)
        light("W", 5, x, 430, H_BLOCK - 35)
    light("G", 3, x1 - 150, yc, 200)


def build_armory():
    p = "Armory"
    x0, x1, y0, y1 = X_DIV[1] + 15, X_BLK1 - 15, -Y_BLK + 15, -Y_SP - 15
    xc = 1000.0
    # cage partition: bars with a gate and a counter window (quartermaster aft of it)
    gate = (-310.0, -170.0)
    win = (-560.0, -380.0)
    y = y0
    while y < y1:
        in_gate = gate[0] < y < gate[1]
        in_win = win[0] < y < win[1]
        if not in_gate:
            if in_win:
                pipe(p, HT, (xc, y, 190), (xc, y, 240), 1.6, 6)
            else:
                pipe(p, HT, (xc, y, 0), (xc, y, 240), 1.6, 6)
        y += 14
    for z in (8.0, 100.0, 238.0):
        pipe(p, BH, (xc, y0, z), (xc, gate[0], z), 3, 8)
        pipe(p, BH, (xc, gate[1], z), (xc, y1, z), 3, 8)
    box(p, ST, xc - 40, xc + 40, win[0], win[1], 95, 100)                               # counter
    box(p, HT, xc - 35, xc + 35, win[0] + 5, win[1] - 5, 0, 95)
    box(p, BH, xc - 20, xc + 20, win[0], win[1], 186, 190)
    for g in gate:
        box(p, HZ, xc - 6, xc + 6, g - 6, g + 6, 0, 240)
    col(xc - 40, xc + 40, y0, gate[0], 0, 240)
    col(xc - 8, xc + 8, gate[1], y1, 0, 240)
    # quartermaster side: shelving with crates
    for yb in (-620.0, -460.0):
        box(p, HT, x0, x0 + 60, yb - 65, yb + 65, 0, 200)
        for z in (40.0, 100.0, 160.0):
            box(p, ST, x0, x0 + 60, yb - 65, yb + 65, z, z + 4)
            box(p, BH, x0 + 8, x0 + 52, yb - 48, yb + 48, z + 4, z + 36)
    col(x0, x0 + 60, -685, -395, 0, 200)
    # rifle racks: outboard wall and forward wall
    box(p, BH, 1080, 1560, y0, y0 + 16, 70, 212)
    box(p, HT, 1080, 1560, y0, y0 + 30, 70, 84)
    x = 1100.0
    while x < 1540:
        rifle(p, x, y0 + 24, 90.0)
        x += 32
    col(1080, 1560, y0, y0 + 40, 0, 212)
    box(p, BH, x1 - 16, x1, -560, -260, 70, 212)
    box(p, HT, x1 - 30, x1, -560, -260, 70, 84)
    y = -540.0
    while y < -270:
        rifle(p, x1 - 24, y, 180.0)
        y += 32
    col(x1 - 40, x1, -560, -260, 0, 212)
    # smart-gun harness on a stand
    sx, sy = 1300.0, -420.0
    cyl(p, HT, sx, sy, 0, 8, 40, 12)
    cyl(p, HT, sx, sy, 8, 120, 6, 8)
    box(p, BH, sx - 25, sx + 25, sy - 15, sy + 15, 120, 180)
    pipe(p, HT, (sx + 25, sy - 30, 160), (sx + 95, sy - 30, 162), 6, 10)
    box(p, HT, sx + 20, sx + 60, sy - 42, sy - 18, 145, 172)
    pipe(p, HT, (sx - 15, sy, 180), (sx + 25, sy - 30, 165), 3, 6)
    cyl("Collision", CL, sx, sy, 0, 190, 55, 8)
    # ammo crate stacks
    for (cx, cy, n) in ((1480.0, -420.0, 3), (1100.0, -560.0, 2)):
        for k in range(n):
            crate(p, cx, cy, k * 52, 100, 64, 50, yaw=k * 7.0, collide=(k == 0))
        col(cx - 60, cx + 60, cy - 42, cy + 42, 0, n * 52)
    # workbench + tool board on the spine wall, forward of the door
    box(p, ST, 1260, 1560, y1 - 70, y1, 88, 94)
    for xl in (1270.0, 1550.0):
        box(p, HT, xl - 5, xl + 5, y1 - 65, y1 - 5, 0, 88)
    box(p, HT, 1380, 1420, y1 - 60, y1 - 30, 94, 118)
    box(p, BH, 1260, 1560, y1 - 6, y1, 120, 230)
    for k in range(6):
        box(p, HT, 1290 + k * 45, 1294 + k * 45, y1 - 10, y1 - 6, 150, 190 + (k % 3) * 12)
    col(1260, 1560, y1 - 70, y1, 0, 100)
    room_services(p, x0, x1, y0, y1, duct=False, skip_x=[(1080, 1560)])
    for (x, y) in ((850.0, -420.0), (1300.0, -300.0), (1300.0, -580.0)):
        ceiling_panel(p, x, y, H_BLOCK - 2, 0.0, 150)
    light("W", 5, 850, -420, H_BLOCK - 30)
    light("W", 6, 1300, -440, H_BLOCK - 30)
    cage_lamp(p, 1300, y0, 200, 90.0)
    light("R", 3, 1300, y0 + 50, 200)


def build_hangar():
    p = "Hangar"
    x0, x1, y0, y1 = X_HGR0 + 15, X_HGR1 - 15, -Y_HGR + 15, Y_HGR - 15
    # --- wall ribs + deckhead trusses ---
    ribs(p, (X_HGR0, Y_HGR), (X_HGR1, Y_HGR), -1, H_BAY, step=350)
    ribs(p, (X_HGR0, -Y_HGR), (X_HGR1, -Y_HGR), 1, H_BAY, step=350,
         skip=[(PCX - X_HGR0 - 760, PCX - X_HGR0 + 760)])
    ribs(p, (X_HGR0, -Y_HGR), (X_HGR0, Y_HGR), -1, H_BAY, step=380, skip=[(Y_HGR - 200, Y_HGR + 200)])
    for y in range(-1050, 1100, 420):
        box(p, BH, x0, x1, y - 16, y + 16, H_BAY - 60, H_BAY)
        box(p, HT, x0, x1, y - 26, y + 26, H_BAY - 64, H_BAY - 60)
    for x in (-2800.0, -2450.0, -2100.0, -1750.0, -1400.0):
        box(p, BH, x - 10, x + 10, y0, y1, H_BAY - 40, H_BAY)
    # --- floor markings: landing pad, launch lane, tie-downs ---
    K[p].annulus(HZ, PCX, PCY, 820, 850, 0, 0.6, 48)
    K[p].annulus(HZ, PCX, PCY, 200, 225, 0, 0.6, 32)
    for s in (-1, 1):
        box(p, HZ, PCX + s * 500 - 15, PCX + s * 500 + 15, y0, PCY - 820, 0, 0.6)
    for gx in range(-3000, -1150, 300):
        for gy in range(-1050, 1100, 300):
            cyl(p, HT, gx, gy, 0, 1.5, 8, 8)
    # --- launch door (starboard wall) ---
    dx0, dx1 = PCX - 700, PCX + 700
    ydw = -Y_HGR
    for s in (-1, 1):
        xa, xb = (dx0, PCX - 2) if s < 0 else (PCX + 2, dx1)
        box(p, HT, xa, xb, ydw - 12, ydw + 20, 0, 800)
        col(xa, xb, ydw - 12, ydw + 20, 0, 800)
        for z in range(60, 800, 110):
            box(p, BH, xa + 20, xb - 20, ydw + 20, ydw + 34, z, z + 40)
        xe = PCX - 2 if s < 0 else PCX + 2
        for z in range(0, 800, 90):
            box(p, HZ, xe - s * 70, xe, ydw + 20, ydw + 24, z, z + 45)
    box(p, BH, dx0 - 60, dx1 + 60, ydw - 12, ydw + 46, 800, 870)
    for s in (-1, 1):
        xj = dx0 - 30 if s < 0 else dx1 + 30
        box(p, BH, xj - 30, xj + 30, ydw - 12, ydw + 46, 0, 870)
        box(p, HZ, xj - 31, xj + 31, ydw + 46, ydw + 48, 0, 800)
        cyl(p, HT, xj, ydw + 70, 880, 895, 22, 12)
        cyl("Lights", LR, xj, ydw + 70, 895, 925, 17, 12)
        light("R", 6, xj, ydw + 150, 880)
    # --- dropship on the pad, nose to the launch door ---
    build_dropship(PCX, PCY)
    # --- catwalk along the forward wall at CAT_Z, stairs up the port wall ---
    cz = CAT_Z
    cx0 = X_HGR1 - 15 - 260
    solid(p, GR, cx0, X_HGR1 - 15, -900, y1, cz - 10, cz)
    box(p, HT, cx0, X_HGR1 - 15, -900, y1, cz - 36, cz - 10)
    for y in range(-900, 1050, 380):
        pipe(p, HT, (cx0 + 20, y, 0), (cx0 + 20, y, cz - 36), 8, 10)
        cyl(p, HT, cx0 + 20, y, 0, 6, 24, 10)
        pipe(p, HT, (cx0 + 20, y, cz - 220), (X_HGR1 - 15, y, cz - 36), 4, 8)
        cyl("Collision", CL, cx0 + 20, y, 0, cz - 36, 13, 8)
    sy0 = 900.0                                     # stair lane along the port wall
    railing(p, (cx0, -900), (cx0, sy0), cz)
    railing(p, (cx0, -900), (X_HGR1 - 15, -900), cz)
    run = 560.0
    stairs(p, cx0 - run, (sy0 + y1) / 2, run, y1 - sy0, 0, cz, 0.0, rails=(True, False))
    box(p, HZ, cx0 - run - 60, cx0 - run, sy0, y1, 0, 0.6)
    # crane-control station on the catwalk, looking out over the bay
    console(p, cx0 + 72, -600, 0.0, n_crt=2, z=cz)
    # flight-control booth: two consoles at the window over the hangar
    for y in (430.0, 610.0):
        console(p, X_HGR1 + 15 + 70, y, 0.0, n_crt=2, z=cz)
    bx0, bx1, by0, by1 = BOOTH
    ceiling_panel(p, (bx0 + bx1) / 2, (by0 + by1) / 2, cz + 250, 0.0, 150)
    light("W", 4, (bx0 + bx1) / 2, (by0 + by1) / 2, cz + 200)
    # --- gantry crane ---
    for xr in (x0 + 30, x1 - 30):
        box(p, BH, xr - 22, xr + 22, y0, y1, 820, 838)
        box(p, BH, xr - 5, xr + 5, y0, y1, 838, 872)
        box(p, BH, xr - 22, xr + 22, y0, y1, 872, 886)
    yb = PCY + 100
    box(p, HZ, x0 + 30, x1 - 30, yb - 36, yb + 36, 886, 930)
    box(p, BH, x0 + 30, x1 - 30, yb - 40, yb + 40, 876, 886)
    box(p, ST, PCX - 90, PCX + 90, yb - 60, yb + 60, 790, 876)
    pipe(p, HT, (PCX, yb, 790), (PCX, yb, 700), 2.5, 6)
    box(p, HZ, PCX - 26, PCX + 26, yb - 26, yb + 26, 650, 700)
    # --- ordnance, crates, tug, fuel cart ---
    for k in range(3):
        crate(p, x0 + 95, 420 + k * 230, 0, 170, 170, 140, yaw=0.0)
        crate(p, x0 + 95, 420 + k * 230, 140, 150, 150, 100, yaw=4.0, collide=False)
    col(x0 + 5, x0 + 185, 330, 1060, 0, 245)
    for k in range(2):
        mx, my = -2650.0 + k * 330, 980.0
        box(p, HT, mx - 140, mx + 140, my - 45, my + 45, 20, 38)
        for s in (-1, 1):
            pipe(p, HT, (mx + s * 110, my - 50, 11), (mx + s * 110, my + 50, 11), 11, 10)
        for j in range(3):
            yy = my - 28 + j * 28
            pipe(p, BH, (mx - 130, yy, 56), (mx + 110, yy, 56), 11, 10)
            pipe(p, HZ, (mx + 110, yy, 56), (mx + 138, yy, 56), 7, 10)
            box(p, HT, mx - 140, mx - 112, yy - 16, yy + 16, 42, 70)
        col(mx - 145, mx + 145, my - 55, my + 55, 0, 85)
    tx, ty = -1600.0, -950.0
    box(p, HZ, tx - 180, tx + 180, ty - 100, ty + 100, 28, 100)
    box(p, BH, tx - 160, tx - 36, ty - 90, ty + 90, 100, 180)
    box(p, VP, tx - 38, tx - 34, ty - 80, ty + 80, 118, 172)
    for sx in (-115, 115):
        for sy in (-1, 1):
            pipe(p, HT, (tx + sx, ty + sy * 90, 30), (tx + sx, ty + sy * 112, 30), 30, 12)
    col(tx - 185, tx + 185, ty - 118, ty + 118, 0, 180)
    fx, fy = -2950.0, -500.0
    box(p, HZ, fx - 80, fx + 80, fy - 55, fy + 55, 18, 36)
    cyl(p, ST, fx - 36, fy, 36, 160, 40, 14)
    cyl(p, ST, fx + 44, fy, 36, 160, 40, 14)
    pipe(p, PI, (fx + 80, fy, 130), (PCX - 450, PCY - 150, 5), 5, 8)
    col(fx - 85, fx + 85, fy - 60, fy + 60, 0, 160)
    # wall pipes, bulkhead lockers along the port wall
    for zz, r in ((700.0, 18.0), (748.0, 12.0)):
        pipe(p, PI, (x0, y1 - 36, zz), (cx0 - run - 80, y1 - 36, zz), r, 12)
    locker_bank(p, -2700, -2200, y1, 55, -1)
    extinguisher(p, -2150, y1, 1)
    # floodlights
    for (x, y) in ((-2700, -600), (-1600, -600), (-2700, 500), (-1600, 500)):
        flood(p, x, y, H_BAY - 64)
        light("A", 12, x, y, H_BAY - 180, shadow=(y > 0))
    light("W", 5, cx0 + 130, 0, cz + 230)
    for y in (-700.0, 150.0, 800.0):
        cage_lamp(p, cx0 + 10, y, cz + 190, 180.0, color=LA)


def build_dropship(cx, cy):
    """Chunky drop-ship (APC carrier) parked on the pad. Local +X = nose,
    rotated so the nose points at the launch door (-Y)."""
    p = "Dropship"
    M = Mx(cx, cy, 0, -90.0)
    W = 200.0
    side = [(-640, 130), (430, 130), (650, 200), (700, 250), (560, 330), (300, 385), (-470, 385), (-640, 330)]
    extrude(p, BH, side, 'y', -W, W, M)
    # side armour skirts + hazard stencil band
    for s in (-1, 1):
        box(p, HT, -560, 420, s * W, s * (W + 8), 140, 200, M)
        box(p, HZ, -300, -120, s * (W + 8), s * (W + 9), 150, 190, M)
    extrude(p, BH, [(-470, 385), (300, 385), (220, 420), (-420, 420)], 'y', -W + 50, W - 50, M)   # spine hump
    # canopy on the nose slope
    K[p].slope_panel(VP, (560, 330), (700, 250), -130, 130, lift=0.8, thick=2.0, inset=0.08, M=M)
    K[p].slope_panel(BH, (300, 385), (560, 330), -140, 140, lift=0.5, thick=3.0, inset=0.04, M=M)
    # stub wings with rocket pods
    for s in (-1, 1):
        box(p, BH, -260, 120, s * W, s * 640, 250, 290, M)
        box(p, HZ, 90, 120, s * (W + 10), s * 630, 251, 289, M)
        c0 = M @ Vector((-200, s * 560, 225)); c1 = M @ Vector((160, s * 560, 225))
        pipe(p, HT, c0, c1, 32, 12)
        for k in range(3):
            for j in range(2):
                q0 = M @ Vector((160, s * 560 - 14 + k * 14, 225 - 7 + j * 14))
                pipe(p, HT, q0, q0 + (M.to_3x3() @ Vector((6, 0, 0))), 5, 6)
        # engine nacelle
        e0 = M @ Vector((-620, s * 420, 330)); e1 = M @ Vector((-120, s * 420, 330))
        pipe(p, BH, e0, e1, 95, 16)
        pipe(p, HT, e0 + (M.to_3x3() @ Vector((-20, 0, 0))), e0, 80, 16)
        box(p, BH, -520, -220, s * W, s * 330, 300, 360, M)
        # tail fin
        extrude(p, BH, [(-640, 385), (-420, 385), (-560, 620), (-660, 620)], 'y', s * (W - 60) - 8, s * (W - 60) + 8, M)
    # landing gear
    for (gx, gy) in ((420, 120), (420, -120), (-420, 170), (-420, -170)):
        c0 = M @ Vector((gx, gy, 140)); c1 = M @ Vector((gx, gy, 12))
        pipe(p, HT, c0, c1, 12, 10)
        pipe(p, ST, c0 + Vector((0, 0, -20)), (c0 + c1) / 2 + Vector((0, 0, -10)), 7, 8)
        f = M @ Vector((gx, gy, 0))
        cyl(p, HT, f.x, f.y, 0, 12, 40, 12)
    # rear ramp lowered
    extrude(p, HT, [(-640, 130), (-640, 150), (-900, 12), (-900, 0)], 'y', -170, 170, M)
    extrude(p, HZ, [(-890, 0), (-900, 0), (-900, 14), (-890, 14)], 'y', -170, 170, M)
    box(p, HT, -645, -600, -170, 170, 150, 360, M)          # dark cargo bay mouth
    box("Lights", LR, -600, -596, -150, 150, 350, 356, M)
    # nav lights
    for s in (-1, 1):
        q = M @ Vector((110, s * 645, 270))
        box("Lights", LG if s > 0 else LR, q.x - 6, q.x + 6, q.y - 6, q.y + 6, q.z - 6, q.z + 6)
    # walk blocker around the hull (wings/nacelles are above head height)
    box("Collision", CL, -910, 710, -W - 10, W + 10, 0, 420, M)
    for (gx, gy) in ((420, 120), (420, -120), (-420, 170), (-420, -170)):
        c = M @ Vector((gx, gy, 0))
        cyl("Collision", CL, c.x, c.y, 0, 140, 42, 8)


def build_engineering():
    p = "Engineering"
    x0, x1, y0, y1 = X_ENG0 + 15, X_ENG1 - 15, -Y_ENG + 15, Y_ENG - 15
    rx, ry = RX, RY
    H = H_ENG
    # floor: grating everywhere, hazard walkway around the plinth
    box(p, GR, x0, x1, y0, y1, 0, 0.5)
    K[p].annulus(HZ, rx, ry, 230, 270, 0, 0.8, 32)
    # reactor: plinth, core housing, glowing bands, collars, conduits
    cyl(p, ST, rx, ry, 0, 70, 230, 32)
    cyl(p, HT, rx, ry, 70, H, 140, 32)
    for (za, zb) in ((150, 200), (470, 530), (680, 730), (820, 860)):
        cyl("Lights", LA, rx, ry, za, zb, 143, 32)
        K[p].annulus(BH, rx, ry, 138, 165, za - 16, za, 32)
        K[p].annulus(BH, rx, ry, 138, 165, zb, zb + 16, 32)
    for k in range(8):
        t = math.radians(k * 45 + 22.5)
        pipe(p, PI, (rx + 180 * math.cos(t), ry + 180 * math.sin(t), 70),
             (rx + 180 * math.cos(t), ry + 180 * math.sin(t), H), 10, 10)
    cyl("Collision", CL, rx, ry, 0, 70, 235, 32)                                    # plinth
    cyl("Collision", CL, rx, ry, 70, H, 195, 32)                                    # core + conduits
    # catwalk ring + gallery along the aft wall + bridge between them
    rz = ENG_RZ
    r_in, r_out = 205.0, 350.0
    K[p].annulus(GR, rx, ry, r_in, r_out, rz - 8, rz, 48)
    K[p].annulus(HT, rx, ry, r_out - 12, r_out, rz - 28, rz - 8, 48)
    K["Collision"].annulus(CL, rx, ry, 195, r_out, rz - 20, rz, 48)
    K["Collision"].annulus(CL, rx, ry, 195, r_in + 10, rz, rz + 130, 48)                # inner rail
    gap = math.degrees(math.asin(75.0 / (r_out - 4)))
    K["Collision"].annulus(CL, rx, ry, r_out - 12, r_out, rz, rz + 130, 48, -180 + gap, 180 - gap)
    for r in (r_in + 8, r_out - 6):
        full = r < 250
        a0, a1 = (0.0, 360.0) if full else (-180 + gap, 180 - gap)
        pts = circle(rx, ry, r, 48, a0, a1)
        n = len(pts) if full else len(pts) - 1
        for i in range(n):
            q0 = pts[i]; q1 = pts[(i + 1) % len(pts)]
            pipe(p, HZ, (q0[0], q0[1], rz + 105), (q1[0], q1[1], rz + 105), 3, 6)
            if i % 3 == 0:
                pipe(p, HT, (q0[0], q0[1], rz), (q0[0], q0[1], rz + 105), 3, 6)
    for a in (45.0, 135.0, 225.0, 315.0):
        t = math.radians(a)
        q = (rx + (r_out - 15) * math.cos(t), ry + (r_out - 15) * math.sin(t))
        pipe(p, HT, (q[0], q[1], 0), (q[0], q[1], rz - 28), 10, 10)
        cyl("Collision", CL, q[0], q[1], 0, rz - 28, 14, 8)
    gx1 = GAL_X
    solid(p, GR, x0, gx1, y0, y1, rz - 10, rz)
    box(p, HT, x0, gx1, y0, y1, rz - 36, rz - 10)
    solid(p, GR, gx1 - 20, rx - r_out + 30, -75, 75, rz - 10, rz)
    railing(p, (gx1, -75), (rx - r_out, -75), rz)
    railing(p, (gx1, 75), (rx - r_out, 75), rz)
    railing(p, (gx1, y0), (gx1, -75), rz)
    railing(p, (gx1, 75), (gx1, 560), rz)
    for y in range(-700, 750, 350):
        pipe(p, HT, (gx1 - 18, y, 0), (gx1 - 18, y, rz - 36), 9, 10)
        cyl("Collision", CL, gx1 - 18, y, 0, rz - 36, 13, 8)
    # stair from the floor up the port wall to the gallery (climbs aft)
    sy0 = 580.0
    stairs(p, gx1 + 500, (sy0 + y1) / 2, 500.0, y1 - sy0, 0, rz, 180.0, rails=(False, True))
    box(p, HZ, gx1 + 500, gx1 + 560, sy0, y1, 0, 0.8)
    # gallery consoles facing the aft bulkhead
    for y in (-450.0, 0.0, 450.0):
        Mg = Mx(x0 + 72, y, rz, 0.0)
        w = 200.0
        extrude(p, ST, [(-70, 0), (0, 0), (0, 70), (-6, 76), (-44, 104), (-70, 106)], 'y', -w / 2, w / 2, Mg)
        for k in range(2):
            c = Mg @ Vector((-40, -w / 2 + 55 + k * 90, 106))
            crt(p, c.x, c.y, c.z, 56, 44, 0.0, screen=LA if k == 1 else LG, d=38)
        box("Collision", CL, -72, 2, -w / 2, w / 2, 0, 120, Mg)
    box(p, HT, x0, x0 + 18, -650, 650, rz + 130, rz + 300)                         # wall schematic
    box(p, CO, x0 + 18, x0 + 20, -635, 635, rz + 145, rz + 285)
    # floor consoles either side of the hangar door
    console(p, x1 - 72, 450, 180.0, n_crt=2)
    console(p, x1 - 72, -450, 180.0, n_crt=2, screen=LA)
    # coolant tanks along the starboard wall + feed pipes to the reactor
    for tx in (-4000.0, -3700.0, -3400.0):
        ty = y0 + 110
        cyl(p, ST, tx, ty, 0, 560, 90, 24)
        cyl(p, HT, tx, ty, 560, 590, 75, 24)
        cyl(p, HT, tx, ty, 0, 24, 98, 24)
        for zb in (120.0, 320.0, 500.0):
            K[p].annulus(BH, tx, ty, 88, 96, zb, zb + 12, 24)
        box(p, HZ, tx - 30, tx + 30, ty + 88, ty + 91, 240, 265)
        cyl("Collision", CL, tx, ty, 0, 590, 98, 12)
        q = Vector((tx, ty + 90, 260)); r_ = Vector((rx, ry, 260))
        d = (r_ - q); d.z = 0
        end = r_ - d.normalized() * 150
        pipe(p, PI, q, end, 12, 12)
        pipe(p, HT, end, (end.x, end.y, 200), 9, 10)
    # overhead coolant headers to the walls
    for (ex, ey) in ((x0, 0.0), (rx, y1), (rx, y0), (x1, 0.0)):
        q = Vector((ex, ey, 800)); r_ = Vector((rx, ry, 800))
        d = (q - r_); d.z = 0
        pipe(p, PI, r_ + d.normalized() * 145, q, 26, 14)
    for y in (-600.0, -200.0, 200.0, 600.0):
        pipe(p, HT, (x1, y, 860), (x0, y, 860), 8, 8)
    ribs(p, (X_ENG0, Y_ENG), (X_ENG1, Y_ENG), -1, H, step=350, slot=BH)
    ribs(p, (X_ENG1, -Y_ENG), (X_ENG0, -Y_ENG), -1, H, step=350, slot=BH)
    extinguisher(p, x1 - 200, y1, 1)
    # lights: amber around the core, work lights on the gallery, white by the door
    for a in (0.0, 90.0, 180.0, 270.0):
        t = math.radians(a + 45)
        light("A", 8, rx + 450 * math.cos(t), ry + 450 * math.sin(t), 300, shadow=(a == 0.0))
    for y in (-550.0, 550.0):
        flood(p, x0 + 120, y, H)
        light("W", 6, x0 + 120, y, 700)
    for y in (-600.0, 600.0):
        flood(p, x1 - 160, y, H, color=LW)
        light("W", 8, x1 - 160, y, 720)
    for (x, y, yaw) in ((x1, 350, 180.0), (x1, -350, 180.0), (x0, 700, 0.0), (x0, -700, 0.0)):
        cage_lamp(p, x, y, 260, yaw)


# ----------------------------------------------------------------------------
# Export + contract
# ----------------------------------------------------------------------------
def export(ob, dry):
    out = os.path.join(GEN, ob.name + ".fbx")
    if dry:
        return out
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE',
    )
    return out


def validate(objs):
    problems = []
    shell = objs["Shell"]
    pts = [shell.matrix_world @ v.co for v in shell.data.vertices]
    mn = Vector([min(p[i] for p in pts) for i in range(3)])
    mx = Vector([max(p[i] for p in pts) for i in range(3)])
    for name, ob in objs.items():
        for v in ob.data.vertices:
            p = v.co
            if any(p[i] < mn[i] - 1 or p[i] > mx[i] + 1 for i in range(3)):
                problems.append(f"{name}: vertex {tuple(round(c, 1) for c in p)} outside shell bounds")
                break
    for k in ("Entry", "Seat"):
        if k not in SOCKETS:
            problems.append("missing socket " + k)
    tris = {}
    for name, ob in objs.items():
        ob.data.calc_loop_triangles()
        tris[name] = len(ob.data.loop_triangles)
    # walk check + route for Tools/pie_walk_ship_decks.py (shared with the socket decks)
    import build_ship_decks as bsd
    bsd.SOCKETS = SOCKETS
    bsd.SPACES = SPACES
    wc = bsd.walk_check(objs["Collision"])
    if not wc.get("entry_free"):
        problems.append("entry is inside a walk blocker")
    elif not wc.get("seat_reachable"):
        problems.append("helm seat not reachable from entry")
    for n in wc.get("unreached", []):
        problems.append(f"space {n} not reachable at deck level ({wc['rooms'][n]:.0%})")
    nl = sum(1 for k in SOCKETS if k.startswith("L"))
    if nl > 64:
        problems.append(f"{nl} light sockets (> 64)")
    return {
        "prefix": PREFIX,
        "ship": "Battleship",
        "bp": "/Game/Blueprints/Ships/BP_Battleship",
        "parts": [q for q in PARTS if q != "Collision"],
        "walk": wc,
        "size_m": [round((mx.x - mn.x) / 100, 1), round((mx.y - mn.y) / 100, 1)],
        "family": "BattleshipDecks",
        "runtime_scale_on_imported_mesh": 0.01,
        "design_bounds_cm": {"min": [round(v, 1) for v in mn], "max": [round(v, 1) for v in mx]},
        "sockets_ue_design_cm": SOCKETS,
        "light_count": sum(1 for k in SOCKETS if k.startswith("L")),
        "tris": tris, "tris_total": sum(tris.values()),
        "slots": {n: [m.name for m in ob.data.materials] for n, ob in objs.items()},
        "blender_to_ue_axes": "UE(x, y, z) = Blender(x, -y, z) * 100 * 0.01",
        "problems": problems,
    }


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    dry = "--dry-run" in argv
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = 'METRIC'
    sc.unit_settings.length_unit = 'CENTIMETERS'
    reset()
    build_structure()
    build_spine()
    build_cic()
    build_quarters()
    build_mess()
    build_medbay()
    build_briefing()
    build_armory()
    build_hangar()
    build_engineering()
    objs = {p: K[p].to_object() for p in PARTS}
    for ob in objs.values():
        for poly in ob.data.polygons:
            poly.use_smooth = False
    info = validate(objs)
    print("CONTRACT", json.dumps({k: v for k, v in info.items() if k not in ("sockets_ue_design_cm", "slots", "walk")}, indent=1))
    print("WALK rooms", info["walk"].get("rooms"), "seat", info["walk"].get("seat_reachable"))
    for p in PARTS:
        print("  export", export(objs[p], dry), info["tris"][p], "tris")
    if not dry:
        with open(os.path.join(GEN, PREFIX + "_contract.json"), "w") as f:
            json.dump(info, f, indent=1)
    if info["problems"]:
        print("CONTRACT_PROBLEMS", len(info["problems"]), info["problems"][:10])


if __name__ == "__main__":
    main()
