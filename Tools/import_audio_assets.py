"""
Imports the generated sound library into Unreal and builds the event catalog.

Run headless (from the project root that contains Adastrea.uproject):
    UnrealEditor-Cmd.exe <root>\\Adastrea.uproject -run=pythonscript
        -script="<root>\\Tools\\import_audio_assets.py" -unattended -nosplash -nop4 -nullrhi

Verify only (no changes; checks every catalog event resolves to a loadable sound):
    ... -script="<root>\\Tools\\import_audio_assets.py --verify"

What it does (idempotent: re-running after regenerating the WAVs updates in place):
1. Sound classes /Game/Audio/Classes/SC_{Master,SFX,Engine,World,Interior,UI,Ambient,Music}
   with the SOUND_PLAN.md section 3 hierarchy:
       Master > SFX > {Engine, World, Interior};  Master > UI;  Master > Ambient;  Master > Music (empty)
2. Attenuation /Game/Audio/Attenuation/ATT_World, ATT_AIEngine (full volume within 10 m,
   silent by 50 m), ATT_StationHum.
3. Every WAV in Assets/Audio/generated/audio_manifest.json -> /Game/Audio/<Category>/SW_<Id>,
   with looping and sound class set.
4. /Game/Audio/DA_AudioCatalog (UAudioCatalogDataAsset) filled from the manifest, including
   groups (e.g. Interior.Footstep -> .01 plus Variations .02-.04).
"""

import json
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GEN_DIR = os.path.join(ROOT, "Assets", "Audio", "generated")
MANIFEST = os.path.join(GEN_DIR, "audio_manifest.json")

AUDIO_ROOT = "/Game/Audio"
CLASSES_DIR = AUDIO_ROOT + "/Classes"
ATT_DIR = AUDIO_ROOT + "/Attenuation"
CATALOG_PATH = AUDIO_ROOT + "/DA_AudioCatalog"

# child -> parent
SOUND_CLASS_TREE = {
    "SC_Master": None,
    "SC_SFX": "SC_Master",
    "SC_Engine": "SC_SFX",
    "SC_World": "SC_SFX",
    "SC_Interior": "SC_SFX",
    "SC_UI": "SC_Master",
    "SC_Ambient": "SC_Master",
    "SC_Music": "SC_Master",
}

# Attenuation presets (distances in cm). inner = full volume radius, falloff = distance
# over which it fades to silence beyond the inner radius.
ATTENUATION = {
    "ATT_World": dict(inner=500.0, falloff=8000.0, model="NATURAL_SOUND"),
    # AI engines: audible only at very close range. Full within 10 m, silent by 50 m.
    # EngineVoice measures from the hull and may tune these.
    "ATT_AIEngine": dict(inner=1000.0, falloff=4000.0, model="LINEAR"),
    "ATT_StationHum": dict(inner=3000.0, falloff=25000.0, model="NATURAL_SOUND"),
}

LOG_PREFIX = "[import_audio_assets]"


def log(msg):
    unreal.log(f"{LOG_PREFIX} {msg}")


def warn(msg):
    unreal.log_warning(f"{LOG_PREFIX} {msg}")


asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def get_or_create(name, folder, asset_class, factory):
    path = f"{folder}/{name}"
    if eal.does_asset_exist(path):
        asset = eal.load_asset(path)
        if asset is not None and isinstance(asset, asset_class):
            return asset
        raise RuntimeError(f"{path} exists but is not a {asset_class.__name__}")
    asset = asset_tools.create_asset(name, folder, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"failed to create {path}")
    log(f"created {path}")
    return asset


def build_sound_classes():
    classes = {n: get_or_create(n, CLASSES_DIR, unreal.SoundClass, unreal.SoundClassFactory())
               for n in SOUND_CLASS_TREE}
    for name, cls in classes.items():
        # USoundClass::PostEditChangeProperty sets ParentClass only on the *first* newly added
        # child per change, so grow the list one child at a time.
        children = []
        for c, p in SOUND_CLASS_TREE.items():
            if p == name:
                children.append(classes[c])
                cls.set_editor_property("child_classes", list(children))
        if not children:
            cls.set_editor_property("child_classes", [])
    # Check the parent links (ParentClass is read-only from Python).
    for name, parent in SOUND_CLASS_TREE.items():
        actual = classes[name].get_editor_property("parent_class")
        actual_name = actual.get_name() if actual else None
        if actual_name != parent:
            warn(f"{name}: parent is {actual_name}, expected {parent}")
    for cls in classes.values():
        eal.save_loaded_asset(cls, only_if_is_dirty=False)
    return classes


