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
    ob.display_type = 'WIRE'
    return ob


def export_with_collision(main_obj, ucx_objs, outname):
    """Export the render mesh together with its UCX_ collision meshes in one FBX,
    mirroring generate_adastrea_assets.export_fbx()'s settings exactly."""
    gen.sel_activate(main_obj)
    for u in ucx_objs:
        u.select_set(True)
    out = os.path.join(OUT_DIR, outname + '.fbx')
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
    joined = gen.join(parts, outname + "_Geo")
    gen.apply_mods(joined)
    gen.clean_mesh(joined)
    gen.sel_activate(joined)
    bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_VOLUME')
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
# ----------------------------------------------------------------------------
def build_docking_bay():
    L, W, H = 3 * GRID, 2 * GRID, 1 * GRID  # 1200, 800, 400
    parts = []

    body = gen.box("DB_Body", L - T * 2, W - T * 2, H - T * 2, loc=(0, 0, 0))
    gen.bevel(body, 18, 3)
    parts.append(body)

    skin = gen.box("DB_Skin", L, W, H, loc=(0, 0, 0))
    gen.bevel(skin, 22, 3)
    parts.append(skin)

    # side collars on N/S/E/W so this reads as connection-grid compliant,
    # same as build_station_module_shells()'s module_shell()
    for axis, sign, dim in ((0, 1, L), (0, -1, L), (1, 1, W), (1, -1, W)):
        cx = sign * (dim / 2 - 7.0) if axis == 0 else 0
        cy = sign * (dim / 2 - 7.0) if axis == 1 else 0
        if axis == 0:
            collar = gen.torus(f"DB_Collar{'N' if sign > 0 else 'S'}",
                                min(W, H) * 0.32, 8, loc=(cx, 0, 0),
                                rot=(0, math.radians(90), 0), maj=20, minr=6)
        else:
            collar = gen.torus(f"DB_Collar{'E' if sign > 0 else 'W'}",
                                min(L, H) * 0.32, 8, loc=(0, cy, 0),
                                rot=(math.radians(90), 0, 0), maj=20, minr=6)
        parts.append(collar)

    # the actual berthing port: a ring collar proud of the +Y face with a
    # recessed tunnel, this is the face a ship docks against
    port_y = W / 2 + 4
    port_ring = gen.torus("DB_PortRing", H * 0.34, 16, loc=(0, port_y, 0),
                           rot=(math.radians(90), 0, 0), maj=24, minr=8)
    parts.append(port_ring)
    port_collar = gen.cyl("DB_PortCollar", H * 0.30, 40, loc=(0, port_y + 15, 0),
                           rot=(math.radians(90), 0, 0), verts=20)
    parts.append(port_collar)

    # two mechanical clamp arms reaching from the body toward the port, the
    # docking-clamp read the task calls for
    for sign in (-1, 1):
        arm_x = sign * (H * 0.55)
        arm = gen.box(f"DB_ClampArm{'R' if sign > 0 else 'L'}",
                       26, W * 0.55, 30, loc=(arm_x, W * 0.12, H * 0.32),
                       rot=(0, 0, math.radians(sign * 12)))
        gen.bevel(arm, 6, 2)
        parts.append(arm)
        claw = gen.box(f"DB_ClampClaw{'R' if sign > 0 else 'L'}",
                        40, 30, 46, loc=(arm_x * 0.6, W * 0.42, H * 0.30))
        gen.bevel(claw, 5, 2)
        parts.append(claw)

    # X4-style structural girders on the flanks
    for sign in (-1, 1):
        girder = gen.box(f"DB_Girder{'R' if sign > 0 else 'L'}", 16, W - 60, 16,
                          loc=(sign * (L / 2 - 30), 0, -H / 2 + 25),
                          rot=(0, 0, 0))
        parts.append(girder)
        girder_top = gen.box(f"DB_GirderTop{'R' if sign > 0 else 'L'}", 16, W - 60, 16,
                              loc=(sign * (L / 2 - 30), 0, H / 2 - 25))
        parts.append(girder_top)

    # a few vents/panel greebles, matching module_shell's dressing
    for i, (gx, gy) in enumerate([(-L * 0.28, -W * 0.25), (L * 0.28, -W * 0.25)]):
        vent = gen.box(f"DB_Vent{i}", 46, 46, 6, loc=(gx, gy, H / 2 - 3))
        parts.append(vent)

    ucx_specs = [
        (0, 0, 0, L, W, H),                              # main body
        (0, port_y + 7, 0, H * 0.65, 34, H * 0.65),        # port collar bulge
    ]
    return finalize_module(parts, "SM_StationModule_DockingBay_01",
                            "M_StationModule_Connector", ucx_specs)


