#!/usr/bin/env python3
"""
BlenderKit fetch tool (headless, no bpy/add-on required).

Searches the public BlenderKit API and downloads free-licensed assets
(royalty_free / cc_zero only) into a local, gitignored library cache, with a
tracked manifest recording attribution for every asset pulled in.

This is deliberately separate from the Blender add-on: the rest of the
Adastrea content pipeline runs headless (`blender -b --python ...`), and the
add-on's asset browser is an interactive-GUI-only workflow. Hitting the REST
API directly keeps BlenderKit usable from the same headless scripts.

Setup:
    Put BLENDERKIT_API_KEY=<your key> in a .env file at the repo root
    (.env is gitignored; see .env.template). Get a key at
    https://www.blenderkit.com/ -> profile -> API key.

Usage:
    python Tools/blenderkit_fetch.py search "sci-fi control panel"
    python Tools/blenderkit_fetch.py fetch "sci-fi control panel" --index 0

Output:
    Raw downloads:  Assets/BlenderKit/library/<assetBaseId>.<ext>   (gitignored)
    Manifest:       Assets/BlenderKit/manifest.json                (tracked)

The manifest is the license/attribution record — don't delete entries for
assets still used in Content/. Only royalty_free and cc_zero licensed assets
are ever fetched (matches the precedent set on the AGENT_BOARD 2026-09-10
BlenderKit review: every asset pulled in must have a verified game-safe
license).
"""
import argparse
import json
import os
import sys
import uuid
import urllib.request
import urllib.error
import urllib.parse

try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENV_PATH = os.path.join(REPO_ROOT, ".env")
LIBRARY_DIR = os.path.join(REPO_ROOT, "Assets", "BlenderKit", "library")
MANIFEST_PATH = os.path.join(REPO_ROOT, "Assets", "BlenderKit", "manifest.json")

API_BASE = "https://www.blenderkit.com/api/v1"
ALLOWED_LICENSES = {"royalty_free", "cc_zero"}
FILETYPE_PREFERENCE = ("blend", "gltf")


def load_api_key():
    key = os.environ.get("BLENDERKIT_API_KEY")
    if key:
        return key
    if os.path.isfile(ENV_PATH):
        with open(ENV_PATH, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                k, _, v = line.partition("=")
                if k.strip() == "BLENDERKIT_API_KEY":
                    return v.strip()
    return None


def _request(url, api_key=None, headers=None):
    req_headers = {"Accept": "application/json"}
    if headers:
        req_headers.update(headers)
    if api_key:
        req_headers["Authorization"] = f"Bearer {api_key}"
    req = urllib.request.Request(url, headers=req_headers)
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read().decode("utf-8"))


def search(query, asset_type="model", free_only=True, limit=24):
    q = query
    if asset_type:
        q += f" asset_type:{asset_type}"
    if free_only:
        q += " is_free:true"
    url = f"{API_BASE}/search/?query={urllib.parse.quote(q)}"
    data = _request(url)
    results = data.get("results", [])
    return results[:limit]


def pick_file(result, prefer=FILETYPE_PREFERENCE):
    files = {f["fileType"]: f for f in result.get("files", [])}
    for ft in prefer:
        if ft in files:
            return files[ft]
    return None


def resolve_download_url(download_endpoint, api_key):
    scene_uuid = str(uuid.uuid4())
    sep = "&" if "?" in download_endpoint else "?"
    url = f"{download_endpoint}{sep}scene_uuid={scene_uuid}"
    data = _request(url, api_key=api_key)
    if "filePath" not in data:
        raise RuntimeError(f"download not resolved: {data}")
    return data["filePath"]


def stream_download(url, dest_path):
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "Adastrea-BlenderKit-Fetch"})
    with urllib.request.urlopen(req, timeout=120) as resp, open(dest_path, "wb") as out:
        total = int(resp.headers.get("Content-Length", 0))
        read = 0
        chunk = 1024 * 256
        while True:
            buf = resp.read(chunk)
            if not buf:
                break
            out.write(buf)
            read += len(buf)
            if total:
                pct = 100 * read / total
                print(f"\r  downloading... {read/1e6:.1f}/{total/1e6:.1f} MB ({pct:.0f}%)", end="")
        print()
    return dest_path


