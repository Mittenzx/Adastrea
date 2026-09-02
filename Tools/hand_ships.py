"""HAND-AUTHORED flagship ships — deliberate, hard-surface-quality silhouettes.

Unlike the randomized procedural kitbash, each ship here is hand-crafted: every
primitive is placed with intent, booleans/bevels are applied to give clean angular
edges, and the silhouette is designed to read as a specific vessel. Run headless:
    blender -b --python Tools/hand_ships.py -- <ship_name>

Fits the user's chosen direction: procedural stays as the filler fleet; these are
the premium hand-authored silhouettes. Exports to Assets/FBX/hand_author/.
"""
import bpy, os, math
import bmesh

HAND_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        "..", "Assets", "FBX", "hand_author")
MAT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "..", "Assets", "FBX", "generated", "Textures")
os.makedirs(HAND_DIR, exist_ok=True)

def clear():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for ob in list(bpy.data.objects):
        if ob.name != "Camera":
            bpy.data.objects.remove(ob)

def box(name, sx, sy, sz, loc=(0,0,0), rot=(0,0,0)):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    ob = bpy.context.active_object; ob.name = name
    ob.scale = (sx, sy, sz); ob.rotation_euler = rot
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    return ob

def bevel(ob, amt=4, seg=2):
    """Hard bevel for crisp angular edges (hard-surface look)."""
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.modifier_add(type='BEVEL')
    ob.modifiers[-1].width = amt
    ob.modifiers[-1].segments = seg
    ob.modifiers[-1].limit_method = 'ANGLE'
    ob.modifiers[-1].angle_limit = math.radians(30)

def fin(name, lx, ly, lz, loc=(0,0,0), rot=(0,0,0)):
    """A single tapered wedge fin (swept wing)."""
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    ob = bpy.context.active_object; ob.name = name
    ob.rotation_euler = rot
    ob.scale = (lx, ly, lz)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bm = bmesh.new(); bm.from_mesh(ob.data)
    # taper: pull +Y top verts toward center axis to make a swept fin
    for v in bm.verts:
        if v.co.y > 0.05:
            v.co.x *= 0.25
    bm.to_mesh(ob.data); bm.free()
    bpy.data.objects[ob.name].data.update()
    return ob

def union(objnames, outname, bevel_amt=6):
    """Boolean-union a set of objects into one clean hull (hard-surface merge)."""
    base = bpy.data.objects[objnames[0]]
    bpy.context.view_layer.objects.active = base
    for name in objnames[1:]:
        ob = bpy.data.objects[name]
        mod = base.modifiers.new(name="B"+ob.name, type='BOOLEAN')
        mod.operation = 'UNION'; mod.object = ob
    for i in range(len(objnames)-1):
        bpy.ops.object.modifier_apply(modifier=base.modifiers[0].name)
    bevel(base, bevel_amt, seg=3)
    base.name = outname
    return base

_TEX = os.path.join(os.path.dirname(os.path.abspath(__file__)),
               "..", "Assets", "FBX", "generated", "Textures")


def _wire_pbr(mat, name):
    """Wire a PBR texture set into the material so hand-authored ships aren't
    flat. Tries the base hull texture (Ship_Hull) + a per-ship override if the
    ship name maps to a known faction set."""
    import bpy, os
    overrides = {"Frigate": "Freighter", "Cutlass": "Gunship"}
    texset = None
    for k, v in overrides.items():
        if k.lower() in name.lower():
            texset = v
    if texset is None:
        texset = "Ship_Hull"
    dpath = os.path.join(_TEX, f"T_{texset}_D.png")
    if not os.path.exists(dpath):
        return
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    prin = nt.nodes.new("ShaderNodeBsdfPrincipled")
    out.location = (720, 0); prin.location = (120, 0)
    nt.links.new(prin.outputs["BSDF"], out.inputs["Surface"])
    imgD = nt.nodes.new("ShaderNodeTexImage")
    imgD.image = bpy.data.images.load(dpath)
    imgD.label = "BaseColor_D"; imgD.location = (-760, 200)
    nt.links.new(imgD.outputs["Color"], prin.inputs["Base Color"])
    np_ = os.path.join(_TEX, f"T_{texset}_N.png")
    if os.path.exists(np_):
        imgN = nt.nodes.new("ShaderNodeTexImage")
        imgN.image = bpy.data.images.load(np_); imgN.image.colorspace_settings.name = "Non-Color"
        imgN.location = (-760, -80)
        nm = nt.nodes.new("ShaderNodeNormalMap"); nm.location = (-420, -80)
        nt.links.new(imgN.outputs["Color"], nm.inputs["Color"])
        nt.links.new(nm.outputs["Normal"], prin.inputs["Normal"])
    rp = os.path.join(_TEX, f"T_{texset}_R.png")
    if os.path.exists(rp):
        imgR = nt.nodes.new("ShaderNodeTexImage")
        imgR.image = bpy.data.images.load(rp); imgR.image.colorspace_settings.name = "Non-Color"
        imgR.location = (-760, -260)
        nt.links.new(imgR.outputs["Color"], prin.inputs["Roughness"])
    mp = os.path.join(_TEX, f"T_{texset}_M.png")
    if os.path.exists(mp):
        imgM = nt.nodes.new("ShaderNodeTexImage")
        imgM.image = bpy.data.images.load(mp); imgM.image.colorspace_settings.name = "Non-Color"
        imgM.location = (-760, -420)
        nt.links.new(imgM.outputs["Color"], prin.inputs["Metallic"])
    ap = os.path.join(_TEX, f"T_{texset}_AO.png")
    if os.path.exists(ap):
        imgAO = nt.nodes.new("ShaderNodeTexImage")
        imgAO.image = bpy.data.images.load(ap); imgAO.image.colorspace_settings.name = "Non-Color"
        imgAO.location = (-760, 460)
        mul = nt.nodes.new("ShaderNodeMix")
        mul.data_type = "RGBA"; mul.inputs["Factor"].default_value = 1.0
        mul.location = (-400, 320)
        nt.links.new(imgD.outputs["Color"], mul.inputs["A"])
        nt.links.new(imgAO.outputs["Color"], mul.inputs["B"])
        nt.links.new(mul.outputs["Result"], prin.inputs["Base Color"])
    em = nt.nodes.new("ShaderNodeEmission"); em.location = (120, 320)
    em.inputs["Strength"].default_value = 2.6
    nt.links.new(imgD.outputs["Color"], em.inputs["Color"])
    mix = nt.nodes.new("ShaderNodeMixShader"); mix.location = (420, 180)
    mix.inputs["Fac"].default_value = 0.92
    nt.links.new(em.outputs["Emission"], mix.inputs[1])
    nt.links.new(prin.outputs["BSDF"], mix.inputs[2])
    nt.links.new(mix.outputs["Shader"], out.inputs["Surface"])


