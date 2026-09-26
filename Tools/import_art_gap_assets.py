"""
Adastrea art-gap importer (Unreal Editor Python)
================================================
Imports / reimports FBX from Assets/FBX/generated/ into the right plugin/content
folder, then assigns materials by slot name. Also carries the Phase-1 material fixes
(engineering-bay material, bridge viewport glass, station-module sampler/slot fixes)
so they can be replayed after any reimport.

Runs either headless (editor CLOSED):
  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" ^
     C:\\Users\\akuma\\Adastrea\\Adastrea.uproject -run=pythonscript ^
     -script="C:\\Users\\akuma\\Adastrea\\Tools\\import_art_gap_assets.py <args>" ^
     -unattended -nosplash -nop4 -nullrhi
or from the editor's Python console:  py "C:/.../import_art_gap_assets.py" <args>

Args (combinable):
  SM_Foo SM_Bar ...     FBX basenames (with or without .fbx) to import/reimport
  --glob=PATTERN        also import every Assets/FBX/generated/PATTERN (e.g. SM_StationModule_*_01.fbx)
  --fixes               (re)build M_Int_Eng + M_Int_Viewport glass, fix station-module
                        sampler types, assign family materials to the shared shells
  --accents             (re)build M_StationModule_Accent + per-domain MI_StationModule_Accent_<X>
                        (run before importing meshes that have an M_StationModule_Accent slot)
  --interiors           import the Interiors texture library (Assets/Textures/generated/interiors)
                        and build M_IntSurface_Oriented + all interior/station MIs + viewport
                        glass per that folder's MATERIAL_MAPPING.md, then rebind SM_Int_* slots
  --unique-hull=Battleship  import T_<Ship>_Unique_* and build M_<Ship>_Hull_Unique (run with the
                        SM_Ship_<Ship>_01_Assembled_UniqueUV mesh so its M_Assembled slot binds)
  --unique-interior=CommandXLBridge  import the kit's baked T_Int_<Kit>_{Floor,Room,Kit}_* atlases
                        (Tools/bake_interior_unique.py) + T_Int_DetailMicro_N and build
                        M_Int_UniqueBaked + MI_Int_<Kit>_<Atlas> (run with the kit's SM_Int_* meshes)
  --capital-bps         set InteriorShellMesh/InteriorFamily on BP_Battleship / BP_CommandXL
  --glass               rebuild only the M_Int_Viewport library glass (no other interior MIs)
  --textures=T_A,T_B    (re)import texture sets from Assets/FBX/generated/Textures with
                        the project's settings (see TEXTURE RULES below)
  --dry-run             print what would happen, change nothing

Import conventions (match the Blender exporters: FBX_SCALE_ALL + apply_unit_scale,
-Y forward / Z up, real-world centimetres):
  * import_uniform_scale 1.0, no import rotation/translation, combine meshes
  * UCX_<MeshName>_NN hulls present in the FBX -> used as collision (auto-collision OFF);
    otherwise UE auto-generated simple collision (the shared-shell precedent)
  * no FBX materials/textures imported - slots are bound by SLOT NAME to existing
    material assets (SLOT_ALIASES first, then MATERIAL_SEARCH_DIRS); on reimport, a slot
    with no match keeps whatever real material it already had
  * Nanite left off (project ships SM5 + SM6, Lumen/VSM disabled)

TEXTURE RULES (commits 85606ca1 / 88188d68 / 6385d477):
  _D, _E           sRGB on,  TC_DEFAULT,   sampler Color
  _N               sRGB off, TC_NORMALMAP, Flip Green OFF (generator writes DirectX-style), sampler Normal
  _R, _M, _AO      sRGB off, TC_MASKS,     sampler Masks

Collision *profiles* are a component-level setting, not a mesh-asset one; this script
never edits component collision. (If you do, set collisionProfileName AND
collisionEnabled together or it reverts on reload.)
"""
import fnmatch
import os
import sys

import unreal