# ----------------------------------------------------------------------------
# CargoBay -- 2x2x1 cells (800 x 800 x 400 cm): blocky container massing,
# hatches, corrugated paneling. Material: M_StationModule_Utility (industrial
# / functional storage read).
# ----------------------------------------------------------------------------
def build_cargo_bay():
    L, W, H = 2 * GRID, 2 * GRID, 1 * GRID  # 800, 800, 400
    parts = []

    body = gen.box("CB_Body", L - T * 2, W - T * 2, H - T * 2, loc=(0, 0, 0))
    gen.bevel(body, 14, 2)
    parts.append(body)

    skin = gen.box("CB_Skin", L, W, H, loc=(0, 0, 0))
    gen.bevel(skin, 18, 2)
    parts.append(skin)

    for axis, sign, dim in ((0, 1, L), (0, -1, L), (1, 1, W), (1, -1, W)):
        cx = sign * (dim / 2 - 7.0) if axis == 0 else 0
        cy = sign * (dim / 2 - 7.0) if axis == 1 else 0
        if axis == 0:
            collar = gen.torus(f"CB_Collar{'N' if sign > 0 else 'S'}",
                                min(W, H) * 0.32, 8, loc=(cx, 0, 0),
                                rot=(0, math.radians(90), 0), maj=20, minr=6)
        else:
            collar = gen.torus(f"CB_Collar{'E' if sign > 0 else 'W'}",
                                min(L, H) * 0.32, 8, loc=(0, cy, 0),
                                rot=(math.radians(90), 0, 0), maj=20, minr=6)
        parts.append(collar)

    # blocky "stacked container" massing bulging off the +Z face -- reads as
    # cargo pods bolted onto the module rather than a bare box
    for i, (ox, oy) in enumerate([(-L * 0.22, -W * 0.20), (L * 0.20, -W * 0.05),
                                    (-L * 0.05, W * 0.25)]):
        cont = gen.box(f"CB_Container{i}", L * 0.34, W * 0.30, 60,
                        loc=(ox, oy, H / 2 + 26))
        gen.bevel(cont, 5, 2)
        parts.append(cont)
        # corrugation ribs across the long side of each container
        for r in range(4):
            rib = gen.box(f"CB_ContainerRib{i}_{r}", 6, W * 0.30 - 10, 4,
                           loc=(ox - L * 0.15 + r * (L * 0.30 / 3), oy, H / 2 + 26 + 31))
            parts.append(rib)

    # cargo hatch doors flush on the -Y face (paired sliding-door look)
    for sign in (-1, 1):
        hatch = gen.box(f"CB_Hatch{'L' if sign < 0 else 'R'}", L * 0.34, 6, H * 0.55,
                         loc=(sign * L * 0.19, -W / 2 + 3, -H * 0.05))
        parts.append(hatch)
        for r in range(3):
            rib = gen.box(f"CB_HatchRib{'L' if sign < 0 else 'R'}_{r}",
                           L * 0.30, 3, 4,
                           loc=(sign * L * 0.19, -W / 2 + 6.5,
                                -H * 0.05 - H * 0.2 + r * (H * 0.55 / 3)))
            parts.append(rib)

    # a couple of exterior vent/paneling greebles
    for i, (gx, gy) in enumerate([(-L * 0.3, W * 0.32), (L * 0.3, W * 0.32)]):
        vent = gen.box(f"CB_Vent{i}", 40, 40, 6, loc=(gx, gy, H / 2 - 3))
        parts.append(vent)

    ucx_specs = [
        (0, 0, 0, L, W, H),                           # main body
        (0, 0, H / 2 + 26, L * 0.94, W * 0.80, 60),   # stacked-container bulge
    ]
    return finalize_module(parts, "SM_StationModule_CargoBay_01",
                            "M_StationModule_Utility", ucx_specs)


