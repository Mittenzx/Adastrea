"""
Adastrea Station Module Meshes -- Research Labs + Defence (Blender 5.x, headless)
================================================================================
Dedicated meshes for the 14 research-lab module classes and the 2 defence module
classes that currently render one of the 4 shared shells
(SM_StationModule_Shell_Standard / _ConnectorThin), per the
ASpaceStationModule constructor.

  ScienceLabModule        -> SM_StationModule_ScienceLab_01        [2,2,1]
  ResearchLabModule       -> SM_StationModule_ResearchLab_01       [2,2,1] (abstract base; generic)
  PhysicsLabModule        -> SM_StationModule_PhysicsLab_01        [2,2,1]
  MaterialsLabModule      -> SM_StationModule_MaterialsLab_01      [2,2,1]
  ElectronicsLabModule    -> SM_StationModule_ElectronicsLab_01    [2,2,1]
  WeaponsLabModule        -> SM_StationModule_WeaponsLab_01        [2,2,1]
  BiologyLabModule        -> SM_StationModule_BiologyLab_01        [2,2,1]
  ProjectileWeaponsLab    -> SM_StationModule_ProjectileWeaponsLab_01 (Weapons base + railgun)
  BeamWeaponsLab          -> SM_StationModule_BeamWeaponsLab_01       (Weapons base + beam emitter)
  IonPropulsionLab        -> SM_StationModule_IonPropulsionLab_01     (Physics base + ion thrusters)
  GravMaterialsLab        -> SM_StationModule_GravMaterialsLab_01     (Materials base + grav gimbal rings)
  EncryptionLab           -> SM_StationModule_EncryptionLab_01        (Electronics base + dish array)
  OptronicsLab            -> SM_StationModule_OptronicsLab_01         (Electronics base + lens array)
  CyberneticsLab          -> SM_StationModule_CyberneticsLab_01       (Biology base + cyber pods)
  TurretModule            -> SM_StationModule_Turret_01     [1,1,1] armoured base
                           + SM_StationModule_TurretHead_01          rotating head (own FBX)
  ShieldGeneratorModule   -> SM_StationModule_ShieldGenerator_01     [2,2,1]

Conventions reused verbatim from Tools/generate_station_module_meshes.py /
generate_adastrea_assets.py: same primitive helpers (imported as `gen`), 400 cm per
grid cell, 1 Blender unit = 1 cm = 1 Unreal unit, triplanar gen.smart_uv, identical
FBX export settings (FBX_SCALE_ALL, apply_unit_scale, Face smoothing, -Y fwd / Z up),
authored UCX_<Mesh>_NN box collision exported in the same FBX.

Differences (deliberate):
  * Multi-slot materials (Shell / Connector / Utility / Accent, or Defence / ...),
    one slot per family material, so the integrator can bind the existing
    M_StationModule_* materials by slot name. M_StationModule_Accent is a NEW
    emissive slot intended for per-domain colour Material Instances.
  * Pivot is forced to the footprint centre (0,0,0) instead of centre-of-volume,
    because these props are asymmetric and the grid system expects the pivot at
    the cell-block centre (the shared shells are symmetric, so their COV == centre).
  * Every vertex is kept inside the footprint envelope (size*400 cm, centred), so
    stacked / adjacent modules never interpenetrate. Side connection collars are
    at z=0 on flush bulkheads, matching the shared shells' collar height.
  * SOCKET_ empties are exported with the mesh (object_types MESH+EMPTY).

Usage:
  "C:\\Program Files\\Blender Foundation\\Blender 5.2\\blender.exe" -b --python Tools/generate_station_lab_defence_modules.py
  optional:  -- --only PhysicsLab Turret     (build subset)
             -- --no-render                  (skip preview renders)
Outputs:
  Assets/FBX/generated/SM_StationModule_<Name>_01.fbx
  Assets/FBX/generated/station_modules/SM_StationModule_<Name>_01.png
  Assets/FBX/generated/station_modules/SM_StationModule_LabDefence_ContactSheet.png
"""
import os
import sys
import math

import bpy
import bmesh
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen  # noqa: E402

GRID = 400.0
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(REPO, "Assets", "FBX", "generated")
PREVIEW_DIR = os.path.join(OUT_DIR, "station_modules")
gen.BASE = OUT_DIR
gen.TEXDIR = os.path.join(OUT_DIR, "Textures")

MAT_SHELL = "M_StationModule_Shell"
MAT_CONN = "M_StationModule_Connector"
MAT_UTIL = "M_StationModule_Utility"
MAT_DEF = "M_StationModule_Defence"
MAT_ACCENT = "M_StationModule_Accent"

TRI_BUDGET = 8000

# per-domain accent colours -- only used for the preview renders (and as a hint
# for the integrator's per-domain Material Instances of M_StationModule_Accent)
DOMAIN_ACCENT = {
    "ScienceLab": (0.35, 0.80, 1.00), "ResearchLab": (0.60, 0.85, 1.00),
    "PhysicsLab": (0.55, 0.45, 1.00), "IonPropulsionLab": (0.40, 0.55, 1.00),
    "MaterialsLab": (1.00, 0.50, 0.15), "GravMaterialsLab": (0.80, 0.30, 1.00),
    "ElectronicsLab": (0.20, 1.00, 0.70), "EncryptionLab": (0.20, 1.00, 0.35),
    "OptronicsLab": (1.00, 0.95, 0.55),
    "WeaponsLab": (1.00, 0.25, 0.15), "ProjectileWeaponsLab": (1.00, 0.55, 0.10),
    "BeamWeaponsLab": (1.00, 0.20, 0.50),
    "BiologyLab": (0.45, 1.00, 0.30), "CyberneticsLab": (0.20, 0.95, 0.90),
    "Turret": (1.00, 0.18, 0.12), "ShieldGenerator": (0.30, 0.70, 1.00),
}


# ============================================================================
# Kit: collects parts with a material each, joins, UVs, exports with UCX + sockets
# ============================================================================
def get_mat(name):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
        mat.use_nodes = True
        if name == MAT_ACCENT:
            # neutral bright emissive; per-domain colour comes from UE instances
            bsdf = mat.node_tree.nodes.get("Principled BSDF")
            if bsdf is not None:
                bsdf.inputs["Base Color"].default_value = (0.8, 0.9, 1.0, 1.0)
                bsdf.inputs["Emission Color"].default_value = (0.8, 0.9, 1.0, 1.0)
                bsdf.inputs["Emission Strength"].default_value = 4.0
                bsdf.inputs["Roughness"].default_value = 0.2
        else:
            try:
                gen.wire_pbr_material(mat, gen.resolve_texset(name), emissive_strength=2.6)
            except Exception as e:
                print(f"  [warn] material wire fail {name}: {e}")
    return mat