def finalize(name, outname, matname="M_Hull"):
    """Export the whole authored ship (all mesh objects) as one FBX."""
    mat = bpy.data.materials.get(matname)
    if mat is None:
        mat = bpy.data.materials.new(matname); mat.use_nodes = True
    _wire_pbr(mat, outname)
    meshes = [o for o in bpy.data.objects if o.type == 'MESH']
    for ob in meshes:
        if not ob.data.materials:
            ob.data.materials.append(mat)
        else:
            ob.data.materials[0] = mat
        ob.select_set(True)
    out = os.path.join(HAND_DIR, outname + ".fbx")
    bpy.ops.export_scene.fbx(filepath=out, use_selection=True)
    for ob in meshes:
        ob.select_set(False)
    print("hand-authored exported:", out)
    return out

def build_frigate():
    """A hand-crafted heavy frigate: long angular razor-hull, twin dorsal spines,
    flank thruster booms. Deliberate silhouette — reads as an armored fleet escorter."""
    clear()
    objs = []
    # main razor hull (long, tapering fore)
    hull = box("Hull", 1.0, 3.2, 0.55, rot=(0, math.radians(2), 0))
    bevel(hull, 8, 3); objs.append(hull)
    # angular fore wedge (razor bow)
    fore = box("Fore", 1.1, 0.9, 0.45, loc=(0, 1.9, 0), rot=(0, math.radians(-6), 0))
    bevel(fore, 6, 3); objs.append(fore)
    # dorsal spines (command fins) along mid-to-aft
    for i, (y, h) in enumerate([(0.4, 0.5), (-0.4, 0.6)]):
        spine = fin(f"Spine{i}", 0.5, 0.9, h, loc=(0, y*1.0, 0.42))
        bevel(spine, 4, 2); objs.append(spine)
    # flank thruster booms (twin engine outriggers)
    for side in (-1, 1):
        boom = box(f"Boom{side}", 0.6, 1.4, 0.4, loc=(side*0.75, -0.9, 0))
        bevel(boom, 5, 3); objs.append(boom)
        nozzle = box(f"Noz{side}", 0.35, 0.5, 0.5, loc=(side*0.75, -1.7, 0), rot=(0,0,0))
        bevel(nozzle, 3, 2); objs.append(nozzle)
    return finalize(objs[0].name, "SM_Ship_Frigate_Hand", "M_Hull")

def build_cutlass():
    """A hand-crafted sleek corvette/cutlass: swept delta hull with forward raked
    canards and twin tailplane — a fast, aggressive line."""
    clear()
    objs = []
    hull = box("Cut", 0.9, 2.6, 0.45, rot=(0, math.radians(1), 0)); bevel(hull, 7, 3); objs.append(hull)
    nose = box("CutNose", 0.6, 0.5, 0.35, loc=(0, 1.45, 0)); bevel(nose, 5, 3); objs.append(nose)
    # delta wings (swept wedges)
    for side in (-1, 1):
        w = fin(f"CutWing{side}", 1.2, 0.9, 0.12, loc=(side*0.62, 0.1, -0.05), rot=(0,0,math.radians(-8*side)))
        objs.append(w)
        canard = fin(f"CutCan{side}", 0.5, 0.4, 0.1, loc=(side*0.45, 1.1, 0.05), rot=(0,0,math.radians(-12*side)))
        objs.append(canard)
    # twin tailplanes aft
    for side in (-1,1):
        objs.append(fin(f"CutTail{side}", 0.7, 0.6, 0.1, loc=(side*0.55, -1.1, 0.05), rot=(0,0,math.radians(-6*side))))
    return finalize(objs[0].name, "SM_Ship_Cutlass_Hand", "M_Hull")

HAND_SHIPS = {"frigate": build_frigate, "cutlass": build_cutlass}

if __name__ == "__main__":
    import sys
    which = sys.argv[sys.argv.index("--")+1] if "--" in sys.argv else "frigate"
    fn = HAND_SHIPS.get(which)
    if fn:
        out = fn()
        print("OK", which, "->", out)
    else:
        print("available:", list(HAND_SHIPS))