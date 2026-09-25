"""Capital-ship bridge interiors: Battleship + CommandXL (Blender 5.x, headless).

    blender -b --python Tools/build_capital_interiors.py -- [--dry-run] [battleship] [commandxl]

Builds two walkable interior families in the same split-zone contract as the
existing CommandBridge / CorvetteBridge kits (Tools/generate_adastrea_assets.py
build_bridge_interior / build_corvette_bridge_interior):

    SM_Int_Battleship_Bridge_{Shell,Deck,Console,Stations,Viewport,Lights,Hatch}.fbx
    SM_Int_CommandXL_Bridge_{Shell,Deck,Console,Stations,Viewport,Lights,Hatch}.fbx

The part suffixes are exactly the CommandBridge/CorvetteBridge TryPart() list
in ASpaceshipInterior::MountInteriorParts, so the C++ side only needs a new
EShipInteriorFamily value per ship that routes to that same branch.

CONTRACT (read from Source/Adastrea/Private/Ships/SpaceshipInterior.cpp and
checked against the live UE log values for the corvette and fighter shells):
  * ConfigureInterior scales the whole kit uniformly by
    650 / Shell.Bounds.SphereRadius. UE's SphereRadius is the max vertex
    distance from the bbox centre, NOT the box half-diagonal, so chamfered
    room corners buy real floor area. Only the Shell sets the scale; parts
    get the same scale and the same origin.
  * FitVolumeToMesh puts the Pawn-blocking floor slab top at bounds-min Z,
    and four Pawn-blocking walls just outside the bounds X/Y box. Shell and
    parts have NO Pawn collision, so all walkable floor is the single plane
    at min Z: no sunken pits, only token (<=8 cm) raised plates.
  * Exit (re-board) trigger at local (+0.6*HalfDepth, 0, 0.6*HalfHeight);
    entry spawn at (-0.6*HalfDepth, 0, FloorZ + 100). Axes: FBX export with
    forward=-Y / up=Z maps Blender +X to UE +X (and Blender Y to UE -Y). So
    the helm seat sits at Blender +X and the rear hatch and entry at -X.
  * Every vertex of every part stays inside the chamfered room outline, so
    nothing pokes through the walls and nothing inflates the sphere radius.

Design units are centimetres at (almost) final scale: each room is sized so
that the shell's sphere radius lands close to 650, which makes the runtime
scale about 1.0. Human reference: avatar ~192 cm tall; desk 75 cm; seat 46 cm.

Material slots are M_Int_* names, which is the key ApplyKitMaterialsBySlot maps
to /AdastreaShips/Materials/Interiors/*. Multi-slot parts are used where it
reads better: screen faces use M_Int_Console (M_Int_Lights is a flat x8 emissive
meant for small fixtures, so large screens on it blow out to white), light strips
and holograms use M_Int_Lights, and window frames use M_Int_Shell so
that only the panes get the (translucent) M_Int_Viewport.
"""
import bpy, bmesh, math, os, sys, json
from mathutils import Vector, Matrix

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TARGET_RADIUS = 650.0
TRIGGER_EXTENT_UU = (100.0, 150.0, 200.0)   # ASpaceshipInterior::ExitTriggerSize (box extent)
PART_ORDER = ["Shell", "Deck", "Console", "Stations", "Viewport", "Lights", "Hatch"]

# Preview colours only (the FBX carries slot NAMES; UE maps them). Same values
# as Plugins/AdastreaShips/import_interiors.py INT_MATERIALS.
SLOT_COLORS = {
    "M_Int_Shell": (0.42, 0.44, 0.47), "M_Int_Lights": (0.50, 0.75, 1.00),
    "M_Int_Bunks": (0.80, 0.66, 0.45), "M_Int_Vents": (0.70, 0.25, 0.20),
    "M_Int_Hatch": (0.18, 0.18, 0.20), "M_Int_Deck": (0.45, 0.45, 0.48),
    "M_Int_Console": (0.10, 0.32, 0.36), "M_Int_Stations": (0.22, 0.55, 0.55),
    "M_Int_Viewport": (0.08, 0.10, 0.14),
    # CommandXL hero pass only: command furniture bodies and the gold wall trim.
    # Both are re-slotted onto the unique baked atlas by
    # Tools/bake_interior_unique.py, so UE never sees these names.
    "M_Int_Command": (0.07, 0.10, 0.20), "M_Int_Trim": (0.55, 0.45, 0.15),
}

# Slots that get real bevels in the hero pass (emissive strips and glass stay
# crisp: a bevel there costs tris and is invisible under the bloom).
BEVEL_SKIP_SLOTS = {"M_Int_Lights", "M_Int_Viewport", "M_Int_Console"}


# ----------------------------------------------------------------------------
# Plan-polygon helpers (the room outline is a convex chamfered rectangle)
# ----------------------------------------------------------------------------
def room_outline(xh, yh, cf, cr):
    """CCW convex octagon: front (+X) corners chamfered by cf, rear by cr."""
    return [(xh, -yh + cf), (xh, yh - cf), (xh - cf, yh), (-xh + cr, yh),
            (-xh, yh - cr), (-xh, -yh + cr), (-xh + cr, -yh), (xh - cf, -yh)]


def offset_convex(poly, d):
    """Inset a CCW convex polygon by d (mitred)."""
    n = len(poly)
    lines = []
    for i in range(n):
        p0 = Vector(poly[i]); p1 = Vector(poly[(i + 1) % n])
        e = (p1 - p0).normalized()
        nrm = Vector((-e.y, e.x))          # inward normal for CCW
        lines.append((p0 + nrm * d, e))
    out = []
    for i in range(n):
        (a, ea), (b, eb) = lines[i - 1], lines[i]
        # intersect a + s*ea = b + t*eb
        den = ea.x * eb.y - ea.y * eb.x
        s = ((b.x - a.x) * eb.y - (b.y - a.y) * eb.x) / den
        out.append(tuple(a + ea * s))
    return out


def clip_convex(subject, clip):
    """Sutherland-Hodgman: clip polygon `subject` by CCW convex polygon `clip`."""
    def inside(p, a, b):
        return (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0]) >= -1e-6

    def inter(p, q, a, b):
        x1, y1 = p; x2, y2 = q; x3, y3 = a; x4, y4 = b
        den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
        t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den
        return (x1 + t * (x2 - x1), y1 + t * (y2 - y1))

    out = list(subject)
    for i in range(len(clip)):
        a, b = clip[i], clip[(i + 1) % len(clip)]
        inp, out = out, []
        if not inp:
            break
        s = inp[-1]
        for e in inp:
            if inside(e, a, b):
                if not inside(s, a, b):
                    out.append(inter(s, e, a, b))
                out.append(e)
            elif inside(s, a, b):
                out.append(inter(s, e, a, b))
            s = e
    return out


def rect(x0, x1, y0, y1):
    return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]


def circle(cx, cy, r, segs=24, a0=0.0, a1=360.0):
    full = abs(a1 - a0) >= 359.999
    n = segs if full else segs + 1
    pts = []
    for i in range(n):
        a = math.radians(a0 + (a1 - a0) * i / segs)
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


