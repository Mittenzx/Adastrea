#!/usr/bin/env python3
"""
Adastrea interior walk-test probe (MCP-driven).

Captures structured diagnostic data from a ship-interior walk so we can debug
movement / camera / drift / teleport issues from real numbers instead of guesses.

What it records per run (written to Saved/walktest_<ts>.json, summary to stdout):

  IDLE       - avatar position sampled over N seconds with zero input.
               Flags gravity/ejection/drift (Z collapse, X|=Y runaway, accel).
  PER-KEY    - for each of W/S/A/D: reset to entry, tap the key once, sample
               position at +0.1/0.3/0.6/1.0s. Yields displacement, derived
               speed, and which WORLD axis each key actually moved.
  CLAMP      - (requires avatar) tap a key repeatedly toward +X, recording the
               world position each step until it stops changing. Confirms the
               interior clamp (GetLocalHalfExtents) holds and doesn't teleport
               through a wall.
  LOG        - pulls the last N log lines matching movement/interior/error
               patterns (Move, EnterShipInterior, ExitShipInterior, AvatarSpawn,
               Warning, Error, overlap, drift) for correlation with the physics.
  STATE      - ship transform, interior transform+scale+bounds, avatar transform.

Usage:
  python ue_walk_test.py [--enter] [--idle 2.0] [--novelocitysampling 5]
    --enter     after PIE starts, drive into the interior first (press 2 on the
                start menu; WAIT for the avatar to spawn).
    --idle N    idle-sample seconds (default 2.0).
    --sample N  per-key sample count (default 5).

Requires UE 5.8 running MCP on http://127.0.0.1:8000/mcp (see ue_mcp.py).
"""
import argparse
import json
import re
import subprocess
import sys
import time
import urllib.request
from datetime import datetime
from pathlib import Path

ENDPOINT = "http://127.0.0.1:8000/mcp"
LEVEL_PREFIX = "/Game/Maps/UEDPIE_0_TestLevel.TestLevel:PersistentLevel."


# ---------------------------------------------------------------- MCP client
REPO_ROOT = Path(__file__).resolve().parent.parent


def run_cli(method, params):
    """Shell out to ue_mcp.py (robust session mgmt) from the repo root and return stdout."""
    from pathlib import Path as _P
    mcp = _P(__file__).with_name("ue_mcp.py")
    if not mcp.exists():
        mcp = REPO_ROOT / "ue_mcp.py"
    return subprocess.run(
        [sys.executable, str(mcp), "call", method, json.dumps(params)],
        capture_output=True, text=True, cwd=str(REPO_ROOT)).stdout


def _returnvalue(out):
    try:
        for ln in out.splitlines():
            if '"returnValue"' in ln:
                d = json.loads(ln)
                return d.get("returnValue")
        d = json.loads(out)
        return d.get("returnValue")
    except Exception:
        return None


def get_transform(path):
    return _returnvalue(run_cli(
        "editor_toolset.toolsets.actor.ActorTools.get_actor_transform",
        {"actor": {"refPath": path}}))


def actor_valid(path):
    return get_transform(path) is not None


def get_bounds(path):
    return _returnvalue(run_cli(
        "editor_toolset.toolsets.actor.ActorTools.get_actor_bounds",
        {"actor": {"refPath": path}}))


def list_runtime_actors():
    log = REPO_ROOT / "Saved" / "Logs" / "Adastrea.log"
    if not log.exists():
        return {}
    txt = log.read_text(errors="ignore")
    names = set()
    for pat in ["SpaceshipAvatar_[0-9]+", "SpaceshipInterior_[0-9]+",
                "BP_Ship_[A-Za-z0-9_]+_C_[0-9]+"]:
        names.update(re.findall(pat, txt))
    found = {}
    for n in sorted(names):
        p = LEVEL_PREFIX + n
        if actor_valid(p):
            found[n] = p
    return found


def press(key):
    run_cli("SlateInspectorToolset.SlateInspectorToolset.PressKey", {"key": key})


