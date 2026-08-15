#!/usr/bin/env python3
"""
Adastrea UE 5.8 MCP helper.
Manages the JSON-RPC 2.0 HTTP session with the Unreal Engine MCP server and
makes it trivial to list toolsets / call tools from the command line.

Usage:
  python ue_mcp.py list_toolsets
  python ue_mcp.py describe <toolset_name>
  python ue_mcp.py call <tool_name> '<json args>' [toolset_name]
Examples:
  python ue_mcp.py call EditorToolset.EditorAppToolset.IsPIERunning '{}'
  python ue_mcp.py call EditorToolset.EditorAppToolset.StartPIE \
      '{"options":{"bSimulate":false,"playMode":"PlayMode_InViewPort","warmupSeconds":2}}'
"""
import json
import subprocess
import sys
import urllib.request

ENDPOINT = "http://127.0.0.1:8000/mcp"


def _post(payload, session_id=None):
    headers = {"Content-Type": "application/json", "Accept": "application/json"}
    if session_id:
        headers["Mcp-Session-Id"] = session_id
    req = urllib.request.Request(
        ENDPOINT,
        data=json.dumps(payload).encode("utf-8"),
        headers=headers,
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=20) as resp:
            sid = resp.headers.get("Mcp-Session-Id")
            body = resp.read().decode("utf-8")
            return sid, json.loads(body)
    except Exception as e:
        return None, {"jsonrpc": "2.0", "id": None, "error": {"message": str(e)}}


def get_session():
    """Initialize and return a session id."""
    sid, resp = _post({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                   "clientInfo": {"name": "adastrea-cli", "version": "1.0"}},
    })
    if not sid:
        print("ERROR: MCP server not reachable. Is UE 5.8 running with MCP enabled?")
        print(json.dumps(resp, indent=2))
        sys.exit(1)
    return sid


def call(method, params, sid):
    _, resp = _post({"jsonrpc": "2.0", "id": 99, "method": method, "params": params}, sid)
    return resp


def extract_text(resp):
    """Return the main text content from an MCP tools/call result, or the error."""
    if "error" in resp:
        return "ERROR: " + json.dumps(resp["error"])
    content = resp.get("result", {}).get("content", [])
    texts = [c.get("text", "") for c in content if c.get("type") == "text"]
    return "\n".join(texts) if texts else json.dumps(resp.get("result", {}))


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(0)

    cmd = sys.argv[1]
    sid = get_session()

    if cmd == "list_toolsets":
        resp = call("tools/call", {"name": "list_toolsets", "arguments": {}}, sid)
    elif cmd == "describe":
        ts = sys.argv[2]
        resp = call("tools/call", {"name": "describe_toolset", "arguments": {"toolset_name": ts}}, sid)
    elif cmd == "call":
        tool = sys.argv[2]
        args = json.loads(sys.argv[3]) if len(sys.argv) > 3 else {}
        specified_ts = sys.argv[4] if len(sys.argv) > 4 else None
        # If tool name is fully qualified "A.B.C", infer toolset_name as "A.B"
        toolset_name = specified_ts
        if not specified_ts and "." in tool:
            toolset_name, _, tool = tool.rpartition(".")
        resp = call("tools/call", {"name": "call_tool",
                                   "arguments": {"tool_name": tool,
                                                 "toolset_name": toolset_name,
                                                 "arguments": args}}, sid)
    else:
        print(f"Unknown command: {cmd}")
        sys.exit(1)

    out = extract_text(resp)
    print(out)
    if out.startswith("ERROR"):
        sys.exit(1)


if __name__ == "__main__":
    main()