PROJECT_DIR = os.path.normpath(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
GEN_DIR = os.path.join(PROJECT_DIR, "Assets", "FBX", "generated")
TEX_SRC_DIR = os.path.join(GEN_DIR, "Textures")
TEX_DEST_ROOT = "/Game/Textures"

EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

# ---------------------------------------------------------------------------
# Destination by name
# ---------------------------------------------------------------------------
# (fnmatch pattern, destination folder) - first match wins.
DESTINATIONS = [
    ("SM_Ship_*_UniqueUV", "/Game/Assets/Ships"),           # baked unique-UV hull prototypes
    ("SM_StarDome*",       "/Game/Meshes/Environment"),     # the level's dome actor uses this copy
    ("SM_StationModule_*", "/AdastreaShips/Meshes/Station"),
    ("SM_Station_*",       "/AdastreaShips/Meshes/Station"),
    ("SM_Int_*",           "/AdastreaShips/Meshes/Interiors"),
    ("SM_Ship_*",          "/AdastreaShips/Meshes/Ships"),
    ("*",                  "/AdastreaShips/Meshes/Objects"),
]

# ---------------------------------------------------------------------------
# Slot name -> material
# ---------------------------------------------------------------------------
# Explicit aliases for slot names that don't equal a material asset name.
SLOT_ALIASES = {
    "M_Interior_Eng":     "/AdastreaShips/Materials/Interiors/MI_Interior_Eng",
    "M_Interior_Cockpit": "/AdastreaShips/Materials/Interiors/MI_Interior_Cockpit",
    "M_Interior_Hab":     "/AdastreaShips/Materials/Interiors/MI_Interior_Hab",
}
# (mesh name, slot) -> material, checked before everything else.
MESH_SLOT_OVERRIDES = {
    ("SM_Int_Standard_Corridor", "M_Interior_Hab"): "/AdastreaShips/Materials/Interiors/MI_Interior_Corridor",
    ("SM_Int_Standard_Airlock", "M_Interior_Hab"):  "/AdastreaShips/Materials/Interiors/MI_Interior_Corridor",
    ("SM_Ship_Battleship_01_Assembled_UniqueUV", "M_Assembled"): "/Game/Materials/M_Battleship_Hull_Unique",
    ("SM_Ship_Corvette_01_Assembled_UniqueUV", "M_Assembled"):   "/Game/Materials/M_Corvette_Hull_Unique",
}
# Otherwise: a material asset with the slot's exact name in one of these folders.
MATERIAL_SEARCH_DIRS = [
    "/AdastreaShips/Materials/Interiors",
    "/Game/Materials",
    "/AdastreaShips/Materials/Master",
    "/AdastreaShips/Materials/Instances",
    "/Game/Materials/Asteroids",
]

WORLD_GRID = "WorldGridMaterial"

DRY_RUN = False


def log(msg):
    # unreal.log only: print() is also routed to LogPython and would duplicate lines.
    unreal.log("[art-gap] " + msg)


MOUNT_ROOTS = {
    "/Game/": os.path.join(PROJECT_DIR, "Content"),
    "/AdastreaShips/": os.path.join(PROJECT_DIR, "Plugins", "AdastreaShips", "Content"),
}


def asset_exists(path):
    """does_asset_exist, falling back to the .uasset on disk.

    In commandlet runs the asset registry can report False for existing plugin assets
    (seen with /AdastreaShips/Materials/Interiors/M_Int_Viewport), which made
    create_asset collide with the real package. Checking the file avoids both that and
    the warning spam of speculative load_asset() calls on missing paths.
    """
    if EAL.does_asset_exist(path):
        return True
    for mount, root in MOUNT_ROOTS.items():
        if path.startswith(mount):
            rel = path[len(mount):].split(".")[0]
            return os.path.exists(os.path.join(root, *rel.split("/")) + ".uasset")
    return False


def ensure_dir(path):
    if not EAL.does_directory_exist(path):
        EAL.make_directory(path)


def destination_for(name):
    for pattern, dest in DESTINATIONS:
        if fnmatch.fnmatch(name, pattern):
            return dest
    return DESTINATIONS[-1][1]


def fbx_has_ucx(fbx_path):
    """UCX_ node names are stored as plain strings in both binary and ASCII FBX."""
    with open(fbx_path, "rb") as fh:
        return b"UCX_" in fh.read()


# Per-mesh instance overrides: for a slot "M_<Base>" on mesh "SM_StationModule_<Short>_01",
# a material instance "MI_<Base>_<Short>" (in PER_MESH_MI_DIR) wins over the shared
# material - used for the per-domain accent colours (M_StationModule_Accent).
PER_MESH_MI_DIR = "/Game/Materials/StationModuleAccents"


def mesh_short_name(mesh_name):
    name = str(mesh_name)
    if name.startswith("SM_StationModule_"):
        name = name[len("SM_StationModule_"):]
        if name.endswith("_01"):
            name = name[:-3]
        return name
    return None


def resolve_slot_material(slot_name, mesh_name=None):
    slot = str(slot_name)
    override = MESH_SLOT_OVERRIDES.get((str(mesh_name), slot)) if mesh_name else None
    if override and asset_exists(override):
        return unreal.load_asset(override)
    short = mesh_short_name(mesh_name) if mesh_name else None
    if short and slot.startswith("M_"):
        mi_path = "%s/MI_%s_%s" % (PER_MESH_MI_DIR, slot[2:], short)
        if asset_exists(mi_path):
            return unreal.load_asset(mi_path)
    path = SLOT_ALIASES.get(slot)
    if path and asset_exists(path):
        return unreal.load_asset(path)
    for folder in MATERIAL_SEARCH_DIRS:
        # A textured instance "MI_<X>" beats the legacy flat material "M_<X>".
        if slot.startswith("M_"):
            mi = folder + "/MI_" + slot[2:]
            if asset_exists(mi):
                return unreal.load_asset(mi)
        path = folder + "/" + slot
        if asset_exists(path):
            return unreal.load_asset(path)
    return None


def assign_materials_by_slot(mesh, previous=None):
    """Bind each slot to the material of the same name; keep previous real material if none."""
    previous = previous or {}
    changed = []
    for index, sm in enumerate(mesh.static_materials):
        slot = str(sm.material_slot_name)
        mat = resolve_slot_material(slot, mesh.get_name())
        if mat is None:
            prev = previous.get(slot)
            if prev is not None and prev.get_name() != WORLD_GRID:
                mat = prev
        if mat is None:
            log("   slot %s: no material named %s found - left as %s" % (
                slot, slot, sm.material_interface.get_name() if sm.material_interface else None))
            continue
        cur = sm.material_interface
        if cur is None or cur.get_path_name() != mat.get_path_name():
            mesh.set_material(index, mat)
            changed.append("%s->%s" % (slot, mat.get_name()))
    return changed


def collision_summary(mesh):
    for lib_name in ("StaticMeshEditorSubsystem", "EditorStaticMeshLibrary"):
        lib = None
        if lib_name == "StaticMeshEditorSubsystem":
            try:
                lib = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
            except Exception:
                lib = None
        else:
            lib = getattr(unreal, lib_name, None)
        if lib is None:
            continue
        try:
            return "convex=%d simple=%d" % (lib.get_convex_collision_count(mesh), lib.get_simple_collision_count(mesh))
        except Exception:
            continue
    return "collision=?"


# ---------------------------------------------------------------------------
# Mesh import
# ---------------------------------------------------------------------------
def import_mesh(name):
    base = name[:-4] if name.lower().endswith(".fbx") else name
    src = os.path.join(GEN_DIR, base + ".fbx")
    if not os.path.exists(src):
        log("! missing FBX %s" % src)
        return False
    dest = destination_for(base)
    asset_path = dest + "/" + base
    has_ucx = fbx_has_ucx(src)
    existed = asset_exists(asset_path)
    previous = {}
    if existed:
        old = unreal.load_asset(asset_path)
        if old:
            previous = {str(s.material_slot_name): s.material_interface for s in old.static_materials}
    log("%s %s -> %s (UCX collision: %s)" % ("reimport" if existed else "import", base, dest, has_ucx))
    if DRY_RUN:
        return True

    ensure_dir(dest)
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest
    task.destination_name = base
    task.replace_existing = True
    task.replace_existing_settings = True
    task.automated = True
    task.save = False  # saved below, after materials are bound

    options = unreal.FbxImportUI()
    options.import_as_skeletal = False
    options.import_mesh = True
    options.import_animations = False
    options.import_materials = False
    options.import_textures = False
    options.create_physics_asset = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    sm_data = unreal.FbxStaticMeshImportData()
    sm_data.set_editor_property("import_translation", unreal.Vector(0, 0, 0))
    sm_data.set_editor_property("import_rotation", unreal.Rotator(0, 0, 0))
    sm_data.set_editor_property("import_uniform_scale", 1.0)
    sm_data.set_editor_property("convert_scene", True)
    sm_data.set_editor_property("combine_meshes", True)
    sm_data.set_editor_property("generate_lightmap_u_vs", True)
    # Keep this ON even when the FBX has UCX_ hulls. UE 5.8 routes FbxImportUI through
    # Interchange, which maps auto_generate_collision onto the pipeline's "import
    # collision" switch: OFF silently dropped every authored UCX hull (modules came in
    # with zero collision). With it ON, Interchange (and the legacy importer) use the
    # UCX_ hulls by name and only generate a hull when the FBX has none.
    sm_data.set_editor_property("auto_generate_collision", True)
    try:
        sm_data.set_editor_property("one_convex_hull_per_ucx", True)
    except Exception:
        pass
    try:
        sm_data.set_editor_property("build_nanite", False)
    except Exception:
        pass
    options.static_mesh_import_data = sm_data
    task.options = options
    ASSET_TOOLS.import_asset_tasks([task])

    mesh = unreal.load_asset(asset_path)
    if not mesh:
        log("! import FAILED for %s" % base)
        return False
    changed = assign_materials_by_slot(mesh, previous)
    EAL.save_asset(asset_path, only_if_is_dirty=False)
    b = mesh.get_bounds().box_extent
    log("   OK %s  extent=(%.0f, %.0f, %.0f)cm  %s  slots=%s" % (
        base, b.x, b.y, b.z, collision_summary(mesh),
        [str(s.material_slot_name) + ":" + (s.material_interface.get_name() if s.material_interface else "None")
         for s in mesh.static_materials]))
    if changed:
        log("   materials bound: " + ", ".join(changed))
    # Station modules live on a 400 cm grid (largest footprint 3x2x1 = 1200x800x400 cm,
    # half-extent <= 600). Anything far beyond that is a unit-scale export bug upstream.
    if base.startswith("SM_StationModule_") and max(b.x, b.y, b.z) > 1500.0:
        unreal.log_warning("[art-gap] WARNING %s half-extent %.0f cm is far above the 400 cm grid "
                           "footprint - likely a 100x unit-scale export issue in the source FBX" % (base, max(b.x, b.y, b.z)))
    return True


# ---------------------------------------------------------------------------
# Textures
# ---------------------------------------------------------------------------
MAP_SUFFIXES = ["D", "N", "R", "M", "AO", "E", "MASK", "SKIN"]


def apply_texture_rules(tex, suffix):
    if suffix == "N":
        tex.set_editor_property("srgb", False)
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        tex.set_editor_property("flip_green_channel", False)
    elif suffix in ("R", "M", "AO", "MASK"):
        tex.set_editor_property("srgb", False)
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
    else:
        tex.set_editor_property("srgb", True)
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)


def import_texture_set(texset, src_dir=TEX_SRC_DIR, dest_root=TEX_DEST_ROOT):
    folder = dest_root + "/" + texset
    out = {}
    for suffix in MAP_SUFFIXES:
        png = os.path.join(src_dir, "%s_%s.png" % (texset, suffix))
        name = "%s_%s" % (texset, suffix)
        if not os.path.exists(png):
            continue
        log("texture %s -> %s" % (name, folder))
        if DRY_RUN:
            continue
        ensure_dir(folder)
        task = unreal.AssetImportTask()
        task.filename = png
        task.destination_path = folder
        task.destination_name = name
        task.replace_existing = True
        task.automated = True
        task.save = False
        ASSET_TOOLS.import_asset_tasks([task])
        tex = unreal.load_asset(folder + "/" + name)
        if tex:
            apply_texture_rules(tex, suffix)
            EAL.save_asset(folder + "/" + name, only_if_is_dirty=False)
            out[suffix] = tex
    return out


def load_texture_set(texset, dest_root=TEX_DEST_ROOT):
    out = {}
    for suffix in MAP_SUFFIXES:
        path = "%s/%s/%s_%s" % (dest_root, texset, texset, suffix)
        if asset_exists(path):
            out[suffix] = unreal.load_asset(path)
    return out


