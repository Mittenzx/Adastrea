"""Exterior props for ship hulls and station modules (Blender 5.x).

Same drawing API as the interior library (Tools/deck_props.py: PT frames, the
M_Prop_* palette in Tools/prop_materials.py), exported 1:1 as SM_ExtProp_<Name>
by Tools/build_interior_props.py and placed at runtime by
UExteriorDressingComponent (hull-traced mount points), or by hand in any BP/level.

Mount convention: z=0 is the hull surface and +Z points away from it (the
component aligns +Z with the surface normal). +X is "forward" for directional
props (floodlights, dishes, turrets aim along +X).

Light lenses use M_Prop_Beacon, a flat emissive whose Color/Intensity the
component drives per light (nav red/green/white, strobes, anti-collision
beacons). Parts that spin (radar head, beacon rotor) are separate meshes named
<Name>Head so the component can rotate them.
"""
import math

import deck_kit as dk
from deck_props import (RED, WHT, BLK, STE, ALU, YEL, OLV, ORG, SCR, GLS, COP, PT)  # noqa: F401
from deck_kit import LR, LG, LA, LB, LC

BEACON = "M_Prop_Beacon"
HULL = "M_Prop_Hull"              # light hull grey plate


# -----------------------------------------------------------------------------
# Lights
# -----------------------------------------------------------------------------
def nav_light(t):
    """Wingtip navigation light pod: faired housing + lens (colour set at runtime)."""
    t.e(HULL, [(-18, 0), (14, 0), (18, 6), (10, 12), (-18, 10)], 'y', -8, 8)
    t.sph(BEACON, (12, 0, 9), 6.5, 1)
    t.b(BLK, -6, 4, -8.5, 8.5, 2, 10)


def strobe(t):
    """Anti-collision strobe: low dome on a round base."""
    t.c(STE, 0, 0, 0, 3, 9, 16)
    t.c(BLK, 0, 0, 3, 5, 7, 16)
    t.c(BEACON, 0, 0, 5, 9, 5.5, 16)
    t.sph(GLS, (0, 0, 9), 5.5, 1)


def beacon(t):
    """Rotating anti-collision beacon (static base; BeaconHead spins)."""
    t.c(STE, 0, 0, 0, 6, 16, 16)
    t.c(BLK, 0, 0, 6, 10, 12, 16)


def beacon_head(t):
    t.c(BEACON, 0, 0, 10, 26, 10, 16)
    t.b(BLK, -11, 11, -1.5, 1.5, 10, 26)          # rotating shutter makes the sweep read
    t.c(STE, 0, 0, 26, 29, 11, 16)


def floodlight(t):
    """Hull floodlight on a yoke, aimed along +X (tilted 20 deg down-range)."""
    t.c(STE, 0, 0, 0, 6, 14, 12)
    for s in (-1, 1):
        t.b(STE, -3, 3, s * 16, s * 19, 6, 34)
    import deck_kit as _dk
    M = t.M @ _dk.Mx(0, 0, 30) @ __import__("mathutils").Matrix.Rotation(math.radians(-20), 4, 'Y')
    sub = PT(t.part, lights=t.lights)
    sub.M = M
    sub.b(BLK, -14, 10, -15, 15, -12, 12)
    sub.b(BEACON, 10, 11, -12, 12, -9, 9)
    sub.b(STE, 11, 13, -15, 15, -12, -10)
    sub.b(STE, 11, 13, -15, 15, 10, 12)


def docking_guide(t):
    """Docking guide chevron bar: five lit chevrons pointing +X."""
    t.b(STE, -60, 60, -14, 14, 0, 4)
    for k in range(5):
        x = -48 + k * 24
        t.e(BEACON, [(x - 8, -10), (x + 2, 0), (x - 8, 10), (x - 4, 10), (x + 6, 0), (x - 4, -10)], 'z', 4, 5.5)


