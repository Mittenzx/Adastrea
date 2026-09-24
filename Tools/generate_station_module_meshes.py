"""
Adastrea Station Module Meshes -- DockingBay / CargoBay / Market (Blender 5.x, headless)
=========================================================================================
Builds the three real static meshes that Content/Blueprints/Stations/Modules/*.uasset
currently reference only as the /Engine/BasicShapes/Cube.Cube placeholder:

  BP_SpaceStationModule_DockingBay  -> SM_StationModule_DockingBay_01
  BP_CargoBayModule                -> SM_StationModule_CargoBay_01
  BP_SpaceStationModule_Market     -> SM_StationModule_Market_01

Reuses the exact conventions already established by Tools/generate_adastrea_assets.py
(same primitive helpers, same 400cm/grid-cell sizing used by build_station_module_shells(),
same triplanar smart_uv, same real-world-cm FBX export settings) so these three meshes read
as part of the same kit as SM_StationModule_Shell_Standard/_Large/etc. Unlike that shared-shell
pass (which relies on UE's "Add Auto Convex" on import per Assets/FBX/generated/README_IMPORT.md,
since no UCX_ precedent exists anywhere in this project), these three ship with authored
UCX_<MeshName>_NN convex/box collision per the blender-expert skill's Unreal collision
convention, because the task calling for these meshes explicitly asked for real collision
geometry rather than relying on auto-convex.

Grid sizes match docs/11-TECHNICAL_SPECS/STATION_BUILDER.md's module catalog table and
generate_adastrea_assets.py's build_station_module_shells() distribution notes:
  DockingBayModule  3x2 cells -> 1200 x 800 x 400 cm (the one "Large" footprint)
  CargoBayModule    2x2 cells ->  800 x 800 x 400 cm (the "Standard" workhorse size)
  MarketplaceModule 2x2 cells ->  800 x 800 x 400 cm (same Standard footprint)

Usage:
  "C:\\Program Files\\Blender Foundation\\Blender 5.2\\blender.exe" -b --python Tools/generate_station_module_meshes.py
"""
import os
import sys
import math

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen  # noqa: E402  (reuse the project's own helpers)

GRID = 400.0  # cm per grid cell, matches build_station_module_shells()
T = 10.0      # hull plate thickness, matches build_station_module_shells()
# 2026-09-23 review fix (Modules-A): the hull skin used to fill the whole footprint, which
# buried every collar/window/hatch/clamp inside it and forced the port ring, cargo pods and
# observation blister to overhang the grid cell. The skin is now inset INSET cm per side
# so details sit proud of it while everything stays inside the footprint box.
INSET = 30.0

# Assets/FBX/generated -- same folder as SM_StationModule_Shell_*.fbx. NOTE: this is
# computed from this script's own location rather than reusing gen.BASE, because
# generate_adastrea_assets.py hardcodes an absolute path into the *main* checkout
# (C:\Users\akuma\Adastrea\...) which is not this worktree's copy of the repo. Also
# repoint gen.BASE/gen.TEXDIR so wire_pbr_material() reads textures from this
# worktree instead of the main checkout.
OUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                        "Assets", "FBX", "generated")
gen.BASE = OUT_DIR
gen.TEXDIR = os.path.join(OUT_DIR, "Textures")


def make_ucx_box(name, cx, cy, cz, sx, sy, sz):
    """A simple box collision hull, named for Unreal's UCX_ auto-collision convention."""
    ob = gen.box(name, sx, sy, sz, loc=(cx, cy, cz))
    gen.sel_activate(ob)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    ob.display_type = 'WIRE'
    return ob


def face_collar(prefix, axis, sign, half, r, zc=0.0):
    """Connection collar on one side face: a neck from the inset skin out to the
    footprint boundary plus a trim ring, so neighbours' collars meet on the boundary."""
    tag = f"{prefix}_Collar{'NSEW'[(0 if sign > 0 else 1) if axis == 0 else (3 if sign > 0 else 2)]}"
    t_mid = sign * (half - INSET / 2)
    t_ring = sign * (half - 8)
    if axis == 0:
        neck = gen.cyl(tag + "Neck", r, INSET, loc=(t_mid, 0, zc), rot=(0, math.radians(90), 0), verts=16)
        ring = gen.torus(tag, r, 6, loc=(t_ring, 0, zc), rot=(0, math.radians(90), 0), maj=20, minr=6)
    else:
        neck = gen.cyl(tag + "Neck", r, INSET, loc=(0, t_mid, zc), rot=(math.radians(90), 0, 0), verts=16)
        ring = gen.torus(tag, r, 6, loc=(0, t_ring, zc), rot=(math.radians(90), 0, 0), maj=20, minr=6)
    return [neck, ring]


