"""
Adastrea Station Module Meshes, batch B (Blender 5.x, headless)
===============================================================
Dedicated static meshes for the dock / trade / utility station modules that
currently all render one of the 4 generic shells (SpaceStationModule.cpp):

  DockingPortModule  -> SM_StationModule_DockingPort_01   1x1x1  400 x 400 x 400 cm
  CorridorModule     -> SM_StationModule_Corridor_01      1x1x1  400 x 400 x 400 cm
  HabitationModule   -> SM_StationModule_Habitation_01    2x2x1  800 x 800 x 400 cm
  ReactorModule      -> SM_StationModule_Reactor_01       2x2x1  800 x 800 x 400 cm
  SolarArrayModule   -> SM_StationModule_SolarArray_01    3x1x1 1200 x 400 x 400 cm
  FuelDepotModule    -> SM_StationModule_FuelDepot_01     2x2x1  800 x 800 x 400 cm
  ProcessingModule   -> SM_StationModule_Processing_01    2x2x1  800 x 800 x 400 cm
  FabricationModule  -> SM_StationModule_Fabrication_01   2x2x1  800 x 800 x 400 cm
  BarracksModule     -> SM_StationModule_Barracks_01      2x2x1  800 x 800 x 400 cm

Conventions (same as Tools/generate_station_module_meshes.py / build_station_module_shells):
  * 400 cm per grid cell, footprints from Content/Data/StationModuleBuilderData.json.
  * Values are real-world cm (1 value = 1 UE unit); FBX export settings identical to
    the kit except smoothing = EDGE (these meshes carry smooth-by-angle sharp edges).
  * World-aligned triplanar UV via gen.smart_uv (200 cm tile).
  * Authored UCX_<Mesh>_NN convex collision in the same FBX.
  * Pivot = footprint centre (NOT centre-of-volume): the station editor resolves
    adjacency from actor location (StationEditorManager::IsAdjacentToExistingModule).
  * NOTHING extends past the footprint box, so neighbours butt without interpenetrating.

Connection faces (StationEditorManager::DoesModuleFaceDirection, UE local space):
  N=+X  S=-X  E=+Y  W=-Y  Up=+Z  Down=-Z.
  UE's FBX import mirrors Y (Blender +Y arrives as UE -Y), so in Blender space:
  N=+X  S=-X  E=-Y  W=+Y.  SolarArray (faces=['W']) therefore mounts on Blender +Y.

Kit "port": every connectable side gets one standard port per grid cell: an octagonal
neck, a 190 cm square Connector flange recessed 6 cm, and a hatch disc whose outer face
is exactly on the footprint boundary, plus two small green M_Nav_Green "connection
point" lights. Two neighbouring modules' hatches touch face-to-face.

Material slots reuse existing UE materials by exact name (1:1 mapping on import):
  M_StationModule_Shell / _Connector / _Utility / _Defence / _SolarArray, M_Station_Hab,
  M_Nav_White / M_Nav_Green / M_Nav_Red / M_Nav_Beacon (static emissives).

Usage:
  blender -b --python Tools/generate_station_module_meshes_b.py -- [--dry-run]
          [--only DockingPort,Corridor] [--no-preview] [--review]
  --dry-run   build + report tris/bounds/slots, write nothing
  --review    only render previews of the existing DockingBay/CargoBay/Market FBX
"""
import os
import sys
import math

import bpy
import bmesh
from mathutils import Vector, Euler

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen  # noqa: E402

GRID = 400.0
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "Assets", "FBX", "generated")
PREVIEW_DIR = os.path.join(OUT_DIR, "station_modules")
gen.BASE = OUT_DIR
gen.TEXDIR = os.path.join(OUT_DIR, "Textures")

SHELL = "M_StationModule_Shell"
CONN = "M_StationModule_Connector"
UTIL = "M_StationModule_Utility"
DEF = "M_StationModule_Defence"
SOLAR = "M_StationModule_SolarArray"
HAB = "M_Station_Hab"
L_GREEN = "M_Nav_Green"
L_RED = "M_Nav_Red"
L_WHITE = "M_Nav_White"
L_AMBER = "M_Nav_Beacon"
EMISSIVE = {L_GREEN: (0.0, 1.0, 0.4), L_RED: (1.0, 0.15, 0.15),
            L_WHITE: (1.0, 1.0, 0.95), L_AMBER: (1.0, 0.55, 0.1)}

AXES = {'X': 0, 'Y': 1, 'Z': 2}
AXIS_NAME = 'XYZ'


# ----------------------------------------------------------------------------
# Geometry helpers (bmesh-built, no operator context needed)
# ----------------------------------------------------------------------------
def _map(u, v, w, axis):
    if axis == 'X':
        return Vector((w, u, v))
    if axis == 'Y':
        return Vector((v, w, u))
    return Vector((u, v, w))


def _finish(name, bm, loc, rot):
    m = Euler(rot).to_matrix()
    off = Vector(loc)
    for v in bm.verts:
        v.co = m @ v.co + off
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(ob)
    return ob


def prism(name, r, depth, axis='Z', sides=8, loc=(0, 0, 0), rot=(0, 0, 0), r2=None, phase=None):
    """Closed n-gon prism / frustum along axis. Default phase puts flat faces on the axes."""
    r2 = r if r2 is None else r2
    phase = math.pi / sides if phase is None else phase
    bm = bmesh.new()
    rings = []
    for w, rr in ((-depth / 2, r), (depth / 2, r2)):
        rings.append([bm.verts.new(_map(rr * math.cos(phase + 2 * math.pi * i / sides),
                                        rr * math.sin(phase + 2 * math.pi * i / sides), w, axis))
                      for i in range(sides)])
    for i in range(sides):
        j = (i + 1) % sides
        bm.faces.new((rings[0][i], rings[0][j], rings[1][j], rings[1][i]))
    bm.faces.new(list(reversed(rings[0])))
    bm.faces.new(rings[1])
    return _finish(name, bm, loc, rot)


def annulus(name, r_out, r_in, depth, axis='Z', sides=24, loc=(0, 0, 0), rot=(0, 0, 0)):
    """Flat ring with a rectangular section (hab rings, collars, bands)."""
    phase = math.pi / sides
    bm = bmesh.new()
    rings = []
    for rr, w in ((r_out, -depth / 2), (r_out, depth / 2), (r_in, depth / 2), (r_in, -depth / 2)):
        rings.append([bm.verts.new(_map(rr * math.cos(phase + 2 * math.pi * i / sides),
                                        rr * math.sin(phase + 2 * math.pi * i / sides), w, axis))
                      for i in range(sides)])
    for k in range(4):
        a, b = rings[k], rings[(k + 1) % 4]
        for i in range(sides):
            j = (i + 1) % sides
            bm.faces.new((a[i], a[j], b[j], b[i]))
    return _finish(name, bm, loc, rot)