# ----------------------------------------------------------------------------
# Part: one exported mesh (one FBX), multiple named material slots
# ----------------------------------------------------------------------------
class Part:
    def __init__(self, name):
        self.name = name
        self.slots = []
        self.bm = bmesh.new()

    def mi(self, slot):
        if slot not in self.slots:
            self.slots.append(slot)
        return self.slots.index(slot)

    def _faces(self, verts, faces, slot):
        vs = [self.bm.verts.new(v) for v in verts]
        m = self.mi(slot)
        for f in faces:
            face = self.bm.faces.new([vs[i] for i in f])
            face.material_index = m

    # -- primitives ---------------------------------------------------------
    def prism_xy(self, slot, poly, z0, z1, M=None):
        """Vertical extrusion of a plan polygon (any simple polygon)."""
        if len(poly) < 3 or z1 - z0 <= 1e-4:
            return
        M = M or Matrix()
        n = len(poly)
        verts = [M @ Vector((x, y, z0)) for x, y in poly] + [M @ Vector((x, y, z1)) for x, y in poly]
        faces = [list(range(n))[::-1], list(range(n, 2 * n))]
        for i in range(n):
            j = (i + 1) % n
            faces.append([i, j, n + j, n + i])
        self._faces(verts, faces, slot)

    def prism_xz(self, slot, prof, y0, y1, M=None):
        """Side-profile (x,z) polygon extruded across local Y."""
        M = M or Matrix()
        n = len(prof)
        verts = [M @ Vector((x, y0, z)) for x, z in prof] + [M @ Vector((x, y1, z)) for x, z in prof]
        faces = [list(range(n)), list(range(n, 2 * n))[::-1]]
        for i in range(n):
            j = (i + 1) % n
            faces.append([i, n + i, n + j, j])
        self._faces(verts, faces, slot)

    def box(self, slot, x0, x1, y0, y1, z0, z1, M=None):
        self.prism_xy(slot, rect(x0, x1, y0, y1), z0, z1, M)

    def boxc(self, slot, c, s, yaw=0.0):
        M = Matrix.Translation(Vector(c)) @ Matrix.Rotation(math.radians(yaw), 4, 'Z')
        self.box(slot, -s[0] / 2, s[0] / 2, -s[1] / 2, s[1] / 2, -s[2] / 2, s[2] / 2, M)

    def cyl(self, slot, cx, cy, z0, z1, r, segs=24):
        self.prism_xy(slot, circle(cx, cy, r, segs), z0, z1)

    def cyl_between(self, slot, p0, p1, r, segs=12):
        p0 = Vector(p0); p1 = Vector(p1)
        d = p1 - p0
        rot = d.to_track_quat('Z', 'Y').to_matrix().to_4x4()
        M = Matrix.Translation(p0) @ rot
        self.prism_xy(slot, circle(0, 0, r, segs), 0.0, d.length, M)

    def annulus(self, slot, cx, cy, r0, r1, z0, z1, segs=32, a0=0.0, a1=360.0):
        """Solid ring (or ring sector) between radii r0<r1."""
        full = abs(a1 - a0) >= 359.999
        outer = circle(cx, cy, r1, segs, a0, a1)
        inner = circle(cx, cy, r0, segs, a0, a1)
        n = len(outer)
        verts = ([Vector((x, y, z0)) for x, y in outer] + [Vector((x, y, z0)) for x, y in inner] +
                 [Vector((x, y, z1)) for x, y in outer] + [Vector((x, y, z1)) for x, y in inner])
        O0, I0, O1, I1 = 0, n, 2 * n, 3 * n
        faces = []
        rng = range(n) if full else range(n - 1)
        for i in rng:
            j = (i + 1) % n
            faces.append([O0 + i, O0 + j, O1 + j, O1 + i])     # outer wall
            faces.append([I0 + j, I0 + i, I1 + i, I1 + j])     # inner wall
            faces.append([O1 + i, O1 + j, I1 + j, I1 + i])     # top
            faces.append([O0 + j, O0 + i, I0 + i, I0 + j])     # bottom
        if not full:
            faces.append([O0, O1, I1, I0])
            e = n - 1
            faces.append([O0 + e, I0 + e, I1 + e, O1 + e])
        self._faces(verts, faces, slot)

    def torus(self, slot, c, R, r, segs=32, tsegs=8, tilt=(0.0, 0.0)):
        M = (Matrix.Translation(Vector(c)) @ Matrix.Rotation(math.radians(tilt[1]), 4, 'Y')
             @ Matrix.Rotation(math.radians(tilt[0]), 4, 'X'))
        verts, faces = [], []
        for i in range(segs):
            a = 2 * math.pi * i / segs
            for j in range(tsegs):
                b = 2 * math.pi * j / tsegs
                rr = R + r * math.cos(b)
                verts.append(M @ Vector((rr * math.cos(a), rr * math.sin(a), r * math.sin(b))))
        for i in range(segs):
            for j in range(tsegs):
                i2 = (i + 1) % segs; j2 = (j + 1) % tsegs
                faces.append([i * tsegs + j, i2 * tsegs + j, i2 * tsegs + j2, i * tsegs + j2])
        self._faces(verts, faces, slot)

    def ico(self, slot, c, r, subdiv=1):
        tmp = bmesh.new()
        bmesh.ops.create_icosphere(tmp, subdivisions=subdiv, radius=r)
        verts = [Vector(c) + v.co for v in tmp.verts]
        faces = [[v.index for v in f.verts] for f in tmp.faces]
        tmp.free()
        self._faces(verts, faces, slot)

    def slope_panel(self, slot, a, b, y0, y1, lift=0.6, thick=0.8, inset=0.08, M=None):
        """Thin panel lying on the profile segment a->b (x,z), raised by `lift`
        along the segment's outward normal (the +z-ish side), trimmed by `inset`."""
        a = Vector(a); b = Vector(b)
        d = b - a
        a2 = a + d * inset; b2 = b - d * inset
        nrm = Vector((-d.y, d.x)).normalized()
        if nrm.y < 0:
            nrm = -nrm
        p = [a2 + nrm * lift, b2 + nrm * lift, b2 + nrm * (lift + thick), a2 + nrm * (lift + thick)]
        w = (y1 - y0)
        self.prism_xz(slot, [tuple(q) for q in p], y0 + w * inset * 0.5, y1 - w * inset * 0.5, M)

    # -- finalize -----------------------------------------------------------
    def grid_cut(self, pitch=60.0, min_area=2500.0):
        """Hero pass: split big flat faces on a world grid so the unique bake
        can drop the parts that are covered (deck under consoles, wall behind
        displays) instead of giving a whole slab texels. Coplanar cuts, so the
        silhouette and the shading are unchanged."""
        bm = self.bm
        for axis in range(3):
            no = Vector([1.0 if i == axis else 0.0 for i in range(3)])
            big = [f for f in bm.faces if f.calc_area() > min_area]
            if not big:
                continue
            lo = min(v.co[axis] for f in big for v in f.verts)
            hi = max(v.co[axis] for f in big for v in f.verts)
            k = math.floor(lo / pitch) + 1
            while k * pitch < hi:
                c = k * pitch
                k += 1
                faces = [f for f in bm.faces if f.is_valid and f.calc_area() > min_area
                         and min(v.co[axis] for v in f.verts) < c - 0.5
                         and max(v.co[axis] for v in f.verts) > c + 0.5]
                if not faces:
                    continue
                edges = {e for f in faces for e in f.edges}
                verts = {v for f in faces for v in f.verts}
                bmesh.ops.bisect_plane(bm, geom=list(faces) + list(edges) + list(verts),
                                       plane_co=no * c, plane_no=no)

    def bevel_hard_edges(self, offset=1.0, segments=2, min_angle=30.0):
        """Hero pass: chamfer every hard edge the way the exterior hulls are
        bevelled (gen.bevel), so edges catch light and the baked edge-wear has
        real geometry to sit on. Edges touching emissive/glass/screen faces are
        left sharp."""
        bm = self.bm
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        skip = {self.slots.index(s) for s in BEVEL_SKIP_SLOTS if s in self.slots}
        edges = []
        for e in bm.edges:
            if len(e.link_faces) != 2:
                continue
            f0, f1 = e.link_faces
            if f0.material_index in skip or f1.material_index in skip:
                continue
            if math.degrees(f0.normal.angle(f1.normal, 0.0)) >= min_angle:
                edges.append(e)
        if edges:
            bmesh.ops.bevel(bm, geom=edges, offset=offset, offset_type='OFFSET',
                            profile_type='SUPERELLIPSE', segments=segments, profile=0.5,
                            affect='EDGES', clamp_overlap=True)
        return len(edges)

    def to_object(self):
        bm = self.bm
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        bmesh.ops.triangulate(bm, faces=bm.faces, quad_method='BEAUTY', ngon_method='BEAUTY')
        # World-aligned triplanar UV, 200 cm tile, same projection as the
        # generator's smart_uv() so kit textures tile at the same density.
        uv = bm.loops.layers.uv.new("UVMap")
        tile = 200.0
        for f in bm.faces:
            n = f.normal
            ax = max(range(3), key=lambda i: abs(n[i]))
            for loop in f.loops:
                v = loop.vert.co
                if ax == 0:
                    loop[uv].uv = (v.y / tile, v.z / tile)
                elif ax == 1:
                    loop[uv].uv = (v.z / tile, v.x / tile)
                else:
                    loop[uv].uv = (v.x / tile, v.y / tile)
        me = bpy.data.meshes.new(self.name)
        bm.to_mesh(me)
        bm.free()
        for s in self.slots:
            mat = bpy.data.materials.get(s) or bpy.data.materials.new(s)
            me.materials.append(mat)
        ob = bpy.data.objects.new(self.name, me)
        bpy.context.scene.collection.objects.link(ob)
        return ob