def export_with_collision(main_obj, ucx_objs, outname):
    """Export the render mesh together with its UCX_ collision meshes in one FBX,
    mirroring generate_adastrea_assets.export_fbx()'s settings exactly."""
    gen.sel_activate(main_obj)
    for u in ucx_objs:
        u.select_set(True)
    out = os.path.join(OUT_DIR, outname + '.fbx')
    # Geometry values are centimetres: declare 1 BU = 1 cm so the FBX carries
    # UnitScaleFactor=1 (cm). With the factory default scale_length=1.0 it wrote 100
    # (metres) and UE imports 100x oversized at import scale 1.0 (AGENT_BOARD 2026-09-23).
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 0.01
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        mesh_smooth_type='FACE',
        axis_forward='-Y', axis_up='Z',
    )
    return out


def finalize_module(parts, outname, matname, ucx_specs):
    """Join render parts into one mesh (matching gen.finalize_part), build UCX_
    collision as separate un-joined objects, and export both together."""
    # apply every part's own modifiers BEFORE joining: bpy.ops.object.join() keeps only
    # the active (first) object's modifier stack, so the other parts' bevels were
    # silently dropped and the first part's bevel was re-applied to the whole mesh.
    for p in parts:
        gen.apply_mods(p)
    joined = gen.join(parts, outname + "_Geo")
    gen.clean_mesh(joined)
    # pivot = footprint centre (the station editor places/adjacency-tests modules by
    # actor location), not centre-of-volume, which drifts with asymmetric details
    bpy.context.scene.cursor.location = (0, 0, 0)
    gen.sel_activate(joined)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    gen.smart_uv(joined)
    joined.name = outname

    mat = bpy.data.materials.get(matname)
    if mat is None:
        mat = bpy.data.materials.new(matname)
    mat.use_nodes = True
    if not joined.data.materials:
        joined.data.materials.append(mat)
    else:
        joined.data.materials[0] = mat
    try:
        gen.wire_pbr_material(mat, gen.resolve_texset(matname), emissive_strength=2.6)
    except Exception as e:
        print(f"  [warn] material wire fail {matname}: {e}")

    ucx_objs = []
    for i, (cx, cy, cz, sx, sy, sz) in enumerate(ucx_specs):
        ucx_objs.append(make_ucx_box(f"UCX_{outname}_{i:02d}", cx, cy, cz, sx, sy, sz))

    verts = len(joined.data.vertices)
    tris = sum(max(0, len(p.vertices) - 2) for p in joined.data.polygons)
    out = export_with_collision(joined, ucx_objs, outname)
    return joined, ucx_objs, out, verts, tris


