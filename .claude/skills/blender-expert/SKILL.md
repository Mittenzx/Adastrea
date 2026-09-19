---
name: blender-expert
description: Expert Blender help for modelling, UVs, materials and baking, rigging, animation, geometry nodes, rendering, bpy Python scripting, and exporting to game engines, especially Unreal Engine (FBX or glTF export, scale and axes, smoothing, collision, sockets, LODs, skeletal meshes, animations). Use whenever the user mentions Blender, .blend files, a mesh, model, rig, armature, UV unwrapping, texture baking, or getting an asset from Blender into Unreal, even if Blender is not named explicitly.
---

# Blender Expert

Help like an experienced technical artist who knows Blender and game pipelines, and who checks the actual files before making claims about them.

## 1. Get your bearings

- **Version first.** Blender's UI and Python API changed a lot across 3.x, 4.x and 5.x, so match advice to the user's version (`blender --version`, or ask). Examples of drift: 4.0 renamed several Principled BSDF inputs (Specular became Specular IOR Level), 4.1 replaced Auto Smooth with the Smooth by Angle modifier, and 4.2 introduced Extensions for installing add-ons.
- **Find the art.** Locate the `.blend` files (often in an `Art/` or `SourceArt/` folder, sometimes outside the game project). In Claude Code, folders outside the working directory need `/add-dir`.
- Check `CLAUDE.md` for pipeline notes (export folder, naming, units), and offer to record new durable facts there.
- No file access (for example in the Claude app)? Ask for the Blender version and a screenshot or description of the relevant panel instead of guessing.

## 2. Looking inside .blend files

`.blend` files are binary, so don't read them as text. Inspect them by running Blender headless with a read-only script. This skill ships `scripts/inspect_blend.py`, which reports version, units, objects, transforms, modifiers, UVs, materials, armatures, actions and image paths, and flags common export problems:

```
blender -b "path/to/file.blend" --python "path/to/blender-expert/scripts/inspect_blend.py"
blender -b "path/to/file.blend" --python "path/to/blender-expert/scripts/inspect_blend.py" -- --json report.json
```

- If `blender` isn't on PATH, Windows installs usually live in `C:\Program Files\Blender Foundation\Blender 5.x\blender.exe` (or the Steam library). Find it or ask.
- Headless runs never save the file unless a script calls a save operator, so inspection is safe. Don't save from scripts unless asked, and prefer saving a copy when you do.
- For live, interactive work, a Blender MCP connection (the official Blender Lab server or the community blender-mcp add-on) lets you inspect and change the open scene. Those tools run Python with no safety net, so ask the user to save first and keep each change small.

## 3. Scripting with bpy

- Prefer the data API (`bpy.data`, object and mesh properties) over `bpy.ops`. Operators depend on context (mode, selection, active object, editor type) and often fail in headless runs.
- When an operator is unavoidable, set the selection and active object explicitly, and use `bpy.context.temp_override(...)` (Blender 3.2+) for operators that need a particular editor.
- When a call fails, check the Python API docs for the user's version (docs.blender.org/api/) instead of retrying guesses.

## 4. Blender to Unreal

Most "it looks wrong in Unreal" problems come from a few causes. Check these before anything exotic, and test-import one asset before exporting a batch.

**Units and scale.** Blender works in metres by default and Unreal in centimetres, so 1 Blender metre should arrive as 100 Unreal units. A 2 m test cube makes an easy check. For FBX, *Apply Scalings: FBX Units Scale* is the commonly recommended setting for Unreal, especially for skeletal meshes, where other options can leave a 100x scale on the root. Whatever works, keep it consistent across the project.

**Transforms and pivot.** Apply rotation and scale (Ctrl+A) on meshes and armatures before export, otherwise collision, physics and sockets come out skewed. Unreal uses the object origin as the pivot, so put the origin where the pivot belongs (the base of a prop, the hinge of a door).

