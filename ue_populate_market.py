#!/usr/bin/env python3
"""
Populate a UE 5.8 MarketDataAsset's inventory via MCP.

The ObjectTools.set_properties tool cannot change an array's SIZE and CONTENTS
simultaneously ("ArrayAdd: elements changed alongside the size change").
Workaround: first grow the array to N placeholder (empty) entries, then write
the real data in (size is stable, so it's a pure content update).

Usage:
    python ue_populate_market.py <market_asset_path> '<json_inventory>'

Example:
    python ue_populate_market.py /Game/DataAssets/Trading/Markets/DA_Market_FreePort \
      '[{"tradeItem":{"refPath":"/Game/DataAssets/Trading/Items/DA_TradeItem_FoodRations.DA_TradeItem_FoodRations"},"currentStock":5000,"maxStock":20000,"supplyLevel":1.2,"demandLevel":1.0,"lastTradePrice":0.0,"bInStock":true}]'
"""
import json
import sys
import urllib.request

ENDPOINT = "http://127.0.0.1:8000/mcp"


def _post(payload, session_id=None):
    headers = {"Content-Type": "application/json", "Accept": "application/json"}
    if session_id:
        headers["Mcp-Session-Id"] = session_id
    req = urllib.request.Request(
        ENDPOINT, data=json.dumps(payload).encode("utf-8"),
        headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=30) as resp:
        sid = resp.headers.get("Mcp-Session-Id")
        return sid, json.loads(resp.read().decode("utf-8"))


def get_session():
    sid, _ = _post({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                    "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                               "clientInfo": {"name": "market-pop", "version": "1"}}})
    return sid


def call_tool(sid, ts, tool, args):
    _, resp = _post({"jsonrpc": "2.0", "id": 9, "method": "tools/call", "params": {
        "name": "call_tool",
        "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args}}}, sid)
    text = resp.get("result", {}).get("content", [{}])[0].get("text", "")
    if resp.get("result", {}).get("isError"):
        raise RuntimeError(text)
    return text


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    asset_path = sys.argv[1]
    inv = json.loads(sys.argv[2])
    instance = {"refPath": f"{asset_path}.{asset_path.rsplit('/', 1)[-1]}"}

    sid = get_session()

    # 1) Grow to N placeholders
    placeholders = [{"tradeItem": None, "currentStock": 0, "maxStock": 0,
                     "supplyLevel": 1.0, "demandLevel": 1.0,
                     "lastTradePrice": 0.0, "bInStock": False} for _ in inv]
    call_tool(sid, "editor_toolset.toolsets.object.ObjectTools", "set_properties",
              {"instance": instance, "values": json.dumps({"inventory": placeholders})})
    print("placeholders set")

    # 2) Write real data (stable size)
    call_tool(sid, "editor_toolset.toolsets.object.ObjectTools", "set_properties",
              {"instance": instance, "values": json.dumps({"inventory": inv})})
    print(f"inventory populated ({len(inv)} entries)")

    # 3) Save
    call_tool(sid, "editor_toolset.toolsets.asset.AssetTools", "save_assets",
              {"asset_paths": [asset_path]})
    print("saved")


if __name__ == "__main__":
    main()