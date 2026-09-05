#!/usr/bin/env python3
"""
Live PIE Interior Test

Verifies that every interior mesh in the Adastrea kit loads onto the real
ASpaceshipInterior actor in a running Play-In-Editor session. This exercises the
actual game component (UStaticMeshComponent.InteriorMesh) rather than just the
asset files on disk.

Requires:
  - UE 5.8 editor running with MCP on http://127.0.0.1:8000/mcp
  - A PIE session active with the player ship + its SpaceshipInterior_0 actor

Usage:
  python Tools/test_interiors_live.py        # prints per-interior PASS/FAIL

Exit code: 0 if all interiors resolve, 1 otherwise.
"""

import json
import subprocess
import sys
import time

INTERIOR_MESH_REF = "/Game/Maps/UEDPIE_0_TestLevel.TestLevel:PersistentLevel.SpaceshipInterior_0.InteriorMesh"
RESTORE_MESH = "SM_Int_Fighter_Cockpit"
SUBDIR = "/AdastreaShips/Meshes/Interiors"

# The full 30-mesh interior kit (shells + companion parts + singletons).
INTERIORS = [
    "SM_Int_Fighter_Cockpit",
    "SM_Int_CommandBridge_Shell", "SM_Int_CommandBridge_Console", "SM_Int_CommandBridge_Deck",
    "SM_Int_CommandBridge_Hatch", "SM_Int_CommandBridge_Lights", "SM_Int_CommandBridge_Stations",
    "SM_Int_CommandBridge_Viewport",
    "SM_Int_Freighter_CrewQuarters", "SM_Int_Freighter_CrewQuarters_Bunks",
    "SM_Int_Freighter_CrewQuarters_Desks", "SM_Int_Freighter_CrewQuarters_Galley",
    "SM_Int_Freighter_CrewQuarters_Hatch", "SM_Int_Freighter_CrewQuarters_Lights",
    "SM_Int_Freighter_CrewQuarters_Mess", "SM_Int_Freighter_CrewQuarters_Shell",
    "SM_Int_Freighter_CrewQuarters_Vents", "SM_Int_Freighter_EngineRoom",
    "SM_Int_Generationship_Hab", "SM_Int_Generationship_Hab_Bunks",
    "SM_Int_Generationship_Hab_Desks", "SM_Int_Generationship_Hab_Galley",
    "SM_Int_Generationship_Hab_Hatch", "SM_Int_Generationship_Hab_Lights",
    "SM_Int_Generationship_Hab_Mess", "SM_Int_Generationship_Hab_Shell",
    "SM_Int_Generationship_Hab_Vents",
    "SM_Int_Standard_Airlock", "SM_Int_Standard_Corridor", "SM_Int_Xenomorph_AlienHold",
]


def call(tool, payload):
    r = subprocess.run(["python", "ue_mcp.py", "call", tool, json.dumps(payload)],
                       capture_output=True, text=True)
    return r.stdout.strip()


def get_mesh_ref():
    r = call("editor_toolset.toolsets.object.ObjectTools.get_properties",
             {"instance": {"refPath": INTERIOR_MESH_REF}, "properties": ["staticMesh"]})
    try:
        top = json.loads(r)["returnValue"]
        inner = json.loads(top) if isinstance(top, str) else top
        return inner.get("staticMesh", {}).get("refPath", "")
    except Exception:
        return "PARSE_ERR"


def set_mesh(name):
    payload = {"staticMesh": {"refPath": SUBDIR + "/" + name + "." + name}}
    r = call("editor_toolset.toolsets.object.ObjectTools.set_properties",
             {"instance": {"refPath": INTERIOR_MESH_REF}, "values": json.dumps(payload)})
    return "true" in r.lower()


def main():
    passed = 0
    failed = []
    for name in INTERIORS:
        set_ok = set_mesh(name)
        time.sleep(0.2)
        after = get_mesh_ref()
        ok = set_ok and after.endswith("." + name)
        if ok:
            passed += 1
            print(f"[PASS] {name}")
        else:
            failed.append(name)
            print(f"[FAIL] {name}: set_ok={set_ok} after={after[:70]}")

    # Restore the ship's real interior.
    set_mesh(RESTORE_MESH)
    print(f"\n=== LIVE PIE INTERIOR TEST: {len(INTERIORS)} tested, "
          f"{passed} passed, {len(failed)} failed ===")
    if failed:
        print("FAILED:", failed)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())