# ----------------------------------------------------------------------------
# Room scaffolding shared by both layouts
# ----------------------------------------------------------------------------
class Room:
    def __init__(self, prefix, xh, yh, h, cf, cr, wall_t=8.0, ceil_t=6.0):
        self.prefix = prefix
        self.xh, self.yh, self.h = xh, yh, h
        self.outline = room_outline(xh, yh, cf, cr)
        self.inner = offset_convex(self.outline, wall_t)      # inside face of walls
        self.clip_poly = offset_convex(self.outline, wall_t + 0.5)
        self.wall_t, self.ceil_t = wall_t, ceil_t
        self.ceil_z = h - ceil_t
        self.parts = {p: Part(f"{prefix}_{p}") for p in PART_ORDER}

    def P(self, name):
        return self.parts[name]

    def clip(self, poly):
        return clip_convex(poly, self.clip_poly)

    def cbox(self, part, slot, x0, x1, y0, y1, z0, z1):
        """Box clipped to the room interior (safe near chamfers)."""
        poly = self.clip(rect(x0, x1, y0, y1))
        if len(poly) >= 3:
            self.P(part).prism_xy(slot, poly, z0, z1)

    # edge helpers: point on outer / inner line of wall edge i at distance t
    def edge(self, i):
        n = len(self.outline)
        o0, o1 = Vector(self.outline[i]), Vector(self.outline[(i + 1) % n])
        i0, i1 = Vector(self.inner[i]), Vector(self.inner[(i + 1) % n])
        return o0, o1, i0, i1

    def edge_len(self, i):
        o0, o1, _, _ = self.edge(i)
        return (o1 - o0).length

    def edge_quad(self, i, t0, t1, depth0=0.0, depth1=None):
        """Plan quad of wall edge i between along-distances t0..t1, spanning from
        depth0 to depth1 measured inward from the outer face (default full wall)."""
        o0, o1, i0, i1 = self.edge(i)
        L = (o1 - o0).length
        f0, f1 = t0 / L, t1 / L
        depth1 = self.wall_t if depth1 is None else depth1
        oa, ob = o0.lerp(o1, f0), o0.lerp(o1, f1)
        ia, ib = i0.lerp(i1, f0), i0.lerp(i1, f1)
        k0, k1 = depth0 / self.wall_t, depth1 / self.wall_t
        return [tuple(oa.lerp(ia, k0)), tuple(ob.lerp(ib, k0)), tuple(ob.lerp(ib, k1)), tuple(oa.lerp(ia, k1))]

    def edge_frame(self, i, t):
        """(point on inner wall face at t, inward normal, along-direction)."""
        o0, o1, i0, i1 = self.edge(i)
        L = (o1 - o0).length
        e = (o1 - o0).normalized()
        nrm = Vector((-e.y, e.x))
        p = i0.lerp(i1, t / L)
        return p, nrm, e

    def walls(self, openings):
        """Perimeter walls with rectangular openings: openings[i] = [(t0,t1,z0,z1),...]."""
        sh = self.P("Shell")
        z_top = self.ceil_z
        for i in range(len(self.outline)):
            L = self.edge_len(i)
            ops = sorted(openings.get(i, []))
            # vertical strips between openings (full height)
            cursor = 0.0
            for (t0, t1, z0, z1) in ops:
                if t0 > cursor:
                    sh.prism_xy("M_Int_Shell", self.edge_quad(i, cursor, t0), 0.0, z_top)
                # below and above the opening
                if z0 > 0.0:
                    sh.prism_xy("M_Int_Shell", self.edge_quad(i, t0, t1), 0.0, z0)
                if z1 < z_top:
                    sh.prism_xy("M_Int_Shell", self.edge_quad(i, t0, t1), z1, z_top)
                cursor = t1
            if cursor < L:
                sh.prism_xy("M_Int_Shell", self.edge_quad(i, cursor, L), 0.0, z_top)

    def floor_ceiling(self):
        sh = self.P("Shell")
        sh.prism_xy("M_Int_Deck", self.outline, -2.0, 0.0)          # bounds-min Z = -2
        sh.prism_xy("M_Int_Shell", self.outline, self.ceil_z, self.h)  # bounds-max Z = h

    def window(self, i, t0, t1, z0, z1, mullion_step=90.0, frame_slot="M_Int_Shell"):
        """Glass pane + frame for an opening on wall edge i (Viewport part)."""
        vp = self.P("Viewport")
        mid0, mid1 = self.wall_t * 0.45, self.wall_t * 0.55
        vp.prism_xy("M_Int_Viewport", self.edge_quad(i, t0, t1, mid0, mid1), z0, z1)
        # frame: sill, header, jambs (sit in the opening, flush with inner face +3 cm)
        fd0, fd1 = self.wall_t * 0.55, self.wall_t + 3.0
        vp.prism_xy(frame_slot, self.edge_quad(i, t0, t1, fd0 - 4.0, fd1), z0, z0 + 6.0)
        vp.prism_xy(frame_slot, self.edge_quad(i, t0, t1, fd0 - 4.0, fd1), z1 - 6.0, z1)
        vp.prism_xy(frame_slot, self.edge_quad(i, t0, t0 + 6.0, fd0 - 4.0, fd1), z0, z1)
        vp.prism_xy(frame_slot, self.edge_quad(i, t1 - 6.0, t1, fd0 - 4.0, fd1), z0, z1)
        span = t1 - t0
        n = max(0, int(round(span / mullion_step)) - 1)
        for k in range(1, n + 1):
            tm = t0 + span * k / (n + 1)
            vp.prism_xy(frame_slot, self.edge_quad(i, tm - 3.0, tm + 3.0, fd0 - 4.0, fd1), z0, z1)
        # horizontal transom on tall windows
        if z1 - z0 > 180:
            zt = z0 + (z1 - z0) * 0.68
            vp.prism_xy(frame_slot, self.edge_quad(i, t0, t1, fd0 - 4.0, fd1 - 1.0), zt - 2.5, zt + 2.5)

    def hatch(self, i, t0, t1, z1):
        """Door in a wall opening on edge i: split leaves, frame, status bar."""
        hp = self.P("Hatch")
        tm = (t0 + t1) / 2
        # two leaves recessed in the wall plane, 1.5 cm centre seam
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, t0, tm - 0.75, 1.0, 5.0), 0.0, z1)
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, tm + 0.75, t1, 1.0, 5.0), 0.0, z1)
        # heavy frame proud of the wall (inside the room)
        fw = 14.0
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, t0 - fw, t0, 0.0, self.wall_t + 7.0), 0.0, z1 + fw)
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, t1, t1 + fw, 0.0, self.wall_t + 7.0), 0.0, z1 + fw)
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, t0 - fw, t1 + fw, 0.0, self.wall_t + 7.0), z1, z1 + fw)
        # hazard-ish threshold plate
        p, nrm, e = self.edge_frame(i, tm)
        th = [tuple(p + e * (t0 - tm) + nrm * 0.0), tuple(p + e * (t1 - tm)),
              tuple(p + e * (t1 - tm) + nrm * 40.0), tuple(p + e * (t0 - tm) + nrm * 40.0)]
        hp.prism_xy("M_Int_Hatch", th, 0.0, 1.2)
        # status light bar above the door + wall control panel beside it
        hp.prism_xy("M_Int_Lights", self.edge_quad(i, t0 + 10, t1 - 10, self.wall_t, self.wall_t + 8.5),
                    z1 + fw + 4.0, z1 + fw + 9.0)
        hp.prism_xy("M_Int_Hatch", self.edge_quad(i, t1 + fw + 10, t1 + fw + 40, self.wall_t, self.wall_t + 5.0),
                    105.0, 150.0)
        hp.prism_xy("M_Int_Lights", self.edge_quad(i, t1 + fw + 13, t1 + fw + 37, self.wall_t + 5.0, self.wall_t + 5.8),
                    110.0, 145.0)


# ----------------------------------------------------------------------------
# Furniture
# ----------------------------------------------------------------------------
def Mxy(x, y, yaw):
    return Matrix.Translation(Vector((x, y, 0))) @ Matrix.Rotation(math.radians(yaw), 4, 'Z')


