#!/usr/bin/env python3
"""
Event SFX contract tests: gameplay moments fire catalog event IDs.

Like test_economy_wallet_quicksave.py, these read the real C++ source and
assert the contracts that are easy to regress without noticing in a build:

- every event ID in SOUND_PLAN section 2 (flight, docking, mining, trading,
  Station Editor, interiors, UI) is fired from gameplay code
- gameplay code only plays sounds through UAudioEventLibrary (event IDs), never
  by naming a sound asset or calling UGameplayStatics sound functions
- repeating events are rate limited, and "secondary" events (toasts, credit
  dings, menu open/close/hover) go through PlaySecondary2D so they yield to
  the primary sound of the same moment
- the load path is muted (re-docking and wallet rewrites don't sound)
- player-only feedback is gated on the local player's pawn
- loops (laser, interior hum) are released when their owner goes away
- adastrea.AudioEventLog exists for PIE evidence

Run:  pytest tests/test_event_sfx_hooks.py
"""

import re
from pathlib import Path

import pytest

PROJECT_ROOT = Path(__file__).parent.parent
SRC = PROJECT_ROOT / "Source"
ADASTREA = SRC / "Adastrea"
AUDIO_DIRS = {ADASTREA / "Public" / "Audio", ADASTREA / "Private" / "Audio"}

LIBRARY_H = ADASTREA / "Public" / "Audio" / "AudioEventLibrary.h"
LIBRARY_CPP = ADASTREA / "Private" / "Audio" / "AudioEventLibrary.cpp"
SHIP_CPP = ADASTREA / "Private" / "Ships" / "Spaceship.cpp"
AVATAR_CPP = ADASTREA / "Private" / "Ships" / "SpaceshipAvatar.cpp"
LASER_CPP = ADASTREA / "Private" / "Mining" / "MiningLaserComponent.cpp"
PC_CPP = ADASTREA / "Private" / "Player" / "AdastreaPlayerController.cpp"
HUD_CPP = ADASTREA / "Private" / "AdastreaHUD.cpp"
SAVE_CPP = ADASTREA / "Private" / "Player" / "SaveGameSubsystem.cpp"
TERMINAL_CPP = ADASTREA / "Private" / "Stations" / "StationInterior.cpp"
MANAGER_CPP = SRC / "StationEditor" / "Private" / "StationEditorManager.cpp"
WIDGET_CPP = SRC / "StationEditor" / "Public" / "UI" / "StationEditorWidgetCpp.cpp"
LIST_ITEM_CPP = SRC / "StationEditor" / "Public" / "UI" / "ModuleListItemWidget.cpp"

# SOUND_PLAN.md section 2, as event IDs (footsteps are built as Interior.Footstep.%02d).
REQUIRED_EVENTS = [
    "Thruster.Puff", "Thruster.HeavyGroan", "Flight.CollisionBump", "Flight.SpeedWarning",
    "Dock.Beacon", "Dock.ClampEngage", "Dock.AirlockHiss", "Dock.Release",
    "Mining.LaserLoop", "Mining.OreTick", "Mining.CargoFull", "Mining.AsteroidDepleted",
    "Trade.Buy", "Trade.Sell", "Trade.CreditsDing", "Trade.Denied",
    "Editor.Place.Small", "Editor.Place.Large", "Editor.Remove", "Editor.Invalid",
    "Editor.Undo", "Editor.Redo", "Editor.Rotate", "Editor.Save",
    "Interior.Door", "Interior.CockpitEnter", "Interior.CockpitExit",
    "Interior.ShipHum", "Interior.ConsoleChirp",
    "UI.Hover", "UI.Click", "UI.Open", "UI.Close", "UI.Toast",
    "UI.QuickSave", "UI.QuickLoad", "UI.Error",
]

SECONDARY_EVENTS = ["UI.Toast", "Trade.CreditsDing", "UI.Open", "UI.Close", "UI.Hover"]