# ---------------------------------------------------------------------------
# Materials
# ---------------------------------------------------------------------------
SAMPLER_FOR_SUFFIX = {
    "N": unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
    "R": unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
    "M": unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
    "AO": unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
    "D": unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
    "E": unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
}


def sampler_for_texture(tex):
    comp = tex.get_editor_property("compression_settings")
    if comp == unreal.TextureCompressionSettings.TC_NORMALMAP:
        return unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    if comp == unreal.TextureCompressionSettings.TC_MASKS:
        return unreal.MaterialSamplerType.SAMPLERTYPE_MASKS
    if comp == unreal.TextureCompressionSettings.TC_GRAYSCALE:
        return (unreal.MaterialSamplerType.SAMPLERTYPE_GRAYSCALE if tex.get_editor_property("srgb")
                else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE)
    return (unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if tex.get_editor_property("srgb")
            else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)


def fix_sampler_types(material):
    """Make every directly-connected TextureSample's sampler match its texture's settings."""
    fixed = []
    props = ["MP_BASE_COLOR", "MP_NORMAL", "MP_ROUGHNESS", "MP_METALLIC", "MP_EMISSIVE_COLOR",
             "MP_AMBIENT_OCCLUSION", "MP_SPECULAR", "MP_OPACITY"]
    for prop in props:
        try:
            node = MEL.get_material_property_input_node(material, getattr(unreal.MaterialProperty, prop))
        except Exception:
            node = None
        if not isinstance(node, unreal.MaterialExpressionTextureSample):
            continue
        tex = node.get_editor_property("texture")
        if not tex:
            continue
        want = sampler_for_texture(tex)
        if node.get_editor_property("sampler_type") != want:
            node.set_editor_property("sampler_type", want)
            fixed.append("%s:%s" % (prop, tex.get_name()))
    if fixed and not DRY_RUN:
        MEL.recompile_material(material)
        EAL.save_asset(material.get_path_name().split(".")[0], only_if_is_dirty=False)
    return fixed


def get_or_create_material(folder, name):
    path = folder + "/" + name
    # Load first: EditorAssetLibrary.does_asset_exist can report False for existing
    # plugin-content assets in commandlet runs before the registry scan settles.
    mat = unreal.load_asset(path) if asset_exists(path) else None
    if mat is None:
        ensure_dir(folder)
        mat = ASSET_TOOLS.create_asset(name, folder, unreal.Material, unreal.MaterialFactoryNew())
    return mat, path


def build_tiled_pbr_material(folder, name, texset, tiling=4.0, tint=(1.0, 1.0, 1.0), emissive_strength=1.0):
    """Opaque DefaultLit PBR material from a texture set, UV-tiled via a 'Tiling' scalar param."""
    tex = load_texture_set(texset)
    if "D" not in tex:
        log("! texture set %s not found under %s" % (texset, TEX_DEST_ROOT))
        return None
    log("material %s/%s from %s (tiling %.1f)" % (folder, name, texset, tiling))
    if DRY_RUN:
        return None
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)

    uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
    til = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1200, 150)
    til.set_editor_property("parameter_name", "Tiling")
    til.set_editor_property("default_value", tiling)
    mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -1000, 60)
    MEL.connect_material_expressions(uv, "", mul, "A")
    MEL.connect_material_expressions(til, "", mul, "B")

    def sample(suffix, y):
        node = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -700, y)
        node.set_editor_property("texture", tex[suffix])
        node.set_editor_property("sampler_type", sampler_for_texture(tex[suffix]))
        MEL.connect_material_expressions(mul, "", node, "UVs")
        return node

    y = -300
    d = sample("D", y)
    tint_node = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, y - 220)
    tint_node.set_editor_property("parameter_name", "Tint")
    tint_node.set_editor_property("default_value", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    base_mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -350, y)
    MEL.connect_material_expressions(d, "RGB", base_mul, "A")
    MEL.connect_material_expressions(tint_node, "", base_mul, "B")
    MEL.connect_material_property(base_mul, "", unreal.MaterialProperty.MP_BASE_COLOR)
    y += 260
    for suffix, out, prop in (("N", "RGB", "MP_NORMAL"), ("R", "R", "MP_ROUGHNESS"),
                              ("M", "R", "MP_METALLIC"), ("AO", "R", "MP_AMBIENT_OCCLUSION")):
        if suffix in tex:
            node = sample(suffix, y)
            MEL.connect_material_property(node, out, getattr(unreal.MaterialProperty, prop))
            y += 260
    if "E" in tex and emissive_strength > 0.0:
        e = sample("E", y)
        es = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, y + 220)
        es.set_editor_property("parameter_name", "EmissiveStrength")
        es.set_editor_property("default_value", emissive_strength)
        em = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -350, y)
        MEL.connect_material_expressions(e, "RGB", em, "A")
        MEL.connect_material_expressions(es, "", em, "B")
        MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.layout_material_expressions(mat)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)
    return mat


