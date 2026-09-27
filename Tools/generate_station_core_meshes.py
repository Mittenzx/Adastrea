"""
Adastrea Station Core Meshes (Blender 5.x, headless)
=====================================================
Builds the six station core hulls AStationCore_* (Source/Adastrea/Private/Stations/
StationCoreModule.cpp) use, one per archetype:

  SM_Station_Core_TradeHub_01      4x4 cells  tiered octagonal trade tower
  SM_Station_Core_Agricultural_01  4x4 cells  greenhouse ring round a domed hub
  SM_Station_Core_Industrial_01    3x3 cells  refinery block, smelter stacks, ore hopper
  SM_Station_Core_Research_01      3x3 cells  lab drum, big sensor dish, observatory sphere
  SM_Station_Core_Luxury_01        3x3 cells  octagonal gallery under a glass spire
  SM_Station_Core_BlackMarket_01   3x3 cells  hollowed asteroid with bolted-on habs

Authored to the Station Editor grid exactly like the module kit
(Tools/generate_station_module_meshes.py): centimetres, 400 cm cells, pivot on the
footprint centre at module mid-height (z = 0), the hull body inset 30 cm per side and
a connection collar on each of the four side faces reaching the footprint boundary,
where an attached module's own collar meets it. Everything stays inside the
footprint horizontally; the core is free to rise above and hang below the module band.
The footprint sizes must match InitCore() in StationCoreModule.cpp.

Material slots are bound by name on import (Tools/import_art_gap_assets.py):
  M_Station_<Type>            archetype hull (each archetype's own texture set)
  M_StationModule_Connector   connection collars (same as the modules' collars)
  M_StationModule_Shell       trim, decks, struts
  M_Fighter_Glass             windows, greenhouse glass, domes
  M_Nav_White / M_Nav_Red     marker lights
  M_Nav_Green                 greenhouse grow-light domes (Agricultural)
  MI_Asteroid_Iron            the Black Market's asteroid (same as the level's asteroids)
Collision is authored as UCX_ boxes.

Usage:
  "C:\\Program Files\\Blender Foundation\\Blender 5.2\\blender.exe" -b --python Tools/generate_station_core_meshes.py [-- TradeHub Research ...]
"""
import math
import os
import random
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen  # noqa: E402
import generate_station_module_meshes as modkit  # noqa: E402  (collision export + OUT_DIR)

GRID = modkit.GRID
INSET = modkit.INSET
COLLAR_R = GRID * 0.32   # modules' face_collar radius (min(L, H) * 0.32 with H = 400)

GLASS = "M_Fighter_Glass"
CONNECTOR = "M_StationModule_Connector"
TRIM = "M_StationModule_Shell"
LIGHT = "M_Nav_White"
LIGHT_RED = "M_Nav_Red"
GROW_LIGHT = "M_Nav_Green"


# ----------------------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------------------
def slot(ob, matname):
    """Give a part its material slot (join merges slots by material)."""
    mat = bpy.data.materials.get(matname) or bpy.data.materials.new(matname)
    ob.data.materials.clear()
    ob.data.materials.append(mat)
    return ob


def frustum(name, r_bottom, r_top, z0, z1, verts=32, rot_z=0.0, loc_xy=(0, 0)):
    bpy.ops.mesh.primitive_cone_add(radius1=r_bottom, radius2=r_top, depth=z1 - z0, vertices=verts,
                                    location=(loc_xy[0], loc_xy[1], (z0 + z1) / 2))
    ob = bpy.context.active_object
    ob.name = name
    ob.rotation_euler = (0, 0, rot_z)
    gen.sel_activate(ob)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
    return ob


def octagon(name, flats, z0, z1, flats_top=None):
    """Octagonal drum with its flat faces on the X/Y axes (collars sit on flats)."""
    r = flats / math.cos(math.pi / 8)
    rt = (flats_top if flats_top is not None else flats) / math.cos(math.pi / 8)
    return frustum(name, r, rt, z0, z1, verts=8, rot_z=math.pi / 8)