def build_attenuations():
    out = {}
    for name, cfg in ATTENUATION.items():
        att = get_or_create(name, ATT_DIR, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
        s = att.get_editor_property("attenuation")
        s.set_editor_property("attenuate", True)
        s.set_editor_property("spatialize", True)
        s.set_editor_property("attenuation_shape", unreal.AttenuationShape.SPHERE)
        s.set_editor_property("attenuation_shape_extents", unreal.Vector(cfg["inner"], 0.0, 0.0))
        s.set_editor_property("falloff_distance", cfg["falloff"])
        s.set_editor_property("distance_algorithm", getattr(unreal.AttenuationDistanceModel, cfg["model"]))
        att.set_editor_property("attenuation", s)
        eal.save_loaded_asset(att, only_if_is_dirty=False)
        out[name] = att
    return out


def import_waves(manifest, classes):
    tasks = []
    for s in manifest["sounds"]:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", os.path.join(GEN_DIR, s["file"]).replace("\\", "/"))
        task.set_editor_property("destination_path", f"{AUDIO_ROOT}/{s['category']}")
        task.set_editor_property("destination_name", s["asset_name"])
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("replace_existing_settings", True)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", False)
        tasks.append(task)
    asset_tools.import_asset_tasks(tasks)

    waves = {}
    for s in manifest["sounds"]:
        path = f"{AUDIO_ROOT}/{s['category']}/{s['asset_name']}"
        wave = eal.load_asset(path)
        if not isinstance(wave, unreal.SoundWave):
            raise RuntimeError(f"import failed for {s['id']} -> {path}")
        wave.set_editor_property("looping", bool(s["loop"]))
        wave.set_editor_property("sound_class_object", classes[s["sound_class"]])
        eal.save_loaded_asset(wave, only_if_is_dirty=False)
        waves[s["id"]] = wave
    log(f"imported {len(waves)} sound waves")
    return waves


def make_entry(sound, spec, classes, atts, variations=()):
    e = unreal.AudioCatalogEntry()
    e.set_editor_property("sound", sound)
    e.set_editor_property("volume", float(spec.get("volume", 1.0)))
    pr = spec.get("pitch_range", [1.0, 1.0])
    e.set_editor_property("pitch_range", unreal.Vector2D(float(pr[0]), float(pr[1])))
    e.set_editor_property("sound_class", classes[spec["sound_class"]])
    if spec.get("attenuation"):
        e.set_editor_property("attenuation", atts[spec["attenuation"]])
    e.set_editor_property("variations", list(variations))
    e.set_editor_property("looping", bool(spec.get("loop", False)))
    return e


def build_catalog(manifest, waves, classes, atts):
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.AudioCatalogDataAsset)
    catalog = get_or_create("DA_AudioCatalog", AUDIO_ROOT, unreal.AudioCatalogDataAsset, factory)

    entries = {}
    for s in manifest["sounds"]:
        entries[s["id"]] = make_entry(waves[s["id"]], s, classes, atts)
    for g in manifest.get("groups", []):
        members = [waves[m] for m in g["members"]]
        entries[g["id"]] = make_entry(members[0], g, classes, atts, members[1:])
    catalog.set_editor_property("entries", entries)
    eal.save_loaded_asset(catalog, only_if_is_dirty=False)
    log(f"catalog {CATALOG_PATH}: {len(entries)} events")
    return catalog


def verify(manifest):
    """Loads the catalog from disk and checks every manifest event resolves to a loadable sound."""
    catalog = unreal.load_asset(CATALOG_PATH)
    if catalog is None:
        unreal.log_error(f"{LOG_PREFIX} VERIFY FAILED: {CATALOG_PATH} missing")
        return False
    # Soft references read back through Python resolve only if the target is loaded, so load
    # everything under /Game/Audio first. A catalog pointing at a wrong path still reads as None.
    for p in eal.list_assets(AUDIO_ROOT, recursive=True, include_folder=False):
        eal.load_asset(p)
    entries = catalog.get_editor_property("entries")
    expected = [s["id"] for s in manifest["sounds"]] + [g["id"] for g in manifest.get("groups", [])]
    bad = []
    for eid in expected:
        e = entries.get(eid)
        if e is None:
            bad.append(f"{eid}: no entry")
            continue
        refs = [e.get_editor_property("sound")] + list(e.get_editor_property("variations"))
        for r in refs:
            obj = r
            if r is not None and not isinstance(r, unreal.Object):
                obj = unreal.SystemLibrary.load_asset_blocking(r)
            if not isinstance(obj, unreal.SoundWave):
                bad.append(f"{eid}: sound {r} does not load")
        if e.get_editor_property("sound_class") is None:
            bad.append(f"{eid}: no sound class")
    extra = [str(k) for k in entries.keys() if str(k) not in expected]
    if bad:
        for b in bad:
            unreal.log_error(f"{LOG_PREFIX} VERIFY: {b}")
        unreal.log_error(f"{LOG_PREFIX} VERIFY FAILED: {len(bad)} problems")
        return False
    log(f"VERIFY OK: {len(expected)} events resolve ({len(entries)} catalog entries, extra={extra})")
    return True


def main(argv):
    with open(MANIFEST, "r", encoding="utf-8") as f:
        manifest = json.load(f)
    if "--verify" not in argv:
        classes = build_sound_classes()
        atts = build_attenuations()
        waves = import_waves(manifest, classes)
        build_catalog(manifest, waves, classes, atts)
    ok = verify(manifest)
    log("DONE" if ok else "DONE WITH ERRORS")


main(sys.argv)
