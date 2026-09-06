import unreal
# probe MaterialEditingLibrary methods for node-to-node linking (read-only print)
names = [n for n in dir(unreal.MaterialEditingLibrary) if 'connect' in n.lower() or 'linked' in n.lower() or 'expression' in n.lower()]
print("[probe-skin]", names)
print("[probe-skin] DONE")