def spotlight_tower(t):
    """Station floodlight mast: lattice post with three lamp heads."""
    for (x, y) in ((-10, -10), (10, -10), (10, 10), (-10, 10)):
        t.p(STE, (x, y, 0), (x * 0.4, y * 0.4, 400), 2.2)
    for z in range(40, 400, 60):
        pts = [(-10 + z * 0.015, -10 + z * 0.015), (10 - z * 0.015, -10 + z * 0.015),
               (10 - z * 0.015, 10 - z * 0.015), (-10 + z * 0.015, 10 - z * 0.015)]
        for i in range(4):
            a, b = pts[i], pts[(i + 1) % 4]
            t.p(STE, (a[0], a[1], z), (b[0], b[1], z), 1.2, 6)
    for k in range(3):
        a = math.radians(k * 120)
        c = (18 * math.cos(a), 18 * math.sin(a), 400)
        t.p(STE, (0, 0, 400), c, 2)
        t.c(BLK, c[0], c[1], 392, 408, 12, 12)
        t.c(BEACON, c[0], c[1], 391, 392, 10, 12)
    t.c(STE, 0, 0, 0, 5, 26, 12)


# -----------------------------------------------------------------------------
# Antennas / sensors
# -----------------------------------------------------------------------------
def antenna_mast(t, h=180.0):
    """Tapered comms mast with cross-yards and a tip light."""
    t.c(STE, 0, 0, 0, 8, 14, 12)
    t.p(ALU, (0, 0, 8), (0, 0, h), 3.2, 8)
    for k, z in enumerate((h * 0.45, h * 0.7, h * 0.88)):
        w = 40 - k * 10
        t.p(ALU, (0, -w, z), (0, w, z), 1.4, 6)
        for s in (-1, 1):
            t.p(ALU, (0, s * w, z), (0, s * w, z - 18 + k * 4), 0.9, 6)
    t.sph(BEACON, (0, 0, h + 4), 3.5, 1)


def whip_antenna(t):
    t.c(BLK, 0, 0, 0, 6, 5, 10)
    t.p(BLK, (0, 0, 6), (0, 0, 14), 2.2, 8)
    t.p(STE, (0, 0, 14), (6, 0, 160), 0.8, 6)


def sensor_dish(t):
    """Parabolic dish on a pedestal, facing +X and tilted up 30 deg."""
    from mathutils import Matrix
    t.c(STE, 0, 0, 0, 8, 22, 16)
    t.c(OLV, 0, 0, 8, 50, 7, 12)
    M = t.M @ dk.Mx(0, 0, 60) @ Matrix.Rotation(math.radians(-60), 4, 'Y')
    sub = PT(t.part, lights=t.lights)
    sub.M = M
    _dish(sub, 55.0, 18.0)


def _dish(sub, R, depth):
    """Paraboloid shell in rings (axis +Z), feed horn on struts."""
    rings = 6
    for i in range(rings):
        r0, r1 = R * i / rings, R * (i + 1) / rings
        z0 = depth * (r0 / R) ** 2
        z1 = depth * (r1 / R) ** 2
        sub.ring(WHT, 0, 0, max(0.1, r0), r1, z0, z0 + max(2.0, z1 - z0 + 1.5), 24)
    for a in range(3):
        ang = math.radians(a * 120)
        sub.p(STE, (R * 0.9 * math.cos(ang), R * 0.9 * math.sin(ang), depth), (0, 0, R * 0.8), 1.0, 6)
    sub.c(BLK, 0, 0, R * 0.8 - 6, R * 0.8 + 6, 5, 10)
    sub.c(STE, 0, 0, -8, 0, 14, 12)


def radar_base(t):
    """Rotating surveillance radar: pedestal (RadarHead spins on top)."""
    t.c(STE, 0, 0, 0, 10, 26, 16)
    t.c(OLV, 0, 0, 10, 40, 12, 12)
    t.c(BLK, 0, 0, 40, 46, 16, 12)


