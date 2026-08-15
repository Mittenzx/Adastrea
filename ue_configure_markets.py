#!/usr/bin/env python3
"""Configure + populate the 5 MVP markets via MCP."""
import json
import subprocess
import sys
import urllib.request

ENDPOINT = "http://127.0.0.1:8000/mcp"

def _post(payload, session_id=None):
    h = {"Content-Type": "application/json", "Accept": "application/json"}
    if session_id:
        h["Mcp-Session-Id"] = session_id
    req = urllib.request.Request(ENDPOINT, data=json.dumps(payload).encode(),
                                 headers=h, method="POST")
    with urllib.request.urlopen(req, timeout=30) as r:
        sid = r.headers.get("Mcp-Session-Id")
        return sid, json.loads(r.read().decode())

def get_session():
    sid, _ = _post({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                    "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                               "clientInfo": {"name": "mkt", "version": "1"}}})
    return sid

def cset(sid, ts, tool, args):
    _, resp = _post({"jsonrpc": "2.0", "id": 9, "method": "tools/call", "params": {
        "name": "call_tool",
        "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args}}}, sid)
    text = resp.get("result", {}).get("content", [{}])[0].get("text", "")
    if resp.get("result", {}).get("isError"):
        raise RuntimeError(text)
    return text

def set_market(sid, asset_name, props, inv):
    path = f"/Game/DataAssets/Trading/Markets/{asset_name}"
    inst = {"refPath": f"{path}.{asset_name}"}
    # 1) basic props (scalars - fine)
    cset(sid, "editor_toolset.toolsets.object.ObjectTools", "set_properties",
         {"instance": inst, "values": json.dumps(props)})
    # 2) grow placeholders
    ph = [{"tradeItem": None, "currentStock": 0, "maxStock": 0, "supplyLevel": 1.0,
           "demandLevel": 1.0, "lastTradePrice": 0.0, "bInStock": False} for _ in inv]
    cset(sid, "editor_toolset.toolsets.object.ObjectTools", "set_properties",
         {"instance": inst, "values": json.dumps({"inventory": ph})})
    # 3) fill real
    cset(sid, "editor_toolset.toolsets.object.ObjectTools", "set_properties",
         {"instance": inst, "values": json.dumps({"inventory": inv})})
    # 4) save
    cset(sid, "editor_toolset.toolsets.asset.AssetTools", "save_assets",
         {"asset_paths": [path]})
    print(f"OK {asset_name}: {len(inv)} items")

def main():
    sid = get_session()
    with open("/tmp/markets.json") as f:
        markets = json.load(f)
    for name, cfg in markets.items():
        asset_name = "DA_Market_" + name
        inv = []
        for ti, cur, mx, sup, dem in cfg["inv"]:
            inv.append({"tradeItem": ti, "currentStock": cur, "maxStock": mx,
                        "supplyLevel": sup, "demandLevel": dem,
                        "lastTradePrice": 0.0, "bInStock": True})
        set_market(sid, asset_name, cfg["props"], inv)

if __name__ == "__main__":
    main()