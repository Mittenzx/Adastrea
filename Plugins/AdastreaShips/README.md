# AdastreaShips — Modular Space Asset Kit (UE 5.8 Content Plugin)

Procedurally generated, realistic-cyberpunk space assets for Adastrea, packaged
as a Unreal Engine 5.8 **Content Plugin** so everything lands in the Content
Browser as real, place-able `SM_*` meshes with full PBR materials.

Exactly what the •asset generator• produces, but integrated the way a game
actually consumes it — not loose files.

## Contents

```
Plugins/AdastreaShips/
├── AdastreaShips.uplugin        # plugin manifest (UE 5.8, content-capable)
├── ImportAdastreaShips.py       # ★ the importer (materializes assets in UE)
├── Resources/Icon128.png        # plugin icon (teal habitat-ring motif)
└── Content/
    ├── Python/startup/          # auto-registers the "Adastrea Ships" editor menu
    │   └── register_adastrea_ships.py
    ├── Meshes/
    │   ├── Ships/      SM_Ship_*_Carcass / _Engine / _Cargo / _Weapon / _Sensor / _Reactor
    │   ├── Station/    SM_Station_* (+ Project-Hyperion habit ring / asteroid shell)
    │   └── Objects/    SM_Obj_*, SM_Prop_*
    ├── Textures/       Ships/ Station/ Objects/  (T_*_D / _N / _R / _M / _AO / _E)
    ├── Materials/
    │   ├── Master/     M_Ship_PBR_Master (6 texture params)
    │   └── Instances/  MI_Ship_Hull, MI_HabRing, MI_AsteroidShell, ...
    └── Data/
```

## Install & enable
1. The plugin already lives at `Plugins/AdastreaShips/` inside the Adastrea project
   (content plugins don't need recompiling) — it's auto-discovered.
2. If UE didn't pick it up, open **Edit > Plugins**, search "AdastreaShips",
   and enable it (restart if prompted). It's listed as Enabled by the project already.

## ★ Materialize the assets (two ways)
The `.uasset` binaries are created by the editor, not committed as loose text, so
the import runs once inside UE (either is fine; the menu is recommended):

**A) One-click editor menu (automatic, recommended)**
Enable the plugin, restart the editor, and a **"Adastrea Ships"** menu appears
under the level menu bar (registered automatically from `Content/Python/startup/`):
- **Adastrea Ships → Import All Assets** — materializes meshes, textures, materials
- **Adastrea Ships → Rescan Generator Output** — counts what's in `Assets/FBX/generated`

**B) Manual script**
- **Tools > Execute Python Script...** → `ImportAdastreaShips.py`, or
- In the Python console: `py "Plugins/AdastreaShips/ImportAdastreaShips.py"`

Either imports every `SM_*.fbx` as a StaticMesh (auto-collision, lightmap UVs),
every `T_*.png` as a Texture2D (routed into `Textures/Ships|Station|Objects`),
builds the master PBR material, and creates the per-part material instances.
Wait for the log line `=== AdastreaShips import complete ===`.

> Dependencies: the **Python Script Plugin** and **Editor Scripting Utilities**
> (declared in the .uplugin). The `unreal` module is what runs the script.

## Regenerate & re-import
The source meshes/textures are produced by the code-driven pipeline:
```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" -b --python Tools/generate_adastrea_assets.py
python Tools/fbx2obj.py && python Tools/qa_assets.py && python Tools/render_objs.py
```
After regenerating, use **Adastrea Ships → Import All Assets** again (it replaces
existing assets) to sync.

## Notes / honest caveats
- **This is content-only** — no C++ module is needed, so `CanContainContent: true`
  with no `Modules` entry (matches most content packs).
- **Scale**: all meshes are real-world cm (1 cm = 1 Unreal unit), Z-up, forward −Y.
- **Modularity**: ships are separate per-part meshes (carcass + engine + ...) so
  parts are independently placeable/detachable, X4/Starfield/SE-style.
- The importer uses the UE 5.8 `unreal` Python API (AssetImportTask,
  MaterialEditingLibrary). If a call differs in a future engine version, the
  relevant `set_editor_property` / factory lines are the ones to adjust.