def load_manifest():
    if os.path.isfile(MANIFEST_PATH):
        with open(MANIFEST_PATH, "r", encoding="utf-8") as f:
            return json.load(f)
    return {}


def save_manifest(manifest):
    os.makedirs(os.path.dirname(MANIFEST_PATH), exist_ok=True)
    with open(MANIFEST_PATH, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)


def fetch_asset(result, api_key, prefer=FILETYPE_PREFERENCE):
    license_ = result.get("license")
    if license_ not in ALLOWED_LICENSES:
        raise ValueError(
            f"refusing to fetch '{result['name']}': license '{license_}' not in "
            f"allowed set {sorted(ALLOWED_LICENSES)} (game-safe only)"
        )
    file_entry = pick_file(result, prefer=prefer)
    if file_entry is None:
        raise ValueError(f"no usable file type ({prefer}) for '{result['name']}'")

    asset_base_id = result["assetBaseId"]
    ext = file_entry["fileType"]
    dest = os.path.join(LIBRARY_DIR, f"{asset_base_id}.{ext}")

    if os.path.isfile(dest):
        print(f"  already cached: {dest}")
    else:
        signed_url = resolve_download_url(file_entry["downloadUrl"], api_key)
        print(f"  fetching '{result['name']}' ({license_}) -> {dest}")
        stream_download(signed_url, dest)

    manifest = load_manifest()
    manifest[asset_base_id] = {
        "name": result["name"],
        "author": result.get("author", {}).get("fullName"),
        "license": license_,
        "asset_type": result.get("assetType"),
        "source_url": f"https://www.blenderkit.com/asset-gallery-detail/{asset_base_id}/",
        "file_type": ext,
        "local_path": os.path.relpath(dest, REPO_ROOT).replace(os.sep, "/"),
        "tags": result.get("tags", []),
    }
    save_manifest(manifest)
    return dest


def cmd_search(args):
    results = search(args.query, asset_type=args.type, free_only=not args.all_licenses, limit=args.limit)
    if not results:
        print("no results")
        return
    for i, r in enumerate(results):
        flag = "" if r.get("license") in ALLOWED_LICENSES else "  [SKIP: non-free license]"
        print(f"[{i}] {r['name']}  ({r.get('license')}, {r.get('assetType')}){flag}")


def cmd_fetch(args):
    api_key = load_api_key()
    if not api_key:
        print("ERROR: BLENDERKIT_API_KEY not set (add it to .env)", file=sys.stderr)
        sys.exit(1)
    results = search(args.query, asset_type=args.type, free_only=not args.all_licenses, limit=args.index + 1)
    if len(results) <= args.index:
        print(f"ERROR: only {len(results)} result(s), index {args.index} out of range", file=sys.stderr)
        sys.exit(1)
    result = results[args.index]
    try:
        dest = fetch_asset(result, api_key)
    except ValueError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
    print(f"OK: {dest}")
    print("Next: convert to FBX with Tools/blenderkit_import.py, e.g.")
    print(f'  blender -b --python Tools/blenderkit_import.py -- "{dest}" SM_Prop_<Name>')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sp = sub.add_parser("search", help="search BlenderKit, list results (no download)")
    sp.add_argument("query")
    sp.add_argument("--type", default="model")
    sp.add_argument("--limit", type=int, default=10)
    sp.add_argument("--all-licenses", action="store_true", help="also show non-free-licensed results (still won't fetch them)")
    sp.set_defaults(func=cmd_search)

    fp = sub.add_parser("fetch", help="search + download one result's source file")
    fp.add_argument("query")
    fp.add_argument("--type", default="model")
    fp.add_argument("--index", type=int, default=0, help="which search result to fetch (default 0 = top match)")
    fp.add_argument("--all-licenses", action="store_true", help="allow searching non-free results (fetch still refuses non-royalty_free/cc_zero)")
    fp.set_defaults(func=cmd_fetch)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