def radar_head(t):
    """Wide slotted-array radar bar (spins about Z)."""
    t.c(STE, 0, 0, 46, 54, 10, 12)
    t.e(WHT, [(-6, 50), (6, 50), (8, 74), (-8, 74)], 'y', -90, 90)
    for k in range(-8, 9):
        t.b(BLK, 8, 8.6, k * 10 - 3, k * 10 + 3, 56, 70)
    t.b(STE, -10, 0, -12, 12, 52, 62)


def comm_array(t):
    """Phased-array panel pair on a boom."""
    t.c(STE, 0, 0, 0, 10, 18, 12)
    t.p(STE, (0, 0, 10), (0, 0, 90), 4, 8)
    for s in (-1, 1):
        t.p(STE, (0, 0, 80), (0, s * 40, 80), 2.5, 6)
        t.e(BLK, [(-2, 60), (2, 60), (6, 120), (2, 120)], 'y', s * 40 - 30, s * 40 + 30)
        for k in range(4):
            t.b(ALU, 2.2, 3, s * 40 - 26 + k * 13.5, s * 40 - 16 + k * 13.5, 70, 110)
    t.sph(BEACON, (0, 0, 94), 3, 1)


def sensor_ball(t):
    """Electro-optical sensor turret: gimballed ball with a lens."""
    t.c(STE, 0, 0, 0, 6, 16, 16)
    t.sph(OLV, (0, 0, 22), 17, 2)
    t.b(SCR, 13, 17, -6, 6, 17, 27)                     # lens housing
    t.L(LB, 17, 17.4, -4, 4, 19, 25)                    # lens glow


# -----------------------------------------------------------------------------
# Systems
# -----------------------------------------------------------------------------
def radiator_panel(t):
    """Heat-radiator fin assembly: root block + three fins (extends +Y)."""
    t.b(STE, -40, 40, -10, 10, 0, 20)
    for k in range(3):
        x = -28 + k * 28
        t.b(HULL, x - 6, x + 6, 10, 240, 4, 16)
        for j in range(8):
            t.b(COP, x - 6.5, x + 6.5, 30 + j * 26, 38 + j * 26, 16, 17)
        t.L(LA, x - 1, x + 1, 230, 238, 16, 17)


def solar_wing(t):
    """Folding solar wing: mast + three panels (extends +Y)."""
    t.b(STE, -8, 8, -8, 8, 0, 12)
    t.p(STE, (0, 0, 6), (0, 380, 6), 3, 8)
    for k in range(3):
        y0 = 30 + k * 115
        t.b(STE, -62, 62, y0, y0 + 110, 4, 7)
        t.b(BLU_PV, -58, 58, y0 + 4, y0 + 106, 7, 7.6)
        for j in range(1, 6):
            t.b(ALU, -58, 58, y0 + j * 18, y0 + j * 18 + 1, 7.6, 7.8)


BLU_PV = "M_Prop_Blue"


def rcs_quad(t):
    """Reaction-control thruster block: four nozzles (+X, -X, +Y, -Y)."""
    t.b(HULL, -14, 14, -14, 14, 0, 18)
    t.b(BLK, -14.2, 14.2, -14.2, 14.2, 15, 16)
    for (dx, dy) in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        yaw = math.degrees(math.atan2(dy, dx))
        n = t.at(dx * 14, dy * 14, 9, yaw)
        n.p(STE, (0, 0, 0), (6, 0, 0), 4, 10)
        n.p(BLK, (6, 0, 0), (12, 0, 0), 5.5, 10)


def fuel_tank(t, L=240.0, r=40.0):
    """Strap-on cylindrical tank along X with saddles and bands."""
    for x in (-L * 0.3, L * 0.3):
        t.b(STE, x - 8, x + 8, -r * 0.8, r * 0.8, 0, r * 0.7)
    t.p(WHT, (-L / 2, 0, r + 6), (L / 2, 0, r + 6), r, 16)
    for x in (-L / 2, L / 2):
        t.sph(WHT, (x, 0, r + 6), r, 1)
    for x in (-L * 0.3, L * 0.3):
        t.p(STE, (x - 3, 0, r + 6), (x + 3, 0, r + 6), r + 2, 16)
    t.b(ORG, -L * 0.15, L * 0.15, -r - 2.5, -r - 1.5, r - 4, r + 16)
    t.p(COP, (L / 2 + r * 0.8, 0, r + 6), (L / 2 + r + 20, 0, 4), 3, 8)