def build_viewport_glass(path="/AdastreaShips/Materials/Interiors/M_Int_Viewport"):
    """Rebuild the bridge viewport material in place as lit translucent glass.

    Translucent + Surface ForwardShading lighting (real specular highlights without
    Lumen), index-of-refraction refraction (a subtle distortion pass; works on SM5
    deferred), fresnel-driven opacity so the pane is nearly clear head-on and reads as
    glass at glancing angles, plus a faint fresnel rim tint so it's visible in dark
    cabins. Two-sided so it reads from either face of the single-sheet pane.
    """
    folder, name = path.rsplit("/", 1)
    log("material %s (translucent glass)" % path)
    if DRY_RUN:
        return None
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", True)
    mat.set_editor_property("translucency_lighting_mode",
                            unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
    try:
        mat.set_editor_property("refraction_method", unreal.RefractionMode.RM_INDEX_OF_REFRACTION)
    except Exception as exc:
        log("  (refraction_method not set: %s)" % exc)

    def const(v, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, x, y)
        n.set_editor_property("r", v)
        return n

    def vec_param(pname, rgb, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
        n.set_editor_property("parameter_name", pname)
        n.set_editor_property("default_value", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
        return n

    def scalar_param(pname, v, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
        n.set_editor_property("parameter_name", pname)
        n.set_editor_property("default_value", v)
        return n

    tint = vec_param("GlassTint", (0.02, 0.05, 0.07), -600, -300)
    MEL.connect_material_property(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(const(0.0, -300, -150), "", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(scalar_param("Roughness", 0.04, -600, -100), "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(const(1.0, -300, -50), "", unreal.MaterialProperty.MP_SPECULAR)

    fres = MEL.create_material_expression(mat, unreal.MaterialExpressionFresnel, -900, 100)
    fres.set_editor_property("exponent", 4.0)
    fres.set_editor_property("base_reflect_fraction", 0.04)
    lerp = MEL.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, 100)
    MEL.connect_material_expressions(scalar_param("OpacityFacing", 0.12, -700, 50), "", lerp, "A")
    MEL.connect_material_expressions(scalar_param("OpacityGrazing", 0.6, -700, 150), "", lerp, "B")
    MEL.connect_material_expressions(fres, "", lerp, "Alpha")
    MEL.connect_material_property(lerp, "", unreal.MaterialProperty.MP_OPACITY)

    rim = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -400, 300)
    MEL.connect_material_expressions(vec_param("RimGlow", (0.0, 0.02, 0.03), -700, 300), "", rim, "A")
    MEL.connect_material_expressions(fres, "", rim, "B")
    MEL.connect_material_property(rim, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    MEL.connect_material_property(scalar_param("IOR", 1.04, -600, 450), "", unreal.MaterialProperty.MP_REFRACTION)
    MEL.layout_material_expressions(mat)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)
    return mat


# Per-domain accent colours - must match DOMAIN_ACCENT in
# Tools/generate_station_lab_defence_modules.py (Modules-B's preview colours).
DOMAIN_ACCENT = {
    "ScienceLab": (0.35, 0.80, 1.00), "ResearchLab": (0.60, 0.85, 1.00),
    "PhysicsLab": (0.55, 0.45, 1.00), "IonPropulsionLab": (0.40, 0.55, 1.00),
    "MaterialsLab": (1.00, 0.50, 0.15), "GravMaterialsLab": (0.80, 0.30, 1.00),
    "ElectronicsLab": (0.20, 1.00, 0.70), "EncryptionLab": (0.20, 1.00, 0.35),
    "OptronicsLab": (1.00, 0.95, 0.55),
    "WeaponsLab": (1.00, 0.25, 0.15), "ProjectileWeaponsLab": (1.00, 0.55, 0.10),
    "BeamWeaponsLab": (1.00, 0.20, 0.50),
    "BiologyLab": (0.45, 1.00, 0.30), "CyberneticsLab": (0.20, 0.95, 0.90),
    "Turret": (1.00, 0.18, 0.12), "TurretHead": (1.00, 0.18, 0.12),
    "ShieldGenerator": (0.30, 0.70, 1.00),
}
ACCENT_MASTER = "/Game/Materials/M_StationModule_Accent"


def build_accent_materials():
    """M_StationModule_Accent (opaque lit emissive trim, 'AccentColor'/'EmissiveStrength'
    params) + one MI_StationModule_Accent_<Domain> per DOMAIN_ACCENT entry."""
    folder, name = ACCENT_MASTER.rsplit("/", 1)
    log("material %s + %d accent instances" % (ACCENT_MASTER, len(DOMAIN_ACCENT)))
    if DRY_RUN:
        return
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    color = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, -100)
    color.set_editor_property("parameter_name", "AccentColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.6, 0.85, 1.0, 1.0))
    strength = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -700, 100)
    strength.set_editor_property("parameter_name", "EmissiveStrength")
    strength.set_editor_property("default_value", 6.0)
    base = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -350, -200)
    MEL.connect_material_expressions(color, "", base, "A")
    base.set_editor_property("const_b", 0.25)
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    emis = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -350, 0)
    MEL.connect_material_expressions(color, "", emis, "A")
    MEL.connect_material_expressions(strength, "", emis, "B")
    MEL.connect_material_property(emis, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -350, 200)
    rough.set_editor_property("r", 0.3)
    MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)

    ensure_dir(PER_MESH_MI_DIR)
    for short, rgb in DOMAIN_ACCENT.items():
        mi_name = "MI_StationModule_Accent_" + short
        mi_path = PER_MESH_MI_DIR + "/" + mi_name
        mi = unreal.load_asset(mi_path) if asset_exists(mi_path) else None
        if mi is None:
            mi = ASSET_TOOLS.create_asset(mi_name, PER_MESH_MI_DIR, unreal.MaterialInstanceConstant,
                                          unreal.MaterialInstanceConstantFactoryNew())
        MEL.set_material_instance_parent(mi, mat)
        MEL.set_material_instance_vector_parameter_value(mi, "AccentColor", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
        EAL.save_asset(mi_path, only_if_is_dirty=False)


# Family materials for the shared shells' slots (they were imported onto WorldGridMaterial).
SHELL_SLOT_FIX = [
    "/AdastreaShips/Meshes/Station/SM_StationModule_Shell_Standard",
    "/AdastreaShips/Meshes/Station/SM_StationModule_Shell_Large",
    "/AdastreaShips/Meshes/Station/SM_StationModule_Shell_ConnectorThin",
    "/AdastreaShips/Meshes/Station/SM_StationModule_Shell_SolarArray",
    "/AdastreaShips/Meshes/Station/SM_StationModule_DockingBay_01",
    "/AdastreaShips/Meshes/Station/SM_StationModule_CargoBay_01",
    "/AdastreaShips/Meshes/Station/SM_StationModule_Market_01",
]
STATION_MODULE_MATERIALS = ["Shell", "Connector", "Utility", "Defence", "SolarArray"]


def run_fixes():
    # 1) Station-module family materials: sampler types must match texture compression
    #    (M_StationModule_Connector was missed by 6385d477 -> failed SM6 compile).
    for fam in STATION_MODULE_MATERIALS:
        path = "/Game/Materials/M_StationModule_" + fam
        mat = unreal.load_asset(path)
        if not mat:
            log("! missing %s" % path)
            continue
        fixed = fix_sampler_types(mat)
        log("sampler fix %s: %s" % (path, fixed or "already correct"))

    # 2) Bind family materials on the station meshes by slot name.
    for path in SHELL_SLOT_FIX:
        mesh = unreal.load_asset(path)
        if not mesh:
            log("! missing %s" % path)
            continue
        changed = [] if DRY_RUN else assign_materials_by_slot(mesh)
        if changed and not DRY_RUN:
            EAL.save_asset(path, only_if_is_dirty=False)
        log("slots %s: %s" % (path.rsplit("/", 1)[1], changed or "already bound"))

    # 3) Engineering bay: now MI_Interior_Eng from the Interiors library (--interiors).
    #    The interim T_Engine-based M_Int_Eng is superseded and deleted if present.
    if not DRY_RUN and EAL.does_asset_exist("/AdastreaShips/Materials/Interiors/M_Int_Eng"):
        EAL.delete_asset("/AdastreaShips/Materials/Interiors/M_Int_Eng")
        log("deleted superseded /AdastreaShips/Materials/Interiors/M_Int_Eng")

    # 4) Bridge viewport glass, and bind it on the viewport meshes' own slots so the
    #    asset default is right even outside ASpaceshipInterior::ApplyInteriorMaterials.
    build_library_glass()
    # List the .uasset files on disk: the registry can be incomplete in commandlet runs.
    int_dir = os.path.join(MOUNT_ROOTS["/AdastreaShips/"], "Meshes", "Interiors")
    for fname in sorted(os.listdir(int_dir)):
        name = fname[:-7] if fname.endswith(".uasset") else None
        if not name or not name.endswith("_Viewport"):
            continue
        path = "/AdastreaShips/Meshes/Interiors/" + name
        mesh = unreal.load_asset(path)
        if mesh and not DRY_RUN:
            changed = assign_materials_by_slot(mesh)
            if changed:
                EAL.save_asset(path, only_if_is_dirty=False)
            log("slots %s: %s" % (name, changed or "already bound"))



# ===========================================================================
# Interiors texture library -> materials (Assets/Textures/generated/interiors/
# MATERIAL_MAPPING.md). One master, M_IntSurface_Oriented, covers every case:
#   * 4 texture sets (Floor / WallLo / WallUp / Ceil) picked by surface normal
#     (|n.z| > 0.7 floor/ceiling, else wall; wall lower below 1 tile = 200 cm
#     above the floor, upper above) - a single-set material just uses the same
#     set in all four slots.
#   * UseWorldAligned = 0 (ship kit meshes): box projection in OBJECT space with
#     UVPerUnit = 1 tile per local unit (1/20000 for the 100x-authored SM_Int_*
#     meshes, i.e. 200 design-cm), so wall bands are floor-anchored regardless of
#     how smart_uv laid the islands out, and it survives ConfigureInterior's
#     runtime rescale.
#   * UseWorldAligned = 1 (AStationInterior's scaled engine cubes): the same box
#     projection in WORLD space, TextureSize cm per tile, FloorZ set per MID.
# Deviations from MATERIAL_MAPPING.md (documented in the report): mesh path uses
# object-space projection instead of TexCoord*Tiling; the tangent-space normal is
# attenuated by NormalStrength (default 0.35) since projected UVs don't match mesh
# tangents; M_IntSurface_Master / M_IntGrate_Masked were not built separately
# (single-set MIs of the oriented master; EngGrate used opaque as the doc allows).
# ===========================================================================
INT_TEX_SRC = os.path.join(PROJECT_DIR, "Assets", "Textures", "generated", "interiors")
INT_TEX_ROOT = "/Game/Textures/Interiors"
INT_MAT_DIR = "/AdastreaShips/Materials/Interiors"
INT_MASTER = INT_MAT_DIR + "/M_IntSurface_Oriented"
STN_MI_DIR = INT_MAT_DIR + "/Station"
INT_SETS = ["ShipDeck", "ShipWall", "ShipWallUpper", "ShipCeiling", "EngWall", "EngWallUpper",
            "EngGrate", "HabWall", "HabFloor", "StnFloor", "StnWall", "StnWallUpper", "StnCeiling",
            "Console", "Hazard", "Glass"]
SURF_SLOTS = ["Floor", "WallLo", "WallUp", "Ceil"]
SURF_MAPS = ["D", "N", "R", "M", "AO", "E"]
KIT_UV_PER_UNIT = 1.0 / 20000.0   # SM_Int_* are authored 100x: 200 design cm = 20000 uu


def int_tex(set_name, suffix):
    t = "T_Int_" + set_name
    return unreal.load_asset("%s/%s/%s_%s" % (INT_TEX_ROOT, t, t, suffix))


def _custom(mat, x, y, code, inputs, out_type, desc):
    node = MEL.create_material_expression(mat, unreal.MaterialExpressionCustom, x, y)
    node.set_editor_property("code", code)
    node.set_editor_property("description", desc)
    node.set_editor_property("output_type", out_type)
    ins = []
    for name in inputs:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", name)
        ins.append(ci)
    node.set_editor_property("inputs", ins)
    return node


def _scalar(mat, name, value, x, y):
    n = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", value)
    return n


def build_oriented_master():
    log("material %s (oriented 4-set interior master)" % INT_MASTER)
    if DRY_RUN:
        return None
    folder, name = INT_MASTER.rsplit("/", 1)
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)
    F2 = unreal.CustomMaterialOutputType.CMOT_FLOAT2
    F1 = unreal.CustomMaterialOutputType.CMOT_FLOAT1
    F3 = unreal.CustomMaterialOutputType.CMOT_FLOAT3
    F4 = unreal.CustomMaterialOutputType.CMOT_FLOAT4

    # --- shared inputs ---
    wp = MEL.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -3000, -400)
    lp = MEL.create_material_expression(mat, unreal.MaterialExpressionLocalPosition, -3000, -300)
    nws = MEL.create_material_expression(mat, unreal.MaterialExpressionVertexNormalWS, -3000, -200)
    nls = MEL.create_material_expression(mat, unreal.MaterialExpressionTransform, -2800, -200)
    nls.set_editor_property("transform_source_type", unreal.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_WORLD)
    nls.set_editor_property("transform_type", unreal.MaterialVectorCoordTransform.TRANSFORM_LOCAL)
    MEL.connect_material_expressions(nws, "", nls, "")
    p = {
        "UseWA": _scalar(mat, "UseWorldAligned", 0.0, -3000, 0),
        "TexSize": _scalar(mat, "TextureSize", 200.0, -3000, 80),
        "FloorZ": _scalar(mat, "FloorZ", 0.0, -3000, 160),
        "LocalFloorZ": _scalar(mat, "LocalFloorZ", 0.0, -3000, 240),
        "UVPerUnit": _scalar(mat, "UVPerUnit", KIT_UV_PER_UNIT, -3000, 320),
        "WallSplit": _scalar(mat, "WallSplit", 1.0, -3000, 400),
        "TF": _scalar(mat, "TilingFloor", 1.0, -3000, 480),
        "TW": _scalar(mat, "TilingWall", 1.0, -3000, 560),
        "TC": _scalar(mat, "TilingCeiling", 1.0, -3000, 640),
    }

    def wire(node, names):
        for nm in names:
            src = {"WP": wp, "LP": lp, "NW": nws, "NL": nls}.get(nm) or p.get(nm)
            MEL.connect_material_expressions(src, "", node, nm)

    PRE = ("float3 P = (UseWA > 0.5) ? WP : LP; "
           "float3 n = normalize((UseWA > 0.5) ? NW : NL); "
           "float S = (UseWA > 0.5) ? (1.0 / max(TexSize, 1.0)) : UVPerUnit; "
           "float F = (UseWA > 0.5) ? FloorZ : LocalFloorZ; ")
    base_in = ["WP", "LP", "NW", "NL", "UseWA", "TexSize", "FloorZ", "LocalFloorZ", "UVPerUnit"]
    uv_floor = _custom(mat, -2500, -600, PRE + "return P.xy * S * Tiling;", base_in + ["Tiling"], F2, "UV floor")
    uv_ceil = _custom(mat, -2500, -300, PRE + "return P.xy * S * Tiling;", base_in + ["Tiling"], F2, "UV ceiling")
    uv_wall = _custom(mat, -2500, 0, PRE +
                      "float u = (abs(n.x) > abs(n.y)) ? P.y : P.x; "
                      "return float2(u * S * Tiling, -(P.z - F) * S * Tiling);",
                      base_in + ["Tiling"], F2, "UV wall (floor-anchored V)")
    weights = _custom(mat, -2500, 300, PRE +
                      "float fl = saturate((n.z - 0.675) / 0.05); "
                      "float ce = saturate((-n.z - 0.675) / 0.05); "
                      "float wall = saturate(1.0 - fl - ce); "
                      "float h = (P.z - F) * S; "
                      "float up = saturate((h - WallSplit) / 0.02); "
                      "return float4(fl, ce, wall * (1.0 - up), wall * up);",
                      base_in + ["WallSplit"], F4, "Surface weights (floor, ceil, wallLo, wallUp)")
    for node in (uv_floor, uv_ceil, uv_wall, weights):
        wire(node, base_in)
    MEL.connect_material_expressions(p["TF"], "", uv_floor, "Tiling")
    MEL.connect_material_expressions(p["TC"], "", uv_ceil, "Tiling")
    MEL.connect_material_expressions(p["TW"], "", uv_wall, "Tiling")
    MEL.connect_material_expressions(p["WallSplit"], "", weights, "WallSplit")

    defaults = {"Floor": "ShipDeck", "WallLo": "ShipWall", "WallUp": "ShipWallUpper", "Ceil": "ShipCeiling"}
    uv_for = {"Floor": uv_floor, "Ceil": uv_ceil, "WallLo": uv_wall, "WallUp": uv_wall}
    samples = {}
    y = -1400
    for slot in SURF_SLOTS:
        for m in SURF_MAPS:
            tex = int_tex(defaults[slot], m)
            node = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -1800, y)
            node.set_editor_property("parameter_name", "%s_%s" % (slot, m))
            node.set_editor_property("texture", tex)
            node.set_editor_property("sampler_type", sampler_for_texture(tex))
            node.set_editor_property("sampler_source", unreal.SamplerSourceMode.SSM_WRAP_WORLD_GROUP_SETTINGS)
            MEL.connect_material_expressions(uv_for[slot], "", node, "UVs")
            samples[(slot, m)] = node
            y += 120

    # Blend: order in the weight vector is (Floor, Ceil, WallLo, WallUp).
    order = ["Floor", "Ceil", "WallLo", "WallUp"]
    blend_code = "return A * W.x + B * W.y + C * W.z + D * W.w;"

    def blend(m, out_type, pin, x, y):
        node = _custom(mat, x, y, blend_code, ["A", "B", "C", "D", "W"], out_type, "Blend " + m)
        for letter, slot in zip("ABCD", order):
            MEL.connect_material_expressions(samples[(slot, m)], pin, node, letter)
        MEL.connect_material_expressions(weights, "", node, "W")
        return node

    d = blend("D", F3, "RGB", -1000, -600)
    tint = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -1000, -750)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1, 1, 1, 1))
    dm = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -700, -600)
    MEL.connect_material_expressions(d, "", dm, "A")
    MEL.connect_material_expressions(tint, "", dm, "B")
    MEL.connect_material_property(dm, "", unreal.MaterialProperty.MP_BASE_COLOR)

    nrm = blend("N", F3, "RGB", -1000, -400)
    flat = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -1000, -300)
    flat.set_editor_property("constant", unreal.LinearColor(0, 0, 1, 1))
    nl = MEL.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -700, -400)
    MEL.connect_material_expressions(flat, "", nl, "A")
    MEL.connect_material_expressions(nrm, "", nl, "B")
    MEL.connect_material_expressions(_scalar(mat, "NormalStrength", 0.35, -1000, -250), "", nl, "Alpha")
    MEL.connect_material_property(nl, "", unreal.MaterialProperty.MP_NORMAL)

    MEL.connect_material_property(blend("R", F1, "R", -1000, -150), "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(blend("M", F1, "R", -1000, 0), "", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(blend("AO", F1, "R", -1000, 150), "", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)

    em = _custom(mat, -1000, 350,
                 "return A * W.x * SF + B * W.y * SC + (C * W.z + D * W.w) * SW;",
                 ["A", "B", "C", "D", "W", "SF", "SC", "SW"], F3, "Emissive")
    for letter, slot in zip("ABCD", order):
        MEL.connect_material_expressions(samples[(slot, "E")], "RGB", em, letter)
    MEL.connect_material_expressions(weights, "", em, "W")
    MEL.connect_material_expressions(_scalar(mat, "EmissiveFloor", 0.0, -1300, 450), "", em, "SF")
    MEL.connect_material_expressions(_scalar(mat, "EmissiveCeiling", 0.0, -1300, 530), "", em, "SC")
    MEL.connect_material_expressions(_scalar(mat, "EmissiveWall", 0.0, -1300, 610), "", em, "SW")
    MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)
    return mat


