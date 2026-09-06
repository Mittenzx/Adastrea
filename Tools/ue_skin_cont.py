"""Continue building M_Fighter_Hull_Skin via MCP: add Blend, wire D/SKIN/Param -> Blend
-> BaseColor, add N/R/M/E samples -> outputs, recompile, save. Assumes the 3 nodes
(D sample, SKIN sample, Param) already exist from the prior run."""
import json, urllib.request

EP="http://127.0.0.1:8000/mcp"
MAT="/Game/Materials/M_Fighter_Hull_Skin.M_Fighter_Hull_Skin"

def init():
    p={"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"cli","version":"1"}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json"},method="POST")
    with urllib.request.urlopen(req,timeout=10) as r: return r.headers.get("Mcp-Session-Id")

def call(sid, tool, args):
    ts,tn=tool.rsplit(".",1)
    p={"jsonrpc":"2.0","id":9,"method":"tools/call","params":{"name":"call_tool","arguments":{"toolset_name":ts,"tool_name":tn,"arguments":args}}}
    req=urllib.request.Request(EP,data=json.dumps(p).encode(),headers={"Content-Type":"application/json","Mcp-Session-Id":sid},method="POST")
    with urllib.request.urlopen(req,timeout=90) as r: return json.loads(r.read().decode())

def rv(resp):
    c=resp.get("result",{}).get("content",[{}]); t=c[0].get("text","") if c else ""
    try: j=json.loads(t); return j.get("returnValue", j)
    except Exception: return t or str(resp.get("error",""))[:200]

MT="editor_toolset.toolsets.material.MaterialTools"
OT="editor_toolset.toolsets.object.ObjectTools"
sid=init(); print("sid",sid[:8])

# nodes that exist (from prior): sample_0, sample_1, parameter_0. Get them.
nodes_r = call(sid, MT+".get_expressions", {"material_or_function":{"refPath":MAT}})
nodes = rv(nodes_r)
print("existing nodes:", nodes)
# name the D sample, SKIN sample, Param, then add Blend
dname=skname=pname=blend=None
if isinstance(nodes,list):
    for i,n in enumerate(nodes):
        ref=n.get("refPath","") if isinstance(n,dict) else str(n)
        if "TextureSample_0" in ref: dname=ref
        elif "TextureSample_1" in ref: skname=ref
        elif "Parameter_0" in ref: pname=ref
print("dname=",dname,"skname=",skname,"pname=",pname)

# add Blend node
r=call(sid, MT+".add_expression", {"material_or_function":{"refPath":MAT},
      "expression_class":{"refPath":"/Script/Engine.MaterialExpressionBlend"}})
blend=rv(r); blend=blend.get("refPath") if isinstance(blend,dict) else str(blend)
print("blend:", blend)

# connect: D -> Blend input0, SkinColor(Param) -> Blend input1, SKIN -> Blend Fac
# get blend input names first
r=call(sid, MT+".get_expression_input_names", {"expression":{"refPath":blend}})
ins=rv(r); print("blend inputs:", ins)
# get output names of D, SKIN, Param
r=call(sid, MT+".get_expression_output_names", {"expression":{"refPath":dname}})
print("D outputs:", rv(r))
r=call(sid, MT+".get_expression_output_names", {"expression":{"refPath":skname}})
print("SKIN outputs:", rv(r))
r=call(sid, MT+".get_expression_output_names", {"expression":{"refPath":pname}})
print("Param outputs:", rv(r))
print("READY")