# ----------------------------------------------------------------------------
# DockingBay -- 3x2x1 cells (1200 x 800 x 400 cm): docking clamps/ports +
# structural framing. Material: M_StationModule_Connector (it's the module
# whose entire job is *connecting* a ship to the station).
# The +Y face carries the berthing port; the other three faces get connection collars.
# ----------------------------------------------------------------------------
def build_docking_bay():
    L, W, H = 3 * GRID, 2 * GRID, 1 * GRID  # 1200, 800, 400
    I = INSET
    parts = []

    skin = gen.box("DB_Skin", L - 2 * I, W - 2 * I, H - 2 * I, loc=(0, 0, 0))
    gen.bevel(skin, 22, 3)
    parts.append(skin)

    parts += face_collar("DB", 0, 1, L / 2, min(W, H) * 0.32)
    parts += face_collar("DB", 0, -1, L / 2, min(W, H) * 0.32)
    parts += face_collar("DB", 1, -1, W / 2, min(L, H) * 0.32)

    # the berthing port on +Y: collar block from the skin to the boundary with a
    # big ring on its face (was overhanging the footprint by 39 cm)
    port_collar = gen.cyl("DB_PortCollar", H * 0.30, I, loc=(0, W / 2 - I / 2, 0),
                           rot=(math.radians(90), 0, 0), verts=20)
    parts.append(port_collar)
    port_ring = gen.torus("DB_PortRing", H * 0.34, 8, loc=(0, W / 2 - 10, 0),
                           rot=(math.radians(90), 0, 0), maj=24, minr=8)
    parts.append(port_ring)

    # two mechanical clamp arms on the roof reaching toward the port (were buried
    # inside the skin)
    top = H / 2 - I
    for sign in (-1, 1):
        arm_x = sign * (H * 0.55)
        arm = gen.box(f"DB_ClampArm{'R' if sign > 0 else 'L'}",
                       26, W * 0.55, 26, loc=(arm_x, W * 0.12, top + 13),
                       rot=(0, 0, math.radians(sign * 12)))
        gen.bevel(arm, 6, 2)
        parts.append(arm)
        claw = gen.box(f"DB_ClampClaw{'R' if sign > 0 else 'L'}",
                        40, 30, 26, loc=(arm_x * 0.6, W * 0.42, top + 13))
        gen.bevel(claw, 5, 2)
        parts.append(claw)

    # X4-style structural girders on the flanks (now proud of the inset skin)
    for sign in (-1, 1):
        for gz in (-H / 2 + I + 25, H / 2 - I - 25):
            girder = gen.box(f"DB_Girder{'R' if sign > 0 else 'L'}{int(gz)}", 16, W - 2 * I - 60, 16,
                              loc=(sign * (L / 2 - I), 0, gz))
            parts.append(girder)

    for i, (gx, gy) in enumerate([(-L * 0.28, -W * 0.25), (L * 0.28, -W * 0.25)]):
        vent = gen.box(f"DB_Vent{i}", 46, 46, 6, loc=(gx, gy, top + 3))
        parts.append(vent)

    ucx_specs = [
        (0, 0, 0, L - 2 * I, W - 2 * I, H - 2 * I),        # main body
        (0, W / 2 - I / 2, 0, H * 0.65, I, H * 0.65),       # port collar block
    ]
    return finalize_module(parts, "SM_StationModule_DockingBay_01",
                            "M_StationModule_Connector", ucx_specs)


# ----------------------------------------------------------------------------
# CargoBay -- 2x2x1 cells (800 x 800 x 400 cm): blocky container massing,
# hatches, corrugated paneling. Material: M_StationModule_Utility (industrial
# / functional storage read). Skin is lowered 20 cm so the roof containers fit
# inside the 400 cm cell height (they used to overhang it by 59 cm).
# ----------------------------------------------------------------------------
def build_cargo_bay():
    L, W, H = 2 * GRID, 2 * GRID, 1 * GRID  # 800, 800, 400
    I = INSET
    zc, sh = -20.0, H - 2 * I - 40            # skin centre / height -> z -170..130
    top = zc + sh / 2
    parts = []

    skin = gen.box("CB_Skin", L - 2 * I, W - 2 * I, sh, loc=(0, 0, zc))
    gen.bevel(skin, 18, 2)
    parts.append(skin)

    for axis, sign in ((0, 1), (0, -1), (1, 1), (1, -1)):
        parts += face_collar("CB", axis, sign, (L if axis == 0 else W) / 2, min(L, H) * 0.32, zc)

    # stacked cargo pods on the roof
    for i, (ox, oy) in enumerate([(-L * 0.22, -W * 0.20), (L * 0.20, -W * 0.05),
                                    (-L * 0.05, W * 0.25)]):
        cont = gen.box(f"CB_Container{i}", L * 0.34, W * 0.30, 60,
                        loc=(ox, oy, top + 30))
        gen.bevel(cont, 5, 2)
        parts.append(cont)
        for r in range(4):
            rib = gen.box(f"CB_ContainerRib{i}_{r}", 6, W * 0.30 - 10, 4,
                           loc=(ox - L * 0.15 + r * (L * 0.30 / 3), oy, top + 61))
            parts.append(rib)

    # paired cargo hatch doors on the -Y face, either side of the collar (they were
    # coplanar with the old full-size skin and z-fought)
    face_y = -(W / 2 - I)
    for sign in (-1, 1):
        hx = sign * 236
        hatch = gen.box(f"CB_Hatch{'L' if sign < 0 else 'R'}", 176, 6, H * 0.55,
                         loc=(hx, face_y - 3, zc))
        parts.append(hatch)
        for r in range(3):
            rib = gen.box(f"CB_HatchRib{'L' if sign < 0 else 'R'}_{r}", 160, 3, 4,
                           loc=(hx, face_y - 7.5, zc - H * 0.2 + r * (H * 0.55 / 3)))
            parts.append(rib)

    for i, (gx, gy) in enumerate([(-L * 0.3, W * 0.32), (L * 0.3, W * 0.32)]):
        vent = gen.box(f"CB_Vent{i}", 40, 40, 6, loc=(gx, gy, top + 3))
        parts.append(vent)

    ucx_specs = [
        (0, 0, zc, L - 2 * I, W - 2 * I, sh),           # main body
        (0, 0, top + 30, L * 0.9, W * 0.80, 60),         # stacked-container bulge
    ]
    return finalize_module(parts, "SM_StationModule_CargoBay_01",
                            "M_StationModule_Utility", ucx_specs)