def console(part, x, y, yaw, width=110.0, body="M_Int_Stations", screen="M_Int_Console",
            depth=55.0, desk_h=76.0, back_h=112.0, monitor=True, detail=False):
    """Sit-down console. Operator sits at local -X, faces local +X. (x,y) is the
    operator edge centre; the console extends forward (+X local) by `depth`.
    detail=True (hero pass) adds cheek plates, a toe-kick vent, a keyboard deck
    and a monitor yoke, i.e. the greeble density of the exterior hulls."""
    M = Mxy(x, y, yaw)
    w0, w1 = -width / 2, width / 2
    prof = [(8, 0), (depth, 0), (depth, back_h), (depth - 12, back_h + 4), (-6, desk_h + 3),
            (-6, desk_h - 3), (8, desk_h - 6)]
    part.prism_xz(body, prof, w0, w1, M)
    part.slope_panel(screen, (-6, desk_h + 3), (depth - 12, back_h + 4), w0, w1, M=M)
    if detail:
        # cheek plates: slightly larger profile, proud of both ends
        cheek = [(4, 0), (depth + 3, 0), (depth + 3, back_h + 3), (depth - 12, back_h + 8),
                 (-9, desk_h + 5), (-9, desk_h - 6), (4, desk_h - 10)]
        part.prism_xz(body, cheek, w0 - 3.0, w0, M)
        part.prism_xz(body, cheek, w1, w1 + 3.0, M)
        # toe-kick recess + vent slats on the operator face
        part.box(body, 8.0, 12.0, w0 + 6, w1 - 6, 0.0, 12.0, M)
        for k in range(4):
            z = desk_h - 36.0 + k * 6.0
            part.box(body, 6.5, 8.5, w0 + 14, w1 - 14, z, z + 2.5, M)
        # keyboard deck on the desk lip (dark body) with key rows (screen)
        a, b = Vector((-6, desk_h + 3)), Vector((depth - 12, back_h + 4))
        d = (b - a).normalized()
        n = Vector((-d.y, d.x))
        p0, p1 = a + d * 3.0, a + d * 17.0
        kb = [tuple(p0 + n * 0.6), tuple(p1 + n * 0.6), tuple(p1 + n * 3.0), tuple(p0 + n * 3.0)]
        kw = min(60.0, width * 0.5)
        part.prism_xz(body, kb, -kw / 2, kw / 2, M)
        for r in range(3):
            q0, q1 = a + d * (4.5 + r * 4.3), a + d * (7.8 + r * 4.3)
            key = [tuple(q0 + n * 3.0), tuple(q1 + n * 3.0), tuple(q1 + n * 3.6), tuple(q0 + n * 3.6)]
            part.prism_xz(screen, key, -kw / 2 + 3, kw / 2 - 3, M)
    if monitor:
        mh0, mh1 = back_h + 4, back_h + 52
        part.box(body, depth - 16, depth - 8, w0 + 8, w1 - 8, mh0 - 6, mh1, M)
        part.box(screen, depth - 16.8, depth - 16, w0 + 12, w1 - 12, mh0, mh1 - 4, M)
        if detail:   # yoke arm + rear heat-sink fins
            part.box(body, depth - 9, depth - 2, -6, 6, back_h - 10, mh0 + 18, M)
            for k in range(-2, 3):
                part.box(body, depth - 8, depth - 5, k * 12 - 1.5, k * 12 + 1.5, mh0 + 6, mh1 - 6, M)


def chair(part, x, y, yaw, slot="M_Int_Stations", big=False, arm_screens=None):
    """Swivel chair facing local +X (the console)."""
    M = Mxy(x, y, yaw)
    k = 1.25 if big else 1.0
    part.prism_xy(slot, circle(0, 0, 24 * k, 12), 0.0, 4.0, M)          # base disc
    part.prism_xy(slot, circle(0, 0, 5 * k, 8), 4.0, 38.0, M)           # column
    part.box(slot, -24 * k, 24 * k, -25 * k, 25 * k, 38.0, 48.0, M)     # seat pan
    back = [(-30 * k, 44), (-20 * k, 44), (-26 * k, 118 * k), (-36 * k, 116 * k)]
    part.prism_xz(slot, back, -22 * k, 22 * k, M)                       # backrest
    part.box(slot, -40 * k, -28 * k, -14 * k, 14 * k, 112 * k, 128 * k, M)  # headrest
    for s in (-1, 1):
        y0 = s * 25 * k
        y1 = s * 31 * k
        part.box(slot, -22 * k, 18 * k, min(y0, y1), max(y0, y1), 60.0, 66.0, M)  # armrest
        part.box(slot, -8 * k, -2 * k, min(y0, y1), max(y0, y1), 46.0, 60.0, M)
        if arm_screens:
            # armrest command pads (angled slabs + glowing faces)
            yy0, yy1 = s * 31 * k, s * 47 * k
            part.box(slot, -14 * k, 22 * k, min(yy0, yy1), max(yy0, yy1), 56.0, 70.0, M)
            part.box(arm_screens, -10 * k, 18 * k, min(yy0, yy1) + 2, max(yy0, yy1) - 2, 70.0, 70.8, M)


def holo_emitter(room, cx, cy, table_r, top_z, holo_r, holo_z, big=False, body="M_Int_Console"):
    """Round holo table (Console) + hologram (Lights)."""
    co, li = room.P("Console"), room.P("Lights")
    co.cyl(body, cx, cy, 0.0, 10.0, table_r * 0.55, 24)
    co.cyl(body, cx, cy, 10.0, top_z - 14, table_r * 0.40, 24)
    co.prism_xy(body, circle(cx, cy, table_r, 32), top_z - 14, top_z)
    li.annulus("M_Int_Lights", cx, cy, table_r * 0.15, table_r * 0.82, top_z, top_z + 0.8, 32)
    li.annulus("M_Int_Lights", cx, cy, table_r * 0.97, table_r * 1.01, top_z - 10, top_z - 4, 32)
    # hologram: gimbal rings + core + orbit
    hz = holo_z
    li.torus("M_Int_Lights", (cx, cy, hz), holo_r, 0.9, 40, 4)
    li.torus("M_Int_Lights", (cx, cy, hz), holo_r, 0.9, 40, 4, tilt=(90, 0))
    li.torus("M_Int_Lights", (cx, cy, hz), holo_r, 0.9, 40, 4, tilt=(90, 60))
    li.torus("M_Int_Lights", (cx, cy, hz), holo_r, 0.9, 40, 4, tilt=(90, 120))
    li.ico("M_Int_Lights", (cx, cy, hz), holo_r * 0.38, 2)
    li.torus("M_Int_Lights", (cx, cy, hz), holo_r * 1.55, 0.7, 48, 4, tilt=(18, 0))
    # projection column (thin bright cone of rings from table to hologram)
    for k in range(1, 4):
        zz = top_z + (hz - holo_r - top_z) * k / 4.0
        li.torus("M_Int_Lights", (cx, cy, zz), holo_r * (0.25 + 0.12 * k), 0.5, 24, 4)
    if big:
        for k in range(6):   # contact markers orbiting the fleet sphere
            a = 2 * math.pi * k / 6
            li.ico("M_Int_Lights", (cx + holo_r * 1.55 * math.cos(a),
                                    cy + holo_r * 1.55 * math.sin(a) * math.cos(math.radians(18)),
                                    hz + holo_r * 1.55 * math.sin(a) * math.sin(math.radians(18))), 5.0, 1)


def ceiling_strip(room, x0, x1, y0, y1, drop=4.0):
    room.cbox("Lights", "M_Int_Lights", x0, x1, y0, y1, room.ceil_z - drop, room.ceil_z)


def wall_display(room, i, t0, t1, z0, z1, frame=6.0, panels=1, gap=8.0):
    """Wall screen bank on edge i: `panels` screen faces (Console) set in one
    Shell bezel, separated by bezel mullions so it reads as a display wall,
    not a single flat emissive sheet."""
    wt = room.wall_t
    room.P("Shell").prism_xy("M_Int_Shell", room.edge_quad(i, t0 - frame, t1 + frame, wt, wt + 5.0),
                             z0 - frame, z1 + frame)
    w = (t1 - t0 - gap * (panels - 1)) / panels
    for k in range(panels):
        a = t0 + k * (w + gap)
        room.P("Lights").prism_xy("M_Int_Console", room.edge_quad(i, a, a + w, wt + 5.0, wt + 5.8), z0, z1)
        # a slim status strip under each panel
        room.P("Lights").prism_xy("M_Int_Lights", room.edge_quad(i, a + w * 0.1, a + w * 0.9, wt + 5.0, wt + 5.8),
                                  z0 - frame - 8.0, z0 - frame - 5.0)


def vent(room, i, t0, t1, z0, z1, slats=6):
    wt = room.wall_t
    sh = room.P("Shell")
    sh.prism_xy("M_Int_Shell", room.edge_quad(i, t0 - 3, t1 + 3, wt, wt + 2.0), z0 - 3, z1 + 3)
    h = (z1 - z0) / slats
    for k in range(slats):
        sh.prism_xy("M_Int_Vents", room.edge_quad(i, t0, t1, wt + 2.0, wt + 4.5),
                    z0 + k * h + h * 0.25, z0 + k * h + h * 0.75)