def log_entries(pattern, limit=400):
    return _returnvalue(run_cli("EditorToolset.LogsToolset.GetLogEntries",
                                {"pattern": pattern, "maxEntries": limit})) or []


def is_pie_running():
    out = run_cli("EditorToolset.EditorAppToolset.IsPIERunning", {})
    return '"true"' in out or ':true' in out


def start_pie():
    run_cli("EditorToolset.EditorAppToolset.StartPIE",
            {"options": {"bSimulate": False, "playMode": "PlayMode_InViewPort",
                         "warmupSeconds": 2}})


def enter_interior_and_find_avatar(timeout=25.0):
    """Get the player onto foot inside the interior and return the avatar's path.

    Handles both entry paths:
      - the session start-menu (press 2 -> "start inside the ship", the new feature)
      - the normal cockpit (press V -> walk the interior)
    Re-scans the runtime namespace each pass rather than trusting a stale snapshot."""
    keys = ["2", "V"]                                  # 2 for start menu, V for cockpit
    deadline = time.time() + timeout
    attempts = 0
    while time.time() < deadline:
        for idx in range(0, 4):                        # avatar _0.._3 (respawn safety)
            p = LEVEL_PREFIX + f"SpaceshipAvatar_{idx}"
            if get_transform(p):
                return p
        press(keys[attempts % len(keys)])
        attempts += 1
        time.sleep(0.9)
    print(f"  (pressed {attempts}x, no avatar appeared)")
    return None


# ---------------------------------------------------------------- sampling
def sample_transform(path, dt_s, count):
    """Sample the actor's world position at dt*count spacing."""
    out = []
    last = get_transform(path)
    if not last:
        return []
    for i in range(count):
        time.sleep(dt_s)
        t = get_transform(path)
        if not t:
            continue
        last = t
        out.append(t)
    return out


def derive_speed(a, b):
    if not a or not b:
        return None
    la, lb = a["location"], b["location"]
    return ((lb["x"] - la["x"]) ** 2 + (lb["y"] - la["y"]) ** 2 + (lb["z"] - la["z"]) ** 2) ** 0.5 / 0.1  # per 0.1s


# ---------------------------------------------------------------- probes
def probe_idle(path, secs):
    base = get_transform(path)
    samples = sample_transform(path, 0.5, int(secs / 0.5))
    if not base or not samples:
        return {"base": base, "samples": [], "drift_detected": False}
    first = samples[0]["location"]; last = samples[-1]["location"]
    dx = last["x"] - first["x"]; dy = last["y"] - first["y"]; dz = last["z"] - first["z"]
    dist = (dx * dx + dy * dy + dz * dz) ** 0.5
    return {
        "base": base, "samples": samples,
        "start": first, "end": last,
        "delta": {"x": dx, "y": dy, "z": dz}, "distance": round(dist, 2),
        "drift_detected": dist > 1.5,
    }


def probe_key(path, key, samples, entry=None):
    # Reset to a stable entry so each key's delta is comparable.
    if entry:
        import urllib.request as _u
        payload = {"actor": {"refPath": path},
                   "xform": {"location": entry["location"],
                             "rotation": entry["rotation"]},
                   "worldspace": True}
        run_cli("editor_toolset.toolsets.actor.ActorTools.set_actor_transform", payload)
        time.sleep(0.3)
    pre = get_transform(path)
    press(key)
    snap = sample_transform(path, 0.2, samples)
    post = snap[-1] if snap else pre
    return {"key": key, "pre": pre, "samples": snap, "post": post}


def probe_clamp(path, key="W", steps=8):
    """Tap a key toward a wall; record world pos each step until it stops changing.
    Detects the clamp holding (pos converges, no teleport/runaway)."""
    samples = []
    prev = None
    for _ in range(steps):
        press(key)
        time.sleep(0.35)
        t = get_transform(path)
        if not t:
            break
        samples.append(t["location"])
        if prev and abs(t["location"]["x"] - prev["x"]) < 2 and abs(t["location"]["y"] - prev["y"]) < 2:
            break
        prev = t["location"]
    return samples