def make_surface_mi(folder, name, parent, sets, scalars=None, tint=None):
    """sets: dict slot->set name (missing slots fall back to 'Floor' then any)."""
    mi_path = folder + "/" + name
    log("  MI %s: %s %s" % (name, sets, scalars or ""))
    if DRY_RUN:
        return
    ensure_dir(folder)
    mi = unreal.load_asset(mi_path) if asset_exists(mi_path) else None
    if mi is None:
        mi = ASSET_TOOLS.create_asset(name, folder, unreal.MaterialInstanceConstant,
                                      unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, parent)
    first = sets.get("Floor") or next(iter(sets.values()))
    for slot in SURF_SLOTS:
        set_name = sets.get(slot) or (sets.get("WallLo") if slot == "WallUp" else None) or first
        for m in SURF_MAPS:
            tex = int_tex(set_name, m)
            if tex:
                MEL.set_material_instance_texture_parameter_value(mi, "%s_%s" % (slot, m), tex)
    for k, v in (scalars or {}).items():
        MEL.set_material_instance_scalar_parameter_value(mi, k, float(v))
    if tint:
        MEL.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    EAL.save_asset(mi_path, only_if_is_dirty=False)


def _all(set_name):
    return {s_: set_name for s_ in SURF_SLOTS}


# name -> (sets, scalars, tint). Ship kit = object-space mode (defaults).
KIT_MIS = {
    "MI_Int_Shell":   ({"Floor": "ShipDeck", "WallLo": "ShipWall", "WallUp": "ShipWallUpper", "Ceil": "ShipCeiling"}, {"EmissiveCeiling": 5}, None),
    "MI_Int_Deck":    (_all("ShipDeck"), {}, None),
    "MI_Int_Console": (_all("Console"), {"TilingFloor": 2, "TilingWall": 2, "TilingCeiling": 2,
                                         "EmissiveFloor": 3.5, "EmissiveWall": 3.5, "EmissiveCeiling": 3.5}, None),
    "MI_Int_Stations": ({"Floor": "Console", "WallLo": "ShipWallUpper", "WallUp": "ShipWallUpper", "Ceil": "ShipWallUpper"},
                        {"TilingFloor": 2, "EmissiveFloor": 3.5}, None),
    "MI_Int_Vents":   (_all("EngWallUpper"), {}, (1.0, 0.78, 0.74)),
    "MI_Int_Hatch":   (_all("ShipWallUpper"), {}, (0.6, 0.6, 0.6)),
    "MI_Int_Bunks":   (_all("HabWall"), {}, None),
    "MI_Int_Desks":   (_all("StnWallUpper"), {}, None),
    "MI_Int_Galley":  ({"Floor": "StnFloor", "WallLo": "StnWallUpper", "WallUp": "StnWallUpper", "Ceil": "StnWallUpper"}, {}, None),
    "MI_Int_Mess":    ({"Floor": "HabFloor", "WallLo": "HabWall", "WallUp": "StnWallUpper", "Ceil": "StnWallUpper"}, {}, None),
    "MI_Interior_Eng": ({"Floor": "EngGrate", "WallLo": "EngWall", "WallUp": "EngWallUpper", "Ceil": "ShipCeiling"},
                        {"EmissiveCeiling": 6, "EmissiveWall": 2}, None),
    "MI_Interior_Corridor": ({"Floor": "ShipDeck", "WallLo": "ShipWall", "WallUp": "ShipWallUpper", "Ceil": "ShipCeiling"},
                             {"EmissiveCeiling": 5}, None),
    "MI_Interior_Hab": ({"Floor": "HabFloor", "WallLo": "HabWall", "WallUp": "StnWallUpper", "Ceil": "StnCeiling"},
                        {"EmissiveCeiling": 4.5}, None),
    "MI_Interior_Cockpit": ({"Floor": "Console", "WallLo": "ShipWallUpper", "WallUp": "ShipWallUpper", "Ceil": "ShipWallUpper"},
                            {"TilingFloor": 2, "EmissiveFloor": 3.5}, None),
}
WA = {"UseWorldAligned": 1, "TextureSize": 200, "NormalStrength": 0.25}
STN_MIS = {
    "MI_Stn_Concourse_Floor":   (_all("StnFloor"), {}, None),
    "MI_Stn_Concourse_Ceiling": (_all("StnCeiling"), {"EmissiveFloor": 5, "EmissiveCeiling": 5, "EmissiveWall": 5}, None),
    "MI_Stn_Concourse_Wall":    ({"Floor": "StnWallUpper", "WallLo": "StnWall", "WallUp": "StnWallUpper", "Ceil": "StnWallUpper"},
                                 {"EmissiveWall": 2.5}, None),
    "MI_Stn_Concourse_Pillar":  (_all("StnWallUpper"), {}, None),
    "MI_Stn_Concourse_Hazard":  (_all("Hazard"), {"TextureSize": 100}, None),
    "MI_Stn_Maint_Floor":   (_all("ShipDeck"), {}, None),
    "MI_Stn_Maint_Ceiling": (_all("ShipCeiling"), {"EmissiveFloor": 6, "EmissiveCeiling": 6, "EmissiveWall": 6}, None),
    "MI_Stn_Maint_Wall":    ({"Floor": "EngWallUpper", "WallLo": "EngWall", "WallUp": "EngWallUpper", "Ceil": "EngWallUpper"},
                             {"EmissiveWall": 2}, None),
    "MI_Stn_Maint_Hazard":  (_all("Hazard"), {"TextureSize": 100}, None),
    "MI_Stn_Maint_Grate":   (_all("EngGrate"), {}, None),
    "MI_Stn_Maint_Steel":   (_all("EngWallUpper"), {}, None),
    "MI_Stn_Hab_Floor":   (_all("HabFloor"), {}, None),
    "MI_Stn_Hab_Ceiling": (_all("StnCeiling"), {"EmissiveFloor": 4.5, "EmissiveCeiling": 4.5, "EmissiveWall": 4.5}, None),
    "MI_Stn_Hab_Wall":    ({"Floor": "StnWallUpper", "WallLo": "HabWall", "WallUp": "StnWallUpper", "Ceil": "StnWallUpper"}, {}, None),
    "MI_Stn_Hab_Wood":    (_all("HabWall"), {}, None),
    "MI_Stn_Hab_Fabric":  (_all("HabFloor"), {}, (0.6, 0.7, 0.95)),
}