def pilaster(room, i, t, w=16.0, d=10.0, light=True):
    wt = room.wall_t
    room.P("Shell").prism_xy("M_Int_Shell", room.edge_quad(i, t - w / 2, t + w / 2, wt, wt + d), 0.0, room.ceil_z)
    if light:
        room.P("Lights").prism_xy("M_Int_Lights", room.edge_quad(i, t - 2, t + 2, wt + d, wt + d + 0.8),
                                  40.0, room.ceil_z - 40.0)


def baseboard(room, h=12.0, d=3.0, skip=None):
    """Kick plate along every wall (skipping openings that reach the floor)."""
    skip = skip or {}
    wt = room.wall_t
    for i in range(len(room.outline)):
        L = room.edge_len(i)
        cuts = sorted(skip.get(i, []))
        cur = 0.0
        for (a, b) in cuts:
            if a > cur:
                room.P("Shell").prism_xy("M_Int_Shell", room.edge_quad(i, cur, a, wt, wt + d), 0.0, h)
            cur = b
        if cur < L:
            room.P("Shell").prism_xy("M_Int_Shell", room.edge_quad(i, cur, L, wt, wt + d), 0.0, h)


# ----------------------------------------------------------------------------
# BATTLESHIP: long command deck = aft crew corridor -> CIC -> forward bridge
# ----------------------------------------------------------------------------
def build_battleship():
    XH, YH, H = 560.0, 310.0, 340.0
    r = Room("SM_Int_Battleship_Bridge", XH, YH, H, cf=60.0, cr=60.0)
    sh, dk, co, st, li = r.P("Shell"), r.P("Deck"), r.P("Console"), r.P("Stations"), r.P("Lights")
    wt = r.wall_t
    YI = YH - wt  # inner side-wall face |y|

    # Edge indices of the octagon: 0 front(+X), 1 front-left chamfer, 2 left(+Y)
    # side (runs +X -> -X), 3 rear-left chamfer, 4 rear(-X) (runs +Y -> -Y),
    # 5 rear-right chamfer, 6 right(-Y) side (runs -X -> +X), 7 front-right chamfer.
    L0, L2, L4 = r.edge_len(0), r.edge_len(2), r.edge_len(4)
    x_at_e2 = lambda x: (XH - 60.0) - x         # along-distance on edge 2 for world x
    x_at_e6 = lambda x: x + (XH - 60.0)         # along-distance on edge 6 for world x
    y_at_e4 = lambda y: (YH - 60.0) - y

    win_z = (92.0, 292.0)
    side_win = (150.0, 262.0)
    openings = {
        0: [(18.0, L0 - 18.0, *win_z)],
        1: [(10.0, r.edge_len(1) - 10.0, *win_z)],
        7: [(10.0, r.edge_len(7) - 10.0, *win_z)],
        2: [(x_at_e2(480.0), x_at_e2(215.0), *side_win)],
        6: [(x_at_e6(215.0), x_at_e6(480.0), *side_win)],
        4: [(y_at_e4(72.0), y_at_e4(-72.0), 0.0, 232.0)],
    }
    r.floor_ceiling()
    r.walls(openings)
    for i, ops in openings.items():
        for (t0, t1, z0, z1) in ops:
            if i == 4:
                r.hatch(4, t0, t1, z1)
            else:
                r.window(i, t0, t1, z0, z1, mullion_step=95.0 if i == 0 else 80.0)
    baseboard(r, skip={4: [(y_at_e4(72.0) - 14, y_at_e4(-72.0) + 14)]})

    # ---------------- AFT: crew corridor (x -552 .. -200) -------------------
    AFT0, AFT1 = -XH + wt, -200.0
    # lowered soffits either side of a central light trough
    r.cbox("Shell", "M_Int_Shell", AFT0, AFT1, -YI, -48.0, 262.0, r.ceil_z)
    r.cbox("Shell", "M_Int_Shell", AFT0, AFT1, 48.0, YI, 262.0, r.ceil_z)
    ceiling_strip(r, AFT0 + 20, AFT1 - 10, -14.0, 14.0, drop=5.0)
    for x in (-470.0, -380.0, -290.0):            # soffit edge lights
        r.cbox("Lights", "M_Int_Lights", x - 25, x + 25, -50.0, -47.0, 262.0, 266.0)
        r.cbox("Lights", "M_Int_Lights", x - 25, x + 25, 47.0, 50.0, 262.0, 266.0)
    # service pipes tucked under the soffits along both walls
    for s in (-1, 1):
        for k, (dy, rad) in enumerate(((22.0, 5.0), (36.0, 3.5))):
            y = s * (YI - dy)
            sh.cyl_between("M_Int_Shell", (AFT0 + 55, y, 250.0 - k * 4), (AFT1 - 5, y, 250.0 - k * 4), rad, 10)

    # Crew bunks, starboard (+Y) wall: one two-high stack, 2 m berths
    BX0, BX1, BD = -440.0, -222.0, 88.0
    by0 = YI - BD
    sh.box("M_Int_Bunks", BX0, BX1, by0, YI, 0.0, 38.0)                    # drawers/base
    sh.box("M_Int_Bunks", BX0 + 5, BX1 - 5, by0 + 3, YI, 38.0, 52.0)       # lower mattress
    sh.box("M_Int_Bunks", BX0, BX1, by0, YI, 118.0, 127.0)                 # upper deck
    sh.box("M_Int_Bunks", BX0 + 5, BX1 - 5, by0 + 3, YI, 127.0, 141.0)     # upper mattress
    sh.box("M_Int_Bunks", BX0, BX1, by0, YI, 216.0, 226.0)                 # canopy
    for xe in (BX0, BX1 - 6.0, (BX0 + BX1) / 2 - 3):
        if xe == (BX0 + BX1) / 2 - 3:
            sh.box("M_Int_Bunks", xe, xe + 6.0, YI - 30.0, YI, 52.0, 216.0)   # centre divider (half)
        else:
            sh.box("M_Int_Bunks", xe, xe + 6.0, by0, YI, 0.0, 226.0)          # end panels
    sh.box("M_Int_Bunks", BX0, BX1, by0 - 2.0, by0, 118.0, 131.0)          # upper guard rail
    li.box("M_Int_Lights", BX0 + 12, BX1 - 12, YI - 8.0, YI - 2.0, 114.5, 118.0)   # reading lights
    li.box("M_Int_Lights", BX0 + 12, BX1 - 12, YI - 8.0, YI - 2.0, 212.5, 216.0)
    # ladder at the aft end of the stack
    for dx in (4.0, 34.0):
        st.box("M_Int_Stations", BX0 + 10 + dx, BX0 + 13 + dx, by0 - 4.0, by0 - 1.0, 0.0, 150.0)
    for zr in (30.0, 60.0, 90.0, 120.0):
        st.box("M_Int_Stations", BX0 + 14, BX0 + 47, by0 - 4.0, by0 - 1.5, zr, zr + 3.0)

    # Lockers + bench, port (-Y) wall
    LX0, LX1, LD = -480.0, -222.0, 55.0
    n_lock = 5
    lw = (LX1 - LX0) / n_lock
    for k in range(n_lock):
        x0 = LX0 + k * lw
        sh.box("M_Int_Bunks", x0 + 0.8, x0 + lw - 0.8, -YI, -YI + LD, 0.0, 212.0)
        li.box("M_Int_Lights", x0 + lw * 0.5 - 1.5, x0 + lw * 0.5 + 1.5, -YI + LD, -YI + LD + 0.6, 150.0, 175.0)
    sh.box("M_Int_Shell", LX0, LX1, -YI, -YI + LD + 3, 212.0, 220.0)
    st.box("M_Int_Stations", LX0 + 20, LX1 - 20, -YI + LD + 8, -YI + LD + 46, 40.0, 46.0)   # bench seat
    for xl in (LX0 + 30, (LX0 + LX1) / 2, LX1 - 34):
        st.box("M_Int_Stations", xl, xl + 4, -YI + LD + 20, -YI + LD + 34, 0.0, 40.0)

    # aft wall vents either side of the hatch
    vent(r, 4, y_at_e4(200.0), y_at_e4(120.0), 20.0, 70.0)
    vent(r, 4, y_at_e4(-120.0), y_at_e4(-200.0), 20.0, 70.0)

    # ---------------- structural frames at the section breaks --------------
    for xf, lit in ((-200.0, True), (140.0, True)):
        for s in (-1, 1):
            r.cbox("Shell", "M_Int_Shell", xf - 16, xf + 16, *sorted((s * YI, s * (YI - 42))), 0.0, r.ceil_z)
            if lit:
                r.cbox("Lights", "M_Int_Lights", xf - 3, xf + 3, *sorted((s * (YI - 42), s * (YI - 42.8))), 30.0, 240.0)
        r.cbox("Shell", "M_Int_Shell", xf - 16, xf + 16, -YI, YI, 250.0, r.ceil_z)
        r.cbox("Lights", "M_Int_Lights", xf - 3, xf + 3, -YI + 42, YI - 42, 249.2, 250.0)
        dk.box("M_Int_Deck", xf - 16, xf + 16, -YI + 42, YI - 42, 0.0, 1.5)   # threshold plate

    # ---------------- CIC (x -200 .. +140) ----------------------------------
    CX = -30.0
    holo_emitter(r, CX, 0.0, table_r=112.0, top_z=98.0, holo_r=46.0, holo_z=178.0)
    co.torus("M_Int_Console", (CX, 0.0, 92.0), 158.0, 2.2, 48, 6)             # lean rail
    for k in range(6):
        a = 2 * math.pi * (k + 0.5) / 6
        co.cyl("M_Int_Console", CX + 158 * math.cos(a), 158 * math.sin(a), 0.0, 92.0, 2.5, 8)
    dk.annulus("M_Int_Deck", CX, 0.0, 128.0, 200.0, 0.0, 1.5, 48)
    li.annulus("M_Int_Lights", CX, 0.0, 200.0, 203.0, 0.0, 1.8, 48)
    # side-wall tactical banks: 3 consoles per side facing the wall + wall screens
    for s in (-1, 1):
        yaw = 90.0 * s                                   # operator faces +-Y (the wall)
        for x in (-140.0, -35.0, 70.0):
            yop = s * (YI - 58.0)
            console(st, x, yop, yaw, width=96.0, depth=52.0, monitor=False)
            chair(st, x, s * (YI - 58.0 - 42.0), yaw)
        # big wall display above the consoles (edge 2 is +Y, edge 6 is -Y)
        ei = 2 if s > 0 else 6
        conv = x_at_e2 if s > 0 else x_at_e6
        ta, tb = sorted((conv(-185.0), conv(115.0)))
        wall_display(r, ei, ta, tb, 132.0, 238.0, panels=3)
    # CIC ceiling light panels
    for dx in (-110.0, 80.0):
        for dy in (-150.0, 150.0):
            ceiling_strip(r, CX + dx - 40, CX + dx + 40, dy - 30, dy + 30)
    li.torus("M_Int_Lights", (CX, 0.0, r.ceil_z - 6.0), 120.0, 2.5, 48, 6)

    # ---------------- BRIDGE (x +140 .. +552) -------------------------------
    # captain's low dais + chair (Console zone = command seat + helm)
    CAPX = 238.0
    dk.prism_xy("M_Int_Deck", circle(CAPX, 0.0, 88.0, 40), 0.0, 7.0)
    li.annulus("M_Int_Lights", CAPX, 0.0, 84.0, 88.5, 7.0, 7.8, 40)
    chair(co, CAPX, 0.0, 0.0, slot="M_Int_Console", big=True, arm_screens="M_Int_Console")
    co.cyl("M_Int_Console", CAPX + 55, 60.0, 7.0, 85.0, 4.0, 10)            # side data post
    co.box("M_Int_Console", CAPX + 45, CAPX + 68, 45.0, 75.0, 85.0, 105.0)
    co.box("M_Int_Lights", CAPX + 46, CAPX + 67, 46.0, 74.0, 105.0, 105.8)
    # helm: chair on the trigger line (x = 0.6*XH) + wide wrap console
    HX = 0.6 * XH
    chair(co, HX - 4.0, 0.0, 0.0, slot="M_Int_Console")
    console(co, HX + 34.0, 0.0, 0.0, width=150.0, body="M_Int_Console", depth=60.0, desk_h=74.0)
    for s in (-1, 1):
        console(co, HX + 26.0, s * 95.0, s * 28.0, width=60.0, body="M_Int_Console", depth=48.0, monitor=False)
    # flanking nav / weapons stations
    for s in (-1, 1):
        console(st, 395.0, s * 190.0, s * 18.0, width=100.0, depth=55.0)
        chair(st, 350.0, s * 176.0, s * 18.0)
    # forward sill console ledge under the main window (clipped to the bow)
    r.cbox("Shell", "M_Int_Shell", XH - wt - 34.0, XH - wt, -YH, YH, 0.0, 88.0)
    r.cbox("Lights", "M_Int_Lights", XH - wt - 34.5, XH - wt - 34.0, -200.0, 200.0, 70.0, 80.0)
    # overhead: ring light over the captain + beams + side strips
    li.torus("M_Int_Lights", (CAPX, 0.0, r.ceil_z - 10.0), 130.0, 2.5, 48, 6)
    for s in (-1, 1):
        r.cbox("Shell", "M_Int_Shell", 160.0, XH - 70.0, s * 150 - 12, s * 150 + 12, 300.0, r.ceil_z)
        r.cbox("Lights", "M_Int_Lights", 164.0, XH - 74.0, s * 150 - 4, s * 150 + 4, 299.2, 300.0)
    # bridge side-wall pilasters between the side windows and the frame
    for x in (200.0, 495.0):
        pilaster(r, 2, x_at_e2(x))
        pilaster(r, 6, x_at_e6(x))

    # ---------------- floor: centre aisle hatch -> helm ---------------------
    dk.box("M_Int_Deck", -XH + wt + 40, HX - 60.0, -62.0, 62.0, 0.0, 1.2)
    for s in (-1, 1):
        for x0 in range(int(-XH + 60), int(HX - 70), 60):
            li.box("M_Int_Lights", x0, x0 + 30, s * 64.0 - 1.5, s * 64.0 + 1.5, 0.0, 1.6)
    return r


