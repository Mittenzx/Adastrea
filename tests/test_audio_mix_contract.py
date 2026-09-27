#!/usr/bin/env python3
"""
Audio mix contract tests (SOUND_PLAN.md section 3).

Like the other contract tests, these read the real C++/config/tool sources and assert
the wiring that is easy to break without a build noticing:

- the mix assets the C++ loads are the ones Tools/create_audio_mixes.py creates
- SM_Interior low-passes and lowers engine + world; SM_MenuDuck ducks world, engine, ambience
- the ambience beds use the four catalog event IDs (never a sound asset path)
- the volume sliders are Master/SFX/UI only (no music slider this session), are persisted
  in GameUserSettings, and are applied as class overrides on SC_Master/SC_SFX/SC_UI
- the pause menu's Esc binding comes before the other Esc handlers
- /Game/Audio is always cooked (sounds are loaded by soft path)

Run:  pytest tests/test_audio_mix_contract.py
"""

import importlib.util
import re
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
SRC = PROJECT_ROOT / "Source" / "Adastrea"
MIX_H = SRC / "Public" / "Audio" / "AudioMixSubsystem.h"
MIX_CPP = SRC / "Private" / "Audio" / "AudioMixSubsystem.cpp"
SETTINGS_H = SRC / "Public" / "Audio" / "AdastreaAudioSettings.h"
PC_CPP = SRC / "Private" / "Player" / "AdastreaPlayerController.cpp"
HUD_CPP = SRC / "Private" / "AdastreaHUD.cpp"
TOOL = PROJECT_ROOT / "Tools" / "create_audio_mixes.py"
DEFAULT_GAME = PROJECT_ROOT / "Config" / "DefaultGame.ini"


def _src(path: Path) -> str:
    assert path.exists(), f"Missing file: {path}"
    return path.read_text(encoding="utf-8", errors="replace")


def _strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def _tool():
    spec = importlib.util.spec_from_file_location("create_audio_mixes", TOOL)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)  # no `unreal` import at module level
    return module


def test_cpp_mix_paths_match_the_tool():
    tool = _tool()
    cpp = _src(MIX_CPP)
    paths = re.findall(r'TEXT\("(/Game/Audio/Mixes/(\w+)\.(\w+))"\)', cpp)
    assert paths, "AudioMixSubsystem.cpp loads no mixes"
    for full, pkg, obj in paths:
        assert pkg == obj, f"object name mismatch in {full}"
        assert full.startswith(tool.MIX_FOLDER + "/")
        assert pkg in tool.MIXES, f"{pkg} is loaded by C++ but not created by the tool"
    assert {p[1] for p in paths} == set(tool.MIXES), "tool creates mixes the C++ never uses"


def test_cpp_class_paths_exist_in_the_tool():
    tool = _tool()
    cpp = _src(MIX_CPP)
    classes = re.findall(r'TEXT\("/Game/Audio/Classes/(\w+)\.(\w+)"\)', cpp)
    assert {c[0] for c in classes} == {"SC_Master", "SC_SFX", "SC_UI"}
    for pkg, obj in classes:
        assert pkg == obj
        assert pkg in tool.CLASS_VOLUMES


def test_interior_mix_muffles_engine_and_world():
    tool = _tool()
    adj = {c: (v, lpf) for c, v, lpf in tool.MIXES["SM_Interior"]["adjusters"]}
    for cls in ("SC_Engine", "SC_World"):
        assert cls in adj, f"SM_Interior must adjust {cls}"
        vol, lpf = adj[cls]
        assert 0.0 < vol < 1.0, f"{cls} should be lowered, not muted or boosted"
        assert lpf < 5000.0, f"{cls} should be low-passed"
    assert tool.MIXES["SM_Interior"]["fade_in"] > 0 and tool.MIXES["SM_Interior"]["fade_out"] > 0