def collars(prefix, half, inner, zc=0.0):
    """A connection collar on each side face: neck from `inner` (inside the hull) out
    to the footprint boundary, a trim ring on the boundary and a sleeve behind it."""
    parts = []
    for axis, sign in ((0, 1), (0, -1), (1, 1), (1, -1)):
        length = half - inner
        mid = sign * (inner + length / 2)
        ring_at = sign * (half - 8)
        sleeve_at = sign * (half - INSET - 20)
        if axis == 0:
            rot = (0, math.radians(90), 0)
            neck_loc, ring_loc, sl_loc = (mid, 0, zc), (ring_at, 0, zc), (sleeve_at, 0, zc)
        else:
            rot = (math.radians(90), 0, 0)
            neck_loc, ring_loc, sl_loc = (0, mid, zc), (0, ring_at, zc), (0, sleeve_at, zc)
        tag = f"{prefix}_Collar{axis}{'p' if sign > 0 else 'n'}"
        parts.append(slot(gen.cyl(tag + "Neck", COLLAR_R, length, loc=neck_loc, rot=rot, verts=20), CONNECTOR))
        parts.append(slot(gen.torus(tag + "Ring", COLLAR_R, 7, loc=ring_loc, rot=rot, maj=24, minr=6), CONNECTOR))
        parts.append(slot(gen.cyl(tag + "Sleeve", COLLAR_R * 1.35, 40, loc=sl_loc, rot=rot, verts=20), TRIM))
    return parts


def lights(prefix, points, r=14, mat=LIGHT):
    return [slot(gen.sphere(f"{prefix}_Light{i}", r, loc=p, verts=10), mat) for i, p in enumerate(points)]


def ring_points(radius, z, n, phase=0.0):
    return [(radius * math.cos(phase + 2 * math.pi * i / n), radius * math.sin(phase + 2 * math.pi * i / n), z)
            for i in range(n)]


def fit_rock(ob, max_half):
    """Scale a displaced rock down so it stays inside the footprint horizontally."""
    gen.apply_mods(ob)
    xs = [v.co.x for v in ob.data.vertices]
    ys = [v.co.y for v in ob.data.vertices]
    ext = max(max(map(abs, xs)), max(map(abs, ys)))
    if ext > max_half:
        k = max_half / ext
        ob.scale = (k, k, k)
        gen.sel_activate(ob)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)


def finalize_core(parts, outname, ucx_specs):
    """Join, pivot on the footprint centre (z = 0), UV, collision, export."""
    for p in parts:
        gen.apply_mods(p)
        # bake every part's rotation/scale: join keeps the active part's transform
        # as an object transform instead of baking it into the mesh
        gen.sel_activate(p)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    joined = gen.join(parts, outname + "_Geo")
    gen.clean_mesh(joined)
    bpy.context.scene.cursor.location = (0, 0, 0)
    gen.sel_activate(joined)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    gen.smart_uv(joined)
    joined.name = outname
    ucx = [modkit.make_ucx_box(f"UCX_{outname}_{i:02d}", *spec) for i, spec in enumerate(ucx_specs)]
    xs = [v.co.x for v in joined.data.vertices]
    ys = [v.co.y for v in joined.data.vertices]
    zs = [v.co.z for v in joined.data.vertices]
    out = modkit.export_with_collision(joined, ucx, outname)
    tris = sum(max(0, len(p.vertices) - 2) for p in joined.data.polygons)
    print(f"  {outname}: x {min(xs):.0f}..{max(xs):.0f}  y {min(ys):.0f}..{max(ys):.0f}  "
          f"z {min(zs):.0f}..{max(zs):.0f}  tris={tris}  slots={[m.name for m in joined.data.materials]}")
    return out