def wall_run(room, i, t0, t1, d0, d1, z0, z1, slot, part="Shell"):
    """Thin plate on the inner face of wall edge i (d measured from that face)."""
    wt = room.wall_t
    room.P(part).prism_xy(slot, room.edge_quad(i, t0, t1, wt + d0, wt + d1), z0, z1)


def commandxl_hero_detail(r, x_at_e2, x_at_e6, y_at_e4):
    """Exterior-grade greeble pass for the flagship bridge: gold livery trim,
    wainscot plating, overhead conduits + cable trays. Matches the CommandXL
    hull (deep navy plating, gold trim) instead of the generic teal kit."""
    sh = r.P("Shell")
    wt, YI = r.wall_t, r.yh - r.wall_t
    L = {i: r.edge_len(i) for i in range(8)}

    # --- gold command trim at 104-110 cm (edges 2..6, around the hatch) -----
    for i in (2, 3, 5, 6):
        wall_run(r, i, 0.0, L[i], 0.0, 1.6, 104.0, 110.0, "M_Int_Trim")
        wall_run(r, i, 0.0, L[i], 0.0, 0.8, 114.0, 115.5, "M_Int_Trim")    # pinstripe
    for (a, b) in ((0.0, 255.0), (495.0, L[4])):
        wall_run(r, 4, a, b, 0.0, 1.6, 104.0, 110.0, "M_Int_Trim")
        wall_run(r, 4, a, b, 0.0, 0.8, 114.0, 115.5, "M_Int_Trim")

    # --- wainscot plating: raised 1 cm plates, 3 cm shadow gaps --------------
    for i in (2, 3, 5, 6):
        n = max(1, int(round(L[i] / 120.0)))
        w = L[i] / n
        for k in range(n):
            wall_run(r, i, k * w + 1.5, (k + 1) * w - 1.5, 0.0, 1.0, 16.0, 98.0, "M_Int_Shell")
        # upper plates between trim and conduits, only where the wall is solid
        # (the chamfers; edges 2/6 carry windows + displays up there)
        if i in (3, 5):
            for k in range(n):
                wall_run(r, i, k * w + 1.5, (k + 1) * w - 1.5, 0.0, 1.0, 122.0, 336.0, "M_Int_Shell")

    # --- overhead conduits + brackets along both long walls -----------------
    for s, conv in ((1, x_at_e2), (-1, x_at_e6)):
        for (dy, rad, z) in ((9.0, 3.6, 352.0), (17.5, 2.6, 358.0), (9.0, 2.2, 362.0)):
            y = s * (YI - dy)
            sh.cyl_between("M_Int_Shell", (-380.0, y, z), (360.0, y, z), rad, 12)
        for x in range(-360, 361, 90):
            a, b = sorted((conv(x - 3.0), conv(x + 3.0)))
            wall_run(r, 2 if s > 0 else 6, a, b, 0.0, 24.0, 344.0, 368.0, "M_Int_Shell")

    # --- ceiling cable trays between the coffer edges ------------------------
    for s in (-1, 1):
        yc = s * 305.0
        r.cbox("Shell", "M_Int_Shell", -400.0, 300.0, yc - 16, yc + 16, 389.0, 391.0)   # tray floor
        for e in (-1, 1):
            r.cbox("Shell", "M_Int_Shell", -400.0, 300.0, yc + e * 16 - 1.2, yc + e * 16 + 1.2, 389.0, 397.0)
        for x in range(-390, 300, 70):                                             # hangers
            r.cbox("Shell", "M_Int_Shell", x - 1.5, x + 1.5, yc - 18, yc + 18, 391.0, r.ceil_z)
        for k, dy in enumerate((-9.0, -2.0, 6.0)):                                  # bundled cables
            sh.cyl_between("M_Int_Shell", (-398.0, yc + dy, 393.0 + (k % 2)), (298.0, yc + dy, 393.0 + (k % 2)),
                           2.2 + 0.6 * (k == 1), 8)

    # --- deck: raised gold-edged plinth ring at the dais foot + aisle edging -
    r.P("Deck").annulus("M_Int_Trim", -40.0, 0.0, 178.5, 181.0, 0.0, 3.0, 64)
    for sgn in (-1, 1):
        r.P("Deck").box("M_Int_Trim", -r.xh + wt + 40, -40.0 - 176.0, sgn * 69.0 - 1.0, sgn * 69.0 + 1.0, 0.0, 1.8)