def beam(name, p0, p1, w, h, extend=0.0):
    """Box whose local X runs from p0 to p1 (struts, spars, conveyors, arms)."""
    p0, p1 = Vector(p0), Vector(p1)
    d = p1 - p0
    q = d.normalized().to_track_quat('X', 'Z')
    return gen.box(name, d.length + extend, w, h, loc=(p0 + p1) / 2, rot=q.to_euler())


def rod(name, p0, p1, r, sides=8):
    p0, p1 = Vector(p0), Vector(p1)
    d = p1 - p0
    q = d.normalized().to_track_quat('X', 'Z')
    return prism(name, r, d.length, 'X', sides, loc=(p0 + p1) / 2, rot=q.to_euler())


def vec_on_axis(axis_i, t, center):
    c = list(center)
    c[axis_i] = t
    return tuple(c)


# ----------------------------------------------------------------------------
# Kit: collects parts per material slot + UCX specs, then joins/exports
# ----------------------------------------------------------------------------
class Kit:
    def __init__(self, name, footprint_cells, primary):
        self.name = name
        self.cells = footprint_cells
        self.size = tuple(c * GRID for c in footprint_cells)
        self.half = tuple(s / 2 for s in self.size)
        self.primary = primary
        self.parts = {}
        self.ucx = []
        self.faces = []
        self._n = 0

    def uid(self, s):
        self._n += 1
        return f"{s}_{self._n:03d}"

    def add(self, slot, ob, bevel=None):
        if bevel:
            gen.bevel(ob, *bevel)
            gen.apply_mods(ob)
        self.parts.setdefault(slot, []).append(ob)
        return ob

    # UCX specs -------------------------------------------------------------
    def ucx_box(self, center, size, rot=(0, 0, 0)):
        self.ucx.append(('box', tuple(center), tuple(size), tuple(rot)))

    def ucx_beam(self, p0, p1, w, h):
        p0, p1 = Vector(p0), Vector(p1)
        d = p1 - p0
        rot = d.normalized().to_track_quat('X', 'Z').to_euler()
        self.ucx.append(('box', tuple((p0 + p1) / 2), (d.length, w, h), tuple(rot)))

    def ucx_prism(self, center, r, depth, axis='Z', sides=12, r2=None, rot=(0, 0, 0)):
        self.ucx.append(('prism', tuple(center), r, depth, axis, sides, r2, tuple(rot)))

    def ucx_ico(self, center, r):
        self.ucx.append(('ico', tuple(center), r))

    # Standard kit port ----------------------------------------------------
    def port(self, axis, sign, center, inner, r=78.0, neck_r=56.0, lights=True, ucx=True,
             pylon_z0=None, flush=False):
        """Connection port on the footprint boundary of `axis` (0/1/2), side `sign`.
        `center` gives the other two coordinates; `inner` is the (positive) distance
        from the module centre where the neck starts (the hull surface).
        Octagonal Connector collar (circumradius r, 18 cm deep, front recessed 5 cm) with
        a 12-sided hatch whose outer face lies exactly on the boundary.
        pylon_z0: instead of a neck, raise a support pylon from z=pylon_z0 behind the collar.
        flush: hatch plate only, sitting on the hull at `inner` (no boundary contact)."""
        a = axis
        half = self.half[a]
        an = AXIS_NAME[a]
        others = [i for i in range(3) if i != a]
        tag = f"Port{an}{'p' if sign > 0 else 'n'}"
        ap = r * math.cos(math.pi / 8)
        if flush:
            self.add(CONN, prism(self.uid(tag + "FlushRing"), r, 8, an, 8,
                                 loc=vec_on_axis(a, sign * (inner + 4), center)))
            self.add(self.primary, prism(self.uid(tag + "FlushHatch"), r * 0.66, 12, an, 12,
                                         loc=vec_on_axis(a, sign * (inner + 4), center)))
            self.faces.append(self._face(a, sign) + "(flush)")
            return
        self.add(self.primary, prism(self.uid(tag + "Hatch"), r * 0.66, 5, an, 12,
                                     loc=vec_on_axis(a, sign * (half - 2.5), center)))
        self.add(CONN, prism(self.uid(tag + "Collar"), r, 18, an, 8,
                             loc=vec_on_axis(a, sign * (half - 14), center)))
        self.add(CONN, annulus(self.uid(tag + "CollarLip"), r + 6, r - 10, 6, an, 8,
                               loc=vec_on_axis(a, sign * (half - 26), center)))
        n0, n1 = inner, half - 23
        if pylon_z0 is not None and a != 2:
            side = [i for i in others if i != 2][0]
            c = list(vec_on_axis(a, sign * (n0 + n1) / 2, center))
            top = center[2] + ap
            c[2] = (pylon_z0 + top) / 2
            dims = [0, 0, top - pylon_z0]
            dims[a] = max(n1 - n0, 4)
            dims[side] = 2 * ap + 20
            self.add(self.primary, gen.box(self.uid(tag + "Pylon"), *dims, loc=c), bevel=(4, 1))
        elif n1 - n0 > 2:
            self.add(CONN, prism(self.uid(tag + "Neck"), neck_r, n1 - n0, an, 8,
                                 loc=vec_on_axis(a, sign * (n0 + n1) / 2, center)))
        if lights:
            up = 2 if a != 2 else 0
            side = [i for i in others if i != up][0]
            for s2 in (-1, 1):
                c = list(vec_on_axis(a, sign * (half - 3.5), center))
                c[side] += s2 * ap * 0.62
                c[up] += ap * 0.62
                ld = [10, 10, 10]
                ld[a] = 3
                self.add(L_GREEN, gen.box(self.uid(tag + "Light"), *ld, loc=c))
        if ucx:
            lo = max(inner - 2, 0)
            dims = [2 * ap, 2 * ap, 2 * ap]
            dims[a] = half - lo
            c = list(vec_on_axis(a, sign * (lo + half) / 2, center))
            if pylon_z0 is not None and a != 2:
                side = [i for i in others if i != 2][0]
                top = center[2] + ap
                dims[2] = top - pylon_z0
                dims[side] = 2 * ap + 20
                c[2] = (pylon_z0 + top) / 2
            self.ucx_box(c, dims)
        self.faces.append(self._face(a, sign))

    @staticmethod
    def _face(a, sign):
        return 'NSEWUD'[{(0, 1): 0, (0, -1): 1, (1, -1): 2, (1, 1): 3, (2, 1): 4, (2, -1): 5}[(a, sign)]]

    def ports_per_cell(self, inner_fn, axes=(0, 1), **kw):
        """One port per grid cell on every side face of the listed axes."""
        for a in axes:
            b = 1 - a if a < 2 else 0
            n = self.cells[b]
            for sign in (1, -1):
                for i in range(n):
                    off = -self.half[b] + GRID / 2 + i * GRID
                    c = [0.0, 0.0, 0.0]
                    c[b] = off
                    self.port(a, sign, tuple(c), inner_fn(a, sign, off), **kw)