# ----------------------------------------------------------------------------
# Cores
# ----------------------------------------------------------------------------
def build_tradehub():
    """4x4: a stepped octagonal tower - wide trading concourse at module height,
    three narrowing office tiers, a comms crown, and a ventral warehouse cone."""
    half = 2 * GRID
    body = "M_Station_Hab"
    flats = half - INSET                                  # 770
    p = []
    base = slot(octagon("TH_Concourse", flats, -260, 260), body); gen.bevel(base, 18, 2); p.append(base)
    p.append(slot(octagon("TH_Window", flats + 4, 90, 170), GLASS))
    p.append(slot(octagon("TH_Deck", flats + 12, 250, 290), TRIM))
    for i, (f0, f1, z0, z1) in enumerate(((600, 560, 290, 590), (440, 400, 590, 860), (300, 270, 860, 1080))):
        t = slot(octagon(f"TH_Tier{i}", f0, z0, z1, f1), body); gen.bevel(t, 12, 2); p.append(t)
        p.append(slot(octagon(f"TH_TierWin{i}", (f0 + f1) / 2 + 4, z0 + (z1 - z0) * 0.45, z0 + (z1 - z0) * 0.7), GLASS))
        p.append(slot(octagon(f"TH_TierLip{i}", f0 + 14, z0, z0 + 24), TRIM))
    # buttress fins on the diagonals tying the tiers to the concourse
    for i in range(4):
        a = math.pi / 4 + i * math.pi / 2
        fin = slot(gen.box(f"TH_Buttress{i}", 380, 50, 520, loc=(560 * math.cos(a), 560 * math.sin(a), 450),
                           rot=(0, 0, a)), TRIM)
        gen.bevel(fin, 8, 1); p.append(fin)
    # comms crown + mast
    p.append(slot(gen.cyl("TH_Crown", 200, 120, loc=(0, 0, 1140), verts=24), TRIM))
    p.append(slot(gen.cyl("TH_Mast", 26, 420, loc=(0, 0, 1410), verts=12), TRIM))
    p.append(slot(gen.torus("TH_MastRing", 110, 10, loc=(0, 0, 1320), maj=24, minr=6), TRIM))
    # ventral warehouse cone + cargo pods
    p.append(slot(octagon("TH_Keel", 620, -620, -260, 420), body))
    p.append(slot(gen.cyl("TH_KeelPort", 160, 140, loc=(0, 0, -690), verts=24), TRIM))
    for i, (x, y, z) in enumerate(ring_points(430, -430, 4, math.pi / 4)):
        pod = slot(gen.box(f"TH_Pod{i}", 180, 180, 220, loc=(x, y, z), rot=(0, 0, math.pi / 4)), TRIM)
        gen.bevel(pod, 10, 1); p.append(pod)
    p += collars("TH", half, flats - 60)
    p += lights("TH", ring_points(flats + 10, 275, 8, math.pi / 8) + [(0, 0, 1630)], r=16)
    ucx = [(0, 0, 0, 2 * flats, 2 * flats, 520), (0, 0, 440, 1180, 1180, 300), (0, 0, 725, 860, 860, 270),
           (0, 0, 970, 580, 580, 220), (0, 0, -440, 1200, 1200, 360)]
    return finalize_core(p, "SM_Station_Core_TradeHub_01", ucx)


