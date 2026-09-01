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
ASSETS_OUT = BASE
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
    # recalc normals outward so the exported FBX faces render correctly in UE
    # and aren't backface-culled in EEVEE (join of primitives can leave
    # inward-flipped faces)
    try:
        bpy.ops.mesh.normals_make_consistent(inside=False)
    except Exception:
        pass
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


def union_into(objs, target_name, bevel_amt=4, bevel_segs=2, keep_named=None):
    """Boolean-UNION a set of overlapping primitives into ONE continuous hull, then
    bevel the transition seams so it reads as a single integrated vessel silhouette
    instead of stacked boxes. Needed because plain joint() overlaps volumes."""
    if len(objs) < 2:
        return join(objs, target_name, keep_named)
    # start from first object, union the rest via a single boolean chain then apply
    base = objs[0]
    sel_activate(base)
    for i, ob in enumerate(objs[1:]):
        m = base.modifiers.new(f"UnionB{i}", 'BOOLEAN')
        m.operation = 'UNION'
        m.object = ob
    bpy.context.view_layer.objects.active = base
    # apply boolean modifiers one at a time (apply-as-DATA renames the mesh)
    for i in range(len(objs) - 1):
        bpy.ops.object.modifier_apply(modifier=f"UnionB{i}")
    joined = base
    joined.name = target_name
    # bevel the union seams to fillet transitions
    try:
        sel_activate(joined)
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.bevel(offset=bevel_amt, offset_type='OFFSET',
                           segments=bevel_segs, affect='EDGES')
        bpy.ops.object.mode_set(mode='OBJECT')
    except Exception:
        pass
    keep = (keep_named or set()) | {target_name}
    for ob in list(bpy.data.objects):
        if ob.name not in keep and ob.type == 'MESH' and ob is not joined:
            bpy.data.objects.remove(ob, do_unlink=True)
    return joined


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
    # 'fixed' marks regions that must NOT be re-skinned (stripes/neon/windows/
    # hazard) — the skin mask is the inverse, so a runtime skin only recolors the
    # plain hull panels and leaves accents/lights intact (mirrors X4's paintmodmask).
    stripe = np.zeros((H, W), dtype=bool)
    fixed = np.zeros((H, W), dtype=bool)
    for i in range(cell, W, cell):
        stripe[max(0,i-3):i+3, :] = True
    fixed |= stripe
    D[stripe] = [accent[0], accent[1], accent[2], 1.0]

    # emissive mask: glow seams + hazard-light dots
    E = np.zeros((H, W, 4), dtype=np.float32)
    E[..., 3] = 1.0
    em = np.array(variant.get('emissive', [0.2, 0.55, 1.0]))
    E[stripe] = [em[0], em[1], em[2], 1.0]

    # ---- Phase 4: material quality (shader knobs) ----
    # roughness: smooth metal with anisotropic grain + rough grooves. A directional
    # low-frequency grain is added so reflections elongate along panel direction
    # (specular breaks realistically instead of a uniform plastic smoothness),
    # and long panels get a subtly varied finish.
    R = np.ones((H, W), dtype=np.float32) * variant.get('rough', 0.34)
    # anisotropic grain: smooth, banded low-freq striations (brushed-metal feel)
    rg_y, rg_x = np.mgrid[0:H, 0:W].astype(np.float32)
    aniso = 0.14 * np.sin(rg_x * 0.06 + 11.7) * np.sin(rg_y * 0.011 + 3.3)
    aniso += 0.08 * np.sin(rg_x * 0.013 + rg_y * 0.02)
    R = np.clip(R + aniso, 0.05, 1.0)
    # accent/edge regions read as machined (slightly smoother, metal-bare)
    R[stripe] = np.maximum(0.05, R[stripe] - 0.12)
    # deep grooves stay rough
    R[h < 0.28] = 0.82
    R[h < 0.16] = 0.95

    # metallic: mostly metal with painted-vs-bare variation. Painted panels are a
    # bit less metallic; worn/raised edges and grooves read as bare metal (higher).
    Mbase = variant.get('metal', 0.96)
    M = np.ones((H, W), dtype=np.float32) * Mbase
    # subtle painted-plating variation across the hull
    paint = 0.10 * np.sin(rg_x * 0.02 + rg_y * 0.03) + 0.06 * np.sin(rg_y * 0.05)
    M = np.clip(M - paint, 0.3, 1.0)
    # accent seams / grooves: bare metal edge highlight (higher metallic)
    M[stripe] = np.minimum(1.0, M[stripe] + 0.15)
    M[h < 0.2] = 0.55  # recessed, slightly non-metal (primer/grime)

    # AO: multi-scale, darker in deep grooves & pits
    AO = (h - 0.5) * 2.0 + 0.62
    AO = np.clip(AO, 0.05, 1.0)

    # ---- optional cyberpunk / industrial overlays ----
    # STARSHIP strip viewports — NOT house windows. These are long, narrow
    # horizontal band windows set into the hull like armored pressure-rated
    # viewports: a slim raised frame (thick top+bottom rail), glass that's either
    # lit (cool/warm interior light) or dark (unlit/reflective), and only thin
    # vertical stiffener mullions at intervals (NOT a centered crossbar). They run
    # along the hull as bands, recessed slightly, rather than being square panes.
    if variant.get('windows'):
        wc = variant['windows']
        cols_n = wc.get('cols', 12)
        wcell = W // cols_n
        cool = np.array(wc.get('cool', [0.3, 0.65, 1.0]))
        warm = np.array(wc.get('warm', [1.0, 0.6, 0.25]))
        litfrac = wc.get('frac', 0.5)
        density = wc.get('density', 0.9)
        rail = max(3, int(wcell*0.06))          # armored top/bottom rail thickness
        band_h = max(4, int(wcell*0.24))        # glass band height (narrow!)
        band_w = int(wcell*0.85)                # glass band width (long)
        mull = max(1, int(wcell*0.025))         # thin vertical stiffener
        row_step = max(int(wcell*1.6), 2)
        for ri, j0 in enumerate(range(wcell//2 + wcell, H - wcell*2, row_step)):
            stagger = int(wcell//2) if ri % 2 else int(rng.integers(0, wcell//3))
            for i in range(wcell//2 + stagger + wcell, W - wcell, wcell):
                if rng.random() > density:
                    continue
                c0 = min(i, W - band_w - 1); c1 = min(c0 + band_w, W)
                r0 = min(j0, H - band_h - 1); r1 = min(r0 + band_h, H)
                if r1 - r0 < 3 or c1 - c0 < 3:
                    continue
                # armored frame: raised rail top+bottom, darker rim sides
                D[r0, c0:c1] = [accent[0]*1.3, accent[1]*1.3, accent[2]*1.3, 1.0]; h[r0, c0:c1] = 0.34
                D[r1-1, c0:c1] = [accent[0]*1.3, accent[1]*1.3, accent[2]*1.3, 1.0]; h[r1-1, c0:c1] = 0.34
                AO[slice(r0,r1), slice(c0,c1)] = 0.4
                R[slice(r0,r1), slice(c0,c1)] = 0.55
                fixed[slice(r0,r1), slice(c0,c1)] = True
                # glass inset (below the rail, above the rail) = the band strip
                gr_, gb_ = r0+rail, r1-rail
                gsr = slice(gr_, max(gr_+1, gb_))
                gsc = slice(c0, c1)
                lit = rng.random() < litfrac
                if lit:
                    col = cool if rng.random() < 0.7 else warm
                    D[gsr, gsc] = [col[0]*0.6, col[1]*0.72, col[2]*0.95, 1.0]
                    E[gsr, gsc] = [col[0], col[1], col[2], 1.0]
                    AO[gsr, gsc] = 0.12; R[gsr, gsc] = 0.12
                else:
                    D[gsr, gsc] = [0.06, 0.08, 0.10, 1.0]
                    AO[gsr, gsc] = 0.55; R[gsr, gsc] = 0.15
                h[gsr, gsc] = 0.12
                # thin vertical stiffener mullions at intervals across the band
                if lit:
                    mid = (gr_ + gb_) // 2
                    for mx in range(c0 + band_w//4, c1, band_w//4):
                        D[mid-1:mid+2, max(c0,mx)-1:max(c0,mx)+2] = [0.05,0.05,0.06,1.0]
                        E[mid-1:mid+2, max(c0,mx)-1:max(c0,mx)+2] = 0.0
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
        fixed |= neon
    # ---- Phase 3: AO-driven weathering overlay (dirt + streaks) ----
    # The single biggest "worn/real" win (X4 trick). Dirt accumulates where AO is
    # low (recessed grooves/crevices), plus directional streak/scorch marks running
    # along panel seams, and faint micro-scratches. All read ABOVE the base albedo.
    if variant.get('grime') or variant.get('weather'):
        # 1) AO-driven dirt: darken D in low-AO / deep-groove regions and roughen them
        #    (dirt pools, doesn't sit on exposed raised panels).
        dirt_mask = (h < 0.42)  # deep grooves/crevices already carry low h from panels/cables
        # soften: only pool where there's actual recessed structure
        Dd = np.empty_like(D[:, :, :3])
        for ch in range(3):
            Dd[..., ch] = np.clip(D[:, :, ch] * (0.82 - 0.06*ch), 0, 1)
        D[:, :, :3][dirt_mask] = Dd[dirt_mask]
        R[dirt_mask] = np.maximum(R[dirt_mask], 0.85)
        AO[dirt_mask] = np.minimum(AO[dirt_mask], 0.45)
        # 2) directional streaks/scorch running along panel seams (vertical + horizontal)
        for _ in range(rng.integers(8, 14)):
            hz = rng.random() < 0.5
            sx = int(rng.integers(0, W)); sy = int(rng.integers(0, H))
            length = int(rng.integers(W//6, W//3)) if hz else int(rng.integers(H//8, H//3))
            w = int(rng.integers(1, 3))
            fade = np.clip(np.linspace(1.0, 0.3, length), 0, 1)
            for k in range(length):
                if hz:
                    x = sx + k; y = sy
                    if 0 <= x < W and 0 <= y < H:
                        D[y, x] *= 0.80 - 0.10*w + (fade[k]*0.1)
                        R[y, x] = 0.9
                else:
                    y = sy + k; x = sx
                    if 0 <= y < H and 0 <= x < W:
                        D[y, x] *= (0.80 - 0.10*w) + (fade[k]*0.1)
                        R[y, x] = 0.9
        # 3) faint micro-scratch noise (short thin lighter marks)
        for _ in range(rng.integers(25, 45)):
            sx = int(rng.integers(0, W)); sy = int(rng.integers(0, H))
            ang = rng.random()*3.14
            L = int(rng.integers(4, 14))
            for k in range(L):
                x = int(sx + k*math.cos(ang)); y = int(sy + k*math.sin(ang))
                if 0 <= x < W and 0 <= y < H:
                    D[y, x] = np.clip(D[y, x] - [0.12, 0.12, 0.12, 0.0], 0, 1)
                    AO[y, x] = min(AO[y, x], 0.5)
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
            fixed[sy:sy+sw, sx:sx+sw] = True
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
    # ---- authored surface detail: access panels, rivets, vents, id marks ----
    # Adds believable "kit" to the hull beyond the tiled base — framed access
    # hatches, rivet/screw rows, vent slats, and small registration markings.
    acc = np.array(accent); bas = np.array(base)
    if variant.get('detail', True):
        # 3-5 debossed access panels with a recessed lip + 4 corner bolts
        for _ in range(rng.integers(3, 6)):
            pw = int(rng.integers(W//16, W//6)); ph = int(pw*rng.uniform(0.6,0.9))
            px0 = int(rng.integers(0, W-pw)); py0 = int(rng.integers(0, H-ph))
            psr = slice(py0, py0+ph); psc = slice(px0, px0+pw)
            h[psr, psc] = 0.20; AO[psr, psc] = 0.45
            D[psr, psc] = [bas[0]*0.88, bas[1]*0.88, bas[2]*0.88, 1.0]
            # recessed border groove
            D[py0, px0:px0+pw] = [acc[0]*1.35, acc[1]*1.35, acc[2]*1.35, 1.0]; h[py0, px0:px0+pw] = 0.32
            D[py0+ph-1, px0:px0+pw] = [acc[0]*1.35, acc[1]*1.35, acc[2]*1.35, 1.0]; h[py0+ph-1, px0:px0+pw] = 0.32
            D[py0:py0+ph, px0] = [acc[0]*1.35, acc[1]*1.35, acc[2]*1.35, 1.0]; h[py0:py0+ph, px0] = 0.32
            D[py0:py0+ph, px0+pw-1] = [acc[0]*1.35, acc[1]*1.35, acc[2]*1.35, 1.0]; h[py0:py0+ph, px0+pw-1] = 0.32
            for (bx,by) in [(px0+3,py0+3),(px0+pw-5,py0+3),(px0+3,py0+ph-5),(px0+pw-5,py0+ph-5)]:
                for dy in range(-3,4):
                    for dx in range(-3,4):
                        if dx*dx+dy*dy<9:
                            yy,xx=by+dy,bx+dx
                            if 0<=yy<H and 0<=xx<W:
                                h[yy,xx]=0.12; AO[yy,xx]=0.3; D[yy,xx]=[0.6,0.62,0.65,1.0]
        # rivet/screw rows along a horizontal seam
        for _ in range(2):
            ry0 = int(rng.integers(H*0.15, H*0.85)); rx0 = int(rng.integers(0, W//4))
            step = max(int(rng.integers(W//28, W//22)), 4)
            for xx0 in range(rx0, W - rx0, step):
                for dy in range(-2,3):
                    for dx in range(-2,3):
                        if dx*dx+dy*dy<4:
                            yy,xx=ry0+dy,xx0+dx
                            if 0<=yy<H and 0<=xx<W:
                                h[yy,xx]=0.10; AO[yy,xx]=0.28; D[yy,xx]=[0.5,0.52,0.55,1.0]
        # vent slat grill (2-3 short slat stacks)
        for _ in range(rng.integers(2,4)):
            gw=int(rng.integers(W//30, W//18)); gx0=int(rng.integers(0,W-gw))
            gy0=int(rng.integers(0,H//2)); gslats=int(rng.integers(6,11)); sh=max(int(gw*0.35),2)
            for k in range(gslats):
                yy=gy0+k*sh*2
                if yy+sh<H:
                    D[yy:yy+sh, gx0:gx0+gw]=[acc[0]*1.25, acc[1]*1.25, acc[2]*1.25, 1.0]
                    h[yy:yy+sh, gx0:gx0+gw]=0.28; AO[yy:yy+sh, gx0:gx0+gw]=0.4
        # small registration / id number dashes (non-emissive dark markings)
        if variant.get('id_marks', True):
            for _ in range(rng.integers(2,4)):
                sx0=int(rng.integers(0,W-W//8)); sy0=int(rng.integers(0,H-20))
                for k in range(9):
                    seg = int(k*7); ch_=1 if k%2==0 else 0
                    D[sy0+4*ch_:sy0+4*ch_+3, sx0+seg:sx0+seg+4]=[0.05,0.05,0.06,1.0]
                    AO[sy0+4*ch_:sy0+4*ch_+3, sx0+seg:sx0+seg+4]=0.5

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

    # skin mask: 1.0 = skinnable hull panel, 0.0 = fixed region (accent/neon/
    # windows/hazard) that a runtime skin must NOT recolor (X4 paintmodmask analog)
    SKIN = np.where(fixed, 0.0, 1.0).astype(np.float32)

    # ---- ship symmetry (NOT destructive) ----
    # Ship L-R symmetry is enforced by smart_uv's U -> 1-U flip on the -X half:
    # the -X faces sample the mirror of their +X twins, so the ship reads symmetric
    # EVEN IF the texture is asymmetric. We deliberately do NOT average the texture
    # with its mirror here (an earlier version did and washed out every random
    # detail: rivets, vents, panels, id marks). Keep detail full-strength; the
    # UV flip guarantees the rendered ship is symmetric.

    maps = {'_D': D, '_N': N, '_R': _to4(R), '_M': _to4(M), '_AO': _to4(AO), '_E': E, '_SKIN': _to4(SKIN)}
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
    """Normalized, consistent UV via world-aligned triplanar projection."""
    import bmesh as bm3
    sel_activate(ob)
    me = ob.data
    bm = bm3.new()
    bm.from_mesh(me)
    bm.faces.ensure_lookup_table()
    tile_cm = 200.0
    uv_layer = bm.loops.layers.uv.get("UVMap") or bm.loops.layers.uv.new("UVMap")
    import mathutils
    axes = [mathutils.Vector((1,0,0)), mathutils.Vector((0,1,0)), mathutils.Vector((0,0,1))]
    for f in bm.faces:
        n = f.normal.copy()
        ax = max(range(3), key=lambda i: abs(n[i]))
        if ax == 0:
            u_ax, v_ax = axes[1], axes[2]
        elif ax == 1:
            u_ax, v_ax = axes[2], axes[0]
        else:
            u_ax, v_ax = axes[0], axes[1]
        for loop in f.loops:
            v = ob.matrix_world @ loop.vert.co
            u = (v @ u_ax) / tile_cm
            ww = (v @ v_ax) / tile_cm
            loop[uv_layer].uv = (u, ww)
        # STRICT L-R mirror: flip U on the -X half so every -X face samples the
        # horizontal mirror of its +X counterpart. Combined with the (now
        # U=0.5-symmetric) texture, paired faces read as exact mirrors.
        fcx = sum((ob.matrix_world @ lc.vert.co)[0] for lc in f.loops) / len(f.loops)
        if fcx < -0.01:
            for loop in f.loops:
                uu2, vv2 = loop[uv_layer].uv
                loop[uv_layer].uv = (1.0 - uu2, vv2)
    bm.to_mesh(me)
    bm.free()
    me.update()

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


def build_cargo_carcass(sz, outname):
    """Cargo/trading-specialist carcass — a freight-hauler that reads as ONE
    integrated vessel, not stacked boxes. A single tapered, broad fuselage forms
    the silhouette (narrowing fore & aft so it reads as a ship hull), with the
    dorsal bridge and flank pods bedded FLUSH into it. A strong fillet bevel rounds
    every transition so nothing looks like a box glued on top of another."""
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']
    locz = s['z']
    k = sc(sz)
    # --- dominant tapered fuselage ---
    hull = box(f"Carg_{sz}_Hull", lx*1.05, ly, lz*0.75, loc=(0, 0, locz))
    # taper: fore + aft wedges pulled inward so the silhouette narrows like a hull
    fore = box("Carg_Fore", lx*0.42, ly*0.30, lz*0.7, loc=(0, ly*0.66, locz)); bevel(fore, 6, 2)
    aft = box("Carg_Aft", lx*0.62, ly*0.22, lz*0.6, loc=(0, -ly*0.66, locz)); bevel(aft, 6, 2)
    # --- shallow dorsal bridge, bedded flush into the hull top ---
    br = box("Carg_Bridge", lx*0.40, ly*0.26, lz*0.16, loc=(0, ly*0.12, locz + lz*0.46))
    bevel(br, 10, 4)
    # --- flank pods: shallow shoulders that only slightly break the hull width ---
    objs = [hull, fore, aft, br]
    for side in (-1, 1):
        pod = box(f"Carg_Pod{side}", lx*0.30, ly*0.30, lz*0.22,
                  loc=(side*lx*0.30, ly*0.02, locz + lz*0.02)); bevel(pod, 8, 3)
        objs.append(pod)
    # slim sensor mast (a slender mast reads as a fixture, not a stacked box)
    mast = cyl("Carg_Mast", lz*0.08, lz*0.5, loc=(0, ly*0.38, locz + lz*0.55), verts=14); bevel(mast, 2, 1)
    objs.append(mast)
    # merge overlapping volumes into one silhouette, then a STRONG fillet bevel
    integrated = union_into(objs, f"{outname}_HullInt", bevel_amt=max(20, int(lz*0.18)),
                            bevel_segs=3)
    return finalize_part([integrated], f"{outname}_Carcass", "M_Hull")


def build_warship_carcass(sz, outname):
    """Warship/combat-specialist carcass — an arrowhead wedge fuselage with a
    dorsal command ridge, twin flank gun-stub outriggers, and a heavy nose canoe —
    reads as a dedicated warship (aggressive, zero cargo volume), distinct from the
    cargo-hauler and the small fighter. Returns the finalized carcass."""
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']
    locz = s['z']
    k = sc(sz)
    # long arrowhead nose wedge (aggressive)
    hull = box(f"War_{sz}_Hull", lx*0.9, ly, lz*0.7, loc=(0, 0, locz)); bevel(hull, 9, 3)
    nose = cone("War_Nose", lx*0.34, ly*0.26, loc=(0, ly*0.66, locz),
                rot=(math.radians(90), 0, 0), verts=14); bevel(nose, 3, 2)
    # dorsal command ridge / spine
    ridge = box("War_Ridge", lx*0.30, ly*0.5, lz*0.42, loc=(0, -ly*0.1, locz + lz*0.5)); bevel(ridge, 6, 3)
    # twin flank gun-stub outriggers (dedicated hardpoint pylons)
    objs = [hull, nose, ridge]
    for side in (-1, 1):
        pylon = box(f"War_Pylon{side}", lx*0.14, ly*0.5, lz*0.3,
                    loc=(side*lx*0.46, -ly*0.1, locz + lz*0.05)); bevel(pylon, 4, 2)
        objs.append(pylon)
    # slim aft fin
    fin = box("War_Fin", lx*0.06, ly*0.18, lz*0.8, loc=(0, -ly*0.52, locz + lz*0.4)); bevel(fin, 3, 2)
    objs.append(fin)
    # Join (plain merge) rather than boolean-union — boolean of overlapping
    # primitives produced degenerate faces + exploded tri counts on the larger
    # classes. Warships read as built-up/armored by nature, so overlapping
    # plates read fine; skip the risky boolean here.
    joined = join(objs, f"{outname}_CarcassGeo")
    return finalize_part([joined], f"{outname}_Carcass", "M_Hull")


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


def build_engine_part(sz, outname, variant='standard'):
    """Engine block add-on, scaled to carcass size class; twin nacelles + bells.
    variant: 'standard' (twin boxes+bells), 'ion' (long pylons), 'compact'."""
    k = sc(sz)
    if variant == 'ion':
        # long ion-drive pylons on both flanks
        blk = box("Ion_Blk", 120*k, 180*k, 80*k, loc=(0, 0, 0)); bevel(blk, 5, 2)
        parts = [blk]
        for side in (-1, 1):
            pylon = cyl(f"Ion_Pylon{side}", 22*k, 260*k, loc=(side*120*k, 5*k, 10*k),
                        rot=(0, 0, math.radians(9*side)), verts=12); bevel(pylon, 2, 1)
            glow = torus(f"Ion_Glow{side}", 22*k, 5*k, loc=(side*120*k, -120*k, 8*k),
                         maj=14, minr=5)
            parts += [pylon, glow]
        return finalize_part(parts, outname, "M_Engine_Ion", origin='ORIGIN_CENTER_OF_VOLUME')
    if variant == 'compact':
        # single squat engine pod
        pod = box("Compact_Pod", 150*k, 160*k, 80*k, loc=(0, 0, 0)); bevel(pod, 6, 2)
        noz = cone("Compact_Noz", 34*k, 60*k, loc=(0, -130*k, 0),
                   rot=(math.radians(90), 0, 0), verts=16)
        return finalize_part([pod, noz], outname, "M_Engine_Compact",
                             origin='ORIGIN_CENTER_OF_VOLUME')
    # standard: twin nacelles + bells
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


def build_cargo_part(sz, outname, variant='containers'):
    """Cargo bay add-on. variant: 'containers' (box + cells), 'bulk_tank'
    (cylindrical hold), 'flat_rack' (open frame)."""
    k = sc(sz)
    if variant == 'bulk_tank':
        tank = cyl("Bulk_Tank", 70*k, 240*k, loc=(0, 0, 0), verts=18); bevel(tank, 3, 1)
        cap1 = sphere("Bulk_Cap1", 70*k, loc=(0, 0, 120*k), verts=12); cap1.scale = (1, 1, 0.3)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
        cap2 = sphere("Bulk_Cap2", 70*k, loc=(0, 0, -120*k), verts=12); cap2.scale = (1, 1, 0.3)
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
        ring1 = torus("Bulk_Ring1", 70*k, 5*k, loc=(0, 0, 0), maj=24, minr=6)
        return finalize_part([tank, cap1, cap2, ring1], outname, "M_Cargo_Tank", origin='ORIGIN_CENTER_OF_VOLUME')
    if variant == 'flat_rack':
        base = box("Rack_Base", 190*k, 240*k, 20*k, loc=(0, 0, 0)); bevel(base, 3, 1)
        # open container frame struts
        parts = [base]
        for side in (-1, 1):
            rail = box(f"Rack_Rail{side}", 12*k, 220*k, 14*k, loc=(side*85*k, 0, 30*k)); bevel(rail, 1, 1)
            parts.append(rail)
        for dy in (-80*k, 80*k):
            beam = box(f"Rack_Beam{dy}", 180*k, 12*k, 14*k, loc=(0, dy, 30*k)); bevel(beam, 1, 1)
            parts.append(beam)
        return finalize_part(parts, outname, "M_Cargo_Rack", origin='ORIGIN_CENTER_OF_VOLUME')
    # containers (default)
    base = box("Cargo_Base", 200*k, 260*k, 90*k, loc=(0, 0, 0))
    bevel(base, 4, 2)
    parts = [base]
    for i, dx in enumerate(range(-60, 61, 60)):
        c = greeble(f"Cargo_Cell{i}", loc=(dx*k, -40*k, 30*k), sx=30*k, sy=70*k, sz=30*k)
        parts.append(c)
    return finalize_part(parts, outname, "M_Cargo_Hold", origin='ORIGIN_CENTER_OF_VOLUME')


def build_weapon_part(sz, outname, twin=True, variant='cannon'):
    """Weapon/cannon add-on (nose mount). 
    variant: 'cannon' (twin/single gun pods), 'tri_laser' (3 barrels), 'missile' (pods)."""
    k = sc(sz)
    if variant == 'tri_laser':
        parts = []
        # 3 laser barrels fanning out
        for side in (-1, 0, 1):
            bx = cyl(f"Tri_Barrel{side}", 4*k, 70*k, loc=(side*18*k, 60*k, side*4*k),
                     rot=(math.radians(90), 0, math.radians(side*10)), verts=10)
            parts.append(bx)
        base = box("Tri_Base", 70*k, 30*k, 24*k, loc=(0, -10*k, 0)); bevel(base, 3, 1)
        return finalize_part([base] + parts, outname, "M_Weapon_TriLaser", origin='ORIGIN_CENTER_OF_VOLUME')
    if variant == 'missile':
        parts = []
        for side in (-1, 1):
            pod = box(f"Missile_Pod{side}", 22*k, 80*k, 22*k, loc=(side*40*k, 30*k, 0)); bevel(pod, 2, 1)
            # 4 missile tubes per pod
            for t in range(4):
                tube = cyl(f"Missile_Tube{side}_{t}", 4*k, 22*k, loc=(side*40*k, 75*k, (-12+t*8)*k),
                           rot=(math.radians(90), 0, 0), verts=8)
                parts.append(tube)
            parts.append(pod)
        return finalize_part(parts, outname, "M_Weapon_Missile", origin='ORIGIN_CENTER_OF_VOLUME')
    # cannon (default): twin/single gun pods
    parts = []
    for side in (-1, 1) if twin else (0,):
        dom = greeble(f"Weapon_{side}", loc=(side*42*k, -6*k, 0), sx=16*k, sy=90*k, sz=16*k)
        barrel = cyl(f"Barrel_{side}", 5*k, 40*k, loc=(side*42*k, 60*k, 0),
                     rot=(math.radians(90), 0, 0), verts=10)
        parts += [dom, barrel]
    return finalize_part(parts, outname, "M_Weapon_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_sensor_part(sz, outname, asym=False, variant='mast'):
    """Sensor array add-on. variant: 'mast' (post+tip), 'dome' (radar dome),
    'cross' (phased-array lattice)."""
    k = sc(sz)
    if variant == 'dome':
        base = box("Dome_Base", 40*k, 40*k, 14*k, loc=(0, 0, 0)); bevel(base, 3, 1)
        dome = sphere("Dome", 24*k, loc=(0, 0, 34*k), verts=16)
        return finalize_part([base, dome], outname, "M_Sensor_Dome", origin='ORIGIN_CENTER_OF_VOLUME')
    if variant == 'cross':
        post = cyl("Cross_Post", 5*k, 90*k, loc=(0, 0, 45*k), verts=10)
        bar_x = box("Cross_X", 90*k, 6*k, 6*k, loc=(0, 0, 90*k))
        bar_y = box("Cross_Y", 6*k, 90*k, 6*k, loc=(0, 0, 90*k))
        tip = sphere("Cross_Tip", 7*k, loc=(0, 0, 100*k), verts=10)
        return finalize_part([post, bar_x, bar_y, tip], outname, "M_Sensor_Cross", origin='ORIGIN_CENTER_OF_VOLUME')
    # mast (default)
    mast = cyl("Sensor_Mast", 6*k, 140*k, loc=(0, 0, 90*k), verts=12)
    tip = sphere("Sensor_Tip", 9*k, loc=(0, 0, 170*k), verts=12)
    parts = [mast, tip]
    if asym:
        dome = sphere("Sensor_Dome", 14*k, loc=(60*k, 20*k, 10*k), verts=12)
        parts.append(dome)
    return finalize_part(parts, outname, "M_Sensor_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_reactor_part(sz, outname, variant='core'):
    """Reactor/core add-on. variant: 'core' (block + bands), 'fusion_ring'
    (torus reactor), 'spike' (reactor with heat-spike fins)."""
    k = sc(sz)
    if variant == 'fusion_ring':
        ring = torus("Fusion_Ring", 60*k, 20*k, loc=(0, 0, 0), maj=30, minr=8)
        core = sphere("Fusion_Core", 26*k, loc=(0, 0, 0), verts=14)
        return finalize_part([ring, core], outname, "M_Reactor_Fusion", origin='ORIGIN_CENTER_OF_VOLUME')
    if variant == 'spike':
        core = box("Spike_Core", 80*k, 80*k, 80*k, loc=(0, 0, 0)); bevel(core, 5, 2)
        spikes = []
        for i, (sx, sy, sz) in enumerate([(1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)]):
            sp = cone(f"Spike{i}", 10*k, 50*k, loc=(sx*60*k, sy*60*k, sz*60*k), verts=8)
            spikes.append(sp)
        return finalize_part([core] + spikes, outname, "M_Reactor_Spike", origin='ORIGIN_CENTER_OF_VOLUME')
    # core (default)
    core = box("Reactor_Core", 90*k, 90*k, 90*k, loc=(0, 0, 0))
    bevel(core, 5, 2)
    bands = [greeble(f"Reactor_Band{i}", loc=(0, 0, z*70*k), sx=100*k, sy=100*k, sz=14*k)
             for i, z in enumerate([-0.5, 0, 0.5])]
    return finalize_part([core] + bands, outname, "M_Reactor_Block", origin='ORIGIN_CENTER_OF_VOLUME')


def build_canopy_part(sz, outname):
    """Glazed command canopy / windscreen — the visible 'pilot's eye'. A prominent
    forward glazed bubble (sphere) + a wide tilted windscreen pane + a lit emissive
    rim band, sized to read from outside against the hull. Carries M_Canopy so it
    renders as dark-tinted glass with a lit edge, distinct from hull material.
    Mounts at the forward command deck."""
    k = sc(sz)
    # prominent glazed bubble (the easy-toread cockpit form)
    bubble = sphere("Canopy_Bubble", 40*k, loc=(0, 0, 0), verts=16)
    bubble.scale = (1.15, 1.5, 0.9)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bevel(bubble, 2, 1)
    # wide tilted windscreen pane sweeping forward
    pane = box("Canopy_Pane", 88*k, 120*k, 18*k, loc=(0, 22*k, 6*k))
    pane.rotation_euler = (math.radians(34), 0, 0)
    bevel(pane, 7, 3)
    # lit emissive rim band along the forward edge (reads as lit viewport/glow)
    strip = box("Canopy_Strip", 82*k, 8*k, 6*k, loc=(0, 40*k, -4*k))
    bevel(strip, 3, 2)
    return finalize_part([bubble, pane, strip], outname, "M_Canopy", origin='ORIGIN_CENTER_OF_VOLUME')


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


# ----------------------------------------------------------------------------
# Hardpoint module-mounting system
# ----------------------------------------------------------------------------
# A data-driven way to attach modules onto a ship carcass and to describe those
# mount points to the GAME (Starfield/SE-style module builder). Each hardpoint
# is a named socket: {name, position (units), mount_scale, allowed part types}.
# The generator can mount parts by hardpoint, and emit a JSON manifest the game
# consumes (which modules fit which hardpoints, where they sit, how they scale).
#
# Hardpoints are defined per size-class (positions relative to the class dims).

HARDPOINT_DEFS = {
    # keyed by size class -> {name: {pos:(dx,dy,dz) frac-of-class, types:[...]}}
    'small': {
        'HP_Engine':      {'pos': (0, -0.52, -0.06), 'types': ['engine']},
        'HP_Cargo':       {'pos': (0, -0.05, -0.18), 'types': ['cargo']},
        'HP_Nose':        {'pos': (0,  0.34, -0.22), 'types': ['weapon']},
        'HP_Sensor':      {'pos': (0,  0.18,  0.72), 'types': ['sensor']},
        'HP_Core':        {'pos': (0, -0.35,  0.40), 'types': ['reactor']},
        'HP_Underslung':  {'pos': (0, -0.30, -0.16), 'types': ['drill']},
        'HP_Forward':     {'pos': (0,  0.30, -0.10), 'types': ['mining_laser']},
    },
    'medium': {
        'HP_Engine':      {'pos': (0, -0.52, -0.03), 'types': ['engine']},
        'HP_Cargo':       {'pos': (0, -0.05, -0.10), 'types': ['cargo']},
        'HP_Nose':        {'pos': (0,  0.30, -0.13), 'types': ['weapon']},
        'HP_Sensor':      {'pos': (0,  0.18,  0.68), 'types': ['sensor']},
        'HP_Core':        {'pos': (0, -0.35,  0.36), 'types': ['reactor']},
        'HP_Underslung':  {'pos': (0, -0.30, -0.11), 'types': ['drill']},
        'HP_Forward':     {'pos': (0,  0.30, -0.06), 'types': ['mining_laser']},
    },
    'corvette': {
        'HP_Engine':      {'pos': (0, -0.50, -0.04), 'types': ['engine']},
        'HP_Cargo':       {'pos': (0, -0.04, -0.09), 'types': ['cargo']},
        'HP_Nose':        {'pos': (0,  0.32, -0.14), 'types': ['weapon']},
        'HP_Sensor':      {'pos': (0,  0.18,  0.70), 'types': ['sensor']},
        'HP_Core':        {'pos': (0, -0.34,  0.38), 'types': ['reactor']},
        'HP_Underslung':  {'pos': (0, -0.30, -0.11), 'types': ['drill']},
        'HP_Forward':     {'pos': (0,  0.30, -0.07), 'types': ['mining_laser']},
    },
    'large': {
        'HP_Engine':      {'pos': (0, -0.50, -0.03), 'types': ['engine']},
        'HP_Cargo':       {'pos': (0, -0.04, -0.08), 'types': ['cargo']},
        'HP_Nose':        {'pos': (0,  0.30, -0.12), 'types': ['weapon']},
        'HP_Sensor':      {'pos': (0,  0.18,  0.66), 'types': ['sensor']},
        'HP_Core':        {'pos': (0, -0.34,  0.36), 'types': ['reactor']},
        'HP_Underslung':  {'pos': (0, -0.30, -0.09), 'types': ['drill']},
        'HP_Forward':     {'pos': (0,  0.30, -0.06), 'types': ['mining_laser']},
    },
}


def hardpoint_positions(sz, dims):
    """Given size-class 'sz' and carcass (lx,ly,lz,locz)-ish dims, return
    {hardpoint_name: (world_x, world_y, world_z)} for mount placement."""
    lx, ly, lz, locz = dims
    k = sc(sz)
    defs = HARDPOINT_DEFS.get(sz, {})
    out = {}
    for name, spec in defs.items():
        dx, dy, dz = spec['pos']
        # convert fractional offsets to world units (y,z scale by their actual
        # dimensions; x offsets left at 0 since these are centered sockets)
        out[name] = (dx * lx, dy * ly, locz + dz * lz)
    return out


def build_hardpoint_manifest(sz, dims, outname, opts):
    """Emit a JSON manifest describing this ship's hardpoints + which module
    types each accepts. The game reads this to render its module-builder UI.
    Returns path to the JSON."""
    import json
    lx, ly, lz, locz = dims
    k = sc(sz)
    manifest = {
        "ship": outname,
        "size_class": sz,
        "scale": k,
        "carcass_dims": [lx, ly, lz],
        "hardpoints": {},
    }
    defs = HARDPOINT_DEFS.get(sz, {})
    for name, spec in defs.items():
        hp = hardpoint_positions(sz, dims)[name]
        manifest["hardpoints"][name] = {
            "position": [round(float(x), 1) for x in hp],
            "mount_orientation": "AXIS_FORWARD",   # -Y forward, Z-up (ship convention)
            "allowed_types": spec['types'],
            "occupied": opts.get(name_to_type_key(name)) if name_to_type_key(name) in opts else False,
        }
    out = os.path.join(ASSETS_OUT, f"{outname}_hardpoints.json")
    with open(out, 'w') as f:
        json.dump(manifest, f, indent=2)
    return out


def name_to_type_key(hp_name):
    """Map a hardpoint name to its module-type option key (e.g. HP_Engine->engine).
    Keeps manifest `occupied` accurate to what assemble mounts there."""
    m = {'HP_Engine':'engine', 'HP_Cargo':'cargo', 'HP_Nose':'weapon',
         'HP_Sensor':'sensor', 'HP_Core':'reactor', 'HP_Underslung':'drill',
         'HP_Forward':'mining_laser'}
    return m.get(hp_name, '')


# Catalog of available module VARIANTS per type — the game's builder offers
# these choices for each hardpoint that accepts the type.
MODULE_CATALOG = {
    'engine': {
        'standard': 'Twin nacelles + bells (balanced)',
        'ion':      'Long ion-drive pylons (fast, sleek)',
        'compact':  'Squat single pod (budget)',
    },
    'cargo': {
        'containers': 'Container bay',
        'bulk_tank':  'Cylindrical bulk tank',
        'flat_rack':  'Open frame rack',
    },
    'weapon': {
        'cannon':   'Twin/single gun pods',
        'tri_laser': 'Triple-laser mount',
        'missile':  'Missile pods',
    },
    'sensor': {
        'mast':  'Mast + tip',
        'dome':  'Radar dome',
        'cross': 'Phased-array lattice',
    },
    'reactor': {
        'core':       'Block core + bands',
        'fusion_ring': 'Fusion torus',
        'spike':       'Heat-spike reactor',
    },
}


def build_module_catalog(outname="SM_Modules_catalog"):
    """Emit a JSON catalog of all module types + variants + the hardpoints that
    accept them. The game reads this to populate its module-builder menu."""
    import json
    catalog = {"modules": {}}
    for mtype, variants in MODULE_CATALOG.items():
        # which hardpoints accept this type (union across size classes)
        hps = set()
        for sz, defs in HARDPOINT_DEFS.items():
            for hp, spec in defs.items():
                if mtype in spec['types']:
                    hps.add(hp)
        catalog["modules"][mtype] = {
            "variant_name": mtype,
            "options": [{"id": v, "description": d} for v, d in variants.items()],
            "fits_hardpoints": sorted(hps),
        }
    out = os.path.join(ASSETS_OUT, f"{outname}.json")
    with open(out, 'w') as f:
        json.dump(catalog, f, indent=2)
    return out


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

    # NOTE: mounting keeps its tuned per-part offsets below (these were visually
    # dialed in per class). The hardpoint manifest (also emitted) records the
    # NOMINAL socket positions + allowed types for the game's module-builder;
    # it doesn't replace the tuned in-mesh mount.
    if opts.get('engine'):
        eobj, ep = build_engine_part(sz, f"{outname}_Engine", variant=opts.get('engine_variant', 'standard'))
        eobj.location = (0, -ly*0.52, locz - 10*k)
        results.append((eobj, ep))
    if opts.get('cargo'):
        cobj, cp = build_cargo_part(sz, f"{outname}_Cargo", variant=opts.get('cargo_variant', 'containers'))
        cobj.location = (0, -ly*0.05, locz - 20*k)
        results.append((cobj, cp))
    if opts.get('weapon'):
        wobj, wp = build_weapon_part(sz, f"{outname}_Weapon", twin=opts.get('weapon_twin', True),
                                     variant=opts.get('weapon_variant', 'cannon'))
        wobj.location = (0, ly*0.34, locz - 25*k)
        results.append((wobj, wp))
    if opts.get('sensor'):
        sobj, sp = build_sensor_part(sz, f"{outname}_Sensor", asym=opts.get('sensor_asym', False),
                                     variant=opts.get('sensor_variant', 'mast'))
        sobj.location = (0, ly*0.18, locz + 70*k)
        results.append((sobj, sp))
    if opts.get('reactor'):
        robj, rp = build_reactor_part(sz, f"{outname}_Reactor", variant=opts.get('reactor_variant', 'core'))
        robj.location = (0, -ly*0.35, locz + 40*k)
        results.append((robj, rp))
    if opts.get('mining_laser'):
        mj, mp = build_mining_laser(sz, f"{outname}_MiningLaser")
        mj.location = (0, ly*0.3, locz - 10*k)
        results.append((mj, mp))
    # A visible cockpit/bridge is mounted by default on every ship so it reads as
    # a piloted vessel from outside; opt out with 'canopy': False if a ship is
    # truly unmanned/drone.
    if opts.get('canopy', True):
        cop = build_canopy_part(sz, f"{outname}_Canopy")
        copobj, cppath = cop
        # forward command position: ahead of the dorsal bridge/ridge, elevated
        copobj.location = (0, ly*0.22, locz + 55*k)
        results.append((copobj, cppath))
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

    # Emit the hardpoint manifest for the game's module-builder (best-effort).
    try:
        build_hardpoint_manifest(sz, dims, outname, opts)
    except Exception as e:
        print(f"  [warn] hardpoint manifest failed for {outname}: {e}")
    return results


def build_tapered_hull(lx, ly, lz, locz, k=1.0, prefix="Hull"):
    """Build a TAPERED, layered fuselage instead of a plain box — the X4-style
    silhouette. Layers stack from bottom to top with offset + scaling so the
    hull reads as angled armor plating, not a single block.

    Returns list of mesh objects."""
    objs = []
    # main tapered belly (wider at rear, tapering narrower forward)
    for i, (sx, sy, sz, yoff) in enumerate([
        (1.00, 1.00, 0.40, -0.12),   # lower belly
        (0.92, 0.94, 0.36, -0.02),   # mid
        (0.82, 0.86, 0.32,  0.08),   # upper
        (0.66, 0.72, 0.26,  0.20),   # spine apex
    ]):
        seg = box(f"{prefix}_Layer{i}", lx*sx, ly*sy, lz*sz,
                  loc=(0, yoff*ly, locz + lz*(0.5 - (i+0.5)*0.24)),
                  rot=(0, math.radians(sy*1.2), math.radians(sx*8)))
        bevel(seg, 6, 2)
        objs.append(seg)
    # angled nose cone (tapered fore) - replaces the box nose
    nose = cone(f"{prefix}_Nose", ly*0.10, ly*0.55, loc=(0, ly*0.60, locz + lz*0.1),
                rot=(math.radians(90), 0, 0), verts=16)
    bevel(nose, 2, 1)
    objs.append(nose)
    # spine ridge (dorsal raised rail)
    spine = box(f"{prefix}_Spine", lx*0.34, ly*0.30, lz*0.30, loc=(0, -ly*0.05, locz + lz*0.75),
                rot=(0, math.radians(4), 0)); bevel(spine, 4, 2)
    objs.append(spine)
    return objs


def assemble_whole_ship(sz, outname, opts, carcass_builder=None):
    """Build a WHOLE ship as a single joined mesh (carcass + all mounted parts
    joined into one object at their true positions). Produces a ready-to-place
    `<outname>_Assembled.fbx` that reads as an actual ship silhouette.

    Uses ONLY raw primitives (never finalize_part, which deletes sibling meshes),
    then joins once into a single mesh.
    """
    clear_scene()
    objs = []
    k = sc(sz)
    s = SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']; locz = s['z']

    # --- carcass (TAPERED layered hull + nose + spine) ---
    objs += build_tapered_hull(lx, ly, lz, locz, k)

    # --- engine block (twin nacelles + bells) ---
    if opts.get('engine'):
        eb = box("EngBlk", 150*k, 200*k, 95*k, loc=(0, -ly*0.52, locz - 10*k)); bevel(eb, 5, 2)
        objs.append(eb)
        for side in (-1, 1):
            nac = box(f"EngNac{side}", 60*k, 150*k, 72*k, loc=(side*120*k, -ly*0.5, locz - 20*k),
                      rot=(0,0,math.radians(8*side))); bevel(nac,4,1)
            noz = cone(f"EngNoz{side}", 22*k, 55*k, loc=(side*120*k, -ly*0.62, locz - 25*k),
                       rot=(math.radians(90),0,0), verts=14)
            objs += [nac, noz]

    # --- cargo bay + cells ---
    if opts.get('cargo'):
        cb = box("Cargo", 200*k, 260*k, 90*k, loc=(0, -ly*0.05, locz - 20*k)); bevel(cb,4,2)
        objs.append(cb)
        for i, dx in enumerate([-60, 0, 60]):
            c = greeble(f"CargoCell{i}", loc=(dx*k, -ly*0.08, locz - 5*k), sx=30*k, sy=70*k, sz=30*k)
            objs.append(c)

    # --- weapon pod ---
    if opts.get('weapon'):
        for side in (-1, 1):
            w = greeble(f"Weapon{side}", loc=(side*42*k, ly*0.34, locz - 25*k), sx=16*k, sy=90*k, sz=16*k)
            barrel = cyl(f"Barrel{side}", 5*k, 40*k, loc=(side*42*k, ly*0.42, locz - 25*k),
                         rot=(math.radians(90),0,0), verts=10)
            objs += [w, barrel]

    # --- sensor mast ---
    if opts.get('sensor'):
        mast = cyl("SensorMast", 6*k, 140*k, loc=(0, ly*0.18, locz + 70*k), verts=12)
        tip = sphere("SensorTip", 9*k, loc=(0, ly*0.18, locz + 160*k), verts=12)
        objs += [mast, tip]

    # --- reactor core ---
    if opts.get('reactor'):
        core = box("Reactor", 90*k, 90*k, 90*k, loc=(0, -ly*0.35, locz + 40*k)); bevel(core,5,2)
        objs.append(core)

    # --- glazed command canopy (forward pilot/crew viewport) ---
    if opts.get('canopy', True):
        pane = box("CanopyPane", 74*k, lz, 14*k, loc=(0, ly*0.22, locz + 55*k))
        pane.rotation_euler = (math.radians(38), 0, 0)
        glass = box("CanopyGlass", 64*k, lz*0.8, 8*k, loc=(0, ly*0.20, locz + 68*k))
        glass.rotation_euler = (math.radians(38), 0, 0)
        objs += [pane, glass]

    # --- drill rig (underslung) ---
    if opts.get('drill'):
        boom = box("DrillBoom", 26*k, 220*k, 34*k, loc=(0, -ly*0.3, locz - 15*k)); bevel(boom,4,2)
        head = cone("DrillHead", 30*k, 90*k, loc=(0, -ly*0.45, locz - 20*k), verts=16)
        objs += [boom, head]

    # --- mining laser ---
    if opts.get('mining_laser'):
        barrel = cyl("MLBarrel", 10*k, 160*k, loc=(0, ly*0.35, locz - 10*k),
                     rot=(math.radians(90),0,0), verts=14)
        emitter = box("MLEmit", 26*k, 24*k, 30*k, loc=(0, ly*0.42, locz - 10*k)); bevel(emitter,3,1)
        objs += [barrel, emitter]

    # ===================== DETAIL PASS =====================
    # Add scale-aware surface detail so the ship reads as a real vessel, not
    # plain boxes: hull panel ridges, vents, antennae, sensor bumps, thrusters,
    # stabilizers, greebles. All sized to the size-class scalar.
    dk = max(k, 1.0)
    # --- hull panel ridges along the flanks (reveals "plating" seams) ---
    for i, xoff in enumerate([-0.42, 0.42]):
        for j, yfrac in enumerate([0.15, 0.45, 0.72]):
            r = box(f"PanelRidge{i}_{j}", 8*k, lz*0.30, 10*k,
                    loc=(xoff*lx, (yfrac-0.5)*ly, locz + lz*0.15),
                    rot=(math.radians(4), 0, 0)); bevel(r, 1, 1)
            objs.append(r)
    # --- dorsal vent/grille strips along the top deck ---
    for i, xoff in enumerate([-0.16, 0.16]):
        for j, yfrac in enumerate([0.05, 0.5, 0.9]):
            v = box(f"Vent{i}_{j}", 6*k, lz*0.5, 4*k,
                    loc=(xoff*lx, (yfrac-0.5)*ly, locz + lz*0.55),
                    rot=(0, math.radians(6), 0)); bevel(v, 1, 1)
            objs.append(v)
    # --- fore antenna array (2-3 masts with tips) ---
    for i, a in enumerate([(-0.2, ly*0.55), (0.2, ly*0.55), (0, ly*0.4)]):
        if i == 2 and not opts.get('sensor'):
            continue
        ax, ay = a
        m = cyl(f"AntMast{i}", 3*k, 55*k, loc=(ax*lx, ay, locz + lz*0.95), verts=8)
        t = sphere(f"AntTip{i}", 5*k, loc=(ax*lx, ay, locz + lz*0.95 + 55*k), verts=8)
        bevel(m, 1, 1)
        objs += [m, t]
    # --- dorsal sensor bumps near the nose ---
    for i, xoff in enumerate([-0.3, 0.3]):
        g = greeble(f"SensorBump{i}", loc=(xoff*lx, ly*0.5, locz + lz*0.42),
                    sx=12*k, sy=14*k, sz=8*k)
        objs.append(g)
    # --- rear thruster exhausts + side maneuvering thrusters ---
    for side in (-1, 1):
        ex = cyl(f"ExNoz{side}", 7*k, 22*k, loc=(side*0.30*lx, -ly*0.72, locz + lz*0.1),
                 rot=(math.radians(90), 0, 0), verts=10)
        thr = cyl(f"ManeuverThr{side}", 5*k, 12*k, loc=(side*0.55*lx, -ly*0.3, locz + lz*0.35),
                  rot=(0, math.radians(90), 0), verts=8)
        objs += [ex, thr]
    # --- tail winglet stabilizers (if not a pure box silhouette) ---
    for side in (-1, 1):
        wl = box(f"Winglet{side}", 16*k, 90*k, 60*k, loc=(side*0.4*lx, -ly*0.78, locz + lz*0.55),
                 rot=(0, 0, math.radians(-14*side))); bevel(wl, 4, 1)
        objs.append(wl)
    # --- HULL GREEBBLE CLADDING: dense X4-style kitbash (mirrored for symmetry) ---
    import random
    rng = random.Random(hash(outname) % 100000)
    def clamp(v):
        return max(-0.98, min(0.98, v))
    gp = []  # (loc, size, rot, kind)
    # large cladding plates/ridges scattered across the hull
    for i in range(12):
        gx = clamp((rng.random()-0.5)*1.7)
        gy = clamp((rng.random()*0.9 - 0.45))
        gz = clamp(rng.random()*0.9 + 0.05)
        gp.append((gx, gy, gz,
                   (14+rng.random()*22, 6+rng.random()*20, 5+rng.random()*10),
                   (rng.random()*0.25, rng.random()*0.25, rng.random()*0.35),
                   'plate'))
    # conduit pipes / cable runs (thin cylinders along hull)
    for i in range(8):
        gx = clamp((rng.random()-0.5)*1.6)
        gy = clamp((rng.random()*0.9 - 0.45))
        gz = clamp(rng.random()*0.7 + 0.15)
        gp.append((gx, gy, gz,
                   (6+rng.random()*8, 3+rng.random()*4, 3+rng.random()*4),
                   (rng.random()*0.4, rng.random()*0.4, rng.random()*0.4),
                   'pipe'))
    # small tanks / canisters
    for i in range(6):
        gx = clamp((rng.random()-0.5)*1.5)
        gy = clamp((rng.random()*0.8 - 0.4))
        gz = clamp(rng.random()*0.5 + 0.3)
        gp.append((gx, gy, gz,
                   (0, 0, 0),
                   (0, 0, 0),
                   'tank'))
    # antennas / masts
    for i in range(5):
        gx = clamp((rng.random()-0.5)*1.4)
        gy = clamp((rng.random()*0.7 - 0.3))
        gz = clamp(rng.random()*0.5 + 0.4)
        gp.append((gx, gy, gz,
                   (0, 0, 0),
                   (0, 0, 0),
                   'antenna'))
    for i, (gx, gy, gz, size, rot, kind) in enumerate(gp):
        px, py, pz = gx*lx, gy*ly, locz + gz*lz
        if kind == 'plate':
            objs.append(greeble(f"KPlate{i}", loc=(px, py, pz), sx=size[0]*k,
                                sy=size[1]*k, sz=size[2]*k, rot=rot))
            objs.append(greeble(f"KPlateM{i}", loc=(-px, py, pz), sx=size[0]*k,
                                sy=size[1]*k, sz=size[2]*k, rot=(-rot[0], rot[1], rot[2])))
        elif kind == 'pipe':
            c = cyl(f"KPipe{i}", size[1]*k, (14+rng.random()*20)*k, loc=(px, py, pz),
                    rot=(rng.random()*math.pi, rng.random()*math.pi/2, rng.random()*math.pi),
                    verts=6)
            bevel(c, 1, 1); objs.append(c)
            c2 = cyl(f"KPipeM{i}", size[1]*k, (14+rng.random()*20)*k, loc=(-px, py, pz),
                     rot=(rng.random()*math.pi, rng.random()*math.pi/2, rng.random()*math.pi),
                     verts=6)
            bevel(c2, 1, 1); objs.append(c2)
        elif kind == 'tank':
            t = cyl(f"KTank{i}", (8+rng.random()*8)*k, (12+rng.random()*14)*k,
                    loc=(px, py, pz), rot=(rng.random()*math.pi/2, 0, 0), verts=10)
            bevel(t, 1, 1); objs.append(t)
            t2 = cyl(f"KTankM{i}", (8+rng.random()*8)*k, (12+rng.random()*14)*k,
                     loc=(-px, py, pz), rot=(rng.random()*math.pi/2, 0, 0), verts=10)
            bevel(t2, 1, 1); objs.append(t2)
        elif kind == 'antenna':
            a = cyl(f"KAnt{i}", 2*k, (20+rng.random()*20)*k, loc=(px, py, pz),
                    verts=6)
            tip = sphere(f"KAntTip{i}", 3*k, loc=(px, py, pz + 28*k), verts=6)
            objs += [a, tip]
            a2 = cyl(f"KAntM{i}", 2*k, (20+rng.random()*20)*k, loc=(-px, py, pz), verts=6)
            tip2 = sphere(f"KAntTipM{i}", 3*k, loc=(-px, py, pz + 28*k), verts=6)
            objs += [a2, tip2]
    # =================== /DETAIL PASS ===================

    # join everything into one mesh (raw join, no finalize deletion)
    joined = join(objs, f"{outname}_AssembledGeo")
    apply_mods(joined)
    clean_mesh(joined)
    smart_uv(joined)
    joined.name = f"{outname}_Assembled"
    m = bpy.data.materials.new("M_Assembled")
    m.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(m)
    out = export_fbx(joined, f"{outname}_Assembled")
    return [(joined, out)]


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
    """Standard spine corridor + hatches (joinable between modules).

    Enriched vs v1: added wall paneling strips, floor conduit runs, ceiling
    light strips, emergency bulkhead rings, junction boxes, and hatches at
    each end so it reads as a proper pressurized corridor.
    """
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
    # wall paneling + conduit runs up the side walls
    for x in range(-120, 121, 30):
        pnl = box(f"WallPanel{x}", 10*k, 3*k, 40*k, loc=(x, 34*k, 35*k))
        parts.append(pnl)
        cond = box(f"Conduit{x}", 4*k, 4*k, H-10*k, loc=(x, -30*k, H/2))
        parts.append(cond)
    # junction boxes on the ceiling + emergency hatch lights
    for x in (-100, 0, 100):
        jb = box(f"Junction{x}", 14*k, 8*k, 4*k, loc=(x, 0, H-4*k))
        parts.append(jb)
    # end hatches (door frames)
    for side in (-1, 1):
        fr_l = box(f"HatchFrame{side}L", 4*k, 6*k, H-10*k, loc=(side*(L/2-8*k), -30*k, 40*k))
        fr_r = box(f"HatchFrame{side}R", 4*k, 6*k, H-10*k, loc=(side*(L/2-8*k), 30*k, 40*k))
        fr_t = box(f"HatchFrame{side}T", 4*k, 60*k, 8*k, loc=(side*(L/2-8*k), 0, H-8*k))
        hatch = box(f"Hatch{side}", 3*k, 52*k, H-14*k, loc=(side*(L/2-2*k), 0, 40*k))  # door slab
        parts += [fr_l, fr_r, fr_t, hatch]
    jo, out = finalize_part(parts, prefix + "_Corridor", "M_Interior_Hab")
    return [(jo, out)]


def build_engineering_bay(sz, outname):
    """Engineering / mechanical bay interior — reactor housing, pipe walls,
    tool benches, conduit rigging. A distinct interior role from crew quarters."""
    k = sc(sz)
    parts = []
    L = 300*k; W = 160*k; H = 120*k
    # shell
    floor = box("EngFloor", L, W, 8*k, loc=(0,0,0)); bevel(floor,2,1); parts.append(floor)
    for side in (-1,1):
        w = box(f"EngWall{side}", 8*k, W, H, loc=(side*L/2, 0, H/2)); bevel(w,2,1); parts.append(w)
    ceil = box("EngCeil", L, W, 6*k, loc=(0,0,H)); bevel(ceil,2,1); parts.append(ceil)
    # central reactor core housing (large cylinder + bands)
    core = cyl("EngCore", 34*k, 90*k, loc=(0, -30*k, 45*k), verts=20); bevel(core,3,1)
    core_band1 = torus("CoreBand1", 34*k, 5*k, loc=(0, -30*k, 25*k), maj=24, minr=6)
    core_band2 = torus("CoreBand2", 34*k, 5*k, loc=(0, -30*k, 70*k), maj=24, minr=6)
    parts += [core, core_band1, core_band2]
    # pipe racks along one wall + energy conduits across the ceiling
    for y in range(int(-W/2)+40, int(W/2), 50):
        rack = box(f"PipeRack{y}", 60*k, 6*k, 8*k, loc=(-L/2+40*k, y, 40*k)); bevel(rack,1,1)
        for p in (-1,1):
            pipe = cyl(f"Pipe{y}_{p}", 4*k, 12*k, loc=(-L/2+40*k+p*14*k, y, 50*k),
                       rot=(math.radians(90),0,0), verts=8)
            parts.append(pipe)
        parts.append(rack)
    # ceiling conduit rigging (cross beams + hanging conduits)
    for x in range(-100, 101, 100):
        beam = box(f"CeilBeam{x}", 8*k, W-20*k, 6*k, loc=(x, 0, H-20*k))
        parts.append(beam)
    # work benches + tool wall + floor battery banks
    bench1 = box("WorkBench1", 80*k, 20*k, 35*k, loc=(L/2-60*k, -50*k, 20*k)); bevel(bench1,2,1)
    bench2 = box("WorkBench2", 60*k, 12*k, 30*k, loc=(L/2-50*k, 40*k, 17*k)); bevel(bench2,2,1)
    tool_wall = box("ToolWall", 5*k, 60*k, 40*k, loc=(L/2-80*k, -30*k, 55*k)); bevel(tool_wall,1,1)
    # batteries along the back
    for i in range(5):
        bat = box(f"Battery{i}", 22*k, 22*k, 50*k, loc=(-L/2+30*k+i*45*k, W/2-16*k, 30*k)); bevel(bat,2,1)
        parts.append(bat)
    # emergency light strips + hazard floor strips
    for x in (-100, 0, 100):
        ls = box(f"EngLight{x}", 40*k, 5*k, 4*k, loc=(x, 0, H-4*k))
        parts.append(ls)
    for y in range(int(-W/2)+10, int(W/2), 40):
        hz = box(f"EngHazard{y}", L-20*k, 6*k, 2*k, loc=(0, y, 5*k))
        parts.append(hz)
    parts += [bench1, bench2, tool_wall]
    jo, out = finalize_part(parts, outname, "M_Interior_Eng")
    return [(jo, out)]


def build_airlock(prefix):
    """Airlock module — the threshold between interior and exterior void."""
    k = 1.0
    parts = []
    L = 140; W = 80; H = 70
    floor = box("ALFloor", L, W, 6*k, loc=(0,0,0)); bevel(floor,1,1); parts.append(floor)
    for side in (-1,1):
        w = box(f"ALWall{side}", 6*k, W, H, loc=(side*L/2, 0, H/2)); bevel(w,1,1); parts.append(w)
    ceil = box("ALCeil", L, W, 5*k, loc=(0,0,H)); bevel(ceil,1,1); parts.append(ceil)
    # inner + outer pressure hatches
    for x in (-L/2+12, L/2-12):
        fr_l = box(f"ALFrL{x}", 6*k, 5*k, H-8*k, loc=(x, -32*k, 40*k))
        fr_r = box(f"ALFrR{x}", 6*k, 5*k, H-8*k, loc=(x, 32*k, 40*k))
        fr_t = box(f"ALFrT{x}", 6*k, 64*k, 6*k, loc=(x, 0, H-6*k))
        door = box(f"ALDoor{x}", 4*k, 50*k, H-12*k, loc=(x, 0, 40*k))  # pressure slab
        parts += [fr_l, fr_r, fr_t, door]
    # light strips + lockers + pressure-lines
    for x in (-40, 0, 40):
        parts.append(box(f"ALLight{x}", 20*k, 5*k, 3*k, loc=(x, 0, H-5*k)))
    for side in (-1,1):
        lk = box(f"ALLocker{side}", 26*k, 4*k, 55*k, loc=(side*38*k, 0, 30*k)); bevel(lk,1,1)
        parts.append(lk)
    # floor conduit
    parts.append(box("ALConduit", L-20*k, 4*k, 4*k, loc=(0, -34*k, 5*k)))
    jo, out = finalize_part(parts, prefix + "_Airlock", "M_Interior_Hab")
    return [(jo, out)]


def build_interior_set():
    """Build all interior instances (cockpit, crew quarters, hab, corridor, +
    engineering bay, airlock)."""
    results = []
    results += build_cockpit_interior('small', "SM_Int_Fighter_Cockpit")
    results += build_hab_interior('medium', "SM_Int_Freighter_CrewQuarters")
    results += build_hab_interior('large', "SM_Int_Generationship_Hab", room_count=2)
    results += build_corridor("SM_Int_Standard")
    # new interior roles
    results += build_engineering_bay('medium', "SM_Int_Freighter_EngineRoom")
    results += build_airlock("SM_Int_Standard")
    return results


# ----------------------------------------------------------------------------
# Combat / weapon-fx assets (projectiles, missile, debris, hull-break pieces)
# ----------------------------------------------------------------------------
# These give the gameplay side (targeting/combat, which is next) ready-to-place
# meshes: rifle/plasma bolts, a missile/torpedo, and reusable debris + hull-break
# fragments for impact/damage states. Each exports as its own FBX via
# finalize_part (per-component style, like ships/props).

def build_combat_assets():
    """Generate projectile + missile + debris parts. Returns list of
    (obj, fbxpath) pairs."""
    results = []
    k = 1.0  # combat props are small; keep at unit scale

    # --- 1. Plasma bolt (glowing energy round) ---
    bolt = sphere("PlasmaBolt", 5*k, loc=(0, 0, 0), verts=14)
    glow = sphere("PlasmaGlow", 7.5*k, loc=(0, 0, 0), verts=14)
    glow.scale = (1, 1, 1.4)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    results.append(finalize_part([bolt, glow], "SM_Combat_PlasmaBolt", "M_Combat_Plasma",
                                 origin='ORIGIN_CENTER_OF_VOLUME'))

    # --- 2. Laser bolt (elongated energy round) ---
    core = cyl("LaserCore", 3.5*k, 18*k, loc=(0, 0, 0), verts=10)
    sleeve = cyl("LaserSleeve", 5.5*k, 16*k, loc=(0, 0, 0), verts=10)
    tip = cone("LaserTip", 4*k, 8*k, loc=(0, 12*k, 0), rot=(math.radians(90), 0, 0), verts=10)
    results.append(finalize_part([core, sleeve, tip], "SM_Combat_LaserBolt", "M_Combat_Laser",
                                 origin='ORIGIN_CENTER_OF_VOLUME'))

    # --- 3. Missile / torpedo ---
    body = cyl("MissileBody", 6*k, 40*k, loc=(0, 0, 0), verts=14)
    nose = cone("MissileNose", 6*k, 14*k, loc=(0, 26*k, 0),
                rot=(math.radians(90), 0, 0), verts=14)
    nozzle = cone("MissileNozzle", 4.5*k, 6*k, loc=(0, -24*k, 0),
                  rot=(math.radians(90), 0, 0), verts=14)
    fin1 = box("MissileFin1", 2*k, 10*k, 4*k, loc=(4*k, -6*k, 0)); bevel(fin1, 1, 1)
    fin2 = box("MissileFin2", 2*k, 10*k, 4*k, loc=(-4*k, -6*k, 0)); bevel(fin2, 1, 1)
    results.append(finalize_part([body, nose, nozzle, fin1, fin2], "SM_Combat_Missile",
                                 "M_Combat_Missile", origin='ORIGIN_CENTER_OF_VOLUME'))

    # --- 4. Impact flash (burst of shards around a point) ---
    flash = []
    for i in range(8):
        fr = rock(f"FragFlash{i}", 6*k, loc=(0, 0, 0), scale_xyz=(1, 1, 1))
        fr.rotation_euler = (i*0.7, i*0.5, i*0.3)
        flash.append(fr)
    results.append(finalize_part(flash, "SM_Combat_ImpactFlash", "M_Combat_Hit",
                                 origin='ORIGIN_CENTER_OF_VOLUME'))

    return results


def build_debris(n=6, radius=14):
    """Build a scatter of reusable hull-break debris rocks for damage states."""
    parts = []
    import random
    rng = random.Random(42)
    for i in range(n):
        d = rock(f"Debris{i}", radius, loc=(0, 0, 0),
                 sub=1, scale_xyz=(rng.uniform(0.5, 1.2),
                                   rng.uniform(0.5, 1.2),
                                   rng.uniform(0.4, 1.0)))
        d.rotation_euler = (rng.uniform(0, 3), rng.uniform(0, 3), rng.uniform(0, 3))
        parts.append(d)
    return parts


# ----------------------------------------------------------------------------
def main():
    setup_scene()
    print("Generating textures...")
    # existing ship/prop textures (kept for compatibility)
    gen_texture_set("Ship_Hull", {'base':[0.5,0.56,0.62], 'accent':[0.1,0.22,0.35], 'emissive':[0.2,0.65,1.0],
                                  'windows':{'cols':12,'frac':0.4,'cool':[0.3,0.7,1.0],'warm':[1.0,0.6,0.25]},
                                  'cable':{'runs':4}, 'rough':0.30, 'metal':0.97}, 2048, seed=11)
    gen_texture_set("Freighter", {'base':[0.45,0.55,0.45], 'accent':[0.15,0.35,0.18], 'emissive':[0.4,0.8,0.3],
                                  'windows':{'cols':10,'frac':0.5,'cool':[0.3,0.7,1.0],'warm':[0.5,0.9,0.4]},
                                  'grime':True, 'hazard':{'bands':3}, 'rough':0.45, 'metal':0.92}, 2048, seed=22)
    gen_texture_set("Prop_Crate", {'base':[0.45,0.5,0.55], 'accent':[0.3,0.3,0.0], 'emissive':[0.2,1.0,0.2],
                                   'hazard':{'bands':3}}, 1024, seed=33)
    gen_texture_set("Prop_Tank", {'base':[0.7,0.32,0.18], 'accent':[0.1,0.1,0.1], 'emissive':[1.0,0.2,0.1],
                                  'hazard':{'bands':4}}, 1024, seed=44)
    # cyberpunk additions
    gen_texture_set("Gunship", {'base':[0.22,0.23,0.26], 'accent':[0.4,0.05,0.05],
                                'emissive':[1.0,0.15,0.05], 'neon':[1.0,0.2,0.1], 'neon_thick':3,
                                'windows':{'cols':16,'frac':0.2,'cool':[0.3,0.7,1.0]},
                                'grime':True, 'hazard':{'bands':3}, 'rough':0.40, 'metal':0.94}, 2048, seed=55)
    gen_texture_set("Corvette", {'base':[0.4,0.34,0.46], 'accent':[0.25,0.15,0.4], 'emissive':[0.6,0.3,1.0],
                                'windows':{'cols':14,'frac':0.45,'cool':[0.6,0.5,1.0],'warm':[1.0,0.5,0.7]},
                                'grime':True, 'hazard':{'bands':3}, 'cable':{'runs':5}, 'rough':0.32, 'metal':0.96}, 2048, seed=77)
    gen_texture_set("Miner", {'base':[0.55,0.47,0.4], 'accent':[0.4,0.2,0.1], 'emissive':[1.0,0.5,0.1],
                              'windows':{'cols':10,'frac':0.3,'cool':[0.5,0.7,1.0],'warm':[1.0,0.6,0.3]},
                              'drill_accent':True, 'grime':True, 'hazard':{'bands':4}, 'cable':{'runs':6}, 'rough':0.52, 'metal':0.88}, 2048, seed=88)
    gen_texture_set("Station_Hab", {'base':[0.3,0.32,0.38], 'accent':[0.15,0.18,0.24],
                                    'emissive':[0.2,0.55,1.0],
                                    'windows':{'cols':14,'frac':0.65,'cool':[0.3,0.65,1.0],'warm':[1.0,0.6,0.25]},
                                    'neon':[0.3,1.0,1.0], 'neon_thick':2, 'grime':True,
                                    'cable':{'runs':6}, 'hazard':{'bands':3}}, 2048, seed=66)
    # ---- Phase 1: per-part texture sets (distinct material sections per part) ----
    # Each part type gets its own accent/emissive palette so a ship reads as an
    # assembled machine (cool carcass, hot engines, dark weapons, etc.) instead of
    # one uniform texture.
    gen_texture_set("Engine", {'base':[0.5,0.48,0.46], 'accent':[0.35,0.35,0.4],
                               'emissive':[1.0,0.35,0.1],  # hot engine glow
                               'neon':[1.0,0.4,0.15], 'neon_thick':2,
                               'grime':True, 'hazard':{'bands':3}, 'cable':{'runs':4}}, 2048, seed=170)
    gen_texture_set("Cargo", {'base':[0.55,0.58,0.5], 'accent':[0.22,0.3,0.2],
                              'emissive':[0.4,0.9,0.3],  # green container marker
                              'neon':[0.4,0.9,0.3], 'neon_thick':2,
                              'grime':True, 'hazard':{'bands':4}, 'cable':{'runs':5}}, 2048, seed=171)
    gen_texture_set("Weapon", {'base':[0.4,0.38,0.35], 'accent':[0.05,0.05,0.08],
                               'emissive':[1.0,0.1,0.05],  # menacing red weapon
                               'neon':[1.0,0.15,0.05], 'neon_thick':2,
                               'grime':True, 'hazard':{'bands':3}, 'cable':{'runs':3}}, 2048, seed=172)
    gen_texture_set("Sensor", {'base':[0.6,0.62,0.65], 'accent':[0.1,0.25,0.4],
                               'emissive':[0.2,0.7,1.0],  # cool sensor/scan
                               'neon':[0.2,0.7,1.0], 'neon_thick':2,
                               'cable':{'runs':4}}, 2048, seed=173)
    gen_texture_set("Reactor", {'base':[0.45,0.44,0.42], 'accent':[0.3,0.15,0.1],
                                'emissive':[1.0,0.6,0.1],  # amber reactor core
                                'neon':[1.0,0.7,0.2], 'neon_thick':3,
                                'grime':True, 'hazard':{'bands':3}, 'cable':{'runs':5}}, 2048, seed=174)
    gen_texture_set("Drill", {'base':[0.5,0.5,0.46], 'accent':[0.35,0.3,0.2],
                              'emissive':[0.9,0.6,0.2], 'grime':True,
                              'hazard':{'bands':4}, 'cable':{'runs':5}}, 1024, seed=175)
    gen_texture_set("MiningLaser", {'base':[0.45,0.42,0.4], 'accent':[0.1,0.2,0.2],
                                    'emissive':[0.3,1.0,0.6],  # teal mining beam
                                    'neon':[0.3,1.0,0.6], 'neon_thick':2,
                                    'grime':True, 'cable':{'runs':4}}, 1024, seed=176)
    # glazed command canopy texture: dark tinted glass with a lit rim/window band
    # so the cockpit/bridge reads from outside as a 'pilot's eye' viewport.
    gen_texture_set("Canopy", {'base':[0.08,0.10,0.12], 'accent':[0.2,0.45,0.6], 'emissive':[0.4,0.8,1.0],
                               'windows':{'cols':18,'frac':0.55,'cool':[0.4,0.8,1.0],'warm':[1.0,0.7,0.4]},
                               'neon':[0.4,0.8,1.0], 'neon_thick':2, 'grime':True}, 1024, seed=180)
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
    # Combat / weapon-fx textures
    gen_texture_set("Combat_Plasma", {'base':[0.9,0.3,0.2], 'accent':[0.5,0.1,0.05], 'emissive':[1.0,0.2,0.1],
                                      'neon':[1.0,0.2,0.1], 'neon_thick':3, 'hazard':{'bands':2}}, 512, seed=160)
    gen_texture_set("Combat_Laser", {'base':[0.2,0.6,0.9], 'accent':[0.1,0.35,0.6], 'emissive':[0.2,0.7,1.0],
                                     'neon':[0.2,0.7,1.0], 'neon_thick':3}, 512, seed=161)
    gen_texture_set("Combat_Missile", {'base':[0.5,0.55,0.6], 'accent':[0.3,0.3,0.35], 'emissive':[1.0,0.4,0.1],
                                       'hazard':{'bands':3}, 'grime':True}, 512, seed=162)
    gen_texture_set("Combat_Hit", {'base':[0.9,0.7,0.4], 'accent':[0.6,0.3,0.1], 'emissive':[1.0,0.6,0.2],
                                   'hazard':{'bands':2}}, 512, seed=163)

    print("Building ships (modular carcass + add-ons)...")
    # Each ship = carcass + scaled parts, each exported as its own FBX.
    ship_parts = {
        "SM_Ship_Fighter_01": assemble_ship('small',
            'SM_Ship_Fighter_01',
            {'engine': True, 'cargo': True, 'weapon': True, 'weapon_twin': True,
             'sensor': True, 'canopy': True}),
        "SM_Ship_Freighter_01": assemble_ship('medium',
            'SM_Ship_Freighter_01',
            {'engine': True, 'cargo': True, 'weapon': False, 'sensor': True,
             'reactor': True}),
        "SM_Ship_Gunship_02": assemble_ship('small',
            'SM_Ship_Gunship_02',
            {'engine': True, 'cargo': False, 'weapon': True, 'weapon_twin': True,
             'sensor': True, 'sensor_asym': True}),
    }
    # ---- Cargo / trading ship line (all sizes) — cargo-specialist haulers ----
    # Each uses build_cargo_carcass (broad box-hauler profile) + a cargo variant
    # suited to its size class, so the trade fleet reads as containerized haulers.
    ship_parts["SM_Ship_Courier_01"] = assemble_ship('small',
        'SM_Ship_Courier_01', {'engine': True, 'cargo': True, 'cargo_variant': 'flat_rack',
                               'sensor': True, 'reactor': True},
        carcass_builder=build_cargo_carcass)
    ship_parts["SM_Ship_CargoFreighter_01"] = assemble_ship('medium',
        'SM_Ship_CargoFreighter_01', {'engine': True, 'cargo': True, 'cargo_variant': 'containers',
                                      'sensor': True, 'reactor': True, 'weapon': True},
        carcass_builder=build_cargo_carcass)
    ship_parts["SM_Ship_HeavyHauler_01"] = assemble_ship('corvette',
        'SM_Ship_HeavyHauler_01', {'engine': True, 'cargo': True, 'cargo_variant': 'bulk_tank',
                                   'sensor': True, 'reactor': True, 'weapon': True,
                                   'weapon_twin': True},
        carcass_builder=build_cargo_carcass)
    ship_parts["SM_Ship_BulkCarrier_01"] = assemble_ship('large',
        'SM_Ship_BulkCarrier_01', {'engine': True, 'cargo': True, 'cargo_variant': 'containers',
                                   'sensor': True, 'reactor': True, 'weapon': True},
        carcass_builder=build_cargo_carcass)
    # ---- Trading ROLE specialists (distinct from the straight haulers) ----
    # Starter Trader: cheap medium trader, flat-rack cargo, minimal weapons — the
    # player's first freighter.
    ship_parts["SM_Ship_Trader_01"] = assemble_ship('medium',
        'SM_Ship_Trader_01', {'engine': True, 'cargo': True, 'cargo_variant': 'flat_rack',
                              'sensor': True, 'reactor': True},
        carcass_builder=build_cargo_carcass)
    # Fast Smuggler: small agile runner, compact engine, no weapons but quick —
    # sneaks small lucrative packs through hostile space.
    ship_parts["SM_Ship_Smuggler_01"] = assemble_ship('small',
        'SM_Ship_Smuggler_01', {'engine': True, 'engine_variant': 'compact',
                                'cargo': True, 'cargo_variant': 'flat_rack', 'sensor': True},
        carcass_builder=build_cargo_carcass)
    # ---- Warship line (dedicated combat, all sizes) — arrowhead hulls ----
    ship_parts["SM_Ship_Escort_01"] = assemble_ship('small',
        'SM_Ship_Escort_01', {'engine': True, 'weapon': True, 'weapon_twin': True,
                              'weapon_variant': 'cannon', 'sensor': True, 'reactor': True},
        carcass_builder=build_warship_carcass)
    ship_parts["SM_Ship_Destroyer_01"] = assemble_ship('corvette',
        'SM_Ship_Destroyer_01', {'engine': True, 'weapon': True, 'weapon_twin': True,
                                 'weapon_variant': 'tri_laser', 'sensor': True,
                                 'reactor': True, 'cargo': False},
        carcass_builder=build_warship_carcass)
    ship_parts["SM_Ship_Battleship_01"] = assemble_ship('large',
        'SM_Ship_Battleship_01', {'engine': True, 'weapon': True, 'weapon_twin': True,
                                  'weapon_variant': 'cannon', 'sensor': True, 'reactor': True,
                                  'cargo': False},
        carcass_builder=build_warship_carcass)
    ship_parts = ship_parts
    for ship_name, parts in ship_parts.items():
        for obj, out in parts:
            print(f"  {ship_name}: {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

    # Whole-ship assembled meshes (single joined object, ready-to-place)
    print("Building assembled whole ships...")
    assembled_specs = {
        "SM_Ship_Fighter_01": ('small', {'engine': True, 'cargo': True, 'weapon': True,
                                          'weapon_twin': True, 'sensor': True}),
        "SM_Ship_Freighter_01": ('medium', {'engine': True, 'cargo': True, 'sensor': True,
                                            'reactor': True}),
        "SM_Ship_Gunship_02": ('small', {'engine': True, 'weapon': True, 'weapon_twin': True,
                                         'sensor': True, 'sensor_asym': True}),
        "SM_Ship_Corvette_01": ('corvette', {'engine': True, 'cargo': True, 'weapon': True,
                                             'weapon_twin': True, 'sensor': True, 'reactor': True}),
        "SM_Ship_Miner_01": ('corvette', {'engine': True, 'cargo': True, 'drill': True,
                                          'mining_laser': True, 'sensor': True}),
        "SM_Ship_Courier_01": ('small', {'engine': True, 'cargo': True,
                                          'cargo_variant': 'flat_rack', 'sensor': True}),
        "SM_Ship_CargoFreighter_01": ('medium', {'engine': True, 'cargo': True,
                                          'sensor': True, 'reactor': True}),
        "SM_Ship_HeavyHauler_01": ('corvette', {'engine': True, 'cargo': True,
                                          'cargo_variant': 'bulk_tank', 'sensor': True,
                                          'reactor': True, 'weapon': True}),
        "SM_Ship_BulkCarrier_01": ('large', {'engine': True, 'cargo': True,
                                          'sensor': True, 'reactor': True}),
        "SM_Ship_Trader_01": ('medium', {'engine': True, 'cargo': True, 'cargo_variant': 'flat_rack',
                                          'sensor': True, 'reactor': True}),
        "SM_Ship_Smuggler_01": ('small', {'engine': True, 'engine_variant': 'compact',
                                          'cargo': True, 'cargo_variant': 'flat_rack',
                                          'sensor': True}), "SM_Ship_Escort_01": ('small', {'engine': True, 'weapon': True, 'weapon_twin': True, 'sensor': True, 'reactor': True}),
        "SM_Ship_Destroyer_01": ('corvette', {'engine': True, 'weapon': True, 'weapon_twin': True, 'weapon_variant': 'tri_laser', 'sensor': True, 'reactor': True}),
        "SM_Ship_Battleship_01": ('large', {'engine': True, 'weapon': True, 'weapon_twin': True, 'sensor': True, 'reactor': True}),
    }
    assembled_count = 0
    for name, (sz, opts) in assembled_specs.items():
        for obj, out in assemble_whole_ship(sz, name, opts):
            assembled_count += 1
            print(f"  {name}: assembled {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

    # Module VARIANT demonstration ships — prove the hardpoint mount system can
    # mix-and-match module types/variants on the same sockets.
    print("Building module-variant ships...")
    variant_specs = {
        # a 'sleek' fighter: ion engines + tri-laser + fusion reactor + dome sensor
        "SM_Ship_Fighter_02_Arcangel": ('small', {
            'engine': True, 'engine_variant': 'ion',
            'weapon': True, 'weapon_variant': 'tri_laser',
            'reactor': True, 'reactor_variant': 'fusion_ring',
            'sensor': True, 'sensor_variant': 'dome',
            'cargo': True, 'cargo_variant': 'flat_rack',
        }),
        # a 'hauler' variant of the freighter: bulk-tank cargo + spiked reactor
        "SM_Ship_Freighter_02_Bulkhauler": ('medium', {
            'engine': True, 'engine_variant': 'compact',
            'cargo': True, 'cargo_variant': 'bulk_tank',
            'reactor': True, 'reactor_variant': 'spike',
            'sensor': True, 'sensor_variant': 'cross',
        }),
    }
    for name, (sz, opts) in variant_specs.items():
        parts = assemble_ship(sz, name, opts)
        for obj, out in parts:
            print(f"  {name}: {os.path.basename(out)} ({os.path.getsize(out) if os.path.exists(out) else 0} B)")

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

    print("Building combat assets...")
    combat = build_combat_assets()
    for ob, out in combat:
        print("  exported:", os.path.basename(out), os.path.getsize(out) if os.path.exists(out) else 0)
    # debris scatter as a ready-to-use hull-break fragment set (single object)
    debris = build_debris(8, 14)
    for i, d in enumerate(debris):
        d.name = f"DebrisPiece{i}"
    dj = join(debris, "SM_Combat_Debris")
    smart_uv(dj); clean_mesh(dj)
    dmat = bpy.data.materials.get("M_Combat_Hit") or bpy.data.materials.new("M_Combat_Hit")
    dmat.use_nodes = True
    if not dj.data.materials: dj.data.materials.append(dmat)
    outd = export_fbx(dj, "SM_Combat_Debris")
    print("  exported: SM_Combat_Debris", os.path.getsize(outd) if os.path.exists(outd) else 0)

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
    # module catalog (variants the game's builder can offer per socket)
    try:
        cat = build_module_catalog()
        print("Module catalog:", os.path.basename(cat))
    except Exception as e:
        print("  [warn] module catalog failed:", e)


if __name__ == "__main__":
    main()