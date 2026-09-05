"""
Phase 5 runtime verification: after loading the catalog, every module entry
should resolve a real ModuleClass (the 12 research labs previously failed
LoadClass because the C++ classes didn't exist). Verifies all 27 entries now
resolve, and lists per-entry the resolved class so lab classes are confirmed.

Writes JSON to Saved/_phase5_verify.json.
"""
import json
import os
import unreal

OUT = os.path.join(unreal.Paths.project_dir(), "Saved", "_phase5_verify.json")
result = {"ok": False, "unresolved": [], "lab_classes": {}, "error": ""}
try:
    asset_path = "/Game/DataAssets/Stations/DA_StationModuleCatalog.DA_StationModuleCatalog"
    catalog = unreal.load_asset(asset_path)
    if catalog is None:
        result["error"] = "catalog asset not found"
    else:
        n = catalog.load_catalog_from_json()
        entries = catalog.get_editor_property("module_entries")
        result["count"] = n
        unresolved = []
        lab_classes = {}
        for e in entries:
            mc = e.get_editor_property("module_class")
            name = str(e.get_editor_property("display_name"))
            if mc is None:
                unresolved.append(name)
            else:
                class_str = str(mc)
                # Report anything that looks like a research lab.
                if any(k in class_str for k in ["Physics", "Materials", "Electronics",
                                                "Weapons", "Biology", "Projectile",
                                                "Beam", "IonPropulsion", "GravMaterials",
                                                "Encryption", "Optronics", "Cybernetics",
                                                "ResearchLab"]):
                    lab_classes[name] = class_str
        result["unresolved"] = unresolved
        result["lab_classes"] = lab_classes
        # ok if all 27 resolve AND the 12 labs resolve to their classes.
        result["ok"] = (n == 27 and len(unresolved) == 0 and len(lab_classes) >= 12)
except Exception as ex:  # noqa: BLE001
    result["error"] = "exception: " + repr(ex)

with open(OUT, "w", encoding="utf-8") as fh:
    json.dump(result, fh, indent=2, default=str)
print("phase5_verify ->", OUT, "ok=", result["ok"], "count=", result.get("count"),
      "unresolved=", result["unresolved"], "err=", result["error"])
print("lab classes resolved:", len(result.get("lab_classes", {})))