# ----------------------------------------------------------------------------
# Market -- 2x2x1 cells (800 x 800 x 400 cm): commerce/habitation read --
# windows, finished paneling, an observation dome.
# Material: M_StationModule_Shell (the most "finished/habitation" texture set).
# The observation blister moved from the +X face (86 cm overhang, and it sat on top of
# the N collar) to a roof dome, so all four side faces keep their connection collars.
# ----------------------------------------------------------------------------
def build_market():
    L, W, H = 2 * GRID, 2 * GRID, 1 * GRID  # 800, 800, 400
    I = INSET
    zc, sh = -35.0, 270.0                      # skin z -170..100, dome above
    top = zc + sh / 2
    parts = []

    skin = gen.box("MK_Skin", L - 2 * I, W - 2 * I, sh, loc=(0, 0, zc))
    gen.bevel(skin, 26, 3)
    parts.append(skin)

    for axis, sign in ((0, 1), (0, -1), (1, 1), (1, -1)):
        parts += face_collar("MK", axis, sign, (L if axis == 0 else W) / 2, min(L, H) * 0.30, zc)

    # glazed window panels on the two long faces, clear of the collar
    face = W / 2 - I
    for sign in (-1, 1):
        for i, wx in enumerate((-280, -180, 180, 280)):
            frame = gen.box(f"MK_WindowFrame{'N' if sign > 0 else 'S'}{i}", 78, 6, 54,
                             loc=(wx, sign * (face + 1), 20))
            parts.append(frame)
            win = gen.box(f"MK_Window{'N' if sign > 0 else 'S'}{i}", 70, 8, 46,
                           loc=(wx, sign * (face + 3), 20))
            parts.append(win)

    # observation dome on the roof
    dome = gen.sphere("MK_Blister", H * 0.42, loc=(0, 0, top), verts=20)
    dome.scale = (1.0, 1.0, 0.55)
    gen.sel_activate(dome)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    parts.append(dome)
    dome_ring = gen.torus("MK_BlisterRing", H * 0.42, 6, loc=(0, 0, top), maj=24, minr=6)
    parts.append(dome_ring)

    # finished paneling seams along the long faces
    for sign in (-1, 1):
        seam = gen.box(f"MK_Seam{sign}", L - 2 * I - 40, 4, 4, loc=(0, sign * (face + 2), 70))
        parts.append(seam)

    ucx_specs = [
        (0, 0, zc, L - 2 * I, W - 2 * I, sh),                        # main body
        (0, 0, top + H * 0.115, H * 0.76, H * 0.76, H * 0.23),       # roof dome
    ]
    return finalize_module(parts, "SM_StationModule_Market_01",
                            "M_StationModule_Shell", ucx_specs)


def main():
    gen.setup_scene()
    os.makedirs(OUT_DIR, exist_ok=True)

    results = {}
    print("Building SM_StationModule_DockingBay_01...")
    gen.clear_scene()
    obj, ucx, out, verts, tris = build_docking_bay()
    results["DockingBay"] = (out, verts, tris, len(ucx))
    print(f"  exported: {os.path.basename(out)} verts={verts} tris={tris} ucx={len(ucx)}")

    print("Building SM_StationModule_CargoBay_01...")
    gen.clear_scene()
    obj, ucx, out, verts, tris = build_cargo_bay()
    results["CargoBay"] = (out, verts, tris, len(ucx))
    print(f"  exported: {os.path.basename(out)} verts={verts} tris={tris} ucx={len(ucx)}")

    print("Building SM_StationModule_Market_01...")
    gen.clear_scene()
    obj, ucx, out, verts, tris = build_market()
    results["Market"] = (out, verts, tris, len(ucx))
    print(f"  exported: {os.path.basename(out)} verts={verts} tris={tris} ucx={len(ucx)}")

    print("DONE")
    for name, (out, verts, tris, nucx) in results.items():
        size = os.path.getsize(out) if os.path.exists(out) else 0
        print(f"  {name}: {out} ({size} B, {verts} verts, {tris} tris, {nucx} UCX hulls)")


if __name__ == "__main__":
    main()
