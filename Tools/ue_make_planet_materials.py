"""Make the gas-giant planet material and one instance per planet sectors need.

Run inside the editor (remote execution). Creates, when missing:
  /Game/Materials/Planets/M_PlanetBands      banded master: latitude bands warped by a slow wave
  /Game/Materials/Planets/MI_PlanetBands_<Name>  an instance per entry in PLANETS (colours re-applied every run)
  /Game/Meshes/Environment/SM_PlanetSphere    smooth lat-long sphere, radius 50 cm, poles on Z
Planets are that sphere scaled up (Tools/build_sector_level.py "planets"). The engine's basic
sphere looks faceted at planet size and its smooth editor spheres don't ship in a packaged build.
Its UVs run pole to pole, so the bands follow latitude. A placeholder until the Art > Hazard VFX, nebulae &
planets to-dos land.
"""
import unreal

DIR = "/Game/Materials/Planets"
MASTER = f"{DIR}/M_PlanetBands"

# name: (band colour A, band colour B, storm colour C, band count, warp amount)
PLANETS = {
    "KestrelIV": ((0.62, 0.38, 0.18), (0.86, 0.70, 0.46), (0.45, 0.20, 0.10), 22.0, 0.025),
}

mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def node(mat, cls, x, y):
    return mel.create_material_expression(mat, cls, x, y)


def make_master():
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset("M_PlanetBands", DIR, unreal.Material, unreal.MaterialFactoryNew())

    # Bands from the planet's own local position, not its UVs: latitude = local Z / 50,
    # so they always circle the vertical axis whatever the mesh's UV layout.
    lp = node(mat, unreal.MaterialExpressionLocalPosition, -1500, 0)
    # Single axes by dot product with a unit vector (scalar out, no channel masks to get wrong).
    ax_x = node(mat, unreal.MaterialExpressionConstant3Vector, -1500, -150)
    ax_x.set_editor_property("constant", unreal.LinearColor(1, 0, 0, 0))
    ax_z = node(mat, unreal.MaterialExpressionConstant3Vector, -1500, 150)
    ax_z.set_editor_property("constant", unreal.LinearColor(0, 0, 1, 0))
    u_raw = node(mat, unreal.MaterialExpressionDotProduct, -1350, -100)
    mel.connect_material_expressions(lp, "", u_raw, "A")
    mel.connect_material_expressions(ax_x, "", u_raw, "B")
    v_raw = node(mat, unreal.MaterialExpressionDotProduct, -1350, 100)
    mel.connect_material_expressions(lp, "", v_raw, "A")
    mel.connect_material_expressions(ax_z, "", v_raw, "B")
    # Local position is the mesh's own, unscaled: SM_PlanetSphere has radius 50, so * 0.02 = -1..1.
    u = node(mat, unreal.MaterialExpressionMultiply, -1200, -100)
    u.set_editor_property("const_b", 0.02)
    mel.connect_material_expressions(u_raw, "", u, "A")
    v = node(mat, unreal.MaterialExpressionMultiply, -1200, 100)
    v.set_editor_property("const_b", 0.02)
    mel.connect_material_expressions(v_raw, "", v, "A")

    # Warp the latitude by a slow wave around the planet: v' = v + sin(u * WarpFreq) * WarpAmount
    warp_freq = node(mat, unreal.MaterialExpressionScalarParameter, -1200, -250)
    warp_freq.set_editor_property("parameter_name", "WarpFreq")
    warp_freq.set_editor_property("default_value", 5.0)
    warp_amt = node(mat, unreal.MaterialExpressionScalarParameter, -900, -250)
    warp_amt.set_editor_property("parameter_name", "WarpAmount")
    warp_amt.set_editor_property("default_value", 0.02)
    u_f = node(mat, unreal.MaterialExpressionMultiply, -1000, -150)
    mel.connect_material_expressions(u, "", u_f, "A")
    mel.connect_material_expressions(warp_freq, "", u_f, "B")
    wave = node(mat, unreal.MaterialExpressionSine, -850, -150)
    mel.connect_material_expressions(u_f, "", wave, "")
    wave_a = node(mat, unreal.MaterialExpressionMultiply, -700, -150)
    mel.connect_material_expressions(wave, "", wave_a, "A")
    mel.connect_material_expressions(warp_amt, "", wave_a, "B")
    v2 = node(mat, unreal.MaterialExpressionAdd, -550, 50)
    mel.connect_material_expressions(v, "", v2, "A")
    mel.connect_material_expressions(wave_a, "", v2, "B")

    # Bands: 0.5 + 0.5 * sin(v' * BandCount), and a second, finer set for storm streaks.
    bands = node(mat, unreal.MaterialExpressionScalarParameter, -550, 250)
    bands.set_editor_property("parameter_name", "BandCount")
    bands.set_editor_property("default_value", 18.0)
    vb = node(mat, unreal.MaterialExpressionMultiply, -400, 100)
    mel.connect_material_expressions(v2, "", vb, "A")
    mel.connect_material_expressions(bands, "", vb, "B")
    s1 = node(mat, unreal.MaterialExpressionSine, -250, 100)
    mel.connect_material_expressions(vb, "", s1, "")
    a1 = node(mat, unreal.MaterialExpressionConstantBiasScale, -100, 100)
    a1.set_editor_property("bias", 1.0)
    a1.set_editor_property("scale", 0.5)
    mel.connect_material_expressions(s1, "", a1, "")
    vb3 = node(mat, unreal.MaterialExpressionMultiply, -400, 300)
    vb3.set_editor_property("const_b", 2.7)
    mel.connect_material_expressions(vb, "", vb3, "A")
    s2 = node(mat, unreal.MaterialExpressionSine, -250, 300)
    mel.connect_material_expressions(vb3, "", s2, "")
    a2 = node(mat, unreal.MaterialExpressionConstantBiasScale, -100, 300)
    a2.set_editor_property("bias", 1.0)
    a2.set_editor_property("scale", 0.25)
    mel.connect_material_expressions(s2, "", a2, "")

    col_a = node(mat, unreal.MaterialExpressionVectorParameter, -100, -350)
    col_a.set_editor_property("parameter_name", "ColorA")
    col_b = node(mat, unreal.MaterialExpressionVectorParameter, -100, -200)
    col_b.set_editor_property("parameter_name", "ColorB")
    col_c = node(mat, unreal.MaterialExpressionVectorParameter, 100, -350)
    col_c.set_editor_property("parameter_name", "ColorC")
    lerp1 = node(mat, unreal.MaterialExpressionLinearInterpolate, 150, -150)
    mel.connect_material_expressions(col_a, "", lerp1, "A")
    mel.connect_material_expressions(col_b, "", lerp1, "B")
    mel.connect_material_expressions(a1, "", lerp1, "Alpha")
    lerp2 = node(mat, unreal.MaterialExpressionLinearInterpolate, 350, -100)
    mel.connect_material_expressions(lerp1, "", lerp2, "A")
    mel.connect_material_expressions(col_c, "", lerp2, "B")
    mel.connect_material_expressions(a2, "", lerp2, "Alpha")
    mel.connect_material_property(lerp2, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = node(mat, unreal.MaterialExpressionConstant, 350, 150)
    rough.set_editor_property("r", 0.95)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    spec = node(mat, unreal.MaterialExpressionConstant, 350, 250)
    spec.set_editor_property("r", 0.15)
    mel.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)

    mel.recompile_material(mat)
    eal.save_loaded_asset(mat)
    return mat