def spherical_tank(t, r=70.0):
    for a in range(4):
        ang = math.radians(a * 90 + 45)
        t.p(STE, (r * 0.7 * math.cos(ang), r * 0.7 * math.sin(ang), 0), (r * 0.6 * math.cos(ang), r * 0.6 * math.sin(ang), r * 0.7), 4)
    t.sph(WHT, (0, 0, r + 10), r, 2)
    t.ring(STE, 0, 0, r - 1, r + 2, r + 6, r + 14, 24)     # equator band
    t.b(YEL, -18, 18, -r - 2, -r, r + 20, r + 40)


def cargo_pod(t, color=ORG):
    """Clamped cargo container pod (L 300 x 150 x 150)."""
    t.b(STE, -150, 150, -60, 60, 0, 10)
    t.b(color, -150, 150, -75, 75, 10, 160)
    for k in range(9):
        x = -135 + k * 34
        for s in (-1, 1):
            t.b(color, x - 4, x + 4, s * 75, s * 77, 20, 150)
    for x in (-150, 150):
        t.b(STE, x - 6 if x > 0 else x, x if x > 0 else x + 6, -78, 78, 8, 162)
    for x in (-90, 90):
        t.b(YEL, x - 10, x + 10, -80, 80, 4, 14)
    t.b(WHT, -40, 40, -77.5, -77, 70, 110)


def turret_pd(t):
    """Point-defence turret: ring mount, housing, twin barrels along +X."""
    t.c(HULL, 0, 0, 0, 10, 45, 20)
    t.c(BLK, 0, 0, 10, 14, 40, 20)
    t.e(OLV, [(-35, 14), (30, 14), (38, 30), (20, 50), (-35, 50)], 'y', -28, 28)
    for s in (-1, 1):
        t.p(BLK, (30, s * 12, 34), (120, s * 12, 34), 4.5, 10)
        t.p(STE, (110, s * 12, 34), (125, s * 12, 34), 5.5, 10)
    t.b(SCR, 22, 30, -8, 8, 40, 48)


def missile_pod(t):
    """Box launcher with a 3x3 tube face (+X)."""
    t.b(OLV, -60, 40, -40, 40, 0, 70)
    for r in range(3):
        for c in range(3):
            y = -24 + c * 24
            z = 12 + r * 22
            t.p(BLK, (40, y, z), (42, y, z), 9, 12)
            t.p(RED, (41.5, y, z), (42.5, y, z), 5, 8)
    t.b(YEL, -60, -20, -40.5, -40, 55, 62)


def engine_nozzle(t):
    """Auxiliary thruster bell pointing -X with a glowing throat."""
    t.b(HULL, 0, 60, -35, 35, 0, 50)
    t.p(STE, (0, 0, 25), (-70, 0, 25), 22, 16)             # bell
    t.p(BLK, (-60, 0, 25), (-72, 0, 25), 26, 16)           # lip
    t.L(LB, -73, -72, -12, 12, 13, 37)                    # hot throat


# -----------------------------------------------------------------------------
# Hull detail / greebles
# -----------------------------------------------------------------------------
def hull_vent(t):
    t.b(HULL, -40, 40, -25, 25, 0, 4)
    for k in range(7):
        t.b(BLK, -34, 34, -21 + k * 6.5, -18 + k * 6.5, 4, 6)


def access_hatch(t):
    t.b(HULL, -45, 45, -45, 45, 0, 3)
    t.b(STE, -38, 38, -38, 38, 3, 5)
    t.b(YEL, -45, -38, -45, 45, 3, 3.5)
    t.b(YEL, 38, 45, -45, 45, 3, 3.5)
    t.c(STE, 0, 0, 5, 8, 10, 12)
    t.b(BLK, -2, 2, -12, 12, 8, 9)