# ----------------------------------------------------------------------------
# COMMANDXL: flagship bridge deck = crew arc + admiral's dais + fleet hologram
# ----------------------------------------------------------------------------
def build_commandxl():
    XH, YH, H = 485.0, 450.0, 410.0
    r = Room("SM_Int_CommandXL_Bridge", XH, YH, H, cf=110.0, cr=95.0)
    sh, dk, co, st, li = r.P("Shell"), r.P("Deck"), r.P("Console"), r.P("Stations"), r.P("Lights")
    wt = r.wall_t
    YI = YH - wt
    CF, CR = 110.0, 95.0
    x_at_e2 = lambda x: (XH - CF) - x
    x_at_e6 = lambda x: x + (XH - CR)
    y_at_e4 = lambda y: (YH - CR) - y
    L0 = r.edge_len(0)

    win_z = (72.0, 352.0)
    side_win = (140.0, 330.0)
    openings = {
        0: [(14.0, L0 - 14.0, *win_z)],
        1: [(10.0, r.edge_len(1) - 10.0, *win_z)],
        7: [(10.0, r.edge_len(7) - 10.0, *win_z)],
        2: [(x_at_e2(350.0), x_at_e2(115.0), *side_win)],
        6: [(x_at_e6(115.0), x_at_e6(350.0), *side_win)],
        4: [(y_at_e4(82.0), y_at_e4(-82.0), 0.0, 245.0)],
    }
    r.floor_ceiling()
    r.walls(openings)
    for i, ops in openings.items():
        for (t0, t1, z0, z1) in ops:
            if i == 4:
                r.hatch(4, t0, t1, z1)
            else:
                r.window(i, t0, t1, z0, z1, mullion_step=85.0)
    baseboard(r, skip={4: [(y_at_e4(82.0) - 14, y_at_e4(-82.0) + 14)]})

    # ---------------- ceiling structure ------------------------------------
    for xb in (-330.0, -150.0, 150.0, 330.0):
        r.cbox("Shell", "M_Int_Shell", xb - 18, xb + 18, -YI, YI, 368.0, r.ceil_z)
        r.cbox("Lights", "M_Int_Lights", xb - 3, xb + 3, -YI + 30, YI - 30, 367.2, 368.0)
    for s in (-1, 1):   # longitudinal coffer edges
        r.cbox("Shell", "M_Int_Shell", -XH, XH, s * 260 - 14, s * 260 + 14, 380.0, r.ceil_z)
    # wall pilasters (between side windows / displays), lit
    for x in (-400.0, -290.0, 85.0, 380.0):
        pilaster(r, 2, x_at_e2(x), w=22.0, d=14.0)
        pilaster(r, 6, x_at_e6(x), w=22.0, d=14.0)

    # ---------------- admiral's dais (centre) --------------------------------
    DX = -40.0
    dk.prism_xy("M_Int_Deck", circle(DX, 0.0, 178.0, 48), 0.0, 8.0)
    li.annulus("M_Int_Lights", DX, 0.0, 173.0, 178.5, 8.0, 8.8, 48)
    dk.prism_xy("M_Int_Deck", circle(DX, 0.0, 150.0, 48), 8.0, 9.0)
    chair(co, DX, 0.0, 0.0, slot="M_Int_Command", big=True, arm_screens="M_Int_Console")
    # C-shaped command ring around the chair, open to the rear
    co.annulus("M_Int_Command", DX, 0.0, 104.0, 140.0, 8.0, 88.0, 40, -128.0, 128.0)
    li.annulus("M_Int_Lights", DX, 0.0, 110.0, 134.0, 88.0, 88.8, 40, -120.0, 120.0)
    co.annulus("M_Int_Command", DX, 0.0, 132.0, 140.0, 88.0, 118.0, 40, -70.0, 70.0)   # forward screen lip
    li.annulus("M_Int_Lights", DX, 0.0, 131.2, 132.0, 92.0, 115.0, 40, -66.0, 66.0)

    # ---------------- fleet hologram between dais and helm -----------------
    HOX = 150.0
    holo_emitter(r, HOX, 0.0, table_r=62.0, top_z=36.0, holo_r=64.0, holo_z=205.0, big=True,
                 body="M_Int_Command")
    co.cyl("M_Int_Command", HOX, 0.0, r.ceil_z - 22.0, r.ceil_z, 34.0, 24)          # ceiling emitter
    li.annulus("M_Int_Lights", HOX, 0.0, 12.0, 30.0, r.ceil_z - 22.8, r.ceil_z - 22.0, 24)

    # ---------------- helm on the trigger line ----------------------------
    HX = 0.6 * XH
    chair(co, HX - 6.0, 0.0, 0.0, slot="M_Int_Command")
    console(co, HX + 32.0, 0.0, 0.0, width=170.0, body="M_Int_Command", depth=62.0, desk_h=74.0, detail=True)
    for s in (-1, 1):
        console(co, HX + 22.0, s * 108.0, s * 30.0, width=64.0, body="M_Int_Command", depth=50.0, monitor=False,
                detail=True)

    # ---------------- forward crew arc (Stations) ----------------------------
    for ang in (-56.0, -34.0, 34.0, 56.0):
        a = math.radians(ang)
        rc = 350.0
        console(st, rc * math.cos(a), rc * math.sin(a), ang, width=104.0, depth=54.0, detail=True)
        rch = 305.0
        chair(st, rch * math.cos(a), rch * math.sin(a), ang)
    dk.annulus("M_Int_Deck", 0.0, 0.0, 262.0, 430.0, 0.0, 1.2, 40, -64.0, 64.0)
    li.annulus("M_Int_Lights", 0.0, 0.0, 259.0, 262.0, 0.0, 1.5, 40, -64.0, 64.0)
    # forward sill console under the panorama
    r.cbox("Shell", "M_Int_Shell", XH - wt - 36.0, XH - wt, -YH, YH, 0.0, 70.0)
    r.cbox("Lights", "M_Int_Lights", XH - wt - 36.5, XH - wt - 36.0, -300.0, 300.0, 54.0, 62.0)

    # ---------------- side-wall operations banks + displays ------------------
    for s in (-1, 1):
        yaw = 90.0 * s
        for x in (-240.0, -130.0, -20.0):
            console(st, x, s * (YI - 58.0), yaw, width=96.0, depth=52.0, monitor=False, detail=True)
            chair(st, x, s * (YI - 58.0 - 42.0), yaw)
        ei = 2 if s > 0 else 6
        conv = x_at_e2 if s > 0 else x_at_e6
        ta, tb = sorted((conv(-280.0), conv(35.0)))
        wall_display(r, ei, ta, tb, 138.0, 300.0, panels=3)

    # ---------------- rear: tactical plot tables + status boards ------------
    for s in (-1, 1):
        cx, cy = -310.0, s * 250.0
        co.box("M_Int_Command", cx - 100, cx + 100, cy - 58, cy + 58, 0.0, 12.0)
        co.box("M_Int_Command", cx - 80, cx + 80, cy - 40, cy + 40, 12.0, 80.0)
        co.box("M_Int_Command", cx - 104, cx + 104, cy - 62, cy + 62, 80.0, 92.0)
        co.box("M_Int_Lights", cx - 92, cx + 92, cy - 50, cy + 50, 92.0, 92.8)
        for k in range(3):   # holo contacts over the plot
            li.ico("M_Int_Lights", (cx - 50 + 50 * k, cy + (20 if k % 2 else -15), 118.0 + 12 * k), 6.0, 1)
        ceiling_strip(r, cx - 100, cx + 100, cy - 20, cy + 20)
        # status boards on the rear wall either side of the hatch
        ta, tb = sorted((y_at_e4(s * 130.0), y_at_e4(s * 330.0)))
        wall_display(r, 4, ta, tb, 130.0, 290.0, panels=2)
    for s in (-1, 1):
        ta, tb = sorted((y_at_e4(s * 140.0), y_at_e4(s * 320.0)))
        vent(r, 4, ta, tb, 22.0, 70.0)

    # ---------------- lighting ------------------------------------------------
    li.torus("M_Int_Lights", (DX, 0.0, r.ceil_z - 8.0), 215.0, 3.0, 64, 6)
    li.torus("M_Int_Lights", (DX, 0.0, r.ceil_z - 8.0), 160.0, 2.2, 56, 6)
    li.annulus("M_Int_Lights", 0.0, 0.0, 345.0, 358.0, r.ceil_z - 4.0, r.ceil_z, 40, -60.0, 60.0)
    for s in (-1, 1):
        ceiling_strip(r, -300.0, 60.0, s * (YI - 70) - 8, s * (YI - 70) + 8)

    # ---------------- floor: centre aisle hatch -> dais ---------------------
    dk.box("M_Int_Deck", -XH + wt + 40, DX - 176.0, -70.0, 70.0, 0.0, 1.2)
    for s in (-1, 1):
        for x0 in range(int(-XH + 60), int(DX - 185), 55):
            li.box("M_Int_Lights", x0, x0 + 28, s * 72.0 - 1.5, s * 72.0 + 1.5, 0.0, 1.6)

    commandxl_hero_detail(r, x_at_e2, x_at_e6, y_at_e4)
    return r