def build_agricultural():
    """4x4: a fat greenhouse ring on four spokes round a domed hydroponics hub,
    with water tanks slung underneath."""
    half = 2 * GRID
    body = "M_Station_Agricultural"
    tube_r = 150
    ring_major = half - INSET - tube_r                     # outer edge at 770
    p = []
    ring = slot(gen.torus("AG_Ring", ring_major, tube_r, maj=48, minr=16), body); p.append(ring)
    p.append(slot(gen.torus("AG_RingBelt", ring_major, tube_r + 6, loc=(0, 0, 0), maj=48, minr=6), TRIM))
    # greenhouse glass along the ring's top, between the spokes
    for i, (x, y, z) in enumerate(ring_points(ring_major, 95, 8, math.pi / 8)):
        dome = gen.sphere(f"AG_Greenhouse{i}", 130, loc=(x, y, z), verts=20)
        dome.scale = (1.0, 1.0, 0.55); gen.sel_activate(dome)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        p.append(slot(dome, GROW_LIGHT))
    # spokes on the axes (collars sit at their ends) + diagonal braces
    for i in range(4):
        a = i * math.pi / 2
        sp = slot(gen.box(f"AG_Spoke{i}", ring_major, 170, 170, loc=(ring_major / 2 * math.cos(a), ring_major / 2 * math.sin(a), 0),
                          rot=(0, 0, a)), TRIM)
        gen.bevel(sp, 10, 1); p.append(sp)
        b = a + math.pi / 4
        p.append(slot(gen.box(f"AG_Brace{i}", ring_major - 120, 50, 50, loc=(ring_major / 2 * math.cos(b), ring_major / 2 * math.sin(b), -90),
                              rot=(0, 0, b)), TRIM))
    hub = slot(gen.cyl("AG_Hub", 330, 600, verts=32), body); gen.bevel(hub, 14, 2); p.append(hub)
    p.append(slot(gen.cyl("AG_HubBand", 336, 70, loc=(0, 0, 150), verts=32), GLASS))
    dome = gen.sphere("AG_Dome", 300, loc=(0, 0, 300), verts=32)
    dome.scale = (1.0, 1.0, 0.8); gen.sel_activate(dome)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    p.append(slot(dome, GLASS))
    for i, z in enumerate((360, 450)):
        p.append(slot(gen.torus(f"AG_DomeRib{i}", 300 * math.sqrt(max(0.0, 1 - ((z - 300) / 240) ** 2)), 8, loc=(0, 0, z), maj=32, minr=6), TRIM))
    # water tanks under the hub
    for i, (x, y, z) in enumerate(ring_points(190, -420, 3)):
        p.append(slot(gen.sphere(f"AG_Tank{i}", 150, loc=(x, y, z), verts=24), TRIM))
    p.append(slot(gen.cyl("AG_TankSpine", 70, 300, loc=(0, 0, -420), verts=16), body))
    p += collars("AG", half, ring_major - 60)
    p += lights("AG", ring_points(ring_major, tube_r + 10, 4, math.pi / 4) + [(0, 0, 545)])
    ucx = [(0, 0, 0, 660, 660, 600), (0, 0, 0, 2 * (ring_major + tube_r), 2 * (ring_major + tube_r), 2 * tube_r),
           (0, 0, 400, 600, 600, 250), (0, 0, -420, 640, 640, 300)]
    return finalize_core(p, "SM_Station_Core_Agricultural_01", ucx)