def build_library_glass(path="/AdastreaShips/Materials/Interiors/M_Int_Viewport"):
    """MATERIAL_MAPPING.md section 5, rebuilt in place on M_Int_Viewport (keeps every
    reference). Translucent, Surface ForwardShading, IOR 1.52, opacity/roughness from
    the Glass _MASK dirt map, normal at 0.3, fresnel opacity boost, faint rim emissive."""
    if not int_tex("Glass", "D"):
        log("! T_Int_Glass not imported - falling back to the untextured glass")
        return build_viewport_glass(path)
    folder, name = path.rsplit("/", 1)
    log("material %s (library glass, T_Int_Glass)" % path)
    if DRY_RUN:
        return None
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    # Doc says Two Sided OFF; kept ON because the viewport panes are single sheets
    # whose normals face outward, so from inside the bridge a one-sided pane culls away.
    mat.set_editor_property("two_sided", True)
    mat.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
    mat.set_editor_property("refraction_method", unreal.RefractionMode.RM_INDEX_OF_REFRACTION)

    def tex_node(m, y):
        t = int_tex("Glass", m)
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -900, y)
        n.set_editor_property("texture", t)
        n.set_editor_property("sampler_type", sampler_for_texture(t))
        return n

    def const(v, x, y):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, x, y)
        n.set_editor_property("r", v)
        return n

    def lerp(a, b, alpha, x, y, alpha_pin="R"):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, x, y)
        MEL.connect_material_expressions(a, "", n, "A")
        MEL.connect_material_expressions(b, "", n, "B")
        MEL.connect_material_expressions(alpha, alpha_pin, n, "Alpha")
        return n

    d = tex_node("D", -500)
    mask = tex_node("MASK", -200)
    nrm = tex_node("N", 100)
    # Darken the dirt tint: at full strength the grey film, lit per-pixel by the bridge
    # lights, read as milky frosted glass and hid the starfield (PR #493 walkthrough).
    dark = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -650, -500)
    MEL.connect_material_expressions(d, "RGB", dark, "A")
    dark.set_editor_property("const_b", 0.2)
    MEL.connect_material_property(dark, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(const(0.0, -500, -350), "", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(const(0.5, -500, -300), "", unreal.MaterialProperty.MP_SPECULAR)
    MEL.connect_material_property(lerp(const(0.03, -700, -250), const(0.45, -700, -200), mask, -400, -220),
                                  "", unreal.MaterialProperty.MP_ROUGHNESS)
    base_op = lerp(const(0.04, -700, -120), const(0.22, -700, -80), mask, -450, -100)
    fres = MEL.create_material_expression(mat, unreal.MaterialExpressionFresnel, -700, 0)
    fres.set_editor_property("exponent", 4.0)
    fmul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -450, 0)
    MEL.connect_material_expressions(fres, "", fmul, "A")
    fmul.set_editor_property("const_b", 0.15)
    add = MEL.create_material_expression(mat, unreal.MaterialExpressionAdd, -300, -50)
    MEL.connect_material_expressions(base_op, "", add, "A")
    MEL.connect_material_expressions(fmul, "", add, "B")
    sat = MEL.create_material_expression(mat, unreal.MaterialExpressionSaturate, -150, -50)
    MEL.connect_material_expressions(add, "", sat, "")
    MEL.connect_material_property(sat, "", unreal.MaterialProperty.MP_OPACITY)
    flat = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -700, 150)
    flat.set_editor_property("constant", unreal.LinearColor(0, 0, 1, 1))
    nl = MEL.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -400, 150)
    MEL.connect_material_expressions(flat, "", nl, "A")
    MEL.connect_material_expressions(nrm, "RGB", nl, "B")
    MEL.connect_material_expressions(const(0.3, -700, 250), "", nl, "Alpha")
    MEL.connect_material_property(nl, "", unreal.MaterialProperty.MP_NORMAL)
    rim = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -400, 350)
    rimc = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -700, 350)
    rimc.set_editor_property("constant", unreal.LinearColor(0.5 * 0.02, 0.7 * 0.02, 1.0 * 0.02, 1))
    MEL.connect_material_expressions(rimc, "", rim, "A")
    MEL.connect_material_expressions(mask, "R", rim, "B")
    MEL.connect_material_property(rim, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    ior = MEL.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -400, 450)
    ior.set_editor_property("parameter_name", "IOR")
    ior.set_editor_property("default_value", 1.52)
    MEL.connect_material_property(ior, "", unreal.MaterialProperty.MP_REFRACTION)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)
    return mat


