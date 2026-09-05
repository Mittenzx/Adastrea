"""
Phase 3 verification: loads DA_StationModuleCatalog, calls LoadCatalogFromJson(),
and writes the resulting entry/class/material breakdown to a JSON file that the
agent reads back. Run manually via -ExecutePythonScript (NOT in startup/ so it
doesn't run on every launch).
"""
import json
import os
import unreal

OUT = os.path.join(unreal.Paths.project_dir(), "Saved", "_catalog_verify.json")

asset_path = "/Game/DataAssets/Stations/DA_StationModuleCatalog.DA_StationModuleCatalog"
catalog = unreal.load_asset(asset_path)
result = {"loaded": False, "error": "", "count": 0, "entries": []}

if catalog is None:
    result["error"] = "catalog asset not found at " + asset_path
else:
    try:
        n = catalog.load_catalog_from_json()
        result["loaded"] = True
        result["count"] = n
        entries = catalog.get_editor_property("module_entries")
        for e in entries:
            item = {
                "display_name": str(e.get_editor_property("display_name")),
                "tech": e.get_editor_property("required_tech_level"),
                "credits": e.get_editor_property("build_cost").get_editor_property("credits"),
                "group": str(e.get_editor_property("module_group")),
                "module_class": str(e.get_editor_property("module_class") or ""),
            }
            mats = e.get_editor_property("build_cost").get_editor_property("materials") or {}
            item["materials"] = {str(k): int(v) for k, v in mats.items()}
            result["entries"].append(item)
    except Exception as ex:  # noqa: BLE001
        result["error"] = "exception: " + repr(ex)

with open(OUT, "w", encoding="utf-8") as fh:
    json.dump(result, fh, indent=2, default=str)
print("catalog_verify ->", OUT, "count=", result["count"], "err=", result["error"])