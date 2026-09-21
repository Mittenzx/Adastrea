"""List every project asset (path, class, Blueprint parent) without loading or saving anything.

Run with the editor CLOSED (Windows example, adjust paths):
  "C:/Program Files/Epic Games/UE_5.x/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "D:/Path/MyGame.uproject"
      -run=pythonscript -script="D:/Path/list_assets.py" -unattended -nosplash -nop4 -stdout

It also works inside the editor (Tools > Execute Python Script, or the Python console).
Requires the Python Editor Script Plugin to be enabled for the project.

Output: <Project>/Saved/ClaudeReports/assets.tsv  (columns: path, class, parent_class)
"""

import os
from collections import Counter

import unreal

ROOT = "/Game"


def class_name(asset_data):
    """Asset class name; the attribute changed in UE 5.1."""
    path = getattr(asset_data, "asset_class_path", None)  # UE 5.1+
    if path is not None:
        try:
            return str(path.asset_name)
        except Exception:
            return str(path)
    return str(getattr(asset_data, "asset_class", "Unknown"))  # UE 5.0


def parent_class(asset_data):
    """Blueprint parent class from asset registry tags, if present."""
    try:
        value = unreal.AssetRegistryHelpers.get_tag_value(asset_data, "ParentClass")
    except Exception:
        return ""
    if isinstance(value, tuple):  # some versions return (found, value)
        value = value[-1] if value and value[0] else ""
    text = str(value or "")
    # Tags look like /Script/Engine.Character or /Script/CoreUObject.Class'/Script/Engine.Character'
    text = text.rstrip("'").split(".")[-1]
    return text


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        registry.search_all_assets(True)  # make sure the scan is complete in commandlet mode
    except Exception as exc:
        unreal.log_warning(f"[list_assets] search_all_assets unavailable: {exc}")

    assets = registry.get_assets_by_path(ROOT, recursive=True)
    rows = []
    for data in assets:
        rows.append((str(data.package_name), class_name(data), parent_class(data)))
    rows.sort()

    saved_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir())
    out_dir = os.path.join(saved_dir, "ClaudeReports")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "assets.tsv")

    with open(out_path, "w", encoding="utf-8") as handle:
        handle.write(f"# engine: {unreal.SystemLibrary.get_engine_version()}\n")
        handle.write("path\tclass\tparent_class\n")
        for row in rows:
            handle.write("\t".join(row) + "\n")

    counts = Counter(cls for _, cls, _ in rows)
    summary = ", ".join(f"{cls}: {n}" for cls, n in counts.most_common(15))
    unreal.log(f"[list_assets] {len(rows)} assets under {ROOT} -> {out_path}")
    unreal.log(f"[list_assets] top classes: {summary}")


main()
