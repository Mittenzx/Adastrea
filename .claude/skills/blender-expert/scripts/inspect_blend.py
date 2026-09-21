"""Read-only report on a .blend file. Never saves anything.

Headless:
    blender -b path/to/file.blend --python inspect_blend.py
    blender -b path/to/file.blend --python inspect_blend.py -- --json report.json
Inside Blender: open in the Text Editor and press Run Script (output goes to the system console).
"""

import json
import sys

import bpy

EPS = 1e-4
HELPER_PREFIXES = ("UCX_", "UBX_", "USP_", "UCP_", "SOCKET_")


def script_args():
    return sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []


def rounded(values, digits=4):
    return [round(v, digits) for v in values]


def has_rotation(ob):
    if ob.rotation_mode == "QUATERNION":
        return any(abs(a - b) > EPS for a, b in zip(ob.rotation_quaternion, (1.0, 0.0, 0.0, 0.0)))
    if ob.rotation_mode == "AXIS_ANGLE":
        return abs(ob.rotation_axis_angle[0]) > EPS
    return any(abs(r) > EPS for r in ob.rotation_euler)


def has_scale(ob):
    return any(abs(s - 1.0) > EPS for s in ob.scale)


def build_report():
    scene = bpy.context.scene
    units = scene.unit_settings
    report = {
        "blender_version": bpy.app.version_string,
        "file": bpy.data.filepath or "(unsaved)",
        "scene": scene.name,
        "units": {
            "system": units.system,
            "scale_length": round(units.scale_length, 6),
            "length_unit": units.length_unit,
        },
        "frame_range": [scene.frame_start, scene.frame_end],
        "fps": scene.render.fps,
        "render_engine": scene.render.engine,
        "collections": sorted(c.name for c in bpy.data.collections),
        "objects": [],
        "materials": sorted(m.name for m in bpy.data.materials),
        "actions": sorted(a.name for a in bpy.data.actions),
        "images": [],
        "notes": [],
    }
    notes = report["notes"]

    if units.system != "METRIC" or abs(units.scale_length - 1.0) > EPS:
        notes.append(
            f"Scene units are {units.system} with unit scale {units.scale_length}; "
            "check this matches the project's Unreal export convention."
        )

    for ob in bpy.data.objects:
        entry = {
            "name": ob.name,
            "type": ob.type,
            "parent": ob.parent.name if ob.parent else None,
            "in_scene": ob.name in scene.objects,
            "location": rounded(ob.location),
            "rotation_mode": ob.rotation_mode,
            "scale": rounded(ob.scale),
            "modifiers": [f"{m.name} ({m.type})" for m in ob.modifiers],
        }

        if ob.type == "MESH":
            mesh = ob.data
            entry.update({
                "vertices": len(mesh.vertices),
                "faces": len(mesh.polygons),
                "uv_maps": [uv.name for uv in mesh.uv_layers],
                "material_slots": [s.material.name if s.material else None for s in ob.material_slots],
            })
            if not mesh.uv_layers and not ob.name.startswith(HELPER_PREFIXES):
                notes.append(f"{ob.name}: no UV map")
            if any(slot is None for slot in entry["material_slots"]) or not ob.material_slots:
                if not ob.name.startswith(HELPER_PREFIXES):
                    notes.append(f"{ob.name}: empty or missing material slot")
        elif ob.type == "ARMATURE":
            bones = ob.data.bones
            entry.update({
                "bones": len(bones),
                "deform_bones": sum(1 for b in bones if b.use_deform),
                "root_bones": [b.name for b in bones if b.parent is None],
            })
            if len(entry["root_bones"]) > 1:
                notes.append(
                    f"{ob.name}: {len(entry['root_bones'])} root bones {entry['root_bones']}; "
                    "Unreal needs a single root among exported bones (parent them, or export deform bones only)"
                )

        if ob.type in {"MESH", "ARMATURE"}:
            if has_scale(ob):
                notes.append(f"{ob.name}: unapplied scale {entry['scale']}")
            if has_rotation(ob):
                notes.append(f"{ob.name}: unapplied rotation (fine for placed objects; apply before exporting a single asset)")

        report["objects"].append(entry)

    for img in bpy.data.images:
        if img.source == "VIEWER" or img.type in {"RENDER_RESULT", "COMPOSITING"}:
            continue
        info = {"name": img.name, "filepath": img.filepath, "packed": img.packed_file is not None}
        report["images"].append(info)
        if img.filepath and not img.filepath.startswith("//") and not info["packed"]:
            notes.append(f"Image {img.name}: absolute path {img.filepath}")

    return report


def print_report(report):
    print("===== BLEND REPORT START =====")
    print(f"Blender {report['blender_version']} | {report['file']}")
    u = report["units"]
    print(f"Scene: {report['scene']} | units {u['system']} x{u['scale_length']} ({u['length_unit']}) | "
          f"frames {report['frame_range'][0]}-{report['frame_range'][1]} @ {report['fps']} fps | {report['render_engine']}")
    print(f"Collections: {', '.join(report['collections']) or '-'}")
    print("Objects:")
    for o in report["objects"]:
        extra = ""
        if o["type"] == "MESH":
            extra = f" | {o['faces']} faces | UVs {o['uv_maps'] or '-'} | mats {o['material_slots'] or '-'}"
        elif o["type"] == "ARMATURE":
            extra = f" | {o['bones']} bones ({o['deform_bones']} deform) | roots {o['root_bones']}"
        scene_flag = "" if o["in_scene"] else " | NOT IN SCENE"
        mods = f" | mods {o['modifiers']}" if o["modifiers"] else ""
        print(f"  - {o['name']} [{o['type']}] parent={o['parent']} scale={o['scale']}{extra}{mods}{scene_flag}")
    print(f"Materials: {', '.join(report['materials']) or '-'}")
    print(f"Actions: {', '.join(report['actions']) or '-'}")
    print(f"Images: {len(report['images'])}")
    print("Notes:")
    for note in report["notes"] or ["none"]:
        print(f"  * {note}")
    print("===== BLEND REPORT END =====")


def main():
    args = script_args()
    report = build_report()
    print_report(report)
    if "--json" in args:
        idx = args.index("--json")
        if idx + 1 < len(args):
            path = bpy.path.abspath(args[idx + 1])
            with open(path, "w", encoding="utf-8") as handle:
                json.dump(report, handle, indent=2)
            print(f"JSON written to {path}")


main()