def build_industrial():
    """3x3: a heavy refinery block with four smelter stacks, a furnace house, ore
    tanks and pipework on top, and an ore-intake hopper underneath."""
    half = 1.5 * GRID
    body = "M_Station_Industrial"
    s = half - INSET                                      # 570
    p = []
    blk = slot(gen.box("IN_Block", 2 * s, 2 * s, 480), body); gen.bevel(blk, 24, 2); p.append(blk)
    p.append(slot(gen.box("IN_Deck", 2 * s + 20, 2 * s + 20, 30, loc=(0, 0, 250)), TRIM))
    for i, (x, y) in enumerate(((1, 1), (-1, 1), (1, -1), (-1, -1))):
        cx, cy = x * 390, y * 390
        p.append(slot(gen.cyl(f"IN_Stack{i}", 95, 720, loc=(cx, cy, 625), verts=20), body))
        p.append(slot(gen.torus(f"IN_StackRim{i}", 95, 14, loc=(cx, cy, 985), maj=20, minr=6), TRIM))
        for z in (520, 760):
            p.append(slot(gen.torus(f"IN_StackBand{i}_{z}", 98, 9, loc=(cx, cy, z), maj=20, minr=6), TRIM))
    furnace = slot(gen.box("IN_Furnace", 460, 460, 340, loc=(0, 0, 435)), body); gen.bevel(furnace, 16, 2); p.append(furnace)
    p.append(slot(gen.box("IN_FurnaceVent", 300, 300, 40, loc=(0, 0, 625)), TRIM))
    for i, y in enumerate((-390, 390)):
        p.append(slot(gen.sphere(f"IN_Tank{i}", 150, loc=(0, y, 420), verts=24), TRIM))
    # pipe runs between the stacks and the furnace
    for i, (x, y, rot) in enumerate(((390, 0, (math.radians(90), 0, 0)), (-390, 0, (math.radians(90), 0, 0)),
                                     (0, 0, (0, math.radians(90), 0)))):
        p.append(slot(gen.cyl(f"IN_Pipe{i}", 26, 780, loc=(x, y, 330 if i < 2 else 560), rot=rot, verts=12), CONNECTOR))
    # side vent banks (clear of the centre collars)
    for i, (axis, sign) in enumerate(((0, 1), (0, -1), (1, 1), (1, -1))):
        for off in (-330, 330):
            loc = (sign * (s + 6), off, 60) if axis == 0 else (off, sign * (s + 6), 60)
            size = (14, 180, 200) if axis == 0 else (180, 14, 200)
            p.append(slot(gen.box(f"IN_Vent{i}_{off}", *size, loc=loc), TRIM))
    hopper = slot(frustum("IN_Hopper", 180, 520, -620, -240, verts=4, rot_z=math.pi / 4), body); p.append(hopper)
    p.append(slot(gen.box("IN_Intake", 300, 300, 80, loc=(0, 0, -660)), TRIM))
    p += collars("IN", half, s - 60)
    p += lights("IN", [(x * 390, y * 390, 1005) for x, y in ((1, 1), (-1, 1), (1, -1), (-1, -1))], r=18, mat=LIGHT_RED)
    ucx = [(0, 0, 0, 2 * s, 2 * s, 480), (0, 0, 435, 460, 460, 340),
           (0, 0, 625, 2 * 485, 2 * 485, 720), (0, 0, -430, 740, 740, 380)]
    return finalize_core(p, "SM_Station_Core_Industrial_01", ucx)


