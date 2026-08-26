"""Startup runner for the AdastreaShips importer (invoked via -ExecutePythonScript).

Runs the importer module's main() inside the editor's Python so the generated
FBX/textures are materialized as .uasset content. Mirrors how the plugin's menu
'register' script exec's it (unreal + os injected into the module globals).
"""
import os
import unreal

try:
    importer = r"C:\Users\akuma\Adastrea\Plugins\AdastreaShips\ImportAdastreaShips.py"
    with open(importer, encoding="utf-8") as fh:
        code = fh.read()
    g = {"__name__": "adastrea_import", "os": os, "unreal": unreal}
    exec(compile(code, importer, "exec"), g)
    main = g.get("main")
    if main:
        main()
    unreal.log("=== AdastreaShips startup importer ran ===")
except Exception as e:
    unreal.log_error("AdastreaShips import failed: %s" % e)
    import traceback
    unreal.log_error(traceback.format_exc())