def build_lights_material(path="/AdastreaShips/Materials/Interiors/M_Int_Lights"):
    """Small light fixtures: flat emissive (0.88,0.94,1.0) x 8 per MATERIAL_MAPPING.md."""
    folder, name = path.rsplit("/", 1)
    log("material %s (flat emissive fixture)" % path)
    if DRY_RUN:
        return
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    col = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -600, 0)
    col.set_editor_property("constant", unreal.LinearColor(0.88, 0.94, 1.0, 1))
    MEL.connect_material_property(col, "", unreal.MaterialProperty.MP_BASE_COLOR)
    em = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    MEL.connect_material_expressions(col, "", em, "A")
    em.set_editor_property("const_b", 8.0)
    MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)


def rebind_interior_meshes():
    int_dir = os.path.join(MOUNT_ROOTS["/AdastreaShips/"], "Meshes", "Interiors")
    for fname in sorted(os.listdir(int_dir)):
        if not (fname.startswith("SM_Int_") and fname.endswith(".uasset")):
            continue
        name = fname[:-7]
        path = "/AdastreaShips/Meshes/Interiors/" + name
        mesh = unreal.load_asset(path)
        if not mesh or DRY_RUN:
            continue
        changed = assign_materials_by_slot(mesh)
        if changed:
            EAL.save_asset(path, only_if_is_dirty=False)
            log("  slots %s: %s" % (name, changed))


def run_interiors():
    for set_name in INT_SETS:
        import_texture_set("T_Int_" + set_name, src_dir=INT_TEX_SRC, dest_root=INT_TEX_ROOT)
    master = build_oriented_master()
    for name, (sets, scalars, tint) in KIT_MIS.items():
        make_surface_mi(INT_MAT_DIR, name, master, sets, scalars, tint)
    for name, (sets, scalars, tint) in STN_MIS.items():
        sc = dict(WA)
        sc.update(scalars)
        make_surface_mi(STN_MI_DIR, name, master, sets, sc, tint)
    build_library_glass()
    build_lights_material()
    if not DRY_RUN and EAL.does_asset_exist("/AdastreaShips/Materials/Interiors/M_Int_Eng"):
        EAL.delete_asset("/AdastreaShips/Materials/Interiors/M_Int_Eng")
        log("deleted superseded /AdastreaShips/Materials/Interiors/M_Int_Eng")
    rebind_interior_meshes()


# ---------------------------------------------------------------------------
# Unique-baked interiors (Tools/bake_interior_unique.py): the ship-EXTERIOR
# recipe applied to a room. Per kit: T_Int_<Kit>_{Floor,Room,Kit}_* 4K atlases
# on UV0 + one shared tiling micro-detail normal for close range.
# ---------------------------------------------------------------------------
UNIQUE_INT_MASTER = INT_MAT_DIR + "/M_Int_UniqueBaked"
UNIQUE_INT_ATLASES = ["Floor", "Room", "Kit"]
DETAIL_TILE_M = 0.5            # T_Int_DetailMicro_N covers 50 cm