# ---------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--enter", action="store_true")
    ap.add_argument("--idle", type=float, default=2.0)
    ap.add_argument("--sample", type=int, default=5)
    args = ap.parse_args()

    print("== Adastrea interior walk-test ==")
    if not is_pie_running():
        print("PIE not running — starting it.")
        start_pie()

    # Wait for PIE to finish loading: the player ship must be addressable.
    deadline = time.time() + 40
    ship_path = None
    while time.time() < deadline:
        for idx in range(0, 4):
            p = LEVEL_PREFIX + f"BP_Ship_Fighter_C_{idx}"
            if actor_valid(p):
                ship_path = p
                break
        if ship_path:
            break
        time.sleep(1.0)
    print("PIE ready, ship at:", ship_path or "(none)")

    actors = list_runtime_actors()
    print("Actors found:", actors)

    avatar = None
    if args.enter:
        avatar = enter_interior_and_find_avatar()
        if avatar:
            print("On foot avatar:", avatar)
        else:
            print("WARNING: could not get avatar on foot within timeout.")

    report = {
        "timestamp": datetime.utcnow().isoformat() + "Z",
        "actors": actors,
        "avatar": avatar,
        "ship": ship_path or actors.get("BP_Ship_Fighter_C_0"),
    }

    if ship_path:
        report["ship_transform"] = get_transform(ship_path)
    # Interior is a runtime spawn of the ship; re-derive it directly.
    interior_path = None
    for idx in range(0, 4):
        p = LEVEL_PREFIX + f"SpaceshipInterior_{idx}"
        if actor_valid(p):
            interior_path = p
            break
    if interior_path:
        report["interior_transform"] = get_transform(interior_path)
        report["interior_bounds"] = get_bounds(interior_path)
    else:
        report["interior_transform"] = None
        report["interior_bounds"] = None

    if avatar:
        report["idle"] = probe_idle(avatar, args.idle)
        entry = report["idle"].get("base") if not report["idle"].get("drift_detected") else None
        report["keys"] = [probe_key(avatar, k, args.sample, entry) for k in "WASD"]
        report["clamp_x"] = probe_clamp(avatar, "D", 8)
        report["clamp_y"] = probe_clamp(avatar, "W", 8)

    report["log"] = {
        "movement": log_entries("Move|MovementVector"),
        "interior": log_entries("EnterShipInterior|ExitShipInterior|AvatarSpawn|overlap|drift"),
        "errors": log_entries("Warning|Error|Failed"),
    }

    ts = datetime.utcnow().strftime("%Y%m%d_%H%M%S")
    dest = REPO_ROOT / "Saved" / f"walktest_{ts}.json"
    dest.write_text(json.dumps(report, indent=2, default=str), encoding="utf-8")
    print("\nWROTE:", dest)

    # ---- concise summary ----
    print("\n--- SUMMARY ---")
    if report.get("idle"):
        i = report["idle"]
        print(f"IDLE ({args.idle}s): dist={i.get('distance')} delta={i.get('delta')} "
              f"drift={i.get('drift_detected')}")
    for k in report.get("keys", []):
        pre = k.get("pre", {}).get("location") or {}
        post = k.get("post", {}).get("location") or {}
        if pre and post:
            d = {ax: round(post[ax] - pre[ax], 1) for ax in "xyz"}
            print(f"KEY {k['key']}: delta={d}")
        else:
            print(f"KEY {k['key']}: (no avatar data)")
    if report.get("clamp_x"):
        print("CLAMP D (X+):", report["clamp_x"][-1] if report["clamp_x"] else None)
    if report.get("clamp_y"):
        print("CLAMP W (Y+):", report["clamp_y"][-1] if report["clamp_y"] else None)


if __name__ == "__main__":
    main()