def conduit_run(t, L=300.0):
    for k, (y, r) in enumerate(((-12, 5), (0, 7), (12, 4))):
        t.p((COP, STE, COP)[k], (-L / 2, y, r + 2), (L / 2, y, r + 2), r, 10)
    for x in range(int(-L / 2 + 20), int(L / 2), 60):
        t.b(STE, x - 4, x + 4, -20, 20, 0, 18)


def greeble_panel(t):
    """Mixed surface detail block: plates, boxes, pipe stubs."""
    t.b(HULL, -60, 60, -40, 40, 0, 4)
    t.b(STE, -50, -10, -30, 0, 4, 14)
    t.b(BLK, 0, 45, 10, 30, 4, 10)
    t.b(HULL, 10, 30, -30, -5, 4, 20)
    t.p(COP, (-50, 20, 9), (-5, 20, 9), 4, 8)
    t.c(STE, 40, -20, 4, 16, 8, 10)
    t.L(LG, 44, 45, 12, 16, 6, 8)


def airlock_hatch_ext(t):
    """Exterior airlock door with hazard frame and status lamps (faces +Z)."""
    t.b(STE, -80, 80, -110, 110, 0, 8)
    t.b(HULL, -64, 64, -94, 94, 8, 12)
    for k in range(10):
        t.b(YEL if k % 2 else BLK, -80 + k * 16, -64 + k * 16, 94, 110, 8, 9)
        t.b(YEL if k % 2 else BLK, -80 + k * 16, -64 + k * 16, -110, -94, 8, 9)
    t.c(STE, 0, 0, 12, 16, 22, 16)
    for s in (-1, 1):
        t.b(BEACON, s * 70 - 5, s * 70 + 5, 100, 106, 9, 12)


def observation_blister(t):
    t.c(HULL, 0, 0, 0, 12, 90, 24)
    t.sph(GLS, (0, 0, 12), 80, 2)
    for a in range(6):
        ang = math.radians(a * 60)
        t.p(STE, (80 * math.cos(ang), 80 * math.sin(ang), 12), (0, 0, 88), 1.5, 6)


def docking_clamp(t):
    """Station docking clamp: arm pair with pads (grips along +X)."""
    t.b(STE, -40, 40, -40, 40, 0, 30)
    for s in (-1, 1):
        t.b(HULL, 20, 160, s * 30 - 10, s * 30 + 10, 20, 40)
        t.b(STE, 150, 170, s * 30 - 20, s * 30 + 20, 10, 50)
        t.b(YEL, 150, 170, s * 30 - 21, s * 30 - 19 if s < 0 else s * 30 + 21, 10, 50)
    t.p(BLK, (0, 0, 30), (140, 0, 30), 6, 10)


def station_sign(t):
    """Lit station name board on standoff legs (faces +X): header strip,
    five glowing bars that read as lettering from a distance."""
    for s in (-1, 1):
        t.p(STE, (0, s * 160, 0), (0, s * 160, 80), 5, 8)
    t.b(STE, -8, 0, -200, 200, 80, 200)
    t.b(BLK, 0, 2, -195, 195, 85, 195)
    t.b(BEACON, 2, 3, -190, 190, 180, 190)
    for k in range(7):
        y = -165 + k * 55
        t.b(BEACON, 2, 3, y - 20, y + 20, 100 + (k % 2) * 8, 160 - (k % 3) * 6)


def holo_billboard(t):
    """Holographic advert frame: emitter rails and a glowing panel."""
    for s in (-1, 1):
        t.b(STE, -10, 10, s * 150 - 10, s * 150 + 10, 0, 260)
        t.L(LC, 10, 11, s * 150 - 4, s * 150 + 4, 20, 250)
    t.b(STE, -10, 10, -160, 160, 250, 270)
    t.b(BEACON, -0.5, 0.5, -140, 140, 30, 240)


