"""
Adastrea 3D Asset Generator (Blender 5.x, headless)
====================================================
Procedurally builds hard-surface sci-fi assets and exports FBX + PBR textures
that import cleanly into Unreal Engine 5.8.

Conventions (match Adastrea CONTENT_ORGANIZATION.md):
  - Real-world cm scale (1 Blender cm = 1 Unreal unit)
  - Z-up, nose forward along -Y (UE default FBX forward)
  - Meshes: SM_[Category]_[Object]_[Variant]
  - Textures: T_[Object]_[MapType]_[Variant]  (_D _N _R _M _AO _E)
  - Ship tri budgets: small <10k, props <1k, large <50k
  - 2K tileable textures for ships, 1K for props

Usage:
  blender -b --python generate_adastrea_assets.py
"""
import bpy, os, math
from mathutils import Vector
import numpy as np

BASE = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TEXDIR = os.path.join(BASE, "Textures")
os.makedirs(TEXDIR, exist_ok=True)

# ----------------------------------------------------------------------------
# Scene setup
# ----------------------------------------------------------------------------
def setup_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.length_unit = 'CENTIMETERS'
    scene.render.engine = 'BLENDER_EEVEE'

def clear_scene():
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    for m in list(bpy.data.meshes):
        bpy.data.meshes.remove(m)
    for im in list(bpy.data.images):
        bpy.data.images.remove(im)

def sel_activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

def clean_mesh(ob, eps=0.01):
    """Merge near-coincident vertices and drop degenerate faces (UE-safe).

    Joining beveled primitives leaves coincident duplicate vertices that become
    zero-area triangles -- these break UE collision generation and can glitch
    normals. Uses Blender's native remove_doubles so the exported FBX imports clean.
    """
    nv_before = len(ob.data.vertices)
    sel_activate(ob)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.remove_doubles(threshold=eps / 100.0)  # in Blender units (meters)
    bpy.ops.object.mode_set(mode='OBJECT')
    nv_after = len(ob.data.vertices)
    print(f"  clean_mesh: {nv_before} -> {nv_after} verts (merged {nv_before - nv_after})")

def delete_operands(mesh_objs):
    for ob in mesh_objs:
        if ob.name in bpy.data.objects:
            bpy.data.objects.remove(ob, do_unlink=True)