def test_menu_duck_ducks_world_engine_ambient_only():
    tool = _tool()
    adj = {c: v for c, v, _ in tool.MIXES["SM_MenuDuck"]["adjusters"]}
    assert set(adj) == {"SC_World", "SC_Engine", "SC_Ambient"}
    assert all(0.0 < v < 1.0 for v in adj.values())
    assert "SC_UI" not in adj, "menus must stay audible"


def test_settings_mix_starts_empty():
    tool = _tool()
    assert tool.MIXES["SM_Settings"]["adjusters"] == []
    assert tool.MIXES["SM_Settings"]["fade_in"] == 0.0


def test_ambience_beds_use_catalog_event_ids():
    cpp = _strip_comments(_src(MIX_CPP))
    for event in ("Ambient.Space", "Ambient.StationHum", "Ambient.AsteroidCreak", "Ambient.MapRoomTone"):
        assert f'TEXT("{event}")' in cpp, f"{event} not wired"
    # Sounds only ever come through the catalog.
    assert "SpawnEventAttached" in cpp and "PlayEventAtLocation" in cpp
    assert not re.search(r"/Game/Audio/(?!Mixes|Classes)", cpp), "no direct sound asset paths"
    assert "LoadObject<USoundWave>" not in cpp and "LoadObject<USoundBase>" not in cpp


def test_beds_fade_never_hard_cut():
    cpp = _strip_comments(_src(MIX_CPP))
    for fn in ("FadeIn(", "FadeOut(", "AdjustVolume("):
        assert fn in cpp
    # Stop() is only allowed when the world is torn down (Deinitialize).
    start = cpp.index("void UAudioMixSubsystem::Deinitialize()")
    open_brace = cpp.index("{", start)
    depth, i = 0, open_brace
    while True:
        depth += {"{": 1, "}": -1}.get(cpp[i], 0)
        if depth == 0:
            break
        i += 1
    rest = cpp[:start] + cpp[i + 1:]
    assert "->Stop()" not in rest, "beds must fade out, not Stop(), during play"


def test_volume_sliders_are_master_sfx_ui_only():
    h = _strip_comments(_src(SETTINGS_H))
    enum = re.search(r"enum class EAdastreaVolumeCategory[^{]*\{(.*?)\}", h, re.S).group(1)
    names = [n.strip() for n in enum.split(",") if n.strip()]
    assert names == ["Master", "SFX", "UI"], names
    assert "Music" not in h


def test_volume_settings_persist_in_game_user_settings():
    h = _src(SETTINGS_H)
    assert re.search(r"UCLASS\(\s*config\s*=\s*GameUserSettings", h)
    for field in ("MasterVolume", "SFXVolume", "UIVolume"):
        assert re.search(rf"UPROPERTY\(config[^\n]*\)\s*float {field}\b", h), f"{field} not a config property"
    cpp = _src(MIX_CPP)
    assert "SetSoundMixClassOverride" in cpp
    assert "Save()" in cpp and "ApplyVolumeSettings(0.0f)" in cpp, "load on startup + save on change"


def test_pause_menu_esc_binding_precedes_other_esc_handlers():
    cpp = _src(PC_CPP)
    esc = [m.start() for m in re.finditer(r"BindKey\(EKeys::Escape", cpp)]
    pause = cpp.find("BindKey(EKeys::Escape, IE_Pressed, this, &AAdastreaPlayerController::HandlePauseMenuKey)")
    assert pause != -1, "pause menu not bound to Escape"
    assert pause == min(esc), "HandlePauseMenuKey must be the first Escape binding"


def test_pause_menu_draws_sliders():
    hud = _src(HUD_CPP)
    assert "void AAdastreaHUD::DrawPauseMenu" in hud
    assert "SetCategoryVolume" in hud


def test_game_audio_is_always_cooked():
    ini = _src(DEFAULT_GAME)
    assert re.search(r'^\+DirectoriesToAlwaysCook=\(Path="/Game/Audio"\)', ini, re.M), \
        "/Game/Audio must be in DirectoriesToAlwaysCook (sounds, classes and mixes load by soft path)"