class Kit:
    def __init__(self, name, cells):
        self.name = name
        self.cells = cells
        self.L, self.W, self.H = (c * GRID for c in cells)
        self.parts = []
        self.ucx = []       # (cx, cy, cz, sx, sy, sz)
        self.sockets = []   # (name, (x,y,z), (rx,ry,rz))

    def add(self, ob, mat):
        me = ob.data
        me.materials.clear()
        me.materials.append(get_mat(mat))
        self.parts.append(ob)
        return ob

    # primitive shortcuts -------------------------------------------------
    def box(self, mat, sx, sy, sz, loc, rot=(0, 0, 0), bev=0.0, segs=1):
        ob = gen.box(f"{self.name}_p{len(self.parts)}", sx, sy, sz, loc=loc, rot=rot)
        if bev > 0:
            gen.bevel(ob, bev, segs)
        return self.add(ob, mat)

    def cyl(self, mat, r, h, loc, rot=(0, 0, 0), verts=16, bev=0.0):
        ob = gen.cyl(f"{self.name}_p{len(self.parts)}", r, h, loc=loc, rot=rot, verts=verts)
        if bev > 0:
            gen.bevel(ob, bev, 1)
        return self.add(ob, mat)

    def cone(self, mat, r1, r2, h, loc, rot=(0, 0, 0), verts=16):
        bpy.ops.mesh.primitive_cone_add(radius1=r1, radius2=r2, depth=h, vertices=verts,
                                        location=loc, rotation=rot)
        ob = bpy.context.active_object
        ob.name = f"{self.name}_p{len(self.parts)}"
        return self.add(ob, mat)

    def dish(self, mat, r_top, r_bot, h, loc, rot=(0, 0, 0), verts=24, thick=6):
        """Open bowl (uncapped cone + solidify) so the concave side is real
        geometry, not a backface -- reads as a dish from above in UE."""
        bpy.ops.mesh.primitive_cone_add(radius1=r_bot, radius2=r_top, depth=h, vertices=verts,
                                        end_fill_type='NOTHING', location=loc, rotation=rot)
        ob = bpy.context.active_object
        ob.name = f"{self.name}_p{len(self.parts)}"
        s = ob.modifiers.new("sol", 'SOLIDIFY')
        s.thickness = thick
        s.offset = 0
        return self.add(ob, mat)

    def torus(self, mat, R, r, loc, rot=(0, 0, 0), maj=24, minr=8):
        ob = gen.torus(f"{self.name}_p{len(self.parts)}", R, r, loc=loc, rot=rot,
                       maj=maj, minr=minr)
        return self.add(ob, mat)

    def sphere(self, mat, r, loc, segs=16, scale=(1, 1, 1)):
        bpy.ops.mesh.primitive_uv_sphere_add(radius=r, segments=segs, ring_count=max(6, segs // 2),
                                             location=loc)
        ob = bpy.context.active_object
        ob.name = f"{self.name}_p{len(self.parts)}"
        if scale != (1, 1, 1):
            ob.scale = scale
            gen.sel_activate(ob)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        return self.add(ob, mat)

    def hemi(self, mat, r, loc, segs=16, zscale=1.0):
        """Dome: upper half of a UV sphere sitting on loc (open bottom)."""
        ob = self.sphere(mat, r, loc, segs=segs, scale=(1, 1, zscale))
        bm = bmesh.new()
        bm.from_mesh(ob.data)
        kill = [v for v in bm.verts if v.co.z < -0.01]
        bmesh.ops.delete(bm, geom=kill, context='VERTS')
        bm.to_mesh(ob.data)
        bm.free()
        return ob

    def strut(self, mat, p0, p1, r=6, verts=8):
        """Cylinder spanning two points (braces, tripod legs)."""
        a, b = Vector(p0), Vector(p1)
        d = b - a
        ob = gen.cyl(f"{self.name}_p{len(self.parts)}", r, d.length, loc=(a + b) / 2, verts=verts)
        ob.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
        return self.add(ob, mat)

    def socket(self, name, loc, rot=(0, 0, 0)):
        self.sockets.append((name, loc, rot))

    def ucx_box(self, cx, cy, cz, sx, sy, sz):
        self.ucx.append((cx, cy, cz, sx, sy, sz))

    # finalize -----------------------------------------------------------
    def finalize(self, outname, envelope=True, pivot=(0, 0, 0)):
        # Apply each part's own modifiers BEFORE joining: bpy join() keeps only
        # the active object's modifier stack, which would otherwise re-bevel the
        # whole joined mesh with the first part's bevel (tri blow-up) and drop
        # every other part's bevel.
        for p in self.parts:
            if p.modifiers:
                gen.apply_mods(p)
        joined = gen.join(self.parts, outname + "_Geo")
        gen.apply_mods(joined)
        gen.clean_mesh(joined)
        gen.sel_activate(joined)
        # bake any rotation inherited from the first joined part (e.g. a rotated
        # cylinder) so the FBX node carries an identity rotation/scale
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
        bpy.context.scene.cursor.location = pivot
        bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
        bpy.context.scene.cursor.location = (0, 0, 0)
        gen.smart_uv(joined)  # world-space triplanar, evaluated before re-centring
        # move the object to the world origin so the FBX node translation is
        # zero -- UE bakes node transforms into vertices on import, so the pivot
        # must be expressed purely by the vertex positions
        joined.location = (0, 0, 0)
        joined.name = outname
        joined.data.name = outname

        # envelope check -- every vertex must stay within the grid footprint
        me = joined.data
        mn = Vector((1e9,) * 3)
        mx = Vector((-1e9,) * 3)
        for v in me.vertices:
            w = v.co + Vector(pivot)  # object is at origin with identity rot/scale
            for i in range(3):
                mn[i] = min(mn[i], w[i])
                mx[i] = max(mx[i], w[i])
        half = Vector((self.L / 2, self.W / 2, self.H / 2))
        over = [max(mx[i] - half[i], -half[i] - mn[i]) for i in range(3)]
        if envelope and max(over) > 0.5:
            print(f"  [ENVELOPE WARN] {outname} exceeds footprint by {over}")

        me.calc_loop_triangles()
        tris = len(me.loop_triangles)
        slots = [m.name for m in me.materials]

        ucx_objs = []
        for i, (cx, cy, cz, sx, sy, sz) in enumerate(self.ucx):
            u = gen.box(f"UCX_{outname}_{i:02d}", sx, sy, sz,
                        loc=(cx - pivot[0], cy - pivot[1], cz - pivot[2]))
            u.display_type = 'WIRE'
            u.data.materials.clear()
            ucx_objs.append(u)

        sock_objs = []
        for sname, loc, rot in self.sockets:
            e = bpy.data.objects.new(f"SOCKET_{sname}", None)
            e.empty_display_type = 'ARROWS'
            e.empty_display_size = 40
            bpy.context.scene.collection.objects.link(e)
            e.location = Vector(loc) - Vector(pivot)
            e.rotation_euler = rot
            e.parent = joined
            sock_objs.append(e)

        gen.sel_activate(joined)
        for o in ucx_objs + sock_objs:
            o.select_set(True)
        out = os.path.join(OUT_DIR, outname + ".fbx")
        # Geometry values are centimetres, so declare 1 BU = 1 cm; with the default
        # scale_length=1.0 UE imports every module 100x oversized (see
        # generate_station_module_meshes_b.py's export for the same fix).
        bpy.context.scene.unit_settings.system = 'METRIC'
        bpy.context.scene.unit_settings.scale_length = 0.01
        bpy.ops.export_scene.fbx(
            filepath=out, use_selection=True, object_types={'MESH', 'EMPTY'},
            apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
            mesh_smooth_type='FACE',
            axis_forward='-Y', axis_up='Z',
            # Bake the -Y-fwd axis conversion (a 180deg Z turn) into vertex and
            # child positions instead of writing it as Lcl Rotation Z=180 on each
            # root node. Unbaked, UE folded that parent rotation into every
            # SOCKET_ empty (imported yaw 180 / roll 180, Integrator 22:15Z).
            # Baked, every Model node in the FBX has identity rotation, so sockets
            # import at Rotator(0,0,0). Mesh result in UE is unchanged: the importer
            # uses transform_vertex_to_absolute (default), which was already baking
            # that root rotation into the vertices.
            bake_space_transform=True,
        )
        info = dict(fbx=out, tris=tris, verts=len(me.vertices), slots=slots,
                    ucx=len(ucx_objs), sockets=[s[0] for s in self.sockets],
                    bounds=(tuple(round(c, 1) for c in mn), tuple(round(c, 1) for c in mx)),
                    footprint=self.cells)
        flag = "" if tris <= TRI_BUDGET else "  ** OVER BUDGET **"
        print(f"  exported {outname}: tris={tris}{flag} slots={slots} ucx={len(ucx_objs)} "
              f"sockets={info['sockets']} bounds={info['bounds']}")
        return info


# ============================================================================
# Shared lab base kit (2x2x1 = 800 x 800 x 400 cm, pivot at centre)
#   z -200..-10  : pressurised lab hull (platform), observation window bands
#   z -10        : equipment deck
#   corners      : 4 structural towers, full height (flush Up/Down contact)
#   N/S/E/W      : flush connection bulkheads with docking collars at z=0
#                  (same collar height/radius as the shared shells)
#   one tower    : sensor mast + small dish, stays inside the envelope
#   deck         : two socket pads, the domain prop lives above the deck
# ============================================================================
DECK_Z = -10.0
TOP_Z = 200.0


def lab_base(k, variant="std"):
    L, W, H = k.L, k.W, k.H
    hz = -H / 2
    # --- lab hull / platform
    plat_h = DECK_Z - hz  # 190
    k.box(MAT_SHELL, L - 16, W - 16, plat_h, (0, 0, hz + plat_h / 2), bev=14, segs=2)
    # lower belly plating (slightly inset) + hull seam bands
    k.box(MAT_CONN, L - 60, W - 60, 12, (0, 0, hz + 6))
    k.box(MAT_CONN, L - 4, W - 4, 10, (0, 0, DECK_Z - 26))  # rub strip / seam band

    # --- observation window bands on all four walls (Accent = lit glazing)
    for axis in (0, 1):
        for sign in (-1, 1):
            for i in (-1, 1):
                off = i * 215
                if axis == 0:
                    loc = (sign * (L / 2 - 8), off, hz + 95)
                    k.box(MAT_ACCENT, 6, 150, 34, loc)
                    k.box(MAT_CONN, 4, 166, 50, (sign * (L / 2 - 10), off, hz + 95))
                else:
                    loc = (off, sign * (W / 2 - 8), hz + 95)
                    k.box(MAT_ACCENT, 150, 6, 34, loc)
                    k.box(MAT_CONN, 166, 4, 50, (off, sign * (W / 2 - 10), hz + 95))

    # --- corner towers (full height, flush with the envelope corners)
    tw = 70
    for sx in (-1, 1):
        for sy in (-1, 1):
            cx, cy = sx * (L / 2 - tw / 2), sy * (W / 2 - tw / 2)
            k.box(MAT_CONN, tw, tw, H, (cx, cy, 0), bev=6, segs=1)
            # tower cap light
            k.box(MAT_ACCENT, tw * 0.5, tw * 0.5, 6, (cx, cy, TOP_Z - 3))
            # diagonal gusset strut from the tower down onto the deck
            k.strut(MAT_CONN, (cx - sx * 30, cy - sy * 30, DECK_Z + 120),
                    (cx - sx * 110, cy - sy * 110, DECK_Z + 2), r=7)

    # --- connection bulkheads + collars on N/S/E/W (flush with boundary)
    for axis in (0, 1):
        for sign in (-1, 1):
            if axis == 0:
                k.box(MAT_CONN, 34, 290, 330, (sign * (L / 2 - 17), 0, -35), bev=8, segs=1)
                k.torus(MAT_CONN, 110, 8, (sign * (L / 2 - 8), 0, 0),
                        rot=(0, math.radians(90), 0), maj=20, minr=6)
                k.cyl(MAT_SHELL, 96, 10, (sign * (L / 2 - 6), 0, 0),
                      rot=(0, math.radians(90), 0), verts=16)
                k.box(MAT_ACCENT, 6, 60, 8, (sign * (L / 2 - 3), 0, 122))
            else:
                k.box(MAT_CONN, 290, 34, 330, (0, sign * (W / 2 - 17), -35), bev=8, segs=1)
                k.torus(MAT_CONN, 110, 8, (0, sign * (W / 2 - 8), 0),
                        rot=(math.radians(90), 0, 0), maj=20, minr=6)
                k.cyl(MAT_SHELL, 96, 10, (0, sign * (W / 2 - 6), 0),
                      rot=(math.radians(90), 0, 0), verts=16)
                k.box(MAT_ACCENT, 60, 6, 8, (0, sign * (W / 2 - 3), 122))

    # --- sensor mast beside the (-X,+Y) tower: lattice mast from the deck,
    # open dish + feed near the envelope top, tied back to the tower
    mx_, my_ = -(L / 2 - 120), (W / 2 - 120)
    k.cyl(MAT_CONN, 26, 16, (mx_, my_, DECK_Z + 8), verts=8)
    k.cyl(MAT_CONN, 7, 150, (mx_, my_, DECK_Z + 85), verts=8)
    k.strut(MAT_CONN, (mx_, my_, DECK_Z + 110), (-(L / 2 - 70), my_, DECK_Z + 150), r=4)
    k.strut(MAT_CONN, (mx_, my_, DECK_Z + 110), (mx_, (W / 2 - 70), DECK_Z + 150), r=4)
    k.dish(MAT_SHELL, 44, 8, 16, (mx_, my_, TOP_Z - 38),
           rot=(math.radians(-40), 0, math.radians(-135)), verts=16, thick=3)
    k.cyl(MAT_ACCENT, 4, 22, (mx_, my_, TOP_Z - 22), verts=6)
    k.socket("SensorMast", (mx_, my_, TOP_Z - 12))

    # --- deck details: two socket pads + deck hatch + vents
    for i, (px, py) in enumerate(((-140, -140), (140, 140))):
        k.cyl(MAT_CONN, 70, 6, (px, py, DECK_Z + 3), verts=12)
    k.box(MAT_CONN, 90, 60, 6, (200, -230, DECK_Z + 3))
    k.box(MAT_ACCENT, 70, 6, 3, (200, -205, DECK_Z + 7))
    k.socket("DomainProp", (0, 0, DECK_Z))
    k.socket("DeckPadA", (-140, -140, DECK_Z + 6))
    k.socket("DeckPadB", (140, 140, DECK_Z + 6))

    # --- collision: hull platform + 4 towers
    k.ucx_box(0, 0, hz + plat_h / 2, L, W, plat_h)
    for sx in (-1, 1):
        for sy in (-1, 1):
            k.ucx_box(sx * (L / 2 - tw / 2), sy * (W / 2 - tw / 2), 0, tw, tw, H)


def lab_pod(k, cx, cy, sx, sy, sz, mat=MAT_SHELL, windows=True):
    """A small pressurised lab pod standing on the deck (shared kit piece)."""
    k.box(mat, sx, sy, sz, (cx, cy, DECK_Z + sz / 2), bev=10, segs=2)
    if windows:
        k.box(MAT_ACCENT, sx + 4, sy * 0.7, 16, (cx, cy, DECK_Z + sz * 0.62))
    k.box(MAT_CONN, sx * 0.6, sy * 0.6, 8, (cx, cy, DECK_Z + sz + 4))
    k.ucx_box(cx, cy, DECK_Z + sz / 2, sx, sy, sz)


# ============================================================================
# Domain props (deck z=-10 .. envelope top z=+200, xy within +-330)
# ============================================================================
def prop_science(k):
    """Generic science lab: big central observation/sensor dome + ring collar."""
    k.cyl(MAT_CONN, 190, 14, (0, 0, DECK_Z + 7), verts=24)
    k.hemi(MAT_ACCENT, 170, (0, 0, DECK_Z + 14), segs=20, zscale=1.05)
    for a in range(6):  # dome ribs
        ang = a * math.pi / 3
        k.torus(MAT_CONN, 172, 4, (0, 0, DECK_Z + 14),
                rot=(math.radians(90), 0, ang), maj=20, minr=4)
    k.torus(MAT_CONN, 180, 8, (0, 0, DECK_Z + 20), maj=24, minr=6)
    k.cyl(MAT_CONN, 10, 30, (0, 0, DECK_Z + 190), verts=8)
    k.ucx_box(0, 0, DECK_Z + 100, 360, 360, 200)


def prop_research(k):
    """Generic research lab: twin stacked lab pods + a bridging gantry."""
    lab_pod(k, -120, -40, 220, 380, 120)
    lab_pod(k, -120, -40, 150, 280, 60 + 120, windows=False)  # upper tier
    k.box(MAT_ACCENT, 154, 200, 14, (-120, -40, DECK_Z + 150))
    lab_pod(k, 170, 90, 200, 200, 150)
    k.box(MAT_CONN, 120, 40, 30, (40, 60, DECK_Z + 110))  # gantry bridge
    k.cyl(MAT_UTIL, 40, 150, (190, -200, DECK_Z + 75), verts=12)  # tank
    k.cyl(MAT_UTIL, 40, 150, (100, -200, DECK_Z + 75), verts=12)


def prop_physics(k, big=False):
    """Particle accelerator ring on pylons + central target chamber;
    a small thruster test rig on one side."""
    R = 250 if not big else 255
    rz = DECK_Z + 60
    k.torus(MAT_UTIL, R, 22, (0, 0, rz), maj=36, minr=8)
    k.torus(MAT_ACCENT, R, 8, (0, 0, rz + 20), maj=36, minr=4)
    for a in range(8):
        ang = a * math.pi / 4 + math.pi / 8
        px, py = R * math.cos(ang), R * math.sin(ang)
        k.box(MAT_CONN, 26, 26, 52, (px, py, DECK_Z + 26))
        # magnet yokes around the ring
        k.box(MAT_CONN, 34, 60, 60, (px, py, rz), rot=(0, 0, ang))
    # central chamber
    k.cyl(MAT_SHELL, 70, 110, (0, 0, DECK_Z + 55), verts=16, bev=6)
    k.sphere(MAT_ACCENT, 46, (0, 0, DECK_Z + 125), segs=14)
    k.torus(MAT_CONN, 60, 8, (0, 0, DECK_Z + 125), maj=20, minr=6)
    # injector line from ring to core
    k.cyl(MAT_CONN, 10, R - 60, ((R - 60) / 2 + 60, 0, rz),
          rot=(0, math.radians(90), 0), verts=8)
    if not big:
        # thruster test rig: gantry + small nozzle firing along +X
        k.box(MAT_CONN, 20, 120, 140, (-250, 0, DECK_Z + 70))
        k.cyl(MAT_SHELL, 30, 60, (-215, 0, DECK_Z + 150), rot=(0, math.radians(90), 0))
        k.cone(MAT_UTIL, 40, 26, 50, (-160, 0, DECK_Z + 150), rot=(0, math.radians(90), 0))
    k.ucx_box(0, 0, rz - 5, 2 * R + 50, 2 * R + 50, 140)


def prop_ion_propulsion(k):
    """Tier-4: accelerator ring kept, plus 2 large gridded ion thrusters on a
    test gantry straddling the deck (exhausts along +X)."""
    prop_physics(k, big=True)
    gz = DECK_Z + 150
    k.box(MAT_CONN, 560, 26, 26, (0, 0, TOP_Z - 20))          # spine gantry
    for sy in (-1, 1):
        k.box(MAT_CONN, 26, 26, 200, (-280, sy * 150, DECK_Z + 100))
    for sy in (-1, 1):
        y = sy * 150
        k.cyl(MAT_SHELL, 44, 220, (-110, y, gz), rot=(0, math.radians(90), 0), verts=16, bev=4)
        k.cone(MAT_UTIL, 50, 44, 40, (20, y, gz), rot=(0, math.radians(90), 0))
        # ion grid (Accent, glowing blue) + grid rings
        k.cyl(MAT_ACCENT, 46, 4, (42, y, gz), rot=(0, math.radians(90), 0), verts=16)
        k.torus(MAT_CONN, 50, 5, (44, y, gz), rot=(0, math.radians(90), 0), maj=20, minr=6)
        # PPU box
        k.box(MAT_UTIL, 90, 60, 36, (-160, y, gz - 60))
        # hanger strut
        k.box(MAT_CONN, 14, 14, 30, (-110, y * 0.5, TOP_Z - 38), rot=(math.radians(-60 * sy), 0, 0))
    k.box(MAT_CONN, 30, 330, 20, (-110, 0, TOP_Z - 20))
    k.ucx_box(-40, 0, gz, 200, 400, 100)


def prop_materials(k, n_stacks=4):
    """Furnace stacks + crucible: tall chimneys with glowing throats."""
    stacks = [(-180, -160, 58, 190), (-60, -210, 48, 160), (-190, 20, 44, 140),
              (-60, -60, 36, 120)][:n_stacks]
    for sx_, sy_, r, h in stacks:
        k.cyl(MAT_UTIL, r, h, (sx_, sy_, DECK_Z + h / 2), verts=16)
        k.torus(MAT_CONN, r + 4, 5, (sx_, sy_, DECK_Z + h * 0.35), maj=20, minr=4)
        k.torus(MAT_CONN, r + 4, 5, (sx_, sy_, DECK_Z + h * 0.7), maj=20, minr=4)
        k.cyl(MAT_ACCENT, r * 0.7, 4, (sx_, sy_, DECK_Z + h + 1), verts=16)  # glowing throat
        k.torus(MAT_CONN, r, 6, (sx_, sy_, DECK_Z + h - 2), maj=20, minr=4)
    # crucible: bowl on a tilt frame
    cx, cy = 150, 110
    k.box(MAT_CONN, 20, 200, 110, (cx - 100, cy, DECK_Z + 55))
    k.box(MAT_CONN, 20, 200, 110, (cx + 100, cy, DECK_Z + 55))
    k.cyl(MAT_CONN, 10, 220, (cx, cy, DECK_Z + 100), rot=(0, math.radians(90), 0), verts=8)
    k.cone(MAT_UTIL, 90, 55, 100, (cx, cy, DECK_Z + 90), rot=(math.radians(180 - 18), 0, 0), verts=20)
    k.cyl(MAT_ACCENT, 80, 4, (cx, cy + 15, DECK_Z + 138), rot=(math.radians(-18), 0, 0), verts=20)
    # feed conveyor from stacks to crucible
    k.box(MAT_CONN, 180, 40, 14, (-10, 40, DECK_Z + 40), rot=(0, 0, math.radians(20)))
    k.ucx_box(-120, -110, DECK_Z + 95, 230, 290, 190)
    k.ucx_box(cx, cy, DECK_Z + 75, 220, 200, 150)


def prop_grav_materials(k):
    """Tier-4: 2 furnace stacks kept + large grav-gimbal (3 nested rings
    around a glowing singularity core)."""
    prop_materials_small = [(-240, -230, 44, 170), (-240, 230, 44, 170)]
    for sx_, sy_, r, h in prop_materials_small:
        k.cyl(MAT_UTIL, r, h, (sx_, sy_, DECK_Z + h / 2), verts=16)
        k.torus(MAT_CONN, r + 4, 5, (sx_, sy_, DECK_Z + h * 0.5), maj=20, minr=4)
        k.cyl(MAT_ACCENT, r * 0.7, 4, (sx_, sy_, DECK_Z + h + 1), verts=16)
    cz = DECK_Z + 96
    # cradle
    k.cyl(MAT_CONN, 130, 16, (40, 0, DECK_Z + 8), verts=20)
    for sy in (-1, 1):
        k.box(MAT_CONN, 30, 20, 100, (40, sy * 110, DECK_Z + 50))
    # nested gimbal rings, each tilted differently
    k.torus(MAT_UTIL, 102, 9, (40, 0, cz), rot=(math.radians(90), 0, 0), maj=32, minr=8)
    k.torus(MAT_UTIL, 84, 8, (40, 0, cz), rot=(0, math.radians(90), math.radians(35)), maj=28, minr=8)
    k.torus(MAT_ACCENT, 66, 6, (40, 0, cz), rot=(math.radians(45), math.radians(30), 0), maj=24, minr=6)
    k.sphere(MAT_ACCENT, 36, (40, 0, cz), segs=16)
    # emitter spikes pointing at the core
    for a in range(4):
        ang = a * math.pi / 2 + math.pi / 4
        px, py = 40 + 190 * math.cos(ang), 190 * math.sin(ang)
        k.cyl(MAT_CONN, 12, 110, (px, py, DECK_Z + 55), verts=8)
        k.cone(MAT_SHELL, 22, 4, 50, (px, py, DECK_Z + 135), verts=8)
    k.ucx_box(40, 0, cz, 230, 230, 210)


def prop_electronics(k, big=False):
    """Server stacks with tall heat-sink fins + antenna masts."""
    rows = 3
    for r in range(rows):
        x = -200 + r * 130
        k.box(MAT_SHELL, 90, 420, 70, (x, -20, DECK_Z + 35), bev=5, segs=1)
        k.box(MAT_ACCENT, 94, 380, 6, (x, -20, DECK_Z + 50))  # blinking status band
        n_fins = 7
        for f in range(n_fins):
            y = -20 - 190 + f * (380 / (n_fins - 1))
            k.box(MAT_CONN, 6 if not big else 5, 70, 110, (x, y, DECK_Z + 125))
    if not big:
        for i, (ax, ay, h) in enumerate(((200, 170, 200), (230, -120, 150), (140, -230, 120))):
            k.cyl(MAT_CONN, 7, h, (ax, ay, DECK_Z + h / 2), verts=8)
            for j in range(3):
                k.box(MAT_CONN, 70 - j * 16, 5, 5, (ax, ay, DECK_Z + h * (0.5 + 0.17 * j)))
            k.sphere(MAT_ACCENT, 8, (ax, ay, DECK_Z + h - 4), segs=8)
    k.ucx_box(-70, -20, DECK_Z + 90, 350, 440, 180)


def prop_encryption(k):
    """Tier-4: server stacks kept + phased dish array (5 dishes on posts)."""
    for r in range(2):
        x = -230 + r * 110
        k.box(MAT_SHELL, 80, 440, 70, (x, -10, DECK_Z + 35), bev=5, segs=1)
        k.box(MAT_ACCENT, 84, 400, 6, (x, -10, DECK_Z + 50))
        for f in range(6):
            y = -200 + f * 76
            k.box(MAT_CONN, 6, 70, 100, (x, y, DECK_Z + 120))
    dishes = [(120, 0, 88), (240, 150, 60), (240, -150, 60), (40, 210, 52), (40, -210, 52)]
    for (dx, dy, dr) in dishes:
        h = 30 + dr * 0.5
        k.cyl(MAT_CONN, 9, h, (dx, dy, DECK_Z + h / 2), verts=8)
        tilt = math.radians(-35)
        dz = DECK_Z + h + dr * 0.25
        yaw = math.atan2(dy, dx - 330) if dx != 330 else 0
        k.cone(MAT_SHELL, dr, dr * 0.25, dr * 0.35, (dx, dy, dz),
               rot=(tilt, 0, yaw + math.pi / 2), verts=16)
        k.cyl(MAT_ACCENT, dr * 0.9, 3, (dx, dy, dz + dr * 0.18),
              rot=(tilt, 0, yaw + math.pi / 2), verts=16)
        k.cyl(MAT_CONN, 4, dr * 0.9, (dx, dy, dz + dr * 0.55), verts=6)
    k.ucx_box(-180, -10, DECK_Z + 85, 200, 450, 170)
    k.ucx_box(150, 0, DECK_Z + 100, 300, 560, 200)


def prop_optronics(k):
    """Tier-4: 1 server row kept + tilted lens-array wall (3x4 lenses) and a
    long optical telescope tube."""
    x = -240
    k.box(MAT_SHELL, 80, 440, 70, (x, -10, DECK_Z + 35), bev=5, segs=1)
    k.box(MAT_ACCENT, 84, 400, 6, (x, -10, DECK_Z + 50))
    for f in range(6):
        k.box(MAT_CONN, 6, 70, 90, (x, -200 + f * 76, DECK_Z + 115))
    # lens array panel, tilted back 30deg, facing +X
    tilt = math.radians(30)
    pcx, pcz = 40, DECK_Z + 110
    k.box(MAT_CONN, 24, 440, 170, (pcx, 0, pcz), rot=(0, -tilt, 0))
    k.box(MAT_CONN, 20, 20, 110, (pcx - 60, -180, DECK_Z + 55))
    k.box(MAT_CONN, 20, 20, 110, (pcx - 60, 180, DECK_Z + 55))
    nx = math.cos(tilt)
    nz = math.sin(tilt)
    for row in range(3):
        for col in range(4):
            v = (row - 1) * 52   # along panel up-vector
            u = -165 + col * 110
            ux, uz = -math.sin(tilt) * v, math.cos(tilt) * v
            lx, lz = pcx + ux + nx * 14, pcz + uz + nz * 14
            k.cyl(MAT_CONN, 26, 8, (lx, u, lz), rot=(0, math.radians(90) - tilt, 0), verts=16)
            k.sphere(MAT_ACCENT, 22, (lx + nx * 4, u, lz + nz * 4), segs=12,
                     scale=(0.35, 1, 1))
    # telescope tube along +X on a yoke, stays inside envelope
    k.box(MAT_CONN, 30, 60, 90, (230, 0, DECK_Z + 45))
    k.cyl(MAT_SHELL, 40, 160, (250, 0, DECK_Z + 125), rot=(0, math.radians(80), 0), verts=16, bev=3)
    k.torus(MAT_CONN, 42, 6, (320, 0, DECK_Z + 138), rot=(0, math.radians(80), 0), maj=20, minr=6)
    k.cyl(MAT_ACCENT, 36, 3, (328, 0, DECK_Z + 139), rot=(0, math.radians(80), 0), verts=16)
    k.ucx_box(40, 0, pcz, 120, 460, 200)
    k.ucx_box(250, 0, DECK_Z + 100, 160, 100, 200)


def prop_weapons(k):
    """Test-firing range: a gun mount at -X firing along the deck toward a
    rack of target plates at +X, range rails between."""
    # range rails
    for sy in (-1, 1):
        k.box(MAT_CONN, 560, 14, 14, (20, sy * 110, DECK_Z + 7))
    k.box(MAT_ACCENT, 520, 6, 3, (20, 0, DECK_Z + 2))
    # gun mount
    k.cyl(MAT_CONN, 70, 40, (-220, 0, DECK_Z + 20), verts=16)
    k.box(MAT_SHELL, 110, 90, 70, (-220, 0, DECK_Z + 75), bev=8, segs=1)
    k.cyl(MAT_UTIL, 20, 230, (-70, 0, DECK_Z + 88), rot=(0, math.radians(90), 0), verts=12)
    k.cyl(MAT_CONN, 24, 50, (-140, 0, DECK_Z + 88), rot=(0, math.radians(90), 0), verts=12)
    # targets: 3 plates on posts, staggered
    for i, (tx, ty) in enumerate(((210, 0), (260, -150), (260, 150))):
        k.box(MAT_CONN, 12, 12, 90, (tx, ty, DECK_Z + 45))
        k.box(MAT_SHELL, 14, 140, 140, (tx, ty, DECK_Z + 125), bev=4, segs=1)
        k.cyl(MAT_ACCENT, 30, 4, (tx - 8, ty, DECK_Z + 125), rot=(0, math.radians(90), 0), verts=16)
        k.torus(MAT_ACCENT, 54, 3, (tx - 8, ty, DECK_Z + 125), rot=(0, math.radians(90), 0),
                maj=20, minr=4)
    # blast backstop
    k.box(MAT_CONN, 30, 440, 120, (318, 0, DECK_Z + 60), bev=5, segs=1)
    k.ucx_box(-190, 0, DECK_Z + 60, 180, 150, 120)
    k.ucx_box(260, 0, DECK_Z + 95, 140, 440, 190)


def prop_projectile_weapons(k):
    """Tier-4: full-length railgun -- twin rails in coil rings along the
    deck diagonal-ish (+X axis), heavy breech + capacitor banks."""
    rz = DECK_Z + 110
    # breech block
    k.box(MAT_SHELL, 150, 150, 150, (-230, 0, DECK_Z + 90), bev=10, segs=2)
    k.box(MAT_CONN, 170, 170, 20, (-230, 0, DECK_Z + 10))
    # twin rails
    for sy in (-1, 1):
        k.box(MAT_UTIL, 460, 14, 30, (70, sy * 24, rz))
    k.box(MAT_ACCENT, 440, 10, 10, (75, 0, rz))
    # coil rings
    for i in range(6):
        x = -120 + i * 75
        k.torus(MAT_CONN, 52, 10, (x, 0, rz), rot=(0, math.radians(90), 0), maj=16, minr=6)
        if i % 2 == 0:
            k.box(MAT_CONN, 20, 20, rz - DECK_Z - 50, (x, 0, DECK_Z + (rz - DECK_Z - 50) / 2))
    # muzzle brake
    k.box(MAT_CONN, 40, 100, 70, (310, 0, rz), bev=4, segs=1)
    # capacitor banks along the flanks
    for sy in (-1, 1):
        for i in range(3):
            k.cyl(MAT_UTIL, 30, 90, (-100 + i * 110, sy * 220, DECK_Z + 45), verts=12)
            k.cyl(MAT_ACCENT, 20, 4, (-100 + i * 110, sy * 220, DECK_Z + 92), verts=12)
        k.box(MAT_CONN, 330, 16, 16, (10, sy * 160, DECK_Z + 8))
    k.ucx_box(-230, 0, DECK_Z + 85, 170, 170, 170)
    k.ucx_box(75, 0, rz, 480, 120, 120)


def prop_beam_weapons(k):
    """Tier-4: tall beam emitter tower -- capacitor ring, focusing cone and
    a crystal lens at the tip aimed along +X, with a heat-sink skirt."""
    ex, ey = -40, 0
    k.cyl(MAT_CONN, 140, 20, (ex, ey, DECK_Z + 10), verts=24)
    k.cyl(MAT_SHELL, 95, 70, (ex, ey, DECK_Z + 55), verts=20, bev=6)
    for a in range(8):  # heat sink skirt fins
        ang = a * math.pi / 4
        k.box(MAT_CONN, 70, 8, 80, (ex + 120 * math.cos(ang), ey + 120 * math.sin(ang), DECK_Z + 50),
              rot=(0, 0, ang))
    # capacitor torus
    k.torus(MAT_UTIL, 110, 18, (ex, ey, DECK_Z + 95), maj=28, minr=8)
    k.torus(MAT_ACCENT, 110, 6, (ex, ey, DECK_Z + 114), maj=28, minr=4)
    # emitter body: horizontal barrel with focusing cone to +X
    bz = DECK_Z + 150
    k.cyl(MAT_SHELL, 50, 170, (ex + 20, ey, bz), rot=(0, math.radians(90), 0), verts=16, bev=4)
    k.cone(MAT_UTIL, 50, 18, 150, (ex + 180, ey, bz), rot=(0, math.radians(90), 0), verts=16)
    for i in range(3):
        k.torus(MAT_ACCENT, 38 - i * 8, 4, (ex + 135 + i * 45, ey, bz),
                rot=(0, math.radians(90), 0), maj=16, minr=4)
    k.sphere(MAT_ACCENT, 20, (ex + 265, ey, bz), segs=12)
    k.cyl(MAT_CONN, 30, 90, (ex, ey, DECK_Z + 110), verts=12)  # yoke column
    # rear capacitor stacks
    for sy in (-1, 1):
        k.box(MAT_UTIL, 80, 80, 130, (-250, sy * 170, DECK_Z + 65), bev=6, segs=1)
        k.box(MAT_ACCENT, 84, 20, 60, (-250, sy * 170, DECK_Z + 80))
    k.ucx_box(ex, ey, DECK_Z + 70, 300, 300, 140)
    k.ucx_box(ex + 120, ey, bz, 330, 100, 100)


def prop_biology(k, n_domes=3):
    """Greenhouse domes: glazed hemispheres with ribs on low plinths."""
    domes = [(-130, -110, 150), (170, -150, 110), (150, 160, 125)][:n_domes]
    for (dx, dy, r) in domes:
        k.cyl(MAT_CONN, r + 12, 16, (dx, dy, DECK_Z + 8), verts=20)
        k.hemi(MAT_ACCENT, r, (dx, dy, DECK_Z + 16), segs=16, zscale=min(1.0, 170 / r))
        for a in range(4):
            k.torus(MAT_CONN, r + 1, 3, (dx, dy, DECK_Z + 16),
                    rot=(math.radians(90), 0, a * math.pi / 4), maj=24, minr=4)
        k.torus(MAT_CONN, r * 0.72, 3, (dx, dy, DECK_Z + 16 + r * 0.68), maj=20, minr=4)
    # connecting walk tubes
    k.cyl(MAT_CONN, 18, 200, (20, -130, DECK_Z + 30), rot=(0, math.radians(90), 0), verts=8)
    k.cyl(MAT_CONN, 18, 230, (10, 30, DECK_Z + 30), rot=(0, math.radians(90), math.radians(35)), verts=8)
    # water tank
    if n_domes < 3:
        pass
    else:
        k.cyl(MAT_UTIL, 40, 110, (-230, 200, DECK_Z + 55), verts=12)
    for (dx, dy, r) in domes:
        k.ucx_box(dx, dy, DECK_Z + (16 + r) / 2, 2 * r, 2 * r, 16 + r)


def prop_cybernetics(k):
    """Tier-4: 2 greenhouse domes kept + a radial rack of 6 cybernetics
    capsule pods around a central neural core."""
    prop_biology(k, n_domes=0)
    for (dx, dy, r) in ((-210, 210, 95), (-210, -210, 95)):
        k.cyl(MAT_CONN, r + 10, 14, (dx, dy, DECK_Z + 7), verts=20)
        k.hemi(MAT_ACCENT, r, (dx, dy, DECK_Z + 14), segs=16)
        for a in range(3):
            k.torus(MAT_CONN, r + 1, 3, (dx, dy, DECK_Z + 14),
                    rot=(math.radians(90), 0, a * math.pi / 3), maj=24, minr=4)
        k.ucx_box(dx, dy, DECK_Z + (14 + r) / 2, 2 * r, 2 * r, 14 + r)
    cx, cy = 90, 0
    k.cyl(MAT_CONN, 170, 16, (cx, cy, DECK_Z + 8), verts=24)
    k.cyl(MAT_SHELL, 50, 160, (cx, cy, DECK_Z + 96), verts=16, bev=5)
    k.sphere(MAT_ACCENT, 40, (cx, cy, DECK_Z + 185 - 40), segs=14)
    k.torus(MAT_CONN, 52, 6, (cx, cy, DECK_Z + 145), maj=20, minr=6)
    for a in range(6):
        ang = a * math.pi / 3
        px, py = cx + 125 * math.cos(ang), cy + 125 * math.sin(ang)
        # pods: capsules reclined 25deg outward
        rot = (0, math.radians(25), ang)
        k.cyl(MAT_SHELL, 26, 100, (px, py, DECK_Z + 70), rot=rot, verts=12)
        dxo = math.sin(math.radians(25)) * 50
        k.sphere(MAT_ACCENT, 26, (px + dxo * math.cos(ang), py + dxo * math.sin(ang),
                                  DECK_Z + 70 + math.cos(math.radians(25)) * 50), segs=12)
        k.box(MAT_CONN, 60, 10, 10, ((px + cx) / 2, (py + cy) / 2, DECK_Z + 30), rot=(0, 0, ang))
    k.ucx_box(cx, cy, DECK_Z + 95, 340, 340, 190)


LABS = [
    ("ScienceLab", prop_science),
    ("ResearchLab", prop_research),
    ("PhysicsLab", prop_physics),
    ("MaterialsLab", prop_materials),
    ("ElectronicsLab", prop_electronics),
    ("WeaponsLab", prop_weapons),
    ("BiologyLab", prop_biology),
    ("ProjectileWeaponsLab", prop_projectile_weapons),
    ("BeamWeaponsLab", prop_beam_weapons),
    ("IonPropulsionLab", prop_ion_propulsion),
    ("GravMaterialsLab", prop_grav_materials),
    ("EncryptionLab", prop_encryption),
    ("OptronicsLab", prop_optronics),
    ("CyberneticsLab", prop_cybernetics),
]


def build_lab(short, prop_fn):
    outname = f"SM_StationModule_{short}_01"
    k = Kit(outname, (2, 2, 1))
    lab_base(k)
    prop_fn(k)
    return {outname: k.finalize(outname)}


# ============================================================================
# Turret: 1x1x1 (400 cm cube). Armoured base z -200..+50, turret ring at +50.
# Head = separate FBX, pivot at the yaw axis (0,0,0 in head space = ring top
# centre), barrels along +X, stays inside the cell (z <= +200) at rest.
# ============================================================================
TURRET_RING_Z = 50.0


def build_turret_base():
    outname = "SM_StationModule_Turret_01"
    k = Kit(outname, (1, 1, 1))
    L, W, H = k.L, k.W, k.H
    hz = -H / 2
    # armoured octagonal plinth, full footprint at the bottom, tapering up
    k.cyl(MAT_DEF, L / 2 / math.cos(math.pi / 8) - 1, 120, (0, 0, hz + 60),
          rot=(0, 0, math.pi / 8), verts=8)
    k.cone(MAT_DEF, 180, 130, 130, (0, 0, hz + 120 + 65), rot=(0, 0, math.pi / 8), verts=8)
    # corner armour blocks (flush to cell corners) -- keeps a square read
    for sx in (-1, 1):
        for sy in (-1, 1):
            k.box(MAT_DEF, 80, 80, 140, (sx * (L / 2 - 40), sy * (W / 2 - 40), hz + 70), bev=10, segs=1)
            k.box(MAT_ACCENT, 20, 20, 8, (sx * (L / 2 - 40), sy * (W / 2 - 40), hz + 142))
    # connection faces: flush square frames with collars at z=-80 -- a
    # 1x1x1 cell is small; the collar sits on the plinth band
    for axis in (0, 1):
        for sign in (-1, 1):
            if axis == 0:
                k.box(MAT_CONN, 20, 200, 150, (sign * (L / 2 - 10), 0, hz + 85), bev=4, segs=1)
                k.torus(MAT_CONN, 62, 6, (sign * (L / 2 - 6), 0, hz + 85),
                        rot=(0, math.radians(90), 0), maj=16, minr=6)
            else:
                k.box(MAT_CONN, 200, 20, 150, (0, sign * (W / 2 - 10), hz + 85), bev=4, segs=1)
                k.torus(MAT_CONN, 62, 6, (0, sign * (W / 2 - 6), hz + 85),
                        rot=(math.radians(90), 0, 0), maj=16, minr=6)
    # upper armour collar + turret ring (bearing race)
    k.cyl(MAT_DEF, 125, TURRET_RING_Z - (hz + 250) , (0, 0, (TURRET_RING_Z + hz + 250) / 2), verts=16, bev=4)
    k.torus(MAT_CONN, 118, 8, (0, 0, TURRET_RING_Z - 4), maj=24, minr=6)
    k.torus(MAT_ACCENT, 128, 3, (0, 0, TURRET_RING_Z - 16), maj=24, minr=4)
    # ammo feed / cable conduits running down the plinth
    for sx in (-1, 1):
        k.box(MAT_CONN, 24, 40, 140, (sx * 120, 0, -30), rot=(0, sx * math.radians(-16), 0))
    k.socket("TurretHead", (0, 0, TURRET_RING_Z))
    k.ucx_box(0, 0, hz + 60, L, W, 120)
    k.ucx_box(0, 0, (hz + 120 + TURRET_RING_Z) / 2, 300, 300, TURRET_RING_Z - (hz + 120))
    return {outname: k.finalize(outname)}


def build_turret_head():
    """Built directly in module space above the ring (so the envelope check and
    the combined preview line up), then the pivot is moved to the yaw axis at
    the ring top. Head-local: +Z up = yaw axis, +X = barrel/forward."""
    outname = "SM_StationModule_TurretHead_01"
    k = Kit(outname, (1, 1, 1))
    z0 = TURRET_RING_Z
    # yaw platform
    k.cyl(MAT_DEF, 115, 16, (0, 0, z0 + 8), verts=16)
    # armoured mantlet / gun house, sloped front
    k.box(MAT_DEF, 170, 170, 80, (-15, 0, z0 + 56), bev=14, segs=2)
    k.box(MAT_DEF, 90, 150, 50, (-70, 0, z0 + 110), bev=10, segs=1)
    k.box(MAT_DEF, 60, 190, 40, (55, 0, z0 + 70), rot=(0, math.radians(-25), 0), bev=6, segs=1)
    # trunnion (pitch axis) cheeks
    pz = z0 + 70
    for sy in (-1, 1):
        k.box(MAT_CONN, 60, 20, 70, (40, sy * 60, pz))
    k.cyl(MAT_CONN, 18, 140, (40, 0, pz), rot=(math.radians(90), 0, 0), verts=12)
    # twin barrels along +X, ending inside the cell (x <= +198)
    for sy in (-1, 1):
        k.cyl(MAT_DEF, 16, 140, (115, sy * 30, pz), rot=(0, math.radians(90), 0), verts=12)
        k.cyl(MAT_CONN, 11, 20, (185, sy * 30, pz), rot=(0, math.radians(90), 0), verts=12)
        k.cyl(MAT_ACCENT, 7, 2, (195.5, sy * 30, pz), rot=(0, math.radians(90), 0), verts=12)
        k.torus(MAT_CONN, 18, 4, (70, sy * 30, pz), rot=(0, math.radians(90), 0), maj=16, minr=4)
    # sensor pod + targeting eye
    k.box(MAT_CONN, 50, 36, 30, (-30, -75, z0 + 115), bev=4, segs=1)
    k.cyl(MAT_ACCENT, 10, 4, (-4, -75, z0 + 118), rot=(0, math.radians(90), 0), verts=12)
    k.box(MAT_ACCENT, 6, 120, 8, (26, 0, z0 + 90))  # vision slit
    k.socket("Muzzle_L", (197, -30, pz))
    k.socket("Muzzle_R", (197, 30, pz))
    k.socket("PitchAxis", (40, 0, pz))
    k.ucx_box(-15, 0, z0 + 60, 180, 180, 120)
    k.ucx_box(120, 0, pz, 160, 90, 40)
    info = k.finalize(outname, pivot=(0, 0, z0))
    return {outname: info}


# ============================================================================
# Shield generator: 2x2x1. Armoured base + 4 emitter pylons + central core
# column + upward-facing emitter dish (focus at the top of the envelope).
# ============================================================================
def build_shield_generator():
    outname = "SM_StationModule_ShieldGenerator_01"
    k = Kit(outname, (2, 2, 1))
    L, W, H = k.L, k.W, k.H
    hz = -H / 2
    base_top = -60.0
    bh = base_top - hz
    k.box(MAT_DEF, L - 16, W - 16, bh, (0, 0, hz + bh / 2), bev=16, segs=2)
    k.box(MAT_CONN, L - 4, W - 4, 10, (0, 0, base_top - 30))
    # connection bulkheads with collars at z=0 (kit standard)
    for axis in (0, 1):
        for sign in (-1, 1):
            if axis == 0:
                k.box(MAT_CONN, 34, 290, 330, (sign * (L / 2 - 17), 0, -35), bev=8, segs=1)
                k.torus(MAT_CONN, 110, 8, (sign * (L / 2 - 8), 0, 0),
                        rot=(0, math.radians(90), 0), maj=20, minr=6)
                k.cyl(MAT_DEF, 96, 10, (sign * (L / 2 - 6), 0, 0),
                      rot=(0, math.radians(90), 0), verts=16)
            else:
                k.box(MAT_CONN, 290, 34, 330, (0, sign * (W / 2 - 17), -35), bev=8, segs=1)
                k.torus(MAT_CONN, 110, 8, (0, sign * (W / 2 - 8), 0),
                        rot=(math.radians(90), 0, 0), maj=20, minr=6)
                k.cyl(MAT_DEF, 96, 10, (0, sign * (W / 2 - 6), 0),
                      rot=(math.radians(90), 0, 0), verts=16)
    # armour window slits (Accent) on the base
    for axis in (0, 1):
        for sign in (-1, 1):
            for off in (-230, 230):
                if axis == 0:
                    k.box(MAT_ACCENT, 6, 110, 14, (sign * (L / 2 - 8), off, hz + 80))
                else:
                    k.box(MAT_ACCENT, 110, 6, 14, (off, sign * (W / 2 - 8), hz + 80))
    # 4 corner emitter pylons, tapering, leaning inward, with emitter nodes
    for sx in (-1, 1):
        for sy in (-1, 1):
            px, py = sx * (L / 2 - 70), sy * (W / 2 - 70)
            k.box(MAT_DEF, 110, 110, 40, (px, py, base_top + 20), bev=8, segs=1)
            k.cone(MAT_DEF, 42, 16, 220, (px - sx * 22, py - sy * 22, base_top + 150),
                   rot=(sy * math.radians(-12), sx * math.radians(12), 0), verts=8)
            k.sphere(MAT_ACCENT, 22, (px - sx * 44, py - sy * 44, TOP_Z - 24), segs=12)
            k.torus(MAT_CONN, 28, 5, (px - sx * 40, py - sy * 40, TOP_Z - 60), maj=16, minr=4)
    # central core column with capacitor rings
    k.cyl(MAT_DEF, 140, 30, (0, 0, base_top + 15), verts=24)
    k.cyl(MAT_SHELL if False else MAT_DEF, 60, 150, (0, 0, base_top + 105), verts=16, bev=4)
    for i in range(3):
        k.torus(MAT_ACCENT if i == 1 else MAT_CONN, 74, 9, (0, 0, base_top + 60 + i * 40),
                maj=24, minr=6)
    # emitter dish: open concave-up bowl, glowing floor, rim ring
    dz = TOP_Z - 75
    k.dish(MAT_DEF, 215, 70, 70, (0, 0, dz), verts=28, thick=8)
    k.torus(MAT_CONN, 215, 9, (0, 0, dz + 35), maj=28, minr=6)
    k.cyl(MAT_ACCENT, 72, 4, (0, 0, dz - 33), verts=20)
    k.torus(MAT_ACCENT, 150, 4, (0, 0, dz + 5), maj=28, minr=4)
    # feed tripod from the rim to a focus node at the envelope top
    for a in range(3):
        ang = a * 2 * math.pi / 3 + math.pi / 6
        k.strut(MAT_CONN, (200 * math.cos(ang), 200 * math.sin(ang), dz + 38),
                (0, 0, TOP_Z - 30), r=6)
    k.sphere(MAT_ACCENT, 20, (0, 0, TOP_Z - 22), segs=12)
    k.socket("ShieldEmitter", (0, 0, TOP_Z - 22))
    k.ucx_box(0, 0, hz + bh / 2, L, W, bh)
    k.ucx_box(0, 0, dz + 5, 440, 440, 80)
    k.ucx_box(0, 0, base_top + 90, 160, 160, 180)
    for sx in (-1, 1):
        for sy in (-1, 1):
            k.ucx_box(sx * (L / 2 - 90), sy * (W / 2 - 90), base_top + 130, 100, 100, 260)
    return {outname: k.finalize(outname)}


# ============================================================================
# Preview renders: re-import the exported FBX (validates the export), swap the
# slot materials for lit preview materials, EEVEE 3/4 view.
# ============================================================================
PREVIEW_TEX = {MAT_SHELL: "StationModule_Shell", MAT_CONN: "StationModule_Connector",
               MAT_UTIL: "StationModule_Utility", MAT_DEF: "StationModule_Defence"}


def preview_mat(slot, accent):
    key = f"PV_{slot}_{accent}"
    m = bpy.data.materials.get(key)
    if m:
        return m
    m = bpy.data.materials.new(key)
    m.use_nodes = True
    nt = m.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    if slot == MAT_ACCENT:
        bsdf.inputs["Base Color"].default_value = (*accent, 1)
        bsdf.inputs["Emission Color"].default_value = (*accent, 1)
        bsdf.inputs["Emission Strength"].default_value = 2.5
        return m
    tex = PREVIEW_TEX.get(slot)
    p = os.path.join(gen.TEXDIR, f"T_{tex}_D.png") if tex else None
    if p and os.path.exists(p):
        img = nt.nodes.new("ShaderNodeTexImage")
        img.image = bpy.data.images.load(p, check_existing=True)
        mix = nt.nodes.new("ShaderNodeMixRGB")
        mix.inputs["Fac"].default_value = 0.45
        mix.inputs["Color2"].default_value = (0.55, 0.57, 0.62, 1.0)
        nt.links.new(img.outputs["Color"], mix.inputs["Color1"])
        nt.links.new(mix.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Metallic"].default_value = 0.6
    bsdf.inputs["Roughness"].default_value = 0.45
    return m


def render_preview(fbx_list, png_name, accent, footprint):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    # match the export (1 BU = 1 cm) so the FBX importer brings the cm-unit
    # file back at 800 BU rather than 8 BU
    sc.unit_settings.system = 'METRIC'
    sc.unit_settings.scale_length = 0.01
    for f in fbx_list:
        bpy.ops.import_scene.fbx(filepath=f)
    meshes = []
    for o in list(bpy.data.objects):
        if o.name.startswith("UCX_"):
            o.hide_render = True
            continue
        if o.type == 'MESH':
            meshes.append(o)
            for i, s in enumerate(o.material_slots):
                base = s.material.name.split(".")[0] if s.material else ""
                o.material_slots[i].material = preview_mat(base, accent)
    # turret head: FBX import puts it at its own pivot; lift it onto the ring
    for o in meshes:
        if "TurretHead" in o.name:
            o.location.z += TURRET_RING_Z
    # footprint ghost (wire box) + floor plate showing the grid cells
    L, W, H = (c * GRID for c in footprint)
    bpy.ops.mesh.primitive_plane_add(size=1, location=(0, 0, -H / 2 - 1))
    fl = bpy.context.active_object
    fl.scale = (L * 3, W * 3, 1)
    fm = bpy.data.materials.new("Floor")
    fm.use_nodes = True
    fm.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.05, 0.055, 0.07, 1)
    fl.data.materials.append(fm)

    diag = (L * L + W * W + H * H) ** 0.5
    center = Vector((0, 0, 0))
    az, el = math.radians(38), math.radians(32)
    dist = diag * (1.55 if footprint[0] > 1 else 1.9)
    camloc = center + Vector((dist * math.cos(el) * math.cos(az) * 1.0,
                              -dist * math.cos(el) * math.sin(az),
                              dist * math.sin(el)))
    bpy.ops.object.camera_add(location=camloc)
    cam = bpy.context.active_object
    sc.camera = cam
    cam.data.lens = 50
    cam.data.clip_end = 100000
    d = (center - camloc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()

    w = bpy.data.worlds.new("PV")
    sc.world = w
    w.use_nodes = True
    w.node_tree.nodes["Background"].inputs[0].default_value = (0.02, 0.025, 0.04, 1)
    w.node_tree.nodes["Background"].inputs[1].default_value = 1.0
    for loc, energy, col, size in (
            ((diag, -diag * 0.6, diag * 0.9), 16 * diag * diag, (1.0, 0.97, 0.9), diag),
            ((-diag, diag * 0.8, diag * 0.6), 9 * diag * diag, (0.55, 0.65, 1.0), diag),
            ((diag * 0.2, -diag, diag * 0.2), 4 * diag * diag, (0.8, 0.85, 1.0), diag)):
        bpy.ops.object.light_add(type='AREA', location=loc)
        lt = bpy.context.active_object
        lt.data.energy = energy
        lt.data.color = col
        lt.data.size = size
        lt.rotation_euler = (center - Vector(loc)).normalized().to_track_quat('-Z', 'Y').to_euler()

    sc.render.engine = 'BLENDER_EEVEE'
    try:
        sc.eevee.taa_render_samples = 32
    except Exception:
        pass
    sc.render.resolution_x = 1100
    sc.render.resolution_y = 800
    sc.render.image_settings.file_format = 'PNG'
    try:
        sc.view_settings.view_transform = 'AgX'
    except Exception:
        pass
    sc.render.filepath = os.path.join(PREVIEW_DIR, png_name)
    bpy.ops.render.render(write_still=True)
    return sc.render.filepath


def contact_sheet(pngs, out):
    try:
        import numpy as np
    except Exception:
        return None
    imgs = []
    for p in pngs:
        im = bpy.data.images.load(p)
        w, h = im.size
        a = np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4)
        imgs.append(a[::2, ::2])  # half res
        bpy.data.images.remove(im)
    th, tw = imgs[0].shape[:2]
    cols = 4
    rows = (len(imgs) + cols - 1) // cols
    sheet = np.zeros((rows * th, cols * tw, 4), dtype=np.float32)
    sheet[..., 3] = 1
    for i, a in enumerate(imgs):
        r, c = divmod(i, cols)
        # blender pixel rows are bottom-up
        rr = rows - 1 - r
        sheet[rr * th:(rr + 1) * th, c * tw:(c + 1) * tw] = a[:th, :tw]
    im = bpy.data.images.new("sheet", cols * tw, rows * th, alpha=True)
    im.pixels[:] = sheet.ravel()
    im.filepath_raw = out
    im.file_format = 'PNG'
    im.save()
    return out


# ============================================================================
def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    only = None
    if "--only" in argv:
        only = [a for a in argv[argv.index("--only") + 1:] if not a.startswith("--")]
    do_render = "--no-render" not in argv

    os.makedirs(OUT_DIR, exist_ok=True)
    os.makedirs(PREVIEW_DIR, exist_ok=True)
    gen.setup_scene()

    jobs = [(short, (lambda s=short, f=fn: build_lab(s, f))) for short, fn in LABS]
    jobs.append(("Turret", lambda: {**build_turret_base(), **_fresh(build_turret_head)}))
    jobs.append(("ShieldGenerator", build_shield_generator))

    results = {}
    previews = []
    for short, fn in jobs:
        if only and short not in only:
            continue
        print(f"Building {short}...")
        gen.setup_scene()
        infos = fn()
        results.update(infos)
        if do_render:
            fbxs = [i["fbx"] for i in infos.values()]
            png = f"SM_StationModule_{short}_01.png"
            print(f"  rendering {png}")
            previews.append(render_preview(fbxs, png, DOMAIN_ACCENT.get(short, (0.5, 0.8, 1)),
                                           next(iter(infos.values()))["footprint"]))

    if do_render and len(previews) > 1:
        cs = contact_sheet(previews, os.path.join(PREVIEW_DIR,
                                                  "SM_StationModule_LabDefence_ContactSheet.png"))
        print("contact sheet:", cs)

    print("SUMMARY")
    for name, i in results.items():
        print(f"  {name}: tris={i['tris']} verts={i['verts']} footprint={i['footprint']} "
              f"slots={i['slots']} ucx={i['ucx']} sockets={i['sockets']} bounds={i['bounds']}")
    print("DONE")


def _fresh(fn):
    gen.setup_scene()
    return fn()


if __name__ == "__main__":
    main()
