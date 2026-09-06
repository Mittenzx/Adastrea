"""Finish M_Fighter_Hull_Skin graph via MCP: wire Blend + samples to outputs, set
texture refs + SkinColor param default, recompile, save."""
import json, urllib.request
EP="http://127.0.0.1:8000/mcp"
MAT="/Game/Materials/M_Fighter_Hull_Skin.M_Fighter_Hull_Skin"
TEX="/Game/Textures/T_Ship_Hull"

def init():
    p={"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"cli","version":"1"}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json"},method="POST")
    with urllib.request.urlopen(req,timeout=10) as r: return r.headers.get("Mcp-Session-Id")
def call(sid,tool,args):
    ts,tn=tool.rsplit(".",1)
    p={"jsonrpc":"2.0","id":9,"method":"tools/call","params":{"name":"call_tool","arguments":{"toolset_name":ts,"tool_name":tn,"arguments":args}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json","Mcp-Session-Id":sid},method="POST")
    with urllib.request.urlopen(req,timeout=90) as r: return json.loads(r.read().decode())
def rv(resp):
    c=resp.get("result",{}).get("content",[{}]); t=c[0].get("text","") if c else ""
    try: return json.loads(t).get("returnValue")
    except Exception: return t or str(resp.get("error",""))[:200]
MT="editor_toolset.toolsets.material.MaterialTools"
AT="editor_toolset.toolsets.asset.AssetTools"
sid=init()
n=lambda s: MAT+":"+s
D,SK,PA,BL = n("MaterialExpressionTextureSample_0"),n("MaterialExpressionTextureSample_1"),n("MaterialExpressionParameter_0"),n("MaterialExpressionBlend_0")

# 1. set texture on sample nodes via ObjectTools.set_properties (texture property)
OT="editor_toolset.toolsets.object.ObjectTools"
for ref,tex in [(D,"T_Ship_Hull_D"),(SK,"T_Ship_Hull_SKIN")]:
    vals=json.dumps({"texture":{"refPath":TEX+"/"+tex+"."+tex}})
    r=call(sid, OT+".set_properties", {"instance":{"refPath":ref},"values":vals})
    print("set tex on", ref.split(":")[-1], "->", rv(r)[:60])

# 2. set SkinColor param default + name via set_properties
r=call(sid, OT+".set_properties", {"instance":{"refPath":PA},"values":json.dumps({"parameter_name":"SkinColor","default_value":{"r":0.9,"g":0.9,"b":0.95,"a":1.0}})})
print("set param SkinColor ->", rv(r)[:80])

# 3. connect: D->Blend.A, Param->Blend.B, SKIN->Blend.Alpha
for from_ref,out,to_ref,to_in in [(D,"RGB",BL,"A"),(PA,"",BL,"B"),(SK,"R",BL,"Alpha")]:
    r=call(sid, MT+".connect_expressions", {"from_expression":{"refPath":from_ref},"from_output_name":out,
            "to_expression":{"refPath":to_ref},"to_input_name":to_in})
    print("connect", from_ref.split(":")[-1], "->", to_in, ":", (rv(r) or "")[:60])

# 4. connect blend -> BaseColor output
r=call(sid, MT+".connect_to_output", {"expression":{"refPath":BL},"output_name":"RGB","material_property":"MP_BASE_COLOR"})
print("blend->BaseColor:", rv(r)[:80])

# 5. recompile + save
r=call(sid, MT+".recompile", {"material":{"refPath":MAT}})
print("recompile:", rv(r)[:80])
r=call(sid, AT+".save_assets", {})
print("save:", rv(r)[:80])
print("SKIN_GRAPH_DONE")