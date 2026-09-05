"""
Station-info verification: find an existing space station in the level (with its
editor-placed modules already attached via AddModule) and read the live aggregate
values the info screen draws. No spawning needed.
"""
import json
import os
import unreal

OUT = os.path.join(unreal.Paths.project_dir(), "Saved", "_stationinfo_verify.json")
result = {"ok": False, "checks": {}, "error": ""}
try:
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    stations = [a for a in actors if a and "SpaceStation" in a.get_class().get_name()]
    if not stations:
        # Spawn one and rely on its own modules? No modules -> adds nothing.
        result["error"] = "no stations in level"
        result["checks"]["stations_found"] = 0
    else:
        st = stations[0]
        result["checks"]["stations_found"] = len(stations)
        result["checks"]["station_name"] = str(st.get_actor_label())
        result["checks"]["power_gen"] = float(st.call_method("GetTotalPowerGeneration"))
        result["checks"]["power_cons"] = float(st.call_method("GetTotalPowerConsumption"))
        result["checks"]["balance"] = float(st.call_method("GetPowerBalance"))
        result["checks"]["shield_cur"] = float(st.call_method("GetTotalCurrentShieldStrength"))
        result["checks"]["shield_max"] = float(st.call_method("GetTotalShieldStrength"))
        result["checks"]["residents"] = int(st.call_method("GetTotalResidents"))
        result["checks"]["crew"] = int(st.call_method("GetTotalCrewCapacity"))
        result["checks"]["cargo_stored"] = int(st.call_method("GetTotalCargoStored"))
        result["checks"]["cargo_cap"] = int(st.call_method("GetTotalStorageCapacity"))
        result["checks"]["fuel_stored"] = float(st.call_method("GetTotalFuelStored"))
        result["checks"]["fuel_cap"] = float(st.call_method("GetTotalFuelCapacity"))
        result["checks"]["markets_open"] = int(st.call_method("GetOpenMarketplaceCount"))
        result["checks"]["markets_total"] = int(st.call_method("GetTotalMarketplaceCount"))
        result["checks"]["docking"] = int(st.call_method("GetTotalDockingPoints"))
        result["checks"]["has_docking"] = bool(st.call_method("HasDockingCapability"))
        result["checks"]["has_market"] = bool(st.call_method("HasMarketplace"))
        result["checks"]["has_cargo"] = bool(st.call_method("HasCargoStorage"))
        result["checks"]["reactor_out"] = float(st.call_method("GetTotalReactorOutput"))
        result["checks"]["solar_out"] = float(st.call_method("GetTotalSolarOutput"))
        result["ok"] = True
except Exception as ex:
    result["error"] = "exception: " + repr(ex)

with open(OUT, "w", encoding="utf-8") as fh:
    json.dump(result, fh, indent=2, default=str)
print("STATIONINFO_VERIFY ->", OUT, "ok=", result["ok"], "err=", result["error"])
print(json.dumps(result.get("checks", {}), indent=2))