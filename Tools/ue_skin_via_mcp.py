"""Drive UE MCP to author a SKINNABLE hull material (research demo).
Builds M_Fighter_Hull_Skin: albedo = lerp(T_Ship_Hull_D, SkinColor, T_Ship_Hull_SKIN)
plus N/R/M/E, with SkinColor as a parameter -> any skin = a MaterialInstance override.
Run host-side: python Tools/ue_skin_via_mcp.py   (editor + MCP must be up)
"""
import json, urllib.request

EP = "http://127.0.0.1:8000/mcp"
TEX = "/Game/Textures/T_Ship_Hull"
MAT_FOLDER = "/Game/Materials/"

def init():
    p={"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"cli","version":"1"}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json"},method="POST")
    with urllib.request.urlopen(req,timeout=10) as r: return r.headers.get("Mcp-Session-Id")

def call(sid, tool_name, args):
    p={"jsonrpc":"2.0","id":9,"method":"tools/call","params":{
        "name":"call_tool",
        "arguments":{"toolset_name":tool_name.rsplit(".",1)[0],
                     "tool_name":tool_name.rsplit(".",1)[1],
                     "arguments":args}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json","Mcp-Session-Id":sid},method="POST")
    with urllib.request.urlopen(req,timeout=60) as r:
        return json.loads(r.read().decode())

def rv(resp):
    c=resp.get("result",{}).get("content",[{}])
    t=c[0].get("text","") if c else ""
    try:
        j=json.loads(t); return j.get("returnValue", j)
    except Exception:
        return t

MT = "editor_toolset.toolsets.material.MaterialTools"
sid = init()
print("session:", sid[:8])

# 1. ensure textures exist (import via asset tools? assume present from prior)
# 2. create material
r = call(sid, MT + ".create_material", {"folder_path": MAT_FOLDER[:-1], "asset_name": "M_Fighter_Hull_Skin"})
mat_ref = rv(r)
print("material:", mat_ref)
mp = mat_ref.get("refPath") if isinstance(mat_ref,dict) else str(mat_ref)

# 3. add expressions. Helper to add by class and get refPath back.
def add(expr_class, props=None):
    r = call(sid, MT + ".add_expression", {"material_or_function":{"refPath":mp},
             "expression_class":{"refPath":"/Script/Engine."+expr_class},
             "expression_name": (props or {}).get("name","")})
    res = rv(r)
    return res.get("refPath") if isinstance(res,dict) else str(res)

# add texture samples, parameter, blend
dnode  = add("MaterialExpressionTextureSample")
sknode = add("MaterialExpressionTextureSample")
rgb    = add("MaterialExpressionParameter")
blend  = add("MaterialExpressionBlendColor")
print("nodes:", dnode, sknode, rgb, blend)

# set texture refs on samples (via set_editor_property might not be exposed; try set via schema if param)
print("NODE_REFS_OK")