def join_objs(objs, name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    if len(objs) > 1:
        bpy.ops.object.join()
    out = bpy.context.view_layer.objects.active
    out.name = name
    out.data.name = name
    return out


def get_material(slot):
    if slot in EMISSIVE:
        mat = gen.emissive_mat(slot, EMISSIVE[slot], strength=10.0)
    else:
        mat = bpy.data.materials.get(slot) or bpy.data.materials.new(slot)
        try:
            gen.wire_pbr_material(mat, gen.resolve_texset(slot), emissive_strength=2.6)
        except Exception as e:  # pragma: no cover
            print(f"  [warn] material wire fail {slot}: {e}")
    return mat


def build_ucx(k):
    objs = []
    for i, spec in enumerate(k.ucx):
        nm = f"UCX_{k.name}_{i:02d}"
        if spec[0] == 'box':
            _, c, s, rot = spec
            ob = gen.box(nm, *s, loc=c, rot=rot)
        elif spec[0] == 'prism':
            _, c, r, depth, axis, sides, r2, rot = spec
            ob = prism(nm, r, depth, axis, sides, loc=c, rot=rot, r2=r2)
        else:
            _, c, r = spec
            bpy.ops.mesh.primitive_ico_sphere_add(radius=r, subdivisions=2, location=c)
            ob = bpy.context.active_object
            ob.name = nm
        ob.name = nm
        ob.data.name = nm
        gen.sel_activate(ob)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        ob.display_type = 'WIRE'
        objs.append(ob)
    return objs


def finalize(k, dry_run=False):
    groups = []
    slots = list(k.parts.keys())
    # primary slot first so it is material index 0
    slots.sort(key=lambda s: (s != k.primary,))
    for slot in slots:
        g = join_objs(k.parts[slot], f"{k.name}__{slot}")
        g.data.materials.clear()
        g.data.materials.append(get_material(slot))
        for p in g.data.polygons:
            p.material_index = 0
        groups.append(g)
    main = join_objs(groups, k.name)
    bpy.context.scene.cursor.location = (0, 0, 0)
    gen.sel_activate(main)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    gen.clean_mesh(main)
    gen.sel_activate(main)
    try:
        bpy.ops.object.shade_smooth_by_angle(angle=math.radians(35))
    except Exception as e:  # pragma: no cover
        print(f"  [warn] smooth by angle failed: {e}")
    gen.smart_uv(main)

    ucx = build_ucx(k)

    tris = sum(len(p.vertices) - 2 for p in main.data.polygons)
    bb = [main.matrix_world @ Vector(c) for c in main.bound_box]
    mn = [min(v[i] for v in bb) for i in range(3)]
    mx = [max(v[i] for v in bb) for i in range(3)]
    over = [AXIS_NAME[i] for i in range(3)
            if mn[i] < -k.half[i] - 0.05 or mx[i] > k.half[i] + 0.05]
    info = dict(name=k.name, tris=tris, verts=len(main.data.vertices),
                slots=[m.name for m in main.data.materials], ucx=len(ucx),
                bounds_min=[round(v, 1) for v in mn], bounds_max=[round(v, 1) for v in mx],
                footprint_cells=k.cells, footprint_cm=k.size, overrun=over,
                port_faces=sorted(set(k.faces)))
    if not dry_run:
        gen.sel_activate(main)
        for u in ucx:
            u.select_set(True)
        out = os.path.join(OUT_DIR, k.name + ".fbx")
        # Geometry values are centimetres, so declare 1 BU = 1 cm. With the factory
        # default scale_length=1.0 the exporter writes UnitScaleFactor=100 (metres) and
        # UE imports every module 100x oversized (Integrator, AGENT_BOARD 2026-09-23T22:15Z).
        bpy.context.scene.unit_settings.system = 'METRIC'
        bpy.context.scene.unit_settings.scale_length = 0.01
        bpy.ops.export_scene.fbx(
            filepath=out, use_selection=True, object_types={'MESH'},
            apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
            mesh_smooth_type='EDGE', axis_forward='-Y', axis_up='Z',
        )
        info['fbx'] = out
    return main, ucx, info


# ----------------------------------------------------------------------------
# Modules
# ----------------------------------------------------------------------------
def build_docking_port():
    """1x1x1. Octagonal airlock body, big docking collar on N (+X) with clamp latches,
    green/red guide lights around the ring and four diagonal approach-light struts.
    Standard ports on S/E/W (the collar itself sits flush on N)."""
    k = Kit("SM_StationModule_DockingPort_01", (1, 1, 1), SHELL)
    h = 200.0
    k.add(SHELL, prism("DP_Core", 150, 290, 'X', 8, loc=(-35, 0, 0)))
    for x in (-120, -10, 90):
        k.add(SHELL, prism(k.uid("DP_Rib"), 160, 14, 'X', 8, loc=(x, 0, 0)))
    # funnel + collar ring + recessed docking door
    k.add(CONN, prism("DP_Funnel", 128, 50, 'X', 16, loc=(135, 0, 0), r2=165))
    k.add(CONN, annulus("DP_Collar", 186, 140, 34, 'X', 24, loc=(177, 0, 0)))
    k.add(SHELL, prism("DP_Door", 142, 10, 'X', 24, loc=(165, 0, 0)))
    k.add(CONN, annulus("DP_DoorSeam", 60, 50, 4, 'X', 16, loc=(171, 0, 0)))
    for i in range(4):
        a = math.radians(90 * i)
        k.add(SHELL, gen.box(k.uid("DP_Latch"), 26, 36, 46,
                             loc=(187, 162 * math.cos(a), 162 * math.sin(a)), rot=(a, 0, 0)),
              bevel=(3, 1))
    for i in range(8):
        a = math.radians(22.5 + 45 * i)
        y, z = 164 * math.cos(a), 164 * math.sin(a)
        k.add(L_RED if y > 0 else L_GREEN, gen.box(k.uid("DP_GuideLight"), 6, 14, 14,
                                                    loc=(197, y, z), rot=(a, 0, 0)))
    # diagonal approach-light struts reaching toward the cell corners
    for i in range(4):
        a = math.radians(45 + 90 * i)
        c, s = math.cos(a), math.sin(a)
        k.add(CONN, beam(k.uid("DP_Strut"), (178, 176 * c, 176 * s), (178, 256 * c, 256 * s), 18, 18))
        k.add(SHELL, gen.box(k.uid("DP_StrutRoot"), 40, 34, 34, loc=(172, 170 * c, 170 * s),
                             rot=(a, 0, 0)))
        k.add(L_RED if c > 0 else L_GREEN,
              gen.box(k.uid("DP_TipLight"), 18, 18, 18, loc=(186, 264 * c, 264 * s), rot=(a, 0, 0)))
    # dorsal antenna + ventral RCS pod
    k.add(CONN, prism("DP_Antenna", 5, 50, 'Z', 6, loc=(-70, 0, 163)))
    k.add(SHELL, gen.box("DP_AntBase", 40, 30, 12, loc=(-70, 0, 143)))
    k.add(SHELL, gen.box("DP_RCS", 60, 50, 26, loc=(-60, 0, -150)), bevel=(4, 1))
    # ports S / E / W
    k.port(0, -1, (0, 0, 0), inner=180)
    for sign in (1, -1):
        k.port(1, sign, (-35, 0, 0), inner=136)
    k.faces.append("N")  # the collar face is flush on N as well
    k.ucx_prism((-25, 0, 0), 150, 310, 'X', 8)            # body (+S port)
    k.ucx_prism((175, 0, 0), 186, 50, 'X', 12)            # collar disc
    return k


def build_corridor():
    """1x1x1. Straight octagonal transit tube N-S with full ports at both ends,
    frame ribs, glazed strips on the upper diagonals, underside conduits, and smaller
    side / up / down hatches (corridor faces = all)."""
    k = Kit("SM_StationModule_Corridor_01", (1, 1, 1), CONN)
    k.add(CONN, prism("CR_Tube", 105, 360, 'X', 8))
    for x in (-140, -60, 60, 140):
        k.add(SHELL, prism(k.uid("CR_Rib"), 122, 16, 'X', 8, loc=(x, 0, 0)))
    # window strips on the two upper-diagonal faces
    ap = 105 * math.cos(math.pi / 8)
    for sgn in (1, -1):
        a = math.radians(45 * sgn)
        y, z = (ap + 1.5) * math.sin(a) * 1, (ap + 1.5) * math.cos(a)
        for x0, x1 in ((-132, -68), (-52, 52), (68, 132)):
            k.add(L_WHITE, gen.box(k.uid("CR_Window"), x1 - x0, 4, 22,
                                   loc=((x0 + x1) / 2, y, z), rot=(-a, 0, 0)))
    for y in (-55, 55):
        k.add(SHELL, prism(k.uid("CR_Conduit"), 9, 330, 'X', 6, loc=(0, y, -104)))
    k.port(0, 1, (0, 0, 0), inner=180, r=114)
    k.port(0, -1, (0, 0, 0), inner=180, r=114)
    for sign in (1, -1):
        k.port(1, sign, (0, 0, 0), inner=92, r=66, neck_r=46)
        k.port(2, sign, (0, 0, 0), inner=97, r=56, flush=True)
    k.ucx.insert(0, ('box', (0, 0, 0), (400, 200, 200), (0, 0, 0)))
    return k


def build_habitation():
    """2x2x1. Central hub drum with a horizontal hab ring (M_Station_Hab) carrying two
    rows of lit windows; spokes to the hub; 8 per-cell ports around the edge."""
    k = Kit("SM_StationModule_Habitation_01", (2, 2, 1), HAB)
    k.add(SHELL, prism("HB_Hub", 150, 300, 'Z', 16))
    k.add(SHELL, prism("HB_CapTop", 150, 40, 'Z', 16, loc=(0, 0, 170), r2=95))
    k.add(SHELL, prism("HB_CapBot", 95, 40, 'Z', 16, loc=(0, 0, -170), r2=150))
    k.add(CONN, prism("HB_TopHatch", 60, 8, 'Z', 12, loc=(0, 0, 194)))
    for z in (-90, 90):
        k.add(CONN, annulus(k.uid("HB_HubBand"), 158, 140, 18, 'Z', 16, loc=(0, 0, z)))
    # the hab ring
    k.add(HAB, annulus("HB_Ring", 320, 250, 150, 'Z', 32))
    for z in (-79, 79):
        k.add(SHELL, annulus(k.uid("HB_RingTrim"), 327, 243, 10, 'Z', 32, loc=(0, 0, z)))
    port_lines = [(0, 1, 200), (0, 1, -200), (0, -1, 200), (0, -1, -200),
                  (1, 1, 200), (1, 1, -200), (1, -1, 200), (1, -1, -200)]
    ap = 320 * math.cos(math.pi / 32)
    for i in range(32):
        aa = 2 * math.pi * i / 32  # annulus face centres (vertices sit at +pi/32 offsets)
        p = (ap * math.cos(aa), ap * math.sin(aa))
        blocked = any(s * p[ax] > 0 and abs(p[1 - ax] - off) < 62 for ax, s, off in port_lines)
        if blocked:
            continue
        for z in (30, -25):
            k.add(L_WHITE, gen.box(k.uid("HB_Window"), 4, 38, 26,
                                   loc=((ap + 1) * math.cos(aa), (ap + 1) * math.sin(aa), z),
                                   rot=(0, 0, aa)))
    # spokes (on the axes, between the port pairs)
    for i in range(4):
        a = math.radians(90 * i)
        c, s = math.cos(a), math.sin(a)
        k.add(SHELL, beam(k.uid("HB_Spoke"), (145 * c, 145 * s, 0), (255 * c, 255 * s, 0), 50, 60))
    # small dorsal radiator pair on the hub cap
    for s in (-1, 1):
        k.add(CONN, gen.box(k.uid("HB_Rad"), 110, 6, 50, loc=(0, s * 60, 175)))
    k.ports_per_cell(lambda a, sgn, off: 240)
    k.ucx.insert(0, ('prism', (0, 0, 0), 150, 380, 'Z', 12, None, (0, 0, 0)))
    for i in range(8):
        a = 2 * math.pi * i / 8
        k.ucx_box((285 * math.cos(a), 285 * math.sin(a), 0), (70, 265, 150), (0, 0, a))
    return k


def build_reactor():
    """2x2x1. Glowing core drum inside eight armour plates (glow shows through the
    slits), capped with emitter domes; four big diagonal radiator fins to the corners;
    square service frame carrying the 8 ports."""
    k = Kit("SM_StationModule_Reactor_01", (2, 2, 1), SHELL)
    k.add(L_WHITE, prism("RX_CoreGlow", 122, 330, 'Z', 16))
    for i in range(8):
        a = math.radians(22.5 + 45 * i)
        k.add(SHELL, gen.box(k.uid("RX_Armour"), 30, 62, 280,
                             loc=(150 * math.cos(a), 150 * math.sin(a), 0), rot=(0, 0, a)), bevel=(3, 1))
    for z in (-105, 105):
        k.add(CONN, annulus(k.uid("RX_Band"), 172, 128, 24, 'Z', 16, loc=(0, 0, z)))
    k.add(SHELL, prism("RX_CapTop", 172, 40, 'Z', 16, loc=(0, 0, 160), r2=110))
    k.add(SHELL, prism("RX_CapBot", 110, 40, 'Z', 16, loc=(0, 0, -160), r2=172))
    k.add(L_WHITE, prism("RX_EmitTop", 60, 16, 'Z', 16, loc=(0, 0, 188)))
    k.add(L_WHITE, prism("RX_EmitBot", 60, 16, 'Z', 16, loc=(0, 0, -188)))
    # radiator fins on the diagonals
    for i in range(4):
        a = math.radians(45 + 90 * i)
        c, s = math.cos(a), math.sin(a)
        for zi in range(6):  # slatted radiator panel
            z = -125 + zi * 50
            k.add(UTIL, beam(k.uid("RX_Slat"), (200 * c, 200 * s, z), (515 * c, 515 * s, z), 8, 32))
        for z in (-150, 150):
            k.add(UTIL, beam(k.uid("RX_FinSpar"), (200 * c, 200 * s, z), (518 * c, 518 * s, z), 24, 18))
        for t in (260, 330, 400, 470):
            k.add(CONN, gen.box(k.uid("RX_FinRib"), 10, 22, 296, loc=(t * c, t * s, 0), rot=(0, 0, a)))
        k.add(UTIL, gen.box(k.uid("RX_HeatX"), 70, 80, 230, loc=(200 * c, 200 * s, 0), rot=(0, 0, a)),
              bevel=(6, 1))
        k.ucx_beam((190 * c, 190 * s, 0), (520 * c, 520 * s, 0), 30, 318)
    # service frame + struts
    for sgn in (1, -1):
        k.add(SHELL, gen.box(k.uid("RX_FrameY"), 40, 640, 40, loc=(sgn * 300, 0, 0)), bevel=(4, 1))
        k.add(SHELL, gen.box(k.uid("RX_FrameX"), 640, 40, 40, loc=(0, sgn * 300, 0)), bevel=(4, 1))
        k.add(SHELL, beam(k.uid("RX_Strut"), (165 * sgn, 0, 0), (300 * sgn, 0, 0), 36, 36))
        k.add(SHELL, beam(k.uid("RX_Strut"), (0, 165 * sgn, 0), (0, 300 * sgn, 0), 36, 36))
    k.ports_per_cell(lambda a, sgn, off: 300)
    k.ucx.insert(0, ('prism', (0, 0, 0), 175, 396, 'Z', 12, None, (0, 0, 0)))
    for sgn in (1, -1):
        k.ucx_box((sgn * 300, 0, 0), (44, 644, 44))
        k.ucx_box((0, sgn * 300, 0), (644, 44, 44))
    return k


def build_solar_array():
    """3x1x1, faces=['W'] (UE -Y = Blender +Y). Hub + mount port ONLY on the W face;
    a boom out to a long mast carrying two 3-panel wings tilted 35 deg. Nav lights on
    the mast tips. No connectors on any other face so the restriction reads."""
    k = Kit("SM_StationModule_SolarArray_01", (3, 1, 1), SOLAR)
    tilt = math.radians(35)
    # hub on the W (Blender +Y) side
    k.add(CONN, gen.box("SA_Hub", 180, 150, 200, loc=(0, 105, 0)), bevel=(8, 2))
    for s in (-1, 1):
        k.add(SHELL, gen.box(k.uid("SA_HubRad"), 70, 110, 8, loc=(s * 60, 110, 104)))
    k.add(SHELL, prism("SA_Joint", 62, 30, 'Y', 16, loc=(0, 15, 0)))
    k.add(CONN, prism("SA_Boom", 38, 100, 'Y', 8, loc=(0, -40, 0)))
    k.add(SHELL, prism("SA_MastHub", 50, 70, 'X', 8, loc=(0, -80, 0)))
    k.add(CONN, prism("SA_Mast", 22, 1180, 'X', 8, loc=(0, -80, 0)))
    rotm = Euler((tilt, 0, 0)).to_matrix()
    wing_c = Vector((0, -80, 0))
    for side in (-1, 1):
        for j in range(3):
            cx = side * (195 + j * 160)
            c = Vector((cx, 0, 0)) + wing_c
            k.add(SOLAR, gen.box(k.uid("SA_Panel"), 150, 5, 330, loc=c, rot=(tilt, 0, 0)))
            for dz in (-165, 165):
                o = rotm @ Vector((0, 0, dz))
                k.add(CONN, gen.box(k.uid("SA_FrameH"), 152, 9, 9, loc=c + o, rot=(tilt, 0, 0)))
            for dx in (-75, 75):
                k.add(CONN, gen.box(k.uid("SA_FrameV"), 8, 9, 330, loc=c + Vector((dx, 0, 0)),
                                    rot=(tilt, 0, 0)))
            for dz in (-55, 55):
                o = rotm @ Vector((0, -3, dz))
                k.add(CONN, gen.box(k.uid("SA_Mullion"), 150, 3, 4, loc=c + o, rot=(tilt, 0, 0)))
        # wing root clamp and tip light (green +X, red -X)
        k.add(SHELL, gen.box(k.uid("SA_WingRoot"), 30, 50, 60, loc=(side * 105, -80, 0)))
        k.add(L_GREEN if side > 0 else L_RED,
              gen.box(k.uid("SA_TipLight"), 12, 14, 14, loc=(side * 592, -80, 0)))
        k.ucx_box((side * 355, -80, 0), (470, 12, 330), (tilt, 0, 0))
    k.port(1, 1, (0, 0, 0), inner=180)  # W face (UE -Y)
    k.ucx.insert(0, ('box', (0, 105, 0), (180, 150, 200), (0, 0, 0)))
    k.ucx_prism((0, -40, 0), 40, 100, 'Y', 8)
    k.ucx_box((0, -80, 0), (1184, 48, 48))
    return k


def build_fuel_depot():
    """2x2x1. Four spherical tanks on the cell centres with contrasting belts and
    red warning lights, a central cylindrical tank, pipe runs between tanks."""
    k = Kit("SM_StationModule_FuelDepot_01", (2, 2, 1), UTIL)
    for sx in (-1, 1):
        for sy in (-1, 1):
            c = (200 * sx, 200 * sy, 0)
            k.add(UTIL, gen.sphere(k.uid("FD_Tank"), 150, loc=c, verts=20))
            k.add(SHELL, annulus(k.uid("FD_Belt"), 156, 138, 20, 'Z', 20, loc=c))
            k.add(SHELL, prism(k.uid("FD_Valve"), 28, 18, 'Z', 8, loc=(c[0], c[1], 151)))
            k.add(L_RED, gen.box(k.uid("FD_Warn"), 12, 12, 12, loc=(c[0], c[1], 166)))
            k.add(SHELL, prism(k.uid("FD_Drain"), 28, 18, 'Z', 8, loc=(c[0], c[1], -151)))
            k.ucx_ico(c, 150)
    k.add(UTIL, prism("FD_Core", 95, 300, 'Z', 16))
    k.add(UTIL, prism("FD_CoreTop", 95, 40, 'Z', 16, loc=(0, 0, 170), r2=50))
    k.add(UTIL, prism("FD_CoreBot", 50, 40, 'Z', 16, loc=(0, 0, -170), r2=95))
    for z in (-80, 0, 80):
        k.add(SHELL, annulus(k.uid("FD_CoreBand"), 102, 88, 14, 'Z', 16, loc=(0, 0, z)))
    k.add(SHELL, gen.box("FD_Manifold", 60, 60, 16, loc=(0, 0, 190)))
    # pipe runs between tanks (X runs high, Y runs low) + radial braces to the core
    for s in (-1, 1):
        k.add(SHELL, prism(k.uid("FD_PipeX"), 16, 400, 'X', 8, loc=(0, 200 * s, 70)))
        k.add(SHELL, prism(k.uid("FD_PipeY"), 16, 400, 'Y', 8, loc=(200 * s, 0, -70)))
    for sx in (-1, 1):
        for sy in (-1, 1):
            d = Vector((sx, sy, 0)).normalized()
            k.add(CONN, beam(k.uid("FD_Brace"), d * 90, d * 170, 34, 34))
            k.add(SHELL, rod(k.uid("FD_Feed"), d * 90 + Vector((0, 0, 120)), d * 200 + Vector((0, 0, 120)), 10, 6))
    k.ports_per_cell(lambda a, sgn, off: 340)
    k.ucx.insert(0, ('prism', (0, 0, 0), 95, 380, 'Z', 12, None, (0, 0, 0)))
    return k


def build_processing():
    """2x2x1. Heavy refinery base block, two open ore hoppers with ore mounds and legs,
    inclined conveyors feeding a smelter drum with a glowing furnace band and stacks."""
    k = Kit("SM_StationModule_Processing_01", (2, 2, 1), SHELL)
    k.add(SHELL, gen.box("PR_Base", 700, 700, 170, loc=(0, 0, -105)), bevel=(10, 2))
    for s in (-1, 1):
        k.add(UTIL, gen.box(k.uid("PR_Skirt"), 720, 30, 60, loc=(0, s * 345, -160)), bevel=(4, 1))
    hoppers = [(-170, 170), (-170, -170)]
    for hx, hy in hoppers:
        k.add(UTIL, prism(k.uid("PR_Hopper"), 60, 150, 'Z', 4, loc=(hx, hy, 55), r2=170,
                          phase=math.pi / 4))
        k.add(CONN, annulus(k.uid("PR_HopRim"), 182, 160, 12, 'Z', 4, loc=(hx, hy, 136)))
        k.add(UTIL, gen.rock(k.uid("PR_Ore"), 95, loc=(hx, hy, 128), sub=2, scale_xyz=(1.15, 1.15, 0.3)))
        for dx in (-1, 1):
            for dy in (-1, 1):
                k.add(CONN, gen.box(k.uid("PR_Leg"), 16, 16, 150, loc=(hx + dx * 112, hy + dy * 112, 55)))
        k.ucx_prism((hx, hy, 61), 60, 162, 'Z', 4, r2=182)
    # smelter drum
    sx = 190
    k.add(SHELL, prism("PR_Smelter", 120, 208, 'Z', 12, loc=(sx, 0, 84)))
    for z in (40, 110):
        k.add(L_AMBER, annulus(k.uid("PR_Furnace"), 123, 100, 14, 'Z', 12, loc=(sx, 0, z)))
    k.add(SHELL, prism("PR_SmeltCap", 120, 8, 'Z', 12, loc=(sx, 0, 192), r2=80))
    k.add(L_AMBER, prism("PR_Mouth", 50, 4, 'Z', 12, loc=(sx, 0, 197)))
    k.ucx_prism((sx, 0, 88), 124, 216, 'Z', 12)
    for s in (-1, 1):
        k.add(CONN, prism(k.uid("PR_Stack"), 24, 216, 'Z', 8, loc=(300, s * 125, 88)))
        k.add(SHELL, annulus(k.uid("PR_StackRing"), 30, 20, 10, 'Z', 8, loc=(300, s * 125, 190)))
    # conveyors hopper -> smelter
    for s in (-1, 1):
        p0 = Vector((-60, s * 170, 10))
        p1 = Vector((130, s * 60, 185))
        d = (p1 - p0).normalized()
        perp = d.cross(Vector((0, 0, 1))).normalized()
        k.add(UTIL, beam(k.uid("PR_Belt"), p0, p1, 50, 8))
        for sp in (-1, 1):
            k.add(CONN, beam(k.uid("PR_Rail"), p0 + perp * 30 * sp + Vector((0, 0, 6)),
                             p1 + perp * 30 * sp + Vector((0, 0, 6)), 8, 16))
        for t in (0.25, 0.7):
            m = p0.lerp(p1, t)
            k.add(CONN, gen.box(k.uid("PR_BeltLeg"), 14, 14, m.z + 20, loc=(m.x, m.y, (m.z - 20) / 2)))
        k.ucx_beam(p0, p1, 66, 24)
    # deck walkway greebles
    for gx, gy in ((60, 250), (60, -250), (-330 + 40, 0)):
        k.add(UTIL, gen.box(k.uid("PR_Vent"), 70, 50, 12, loc=(gx, gy, -14)))
    k.ports_per_cell(lambda a, sgn, off: 350, pylon_z0=-20)
    k.ucx.insert(0, ('box', (0, 0, -105), (700, 700, 170), (0, 0, 0)))
    return k


def build_fabrication():
    """2x2x1. Low deck with rim walls (ports on the rim), a corner-post gantry with a
    bridge crane holding a hull plate, three robot arms working a hull section on a
    cradle, amber weld glow at the tool heads."""
    k = Kit("SM_StationModule_Fabrication_01", (2, 2, 1), SHELL)
    k.add(SHELL, gen.box("FB_Deck", 720, 720, 80, loc=(0, 0, -150)), bevel=(10, 2))
    for s in (-1, 1):  # low kerb around the bay floor
        k.add(UTIL, gen.box(k.uid("FB_KerbX"), 20, 700, 16, loc=(s * 350, 0, -102)))
        k.add(UTIL, gen.box(k.uid("FB_KerbY"), 700, 20, 16, loc=(0, s * 350, -102)))
    # gantry
    for sx in (-1, 1):
        for sy in (-1, 1):
            k.add(UTIL, gen.box(k.uid("FB_Post"), 44, 44, 300, loc=(sx * 300, sy * 300, 40)), bevel=(3, 1))
            k.ucx_box((sx * 300, sy * 300, 40), (44, 44, 300))
    for sy in (-1, 1):
        k.add(UTIL, gen.box(k.uid("FB_Rail"), 644, 40, 30, loc=(0, sy * 300, 178)), bevel=(3, 1))
        k.ucx_box((0, sy * 300, 178), (644, 40, 30))
    bx = 90
    k.add(UTIL, gen.box("FB_Bridge", 44, 600, 36, loc=(bx, 0, 158)), bevel=(3, 1))
    k.add(CONN, gen.box("FB_Trolley", 76, 76, 40, loc=(bx, -170, 120)), bevel=(4, 1))
    for dx in (-22, 22):
        k.add(CONN, gen.box(k.uid("FB_Cable"), 4, 4, 70, loc=(bx + dx, -170, 65)))
    k.add(CONN, gen.box("FB_Spreader", 130, 40, 12, loc=(bx, -170, 30)))
    k.add(SHELL, gen.box("FB_Plate", 170, 120, 14, loc=(bx, -170, 10), rot=(math.radians(8), 0, 0)),
          bevel=(4, 1))
    k.ucx_box((bx, 0, 158), (44, 600, 36))
    # cradle + hull section under construction
    k.add(CONN, gen.box("FB_Cradle", 240, 120, 16, loc=(-20, 70, -102)))
    for dx in (-90, 90):
        k.add(CONN, gen.box(k.uid("FB_CradleArm"), 20, 150, 50, loc=(-20 + dx, 70, -80)))
    k.add(SHELL, gen.box("FB_Work", 260, 150, 100, loc=(-20, 70, -40)), bevel=(24, 3))
    for dx in (-80, 0, 80):
        k.add(UTIL, gen.box(k.uid("FB_WorkRib"), 12, 156, 104, loc=(-20 + dx, 70, -40)))
    k.ucx_box((-20, 70, -60), (264, 156, 140))
    # robot arms: (base xy, wrist target)
    arms = [((-240, 70), (-150, 70, 30)), ((200, 70), (110, 70, 30)), ((-20, -200), (-20, -5, 30))]
    for (ax_, ay_), wrist in arms:
        k.add(CONN, prism(k.uid("FB_ArmBase"), 42, 24, 'Z', 12, loc=(ax_, ay_, -98)))
        k.add(UTIL, gen.box(k.uid("FB_ArmTurret"), 54, 54, 46, loc=(ax_, ay_, -63)), bevel=(5, 1))
        sh = Vector((ax_, ay_, -45))
        w = Vector(wrist)
        el = sh.lerp(w, 0.35) + Vector((0, 0, 150))
        k.add(UTIL, beam(k.uid("FB_UpperArm"), sh, el, 46, 46, extend=24))
        k.add(UTIL, beam(k.uid("FB_Forearm"), el, w, 36, 36, extend=18))
        axis_side = 'Y' if abs(ax_ + 20) > 1 else 'X'
        k.add(CONN, prism(k.uid("FB_Elbow"), 32, 56, axis_side, 10, loc=el))
        k.add(CONN, gen.box(k.uid("FB_Tool"), 34, 34, 30, loc=w), bevel=(3, 1))
        k.add(L_AMBER, gen.box(k.uid("FB_Weld"), 18, 18, 14, loc=w - Vector((0, 0, 22))))
        k.ucx_beam(sh, el, 50, 50)
    k.ports_per_cell(lambda a, sgn, off: 322, pylon_z0=-110)
    k.ucx.insert(0, ('box', (0, 0, -150), (720, 720, 80), (0, 0, 0)))
    return k


def build_barracks():
    """2x2x1. Armoured hab block: M_Station_Hab body wrapped in inward-leaning Defence
    armour plates (each pierced by a port), heavy corner pillars, armoured roof with a
    raised command block, sensor mast and two sensor blisters, a row of lit slit windows."""
    k = Kit("SM_StationModule_Barracks_01", (2, 2, 1), DEF)
    k.add(HAB, gen.box("BK_Body", 560, 560, 300, loc=(0, 0, -20)), bevel=(10, 2))
    for f in range(4):
        fa = math.radians(90 * f)
        rz = Euler((0, 0, fa)).to_matrix()
        for off in (-200, 200):
            c = rz @ Vector((292, off, -60))
            k.add(DEF, gen.box(k.uid("BK_Plate"), 18, 230, 210, loc=c,
                               rot=(0, math.radians(-10), fa)), bevel=(4, 1))
        c = rz @ Vector((290, 0, -30))
        k.add(DEF, gen.box(k.uid("BK_Rib"), 22, 60, 280, loc=c, rot=(0, 0, fa)), bevel=(4, 1))
        for i in range(8):
            y = -245 + i * 70
            c = rz @ Vector((281.5, y, 92))
            k.add(L_WHITE, gen.box(k.uid("BK_Slit"), 4, 40, 12, loc=c, rot=(0, 0, fa)))
    for sx in (-1, 1):
        for sy in (-1, 1):
            k.add(DEF, prism(k.uid("BK_Pillar"), 62, 344, 'Z', 8, loc=(sx * 288, sy * 288, -10)))
            k.add(CONN, prism(k.uid("BK_PillarCap"), 48, 14, 'Z', 8, loc=(sx * 288, sy * 288, 169)))
            k.ucx_prism((sx * 288, sy * 288, -10), 62, 344, 'Z', 8)
    k.add(DEF, gen.box("BK_Roof", 600, 600, 30, loc=(0, 0, 145)), bevel=(8, 2))
    k.add(DEF, gen.box("BK_Command", 250, 250, 26, loc=(0, 0, 172)), bevel=(6, 1))
    k.add(CONN, prism("BK_Mast", 8, 36, 'Z', 6, loc=(90, 90, 180)))
    k.add(L_WHITE, gen.box("BK_MastLight", 12, 12, 12, loc=(90, 90, 193)))
    for sy in (-1, 1):
        k.add(DEF, gen.sphere(k.uid("BK_Blister"), 38, loc=(-190, sy * 190, 158), verts=12))
    k.ports_per_cell(lambda a, sgn, off: 280)
    k.ucx.insert(0, ('box', (0, 0, -10), (620, 620, 340), (0, 0, 0)))
    k.ucx_box((0, 0, 172), (250, 250, 26))
    return k


BUILDERS = {
    "DockingPort": build_docking_port,
    "Corridor": build_corridor,
    "Habitation": build_habitation,
    "Reactor": build_reactor,
    "SolarArray": build_solar_array,
    "FuelDepot": build_fuel_depot,
    "Processing": build_processing,
    "Fabrication": build_fabrication,
    "Barracks": build_barracks,
}


# ----------------------------------------------------------------------------
# Preview rendering
# ----------------------------------------------------------------------------
def _preview_env():
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_EEVEE'
    scene.render.resolution_x = 1000
    scene.render.resolution_y = 750
    scene.render.resolution_percentage = 100
    try:
        scene.eevee.taa_render_samples = 32
    except Exception:
        pass
    world = scene.world or bpy.data.worlds.new("PreviewWorld")
    scene.world = world
    try:
        world.use_nodes = True
    except Exception:
        pass
    if world.node_tree:
        bg = world.node_tree.nodes.get('Background')
        if bg:
            bg.inputs[0].default_value = (0.10, 0.11, 0.14, 1)
            bg.inputs[1].default_value = 1.0
    for nm, energy, rot, col in (("PKey", 4.0, (math.radians(50), 0, math.radians(35)), (1, 0.97, 0.92)),
                                 ("PFill", 1.0, (math.radians(115), 0, math.radians(215)), (0.6, 0.7, 1.0))):
        ld = bpy.data.lights.new(nm, 'SUN')
        ld.energy = energy
        ld.color = col
        lo = bpy.data.objects.new(nm, ld)
        lo.rotation_euler = rot
        scene.collection.objects.link(lo)


def _preview_materials():
    for mat in bpy.data.materials:
        if not mat.node_tree:
            continue
        for n in mat.node_tree.nodes:
            if n.type == 'MIX_SHADER':
                n.inputs['Fac'].default_value = 0.12
            if n.type == 'BSDF_PRINCIPLED':
                # preview only: the kit's 0.8-0.95 metallic reads pure black against an
                # empty studio world, so drop it and let the key light show the forms
                for link in list(mat.node_tree.links):
                    if link.to_node == n and link.to_socket.name == 'Metallic':
                        mat.node_tree.links.remove(link)
                n.inputs['Metallic'].default_value = 0.25


def render_previews(name, size, tag=""):
    """Two 3/4 views (NE-top and SW-top in UE face terms) with the footprint wireframe."""
    os.makedirs(PREVIEW_DIR, exist_ok=True)
    scene = bpy.context.scene
    _preview_env()
    _preview_materials()
    # footprint wireframe
    fp = gen.box("PreviewFootprint", *size, loc=(0, 0, 0))
    wm = fp.modifiers.new("wf", 'WIREFRAME')
    wm.thickness = 3
    fmat = gen.emissive_mat("PreviewFootprintMat", (0.1, 0.6, 1.0), strength=1.5)
    fp.data.materials.append(fmat)
    for o in bpy.data.objects:
        if o.name.startswith("UCX_"):
            o.hide_render = True
    radius = 0.5 * Vector(size).length
    cam_d = bpy.data.cameras.new("PCam")
    cam_d.lens = 40
    cam_d.clip_start = 10
    cam_d.clip_end = 100000
    cam = bpy.data.objects.new("PCam", cam_d)
    scene.collection.objects.link(cam)
    scene.camera = cam
    outs = []
    for view, d in (("A", Vector((1.0, -1.15, 0.8))), ("B", Vector((-1.1, 1.0, 0.65)))):
        loc = d.normalized() * radius * 2.55
        cam.location = loc
        cam.rotation_euler = (-loc).to_track_quat('-Z', 'Y').to_euler()
        out = os.path.join(PREVIEW_DIR, f"{name}{tag}_{view}.png")
        scene.render.filepath = out
        bpy.ops.render.render(write_still=True)
        outs.append(out)
    return outs


def review_existing():
    results = []
    for nm, size in (("SM_StationModule_DockingBay_01", (1200, 800, 400)),
                     ("SM_StationModule_CargoBay_01", (800, 800, 400)),
                     ("SM_StationModule_Market_01", (800, 800, 400))):
        gen.setup_scene()
        bpy.ops.import_scene.fbx(filepath=os.path.join(OUT_DIR, nm + ".fbx"))
        for mat in list(bpy.data.materials):
            base = mat.name.split(".")[0]
            gen.wire_pbr_material(mat, gen.resolve_texset(base))
        results.append(render_previews(nm, size, tag="_review"))
    return results


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    dry = "--dry-run" in argv
    preview = "--no-preview" not in argv and not dry
    if "--review" in argv:
        for r in review_existing():
            print("REVIEW_PNG", r)
        return
    only = None
    if "--only" in argv:
        only = argv[argv.index("--only") + 1].split(",")
    report = []
    for key, fn in BUILDERS.items():
        if only and key not in only:
            continue
        gen.setup_scene()
        print(f"Building {key}...")
        k = fn()
        main_obj, ucx, info = finalize(k, dry_run=dry)
        if preview:
            info['previews'] = render_previews(k.name, k.size)
        report.append(info)
    print("=" * 60)
    for info in report:
        print("REPORT", info)


if __name__ == "__main__":
    main()