# ----------------------------------------------------------------------------
# Primitive builders
# ----------------------------------------------------------------------------
def cube(name, size, loc=(0,0,0), rot=(0,0,0)):
    bpy.ops.mesh.primitive_cube_add(size=size, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.rotation_euler = rot
    return ob

def box(name, sx, sy, sz, loc=(0,0,0), rot=(0,0,0)):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = (sx, sy, sz)
    ob.rotation_euler = rot
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    return ob

def cyl(name, r, h, loc=(0,0,0), rot=(0,0,0), verts=32):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=h, vertices=verts, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.rotation_euler = rot
    return ob

def cone(name, r, h, loc=(0,0,0), rot=(0,0,0), verts=32):
    bpy.ops.mesh.primitive_cone_add(radius1=r, radius2=0, depth=h, vertices=verts, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.rotation_euler = rot
    return ob

def sphere(name, r, loc=(0,0,0), verts=24):
    bpy.ops.mesh.primitive_uv_sphere_add(radius=r, segments=verts, ring_count=verts//2, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    return ob

def torus(name, major_r, minor_r, loc=(0,0,0), rot=(0,0,0), maj=36, minr=10):
    bpy.ops.mesh.primitive_torus_add(
        major_radius=major_r, minor_radius=minor_r,
        major_segments=maj, minor_segments=minr, location=loc, rotation=rot,
    )
    ob = bpy.context.active_object
    ob.name = name
    return ob

def rock(name, r, loc=(0,0,0), sub=2, scale_xyz=(1,1,1)):
    bpy.ops.mesh.primitive_ico_sphere_add(radius=r, subdivisions=sub, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = scale_xyz
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    return ob

def greeble(name, loc, sx=6, sy=6, sz=6, rot=(0,0,0)):
    """Small raised detail box/greeble on a hull surface."""
    ob = box(name, sx, sy, sz, loc=loc, rot=rot)
    bevel(ob, 1.2, 1)
    return ob

def nozzle(name, r, h, loc=(0,0,0), rot=(0,0,0)):
    """Engine thruster nozzle (flared cylinder)."""
    bpy.ops.mesh.primitive_cone_add(radius1=r*1.25, radius2=r*0.6, depth=h,
                                    vertices=20, location=loc, rotation=rot)
    ob = bpy.context.active_object
    ob.name = name
    bevel(ob, 1, 1)
    return ob

def bevel(ob, width=0.02, segs=2):
    m = ob.modifiers.new("bevel", 'BEVEL')
    m.width = width
    m.segments = segs
    m.offset_type = 'WIDTH'
    m.limit_method = 'ANGLE'
    m.angle_limit = math.radians(45)
    return ob

def solidify(ob, thick=0.02):
    ob.modifiers.new("sol", 'SOLIDIFY')
    ob.modifiers["sol"].thickness = thick
    return ob

def mirror(ob, axis='X'):
    m = ob.modifiers.new("mir", 'MIRROR')
    m.use_axis[0] = axis == 'X'
    m.use_axis[1] = axis == 'Y'
    m.use_axis[2] = axis == 'Z'
    return ob

def apply_mods(ob):
    sel_activate(ob)
    for m in list(ob.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)

def join(objs, target_name, keep_named=None):
    """Join objects into one mesh, deleting operands except those in keep_named."""
    if not objs:
        return None
    sel_activate(objs[0])
    for ob in objs[1:]:
        ob.select_set(True)
    bpy.ops.object.join()
    out = bpy.context.active_object
    out.name = target_name
    # remove boolean operands & temp primitives that should not persist
    keep = (keep_named or set()) | {target_name}
    for ob in list(bpy.data.objects):
        if ob.name not in keep and ob.type == 'MESH':
            bpy.data.objects.remove(ob, do_unlink=True)
    return out

class Mat:
    """Named material slot wrapper so each mesh carries proper slots."""

    def __init__(self, name):
        self.name = name
        self.slot = self._make()

    def _make(self):
        mat = bpy.data.materials.new(self.name)
        mat.use_nodes = True
        return mat

    def assign(self, obj, mat_index=0):
        if not obj.data.materials:
            obj.data.materials.append(self.slot)
        else:
            # ensure len covers index
            while len(obj.data.materials) <= mat_index:
                obj.data.materials.append(bpy.data.materials.new("__slot"))
            obj.data.materials[mat_index] = self.slot

# ----------------------------------------------------------------------------
# Procedural PBR texture generation (tileable, seeded)
# ----------------------------------------------------------------------------
def _smooth_ao(h, blur=3):
    from scipy.ndimage import uniform_filter
    try:
        return uniform_filter(h.astype(np.float32), blur, mode='wrap')
    except ImportError:
        return h

reset_tex = {}

def gen_texture_set(name, variant, size=2048, seed=1):
    """Create D/N/R/M/AO/E PNGs for a detailed sci-fi hull/panel look.

    Higher-detail than v1: finer panel grid, dense rivets + bolt rings, hazard
    warning stripes, cable/conduit runs, and micro-displacement in the normal
    map so even low-tri meshes read as detailed.
    """
    rng = np.random.default_rng(seed)
    W = H = size
    cells = variant.get('cells', 12)          # panel grid resolution (was 8)
    cell = max(6, W // cells)
    # -- height field: base + panel grid --
    h = np.ones((H, W), dtype=np.float32) * 0.5
    def groove(v):
        return 0.14 + 0.06 * rng.random()
    # primary panel grooves
    for i in range(cell, W, cell):
        h[:, max(0,i-1):i+1] = groove(i)
    for j in range(cell, H, cell):
        h[max(0,j-1):j+1, :] = groove(j)
    # secondary micro-paneling (half-grid, fainter) for extra density
    subcell = cell // 2
    for i in range(subcell, W, subcell):
        h[:, max(0,i-1):i] = 0.30
    for j in range(subcell, H, subcell):
        h[max(0,j-1):j, :] = 0.30
    # rivet rows along every seam + field rivets
    rv = cell // 3
    for i in range(cell, W, cell):
        for y in range(0, H, rv):
            h[y:y+2, i-2:i] = 0.34
    for j in range(cell, H, cell):
        for x in range(0, W, rv):
            h[j-2:j, x:x+2] = 0.34
    # bolt rings at panel corners
    br = 2
    for i in range(cell, W, cell):
        for j in range(cell, H, cell):
            for dx, dy in ((0,0),(br,0),(0,br)):
                yy, xx = min(j, H-1), min(i, W-1)
                h[yy:yy+br, xx:xx+br] = 0.12
    # random ablative pitting / battle damage
    for _ in range(rng.integers(40, 70)):
        px, py = rng.integers(0, W), rng.integers(0, H)
        r = rng.integers(size//60, size//18)
        yy, xx = np.ogrid[:size, :size]
        m = (xx - px)**2 + (yy - py)**2 < r**2
        h[m] = 0.24 + 0.12*rng.random()

    # base color
    base = np.array(variant.get('base', [0.55, 0.56, 0.58]))
    accent = np.array(variant.get('accent', [0.1, 0.15, 0.2]))
    D = np.empty((H, W, 4), dtype=np.float32)
    for ch in range(3):
        D[..., ch] = np.clip(base[ch] - (0.5 - h) * 0.3, 0, 1)
    D[..., 3] = 1.0
    # accent seam stripes
    stripe = np.zeros((H, W), dtype=bool)
    for i in range(cell, W, cell):
        stripe[max(0,i-3):i+3, :] = True
    D[stripe] = [accent[0], accent[1], accent[2], 1.0]

    # emissive mask: glow seams + hazard-light dots
    E = np.zeros((H, W, 4), dtype=np.float32)
    E[..., 3] = 1.0
    em = np.array(variant.get('emissive', [0.2, 0.55, 1.0]))
    E[stripe] = [em[0], em[1], em[2], 1.0]

    # roughness: smooth metal, rough grooves
    R = np.ones((H, W), dtype=np.float32) * 0.34
    R[h < 0.28] = 0.82
    R[h < 0.16] = 0.95

    # metallic: mostly metal; accent/emissive regions slightly less
    M = np.ones((H, W), dtype=np.float32) * 0.96
    M[stripe] = 0.35

    # AO: multi-scale, darker in deep grooves & pits
    AO = (h - 0.5) * 2.0 + 0.62
    AO = np.clip(AO, 0.05, 1.0)

    # ---- optional cyberpunk / industrial overlays ----
    # lit window grid (station hab, city-like)
    if variant.get('windows'):
        wc = variant['windows']
        wcell = W // wc.get('cols', 14)
        ww, wh = int(wcell*0.32), int(wcell*0.36)
        cool = np.array(wc.get('cool', [0.3, 0.65, 1.0]))
        warm = np.array(wc.get('warm', [1.0, 0.6, 0.25]))
        frac = wc.get('frac', 0.6)
        for i in range(wcell, W, wcell*2):
            for j in range(wcell, H, wcell*2):
                r0, r1 = j, min(j+ww, H)
                c0, c1 = i, min(i+wh, W)
                h[r0:r1, c0:c1] = 0.16
                if rng.random() < frac:
                    col = cool if rng.random() < 0.7 else warm
                    D[r0:r1, c0:c1] = [col[0]*0.8, col[1]*0.88, col[2]*1.0, 1.0]
                    E[r0:r1, c0:c1] = [col[0], col[1], col[2], 1.0]
                    AO[r0:r1, c0:c1] = 0.1
                    R[r0:r1, c0:c1] = 0.3
    # neon trim
    if variant.get('neon'):
        nc = variant['neon']
        nc = np.array(nc[:3] if isinstance(nc, (list, tuple)) else [0.3,1.0,1.0])
        thick = variant.get('neon_thick', 3)
        neon = np.zeros((H, W), dtype=bool)
        for i in range(cell, W, cell):
            neon[max(0,i-thick):i+thick, :] = True
        D[neon] = [nc[0]*0.7, nc[1]*0.7, nc[2]*0.7, 1.0]
        E[neon] = [nc[0], nc[1], nc[2], 1.0]
        AO[neon] = 0.55
    # grime / scorch streaks
    if variant.get('grime'):
        for _ in range(rng.integers(10, 18)):
            gx = int(rng.integers(0, W)); gy = int(rng.integers(0, H))
            glen = int(rng.integers(H//10, H//3))
            y0 = max(0, gy - glen//2); y1 = min(H, gy + glen//2)
            w = int(rng.integers(1, 4))
            for ch in range(3):
                D[y0:y1, gx:gx+w, ch] *= (0.72 - 0.08*ch)
            R[y0:y1, gx:gx+w] = 0.92
            AO[y0:y1, gx:gx+w] = 0.5
    # hazard warning stripes
    if variant.get('hazard'):
        for _ in range(variant['hazard'].get('bands', 4)):
            sx = int(rng.integers(0, W)); sy = int(rng.integers(0, H))
            slen = int(rng.integers(W//5, W//2))
            sw = int(rng.integers(6, 14))
            horizontal = rng.random() < 0.5
            for k in range(0, slen, sw*2):
                if horizontal:
                    x0 = sx + k
                    D[sy:sy+sw, max(0,x0):min(W, x0+sw)] = [1.0, 0.8, 0.1, 1.0]
                else:
                    y0 = sy + k
                    D[max(0,y0):min(H,y0+sw), sx:sx+sw] = [1.0, 0.8, 0.1, 1.0]
            R[sy:sy+sw, sx:sx+sw] = 0.6
    # cable / conduit runs
    if variant.get('cable'):
        for _ in range(variant['cable'].get('runs', 5)):
            cx = int(rng.integers(0, W)); cy = int(rng.integers(0, H))
            horizontal = rng.random() < 0.5
            length = int(rng.integers(W//4, W))
            for k in range(0, length, 2):
                if horizontal:
                    x = cx + k; y = cy + (0 if (k//8) % 2 == 0 else 2)
                    h[max(0,min(H-1,y)):min(H,y+2), max(0,min(W-1,x)):min(W,x+3)] = 0.10
                else:
                    y = cy + k; x = cx + (0 if (k//8) % 2 == 0 else 2)
                    h[max(0,min(H-1,y)):min(H,y+3), max(0,min(W-1,x)):min(W,x+2)] = 0.10

    # ---- normal map with micro-displacement for high detail ----
    # smooth the macro height a touch, then add high-freq noise
    hn = h.copy()
    gx, gy = np.gradient(hn)
    inv = 2.6
    N = np.empty((H, W, 4), dtype=np.float32)
    N[..., 0] = -gx * inv
    N[..., 1] = -gy * inv
    N[..., 2] = 1.0
    # add fine noise displacement (high-frequency micro-grain)
    noise = rng.random((H, W)).astype(np.float32)
    ngx, ngy = np.gradient(noise)
    N[..., 0] += -ngx * 0.12
    N[..., 1] += -ngy * 0.12
    nrm = np.sqrt(N[...,0]**2 + N[...,1]**2 + N[...,2]**2)
    N[...,0] /= nrm; N[...,1] /= nrm; N[...,2] /= nrm
    N[...,2] = -N[...,2]
    N[...,3] = 1.0

    maps = {'_D': D, '_N': N, '_R': _to4(R), '_M': _to4(M), '_AO': _to4(AO), '_E': E}
    for suf, arr in maps.items():
        fname = f"T_{name}{suf}.png"
        bpy.data.images.new(fname, width=W, height=H)
        bpy.data.images[fname].pixels = arr.flatten().tolist()
        bpy.data.images[fname].filepath_raw = os.path.join(TEXDIR, fname)
        bpy.data.images[fname].file_format = 'PNG'
        bpy.data.images[fname].save()
    return {suf: f"T_{name}{suf}.png" for suf in maps}

def _to4(gray):
    a = np.empty((gray.shape[0], gray.shape[1], 4), dtype=np.float32)
    a[...,0] = gray; a[...,1] = gray; a[...,2] = gray; a[...,3] = 1.0
    return a

# ----------------------------------------------------------------------------
# UV + export helpers
# ----------------------------------------------------------------------------
def smart_uv(ob, angle=66, margin=0.03):
    sel_activate(ob)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=angle, island_margin=margin, scale_to_bounds=True)
    bpy.ops.object.mode_set(mode='OBJECT')

def export_fbx(obj, outname):
    sel_activate(obj)
    out = os.path.join(BASE, outname + '.fbx')
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z',
    )
    return out


def finalize_part(objs, outname, matname, origin='ORIGIN_CENTER_OF_VOLUME'):
    """Join a set of objects into ONE component mesh, clean it, UV it, set a
    sensible local pivot, attach a material slot, and export as its own FBX.

    Mirrors X4's per-component architecture: every hull/engine/sensor/canopy
    is a separate mesh with its own pivot + material, so UE can treat them as
    independent (and, in gameplay, destroyable) parts.
    """
    joined = join(objs, outname + "_Geo")
    apply_mods(joined)
    clean_mesh(joined)
    # center the pivot at the part's own volume (so e.g. an engine rotates in place)
    sel_activate(joined)
    bpy.ops.object.origin_set(type=origin)
    smart_uv(joined)
    joined.name = outname
    mat = bpy.data.materials.get(matname)
    if mat is None:
        mat = bpy.data.materials.new(matname)
    mat.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(mat)
    else:
        joined.data.materials[0] = mat
    out = export_fbx(joined, outname)
    return joined, out


def build_from_parts(base_out, parts, base_name_short):
    """parts: list of (suffix, matname, [objs]). Exports each as its own FBX
    named `<base_out>_<suffix>.fbx` and returns the list of (obj, path)."""
    results = []
    for suffix, matname, objs in parts:
        obj, path = finalize_part(objs, f"{base_out}_{suffix}", matname)
        results.append((obj, path))
    return results

# ----------------------------------------------------------------------------
# Size-class carcass + modular add-on system (Starfield/SE/X4-style)
# ----------------------------------------------------------------------------
# A ship is a CARCASS (base shell defining size/class) plus ADD-ON parts
# (engines, cargo, propulsion, weapons, sensor) that are SCALED to the
# carcass size class and mounted at its socket positions. Parts stay
# separate meshes with their own pivots + materials -> individually removable/
# destroyable, exactly like X4 components that are parameterized per class.

SIZE_CLASSES = {
    'small':   {'scale': 1.0,  'carcass': (250, 340, 95),  'z': 55},   # fighter
    'medium':  {'scale': 1.65, 'carcass': (380, 720, 190), 'z': 130},  # freighter
    'corvette':{'scale': 2.0,  'carcass': (460, 880, 220), 'z': 150},  # frigate/corvette
    'large':   {'scale': 2.3,  'carcass': (520, 980, 250), 'z': 175},  # capship-ish
}

def sc(sz, factor=1.0):
    """Size-class scalar — multiplies a base dimension by the class scale."""
    return SIZE_CLASSES[sz]['scale'] * factor

def scv(sz, x, y, z):
    """Scale a (x,y,z) size/location vector by the class."""
    k = SIZE_CLASSES[sz]['scale']
    return (x*k, y*k, z*k)


def build_carcass(sz):
    """Base shell for a size class. Returns (objs, base_pointers)."""
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']
    locz = s['z']
    hull = box(f"Carcass_{sz}_Hull", lx, ly, lz, loc=(0, 0, locz))
    bevel(hull, 8, 3)
    # dorsal spine so parts have a mounting rail
    spine = box(f"Carcass_{sz}_Spine", lx*0.32, ly*0.9, lz*0.28, loc=(0, 0, locz + lz*0.6))
    bevel(spine, 4, 2)
    return [hull, spine], (lx, ly, lz, locz)


def build_corvette_carcass(sz, outname):
    """Corvette carcass — longer, wedge-fore, twin-split aft (frigate profile)."""
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']
    locz = s['z']
    hull = box(f"Carcass_{sz}_Hull", lx, ly, lz, loc=(0, 0, locz)); bevel(hull, 8, 3)
    # wedge fore (narrowing bow)
    fore = box("Corv_Fore", lx*0.6, ly*0.3, lz*0.7, loc=(0, ly*0.62, locz)); bevel(fore, 5, 2)
    # raised command deck
    cmd = box("Corv_Cmd", lx*0.3, ly*0.28, lz*0.35, loc=(0, ly*0.2, locz + lz*0.6)); bevel(cmd, 4, 2)
    # twin-split aft prongs
    aft_l = box("Corv_AftL", lx*0.18, ly*0.28, lz*0.6, loc=(-lx*0.28, -ly*0.48, locz)); bevel(aft_l, 4, 2)
    aft_r = box("Corv_AftR", lx*0.18, ly*0.28, lz*0.6, loc=(lx*0.28, -ly*0.48, locz)); bevel(aft_r, 4, 2)
    # spine truss
    spine = box("Corv_Spine", lx*0.3, ly*0.86, lz*0.24, loc=(0, 0, locz + lz*0.62)); bevel(spine, 4, 2)
    objs = [hull, fore, cmd, aft_l, aft_r, spine]
    return finalize_part(objs, f"{outname}_Carcass", "M_Hull")


def build_mining_station_carcass(sz, outname, drill=True):
    """Mining-station carcass — a stumpy reactor-core body + big articulated
    drill rig / processing gantry."""
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']
    locz = s['z']
    body = box("Mine_Body", lx, ly, lz, loc=(0, 0, locz)); bevel(body, 8, 3)
    core = cyl("Mine_Core", lz*0.5, lz*1.6, loc=(0, 0, locz + lz*0.9), verts=20); bevel(core, 3, 2)
    parts = [body, core]
    # processing gantry / scaffolding around the body
    for side in (-1, 1):
        g = box(f"Mine_Gantry{side}", lx*0.12, ly*0.9, lz*1.4, loc=(side*lx*0.55, 0, locz + lz*0.3))
        bevel(g, 2, 1)
        parts.append(g)
    # big articulated drill arm (if drill=True)
    if drill:
        boom = box("Mine_Boom", lx*0.5, lx*0.5, lz*0.16, loc=(0, 0, locz - lz*0.2)); bevel(boom, 3, 2)
        drill_head = cone("Mine_Drill", lz*0.45, lz*1.1, loc=(0, 0, locz - lz*1.0), verts=16)
        parts += [boom, drill_head]
    return finalize_part(parts, f"{outname}_Carcass", "M_Station_Hab")


def build_engine_part(sz, outname):
    """Engine block add-on, scaled to carcass size class; twin nacelles + bells."""
    k = sc(sz)
    blk = box("Eng_Blk", 150*k, 200*k, 95*k, loc=(0, 0, 0))
    bevel(blk, 5, 2)
    parts = [blk]
    for side in (-1, 1):
        nac = box(f"Eng_Nac_{side}", 60*k, 150*k, 72*k, loc=(side*120*k, 5*k, -6*k),
                  rot=(0, 0, math.radians(8*side)))
        bevel(nac, 4, 1)
        noz = nozzle(f"Eng_Noz_{side}", 22*k, 55*k, loc=(side*120*k, -90*k, -20*k),
                     rot=(math.radians(90), 0, 0))
        parts += [nac, noz]
    joined = finalize_part(parts, outname, "M_Engine_Block", origin='ORIGIN_CENTER_OF_VOLUME')
    return joined


def build_cargo_part(sz, outname):
    """Cargo bay add-on: box + container cells scaled per class."""
    k = sc(sz)
    base = box("Cargo_Base", 200*k, 260*k, 90*k, loc=(0, 0, 0))
    bevel(base, 4, 2)
    parts = [base]
    for i, dx in enumerate(range(-60, 61, 60)):
        c = greeble(f"Cargo_Cell{i}", loc=(dx*k, -40*k, 30*k), sx=30*k, sy=70*k, sz=30*k)
        parts.append(c)
    return finalize_part(parts, outname, "M_Cargo_Hold", origin='ORIGIN_CENTER_OF_VOLUME')


def build_weapon_part(sz, outname, twin=True):
    """Weapon/cannon add-on, twin gun pods."""
    k = sc(sz)
    parts = []
    for side in (-1, 1) if twin else (0,):
        dom = greeble(f"Weapon_{side}", loc=(side*42*k, -6*k, 0), sx=16*k, sy=90*k, sz=16*k)
        barrel = cyl(f"Barrel_{side}", 5*k, 40*k, loc=(side*42*k, 60*k, 0),
                     rot=(math.radians(90), 0, 0), verts=10)
        parts += [dom, barrel]
    return finalize_part(parts, outname, "M_Weapon_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_sensor_part(sz, outname, asym=False):
    """Sensor array add-on: mast + tip + maybe radar dome."""
    k = sc(sz)
    mast = cyl("Sensor_Mast", 6*k, 140*k, loc=(0, 0, 90*k), verts=12)
    tip = sphere("Sensor_Tip", 9*k, loc=(0, 0, 170*k), verts=12)
    parts = [mast, tip]
    if asym:
        dome = sphere("Sensor_Dome", 14*k, loc=(60*k, 20*k, 10*k), verts=12)
        parts.append(dome)
    return finalize_part(parts, outname, "M_Sensor_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_reactor_part(sz, outname):
    """Reactor/core add-on from SE grid-discipline (a block module)."""
    k = sc(sz)
    core = box("Reactor_Core", 90*k, 90*k, 90*k, loc=(0, 0, 0))
    bevel(core, 5, 2)
    bands = [greeble(f"Reactor_Band{i}", loc=(0, 0, z*70*k), sx=100*k, sy=100*k, sz=14*k)
             for i, z in enumerate([-0.5, 0, 0.5])]
    return finalize_part([core] + bands, outname, "M_Reactor_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_mining_laser(sz, outname):
    """Mining laser / cutter add-on: emitter barrel + focusing optics + coolant."""
    k = sc(sz)
    barrel = cyl("Mining_Barrel", 10*k, 160*k, loc=(0, 130*k, 0),
                 rot=(math.radians(90), 0, 0), verts=14); bevel(barrel, 2, 1)
    emitter = box("Mining_Emitter", 26*k, 24*k, 30*k, loc=(0, 60*k, 0)); bevel(emitter, 3, 1)
    optics = torus("Mining_Optics", 14*k, 5*k, loc=(0, 210*k, 0), maj=16, minr=6)
    coolant_l = box("Mining_CoolL", 8*k, 120*k, 16*k, loc=(-26*k, 130*k, 0)); bevel(coolant_l, 2, 1)
    coolant_r = box("Mining_CoolR", 8*k, 120*k, 16*k, loc=(26*k, 130*k, 0)); bevel(coolant_r, 2, 1)
    return finalize_part([barrel, emitter, optics, coolant_l, coolant_r], outname,
                         "M_Weapon_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_drill_part(sz, outname):
    """Rotary drill rig add-on: heavy boom + spinning cutter head + housing."""
    k = sc(sz)
    boom = box("Drill_Boom", 26*k, 220*k, 34*k, loc=(0, -60*k, 0)); bevel(boom, 4, 2)
    head = cone("Drill_Head", 30*k, 90*k, loc=(0, -170*k, 0), verts=18); bevel(head, 2, 1)
    housing = box("Drill_Housing", 60*k, 50*k, 60*k, loc=(0, 20*k, 0)); bevel(housing, 5, 2)
    ring = torus("Drill_Ring", 26*k, 6*k, loc=(0, -130*k, 0), maj=20, minr=7)
    return finalize_part([boom, head, housing, ring], outname, "M_Station_Hab",
                         origin='ORIGIN_CENTER_OF_VOLUME')


def build_habitat_ring(sz, outname, dual=True):
    """Project-Hyperion-inspired artificial-gravity habitat ring add-on.

    A (double) torus of habitation segments on spokes — self-sustaining colony
    ring like the generation-ship designs. Dual = counter-rotating twin rings.
    """
    k = sc(sz)
    parts = []
    ring_count = 2 if dual else 1
    for r in range(ring_count):
        rr = 220*k + r*70*k
        ring = torus(f"HabRing{r}", rr, 26*k, loc=(0, 0, (r*4-2)*k), maj=44, minr=10)
        parts.append(ring)
        # habitation segment modules around the ring
        for s in range(6):
            ang = math.radians(s*60)
            cx, cy = math.cos(ang)*rr, math.sin(ang)*rr
            seg = box(f"HabSeg{r}_{s}", 30*k, 44*k, 44*k,
                      loc=(cx, cy, (r*4-2)*k + 28*k), rot=(0, 0, math.radians(s*60)))
            bevel(seg, 3, 2)
            parts.append(seg)
    # spokes connecting rings to center
    for s in range(6):
        ang = math.radians(s*60+30)
        cx, cy = math.cos(ang)*200*k, math.sin(ang)*200*k
        spoke = box(f"Spoke{s}", 34*k, 440*k, 26*k, loc=(0, 0, 0),
                    rot=(0, 0, math.radians(s*60+30)))
        bevel(spoke, 2, 1)
        parts.append(spoke)
    return finalize_part(parts, outname, "M_Habitat_Ring", origin='ORIGIN_CENTER_OF_VOLUME')


def build_asteroid_shell(sz, outname):
    """Project-Hyperion-inspired hollowed-asteroid station shell.

    Repurposed asteroid as protective radiation shell + rotational structure,
    with an interior docking well.
    """
    k = sc(sz)
    rock_obj = rock("ShellRock", 240*k, loc=(0, 0, 0), sub=2, scale_xyz=(1.3, 1.0, 0.9))
    # keep the asteroid, add a hollowed rim well + hull plate rim
    well = torus("WellRim", 130*k, 22*k, loc=(0, 0, -40*k), maj=36, minr=9)
    band = greeble("ShellBand", loc=(0, 0, 120*k), sx=320*k, sy=320*k, sz=40*k)
    return finalize_part([rock_obj, well, band], outname, "M_Asteroid_Shell",
                         origin='ORIGIN_CENTER_OF_VOLUME')


def assemble_ship(sz, outname, opts, carcass_builder=None):
    """Build a full ship: carcass + mounted add-ons. Each part stays a separate
    mesh exported as its own FBX (`<outname>_<Part>.fbx`). opts may select which
    add-ons to include (engines/cargo/weapons/sensor/reactor/drill/mining_laser).
    carcass_builder (optional) overrides the default size-class carcass.
    Returns list of (obj, fbxpath)."""
    results = []
    # carcass
    if carcass_builder is not None:
        obj, path = carcass_builder(sz, outname)
        results.append((obj, path))
        # derive dims from the carcass mesh bounds
        vmin = Vector((1e9,1e9,1e9)); vmax = Vector((-1e9,-1e9,-1e9))
        msh = [o for o in bpy.data.objects if o.type=='MESH']
        for o in msh:
            for v in o.data.vertices:
                w = o.matrix_world @ v.co
                for i in range(3):
                    vmin[i]=min(vmin[i],w[i]); vmax[i]=max(vmax[i],w[i])
        dims = (vmax[0]-vmin[0], vmax[1]-vmin[1], vmax[2]-vmin[2], (vmin[2]+vmax[2])/2)
        k = sc(sz)
        ly = dims[1]; locz = dims[3]
    else:
        carc_objs, dims = build_carcass(sz)
        obj, path = finalize_part(carc_objs, f"{outname}_Carcass", "M_Hull")
        results.append((obj, path))
        k = sc(sz)
        _, ly, _, locz = dims

    if opts.get('engine'):
        eobj, ep = build_engine_part(sz, f"{outname}_Engine")
        eobj.location = (0, -ly*0.52, locz - 10*k)
        results.append((eobj, ep))
    if opts.get('cargo'):
        cobj, cp = build_cargo_part(sz, f"{outname}_Cargo")
        cobj.location = (0, -ly*0.05, locz - 20*k)
        results.append((cobj, cp))
    if opts.get('weapon'):
        wobj, wp = build_weapon_part(sz, f"{outname}_Weapon", twin=opts.get('weapon_twin', True))
        wobj.location = (0, ly*0.34, locz - 25*k)
        results.append((wobj, wp))
    if opts.get('sensor'):
        sobj, sp = build_sensor_part(sz, f"{outname}_Sensor", asym=opts.get('sensor_asym', False))
        sobj.location = (0, ly*0.18, locz + 70*k)
        results.append((sobj, sp))
    if opts.get('reactor'):
        robj, rp = build_reactor_part(sz, f"{outname}_Reactor")
        robj.location = (0, -ly*0.35, locz + 40*k)
        results.append((robj, rp))
    if opts.get('mining_laser'):
        mj, mp = build_mining_laser(sz, f"{outname}_MiningLaser")
        mj.location = (0, ly*0.3, locz - 10*k)
        results.append((mj, mp))
    if opts.get('drill'):
        dj, dp = build_drill_part(sz, f"{outname}_Drill")
        dj.location = (0, -ly*0.3, locz - 15*k)
        results.append((dj, dp))
    if opts.get('habitat_ring'):
        robj, rp = build_habitat_ring(sz, f"{outname}_HabRing",
                                      dual=opts.get('ring_dual', True))
        robj.location = (0, 0, locz)
        results.append((robj, rp))
    if opts.get('asteroid_shell'):
        aobj, ap = build_asteroid_shell(sz, f"{outname}_AsteroidShell")
        aobj.location = (0, -ly*0.2, locz)
        results.append((aobj, ap))
    return results


# ----------------------------------------------------------------------------
# Assets
# ----------------------------------------------------------------------------
def build_fighter():
    """SM_Ship_Fighter_01 - small interceptor fighter, single-seat."""
    clear_scene()
    # Type: repurposed cargo/courier shuttle, agile, ~9m
    # fuselage
    body = box("BodyHull", 230, 320, 90, loc=(0, 0, 60))          # main tube
    nose = cone("Nose", 55, 180, loc=(0, 140, 60), rot=(math.radians(90), 0, 0))  # pointed nose -Y
    bevel(body, 6, 2); bevel(nose, 2, 1)
    # cockpit canopy
    canopy = sphere("Canopy", 40, loc=(0, 30, 130), verts=20)
    canopy.scale = (0.7, 1.1, 0.7)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    # wings (canards) - swept
    wing = box("Wing", 340, 60, 12, loc=(0, 30, 55), rot=(0, 0, math.radians(8)))
    # wingtip missile pods
    wt1 = cyl("TipPodL", 9, 70, loc=(-172, 15, 52), rot=(math.radians(90), 0, 0), verts=12)
    wt2 = cyl("TipPodR", 9, 70, loc=(172, 15, 52), rot=(math.radians(90), 0, 0), verts=12)
    # twin engine nacelles
    eng = box("Eng1", 70, 180, 75, loc=(-95, -150, 55), rot=(math.radians(12), 0, 0))
    eng2 = box("Eng2", 70, 180, 75, loc=(95, -150, 55), rot=(math.radians(12), 0, 0))
    # thruster nozzle bells on each nacelle
    noz1 = nozzle("Noz1", 16, 40, loc=(-95, -70, 38), rot=(math.radians(90), 0, 0))
    noz2 = nozzle("Noz2", 16, 40, loc=(95, -70, 38), rot=(math.radians(90), 0, 0))
    # pylons
    p1 = box("Pylon1", 14, 200, 20, loc=(-150, -20, 45), rot=(0, 0, math.radians(60)))
    p2 = box("Pylon2", 14, 200, 20, loc=(150, -20, 45), rot=(0, 0, math.radians(60)))
    # tail fin & ventral keel
    tail = box("Tail", 30, 150, 110, loc=(0, -150, 150), rot=(math.radians(-10), 0, 0))
    keel = box("Keel", 30, 130, 30, loc=(0, -120, 10))
    # hull greebles: dorsal sensor bumps + cockpit frame + antenna
    g_s1 = greeble("Greeble1", loc=(0, 40, 112), sx=26, sy=16, sz=6)
    g_s2 = greeble("Greeble2", loc=(-40, 100, 108), sx=18, sy=14, sz=5)
    g_s3 = greeble("Greeble3", loc=(40, 100, 108), sx=18, sy=14, sz=5)
    frame = torus("CockpitFrame", 44, 4, loc=(0, 28, 130), rot=(math.radians(90), 0, 0), maj=24, minr=6)
    antenna = cyl("Ant", 3, 40, loc=(0, 90, 148), verts=10)
    tip_ant = sphere("AntTip", 5, loc=(0, 90, 170), verts=10)
    for ob in (nose, canopy, wing, eng, eng2, p1, p2, tail, keel, wt1, wt2, frame, antenna):
        bevel(ob, 4, 1)

    # X4-style: separate component meshes, each with own pivot + material slot
    return build_from_parts("SM_Ship_Fighter_01", [
        ("Hull",     "M_Fighter_Hull",  [body, nose, wing, p1, p2, tail, keel, g_s1, g_s2, g_s3,
                                         wt1, wt2]),
        ("Canopy",   "M_Fighter_Glass", [canopy, frame]),
        ("Engine",   "M_Fighter_Engine",[eng, eng2, noz1, noz2]),
        ("Sensor",   "M_Fighter_Accent",[antenna, tip_ant]),
    ], "fighter")

def build_freighter():
    """SM_Ship_Freighter_01 - medium cargo hauler ~26m."""
    clear_scene()
    # box-ish hull with engine block
    hull = box("Hull", 380, 720, 190, loc=(0, 0, 130))
    bevel(hull, 10, 3)
    # superstructure deck
    deck = box("Deck", 200, 360, 130, loc=(0, 40, 260))
    bevel(deck, 6, 2)
    # bridge block
    bridge = box("Bridge", 120, 150, 80, loc=(0, 200, 330))
    # fore cargo hold
    hold = box("Hold", 250, 420, 150, loc=(0, 60, 70))
    # cargo container cells (grid of small boxes on the hold flanks)
    containers = []
    for i, (ox, oy, oz) in enumerate([
            (-150, 150, 60), (-150, 150, 105), (-150, 150, 150),
            (150, 150, 60), (150, 150, 105), (150, 150, 150),
            (-150, -40, 60), (-150, -40, 105), (-150, -40, 150),
            (150, -40, 60), (150, -40, 105), (150, -40, 150),
            (-150, 60, 60), (150, 60, 60)]):
        c = greeble(f"Container{i}", loc=(ox, oy, oz), sx=34, sy=60, sz=30)
        containers.append(c)
    # engine cluster rear
    e1 = cyl("Engine1", 50, 120, loc=(-120, -330, 130), rot=(math.radians(90), 0, 0))
    e2 = cyl("Engine2", 50, 120, loc=(120, -330, 130), rot=(math.radians(90), 0, 0))
    e3 = cyl("Engine3", 34, 90, loc=(-40, -330, 220), rot=(math.radians(90), 0, 0))
    e4 = cyl("Engine4", 34, 90, loc=(40, -330, 220), rot=(math.radians(90), 0, 0))
    # engine nozzle bells
    noz = [nozzle(f"FNoz{i}", 20, 45, loc=(x, -270, z), rot=(math.radians(90), 0, 0))
           for i, (x, z) in enumerate([(-120, 130), (120, 130), (-40, 220), (40, 220)])]
    # wing stabilizers
    st1 = box("Stab1", 320, 60, 14, loc=(-180, -120, 120), rot=(0, 0, math.radians(6)))
    st2 = box("Stab2", 320, 60, 14, loc=(180, -120, 120), rot=(0, 0, math.radians(6)))
    # tail boom
    boom = box("Boom", 40, 200, 40, loc=(0, -370, 120))
    # bridge antenna array + sensor bumps
    bant = cyl("BridgeAnt", 4, 60, loc=(0, 230, 390), verts=10)
    banttip = sphere("BridgeAntTip", 6, loc=(0, 230, 425), verts=10)
    g_d1 = greeble("DeckGreeble", loc=(-40, 90, 330), sx=20, sy=16, sz=6)
    g_d2 = greeble("DeckGreeble2", loc=(40, 90, 330), sx=20, sy=16, sz=6)
    for ob in (deck, bridge, hold, st1, st2, boom):
        bevel(ob, 5, 2)
    for e in (e1, e2, e3, e4):
        bevel(e, 3, 1)
    joined = join([hull, deck, bridge, hold, e1, e2, e3, e4, st1, st2, boom,
                   bant, banttip, g_d1, g_d2] + containers + noz, "FreighterGeo")
    apply_mods(joined)
    clean_mesh(joined)
    smart_uv(joined)
    joined.name = "SM_Ship_Freighter_01"
    m = bpy.data.materials.new("M_Freighter_Hull")
    m.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m)
    return joined

def build_props():
    """SM_Prop_* - shipping crate + hazardous container."""
    results = []
    # -- crate --
    clear_scene()
    crate = box("Crate", 100, 100, 100)
    bevel(crate, 4, 2)
    r1 = box("Rebar1", 12, 96, 96, loc=(0, 0, 0), rot=(math.radians(45), 0, 0))
    r2 = box("Rebar2", 12, 96, 96, loc=(0, 0, 0), rot=(math.radians(-45), 0, 0))
    for ob in (r1, r2):
        bevel(ob, 1, 1)
    joined = join([crate, r1, r2], "PropCrate")
    apply_mods(joined)
    clean_mesh(joined)
    smart_uv(joined)
    joined.name = "SM_Prop_Crate_Container_01"
    m = bpy.data.materials.new("M_Prop_Crate")
    m.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m)
    out1 = export_fbx(joined, "SM_Prop_Crate_Container_01")
    results.append((joined, out1))

    # -- hazardous container --
    clear_scene()
    tank = cyl("Tank", 28, 140, loc=(0, 0, 60), verts=24)
    bevel(tank, 2, 1)
    cap1 = sphere("Cap1", 28, loc=(0, 0, 122), verts=20)
    cap1.scale = (1, 1, 0.4)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    cap2 = sphere("Cap2", 28, loc=(0, 0, -2), verts=20)
    cap2.scale = (1, 1, 0.4)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    cradle = box("Cradle", 70, 140, 16, loc=(0, 0, -4))
    bevel(cradle, 3, 1)
    joined = join([tank, cap1, cap2, cradle], "PropHazard")
    apply_mods(joined)
    clean_mesh(joined)
    smart_uv(joined)
    joined.name = "SM_Prop_Container_Hazard_02"
    m2 = bpy.data.materials.new("M_Prop_Tank")
    m2.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m2)
    out2 = export_fbx(joined, "SM_Prop_Container_Hazard_02")
    results.append((joined, out2))
    return results

# ----------------------------------------------------------------------------
def build_gunship():
    """SM_Ship_Gunship_02 - rugged angular smuggler gunship (cyberpunk)."""
    clear_scene()
    hull = box("Hull", 220, 430, 110, loc=(0, 0, 55)); bevel(hull, 6, 2)
    nose = box("Anose", 80, 130, 70, loc=(0, 210, 55)); bevel(nose, 4, 2)
    # asymmetric port sensor mast (cyberpunk asymmetric silhouette)
    mast = cyl("Mast", 8, 200, loc=(-100, 90, 175), verts=14); bevel(mast, 1, 1)
    tip = sphere("MastTip", 12, loc=(-100, 90, 265), verts=12)
    # angular stub wings + gun pods
    wl = box("WingL", 170, 50, 18, loc=(-165, 30, 45), rot=(0, 0, math.radians(-16)))
    wr = box("WingR", 170, 50, 18, loc=(165, 30, 45), rot=(0, 0, math.radians(16)))
    g1 = box("Gun1", 14, 90, 14, loc=(-185, 10, 38)); g2 = box("Gun2", 14, 90, 14, loc=(185, 10, 38))
    # quad gun cluster under nose
    g3 = greeble("Gun3", loc=(-12, 240, 40), sx=8, sy=56, sz=8)
    g4 = greeble("Gun4", loc=(12, 240, 40), sx=8, sy=56, sz=8)
    # twin engine nozzles + bells
    e1 = cyl("Eng1", 32, 90, loc=(-62, -215, 55), rot=(math.radians(90), 0, 0), verts=20)
    e2 = cyl("Eng2", 32, 90, loc=(62, -215, 55), rot=(math.radians(90), 0, 0), verts=20)
    nzL = nozzle("NzL", 15, 40, loc=(-62, -170, 55), rot=(math.radians(90), 0, 0))
    nzR = nozzle("NzR", 15, 40, loc=(62, -170, 55), rot=(math.radians(90), 0, 0))
    # hull greeble ridges + starboard radar dome (asymmetry theme)
    g_g1 = greeble("HullRidge1", loc=(-70, 60, 118), sx=40, sy=14, sz=6)
    g_g2 = greeble("HullRidge2", loc=(70, 60, 118), sx=40, sy=14, sz=6)
    g_g3 = greeble("HullRidge3", loc=(0, -20, 118), sx=50, sy=14, sz=6)
    dome = sphere("RadarDome", 14, loc=(90, 150, 120), verts=12)
    g_g4 = greeble("StarboardGreeble", loc=(85, 190, 80), sx=12, sy=10, sz=8)
    # canted tail fins
    t1 = box("TailL", 14, 70, 180, loc=(-32, -210, 165), rot=(0, 0, math.radians(15)))
    t2 = box("TailR", 14, 70, 180, loc=(32, -210, 165), rot=(0, 0, math.radians(-15)))
    for ob in (hull, nose, mast, wl, wr, g1, g2, t1, t2):
        bevel(ob, 4, 1)
    joined = join([hull, nose, mast, tip, wl, wr, g1, g2, e1, e2, t1, t2,
                   g3, g4, nzL, nzR, g_g1, g_g2, g_g3, g_g4, dome], "GunshipGeo")
    apply_mods(joined); clean_mesh(joined); smart_uv(joined)
    joined.name = "SM_Ship_Gunship_02"
    m = bpy.data.materials.new("M_Gunship_Hull")
    m.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m)
    return joined


def build_station():
    """SM_Station_Habitation_01 - vertical cyberpunk megabuilding station."""
    clear_scene()
    # central spine
    spine = cyl("Spine", 42, 900, loc=(0, 0, 450), verts=20); bevel(spine, 4, 2)
    # stacked habitation decks with setbacks (megatower profile)
    habs = []
    specs = [
        (300, 220, 170, 430),   # x, y, z height, base z
        (260, 185, 140, 600),
        (220, 160, 120, 740),
        (185, 135, 110, 860),
        (145, 115, 100, 970),
        (115, 95,  90, 1070),
    ]
    for i, (sx, sy, sz, z0) in enumerate(specs):
        hb = box(f"Deck{i}", sx, sy, sz, loc=(0, 0, z0 + sz/2)); bevel(hb, 6, 3)
        habs.append(hb)
    # crowning spire + antenna
    spr = cone("Spire", 42, 240, loc=(0, 0, 1250), verts=16); bevel(spr, 2, 1)
    ant = cyl("Antenna", 6, 220, loc=(0, 0, 1400), verts=12)
    dish = sphere("Dish", 20, loc=(0, 0, 1590), verts=16)
    # docking ring (torus) + 4 cardinal spoke pylons
    ring = torus("DockingRing", 330, 28, loc=(0, 0, 270), maj=48, minr=12)
    spoke_ew1 = box("SpokeE", 620, 80, 80, loc=(330, 0, 270)); bevel(spoke_ew1, 4, 2)
    spoke_ew2 = box("SpokeW", 620, 80, 80, loc=(-330, 0, 270)); bevel(spoke_ew2, 4, 2)
    spoke_ns1 = box("SpokeN", 80, 620, 80, loc=(0, 330, 270)); bevel(spoke_ns1, 4, 2)
    spoke_ns2 = box("SpokeS", 80, 620, 80, loc=(0, -330, 270)); bevel(spoke_ns2, 4, 2)
    # docking pods hung off the ring (8 habitat modules)
    pods = []
    for k in range(8):
        ang = math.radians(k * 45)
        cx, cy = math.cos(ang) * 330, math.sin(ang) * 330
        p = box(f"DockPod{k}", 70, 40, 46, loc=(cx, cy, 270 + 30),
                rot=(0, 0, math.radians(k * 45)))
        bevel(p, 3, 2)
        pods.append(p)
    # per-deck edge rings (meter-high trims around each deck)
    deck_rings = []
    for i, (sx, sy, sz, z0) in enumerate(specs):
        r = box(f"DeckRing{i}", sx + 12, sy + 12, 14, loc=(0, 0, z0 + sz - 7))
        bevel(r, 2, 1)
        deck_rings.append(r)
    # solar array wings + subdivided cell frame
    p1 = box("Solar1A", 24, 220, 160, loc=(-260, 0, 560))
    p2 = box("Solar2A", 24, 220, 160, loc=(260, 0, 560))
    s_frame1 = box("SolarFrame1", 18, 220, 160, loc=(-260, 0, 560))
    s_frame2 = box("SolarFrame2", 18, 220, 160, loc=(260, 0, 560))
    solar_cells = []
    for dx in range(-120, 121, 60):
        for dz in range(-60, 61, 60):
            sc1 = greeble(f"SolarCellL{dx}_{dz}", loc=(-260, dx, 560 + dz), sx=8, sy=56, sz=8)
            sc2 = greeble(f"SolarCellR{dx}_{dz}", loc=(260, dx, 560 + dz), sx=8, sy=56, sz=8)
            solar_cells.append(sc1); solar_cells.append(sc2)
    # dish relay array under the base deck
    relay = []
    for k, (rx, ry) in enumerate([(-150, -150), (150, -150), (-150, 150), (150, 150)]):
        d = torus(f"RelayT{k}", 26, 6, loc=(rx, ry, 300), maj=20, minr=5)
        mast2 = cyl(f"RelayM{k}", 4, 60, loc=(rx, ry, 380), verts=10)
        relay.append(d); relay.append(mast2)
    joined = join([spine] + habs + deck_rings + [spr, ant, dish, ring,
                                                 spoke_ew1, spoke_ew2, spoke_ns1, spoke_ns2,
                                                 p1, p2, s_frame1, s_frame2] +
                  pods + solar_cells + relay, "StationGeo")
    apply_mods(joined); clean_mesh(joined); smart_uv(joined)
    joined.name = "SM_Station_Habitation_01"
    m = bpy.data.materials.new("M_Station_Hab")
    m.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m)
    return joined


def build_extra_objects():
    """SM_Obj_* - satellite, asteroid derelict, fuel battery, comm tower."""
    results = []
    # -- communications satellite --
    clear_scene()
    sat = box("SatBody", 60, 60, 60); bevel(sat, 3, 1)
    pan1 = box("SatPanel1", 12, 240, 130, loc=(0, 290, 0)); bevel(pan1, 1, 1)
    pan2 = box("SatPanel2", 12, 240, 130, loc=(0, -290, 0)); bevel(pan2, 1, 1)
    # panel cell grid on each wing
    s_cells = []
    for dx in range(-100, 101, 50):
        for dz in range(-50, 51, 50):
            c1 = greeble(f"SatCellL{dx}_{dz}", loc=(0, 290, dz), sx=6, sy=48, sz=6)
            c2 = greeble(f"SatCellR{dx}_{dz}", loc=(0, -290, dz), sx=6, sy=48, sz=6)
            s_cells += [c1, c2]
    ant = cyl("SatAnt", 4, 90, loc=(0, 0, 100), verts=12)
    d1 = sphere("SatDish", 16, loc=(0, 0, 145), verts=12); d1.scale = (0.4, 1, 1)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    # 4 thruster nozzles around body
    thr = [nozzle(f"SatThr{n}", 5, 14, loc=(x, y, 5), rot=(0, 0, math.radians(n*90)))
           for n, (x, y) in enumerate([(28, 0), (-28, 0), (0, 28), (0, -28)])]
    j = join([sat, pan1, pan2, ant, d1] + s_cells + thr, "SatGeo")
    apply_mods(j); clean_mesh(j); smart_uv(j)
    j.name = "SM_Obj_Satellite_01"
    m = bpy.data.materials.new("M_Satellite"); m.use_nodes = True
    if not j.data.materials: j.data.materials.append(m)
    out = export_fbx(j, "SM_Obj_Satellite_01"); results.append((j, out))

    # -- asteroid with smashed derelict wreckage --
    clear_scene()
    ast = rock("Asteroid", 170, loc=(0, 0, 50), sub=2, scale_xyz=(1.3, 1.0, 0.85))
    hull = box("HullOff", 90, 150, 80, loc=(130, 60, 95), rot=(0.35, 0.5, 0.4)); bevel(hull, 4, 1)
    mast = cyl("WreckMast", 5, 130, loc=(-145, 70, 150), rot=(0.6, 0.2, 0.9), verts=10)
    panel = box("WreckPanel", 8, 70, 45, loc=(-145, 110, 205), rot=(0.8, 0.3, 0.5))
    debris = rock("Debris", 28, loc=(95, -160, 25), sub=1, scale_xyz=(0.7, 1.0, 0.6))
    debris2 = rock("Debris2", 18, loc=(-60, -200, 90), sub=1, scale_xyz=(0.6, 0.9, 0.7))
    debris3 = rock("Debris3", 12, loc=(190, -40, 30), sub=1, scale_xyz=(1.0, 0.6, 0.7))
    wreck_plate = greeble("WreckPlate", loc=(120, 110, 150), sx=34, sy=20, sz=6, rot=(0.3, 0.4, 0.2))
    j = join([ast, hull, mast, panel, debris, debris2, debris3, wreck_plate], "DerelictGeo")
    apply_mods(j); clean_mesh(j); smart_uv(j)
    j.name = "SM_Obj_AsteroidDerelict_01"
    m = bpy.data.materials.new("M_Derelict"); m.use_nodes = True
    if not j.data.materials: j.data.materials.append(m)
    out = export_fbx(j, "SM_Obj_AsteroidDerelict_01"); results.append((j, out))

    # -- fuel storage battery /
    clear_scene()
    tanks = []
    for i in range(3):
        t = cyl(f"FuelTank{i}", 34, 150, loc=(-40 + i*40, 0, 100), verts=14)
        bevel(t, 2, 1)
        for side in (170, -8):
            cap = sphere(f"Cap{i}_{side}", 34, loc=(-40+i*40, 0, side+75), verts=10)
            cap.scale = (1, 1, 0.5); bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
            tanks.append(cap)
        tanks.append(t)
    base = box("TankBase", 180, 200, 20, loc=(0, 0, 0)); bevel(base, 3, 1)
    # pipe manifold + valve nozzles on each tank top
    pipe = box("Manifold", 150, 10, 10, loc=(0, 0, 178)); bevel(pipe, 1, 1)
    valves = []
    for i in range(3):
        v = nozzle(f"Valve{i}", 5, 16, loc=(-40 + i*40, 0, 190), rot=(0, 0, 0))
        valves.append(v)
    # cradle pads under base
    bump = greeble("BaseBump1", loc=(-70, -80, 12), sx=30, sy=30, sz=8)
    bump2 = greeble("BaseBump2", loc=(70, -80, 12), sx=30, sy=30, sz=8)
    j = join(tanks + [base, pipe] + valves + [bump, bump2], "FuelGeo")
    apply_mods(j); clean_mesh(j); smart_uv(j)
    j.name = "SM_Obj_FuelCell_01"
    m = bpy.data.materials.new("M_FuelCell"); m.use_nodes = True
    if not j.data.materials: j.data.materials.append(m)
    out = export_fbx(j, "SM_Obj_FuelCell_01"); results.append((j, out))

    # -- comms relay tower --
    clear_scene()
    tbase = box("TowerBase", 70, 70, 34, loc=(0, 0, 17)); bevel(tbase, 4, 2)
    tmast = cyl("TowerMast", 13, 380, loc=(0, 0, 230), verts=16)
    platform = box("TowerPlatform", 90, 90, 16, loc=(0, 0, 400)); bevel(platform, 3, 2)
    cross1 = box("CrossArm1", 150, 6, 6, loc=(-30, 0, 320))
    cross2 = box("CrossArm2", 6, 150, 6, loc=(0, -30, 320))
    mid_act = cyl("MidAct", 10, 24, loc=(0, 0, 330), verts=12)
    for k in range(5):
        d = sphere(f"TowerDish{k}", 24, loc=(-45 + k*22, 0, 435), verts=12)
        d.scale = (0.5, 1.15, 1.0); bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    beacon = cone("Beacon", 16, 60, loc=(0, 0, 460), verts=12); bevel(beacon, 2, 1)
    # collect dishes by name
    dishes = [o for o in bpy.data.objects if o.name.startswith("TowerDish")]
    j = join([tbase, tmast, platform, cross1, cross2, mid_act] + list(dishes) + [beacon], "TowerGeo")
    apply_mods(j); clean_mesh(j); smart_uv(j)
    j.name = "SM_Obj_CommsTower_01"
    m = bpy.data.materials.new("M_CommsTower"); m.use_nodes = True
    if not j.data.materials: j.data.materials.append(m)
    out = export_fbx(j, "SM_Obj_CommsTower_01"); results.append((j, out))
    return results


# ----------------------------------------------------------------------------
# Interior design — separate instances entered when leaving the cockpit.
# ----------------------------------------------------------------------------
# Each ship's interior is its OWN asset, distinct from the exterior hull. It's
# a walkable interior shell (floor/walls/ceiling) + set dressing (decks, panels,
# consoles, beds, shelves, light strips) sized to the ship class, exported as a
# separate FBX (`SM_Int_*`). The gameplay hook: when the pilot leaves the cockpit,
# the interior instance is shown instead of the exterior.

def interior_shell(sz, parts, outname, floor_tex="M_Interior_Deck",
                   accent_tex="M_Interior_Accent"):
    """Join an interior's floor/walls/etc into one finalized part."""
    return finalize_part(parts, outname, floor_tex)


def build_cockpit_interior(sz, outname):
    """Cockpit interior — small flight deck shell + proper pilot station.

    Enhanced set dressing vs v1 (addresses vision review): full pilot seat
    (base + back + headrest + armrests), two-tier control console with a
    main screen + secondary panel, overhead console, yoke + throttle,
    pedals, and side-wall control banks.
    """
    k = sc(sz)
    parts = []
    # floor
    floor = box("Floor", 200*k, 240*k, 8*k, loc=(0, 0, 0)); bevel(floor, 2, 1)
    parts.append(floor)
    # low walls / bulkheads (open-top glass ring style)
    for side in (-1, 1):
        w = box(f"Wall{side}", 8*k, 240*k, 90*k, loc=(side*104*k, 0, 45*k))
        bevel(w, 2, 1); parts.append(w)
    wf = box("WallFront", 200*k, 8*k, 90*k, loc=(0, 120*k, 45*k))
    bevel(wf, 2, 1); parts.append(wf)
    wb = box("WallBack", 200*k, 8*k, 90*k, loc=(0, -120*k, 45*k))
    bevel(wb, 2, 1); parts.append(wb)

    # ---- pilot seat (full: base/back/headrest/armrests) ----
    seat_base = box("SeatBase", 40*k, 40*k, 16*k, loc=(0, -30*k, 8*k)); bevel(seat_base, 3, 1)
    seat_cush = box("SeatCush", 34*k, 30*k, 10*k, loc=(0, -28*k, 20*k)); bevel(seat_cush, 3, 1)
    seat_back = box("SeatBack", 34*k, 10*k, 46*k, loc=(0, -48*k, 42*k)); bevel(seat_back, 3, 1)
    headrest = box("Headrest", 26*k, 10*k, 14*k, loc=(0, -48*k, 72*k)); bevel(headrest, 3, 1)
    # armrests: clearly raised, thicker, forward-pointing arms offset from back
    arm_l = box("ArmL", 8*k, 30*k, 12*k, loc=(-22*k, -40*k, 22*k)); bevel(arm_l, 2, 1)
    arm_r = box("ArmR", 8*k, 30*k, 12*k, loc=(22*k, -40*k, 22*k)); bevel(arm_r, 2, 1)
    # broad pads on top so arms read distinctly from the backrest
    pad_l = box("ArmPadL", 10*k, 24*k, 6*k, loc=(-22*k, -38*k, 30*k)); bevel(pad_l, 2, 1)
    pad_r = box("ArmPadR", 10*k, 24*k, 6*k, loc=(22*k, -38*k, 30*k)); bevel(pad_r, 2, 1)
    parts += [seat_base, seat_cush, seat_back, headrest, arm_l, arm_r, pad_l, pad_r]

    # ---- control console: two tiers + main/secondary screens ----
    console_lo = box("ConsoleLo", 90*k, 24*k, 20*k, loc=(0, 78*k, 10*k)); bevel(console_lo, 2, 1)
    console_hi = box("ConsoleHi", 70*k, 14*k, 26*k, loc=(0, 72*k, 34*k)); bevel(console_hi, 2, 1)
    main_screen = box("MainScreen", 46*k, 4*k, 14*k, loc=(0, 66*k, 56*k))   # emissive HUD
    sec_screen = box("SecScreen", 22*k, 4*k, 10*k, loc=(-26*k, 70*k, 50*k))
    parts += [console_lo, console_hi, main_screen, sec_screen]

    # ---- controls: yoke, throttle, pedals ----
    yoke = box("Yoke", 10*k, 14*k, 10*k, loc=(0, 58*k, 28*k)); bevel(yoke, 2, 1)
    throttle = box("Throttle", 6*k, 10*k, 14*k, loc=(24*k, 70*k, 20*k)); bevel(throttle, 1, 1)
    pedal_l = box("PedalL", 6*k, 4*k, 12*k, loc=(-14*k, 56*k, 4*k))
    pedal_r = box("PedalR", 6*k, 4*k, 12*k, loc=(14*k, 56*k, 4*k))
    parts += [yoke, throttle, pedal_l, pedal_r]

    # ---- overhead console + canopy framing ring + light strip ----
    overhead = box("Overhead", 60*k, 16*k, 8*k, loc=(0, 20*k, 88*k)); bevel(overhead, 2, 1)
    frame = torus("CanopyRing", 96*k, 6*k, loc=(0, 0, 95*k), maj=28, minr=7)
    light = box("LightStrip", 160*k, 6*k, 4*k, loc=(0, 0, 118*k))
    parts += [overhead, frame, light]

    # ---- side-wall control banks (starboard/port) ----
    bank_l = box("PanelBankL", 4*k, 40*k, 22*k, loc=(-102*k, 30*k, 30*k)); bevel(bank_l, 1, 1)
    bank_r = box("PanelBankR", 4*k, 40*k, 22*k, loc=(102*k, 30*k, 30*k)); bevel(bank_r, 1, 1)
    parts += [bank_l, bank_r]

    jo, out = finalize_part(parts, outname, "M_Interior_Cockpit")
    return [(jo, out)]


def build_hab_interior(sz, outname, room_count=1):
    """Crew-quarters / hab interior module — floor, walls, ceiling, bunks,
    desks, lockers, central table, light fixtures, paneling.

    Enhanced set dressing vs v1 (addresses vision review): two-tier bunks with
    ladders + pillows, defined desk consoles with screens, floor lockers, a
    central mess table, wall-mounted light fixtures, and wall paneling.
    """
    k = sc(sz)
    parts = []
    L = 300*k
    W = 140*k
    H = 110*k
    # open shell: floor + 2 long walls (front/back open for modularity)
    floor = box("HabFloor", L, W, 8*k, loc=(0, 0, 0)); bevel(floor, 2, 1); parts.append(floor)
    w_l = box("HabWallL", 8*k, W, H, loc=(-L/2, 0, H/2)); bevel(w_l, 2, 1)
    w_r = box("HabWallR", 8*k, W, H, loc=(L/2, 0, H/2)); bevel(w_r, 2, 1)
    parts += [w_l, w_r]
    # ceiling
    ceil = box("HabCeil", L, W, 6*k, loc=(0, 0, H)); bevel(ceil, 2, 1); parts.append(ceil)
    # wall paneling strips (vertical, on the long walls)
    for side in (-1, 1):
        for y in range(int(-W/2)+12, int(W/2), 24):
            pnl = box(f"Panel{side}_{y}", 3*k, 16*k, 40*k, loc=(side*(L/2-6*k), y, 45*k))
            parts.append(pnl)
    # ceiling light fixtures (recessed panels + glow strips)
    for x in (-L/4, 0, L/4):
        ls = box(f"Light{x}", 40*k, 6*k, 4*k, loc=(x, 0, H-6*k))
        lf = box(f"LightFit{x}", 46*k, 10*k, 6*k, loc=(x, 0, H-8*k))  # fixture frame
        parts += [ls, lf]

    # two-tier bunks (upper + lower) with ladders, pillows
    for i in range(max(1, room_count*2)):
        ox = -L/2 + 80*k + i*80*k
        # lower bunk
        b_low_base = box(f"BunkLoBase{i}", 40*k, 60*k, 14*k, loc=(ox, -40*k, 7*k)); bevel(b_low_base, 2, 1)
        b_low_matt = box(f"BunkLoMatt{i}", 34*k, 54*k, 6*k, loc=(ox, -40*k, 17*k))  # mattress
        # upper bunk
        b_hi_base = box(f"BunkHiBase{i}", 40*k, 60*k, 14*k, loc=(ox, -40*k, 48*k)); bevel(b_hi_base, 2, 1)
        b_hi_matt = box(f"BunkHiMatt{i}", 34*k, 54*k, 6*k, loc=(ox, -40*k, 58*k))
        # vertical frame corners
        for dx, dy in ((-20*k, -30*k), (20*k, -30*k), (-20*k, 30*k), (20*k, 30*k)):
            post = box(f"Post{i}_{dx}_{dy}", 5*k, 5*k, 65*k, loc=(ox+dx, -40*k+dy, 33*k))
            parts.append(post)
        # pillow at head
        pillow = box(f"Pillow{i}", 30*k, 16*k, 6*k, loc=(ox, -12*k, 20*k)); bevel(pillow, 2, 1)
        # side ladder rungs
        for rr in range(4):
            rung = box(f"Rung{i}_{rr}", 4*k, 18*k, 4*k, loc=(ox-24*k, -20*k, 12*k + rr*14*k))
            parts.append(rung)
        parts += [b_low_base, b_low_matt, b_hi_base, b_hi_matt, pillow]

    # desk consoles with raised screens + side keyboard
    for i in range(max(1, room_count)):
        ox = -L/2 + 60*k + i*120*k
        desk = box(f"Desk{i}", 50*k, 30*k, 34*k, loc=(ox, 30*k, 17*k)); bevel(desk, 2, 1)
        screen = box(f"DeskScreen{i}", 30*k, 4*k, 18*k, loc=(ox, 26*k, 46*k))   # emissive
        kbd = box(f"DeskKbd{i}", 30*k, 10*k, 4*k, loc=(ox, 36*k, 34*k))
        chair_base = box(f"ChairBase{i}", 16*k, 16*k, 10*k, loc=(ox, 46*k, 5*k))
        chair_back = box(f"ChairBack{i}", 16*k, 6*k, 26*k, loc=(ox, 52*k, 24*k)); bevel(chair_back, 2, 1)
        parts += [desk, screen, kbd, chair_base, chair_back]

    # storage lockers (tall, right wall) + a central mess table
    for i in range(4):
        lk = box(f"Locker{i}", 24*k, 6*k, 60*k, loc=(-L/2+30*k+i*70*k, 62*k, 40*k)); bevel(lk, 1, 1)
        parts.append(lk)
    table = box("MessTable", 70*k, 40*k, 8*k, loc=(0, -6*k, 8*k)); bevel(table, 2, 1)
    table_legs = [box(f"TLeg{i}", 5*k, 5*k, 8*k, loc=(x, y, 2*k))
                  for i, (x, y) in enumerate([(-32*k, -16*k), (32*k, -16*k), (-32*k, 16*k), (32*k, 16*k)])]
    parts += [table] + table_legs

    jo, out = finalize_part(parts, outname, "M_Interior_Hab")
    return [(jo, out)]


def build_corridor(prefix):
    """Standard spine corridor + hatches (joinable between modules)."""
    k = 1.0  # corridors share a standard scale across classes
    parts = []
    L = 260
    W = 70
    H = 80
    floor = box("CorrFloor", L, W, 6*k, loc=(0,0,0)); bevel(floor,1,1); parts.append(floor)
    for side in (-1,1):
        w = box(f"CorrWall{side}", W, 6*k, H, loc=(side*L/2,0,H/2)); bevel(w,1,1); parts.append(w)
    ceil = box("CorrCeil", L, W, 5*k, loc=(0,0,H)); bevel(ceil,1,1); parts.append(ceil)
    # light strips
    for x in (-80, 0, 80):
        parts.append(box(f"CorrLight{x}", 30*k, 5*k, 3*k, loc=(x,0,H-5*k)))
    # bulkhead rings along the corridor
    for x in range(-100, 101, 50):
        ring = box(f"Bulkhead{x}", 70*k, 5*k, 70*k, loc=(x, 0, H/2))
        parts.append(ring)
    jo, out = finalize_part(parts, prefix + "_Corridor", "M_Interior_Hab")
    return [(jo, out)]


def build_interior_set():
    """Build all interior instances, one per ship class + standard corridor."""
    results = []
    results += build_cockpit_interior('small', "SM_Int_Fighter_Cockpit")
    results += build_hab_interior('medium', "SM_Int_Freighter_CrewQuarters")
    results += build_hab_interior('large', "SM_Int_Generationship_Hab", room_count=2)
    results += build_corridor("SM_Int_Standard")
    return results


# ----------------------------------------------------------------------------
def main():
    setup_scene()
    print("Generating textures...")
    # existing ship/prop textures (kept for compatibility)
    gen_texture_set("Ship_Hull", {'base':[0.55,0.58,0.6], 'accent':[0.12,0.16,0.22], 'emissive':[0.2,0.55,1.0],
                                  'cable':{'runs':4}}, 2048, seed=11)
    gen_texture_set("Freighter", {'base':[0.6,0.55,0.5], 'accent':[0.2,0.25,0.2], 'emissive':[0.6,0.3,0.05],
                                  'grime':True, 'hazard':{'bands':3}}, 2048, seed=22)
    gen_texture_set("Prop_Crate", {'base':[0.45,0.5,0.55], 'accent':[0.3,0.3,0.0], 'emissive':[0.2,1.0,0.2],
                                   'hazard':{'bands':3}}, 1024, seed=33)
    gen_texture_set("Prop_Tank", {'base':[0.7,0.32,0.18], 'accent':[0.1,0.1,0.1], 'emissive':[1.0,0.2,0.1],
                                  'hazard':{'bands':4}}, 1024, seed=44)
    # cyberpunk additions
    gen_texture_set("Gunship", {'base':[0.22,0.23,0.26], 'accent':[0.4,0.05,0.05],
                                'emissive':[1.0,0.15,0.05], 'neon':[1.0,0.2,0.1], 'neon_thick':3,
                                'grime':True, 'hazard':{'bands':3}}, 2048, seed=55)
    gen_texture_set("Station_Hab", {'base':[0.3,0.32,0.38], 'accent':[0.15,0.18,0.24],
                                    'emissive':[0.2,0.55,1.0],
                                    'windows':{'cols':14,'frac':0.65,'cool':[0.3,0.65,1.0],'warm':[1.0,0.6,0.25]},
                                    'neon':[0.3,1.0,1.0], 'neon_thick':2, 'grime':True,
                                    'cable':{'runs':6}, 'hazard':{'bands':3}}, 2048, seed=66)
    gen_texture_set("Derelict", {'base':[0.4,0.37,0.33], 'accent':[0.25,0.22,0.18],
                                 'emissive':[0.9,0.5,0.1], 'grime':True, 'hazard':{'bands':3}}, 2048, seed=77)
    gen_texture_set("Satellite", {'base':[0.75,0.76,0.78], 'accent':[0.35,0.4,0.5],
                                  'emissive':[0.2,0.4,1.0], 'windows':{'cols':12,'frac':0.25},
                                  'cable':{'runs':3}}, 1024, seed=88)
    gen_texture_set("SolarPanel", {'base':[0.15,0.2,0.45], 'accent':[0.1,0.15,0.3],
                                   'emissive':[0.3,0.6,1.0], 'windows':{'cols':24,'frac':0.1}}, 1024, seed=99)
    gen_texture_set("FuelCell", {'base':[0.5,0.52,0.48], 'accent':[0.2,0.3,0.2], 'emissive':[0.6,0.7,0.2],
                                 'hazard':{'bands':5}, 'cable':{'runs':5}, 'grime':True}, 1024, seed=120)
    # Project-Hyperion-inspired generation-ship parts
    gen_texture_set("HabRing", {'base':[0.55,0.6,0.62], 'accent':[0.18,0.3,0.35], 'emissive':[0.3,1.0,1.0],
                                'windows':{'cols':18,'frac':0.8,'cool':[0.3,0.8,1.0],'warm':[1.0,0.7,0.3]},
                                'neon':[0.3,1.0,1.0], 'neon_thick':2, 'cable':{'runs':5}}, 2048, seed=130)
    gen_texture_set("AsteroidShell", {'base':[0.42,0.38,0.34], 'accent':[0.3,0.26,0.22], 'emissive':[0.9,0.5,0.1],
                                      'grime':True, 'hazard':{'bands':3}}, 2048, seed=140)
    # Interior set textures
    gen_texture_set("Int_Deck", {'base':[0.22,0.24,0.26], 'accent':[0.35,0.38,0.42], 'emissive':[0.15,0.2,0.3],
                                 'cells':10, 'grime':True, 'cable':{'runs':5}}, 1024, seed=150)
    gen_texture_set("Int_Wall", {'base':[0.3,0.32,0.36], 'accent':[0.4,0.42,0.46], 'emissive':[0.1,0.12,0.16],
                                 'cells':8, 'grime':True, 'cable':{'runs':6}}, 1024, seed=151)
    gen_texture_set("Int_Glow", {'base':[0.5,0.53,0.55], 'accent':[0.6,0.63,0.65], 'emissive':[0.2,0.9,1.0],
                                 'cells':6, 'neon':[0.2,0.9,1.0], 'neon_thick':2}, 512, seed=152)
    gen_texture_set("Int_Cockpit", {'base':[0.25,0.27,0.3], 'accent':[0.18,0.2,0.24], 'emissive':[0.1,0.6,0.9],
                                    'cells':8, 'windows':{'cols':16,'frac':0.5,'cool':[0.2,0.7,1.0]},
                                    'neon':[0.15,0.7,1.0], 'neon_thick':2}, 1024, seed=153)

    print("Building ships (modular carcass + add-ons)...")
    # Each ship = carcass + scaled parts, each exported as its own FBX.
    ship_parts = {
        "SM_Ship_Fighter_01": assemble_ship('small',
            'SM_Ship_Fighter_01',
            {'engine': True, 'cargo': True, 'weapon': True, 'weapon_twin': True,
             'sensor': True}),
        "SM_Ship_Freighter_01": assemble_ship('medium',
            'SM_Ship_Freighter_01',
            {'engine': True, 'cargo': True, 'weapon': False, 'sensor': True,
             'reactor': True}),
        "SM_Ship_Gunship_02": assemble_ship('small',
            'SM_Ship_Gunship_02',
            {'engine': True, 'cargo': False, 'weapon': True, 'weapon_twin': True,
             'sensor': True, 'sensor_asym': True}),
    }
    for ship_name, parts in ship_parts.items():
        for obj, out in parts:
            print(f"  {ship_name}: {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

    # Project-Hyperion-inspired generation ships (ring habitat + asteroid shell)
    print("Building Project-Hyperion generation ships...")
    gen_parts = {
        "SM_Ship_Generationship_01": assemble_ship('medium',
            'SM_Ship_Generationship_01',
            {'engine': True, 'cargo': True, 'sensor': True, 'reactor': True,
             'habitat_ring': True, 'ring_dual': True}),
        "SM_Station_AsteroidHab_01": assemble_ship('large',
            'SM_Station_AsteroidHab_01',
            {'habitat_ring': True, 'ring_dual': False,
             'asteroid_shell': True, 'sensor': True}),
    }
    for name, parts in gen_parts.items():
        for obj, out in parts:
            print(f"  {name}: {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

    # New ship/station kit additions
    print("Building corvette + mining additions...")
    new_parts = {
        # Corvette — frigate profile, heavier armament
        "SM_Ship_Corvette_01": assemble_ship('corvette', 'SM_Ship_Corvette_01',
            {'engine': True, 'cargo': True, 'weapon': True, 'weapon_twin': True,
             'sensor': True, 'reactor': True},
            carcass_builder=build_corvette_carcass),
        # Mining corvette — drill + mining laser, no heavy weapon
        "SM_Ship_Miner_01": assemble_ship('corvette', 'SM_Ship_Miner_01',
            {'engine': True, 'cargo': True, 'drill': True, 'mining_laser': True,
             'sensor': True},
            carcass_builder=build_corvette_carcass),
        # Mining station — drill rig + reactor + cargo + mining laser
        "SM_Station_Mining_01": assemble_ship('large', 'SM_Station_Mining_01',
            {'drill': True, 'mining_laser': True, 'sensor': True, 'reactor': True,
             'cargo': True},
            carcass_builder=build_mining_station_carcass),
    }
    for name, parts in new_parts.items():
        for obj, out in parts:
            print(f"  {name}: {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

    print("Building station...")
    st = build_station(); out4 = export_fbx(st, "SM_Station_Habitation_01")

    print("Building props...")
    props = build_props()
    for ob, out in props:
        print("  exported:", os.path.basename(out), os.path.getsize(out) if os.path.exists(out) else 0)

    print("Building extra space objects...")
    objs = build_extra_objects()
    for ob, out in objs:
        print("  exported:", os.path.basename(out), os.path.getsize(out) if os.path.exists(out) else 0)

    # Interior instances (separate assets — entered when leaving the cockpit)
    print("Building interior instances...")
    inter = build_interior_set()
    for ob, out in inter:
        print("  exported:", os.path.basename(out), os.path.getsize(out) if os.path.exists(out) else 0)

    print("DONE")
    print("Ship parts:", sum(len(v) for v in ship_parts.values()))
    print("Station:", os.path.basename(out4), os.path.getsize(out4) if os.path.exists(out4) else 0)
    print("Interiors:", len(inter))
    print("Textures:", len(os.listdir(TEXDIR)))


if __name__ == "__main__":
    main()