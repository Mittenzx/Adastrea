import unreal
# Link the lone CommsTower mesh to a sensible existing material (comms/sensor theme).
mesh = unreal.load_asset("/Game/Assets/All/SM_Obj_CommsTower_01")
mat  = unreal.load_asset("/Game/Materials/M_Sensor_Block")
if mesh is not None and mat is not None:
    mesh.set_material(0, mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Assets/All/SM_Obj_CommsTower_01")
    unreal.log("[comms-fix] linked CommsTower -> M_Sensor_Block DONE")
else:
    unreal.log("[comms-fix] FAIL mesh=%s mat=%s" % (mesh, mat))