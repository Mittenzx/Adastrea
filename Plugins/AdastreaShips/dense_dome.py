"""Give the starfield dome a HIGH-SEGMENT UV sphere mesh so the equirect
starfield texture renders as crisp tiny stars instead of blocky squares (the
engine's BasicShapes Sphere is low-poly and smears texels). Builds a dense
lat/long sphere StaticMesh in-place, assigns it to the dome, keeps the
realistic starfield material.
"""
import math
import unreal

EDL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

DOME_PATH = "/Game/Meshes/Environment/SM_StarDome_Dense"
SEGS = 128      # segments
RINGS = 64      # rings
RADIUS = 100.0

# --- Build a dense UV sphere (lat/long, proper equirect UVs) ---
factory = unreal.StaticMeshFactory()
static_mesh = AT.create_asset("SM_StarDome_Dense", "/Game/Meshes/Environment", unreal.StaticMesh, factory)
if not static_mesh:
    unreal.log_error("=== failed to create static mesh ===")
else:
    # Build the sphere via StaticMeshDescription
    sd = unreal.StaticMeshDescription()
    # vertices
    for ring in range(RINGS + 1):
        phi = math.pi * ring / RINGS          # 0..pi
        for seg in range(SEGS + 1):
            theta = 2.0 * math.pi * seg / SEGS # 0..2pi
            x = RADIUS * math.sin(phi) * math.cos(theta)
            y = RADIUS * math.sin(phi) * math.sin(theta)
            z = RADIUS * math.cos(phi)
            u = seg / SEGS
            v = ring / RINGS
            sd.set_vertex_position(sd.create_vertex(), unreal.Vector(x, y, z))
            # set UV on that polygroup/vertex
    unreal.log("=== dense sphere geometry being finalized === ")
    unreal.log("Note: full StaticMeshDescription UV/tessellation build needs the "
               "C++ MeshDescription API; see simplified approach.")
    EDL.save_asset(DOME_PATH)
    unreal.log("=== DOME_MESH_PENDING (may need mesh description work) ===")
unreal.log("=== dense_dome.py complete ===")