SPHERE = "/Game/Meshes/Environment/SM_PlanetSphere"


def make_sphere():
    gs = unreal.GeometryScript_Primitives
    mesh = unreal.DynamicMesh()
    gs.append_sphere_lat_long(mesh, unreal.GeometryScriptPrimitiveOptions(), unreal.Transform(),
                              radius=50.0, steps_phi=96, steps_theta=192)
    opts = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    opts.set_editor_property("enable_collision", True)
    asset, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(mesh, SPHERE, opts)
    unreal.log(f"ue_make_planet_materials: {SPHERE} ({outcome})")
    eal.save_loaded_asset(asset)


def main():
    import sys
    if not eal.does_asset_exist(SPHERE):
        make_sphere()
    if "--rebuild-master" in sys.argv:
        # Rebuilding a graph in place leaves stray nodes; start clean. Levels pick the new
        # instances up when Tools/build_sector_level.py rebuilds them.
        for name in PLANETS:
            eal.delete_asset(f"{DIR}/MI_PlanetBands_{name}")
        eal.delete_asset(MASTER)
    master = unreal.load_asset(MASTER) if eal.does_asset_exist(MASTER) else make_master()
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    for name, (a, b, c, bands, warp) in PLANETS.items():
        path = f"{DIR}/MI_PlanetBands_{name}"
        if eal.does_asset_exist(path):
            mi = unreal.load_asset(path)
        else:
            mi = tools.create_asset(f"MI_PlanetBands_{name}", DIR, unreal.MaterialInstanceConstant,
                                    unreal.MaterialInstanceConstantFactoryNew())
            mi.set_editor_property("parent", master)
        for pname, col in (("ColorA", a), ("ColorB", b), ("ColorC", c)):
            mel.set_material_instance_vector_parameter_value(mi, pname, unreal.LinearColor(col[0], col[1], col[2], 1.0))
        mel.set_material_instance_scalar_parameter_value(mi, "BandCount", bands)
        mel.set_material_instance_scalar_parameter_value(mi, "WarpAmount", warp)
        mel.update_material_instance(mi)
        eal.save_loaded_asset(mi)
        unreal.log(f"ue_make_planet_materials: {path}")


main()