def traffic_beacon(t):
    """Traffic-control light tower: three-colour stack on a post."""
    t.c(STE, 0, 0, 0, 200, 8, 12)
    for k, c in enumerate((LR, LA, LG)):
        z = 200 + k * 30
        t.c(BLK, 0, 0, z, z + 26, 14, 16)
        t.Lc(c, 0, 0, z + 4, z + 22, 14.5, 16)
    t.c(STE, 0, 0, 290, 296, 16, 16)


def hab_pod(t):
    """Small habitat pod: capsule with a window band (along X)."""
    t.p(HULL, (-150, 0, 110), (150, 0, 110), 100, 20)
    for x in (-150, 150):
        t.sph(HULL, (x, 0, 110), 100, 2)
    for k in range(6):
        x = -120 + k * 48
        for s in (-1, 1):
            t.b(GLS, x - 14, x + 14, s * 99, s * 101, 108, 132)
    t.L(LA, -130, 130, -101.5, -100.5, 106, 108)
    t.L(LA, -130, 130, 100.5, 101.5, 106, 108)
    for x in (-100, 100):
        t.b(STE, x - 15, x + 15, -60, 60, 0, 20)



def drone_dock(t):
    """Maintenance-drone cradle with a parked drone."""
    t.b(STE, -40, 40, -40, 40, 0, 10)
    t.b(YEL, -40, 40, -41, -40, 0, 10)
    t.c(BLK, 0, 0, 10, 30, 12, 12)
    t.sph(YEL, (0, 0, 44), 16, 2)
    for a in range(4):
        ang = math.radians(a * 90 + 45)
        c = (30 * math.cos(ang), 30 * math.sin(ang), 44)
        t.p(BLK, (0, 0, 44), c, 1.6)
        t.c(BLK, c[0], c[1], 42, 45, 10, 12)
    t.sph(BEACON, (14, 0, 46), 2.5, 1)


# -----------------------------------------------------------------------------
# Catalogue. placement: "surface" (on a hull, +Z = normal), "tip" (wing/edge
# light), "free" (floating, e.g. billboard). Heads are spun by the component.
# -----------------------------------------------------------------------------
EXTERIOR = {
    "NavLight": (nav_light, "tip"), "Strobe": (strobe, "surface"), "Beacon": (beacon, "surface"),
    "BeaconHead": (beacon_head, "surface"), "Floodlight": (floodlight, "surface"),
    "DockingGuide": (docking_guide, "surface"), "SpotlightTower": (spotlight_tower, "surface"),
    "AntennaMast": (antenna_mast, "surface"), "WhipAntenna": (whip_antenna, "surface"),
    "SensorDish": (sensor_dish, "surface"), "RadarBase": (radar_base, "surface"), "RadarHead": (radar_head, "surface"),
    "CommArray": (comm_array, "surface"), "SensorBall": (sensor_ball, "surface"),
    "Radiator": (radiator_panel, "surface"), "SolarWing": (solar_wing, "surface"), "RCSQuad": (rcs_quad, "surface"),
    "FuelTank": (fuel_tank, "surface"), "SphereTank": (spherical_tank, "surface"), "CargoPod": (cargo_pod, "surface"),
    "TurretPD": (turret_pd, "surface"), "MissilePod": (missile_pod, "surface"), "AuxThruster": (engine_nozzle, "surface"),
    "HullVent": (hull_vent, "surface"), "AccessHatch": (access_hatch, "surface"), "ConduitRun": (conduit_run, "surface"),
    "Greeble": (greeble_panel, "surface"), "AirlockHatch": (airlock_hatch_ext, "surface"),
    "ObservationBlister": (observation_blister, "surface"), "DockingClamp": (docking_clamp, "surface"),
    "StationSign": (station_sign, "surface"), "HoloBillboard": (holo_billboard, "surface"),
    "TrafficBeacon": (traffic_beacon, "surface"), "HabPod": (hab_pod, "surface"), "DroneDock": (drone_dock, "surface"),
}
