"""Create/refresh the Adastrea sound mixes and set the sound-class volumes.

Idempotent: re-running updates the existing assets in place. Run headless with the
editor closed (or in the live editor through remote execution):

    UnrealEditor-Cmd.exe <Project>/Adastrea.uproject -run=pythonscript
        -script="<Project>/Tools/create_audio_mixes.py" -unattended -nosplash -nop4 -stdout

What it owns (SOUND_PLAN.md section 3, the "Mix" session):
  /Game/Audio/Mixes/SM_Interior   pushed by UAudioMixSubsystem while the player is on foot:
                                  engine + world low-passed and lowered ("through the hull").
  /Game/Audio/Mixes/SM_MenuDuck   pushed while the pause menu, trading or Station Editor is open:
                                  world, engine and ambience ducked.
  /Game/Audio/Mixes/SM_Settings   empty; the pause-menu sliders are applied to it at runtime as
                                  class overrides on SC_Master / SC_SFX / SC_UI.
  Class volumes on /Game/Audio/Classes/SC_*  (the classes themselves are created by SoundSmith's
                                  importer; this script only sets their Volume).

The C++ side (Source/Adastrea/Private/Audio/AudioMixSubsystem.cpp) loads the mixes by the same
paths; tests/test_audio_mix_contract.py keeps the two in sync.
"""

MIX_FOLDER = "/Game/Audio/Mixes"
CLASS_FOLDER = "/Game/Audio/Classes"

# Low-pass "off" value used by UE's FSoundClassAdjuster (MAX_FILTER_FREQUENCY).
LPF_OFF = 20000.0

# name -> fade in/out (s) and per-class adjusters: (class, volume, low-pass Hz)
MIXES = {
    "SM_Interior": {
        "fade_in": 0.6,
        "fade_out": 0.9,
        "adjusters": [
            ("SC_Engine", 0.45, 700.0),
            ("SC_World", 0.55, 1200.0),
        ],
    },
    "SM_MenuDuck": {
        "fade_in": 0.35,
        "fade_out": 0.6,
        "adjusters": [
            ("SC_World", 0.35, LPF_OFF),
            ("SC_Engine", 0.35, LPF_OFF),
            ("SC_Ambient", 0.5, LPF_OFF),
        ],
    },
    "SM_Settings": {
        "fade_in": 0.0,
        "fade_out": 0.0,
        "adjusters": [],
    },
}

# Sound-class volumes (linear). The generated WAVs are already normalised to their loudness
# targets (UI -18, engine -24, ambience -30 LUFS; see Assets/Audio/generated/audio_manifest.json
# and tests/test_audio_assets.py), so the classes pass them through at unity and keep that
# 6 dB / 12 dB spacing. Adjust here, not in the WAVs, for mix-level tuning after listening.
CLASS_VOLUMES = {
    "SC_Master": 1.0,
    "SC_SFX": 1.0,
    "SC_Engine": 1.0,
    "SC_World": 1.0,
    "SC_Interior": 1.0,
    "SC_UI": 1.0,
    "SC_Ambient": 1.0,
    "SC_Music": 1.0,  # empty until the music session
}


def _log(msg):
    import unreal
    unreal.log(f"[create_audio_mixes] {msg}")


def _load_class(name):
    import unreal
    path = f"{CLASS_FOLDER}/{name}.{name}"
    obj = unreal.load_asset(path)
    if obj is None:
        unreal.log_warning(f"[create_audio_mixes] missing sound class {path} (run SoundSmith's importer first)")
    return obj


def _get_or_create_mix(name):
    import unreal
    path = f"{MIX_FOLDER}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = None
    if hasattr(unreal, "SoundMixFactory"):
        factory = unreal.SoundMixFactory()
    mix = tools.create_asset(name, MIX_FOLDER, unreal.SoundMix, factory)
    _log(f"created {path}")
    return mix


def build_mixes():
    import unreal
    for name, spec in MIXES.items():
        mix = _get_or_create_mix(name)
        if mix is None:
            unreal.log_error(f"[create_audio_mixes] could not create {name}")
            continue
        adjusters = []
        for class_name, volume, lpf in spec["adjusters"]:
            sound_class = _load_class(class_name)
            if sound_class is None:
                continue
            adj = unreal.SoundClassAdjuster()
            adj.set_editor_property("sound_class_object", sound_class)
            adj.set_editor_property("volume_adjuster", float(volume))
            adj.set_editor_property("pitch_adjuster", 1.0)
            adj.set_editor_property("low_pass_filter_frequency", float(lpf))
            adj.set_editor_property("apply_to_children", True)
            adjusters.append(adj)
        mix.set_editor_property("sound_class_effects", adjusters)
        mix.set_editor_property("fade_in_time", float(spec["fade_in"]))
        mix.set_editor_property("fade_out_time", float(spec["fade_out"]))
        mix.set_editor_property("duration", -1.0)  # until popped
        unreal.EditorAssetLibrary.save_loaded_asset(mix, only_if_is_dirty=False)
        _log(f"{name}: {len(adjusters)} adjusters, fade {spec['fade_in']}/{spec['fade_out']}s")


def set_class_volumes():
    import unreal
    for name, volume in CLASS_VOLUMES.items():
        sound_class = _load_class(name)
        if sound_class is None:
            continue
        props = sound_class.get_editor_property("properties")
        props.set_editor_property("volume", float(volume))
        sound_class.set_editor_property("properties", props)
        unreal.EditorAssetLibrary.save_loaded_asset(sound_class, only_if_is_dirty=False)
        _log(f"{name}: volume {volume}")


def report():
    """Read everything back (verifies the saved state instead of trusting the setters)."""
    import unreal
    for name in MIXES:
        mix = unreal.load_asset(f"{MIX_FOLDER}/{name}.{name}")
        if mix is None:
            unreal.log_error(f"[create_audio_mixes] VERIFY: {name} missing")
            continue
        parts = []
        for adj in mix.get_editor_property("sound_class_effects"):
            sc = adj.get_editor_property("sound_class_object")
            parts.append(f"{sc.get_name() if sc else None} vol={adj.get_editor_property('volume_adjuster'):.2f} "
                         f"lpf={adj.get_editor_property('low_pass_filter_frequency'):.0f}")
        _log(f"VERIFY {name}: fade {mix.get_editor_property('fade_in_time')}/{mix.get_editor_property('fade_out_time')} "
             f"[{'; '.join(parts)}]")
    for name in CLASS_VOLUMES:
        sc = unreal.load_asset(f"{CLASS_FOLDER}/{name}.{name}")
        if sc is None:
            continue
        try:
            parent = sc.get_editor_property("parent_class")
        except Exception:  # not exposed in every engine version
            parent = None
        _log(f"VERIFY {name}: volume={sc.get_editor_property('properties').get_editor_property('volume'):.2f} "
             f"parent={parent.get_name() if parent else None}")


def main():
    build_mixes()
    set_class_volumes()
    report()


if __name__ == "__main__":
    main()
