"""Render INTERIORS from INSIDE with textured EEVEE (self-lit hull/glow).

v2 — fixes two bugs found by vision review of v1's output:
  1. Camera eye-height used raw min/max Z, which gets dragged up by tall thin
     features (canopy rings, ceiling lights, antenna) far above where a
     standing/seated occupant's eyes actually are. Now uses a percentile-based
     floor/ceiling estimate (5th/95th percentile Z) so eye height sits at a
     sane fraction of the ROOM height, not the tallest greeble on the mesh.
  2. Split-zone interiors (Hab, Bridge = 8 separate FBX: Shell/Bunks/Desks/
     Galley/Mess/Vents/Lights/Hatch) could only be rendered ONE FBX at a time,
     and each mesh's real per-zone material was being cleared and replaced
     with a single shared texture -- defeating the whole point of the
     "separate colored items" design. Now imports ALL zone FBX for a split
     interior into ONE scene and keeps each zone's OWN baked solid-color
     material (set via solid_mat() in the generator, which survives FBX
     re-import as the diffuse base color) instead of overwriting it.

Usage:
  blender -b --python Tools/render_int_inside.py -- <fbx_or_group> [<fbx_or_group> ...]

A "group" is a base name that matches multiple zone FBX on disk, e.g.
"SM_Int_Freighter_CrewQuarters" pulls in CrewQuarters_Shell/_Bunks/_Desks/...
A single explicit filename (ending .fbx) is imported alone (e.g. non-split
interiors: SM_Int_Fighter_Cockpit.fbx, SM_Int_Xenomorph_AlienHold.fbx).
"""
import bpy, os, sys, glob, bmesh
from mathutils import Vector
from mathutils.bvhtree import BVHTree

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "interior_inside")
os.makedirs(OUT, exist_ok=True)
TEX = os.path.join(GEN, "Textures")


def load_tex(fname):
    p = os.path.join(TEX, fname)
    return p if os.path.exists(p) else None