def build_unique_interior_master():
    log("material %s (unique-baked interior master)" % UNIQUE_INT_MASTER)
    if DRY_RUN:
        return None
    folder, name = UNIQUE_INT_MASTER.rsplit("/", 1)
    mat, path = get_or_create_material(folder, name)
    MEL.delete_all_material_expressions(mat)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", False)
    S = unreal.MaterialSamplerType
    detail = int_tex("DetailMicro", "N")

    def tparam(pname, tex, sampler, x, y, uv=None):
        n = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
        n.set_editor_property("parameter_name", pname)
        if tex:
            n.set_editor_property("texture", tex)
        n.set_editor_property("sampler_type", sampler)
        if uv is not None:
            MEL.connect_material_expressions(uv, "", n, "UVs")
        return n

    # defaults: any existing unique set, so the master compiles standalone
    def any_tex(suffix):
        for kit in ("CommandXLBridge",):
            t = int_tex("%s_Room" % kit, suffix)
            if t:
                return t
        return None
    d = tparam("BaseColor", any_tex("D"), S.SAMPLERTYPE_COLOR, -900, -400)
    n = tparam("Normal", any_tex("N"), S.SAMPLERTYPE_NORMAL, -900, -150)
    r = tparam("Roughness", any_tex("R"), S.SAMPLERTYPE_MASKS, -900, 100)
    m = tparam("Metallic", any_tex("M"), S.SAMPLERTYPE_MASKS, -900, 350)
    ao = tparam("AmbientOcclusion", any_tex("AO"), S.SAMPLERTYPE_MASKS, -900, 600)
    e = tparam("Emissive", any_tex("E"), S.SAMPLERTYPE_COLOR, -900, 850)

    # detail normal on UV0 * DetailTiling: the atlas has uniform texel density,
    # so this is a uniform world-scale tiling (the MI sets it per atlas).
    uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1500, -700)
    til = _scalar(mat, "DetailTiling", 16.0, -1500, -600)
    uvm = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -1300, -650)
    MEL.connect_material_expressions(uv, "", uvm, "A")
    MEL.connect_material_expressions(til, "", uvm, "B")
    dn = tparam("DetailNormal", detail, S.SAMPLERTYPE_NORMAL, -900, -700, uv=uvm)
    ds = _scalar(mat, "DetailStrength", 0.6, -900, -900)
    blend = _custom(mat, -500, -300,
                    "float2 d = D.xy * S; return normalize(float3(N.xy + d, N.z));",
                    ["N", "D", "S"], unreal.CustomMaterialOutputType.CMOT_FLOAT3, "WhiteoutDetailNormal")
    MEL.connect_material_expressions(n, "RGB", blend, "N")
    MEL.connect_material_expressions(dn, "RGB", blend, "D")
    MEL.connect_material_expressions(ds, "", blend, "S")
    MEL.connect_material_property(blend, "", unreal.MaterialProperty.MP_NORMAL)

    tint = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -900, -560)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1, 1, 1, 1))
    bc = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, -450)
    MEL.connect_material_expressions(d, "RGB", bc, "A")
    MEL.connect_material_expressions(tint, "", bc, "B")
    MEL.connect_material_property(bc, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rs = _scalar(mat, "RoughnessScale", 1.0, -700, 180)
    rm = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, 120)
    MEL.connect_material_expressions(r, "R", rm, "A")
    MEL.connect_material_expressions(rs, "", rm, "B")
    MEL.connect_material_property(rm, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(m, "R", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    es = _scalar(mat, "EmissiveStrength", 4.0, -700, 950)
    em = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, 880)
    MEL.connect_material_expressions(e, "RGB", em, "A")
    MEL.connect_material_expressions(es, "", em, "B")
    MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.recompile_material(mat)
    EAL.save_asset(path, only_if_is_dirty=False)
    return mat


def run_unique_interior(kit):
    """kit = texture tag, e.g. CommandXLBridge (from prefix SM_Int_CommandXL_Bridge)."""
    import json
    contract = None
    for fname in os.listdir(GEN_DIR):
        if fname.startswith("SM_Int_") and fname.endswith("_contract.json") \
                and fname[len("SM_Int_"):-len("_contract.json")].replace("_", "") == kit:
            with open(os.path.join(GEN_DIR, fname)) as fh:
                contract = json.load(fh)
    bake = (contract or {}).get("unique_bake", {})
    size = bake.get("size", 4096)
    import_texture_set("T_Int_DetailMicro", src_dir=INT_TEX_SRC, dest_root=INT_TEX_ROOT)
    for atlas in UNIQUE_INT_ATLASES:
        import_texture_set("T_Int_%s_%s" % (kit, atlas), src_dir=INT_TEX_SRC, dest_root=INT_TEX_ROOT)
    master = build_unique_interior_master()
    if DRY_RUN:
        return
    for atlas in UNIQUE_INT_ATLASES:
        set_name = "%s_%s" % (kit, atlas)
        name = "MI_Int_" + set_name
        path = INT_MAT_DIR + "/" + name
        mi = unreal.load_asset(path) if asset_exists(path) else None
        if mi is None:
            mi = ASSET_TOOLS.create_asset(name, INT_MAT_DIR, unreal.MaterialInstanceConstant,
                                          unreal.MaterialInstanceConstantFactoryNew())
        MEL.set_material_instance_parent(mi, master)
        for pname, suffix in (("BaseColor", "D"), ("Normal", "N"), ("Roughness", "R"), ("Metallic", "M"),
                              ("AmbientOcclusion", "AO"), ("Emissive", "E")):
            tex = int_tex(set_name, suffix)
            if tex:
                MEL.set_material_instance_texture_parameter_value(mi, pname, tex)
        ppm = bake.get(atlas, {}).get("px_per_m_area_weighted_mean")
        # UV0 spans size/ppm metres; one detail tile per DETAIL_TILE_M
        tiling = (size / ppm) / DETAIL_TILE_M if ppm else 16.0
        MEL.set_material_instance_scalar_parameter_value(mi, "DetailTiling", tiling)
        MEL.update_material_instance(mi)
        EAL.save_asset(path, only_if_is_dirty=False)
        log("  %s: DetailTiling %.2f (%.0f px/m)" % (name, tiling, ppm or 0))


CAPITAL_BPS = {
    "/Game/Blueprints/Ships/BP_Battleship": ("SM_Int_Battleship_Bridge_Shell", "BATTLESHIP_BRIDGE"),
    "/Game/Blueprints/Ships/BP_CommandXL":  ("SM_Int_CommandXL_Bridge_Shell", "COMMAND_XL_BRIDGE"),
}


def wire_capital_bps():
    for bp_path, (shell, fam) in CAPITAL_BPS.items():
        bp = unreal.load_asset(bp_path)
        mesh = unreal.load_asset("/AdastreaShips/Meshes/Interiors/" + shell)
        if not bp or not mesh:
            log("! capital BP wiring: missing %s or %s" % (bp_path, shell))
            continue
        cdo = unreal.get_default_object(bp.generated_class())
        before = (cdo.get_editor_property("InteriorShellMesh"), cdo.get_editor_property("InteriorFamily"))
        enum_val = getattr(unreal.ShipInteriorFamily, fam, None)
        if enum_val is None:
            enum_val = getattr(unreal.ShipInteriorFamily, fam.replace("_XL_", "XL_"))
        if DRY_RUN:
            log("would set %s: %s -> %s / %s" % (bp_path, before, shell, enum_val))
            continue
        cdo.set_editor_property("InteriorShellMesh", mesh)
        cdo.set_editor_property("InteriorFamily", enum_val)
        EAL.save_asset(bp_path, only_if_is_dirty=False)
        log("capital BP %s: %s -> InteriorShellMesh=%s InteriorFamily=%s" % (
            bp_path, before, shell, cdo.get_editor_property("InteriorFamily")))


def main(argv):
    global DRY_RUN
    names, globs, texsets, fixes, accents = [], [], [], False, False
    interiors = capital_bps = glass = False
    unique_hulls, unique_interiors = [], []
    for arg in argv:
        if arg == "--dry-run":
            DRY_RUN = True
        elif arg == "--fixes":
            fixes = True
        elif arg == "--accents":
            accents = True
        elif arg == "--interiors":
            interiors = True
        elif arg == "--capital-bps":
            capital_bps = True
        elif arg == "--glass":
            glass = True
        elif arg.startswith("--unique-hull="):
            unique_hulls += [h for h in arg.split("=", 1)[1].split(",") if h]
        elif arg.startswith("--unique-interior="):
            unique_interiors += [k for k in arg.split("=", 1)[1].split(",") if k]
        elif arg.startswith("--glob="):
            globs.append(arg.split("=", 1)[1])
        elif arg.startswith("--textures="):
            texsets += [t for t in arg.split("=", 1)[1].split(",") if t]
        elif arg.startswith("-"):
            log("ignoring unknown arg %s" % arg)
        else:
            names.append(arg)
    for pattern in globs:
        names += sorted(f for f in os.listdir(GEN_DIR) if fnmatch.fnmatch(f, pattern))
    names = list(dict.fromkeys(n[:-4] if n.lower().endswith(".fbx") else n for n in names))

    log("=== art-gap import: %d mesh(es), %d texture set(s), fixes=%s, dry_run=%s ===" % (
        len(names), len(texsets), fixes, DRY_RUN))
    ok = True
    if accents:
        build_accent_materials()  # before meshes, so their Accent slots can bind
    for texset in texsets:
        if not import_texture_set(texset) and not DRY_RUN:
            log("! no textures imported for %s" % texset)
            ok = False
    for ship in unique_hulls:
        # Baked unique-UV hull (Tools/build_unique_hull_textures.py), same recipe as the
        # Corvette prototype: T_<Ship>_Unique_* 4K set, non-tiled, M_<Ship>_Hull_Unique.
        texset = "T_%s_Unique" % ship
        import_texture_set(texset)
        build_tiled_pbr_material("/Game/Materials", "M_%s_Hull_Unique" % ship, texset,
                                 tiling=1.0, emissive_strength=1.0)
    if interiors:
        run_interiors()  # before meshes, so new SM_Int_* slots bind to the MIs
    for kit in unique_interiors:
        run_unique_interior(kit)  # before meshes, so M_Int_<Kit>_<Atlas> slots bind to the MIs
    for name in names:
        ok = import_mesh(name) and ok
    if fixes:
        run_fixes()
    if capital_bps:
        wire_capital_bps()
    if glass and not interiors:
        build_library_glass()
    log("RESULT_OK" if ok else "RESULT_FAIL")
    return ok


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except Exception:
        import traceback
        unreal.log_error("[art-gap] crashed:\n" + traceback.format_exc())
        unreal.log("[art-gap] RESULT_FAIL")
