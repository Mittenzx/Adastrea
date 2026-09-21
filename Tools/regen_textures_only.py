"""Texture-only regeneration driver (no meshes, no FBX).

Extracts the literal gen_texture_set(...) calls from the original scripts via
ast (so variants/seeds are exactly the originals) and replays only those calls
against generate_adastrea_assets.gen_texture_set().  Never calls any mesh
builder or FBX exporter.

Run: blender -b --python Tools/regen_textures_only.py -- --src <key> --out <dir> [--names A,B] [--list]
  --src   main | fleet | station | shells | v2   (v2 = regen_ship_hull_textures_v2.py, run with WEATHERING=True)
  --out   output texture folder (default: the real Textures folder)
  --names comma list of set names (without T_); default = all in that source
"""
import ast, os, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

SOURCES = {
    "main":    (os.path.join(HERE, "generate_adastrea_assets.py"), False),
    "fleet":   (os.path.join(HERE, "build_fleet_hull_textures.py"), False),
    "station": (os.path.join(HERE, "build_station_hull_textures.py"), False),
    "shells":  (os.path.join(HERE, "build_station_module_shells.py"), False),
    "v2":      (os.path.join(os.path.dirname(HERE), ".claude", "worktrees", "laughing-blackwell-aba632", "Tools", "regen_ship_hull_textures_v2.py"), True),
}

def extract(path):
    tree = ast.parse(open(path, encoding="utf-8").read())
    calls = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            f = node.func
            nm = f.attr if isinstance(f, ast.Attribute) else getattr(f, "id", None)
            if nm == "gen_texture_set":
                args = [ast.literal_eval(a) for a in node.args]
                kw = {k.arg: ast.literal_eval(k.value) for k in node.keywords}
                calls.append((node.lineno, args, kw))
    calls.sort(key=lambda c: c[0])
    return calls

# The checked-in ship textures (Sep 18) were produced by an uncommitted
# "quality pass #2" variant of gen_texture_set (stronger relief contrast, per-plate
# tone variance, deeper AO, finer normal grain) that never reached HEAD.
# --knobs new re-applies those constants in memory (the repo file is untouched)
# so D/R/M/AO/E come out identical to the checked-in maps; the normal-map
# encoding fix from HEAD is kept.
KNOB_PATCHES = [
    ("np.clip(base[ch] - (0.5 - h) * 0.72, 0, 1)", "np.clip(base[ch] - (0.5 - h) * 0.92, 0, 1)"),
    ("tone = 0.10 * pad", "tone = 0.17 * pad"),
    ("AO = (h - 0.5) * 2.4 + 0.62", "AO = (h - 0.5) * 2.9 + 0.58"),
    ("AO = np.clip(AO, 0.05, 1.0)", "AO = np.clip(AO, 0.035, 1.0)"),
    ("N[..., 0] += -ndx * 0.12", "N[..., 0] += -ndx * 0.17"),
    ("N[..., 1] += -ndy * 0.12", "N[..., 1] += -ndy * 0.17"),
]

def load_generator(knobs):
    if knobs == "old":
        import generate_adastrea_assets as gen
        return gen
    import types
    gpath = os.path.join(HERE, "generate_adastrea_assets.py")
    src = open(gpath, encoding="utf-8").read()
    for old, new in KNOB_PATCHES:
        assert src.count(old) == 1, old
        src = src.replace(old, new)
    mod = types.ModuleType("generate_adastrea_assets")
    mod.__file__ = gpath
    sys.modules["generate_adastrea_assets"] = mod
    exec(compile(src, gpath, "exec"), mod.__dict__)
    return mod

def main():
    a = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    opt = {}
    i = 0
    while i < len(a):
        if a[i] == "--list":
            opt["list"] = True; i += 1
        else:
            opt[a[i][2:]] = a[i + 1]; i += 2
    path, weathering = SOURCES[opt["src"]]
    calls = extract(path)
    names = opt.get("names")
    names = set(names.split(",")) if names else None
    if opt.get("list"):
        for _, args, kw in calls:
            print("SET", args[0], kw.get("seed", args[3] if len(args) > 3 else 1))
        return
    gen = load_generator(opt.get("knobs", "old"))
    gen.setup_scene()
    gen.WEATHERING = (opt["weathering"] == "1") if "weathering" in opt else weathering
    if opt.get("out"):
        gen.TEXDIR = opt["out"]
    os.makedirs(gen.TEXDIR, exist_ok=True)
    t0 = time.time()
    for _, args, kw in calls:
        if names and args[0] not in names:
            continue
        t = time.time()
        gen.gen_texture_set(*args, **kw)
        print("REGEN", opt["src"], args[0], "%.1fs" % (time.time() - t), flush=True)
    print("TOTAL %.1fs" % (time.time() - t0))

main()