# ----------------------------------------------------------------------------
# Validation against the C++ contract + export
# ----------------------------------------------------------------------------
# Kits that get the exterior-grade pass: bevels + weighted normals + unique bake.
HERO_KITS = {"commandxl"}


def weighted_normals(ob):
    """Smooth shading + face-area weighted custom normals: big flat faces stay
    dead flat, the bevel strips carry the curvature (standard hard-surface
    game-art normals, as on the hull meshes)."""
    for poly in ob.data.polygons:
        poly.use_smooth = True
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    m = ob.modifiers.new("wn", 'WEIGHTED_NORMAL')
    m.mode = 'FACE_AREA'
    m.weight = 100
    m.keep_sharp = True
    m.thresh = 0.01
    bpy.ops.object.modifier_apply(modifier=m.name)


def world_pts(ob):
    return [ob.matrix_world @ v.co for v in ob.data.vertices]


def validate(room, objs):
    shell = objs["Shell"]
    pts = world_pts(shell)
    mn = Vector([min(p[i] for p in pts) for i in range(3)])
    mx = Vector([max(p[i] for p in pts) for i in range(3)])
    c = (mn + mx) / 2; e = (mx - mn) / 2
    R = max((p - c).length for p in pts)
    s = TARGET_RADIUS / R
    half = e * s; org = c * s
    floor_z = org.z - half.z
    trig = Vector((org.x + half.x * 0.6, org.y, half.z * 0.6))           # UU, actor-local
    entry = Vector((org.x - half.x * 0.6, org.y, floor_z + 100.0))
    ext = Vector(TRIGGER_EXTENT_UU)
    problems = []
    # all part vertices inside shell bounds + inside the chamfered outline
    for name, ob in objs.items():
        for p in world_pts(ob):
            if any(p[i] < mn[i] - 0.01 or p[i] > mx[i] + 0.01 for i in range(3)):
                problems.append(f"{name}: vertex {tuple(round(v, 1) for v in p)} outside shell bounds")
                break
            if name != "Shell":
                q = (p.x, p.y)
                inside = all(((b[0] - a[0]) * (q[1] - a[1]) - (b[1] - a[1]) * (q[0] - a[0])) >= -0.05
                             for a, b in zip(room.outline, room.outline[1:] + room.outline[:1]))
                if not inside:
                    problems.append(f"{name}: vertex {tuple(round(v, 1) for v in p)} outside the room outline")
                    break
    if abs(mn.z - (-2.0)) > 0.01:
        problems.append(f"floor (bounds min Z) is {mn.z}, expected -2")
    tris = {}
    for name, ob in objs.items():
        ob.data.calc_loop_triangles()
        tris[name] = len(ob.data.loop_triangles)
    info = {
        "prefix": room.prefix,
        "design_bounds_cm": {"min": [round(v, 2) for v in mn], "max": [round(v, 2) for v in mx]},
        "shell_sphere_radius_design": round(R, 2),
        "runtime_uniform_scale_vs_design": round(s, 4),
        "runtime_scale_on_imported_mesh (FBX is 100x design units)": round(s / 100.0, 6),
        "walk_box_uu": {"depth_x": round(half.x * 2), "width_y": round(half.y * 2), "height_z": round(half.z * 2)},
        "floor_z_uu": round(floor_z, 1),
        "exit_trigger_center_uu": [round(v, 1) for v in trig],
        "exit_trigger_center_design_cm": [round(v / s, 1) for v in trig],
        "exit_trigger_extent_design_cm": [round(v / s, 1) for v in ext],
        "entry_spawn_uu": [round(v, 1) for v in entry],
        "entry_spawn_design_cm": [round(v / s, 1) for v in entry],
        "tris": tris, "tris_total": sum(tris.values()),
        "slots": {n: [m.name for m in ob.data.materials] for n, ob in objs.items()},
        "blender_to_ue_axes": "UE(x, y, z) = Blender(x, -y, z) * 100 * runtime scale",
        "problems": problems,
    }
    return info


def export(ob, dry):
    out = os.path.join(GEN, ob.name + ".fbx")
    if dry:
        print("  [dry-run] would write", out)
        return out
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    # Same exporter settings as generate_adastrea_assets.export_fbx() (the
    # existing interior contract), plus face smoothing so UE gets smoothing data.
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE',
    )
    return out


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    dry = "--dry-run" in argv
    builders = {"battleship": build_battleship, "commandxl": build_commandxl}
    which = [a for a in argv if a in builders] or ["battleship", "commandxl"]
    for key in which:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        sc = bpy.context.scene
        sc.unit_settings.system = 'METRIC'
        sc.unit_settings.length_unit = 'CENTIMETERS'
        room = builders[key]()
        hero = key in HERO_KITS
        if hero:
            for p in PART_ORDER:
                if p != "Lights":
                    room.parts[p].grid_cut()
                n = room.parts[p].bevel_hard_edges()
                print(f"  bevel {p}: {n} hard edges")
        objs = {p: room.parts[p].to_object() for p in PART_ORDER}
        for ob in objs.values():
            if hero:
                weighted_normals(ob)
            else:              # flat shading = hard-surface facets
                for poly in ob.data.polygons:
                    poly.use_smooth = False
        info = validate(room, objs)
        if hero and (not dry or "--uv-only" in argv) and not info["problems"] and "--no-bake" not in argv:
            # Unique UV atlas + Cycles-baked PBR set, same recipe as the hull
            # textures (Tools/build_unique_hull_textures.py).
            sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
            import bake_interior_unique
            info["unique_bake"] = bake_interior_unique.bake_kit(room, objs, argv)
            info["slots"] = {n: [m.name for m in ob.data.materials] for n, ob in objs.items()}
        print("CONTRACT", json.dumps(info, indent=1))
        if info["problems"]:
            print("CONTRACT_PROBLEMS", room.prefix, len(info["problems"]))
        for p in PART_ORDER:
            print("  export", export(objs[p], dry), info["tris"][p], "tris", info["slots"][p])
        if not dry:
            with open(os.path.join(GEN, room.prefix + "_contract.json"), "w") as f:
                json.dump(info, f, indent=1)


if __name__ == "__main__":
    main()
