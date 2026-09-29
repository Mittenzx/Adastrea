"""Battleship full-deck walkable interior (Blender 5.x, headless).

    blender -b --python Tools/build_battleship_decks.py -- [--dry-run]

A 118 m x 34 m military deck for the Battleship, in the grimy 1990s
"marine transport" style (Aliens / Starship Troopers / Space: Above and
Beyond) rather than a clean Trek bridge: ribbed chamfered corridors, grating,
yellow-black hazard edging, exposed pipe runs, caged red emergency lamps,
green-phosphor CRT consoles, olive-drab bulkheads.

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

# Heights (cm)
H_BLOCK = 420      # spine-block rooms
H_SPINE = 340      # corridor ceiling (walls run on to H_BLOCK above it)
H_CIC = 650
H_BAY = 1300       # hangar + engineering
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
    """Three-high rack bunk: tube frame, trays, mattresses, end panels."""
    for (x, y) in ((x0 + 3, y0 + 3), (x1 - 3, y0 + 3), (x0 + 3, y1 - 3), (x1 - 3, y1 - 3)):
        pipe(part, HT, (x, y, 0), (x, y, 245), 3.0, 8)
    for z in (28.0, 110.0, 192.0):
        box(part, ST, x0, x1, y0, y1, z, z + 8)
        box(part, BU, x0 + 6, x1 - 6, y0 + 5, y1 - 5, z + 8, z + 22)
        box(part, BU, x1 - 40, x1 - 8, y0 + 12, y1 - 12, z + 22, z + 30)          # pillow
        box(part, HT, x0, x1, y0 - 1, y0 + 1, z + 8, z + 30)                        # rail lip
        box("Lights", LA, x1 - 30, x1 - 12, y1 - 2, y1 - 1, z + 60, z + 66)         # reading lamp
    box(part, BH, x0 - 2, x0 + 2, y0, y1, 0, 245)
    box(part, BH, x1 - 2, x1 + 2, y0, y1, 0, 245)
    box(part, ST, x0, x1, y0, y1, 245, 250)
    if collide:
        col(x0, x1, y0, y1, 0, 250)


# ----------------------------------------------------------------------------
# Zones
# ----------------------------------------------------------------------------
X_ENG0, X_ENG1 = -6000.0, -4400.0
X_HGR0, X_HGR1 = -4400.0, -2000.0
X_BLK0, X_BLK1 = -2000.0, 3600.0
X_CIC1 = 5800.0
Y_BLK = 1400.0
Y_HGR = 1700.0
Y_SP = 200.0          # spine wall lines at +/- this
SPINE_DOORS = {        # room door centres (x) on each spine wall
    +1: [-1000.0, 900.0, 2100.0],     # berths, mess, briefing
    -1: [-1000.0, 500.0, 2100.0],     # ready room, medbay, armory
}


def build_structure():
    sh = "Shell"
    # --- floors (visual) + one walk floor for everything ---------------------
    box(sh, DK, X_ENG0, X_ENG1, -Y_BLK, Y_BLK, -20, 0)
    box(sh, DK, X_HGR0, X_HGR1, -Y_HGR, Y_HGR, -20, 0)
    box(sh, DK, X_BLK0, X_BLK1, -Y_BLK, Y_BLK, -20, 0)
    cic_poly = [(X_BLK1, -1300), (5200, -1300), (X_CIC1, -700), (X_CIC1, 700), (5200, 1300), (X_BLK1, 1300)]
    K[sh].prism_xy(DK, cic_poly, -20, 0)
    col(X_ENG0, X_CIC1, -Y_HGR, Y_HGR, -20, 0)

    # --- ceilings ---------------------------------------------------------
    box(sh, SH, X_ENG0, X_ENG1, -Y_BLK, Y_BLK, H_BAY, H_BAY + 20)
    box(sh, SH, X_HGR0, X_HGR1, -Y_HGR, Y_HGR, H_BAY, H_BAY + 20)
    box(sh, SH, X_BLK0, X_BLK1, -Y_BLK, Y_BLK, H_BLOCK, H_BLOCK + 20)
    K[sh].prism_xy(SH, cic_poly, H_CIC, H_CIC + 20)

    # --- engineering walls ------------------------------------------------
    wall(sh, EN, (X_ENG0, -Y_BLK), (X_ENG0, Y_BLK), 0, H_BAY, frames=False)
    wall(sh, EN, (X_ENG0, Y_BLK), (X_ENG1, Y_BLK), 0, H_BAY, frames=False)
    wall(sh, EN, (X_ENG1, -Y_BLK), (X_ENG0, -Y_BLK), 0, H_BAY, frames=False)
    # --- hangar walls -----------------------------------------------------
    # aft (shared with engineering): big open blast door into engineering
    wall(sh, SH, (X_HGR0, -Y_HGR), (X_HGR0, Y_HGR), 0, H_BAY, openings=[(Y_HGR, 400, 0, 500)])
    wall(sh, SH, (X_HGR0, Y_HGR), (X_HGR1, Y_HGR), 0, H_BAY, frames=False)
    # starboard: launch door opening, filled by the door leaves in build_hangar
    wall(sh, SH, (X_HGR1, -Y_HGR), (X_HGR0, -Y_HGR), 0, H_BAY,
         openings=[(X_HGR1 - (-3200.0), 1400, 0, 900)], frames=False)
    # forward (hangar -> spine block): spine door, ready-room door, booth door
    # + booth window at catwalk level
    wall(sh, SH, (X_HGR1, -Y_HGR), (X_HGR1, Y_HGR), 0, H_BAY,
         openings=[(Y_HGR - 900.0, 200, 0, 240),      # ready room (y = -900)
                   (Y_HGR + 0.0, 300, 0, 280),        # spine
                   (Y_HGR + 240.0, 180, 600, 820),    # booth door (y = 240)
                   (Y_HGR + 570.0, 380, 680, 860)],   # booth window (y = 570)
         frames=False)
    for (y, w, zb, zt) in ((-900.0, 200, 0, 240), (0.0, 300, 0, 280), (240.0, 180, 600, 820)):
        door_frame(sh, X_HGR1, y, zb, 90.0, w, zt - zb)
    # booth window glass + frame
    seg_box(sh, VP, (X_HGR1, 380), (X_HGR1, 760), 4, 680, 860)
    for y in (380, 570, 760):
        seg_box(sh, BH, (X_HGR1, y - 8), (X_HGR1, y + 8), WT + 20, 680, 860)
    seg_box(sh, BH, (X_HGR1, 370), (X_HGR1, 770), WT + 24, 664, 680)
    seg_box(sh, BH, (X_HGR1, 370), (X_HGR1, 770), WT + 24, 860, 878)

    # --- spine block ------------------------------------------------------
    wall(sh, SH, (X_BLK0, Y_BLK), (X_BLK1, Y_BLK), 0, H_BLOCK, frames=False)
    wall(sh, SH, (X_BLK1, -Y_BLK), (X_BLK0, -Y_BLK), 0, H_BLOCK, frames=False)
    for side in (1, -1):
        y = side * Y_SP
        ops = [(x - X_BLK0, 200, 0, 240) for x in SPINE_DOORS[side]]
        wall(sh, SH, (X_BLK0, y), (X_BLK1, y), 0, H_BLOCK, openings=ops)
        for xd in (0.0, 1800.0):
            wall(sh, SH, (xd, y), (xd, side * Y_BLK), 0, H_BLOCK, frames=False)
    # forward wall: spine block -> CIC (CIC side is taller)
    wall(sh, SH, (X_BLK1, -Y_BLK), (X_BLK1, Y_BLK), 0, H_CIC, openings=[(Y_BLK, 300, 0, 280)])

    # --- CIC walls: sides solid, bow + chamfers with armoured slit windows ----
    wall(sh, SH, (X_BLK1, 1300), (5200, 1300), 0, H_CIC, frames=False)
    wall(sh, SH, (5200, -1300), (X_BLK1, -1300), 0, H_CIC, frames=False)
    for p0, p1 in (((5200, -1300), (X_CIC1, -700)), ((X_CIC1, -700), (X_CIC1, 700)), ((X_CIC1, 700), (5200, 1300))):
        window_band(sh, p0, p1, 0, H_CIC, 300, 390)

    # --- flight-control booth (above the berths, off the hangar catwalk) ----
    bx0, bx1, by0, by1, bz = X_HGR1, -1500.0, 100.0, 800.0, 600.0
    solid(sh, DK, bx0, bx1, by0, by1, bz - 20, bz)
    box(sh, SH, bx0, bx1, by0, by1, 940, 960)
    wall(sh, SH, (bx1, by0), (bx1, by1), bz, 940, frames=False)
    wall(sh, SH, (bx0, by0), (bx1, by0), bz, 940, frames=False)
    wall(sh, SH, (bx1, by1), (bx0, by1), bz, 940, frames=False)


def build_spine():
    p = "Spine"
    x0, x1 = X_BLK0 + 15, X_BLK1 - 15
    doors = {s: SPINE_DOORS[s] for s in (1, -1)}
    all_doors = doors[1] + doors[-1]
    # ceiling, chamfer wedges, floor grating + gutters
    box(p, SH, x0, x1, -185, 185, H_SPINE, H_SPINE + 10)
    for s in (1, -1):
        prof = [(s * 185, 240), (s * 185, H_SPINE), (s * 85, H_SPINE)]
        extrude(p, BH, prof, 'x', x0, x1)
        box(p, GR, x0, x1, s * 118, s * 172, 0, 0.6)                                # side gutter grille
        box(p, HT, x0, x1, s * 172, s * 185, 0, 14)                                 # kick trim
    box(p, GR, x0, x1, -60, 60, 0, 0.6)                                             # centre grating
    for s in (1, -1):
        box(p, HZ, x0, x1, s * 60, s * 68, 0, 0.7)                                  # hazard edge lines
    # ribs every 300, skipping doors; recessed ceiling fixture between each pair
    xs = []
    x = x0 + 150
    while x < x1 - 100:
        xs.append(x)
        x += 300
    for i, xr in enumerate(xs):
        near_door = [s for s in (1, -1) if any(abs(xr - d) < 170 for d in doors[s])]
        for s in (1, -1):
            if s in near_door:
                continue
            box(p, BH, xr - 15, xr + 15, s * 163, s * 185, 0, 245)
            box(p, HZ, xr - 16, xr + 16, s * 162, s * 186, 0, 22)                    # striped foot
            A = Vector((s * 185, 240)); B = Vector((s * 85, H_SPINE))
            nrm = Vector((-s, -1)).normalized() * 22
            extrude(p, BH, [tuple(A), tuple(B), tuple(B + nrm), tuple(A + nrm)], 'x', xr - 15, xr + 15)
        box(p, BH, xr - 15, xr + 15, -85, 85, H_SPINE - 22, H_SPINE)
        if i < len(xs) - 1:
            xm = (xr + xs[i + 1]) / 2
            ceiling_panel(p, xm, 0, H_SPINE, 0.0, 150)
            if i % 2 == 0:
                light("W", 7, xm, 0, H_SPINE - 30)
    # pipe runs along both upper chamfers, brackets at the ribs
    for s in (1, -1):
        for (yy, zz, r, slot) in ((140, 255, 8, PI), (118, 280, 6, HT), (96, 304, 4.5, PI)):
            pipe(p, slot, (x0, s * yy, zz), (x1, s * yy, zz), r, 10)
        # cable tray at knee height on the port side, conduit on starboard
        if s > 0:
            box(p, HT, x0, x1, 170, 185, 60, 64)
            box(p, HT, x0, x1, 178, 185, 60, 78)
        else:
            pipe(p, HT, (x0, -178, 70), (x1, -178, 70), 4, 8)
    # between ribs: panels, vents, intercom terminals, caged red lamps
    for i in range(len(xs) - 1):
        xa, xb = xs[i] + 15, xs[i + 1] - 15
        xm = (xa + xb) / 2
        for s in (1, -1):
            if any(xa - 120 < d < xb + 120 for d in doors[s]):
                continue
            box(p, BH, xa + 10, xb - 10, s * 181, s * 185, 90, 225)
            if (i + (s > 0)) % 3 == 0:
                for v in range(6):
                    box(p, HT, xm - 45, xm + 45, s * 179, s * 181, 110 + v * 14, 116 + v * 14)
            elif (i + (s > 0)) % 3 == 1:
                box(p, ST, xm - 22, xm + 22, s * 172, s * 181, 120, 165)              # intercom
                box("Lights", LG, xm - 15, xm + 15, s * 171, s * 172, 138, 160)
                box(p, HT, xm - 20, xm + 20, s * 170, s * 172, 124, 132)
            else:
                cage_lamp(p, xm, s * 181, 205, -90 if s > 0 else 90)
    # stencilled deck-section plates at each door (hazard band over the door)
    for s in (1, -1):
        for d in doors[s]:
            box(p, HZ, d - 165, d - 135, s * 180, s * 185, 30, 200)
            box(p, HZ, d + 135, d + 165, s * 180, s * 185, 30, 200)
    # end bulkhead frames
    for xe in (x0, x1):
        for s in (1, -1):
            box(p, BH, xe - 20, xe + 20, s * 150, s * 185, 0, H_SPINE)


def build_cic():
    p = "CIC"
    cx, cy = 4400.0, 0.0
    # raised dais (two 20 cm steps), hazard edge, plot table
    for (z0, z1, r, slot) in ((0, 20, 540, DK), (20, 40, 480, GR)):
        pts = circle(cx, cy, r, 8, 22.5, 382.5)[:8]
        K[p].prism_xy(slot, pts, z0, z1)
        K["Collision"].prism_xy(CL, pts, z0, z1)
    K[p].annulus(HZ, cx, cy, 515, 540, 20, 20.6, 8, 22.5, 382.5)
    hexp = circle(cx, cy, 210, 6)
    cyl(p, ST, cx, cy, 40, 88, 150, 6)
    K[p].prism_xy(ST, hexp, 88, 110)
    K[p].prism_xy(HT, circle(cx, cy, 216, 6), 102, 108)
    K["Lights"].prism_xy(CO, circle(cx, cy, 188, 6), 110, 111.2)     # plot screen (flat LG blew out)
    # vector-grid "hologram" frame over the table (thin green struts)
    for k in range(-3, 4):
        box("Lights", LG, cx + k * 45 - 0.8, cx + k * 45 + 0.8, -150, 150, 150, 151.5)
        box("Lights", LG, cx - 150, cx + 150, k * 45 - 0.8, k * 45 + 0.8, 150, 151.5)
    for a in range(6):
        t = math.radians(a * 60 + 30)
        pipe(p, HT, (cx + 190 * math.cos(t), cy + 190 * math.sin(t), 110),
             (cx + 150 * math.cos(t), cy + 150 * math.sin(t), 152), 1.5, 6)
    cyl("Collision", CL, cx, cy, 40, 150, 225, 6)
    light("G", 6, cx, cy, 260)
    # overhead hood: octagonal light ring above the table
    K[p].annulus(ST, cx, cy, 470, 560, H_CIC - 90, H_CIC, 8, 22.5, 382.5)
    K["Lights"].annulus(LW, cx, cy, 480, 520, H_CIC - 92, H_CIC - 90, 8, 22.5, 382.5)
    for a in range(8):
        t = math.radians(a * 45 + 22.5)
        pipe(p, HT, (cx + 520 * math.cos(t), cy + 520 * math.sin(t), H_CIC - 90),
             (cx + 520 * math.cos(t), cy + 520 * math.sin(t), H_CIC), 6, 8)
    # captain's chair on the dais, looking forward
    chair(p, cx - 330, cy, 40, 180.0, big=True)
    box("Collision", CL, cx - 370, cx - 290, -40, 40, 40, 150)
    # side-wall stations (operators face the hull, backs to the dais)
    for xs in (3950.0, 4450.0, 4950.0):
        console(p, xs, 1300 - 15 - 72, -90.0, n_crt=3)
        console(p, xs, -1300 + 15 + 72, 90.0, n_crt=3, screen=LA if xs == 4450 else LG)
    # helm row under the bow windows: helm centre, nav + fire control
    console(p, 5500, 0, 180.0, n_crt=3)
    console(p, 5470, 430, 180.0, n_crt=2)
    console(p, 5470, -430, 180.0, n_crt=2, screen=LA)
    # big main displays above the slit windows (bow + chamfers)
    for (p0, p1) in (((5200, -1300), (X_CIC1, -700)), ((X_CIC1, -700), (X_CIC1, 700)), ((X_CIC1, 700), (5200, 1300))):
        a = Vector(p0); b = Vector(p1)
        e = (b - a).normalized(); n = Vector((-e.y, e.x))       # inward (CCW outline)
        L = (b - a).length
        for k in range(2 if L > 1000 else 1):
            f0 = (k + 0.15) / (2 if L > 1000 else 1)
            f1 = (k + 0.85) / (2 if L > 1000 else 1)
            q0 = a + e * (L * f0) + n * 22; q1 = a + e * (L * f1) + n * 22
            seg_box(p, HT, tuple(q0), tuple(q1), 14, 420, 600)
            q0b = q0 + n * 8; q1b = q1 + n * 8
            seg_box(p, CO, tuple(q0b + e * 10), tuple(q1b - e * 10), 3, 432, 588)
    # aft wall CRT banks either side of the door
    for s in (1, -1):
        for col_i in range(4):
            for row in range(2):
                y = s * (380 + col_i * 150)
                crt(p, X_BLK1 + 15 + 60, y, 150 + row * 110, 120, 95, 0.0,
                    screen=LG if (col_i + row) % 3 else LA, d=60)
        box(p, ST, X_BLK1 + 15, X_BLK1 + 80, s * 300, s * 1000, 0, 150)
        col(X_BLK1 + 15, X_BLK1 + 80, s * 300, s * 1000, 0, 150)
    # structural pillars with cable bundles + hazard feet
    for (px, py) in ((3950, 820), (3950, -820), (4950, 820), (4950, -820)):
        solid(p, BH, px - 40, px + 40, py - 40, py + 40, 0, H_CIC)
        box(p, HZ, px - 42, px + 42, py - 42, py + 42, 0, 60)
        for k in range(3):
            pipe(p, PI, (px + 44, py - 20 + k * 20, 0), (px + 44, py - 20 + k * 20, H_CIC), 5, 8)
    # ceiling beams + fixtures, red battle lamps
    for x in (3900.0, 4400.0, 4900.0, 5300.0):
        box(p, BH, x - 20, x + 20, -1285, 1285, H_CIC - 40, H_CIC)
    for (x, y) in ((3900, 700), (3900, -700), (4900, 700), (4900, -700), (5400, 400), (5400, -400)):
        ceiling_panel(p, x + 250, y, H_CIC - 40, 90.0, 160)
        light("W", 10, x + 250, y, H_CIC - 80)
    light("W", 9, 5450, 0, H_CIC - 80)
    for (x, y) in ((3700, 250), (3700, -250), (5700, 900), (5700, -900)):
        cage_lamp(p, x, y, H_CIC - 70, 0.0 if x < 4000 else 180.0)
    light("R", 6, 3750, 0, 400)
    # entry + seat
    SOCKETS["Entry"] = [5020.0, 0.0, 100.0, 180.0]
    SOCKETS["Seat"] = [5450.0, 0.0, 100.0, 0.0]


def build_quarters():
    p = "Quarters"
    # --- berths (port, x -2000..0, y 200..1400) ---
    x0, x1, y0, y1 = X_BLK0 + 15, -15.0, Y_SP + 15, Y_BLK - 15
    for k in range(8):
        xa = x0 + 20 + k * 238
        bunk_stack(p, xa, xa + 210, y1 - 92, y1 - 2)
    for k in range(6):
        xa = x0 + 260 + k * 238
        bunk_stack(p, xa, xa + 210, 700, 792)
        bunk_stack(p, xa, xa + 210, 792, 884)
    locker_bank(p, x0 + 10, -1140, y0, 55, 1)
    locker_bank(p, -860, x1 - 10, y0, 55, 1)
    for x in (-1700.0, -1000.0, -300.0):
        for y in (480.0, 1100.0):
            ceiling_panel(p, x, y, H_BLOCK, 0.0, 180)
    light("W", 9, -1500, 480, H_BLOCK - 40)
    light("W", 9, -500, 480, H_BLOCK - 40)
    light("W", 9, -1000, 1100, H_BLOCK - 40)
    cage_lamp(p, x1, 560, 300, 180.0)
    # footlockers at the island ends
    for x in (x0 + 180, x1 - 120):
        crate(p, x, 790, 0, 80, 50, 45, lid=HT)
    # --- drop-crew ready room (starboard, x -2000..0, y -1400..-200) ---
    y0, y1 = -Y_BLK + 15, -Y_SP - 15
    for k in range(10):
        xa = x0 + 30 + k * 186
        xm = xa + 80
        box(p, BH, xa, xa + 160, y0, y0 + 10, 0, 230)                               # back panel
        box(p, HT, xa, xa + 6, y0, y0 + 75, 0, 230)
        box(p, HT, xa + 154, xa + 160, y0, y0 + 75, 0, 230)
        box(p, HT, xa, xa + 160, y0, y0 + 75, 230, 238)
        box(p, ST, xa + 6, xa + 154, y0, y0 + 75, 0, 12)
        # armour suit on its hanger
        box(p, BH, xm - 26, xm + 26, y0 + 25, y0 + 58, 110, 172)                    # torso
        box(p, HT, xm - 20, xm + 20, y0 + 24, y0 + 60, 100, 112)                    # belt
        for s in (-1, 1):
            box(p, BH, xm + s * 26, xm + s * 42, y0 + 26, y0 + 56, 150, 176)       # pauldron
            box(p, HT, xm + s * 28, xm + s * 38, y0 + 32, y0 + 50, 95, 150)        # arm
            box(p, BH, xm + s * 6, xm + s * 22, y0 + 30, y0 + 52, 15, 100)         # leg
            box(p, HT, xm + s * 5, xm + s * 23, y0 + 28, y0 + 60, 0, 15)           # boot
        K[p].ico(BH, (xm, y0 + 42, 192), 17, 1)                                    # helmet
        box(p, VP, xm - 11, xm + 11, y0 + 57, y0 + 59, 186, 196)                   # visor
        box("Lights", LA, xm - 30, xm + 30, y0 + 70, y0 + 72, 224, 228)
    col(x0 + 30, x0 + 30 + 186 * 10, y0, y0 + 75, 0, 238)
    for xb in (-1500.0, -500.0):
        box(p, HT, xb - 20, xb + 20, -1150, -650, 38, 45)
        box(p, BH, xb - 18, xb + 18, -1148, -652, 45, 50)
        for yl in (-1120, -900, -680):
            box(p, HT, xb - 12, xb + 12, yl - 5, yl + 5, 0, 38)
        col(xb - 22, xb + 22, -1150, -650, 0, 60)
    # status board on the divider wall
    box(p, HT, x1 - 12, x1, -1100, -560, 130, 330)
    box(p, CO, x1 - 14, x1 - 12, -1085, -575, 145, 315)
    for x in (-1500.0, -500.0):
        ceiling_panel(p, x, -800, H_BLOCK, 0.0, 200)
        light("W", 9, x, -800, H_BLOCK - 40)
    cage_lamp(p, x0, -700, 300, 0.0)


def build_mess():
    p = "Mess"
    x0, x1, y0, y1 = 15.0, 1785.0, Y_SP + 15, Y_BLK - 15
    for xt in (300.0, 700.0, 1100.0, 1500.0):
        box(p, ME, xt - 50, xt + 50, 480, 920, 72, 78)
        box(p, HT, xt - 44, xt + 44, 486, 914, 66, 72)
        for yl in (500.0, 900.0):
            box(p, HT, xt - 6, xt + 6, yl - 6, yl + 6, 0, 66)
            box(p, HT, xt - 40, xt + 40, yl - 5, yl + 5, 0, 6)
        for s in (-1, 1):
            box(p, ME, xt + s * 70, xt + s * 100, 480, 920, 42, 46)
            box(p, HT, xt + s * 80, xt + s * 90, 490, 910, 0, 42)
        # trays, mugs
        for k in range(5):
            yy = 520 + k * 90
            box(p, GA, xt - 40, xt - 8, yy, yy + 24, 78, 80)
            cyl(p, ST, xt + 22, yy + 12, 78, 90, 4, 8)
        col(xt - 102, xt + 102, 478, 922, 0, 90)
    # serving line with sneeze guard
    box(p, GA, 400, 1400, 1100, 1165, 0, 92)
    box(p, ST, 400, 1400, 1095, 1170, 92, 96)
    box(p, VP, 420, 1380, 1128, 1132, 110, 150)
    for x in (420.0, 900.0, 1380.0):
        pipe(p, HT, (x, 1130, 96), (x, 1130, 152), 1.5, 6)
    for k in range(6):
        box(p, HT, 460 + k * 150, 580 + k * 150, 1112, 1152, 88, 93)
    col(400, 1400, 1095, 1170, 0, 110)
    # galley back counter, hood, ovens, fridge, urns
    box(p, GA, 150, 1650, y1 - 75, y1, 0, 90)
    box(p, ST, 150, 1650, y1 - 80, y1, 90, 94)
    extrude(p, BH, [(y1 - 90, 250), (y1, 250), (y1, 300), (y1 - 60, 300)], 'x', 300, 1500)
    box("Lights", LA, 320, 1480, y1 - 88, y1 - 60, 247, 249)
    for k in range(4):
        xo = 350 + k * 180
        box(p, HT, xo, xo + 150, y1 - 78, y1 - 76, 12, 80)
        box(p, VP, xo + 20, xo + 130, y1 - 79, y1 - 78, 35, 70)
    box(p, GA, 1620, 1760, y1 - 85, y1, 0, 230)
    box(p, HT, 1689, 1691, y1 - 86, y1 - 85, 10, 225)
    for x in (1100.0, 1200.0, 1300.0):
        cyl(p, ST, x, y1 - 40, 94, 160, 18, 12)
        cyl(p, HT, x, y1 - 40, 160, 166, 19, 12)
    col(150, 1760, y1 - 90, y1, 0, 230)
    for x in (400.0, 1100.0, 1600.0):
        ceiling_panel(p, x, 700, H_BLOCK, 90.0, 220)
    light("W", 9, 400, 700, H_BLOCK - 40)
    light("W", 9, 1400, 700, H_BLOCK - 40)
    light("A", 6, 900, 1200, 260)


def build_medbay():
    p = "Medbay"
    x0, x1, y0, y1 = 15.0, 1785.0, -Y_BLK + 15, -Y_SP - 15
    cx, cy = 1250.0, -820.0
    # cryo ring: 8 hypersleep pods around the control column
    for k in range(8):
        a = 22.5 + 45 * k
        M = Mx(cx, cy, 0, a)
        r0, r1 = 150.0, 370.0
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
        c0 = M @ Vector((r0, 20, 30)); c1 = M @ Vector((100, 20, 30))
        pipe(p, PI, c0, c1, 4, 8)
        box("Collision", CL, r0 - 4, r1 + 6, -50, 50, 0, 110, M)
    cyl(p, ST, cx, cy, 0, H_BLOCK, 95, 12)
    for z in (80.0, 180.0, 280.0):
        cyl("Lights", LB, cx, cy, z, z + 12, 97, 12)
    cyl("Collision", CL, cx, cy, 0, H_BLOCK, 110, 12)
    light("B", 7, cx, cy, 330)
    # med bays against the outer wall
    for mx in (200.0, 520.0):
        box(p, HT, mx - 40, mx + 40, y0 + 40, y0 + 250, 0, 70)
        box(p, GA, mx - 44, mx + 44, y0 + 36, y0 + 254, 70, 84)
        box(p, BU, mx - 38, mx + 38, y0 + 200, y0 + 246, 84, 94)
        cyl(p, HT, mx, y0 + 145, H_BLOCK - 70, H_BLOCK, 5, 8)
        cyl(p, ST, mx, y0 + 145, H_BLOCK - 90, H_BLOCK - 70, 55, 16)
        cyl("Lights", LW, mx, y0 + 145, H_BLOCK - 92, H_BLOCK - 90, 45, 16)
        crt(p, mx + 75, y0 + 45, 120, 50, 40, 90.0, screen=LG, d=40)
        box(p, HT, mx + 50, mx + 100, y0 + 10, y0 + 40, 0, 120)
        col(mx - 46, mx + 46, y0, y0 + 256, 0, 100)
        light("W", 5, mx, y0 + 145, H_BLOCK - 110)
    # instrument cabinets on the divider wall (x = 0)
    for k in range(5):
        ya = -1300 + k * 170
        box(p, GA, x0, x0 + 55, ya, ya + 160, 0, 90)
        box(p, GA, x0, x0 + 35, ya, ya + 160, 150, 240)
        box(p, VP, x0 + 35, x0 + 36, ya + 10, ya + 150, 160, 230)
    col(x0, x0 + 55, -1300, -450, 0, 100)
    box(p, HZ, 900, 1600, y1 - 4, y1, 30, 60)    # stencil band
    for (x, y) in ((400.0, -500.0), (1250.0, -420.0), (1650.0, -1150.0)):
        ceiling_panel(p, x, y, H_BLOCK, 0.0, 180)
    light("W", 8, 400, -500, H_BLOCK - 40)
    light("W", 8, 1600, -1150, H_BLOCK - 40)


def build_briefing():
    p = "Briefing"
    x0, x1, y0, y1 = 1815.0, X_BLK1 - 15, Y_SP + 15, Y_BLK - 15
    # CRT video wall on the forward bulkhead, 4 x 3
    yc = 800.0
    box(p, HT, x1 - 30, x1, yc - 350, yc + 350, 40, 415)
    for i in range(4):
        for j in range(3):
            y = yc - 240 + i * 160
            crt(p, x1 - 30, y, 55 + j * 118, 150, 112, 180.0, screen=CO if (i + j) % 2 else LG, d=10)
    col(x1 - 32, x1, yc - 352, yc + 352, 0, 415)
    # lectern + tactical board
    M = Mx(3280, 420, 0, 180.0)
    extrude(p, BH, [(-30, 0), (30, 0), (24, 110), (-36, 118)], 'y', -35, 35, M)
    box("Lights", LA, 3300 - 26, 3300 - 20, 395, 445, 112, 116)
    col(3245, 3315, 380, 460, 0, 120)
    box(p, HT, 1900, 2800, y1 - 12, y1, 120, 340)
    box(p, CO, 1915, 2785, y1 - 14, y1 - 12, 135, 325)
    # seat rows facing the video wall
    for xr in (2250.0, 2450.0, 2650.0, 2850.0, 3050.0):
        for block in ((380.0, 680.0), (900.0, 1200.0)):
            for k in range(4):
                y = block[0] + k * 100
                chair(p, xr, y, 0, 180.0)
            box(p, HT, xr + 20, xr + 30, block[0] - 40, block[1] + 40, 0, 6)
            col(xr - 35, xr + 35, block[0] - 40, block[1] + 40, 0, 100)
    box(p, ST, 2600, 2700, 750, 850, H_BLOCK - 40, H_BLOCK)                          # ceiling projector
    box("Lights", LW, 2698, 2700, 780, 820, H_BLOCK - 32, H_BLOCK - 10)
    for x in (2200.0, 2800.0, 3350.0):
        ceiling_panel(p, x, 500, H_BLOCK, 90.0, 180)
        ceiling_panel(p, x, 1100, H_BLOCK, 90.0, 180)
    light("W", 9, 2300, 800, H_BLOCK - 40)
    light("W", 9, 3200, 800, H_BLOCK - 40)
    light("G", 5, x1 - 200, yc, 250)


def build_armory():
    p = "Armory"
    x0, x1, y0, y1 = 1815.0, X_BLK1 - 15, -Y_BLK + 15, -Y_SP - 15
    xc = 2400.0
    # cage partition: bars with a gate and a counter window
    gate = (-700.0, -560.0)
    win = (-460.0, -250.0)
    y = y0
    while y < y1:
        in_gate = gate[0] < y < gate[1]
        in_win = win[0] < y < win[1]
        if not in_gate:
            zb = 150.0 if in_win else 0.0
            if in_win:
                pipe(p, HT, (xc, y, 205), (xc, y, 260), 1.6, 6)
            else:
                pipe(p, HT, (xc, y, zb), (xc, y, 260), 1.6, 6)
        y += 14
    for z in (8.0, 100.0, 258.0):
        pipe(p, BH, (xc, y0, z), (xc, gate[0], z), 3, 8)
        pipe(p, BH, (xc, gate[1], z), (xc, y1, z), 3, 8)
    box(p, ST, xc - 40, xc + 40, win[0], win[1], 95, 100)                               # counter
    box(p, HT, xc - 35, xc + 35, win[0] + 5, win[1] - 5, 0, 95)
    box(p, BH, xc - 20, xc + 20, win[0], win[1], 200, 206)
    for s in (0, 1):
        g = gate[s]
        box(p, HZ, xc - 6, xc + 6, g - 6, g + 6, 0, 260)
    col(xc - 8, xc + 8, y0, gate[0], 0, 260)
    col(xc - 40, xc + 40, gate[1], y1, 0, 260)
    # rifle racks: outer wall and forward wall
    box(p, BH, 2500, 3500, y0, y0 + 16, 70, 230)
    box(p, HT, 2500, 3500, y0, y0 + 30, 70, 84)
    x = 2530.0
    while x < 3480:
        rifle(p, x, y0 + 24, 90.0)
        x += 32
    col(2500, 3500, y0, y0 + 40, 0, 230)
    box(p, BH, x1 - 16, x1, -1300, -420, 70, 230)
    box(p, HT, x1 - 30, x1, -1300, -420, 70, 84)
    y = -1270.0
    while y < -440:
        rifle(p, x1 - 24, y, 180.0)
        y += 32
    col(x1 - 40, x1, -1300, -420, 0, 230)
    # smart-gun harness on a stand
    sx, sy = 3000.0, -800.0
    cyl(p, HT, sx, sy, 0, 8, 45, 12)
    cyl(p, HT, sx, sy, 8, 120, 6, 8)
    box(p, BH, sx - 25, sx + 25, sy - 15, sy + 15, 120, 180)
    pipe(p, HT, (sx + 25, sy - 30, 160), (sx + 95, sy - 30, 162), 6, 10)
    box(p, HT, sx + 20, sx + 60, sy - 42, sy - 18, 145, 172)
    pipe(p, HT, (sx - 15, sy, 180), (sx + 25, sy - 30, 165), 3, 6)
    cyl("Collision", CL, sx, sy, 0, 190, 60, 8)
    # ammo crate stacks
    for (cx, cy, n) in ((3300.0, -520.0, 3), (2700.0, -520.0, 2), (3350.0, -760.0, 2)):
        for k in range(n):
            crate(p, cx, cy, k * 52, 110, 70, 50, yaw=k * 7.0, collide=(k == 0))
        col(cx - 65, cx + 65, cy - 45, cy + 45, 0, n * 52)
    # workbench + vise under the spine wall
    box(p, ST, 2550, 3150, y1 - 80, y1, 88, 94)
    for xl in (2560.0, 3140.0):
        box(p, HT, xl - 5, xl + 5, y1 - 75, y1 - 5, 0, 88)
    box(p, HT, 2700, 2740, y1 - 70, y1 - 40, 94, 118)
    box(p, BH, 2550, 3150, y1 - 6, y1, 120, 260)
    for k in range(10):
        box(p, HT, 2580 + k * 55, 2584 + k * 55, y1 - 10, y1 - 6, 150, 200 + (k % 3) * 15)
    col(2550, 3150, y1 - 80, y1, 0, 100)
    for (x, y) in ((2100.0, -800.0), (2950.0, -600.0), (2950.0, -1150.0)):
        ceiling_panel(p, x, y, H_BLOCK, 0.0, 180)
    light("W", 8, 2100, -800, H_BLOCK - 40)
    light("W", 9, 2950, -850, H_BLOCK - 40)
    cage_lamp(p, 3000, y0, 330, 90.0)
    light("R", 5, 3000, y0 + 60, 330)


def build_hangar():
    p = "Hangar"
    x0, x1, y0, y1 = X_HGR0 + 15, X_HGR1 - 15, -Y_HGR + 15, Y_HGR - 15
    # --- wall ribs + ceiling trusses ---
    ribs(p, (X_HGR0, Y_HGR), (X_HGR1, Y_HGR), -1, H_BAY, step=400)
    ribs(p, (X_HGR0, -Y_HGR), (X_HGR1, -Y_HGR), 1, H_BAY, step=400, skip=[(440, 1960)])
    ribs(p, (X_HGR0, -Y_HGR), (X_HGR0, Y_HGR), -1, H_BAY, step=450, skip=[(1450, 1950)])
    for y in range(-1500, 1600, 500):
        box(p, BH, x0, x1, y - 18, y + 18, H_BAY - 70, H_BAY)
        box(p, HT, x0, x1, y - 30, y + 30, H_BAY - 74, H_BAY - 70)
    for x in (-4000.0, -3600.0, -3200.0, -2800.0, -2400.0):
        box(p, BH, x - 12, x + 12, y0, y1, H_BAY - 45, H_BAY)
    # --- floor markings: landing pad, launch lane, tie-downs ---
    pcx, pcy = -3200.0, 150.0
    K[p].annulus(HZ, pcx, pcy, 800, 845, 0, 0.6, 48)
    K[p].annulus(HZ, pcx, pcy, 240, 270, 0, 0.6, 32)
    for s in (-1, 1):
        box(p, HZ, pcx + s * 530 - 20, pcx + s * 530 + 20, y0, -650, 0, 0.6)
    for gx in range(-4100, -2300, 300):
        for gy in range(-1500, 1600, 300):
            cyl(p, HT, gx, gy, 0, 1.5, 9, 8)
    # --- launch door (starboard wall) ---
    dx0, dx1 = -3900.0, -2500.0
    ydw = -Y_HGR
    for s in (-1, 1):
        xa, xb = (dx0, -3202.0) if s < 0 else (-3198.0, dx1)
        box(p, HT, xa, xb, ydw - 12, ydw + 20, 0, 900)
        col(xa, xb, ydw - 12, ydw + 20, 0, 900)
        for z in range(60, 900, 110):
            box(p, BH, xa + 20, xb - 20, ydw + 20, ydw + 34, z, z + 40)
        # chevrons at the meeting edge
        xe = -3202.0 if s < 0 else -3198.0
        for z in range(0, 900, 90):
            box(p, HZ, xe - s * 70, xe, ydw + 20, ydw + 24, z, z + 45)
    box(p, BH, dx0 - 60, dx1 + 60, ydw - 12, ydw + 46, 900, 980)
    for s in (-1, 1):
        xj = dx0 - 30 if s < 0 else dx1 + 30
        box(p, BH, xj - 30, xj + 30, ydw - 12, ydw + 46, 0, 980)
        box(p, HZ, xj - 31, xj + 31, ydw + 46, ydw + 48, 0, 900)
        cyl(p, HT, xj, ydw + 70, 990, 1010, 26, 12)
        cyl("Lights", LR, xj, ydw + 70, 1010, 1045, 20, 12)
        light("R", 7, xj, ydw + 150, 1000)
    # --- dropship on the pad, nose to the launch door ---
    build_dropship(pcx, pcy)
    # --- catwalk along the forward wall at z 600, stairs down the port side ---
    cz = 600.0
    cx0 = X_HGR1 - 15 - 285
    solid(p, GR, cx0, X_HGR1 - 15, -1500, y1, cz - 10, cz)
    box(p, HT, cx0, X_HGR1 - 15, -1500, y1, cz - 40, cz - 10)
    for y in range(-1500, 1400, 400):
        pipe(p, HT, (cx0 + 20, y, 0), (cx0 + 20, y, cz - 40), 9, 10)
        cyl(p, HT, cx0 + 20, y, 0, 6, 28, 10)
        pipe(p, HT, (cx0 + 20, y, cz - 280), (X_HGR1 - 15, y, cz - 40), 5, 8)
        cyl("Collision", CL, cx0 + 20, y, 0, cz - 40, 14, 8)
    railing(p, (cx0, -1500), (cx0, 1450), cz)
    railing(p, (cx0, -1500), (X_HGR1 - 15, -1500), cz)
    # stairs: from floor at x -3500 up to the catwalk at x = cx0, along the port wall
    run = cx0 - (-3500.0)
    stairs(p, -3500.0, (1450 + y1) / 2, run, y1 - 1450, 0, cz, 0.0, rails=(True, False))
    box(p, HZ, -3560, -3500, 1450, y1, 0, 0.6)
    # crane-control station on the catwalk, looking out over the bay
    console(p, cx0 + 72, -900, 0.0, n_crt=2, z=cz)
    # flight-control booth: two consoles at the window over the hangar
    for y in (470.0, 670.0):
        console(p, X_HGR1 + 15 + 70, y, 0.0, n_crt=2, z=600.0)
    light("W", 6, -1750, 450, 900)
    # --- gantry crane ---
    for xr in (-4250.0, -2450.0):
        box(p, BH, xr - 25, xr + 25, y0, y1, 1130, 1150)
        box(p, BH, xr - 6, xr + 6, y0, y1, 1150, 1190)
        box(p, BH, xr - 25, xr + 25, y0, y1, 1190, 1205)
    yb = -300.0
    box(p, HZ, -4250, -2450, yb - 40, yb + 40, 1205, 1255)
    box(p, BH, -4250, -2450, yb - 45, yb + 45, 1195, 1205)
    box(p, ST, -3450, -3250, yb - 70, yb + 70, 1100, 1195)
    pipe(p, HT, (-3350, yb, 1100), (-3350, yb, 760), 2.5, 6)
    box(p, HZ, -3380, -3320, yb - 30, yb + 30, 700, 760)
    # --- ordnance, crates, tug, fuel cart ---
    for k in range(3):
        crate(p, x0 + 120, 700 + k * 260, 0, 200, 200, 160, yaw=0.0)
        crate(p, x0 + 120, 700 + k * 260, 160, 180, 180, 120, yaw=4.0, collide=False)
    col(x0 + 10, x0 + 230, 590, 1330, 0, 280)
    for k in range(2):
        mx, my = -3850.0 + k * 350, 1250.0
        box(p, HT, mx - 150, mx + 150, my - 50, my + 50, 20, 40)
        for s in (-1, 1):
            pipe(p, HT, (mx + s * 120, my - 55, 12), (mx + s * 120, my + 55, 12), 12, 10)
        for j in range(3):
            yy = my - 30 + j * 30
            pipe(p, BH, (mx - 140, yy, 60), (mx + 120, yy, 60), 12, 10)
            pipe(p, HZ, (mx + 120, yy, 60), (mx + 150, yy, 60), 8, 10)
            box(p, HT, mx - 150, mx - 120, yy - 18, yy + 18, 45, 75)
        col(mx - 155, mx + 155, my - 60, my + 60, 0, 90)
    # tow tug
    tx, ty = -2600.0, -1250.0
    box(p, HZ, tx - 200, tx + 200, ty - 110, ty + 110, 30, 110)
    box(p, BH, tx - 180, tx - 40, ty - 100, ty + 100, 110, 200)
    box(p, VP, tx - 42, tx - 38, ty - 90, ty + 90, 130, 190)
    for sx in (-130, 130):
        for sy in (-1, 1):
            pipe(p, HT, (tx + sx, ty + sy * 100, 32), (tx + sx, ty + sy * 125, 32), 32, 12)
    col(tx - 205, tx + 205, ty - 130, ty + 130, 0, 200)
    # fuel cart + hoses
    fx, fy = -4050.0, -600.0
    box(p, HZ, fx - 90, fx + 90, fy - 60, fy + 60, 20, 40)
    cyl(p, ST, fx - 40, fy, 40, 180, 45, 14)
    cyl(p, ST, fx + 50, fy, 40, 180, 45, 14)
    pipe(p, PI, (fx + 90, fy, 150), (pcx - 500, pcy - 200, 5), 5, 8)
    col(fx - 95, fx + 95, fy - 65, fy + 65, 0, 180)
    # wall pipes, bulkhead lockers along the port wall
    for zz, r in ((980.0, 22.0), (1040.0, 14.0)):
        pipe(p, PI, (x0, y1 - 40, zz), (x1 - 300, y1 - 40, zz), r, 12)
    locker_bank(p, -4100, -3600, y1, 60, -1)
    # floodlights
    for (x, y) in ((-3900, -1000), (-2700, -1000), (-3900, 300), (-2700, 300), (-3900, 1300), (-2800, 1300)):
        flood(p, x, y, H_BAY - 74)
        shadow = (x, y) in ((-3900, 300), (-2700, 300))
        light("A", 16, x, y, H_BAY - 200, shadow=shadow)
    light("W", 6, cx0 + 140, 0, cz + 280)
    for y in (-1200.0, 0.0, 1100.0):
        cage_lamp(p, cx0 + 10, y, cz + 220, 180.0, color=LA)


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
    x0, x1, y0, y1 = X_ENG0 + 15, X_ENG1 - 15, -Y_BLK + 15, Y_BLK - 15
    rx, ry = -5100.0, 0.0
    # floor: grating everywhere, hazard walkway around the plinth
    box(p, GR, x0, x1, y0, y1, 0, 0.5)
    K[p].annulus(HZ, rx, ry, 390, 445, 0, 0.8, 32)
    # reactor: plinth, core housing, glowing bands, collars, conduits
    cyl(p, ST, rx, ry, 0, 90, 390, 32)
    cyl(p, HT, rx, ry, 90, H_BAY, 240, 32)
    for (za, zb) in ((200, 270), (660, 740), (930, 1000), (1150, 1200)):
        cyl("Lights", LA, rx, ry, za, zb, 243, 32)
        K[p].annulus(BH, rx, ry, 238, 275, za - 20, za, 32)
        K[p].annulus(BH, rx, ry, 238, 275, zb, zb + 20, 32)
    for k in range(8):
        t = math.radians(k * 45 + 22.5)
        pipe(p, PI, (rx + 290 * math.cos(t), ry + 290 * math.sin(t), 90),
             (rx + 290 * math.cos(t), ry + 290 * math.sin(t), H_BAY), 14, 10)
    cyl("Collision", CL, rx, ry, 0, H_BAY, 395, 32)
    # catwalk ring at 500 + gallery along the aft wall + bridge between them
    rz = 500.0
    K[p].annulus(GR, rx, ry, 330, 560, rz - 8, rz, 48)
    K[p].annulus(HT, rx, ry, 545, 560, rz - 30, rz - 8, 48)
    K["Collision"].annulus(CL, rx, ry, 300, 560, rz - 20, rz, 48)
    K["Collision"].annulus(CL, rx, ry, 300, 345, rz, rz + 130, 48)                        # inner rail
    gap = math.degrees(math.asin(115.0 / 556.0))
    K["Collision"].annulus(CL, rx, ry, 548, 560, rz, rz + 130, 48, -180 + gap, 180 - gap)
    for r in (340.0, 552.0):
        full = r < 400
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
        q = (rx + 540 * math.cos(t), ry + 540 * math.sin(t))
        pipe(p, HT, (q[0], q[1], 0), (q[0], q[1], rz - 30), 12, 10)
        cyl("Collision", CL, q[0], q[1], 0, rz - 30, 16, 8)
    gx1 = -5700.0
    solid(p, GR, x0, gx1, y0, y1, rz - 10, rz)
    box(p, HT, x0, gx1, y0, y1, rz - 40, rz - 10)
    solid(p, GR, gx1 - 20, rx - 500, -115, 115, rz - 10, rz)
    railing(p, (gx1, -115), (rx - 540, -115), rz)
    railing(p, (gx1, 115), (rx - 540, 115), rz)
    railing(p, (gx1, y0), (gx1, -115), rz)
    railing(p, (gx1, 115), (gx1, 1150), rz)
    for y in range(-1200, 1300, 400):
        pipe(p, HT, (gx1 - 20, y, 0), (gx1 - 20, y, rz - 40), 10, 10)
        cyl("Collision", CL, gx1 - 20, y, 0, rz - 40, 14, 8)
    # ramp from the floor up the port wall to the gallery
    stairs(p, -4600.0, (1150 + y1) / 2, 1100.0, y1 - 1150, 0, rz, 180.0, rails=(False, True))
    box(p, HZ, -4600, -4540, 1150, y1, 0, 0.8)
    # gallery consoles facing the aft bulkhead
    for y in (-700.0, 0.0, 700.0):
        Mg = Mx(x0 + 72, y, rz, 0.0)
        w = 240.0
        extrude(p, ST, [(-70, 0), (0, 0), (0, 70), (-6, 76), (-44, 104), (-70, 106)], 'y', -w / 2, w / 2, Mg)
        for k in range(3):
            c = Mg @ Vector((-40, -w / 2 + 45 + k * 75, 106))
            crt(p, c.x, c.y, c.z, 56, 44, 0.0, screen=LA if k == 1 else LG, d=38)
        box("Collision", CL, -72, 2, -w / 2, w / 2, 0, 120, Mg)
    box(p, HT, x0, x0 + 20, -1000, 1000, rz + 140, rz + 330)                         # wall schematic
    box(p, CO, x0 + 20, x0 + 22, -980, 980, rz + 155, rz + 315)
    # floor consoles by the hangar door
    console(p, x1 - 72, 800, 180.0, n_crt=2)
    console(p, x1 - 72, -800, 180.0, n_crt=2, screen=LA)
    # coolant tanks + feed pipes to the reactor
    for tx in (-5450.0, -5050.0, -4650.0):
        ty = y0 + 170
        cyl(p, ST, tx, ty, 0, 700, 145, 24)
        cyl(p, HT, tx, ty, 700, 740, 120, 24)
        cyl(p, HT, tx, ty, 0, 30, 155, 24)
        for zb in (150.0, 400.0, 650.0):
            K[p].annulus(BH, tx, ty, 143, 152, zb, zb + 14, 24)
        box(p, HZ, tx - 40, tx + 40, ty + 144, ty + 147, 300, 330)
        cyl("Collision", CL, tx, ty, 0, 740, 155, 12)
        q = Vector((tx, ty + 145, 320)); r_ = Vector((rx, ry, 320))
        d = (r_ - q); d.z = 0
        end = r_ - d.normalized() * 250
        pipe(p, PI, q, end, 18, 12)
        pipe(p, HT, end, (end.x, end.y, 250), 12, 10)
    # overhead coolant headers to the walls
    for (ex, ey) in ((x0, 0.0), (rx, y1), (rx, y0), (x1, 0.0)):
        q = Vector((ex, ey, 1150)); r_ = Vector((rx, ry, 1150))
        d = (q - r_); d.z = 0
        pipe(p, PI, r_ + d.normalized() * 240, q, 38, 14)
    for y in (-900.0, -300.0, 300.0, 900.0):
        pipe(p, HT, (x1, y, 1240), (x0, y, 1240), 10, 8)
    ribs(p, (X_ENG0, Y_BLK), (X_ENG1, Y_BLK), -1, H_BAY, step=400, slot=BH)
    ribs(p, (X_ENG1, -Y_BLK), (X_ENG0, -Y_BLK), -1, H_BAY, step=400, slot=BH)
    # lights: amber around the core, work lights on the gallery, blue floor pool
    for a in (0.0, 90.0, 180.0, 270.0):
        t = math.radians(a + 45)
        light("A", 12, rx + 700 * math.cos(t), ry + 700 * math.sin(t), 380, shadow=(a == 0.0))
    for y in (-900.0, 900.0):
        flood(p, x0 + 150, y, H_BAY)
        light("W", 8, x0 + 150, y, 900)
    for y in (-1000.0, 1000.0):
        flood(p, x1 - 200, y, H_BAY, color=LW)
        light("W", 12, x1 - 200, y, 1000)
    for (x, y, yaw) in ((x1, 600, 180.0), (x1, -600, 180.0), (x0, 1200, 0.0), (x0, -1200, 0.0)):
        cage_lamp(p, x, y, 300, yaw)


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
    return {
        "prefix": PREFIX,
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
    print("CONTRACT", json.dumps({k: v for k, v in info.items() if k != "sockets_ue_design_cm"}, indent=1))
    for p in PARTS:
        print("  export", export(objs[p], dry), info["tris"][p], "tris")
    if not dry:
        with open(os.path.join(GEN, PREFIX + "_contract.json"), "w") as f:
            json.dump(info, f, indent=1)
    if info["problems"]:
        print("CONTRACT_PROBLEMS", len(info["problems"]), info["problems"][:10])


if __name__ == "__main__":
    main()