def build_research():
    """3x3: a lab drum with a glazed ring deck, a big tilted sensor dish on a mast,
    a spray of antennae, and a glass observatory sphere underneath."""
    half = 1.5 * GRID
    body = "M_Station_Research"
    r_drum = half - INSET                                 # 570
    p = []
    drum = slot(gen.cyl("RS_Drum", r_drum, 460, verts=24), body); gen.bevel(drum, 16, 2); p.append(drum)
    p.append(slot(gen.cyl("RS_DrumWin", r_drum + 4, 70, loc=(0, 0, 110), verts=24), GLASS))
    lab = slot(frustum("RS_Lab", 440, 380, 230, 520, verts=24), body); gen.bevel(lab, 10, 1); p.append(lab)
    p.append(slot(frustum("RS_LabWin", 424, 396, 330, 410, verts=24), GLASS))
    p.append(slot(gen.cyl("RS_Deck", r_drum + 14, 26, loc=(0, 0, 238), verts=24), TRIM))
    # dish on a mast, tilted towards +X
    p.append(slot(gen.cyl("RS_Mast", 60, 320, loc=(0, 0, 680), verts=16), TRIM))
    dish = slot(frustum("RS_Dish", 70, 470, 0, 150, verts=32), TRIM)
    dish.location = (60, 0, 860); dish.rotation_euler = (0, math.radians(22), 0)
    p.append(dish)
    p.append(slot(gen.cyl("RS_Feed", 14, 360, loc=(125, 0, 1070), rot=(0, math.radians(22), 0), verts=8), TRIM))
    p.append(slot(gen.sphere("RS_FeedHorn", 40, loc=(190, 0, 1235), verts=12), TRIM))
    # antennae on the drum rim, between the collars
    for i, (x, y, _) in enumerate(ring_points(470, 0, 4, math.pi / 4)):
        h = 380 if i % 2 else 520
        p.append(slot(gen.cyl(f"RS_Antenna{i}", 12, h, loc=(x, y, 230 + h / 2), verts=8), TRIM))
        p.append(slot(gen.box(f"RS_AntennaBar{i}", 120, 12, 12, loc=(x, y, 230 + h * 0.8), rot=(0, 0, math.atan2(y, x) + math.pi / 2)), TRIM))
    # observatory under the drum
    p.append(slot(frustum("RS_Neck", 260, 200, -380, -230, verts=24), body))
    p.append(slot(gen.sphere("RS_Observatory", 300, loc=(0, 0, -560), verts=32), GLASS))
    p.append(slot(gen.torus("RS_ObsRing", 305, 14, loc=(0, 0, -560), maj=32, minr=8), TRIM))
    p += collars("RS", half, r_drum - 60)
    p += lights("RS", [(190, 0, 1235)] + ring_points(r_drum + 10, 245, 4, math.pi / 4), r=14)
    ucx = [(0, 0, 0, 2 * r_drum, 2 * r_drum, 460), (0, 0, 375, 880, 880, 290),
           (0, 0, 700, 140, 140, 360), (0, 0, -560, 600, 600, 600)]
    return finalize_core(p, "SM_Station_Core_Research_01", ucx)


def build_luxury():
    """3x3: an octagonal gallery hub with a wide glass promenade, a tall glass
    spire ribbed with bronze fins, and a tapering keel."""
    half = 1.5 * GRID
    body = "M_Station_Luxury"
    flats = half - INSET                                  # 570
    p = []
    hub = slot(octagon("LX_Gallery", flats, -230, 230), body); gen.bevel(hub, 14, 2); p.append(hub)
    p.append(slot(octagon("LX_Promenade", flats + 4, 30, 170), GLASS))
    p.append(slot(octagon("LX_Roof", flats + 16, 220, 250), TRIM))
    p.append(slot(gen.torus("LX_Halo", 500, 30, loc=(0, 0, 330), maj=40, minr=10), TRIM))
    spire = slot(frustum("LX_Spire", 360, 200, 250, 1050, verts=8, rot_z=math.pi / 8), GLASS); p.append(spire)
    for i in range(4):
        a = math.pi / 4 + i * math.pi / 2
        fin = slot(gen.box(f"LX_Fin{i}", 40, 40, 820, loc=(300 * math.cos(a), 300 * math.sin(a), 640)), body)
        fin.rotation_euler = (math.radians(-9) * math.sin(a), math.radians(9) * math.cos(a), 0)
        p.append(fin)
    for i, (z, r) in enumerate(((500, 320), (760, 270), (980, 220))):
        p.append(slot(gen.torus(f"LX_SpireRing{i}", r, 12, loc=(0, 0, z), maj=32, minr=6), body))
    p.append(slot(gen.cone("LX_Crown", 210, 420, loc=(0, 0, 1260), verts=8), body))
    p.append(slot(frustum("LX_Keel", 470, 90, -760, -230, verts=8, rot_z=math.pi / 8), body))
    p.append(slot(gen.torus("LX_KeelRing", 360, 16, loc=(0, 0, -380), maj=32, minr=6), TRIM))
    p += collars("LX", half, flats - 60)
    p += lights("LX", [(0, 0, 1480)] + ring_points(500, 330, 8, math.pi / 8), r=12)
    ucx = [(0, 0, 0, 2 * flats, 2 * flats, 460), (0, 0, 650, 700, 700, 800), (0, 0, -490, 860, 860, 520)]
    return finalize_core(p, "SM_Station_Core_Luxury_01", ucx)


