"""
AdastreaShips — UE 5.8 Editor menu registration (one-click import).

Auto-loaded by the Python Script Plugin from Content/Python/startup/ at editor
startup. Installs a "Adastrea Ships" submenu under the level-menu bar with:

  - Import All Assets        materializes FBX + textures + materials (runs the importer)
  - Rescan Generator Output  counts mesh/texture outputs under Assets/FBX/generated

Convention (UE 5.x Python Script Plugin):
  - register_named_function(cmd_name, fn)  binds a Python callable to a command
  - register_menu(menu, label, tooltip, cmd_name, unbind_first)  adds the entry
  - unregister() is called on editor shutdown.
"""

import os
import traceback
import unreal

# This file lives at <plugin>/Content/Python/startup/ ; importer is 3 levels up.
_here = os.path.dirname(os.path.abspath(__file__))
_plugin_dir = os.path.abspath(os.path.join(_here, "..", "..", ".."))
IMPORTER = os.path.join(_plugin_dir, "ImportAdastreaShips.py")
GEN_DIR = os.path.normpath(os.path.join(_plugin_dir, "..", "Assets", "FBX", "generated"))

MENU = "AdastreaShips"
CMD_IMPORT = "AdastreaShips_ImportAll"
CMD_RESCAN = "AdastreaShips_Rescan"


def _run_importer():
    """Run the importer module in-process and call its main()."""
    try:
        with open(IMPORTER, encoding="utf-8") as fh:
            code = fh.read()
        g = {"__name__": "adastrea_import", "os": os, "unreal": unreal}
        exec(compile(code, IMPORTER, "exec"), g)
        main = g.get("main")
        if main:
            main()
        else:
            unreal.log_error("AdastreaShips: importer has no main()")
    except Exception:
        unreal.log_error("AdastreaShips import failed:")
        unreal.log_error(traceback.format_exc())


def _rescan():
    n_mesh = n_tex = 0
    if os.path.isdir(GEN_DIR):
        n_mesh = len([f for f in os.listdir(GEN_DIR) if f.endswith(".fbx")])
        td = os.path.join(GEN_DIR, "Textures")
        if os.path.isdir(td):
            n_tex = len([f for f in os.listdir(td) if f.endswith(".png")])
    unreal.log(f"AdastreaShips: {GEN_DIR}")
    unreal.log(f"AdastreaShips: {n_mesh} meshes, {n_tex} texture maps ready to import")


# ---------------------------------------------------------------------------
# Python Script Plugin hook points
# ---------------------------------------------------------------------------

def register():
    unreal.log("AdastreaShips: registering editor menu")
    # bind the python callables to command names
    unreal.register_named_function(CMD_IMPORT, _run_importer)
    unreal.register_named_function(CMD_RESCAN, _rescan)
    # menu: "Adastrea Ships" under the main level-menu bar
    unreal.register_menu(
        "MainFrame",
        "AdastreaShips",
        "Adastrea Ships",
        "Modular space asset kit: import generated meshes/textures/materials",
        "AdastreaShips_Noop",
        unbind_first=True,
    )
    unreal.register_menu(
        "MainFrame.AdastreaShips",
        "ImportAll",
        "Import All Assets",
        "Materialize all FBX meshes, textures, and materials",
        CMD_IMPORT,
        unbind_first=True,
    )
    unreal.register_menu(
        "MainFrame.AdastreaShips",
        "Rescan",
        "Rescan Generator Output",
        "Count meshes/textures under Assets/FBX/generated",
        CMD_RESCAN,
        unbind_first=True,
    )
    unreal.log("AdastreaShips: menu ready (Tools-style menu bar)")


def unregister():
    try:
        unreal.unregister_menu("MainFrame.AdastreaShips", "ImportAll")
        unreal.unregister_menu("MainFrame.AdastreaShips", "Rescan")
        unreal.unregister_menu("MainFrame", "AdastreaShips")
    except Exception:
        pass
    unreal.log("AdastreaShips: menu unregistered")


if __name__ == "__main__":
    register()
    unreal.log("AdastreaShips: registration hot-run complete")