def _src(path: Path) -> str:
    if not path.exists():
        raise FileNotFoundError(f"Missing source file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def _strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def _function_body(text: str, signature: str) -> str:
    """Body of the first function whose definition line contains `signature`."""
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[brace:i + 1]
    raise AssertionError(f"unbalanced braces after {signature}")


def _gameplay_sources():
    """All .cpp/.h under Source except the audio layer itself."""
    for path in SRC.rglob("*"):
        if path.suffix not in (".cpp", ".h"):
            continue
        if any(audio_dir in path.parents for audio_dir in AUDIO_DIRS):
            continue
        yield path


def _all_gameplay_code() -> str:
    return "\n".join(_strip_comments(_src(p)) for p in _gameplay_sources())


class TestEveryPlannedEventIsFired:
    @pytest.mark.parametrize("event_id", REQUIRED_EVENTS)
    def test_event_id_is_fired(self, event_id):
        code = _all_gameplay_code()
        assert f'TEXT("{event_id}")' in code, f"{event_id} is never fired from gameplay code"

    def test_footsteps_cover_four_variations(self):
        body = _function_body(_strip_comments(_src(AVATAR_CPP)), "ASpaceshipAvatar::UpdateFootsteps")
        assert 'TEXT("Interior.Footstep.%02d")' in body
        assert "RandRange(1, LastFootstepIndex > 0 ? 3 : 4)" in body, "footsteps should pick from 01..04"
        assert "LastFootstepIndex = Index" in body, "footsteps should avoid immediate repeats"


class TestNoDirectSoundAssets:
    FORBIDDEN = [
        r"UGameplayStatics::(PlaySound2D|PlaySoundAtLocation|SpawnSound2D|SpawnSoundAtLocation|SpawnSoundAttached|CreateSound2D)",
        r"/Game/Audio/",
        r"\bSW_[A-Za-z0-9_]+",
    ]

    def test_gameplay_code_never_names_sounds(self):
        offenders = []
        for path in _gameplay_sources():
            code = _strip_comments(_src(path))
            for pattern in self.FORBIDDEN:
                if re.search(pattern, code):
                    offenders.append(f"{path.relative_to(PROJECT_ROOT)}: {pattern}")
        assert not offenders, "sounds must go through catalog event IDs:\n" + "\n".join(offenders)

    def test_library_is_the_only_caller_of_the_catalog(self):
        code = _all_gameplay_code()
        assert "UAudioCatalogSubsystem" not in code, \
            "gameplay should use UAudioEventLibrary (rate limits, mute, event log), not the catalog directly"


class TestSecondaryEventsYield:
    @pytest.mark.parametrize("event_id", SECONDARY_EVENTS)
    def test_secondary_events_only_via_play_secondary(self, event_id):
        code = _all_gameplay_code()
        calls = re.findall(r"UAudioEventLibrary::(\w+)\(\s*this\s*,\s*TEXT\(\"" + re.escape(event_id) + r"\"\)", code)
        assert calls, f"{event_id} not found"
        assert set(calls) == {"PlaySecondary2D"}, f"{event_id} must be secondary, got {calls}"

    def test_secondary_is_deferred_and_yields_to_primaries(self):
        body = _function_body(_strip_comments(_src(LIBRARY_CPP)), "UAudioEventLibrary::PlaySecondary2D")
        assert "FTSTicker" in body, "secondary events are deferred one frame (core ticker keeps running while paused)"
        assert "LastPrimaryTime" in body and "SecondaryYieldSeconds" in body


class TestRateLimits:
    def test_thrusters_rate_limited_by_ship_size(self):
        ship = _strip_comments(_src(SHIP_CPP))
        body = _function_body(ship, "void ASpaceship::UpdateThrusterAudio")
        assert 'TEXT("Thruster.Puff"), PuffMinInterval' in body
        assert 'TEXT("Thruster.HeavyGroan"), GroanMinInterval' in body
        assert "GetShipSizeFactor(ShipDataAsset)" in body, "puff vs groan comes from the ship data asset"
        puff = float(re.search(r"PuffMinInterval = ([0-9.]+)f", ship).group(1))
        groan = float(re.search(r"GroanMinInterval = ([0-9.]+)f", ship).group(1))
        assert 0.2 <= puff < groan, "heavy ships groan less often than light ships puff"
        assert "MinQuietSeconds" in body, "a burst needs a quiet gap first (no per-frame retrigger)"

    def test_size_factor_uses_hull_and_cargo(self):
        body = _function_body(_strip_comments(_src(LIBRARY_CPP)), "UAudioEventLibrary::GetShipSizeFactor")
        assert "HullStrength" in body and "CargoCapacity" in body

    def test_size_split_matches_real_roster(self):
        """PIE found the DA_* assets far larger than ShipClasses.json; the Corvette must still puff."""
        import math
        code = _src(LIBRARY_CPP)
        lo = float(re.search(r"SizeLogMin = ([0-9.]+)f", code).group(1))
        hi = float(re.search(r"SizeLogMax = ([0-9.]+)f", code).group(1))
        heavy = float(re.search(r"HeavyShipSizeFactor = ([0-9.]+)f", _src(LIBRARY_H)).group(1))

        def factor(hull_plus_cargo):
            return min(max((math.log10(hull_plus_cargo) - lo) / (hi - lo), 0.0), 1.0)

        # hull + cargo read from the DA_* assets in PIE (2026-09-27)
        light = {"Fighter_Viper": 630, "MittenzxMk1": 1075, "Patrol_Sentinel": 2000,
                 "Corvette_Raptor": 2350, "Frigate_Shadowblade": 3000}
        heavy_ships = {"Gunship_Warhammer": 5150, "Carrier_Vanguard": 6900, "Command_Sovereign": 17000,
                       "Transport_Behemoth": 28000}
        for name, size in light.items():
            assert factor(size) < heavy, f"{name} should puff"
        for name, size in heavy_ships.items():
            assert factor(size) >= heavy, f"{name} should groan"

    def test_strafe_detection_does_not_use_unbound_action_value(self):
        """MoveAction is bound with BindAction, so GetBoundActionValue always reads zero (found in PIE)."""
        ship = _strip_comments(_src(SHIP_CPP))
        update = _function_body(ship, "void ASpaceship::UpdateThrusterAudio")
        assert "GetBoundActionValue" not in update
        assert "ThrusterAudioLastStrafeTime" in update
        assert "ThrusterAudioLastStrafeTime = GetWorld()->GetTimeSeconds()" in _function_body(ship, "void ASpaceship::Move(")

    def test_beacon_not_while_building(self):
        body = _function_body(_strip_comments(_src(PC_CPP)), "void AAdastreaPlayerController::CheckForNearbyTradableStations")
        assert 'TEXT("Dock.Beacon")' in body and "!IsStationEditorOpen()" in body

    def test_ore_tick_and_credits_ding_rate_limited(self):
        pc = _strip_comments(_src(PC_CPP))
        assert re.search(r'TEXT\("Mining\.OreTick"\),\s*PlayerEventAudio::OreTickMinInterval', pc)
        assert re.search(r'TEXT\("Trade\.CreditsDing"\),\s*PlayerEventAudio::CreditsDingMinInterval', pc)

    def test_gate_checks_min_interval(self):
        body = _function_body(_strip_comments(_src(LIBRARY_CPP)), "UAudioEventLibrary::PassesGate")
        assert "MinInterval" in body and "IsMuted()" in body


class TestHookPlacement:
    def test_docking_sounds_are_player_only(self):
        ship = _strip_comments(_src(SHIP_CPP))
        complete = _function_body(ship, "void ASpaceship::CompleteDocking")
        undock = _function_body(ship, "void ASpaceship::Undock")
        assert "IsLocalPlayerActor(this)" in complete and 'TEXT("Dock.ClampEngage")' in complete
        assert 'TEXT("Dock.AirlockHiss")' in complete and "AirlockHissTimerHandle" in complete
        assert "IsLocalPlayerActor(this)" in undock and 'TEXT("Dock.Release")' in undock
        assert "ClearTimer(AirlockHissTimerHandle)" in undock

    def test_trade_result_sounds(self):
        body = _function_body(_strip_comments(_src(PC_CPP)), "void AAdastreaPlayerController::ExecuteTrade")
        for event_id in ("Trade.Denied", "Trade.Buy", "Trade.Sell"):
            assert f'TEXT("{event_id}")' in body

    def test_quicksave_quickload_results(self):
        pc = _strip_comments(_src(PC_CPP))
        save = _function_body(pc, "void AAdastreaPlayerController::HandleQuickSave")
        load = _function_body(pc, "void AAdastreaPlayerController::HandleQuickLoad")
        assert 'TEXT("UI.QuickSave")' in save and 'TEXT("UI.Error")' in save
        assert 'TEXT("UI.QuickLoad")' in load and load.count('TEXT("UI.Error")') >= 3

    def test_toast_on_hud_show_message(self):
        body = _function_body(_strip_comments(_src(HUD_CPP)), "void AAdastreaHUD::ShowMessage")
        assert 'TEXT("UI.Toast")' in body

    def test_hud_menu_audio_runs_every_frame(self):
        draw = _function_body(_strip_comments(_src(HUD_CPP)), "void AAdastreaHUD::DrawHUD")
        assert "UpdateMenuAudio();" in draw

    def test_editor_manager_hooks(self):
        mgr = _strip_comments(_src(MANAGER_CPP))
        assert "PlayPlaceEvent(this, NewModule)" in _function_body(mgr, "UStationEditorManager::PlaceModule_Implementation")
        assert 'TEXT("Editor.Remove")' in _function_body(mgr, "UStationEditorManager::RemoveModule_Implementation")
        assert 'TEXT("Editor.Undo")' in _function_body(mgr, "bool UStationEditorManager::Undo")
        assert 'TEXT("Editor.Redo")' in _function_body(mgr, "bool UStationEditorManager::Redo")
        assert 'TEXT("Editor.Save")' in _function_body(mgr, "UStationEditorManager::Save_Implementation")

    def test_editor_widget_hooks(self):
        widget = _strip_comments(_src(WIDGET_CPP))
        assert 'TEXT("Editor.Rotate")' in _function_body(widget, "UStationEditorWidgetCpp::RotatePlacement")
        assert 'TEXT("Editor.Invalid")' in _function_body(widget, "UStationEditorWidgetCpp::OnViewportClicked")
        assert 'TEXT("UI.Hover")' in _strip_comments(_src(LIST_ITEM_CPP))

    def test_console_chirp_on_terminals(self):
        body = _function_body(_strip_comments(_src(TERMINAL_CPP)), "void AStationTerminal::Interact_Implementation")
        assert 'TEXT("Interior.ConsoleChirp")' in body

    def test_cockpit_and_hum(self):
        pc = _strip_comments(_src(PC_CPP))
        enter = _function_body(pc, "void AAdastreaPlayerController::EnterShipInterior")
        exit_ = _function_body(pc, "void AAdastreaPlayerController::ExitShipInterior")
        assert 'TEXT("Interior.CockpitExit")' in enter and "StartInteriorHum();" in enter
        assert 'TEXT("Interior.CockpitEnter")' in exit_ and "StopInteriorHum();" in exit_


class TestLoadIsMuted:
    def test_apply_game_state_mutes_gameplay_audio(self):
        body = _function_body(_strip_comments(_src(SAVE_CPP)), "void USaveGameSubsystem::ApplyGameState")
        assert "UAudioEventLibrary::FScopedMute" in body


class TestLoopsAreReleased:
    def test_laser_loop_released_on_end_play(self):
        laser = _strip_comments(_src(LASER_CPP))
        end_play = _function_body(laser, "void UMiningLaserComponent::EndPlay")
        assert "FadeOutAndRelease(LaserLoopAudio" in end_play
        update = _function_body(laser, "void UMiningLaserComponent::UpdateLaserAudio")
        assert 'TEXT("Mining.LaserLoop")' in update and "SetPitchMultiplier" in update

    def test_interior_hum_released_on_end_play(self):
        body = _function_body(_strip_comments(_src(PC_CPP)), "void AAdastreaPlayerController::EndPlay")
        assert "FadeOutAndRelease(InteriorHumAudio" in body


class TestEventLogCVar:
    def test_cvar_exists(self):
        code = _src(LIBRARY_CPP)
        assert 'TEXT("adastrea.AudioEventLog")' in code