def build_blackmarket():
    """3x3: a lumpy hollowed-out asteroid with scavenged hab boxes, a hangar frame,
    junk antennae and blinking red lights bolted on."""
    half = 1.5 * GRID
    body = "M_Station_BlackMarket"
    p = []
    rk = gen.rock("BM_Rock", 470, sub=4, scale_xyz=(1.0, 0.92, 0.82))
    tex = bpy.data.textures.new("BM_RockNoise", type='CLOUDS'); tex.noise_scale = 1.4
    dis = rk.modifiers.new("disp", 'DISPLACE'); dis.texture = tex; dis.strength = 110; dis.mid_level = 0.5
    fit_rock(rk, half - INSET)
    gen.sel_activate(rk); bpy.ops.object.shade_flat()
    p.append(slot(rk, "MI_Asteroid_Iron"))
    rng = random.Random(4404)
    # bolted-on hab boxes on the top and flanks
    for i, (x, y, z, sx, sy, sz, yaw) in enumerate((
            (-120, 90, 330, 340, 220, 200, 12), (170, -150, 300, 260, 200, 180, -20),
            (-260, -250, 190, 200, 160, 160, 35), (260, 250, 170, 180, 180, 150, 50))):
        hab = slot(gen.box(f"BM_Hab{i}", sx, sy, sz, loc=(x, y, z), rot=(0, 0, math.radians(yaw))), body)
        gen.bevel(hab, 8, 1); p.append(hab)
        p.append(slot(gen.box(f"BM_HabWin{i}", sx * 0.7, sy + 6, 30, loc=(x, y, z + sz * 0.15), rot=(0, 0, math.radians(yaw))), GLASS))
    # hangar frame dug into the -Z side + struts
    p.append(slot(gen.box("BM_HangarFrame", 420, 360, 60, loc=(0, 0, -330)), TRIM))
    for i, (x, y) in enumerate(((1, 1), (-1, 1), (1, -1), (-1, -1))):
        p.append(slot(gen.cyl(f"BM_Strut{i}", 20, 260, loc=(x * 180, y * 150, -440), verts=8), CONNECTOR))
    # scrap antennae
    for i in range(3):
        x, y = rng.uniform(-200, 200), rng.uniform(-200, 200)
        h = rng.uniform(280, 460)
        p.append(slot(gen.cyl(f"BM_Junk{i}", 10, h, loc=(x, y, 380 + h / 2), rot=(rng.uniform(-0.25, 0.25), rng.uniform(-0.25, 0.25), 0), verts=6), TRIM))
    p.append(slot(gen.cone("BM_Dish", 110, 60, loc=(-120, 90, 470), rot=(math.radians(200), 0, 0), verts=16), TRIM))
    # long collars: they have to reach through the rock to the footprint boundary
    p += collars("BM", half, 260)
    p += lights("BM", [(-120, 90, 440), (170, -150, 400), (0, 0, -370), (260, 250, 255)], r=14, mat=LIGHT_RED)
    ucx = [(0, 0, 0, 900, 820, 700), (0, 0, 330, 700, 600, 260)]
    return finalize_core(p, "SM_Station_Core_BlackMarket_01", ucx)


CORES = {"TradeHub": build_tradehub, "Agricultural": build_agricultural, "Industrial": build_industrial,
         "Research": build_research, "Luxury": build_luxury, "BlackMarket": build_blackmarket}


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = argv or list(CORES)
    gen.setup_scene()
    os.makedirs(modkit.OUT_DIR, exist_ok=True)
    for n in names:
        print(f"Building core {n}...")
        gen.clear_scene()
        out = CORES[n]()
        print(f"  exported {out}")


if __name__ == "__main__":
    main()