# ----------------------------------------------------------------------------
# Market -- 2x2x1 cells (800 x 800 x 400 cm): commerce/habitation read --
# windows, finished paneling, an observation blister.
# Material: M_StationModule_Shell (the most "finished/habitation" texture set).
# ----------------------------------------------------------------------------
def build_market():
    L, W, H = 2 * GRID, 2 * GRID, 1 * GRID  # 800, 800, 400
    parts = []

    body = gen.box("MK_Body", L - T * 2, W - T * 2, H - T * 2, loc=(0, 0, 0))
    gen.bevel(body, 20, 3)
    parts.append(body)

    skin = gen.box("MK_Skin", L, W, H, loc=(0, 0, 0))
    gen.bevel(skin, 26, 3)
    parts.append(skin)

    for axis, sign, dim in ((0, 1, L), (0, -1, L), (1, 1, W), (1, -1, W)):
        cx = sign * (dim / 2 - 7.0) if axis == 0 else 0
        cy = sign * (dim / 2 - 7.0) if axis == 1 else 0
        if axis == 0:
            collar = gen.torus(f"MK_Collar{'N' if sign > 0 else 'S'}",
                                min(W, H) * 0.32, 8, loc=(cx, 0, 0),
                                rot=(0, math.radians(90), 0), maj=20, minr=6)
        else:
            collar = gen.torus(f"MK_Collar{'E' if sign > 0 else 'W'}",
                                min(L, H) * 0.32, 8, loc=(0, cy, 0),
                                rot=(math.radians(90), 0, 0), maj=20, minr=6)
        parts.append(collar)

    # a row of recessed window strips on two opposite long faces -- inset
    # boxes (not booleans, this kit avoids destructive booleans on the hero
    # shape) so they read as glazed panels once the emissive T_*_E map lands
    for sign in (-1, 1):
        for i in range(3):
            wy = sign * (W / 2 - 4)
            wx = -L * 0.28 + i * (L * 0.28)
            win = gen.box(f"MK_Window{'N' if sign > 0 else 'S'}{i}", 70, 8, 46,
                           loc=(wx, wy, 20))
            parts.append(win)
            frame = gen.box(f"MK_WindowFrame{'N' if sign > 0 else 'S'}{i}", 78, 6, 54,
                             loc=(wx, wy - sign * 2, 20))
            parts.append(frame)

    # observation blister -- a dome bulging off the +X face, the "display
    # element" the task calls for (an observation deck / storefront window)
    blister = gen.sphere("MK_Blister", H * 0.42, loc=(L / 2 - 6, 0, 0), verts=20)
    blister.scale = (0.55, 1.0, 1.0)
    gen.sel_activate(blister)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    parts.append(blister)
    blister_ring = gen.torus("MK_BlisterRing", H * 0.42, 8, loc=(L / 2 - 6, 0, 0),
                              rot=(0, math.radians(90), 0), maj=20, minr=6)
    parts.append(blister_ring)

    # finished paneling seams -- thin raised trim lines instead of bare bevel
    for i in range(2):
        seam = gen.box(f"MK_Seam{i}", L - 40, 4, 4,
                        loc=(0, -W / 2 + 40 + i * (W - 80), H * 0.30))
        parts.append(seam)

    ucx_specs = [
        (0, 0, 0, L, W, H),                          # main body
        (L / 2 - 6, 0, 0, H * 0.55, H * 0.9, H * 0.9),  # observation blister
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