def make_textured_material(prefixes, cache):
    """Build (once, cached by prefix-key) the shared textured-wall material
    used by single-mesh interiors that have NO per-zone solid-color material
    (cockpit, empty room, corridor, airlock, engineering bay, alien hold)."""
    key = tuple(prefixes)
    if key in cache:
        return cache[key]
    mat = bpy.data.materials.new("IntPBR_" + "_".join(prefixes))
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location = (500, 0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location = (200, 0)
    bsdf.inputs['Base Color'].default_value = (0.4, 0.42, 0.45, 1.0)
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    def tex_node(fn, cs='sRGB', x=-500, y=0):
        p = load_tex(fn)
        if not p:
            return None
        img = bpy.data.images.load(p, check_existing=True)
        try:
            img.colorspace_settings.name = cs
        except Exception:
            pass
        n = nt.nodes.new('ShaderNodeTexImage'); n.image = img; n.location = (x, y)
        return n

    result = (mat, None)
    for pref in prefixes:
        if load_tex(f"T_{pref}_D.png"):
            base = tex_node(f"T_{pref}_D.png", 'sRGB')
            nt.links.new(base.outputs['Color'], bsdf.inputs['Base Color'])
            emit = nt.nodes.new('ShaderNodeEmission'); emit.location = (200, 250)
            nt.links.new(base.outputs['Color'], emit.inputs['Color'])
            emit.inputs['Strength'].default_value = 2.0
            e = tex_node(f"T_{pref}_E.png", 'sRGB', -500, 250)
            mix0 = nt.nodes.new('ShaderNodeMixShader'); mix0.location = (420, 60)
            mix0.inputs['Fac'].default_value = 0.9
            nt.links.new(bsdf.outputs['BSDF'], mix0.inputs[1])
            nt.links.new(emit.outputs['Emission'], mix0.inputs[2])
            if e:
                neon = nt.nodes.new('ShaderNodeEmission'); neon.location = (200, 250)
                nt.links.new(e.outputs['Color'], neon.inputs['Color'])
                neon.inputs['Strength'].default_value = 2.5
                mix1 = nt.nodes.new('ShaderNodeMixShader'); mix1.location = (560, 120)
                lum = nt.nodes.new('ShaderNodeSeparateColor'); lum.location = (100, 480)
                nt.links.new(e.outputs['Color'], lum.inputs['Color'])
                val = nt.nodes.new('ShaderNodeMath'); val.operation = 'MULTIPLY_ADD'
                val.inputs[1].default_value = 3.0
                nt.links.new(lum.outputs['Red'], val.inputs[0])
                nt.links.new(val.outputs['Value'], mix1.inputs['Fac'])
                nt.links.new(mix0.outputs['Shader'], mix1.inputs[1])
                nt.links.new(neon.outputs['Emission'], mix1.inputs[2])
                nt.links.new(mix1.outputs['Shader'], out.inputs['Surface'])
            else:
                nt.links.new(mix0.outputs['Shader'], out.inputs['Surface'])
            result = (mat, pref)
            break
    cache[key] = result
    return result


def tex_prefixes(base_name):
    if 'Cockpit' in base_name:
        return ['Int_Cockpit', 'Int_Wall', 'Int_Glow']
    if 'Alien' in base_name or 'Xenomorph' in base_name:
        return ['Alien_Wall']
    return ['Int_Wall', 'Int_Glow']


def find_group_fbx(base_name):
    """Return every FBX on disk that belongs to this interior. A split-zone
    interior has files named '<base>_<Zone>.fbx' (Shell/Bunks/Desks/...); a
    single-mesh interior has just '<base>.fbx'."""
    exact = os.path.join(GEN, base_name + ".fbx")
    zone_pattern = os.path.join(GEN, base_name + "_*.fbx")
    zone_files = sorted(glob.glob(zone_pattern))
    files = []
    if os.path.exists(exact):
        files.append(exact)
    files += [f for f in zone_files if f not in files]
    return files


def has_real_material(obj):
    """True if this mesh already carries a non-default material with a set
    Base Color (i.e. was baked with solid_mat() in the generator) — those
    zone materials must be preserved, not overwritten."""
    if not obj.data.materials:
        return False
    mat = obj.data.materials[0]
    if mat is None or not mat.use_nodes:
        return False
    bsdf = next((n for n in mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED'), None)
    if bsdf is None:
        return False
    # A material built by solid_mat() has ONLY a BSDF + output (no image
    # texture nodes) — that's the signal it's an intentional per-zone color,
    # not the leftover placeholder Blender FBX import assigns.
    has_image_tex = any(n.type == 'TEX_IMAGE' for n in mat.node_tree.nodes)
    return not has_image_tex and mat.name != "" and mat.name.lower() != "material"


def selflight_solid_material(mat):
    """A solid_mat() material only sets Base Color on a plain Principled BSDF.
    Headless EEVEE in this pipeline has no real light sources (see project
    convention: materials must self-light via emission), so a bare-BSDF mesh
    renders almost black — only the dim world-ambient tint bleeds through,
    which is why every zone read as the same flat blue-gray to vision review.
    Mix in an Emission node driven by the material's OWN base color so each
    zone actually displays its assigned color instead of just tinting black."""
    if mat.get("_selflit"):
        return
    nt = mat.node_tree
    bsdf = next((n for n in nt.nodes if n.type == 'BSDF_PRINCIPLED'), None)
    out = next((n for n in nt.nodes if n.type == 'OUTPUT_MATERIAL'), None)
    if bsdf is None or out is None:
        return
    base_color = bsdf.inputs["Base Color"].default_value
    emit = nt.nodes.new('ShaderNodeEmission'); emit.location = (bsdf.location.x, bsdf.location.y + 250)
    emit.inputs['Color'].default_value = base_color
    emit.inputs['Strength'].default_value = 0.9
    mix = nt.nodes.new('ShaderNodeMixShader'); mix.location = (bsdf.location.x + 300, bsdf.location.y + 100)
    mix.inputs['Fac'].default_value = 0.55
    nt.links.new(bsdf.outputs['BSDF'], mix.inputs[1])
    nt.links.new(emit.outputs['Emission'], mix.inputs[2])
    nt.links.new(mix.outputs['Shader'], out.inputs['Surface'])
    mat["_selflit"] = True


def render_group(base_name):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    files = find_group_fbx(base_name)
    if not files:
        print("NO FBX FOUND FOR", base_name)
        return

    tex_cache = {}
    all_objs = []
    for fp in files:
        before = set(bpy.data.objects.keys())
        bpy.ops.import_scene.fbx(filepath=fp)
        new_objs = [o for o in bpy.data.objects if o.name not in before and o.type == 'MESH']
        for o in new_objs:
            # Preserve a real per-zone solid-color material (Hab/Bridge zones);
            # only fall back to the shared textured wall material when the
            # mesh has no intentional material of its own (single-mesh
            # interiors: cockpit, empty room, corridor, airlock, engineering,
            # alien hold all currently fall in this bucket).
            if has_real_material(o):
                selflight_solid_material(o.data.materials[0])
            else:
                mat, _ = make_textured_material(tex_prefixes(base_name), tex_cache)
                o.data.materials.clear()
                o.data.materials.append(mat)
        all_objs += new_objs

    if not all_objs:
        print("NO MESH", base_name)
        return

    sc = bpy.context.scene

    # Percentile-based bounds: raw min/max Z is dragged up by tall thin
    # greeble (canopy ring, ceiling light strips, antenna) far above where a
    # standing/seated occupant's eyes actually sit. Collect all Z values and
    # use a robust low/high percentile instead of the true extremes.
    all_z = []
    vmin = Vector((1e9,) * 3); vmax = Vector((-1e9,) * 3)
    for o in all_objs:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            all_z.append(ww[2])
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    all_z.sort()
    n = len(all_z)
    floor_z = all_z[int(n * 0.03)]      # robust floor (ignore any stray low outliers)
    ceil_z = all_z[int(n * 0.90)]       # robust ceiling (EXCLUDES canopy rings / light
                                         # strips / antenna mounted above the walkable room)
    room_h = max(ceil_z - floor_z, 50.0)
    center = (vmin + vmax) * 0.5
    Lx = vmax[0] - vmin[0]; Ly = vmax[1] - vmin[1]

    # Eye height: seated/standing occupant height fraction of the ROOM (not
    # the whole mesh incl. greeble) above the floor.
    eye_h = floor_z + room_h * 0.35

    along_x = Lx >= Ly

    def build_bvh(obj):
        """Build a world-space BVH tree from an object's evaluated mesh
        (joined interiors are made of primitive boxes, so triangulating them
        gives a clean closed-enough surface for ray-parity solid testing)."""
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        bmesh.ops.triangulate(bm, faces=bm.faces)
        wm = obj.matrix_world
        verts = [wm @ v.co for v in bm.verts]
        tris = [[v.index for v in f.verts] for f in bm.faces]
        bm.free()
        if not tris:
            return None
        return BVHTree.FromPolygons(verts, tris)

    bvh_trees = [t for t in (build_bvh(o) for o in all_objs) if t is not None]

    def point_inside_any_mesh(pt, direction=Vector((1, 0, 0.0001)), max_dist=1e5):
        """Guard against placing the camera INSIDE a solid object (seat,
        console, bunk, wall...) — that renders as a flat single-color fill
        with no visible geometry, reading as a broken/empty render. Uses a
        ray-parity test (odd number of surface crossings along a fixed
        direction = inside a solid region of THAT mesh) instead of a bounding-
        box test, because after finalize_part an entire interior (walls +
        furniture) is one joined mesh whose AABB spans the whole room —
        bounding-box collision can't tell 'inside the room' from 'inside the
        object'."""
        d = direction.normalized()
        for tree in bvh_trees:
            hits = 0
            origin = pt.copy()
            for _ in range(64):
                loc, _nrm, _idx, dist = tree.ray_cast(origin, d, max_dist)
                if loc is None:
                    break
                hits += 1
                origin = loc + d * 1e-3
            if hits % 2 == 1:
                return True
        return False

    # Search along the room's long axis for a camera position that is clear
    # of solid geometry, starting close to one wall and stepping inward.
    # Try both directions (near either end), AND several lateral offsets off
    # the centerline (furniture like a pilot seat or command console often
    # sits exactly on-axis, so centerline-only search finds nothing).
    camloc = None; look = None
    lateral_span = (Ly if along_x else Lx)
    lateral_offsets = [0.0, 0.28, -0.28, 0.45, -0.45]
    for lat_frac in lateral_offsets:
        lat_off = lateral_span * lat_frac
        for start_frac, end_frac, step in [(0.10, 0.85, 0.05), (0.88, 0.10, -0.05)]:
            frac = start_frac
            while (step > 0 and frac <= end_frac) or (step < 0 and frac >= end_frac):
                if along_x:
                    trial = Vector((vmin[0] + Lx * frac, center[1] + lat_off, eye_h))
                else:
                    trial = Vector((center[0] + lat_off, vmin[1] + Ly * frac, eye_h))
                if not point_inside_any_mesh(trial):
                    camloc = trial
                    look_frac = min(frac + step * 3, 0.95) if step > 0 else max(frac + step * 3, 0.05)
                    if along_x:
                        look = Vector((vmin[0] + Lx * look_frac, center[1], eye_h + room_h * 0.05))
                    else:
                        look = Vector((center[0], vmin[1] + Ly * look_frac, eye_h + room_h * 0.05))
                    break
                frac += step
            if camloc is not None:
                break
        if camloc is not None:
            break

    if camloc is None:
        # Fallback: room center, raised a little — better than a hard failure.
        camloc = Vector((center[0], center[1], eye_h))
        look = Vector((center[0], center[1] + (Ly if not along_x else Lx) * 0.3, eye_h))
        print("  [warn] no clear camera spot found for this room; using center fallback")

    bpy.ops.object.camera_add(location=camloc)
    cam = bpy.context.active_object; sc.camera = cam; cam.data.lens = 24
    d = (look - camloc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()

    w = sc.world if sc.world else bpy.data.worlds.new("SG")
    sc.world = w; w.use_nodes = True
    try:
        w.node_tree.nodes['Background'].inputs[0].default_value = (0.03, 0.04, 0.06, 1.0)
    except Exception:
        pass

    sc.render.engine = 'BLENDER_EEVEE'
    sc.render.resolution_x = 1400; sc.render.resolution_y = 960
    sc.render.resolution_percentage = 100
    sc.render.image_settings.file_format = 'PNG'
    # Standard view transform + raised exposure: Blender 5.2 defaults to AgX,
    # which crushes shadows/mid-darks hard at these self-lit emission
    # strengths — a technically-correct render came back "almost black" to
    # vision review. Standard + exposure boost makes the self-lit materials
    # actually legible without changing the underlying scene.
    sc.view_settings.view_transform = 'Standard'
    sc.view_settings.exposure = 0.6
    out_png = os.path.join(OUT, base_name + "_inside.png")
    sc.render.filepath = out_png
    bpy.ops.render.render(write_still=True)
    print("OK" if os.path.exists(out_png) else "FAIL", base_name,
          f"(zones={len(files)}, floor_z={floor_z:.0f}, ceil_z={ceil_z:.0f}, eye_h={eye_h:.0f})")


for t in sys.argv[sys.argv.index("--") + 1:]:
    base = t.replace('.fbx', '')
    try:
        render_group(base)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR", base)