**Facing direction.** Blender is Z-up and right-handed; Unreal is Z-up and left-handed with X forward. A common convention is to model characters facing Blender's front view (-Y). They arrive facing +Y like the UE mannequin, which Character Blueprints rotate by -90° yaw. Confirm with a test import rather than assuming.

**Smoothing and normals.** Export with *Smoothing: Face* (or Edge) so Unreal receives smoothing data. In 4.1+, control shading with Shade Auto Smooth or the Smooth by Angle modifier (the exporter applies modifiers by default). Fix flipped normals before export using the Face Orientation overlay.

**Collision.** Name custom collision meshes after the render mesh with a type prefix: `UCX_` convex, `UBX_` box, `USP_` sphere, `UCP_` capsule (for example `UCX_SM_Crate_01`). Export them in the same FBX as the mesh, and keep `UCX_` hulls convex.

**Sockets.** Empties named `SOCKET_Name`, parented to a static mesh and exported with it, become sockets in Unreal.

**LODs and Nanite.** Unreal can generate LODs in the Static Mesh Editor, where hand-made LODs can also be imported per LOD slot. Nanite handles high-poly static meshes well.

**UVs and lightmaps.** Every mesh needs a UV map for texturing. Lightmap UVs only matter for baked lighting, and Unreal can generate them on import.

**Materials and textures.** Shader node trees don't transfer; only material slots (by name) come across. Name slots meaningfully and rebuild materials in Unreal, ideally as Material Instances of a master material. Bake or export textures separately.
- Normal maps: Blender bakes OpenGL-style (green up) and Unreal expects DirectX-style, so tick *Flip Green Channel* on the texture in Unreal, or flip before export.
- Data textures (roughness, metallic, AO, or a packed ORM with R = AO, G = roughness, B = metallic) need sRGB turned off in Unreal.

**Skeletal meshes and animation.**
- Turn off *Add Leaf Bones*. They add end bones Unreal doesn't need.
- Blender writes the armature object into the FBX as a node, which can show up as an extra root. Check the hierarchy in Unreal's Skeleton Editor and settle on one clean root bone.
- *Only Deform Bones* drops control bones, but keep any non-deforming bones the game relies on (IK or attachment bones, for example).
- Apply scale to the armature before animating. Scaled armatures cause most animation-scale bugs.
- Enable *Bake Animation* when exporting animation, choose *All Actions* or *NLA Strips* to match how actions are organised, and name actions clearly because the names carry into the imported animations.
- To share animations with an existing skeleton (such as the UE mannequin), bone names and hierarchy must match. Otherwise retarget in Unreal with an IK Rig and IK Retargeter.

**Multi-object files.** When one FBX holds several meshes, Unreal imports separate assets unless *Combine Meshes* is enabled at import.

**Automating exports.** `scripts/export_fbx_for_unreal.py` exports each top-level object (with its children and any matching `UCX_`-style collision meshes) to its own FBX using the settings above:

```
blender -b "file.blend" --python "path/to/export_fbx_for_unreal.py" -- --out "D:/Game/SourceArt/Exports" --dry-run
blender -b "file.blend" --python "path/to/export_fbx_for_unreal.py" -- --out "D:/Game/SourceArt/Exports" --collection Props
```

Other options: `--names SM_Crate SM_Barrel`, `--selected`, `--anim`, `--deform-only`. Always run `--dry-run` first to see what it will write. Run from Blender's Text Editor with no arguments, it exports the current selection to an `exports` folder next to the .blend. Keep exports in a stable folder outside Unreal's `Content/` so Reimport can find the source files.

glTF (`.glb`) is a reasonable alternative, and add-ons such as Send to Unreal can automate the round trip.

## 5. General Blender help

- Give exact menu paths and shortcuts for the user's version, and name the mode (Object, Edit, Pose, and so on), since many tools only exist in one.
- For modelling problems, check applied scale, normals, doubled vertices (Merge by Distance), non-manifold geometry (Select All by Trait), and modifier order.
- For rendering questions, confirm the engine (EEVEE or Cycles) and the device before giving performance advice.
