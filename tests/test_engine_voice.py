#!/usr/bin/env python3
"""
Engine voice contract tests (UShipEngineAudioComponent).

Like the other source-contract tests, these read the real C++ and check the rules
that are easy to break without noticing in a build:

- every ASpaceship gets the component by default
- the voice comes from size (hull + cargo) and agility (accel x maneuverability),
  never from MaxSpeed, and a ship data asset can override it
- pitch stays within 0.7x..1.4x, including revs and boost on top of the base pitch
- bigger ships sound deeper: re-implementing DeriveVoice with the header's defaults,
  the effective body frequency falls monotonically with size across ShipClasses.json,
  and nimble ships whine more than sluggish ones
- AI engines are very short range (hull-relative box attenuation, at most 2 voices)
  and quieter than the player's; the player's engine is 2D and muffled on foot
- loops go through the audio catalog (SpawnEventAttached), never by asset path
- the debug console commands exist

Run:  pytest tests/test_engine_voice.py
"""

import json
import math
import re
from pathlib import Path

import pytest

PROJECT_ROOT = Path(__file__).parent.parent
SRC = PROJECT_ROOT / "Source" / "Adastrea"
COMP_H = SRC / "Public" / "Audio" / "ShipEngineAudioComponent.h"
COMP_CPP = SRC / "Private" / "Audio" / "ShipEngineAudioComponent.cpp"
SUBSYS_CPP = SRC / "Private" / "Audio" / "ShipEngineVoiceSubsystem.cpp"
PROFILE_H = SRC / "Public" / "Audio" / "ShipEngineSoundProfile.h"
SHIP_H = SRC / "Public" / "Ships" / "Spaceship.h"
SHIP_CPP = SRC / "Private" / "Ships" / "Spaceship.cpp"
DATA_H = SRC / "Public" / "Ships" / "SpaceshipDataAsset.h"
PC_CPP = SRC / "Private" / "Player" / "AdastreaPlayerController.cpp"
SHIP_CLASSES = PROJECT_ROOT / "Content" / "Data" / "ShipClasses.json"

# Body frequency centre of each generated loop family (SOUND_PLAN.md section 1).
FAMILY_CENTRE_HZ = {"Light": 125.0, "Medium": 80.0, "Heavy": 52.0, "Capital": 35.0}
FAMILIES = ["Light", "Medium", "Heavy", "Capital"]


def _src(path: Path) -> str:
    if not path.exists():
        raise FileNotFoundError(f"Missing source file: {path}")
    return path.read_text(encoding="utf-8-sig", errors="replace")


def _strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def _function_body(text: str, signature: str) -> str:
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


def _defaults() -> dict:
    """Numeric UPROPERTY defaults declared in the component header (`float X = 1.0f;`)."""
    h = _strip_comments(_src(COMP_H))
    out = {}
    for name, value in re.findall(r"\b(?:float|int32)\s+(\w+)\s*=\s*(-?[0-9.]+)f?\s*;", h):
        out[name] = float(value)
    return out


def _derive(d: dict, hull: float, cargo: float, accel: float, man: float) -> dict:
    """Python mirror of UShipEngineAudioComponent::DeriveVoice (no override)."""
    size_score = d["SizeHullWeight"] * math.log(max(hull, 1.0)) + d["SizeCargoWeight"] * math.log(1.0 + max(cargo, 0.0))
    agility_score = math.log(max(accel * man, 1.0))
    size = min(max((size_score - d["SizeScoreMin"]) / (d["SizeScoreMax"] - d["SizeScoreMin"]), 0.0), 1.0)
    agility = min(max((agility_score - d["AgilityScoreMin"]) / (d["AgilityScoreMax"] - d["AgilityScoreMin"]), 0.0), 1.0)
    scaled = size * 4.0
    family = min(max(int(math.floor(scaled)), 0), 3)
    in_band = min(max(scaled - family, 0.0), 1.0)
    pitch = d["FamilyPitchHigh"] + (d["FamilyPitchLow"] - d["FamilyPitchHigh"]) * in_band
    return {
        "size_score": size_score,
        "size": size,
        "agility": agility,
        "family": FAMILIES[family],
        "pitch": min(max(pitch, 0.7), 1.4),
        "whine": agility ** d["WhineExponent"],
    }


def _archetypes() -> dict:
    data = json.loads(SHIP_CLASSES.read_text(encoding="utf-8"))
    out = {}
    for c in data["classes"]:
        out[c["Class"]] = (
            c["Role"]["HullIntegrity"], c["Role"]["CargoCapacity"],
            c["Mobility"]["Acceleration"], c["Mobility"]["Maneuverability"],
        )
    return out


class TestWiring:

    def test_every_spaceship_has_the_component(self):
        h = _src(SHIP_H)
        assert re.search(r"TObjectPtr<UShipEngineAudioComponent>\s+EngineAudio\s*;", h)
        ctor = _function_body(_src(SHIP_CPP), "ASpaceship::ASpaceship()")
        assert "CreateDefaultSubobject<UShipEngineAudioComponent>" in ctor

    def test_data_asset_has_optional_profile(self):
        assert re.search(r"FShipEngineSoundProfile\s+EngineSoundProfile\s*;", _src(DATA_H))
        profile = _strip_comments(_src(PROFILE_H))
        for field in ("bOverride", "Family", "PitchOffset", "WhineAmount",
                      "LowRevsSound", "HighRevsSound", "WhineSound", "BoostSound", "IdleSound"):
            assert re.search(rf"\b{field}\b", profile), f"profile is missing {field}"
        assert re.search(r"bool\s+bOverride\s*=\s*false", profile), "the override must be off by default"

    def test_loops_come_from_the_catalog_not_asset_paths(self):
        cpp = _strip_comments(_src(COMP_CPP))
        assert "SpawnEventAttached(" in cpp
        assert "/Game/Audio/Engine" not in cpp, "engine sounds must be looked up by event ID"
        for event in ("Engine.Whine", "Engine.Boost", "Engine.BoostStart", "Engine.SpoolUp",
                      "Engine.SpoolDown", "Engine.Idle"):
            assert f'"{event}"' in cpp, f"missing event {event}"
        assert 'TEXT("Engine.%s.%s")' in cpp, "family loops are Engine.<Family>.<Low|High>"

    def test_console_commands(self):
        cpp = _src(SUBSYS_CPP)
        assert 'TEXT("adastrea.EngineVoiceReport")' in cpp
        assert 'TEXT("adastrea.EngineVoiceDebug")' in cpp

    def test_player_controller_exposes_ship_left_on_foot(self):
        body = _function_body(_src(PC_CPP), "AAdastreaPlayerController::GetShipLeftOnFoot() const")
        assert "InteriorSourceShip" in body and "StationVisitShip" in body


class TestVoiceDerivation:

    def test_max_speed_is_not_used_for_the_voice(self):
        body = _strip_comments(_function_body(_src(COMP_CPP), "UShipEngineAudioComponent::DeriveVoice("))
        assert "MaxSpeed" not in body
        assert "HullStrength" in body and "CargoCapacity" in body
        assert "Acceleration" in body and "Maneuverability" in body

    def test_override_is_honoured(self):
        body = _function_body(_src(COMP_CPP), "UShipEngineAudioComponent::DeriveVoice(")
        assert "Profile.bOverride" in body
        assert "bFromOverride = true" in body

    def test_pitch_stays_in_range_with_revs_and_boost(self):
        d = _defaults()
        assert 0.7 <= d["FamilyPitchLow"] < d["FamilyPitchHigh"] <= 1.4
        top = d["FamilyPitchHigh"] * (1.0 + d["RevPitchRise"] + d["BoostPitchRise"])
        assert top <= 1.4 + 0.05, f"lightest ship at full revs + boost reaches {top:.2f}x"
        assert d["RevPitchRise"] == pytest.approx(0.25)
        assert d["BoostPitchRise"] == pytest.approx(0.10)

    def test_bigger_ships_sound_deeper(self):
        d = _defaults()
        voices = {name: _derive(d, *stats) for name, stats in _archetypes().items()}
        by_size = sorted(voices.items(), key=lambda kv: kv[1]["size_score"])
        hz = [(name, FAMILY_CENTRE_HZ[v["family"]] * v["pitch"]) for name, v in by_size]
        for (a, fa), (b, fb) in zip(hz, hz[1:]):
            assert fa >= fb, f"{b} is bigger than {a} but sounds higher ({fb:.1f} Hz vs {fa:.1f} Hz)"
        assert voices["Fighter"]["family"] == "Light"
        assert FAMILIES.index(voices["Battleship"]["family"]) > FAMILIES.index(voices["Fighter"]["family"])

    def test_nimble_ships_whine_sluggish_ones_do_not(self):
        v = {name: _derive(_defaults(), *stats) for name, stats in _archetypes().items()}
        assert v["Fighter"]["whine"] > v["Freighter"]["whine"] > v["Battleship"]["whine"]
        assert v["Courier"]["whine"] > v["Freighter"]["whine"]
        assert v["Battleship"]["whine"] < 0.1, "a battleship should have almost no whine"

    def test_family_bands_join_without_pitch_inversion(self):
        # The smallest ship of a heavier family must not sound higher than the largest ship of a lighter one.
        d = _defaults()
        for lighter, heavier in zip(FAMILIES, FAMILIES[1:]):
            assert FAMILY_CENTRE_HZ[lighter] * d["FamilyPitchLow"] >= FAMILY_CENTRE_HZ[heavier] * d["FamilyPitchHigh"]


class TestPlayerAndAI:

    def test_ai_is_very_short_range_and_hull_relative(self):
        d = _defaults()
        assert d["AIFullVolumeRange"] <= 1500.0, "AI engines are full only within ~10 m of the hull"
        assert d["AISilentRange"] <= 6000.0, "AI engines are silent by ~50 m past the hull"
        assert d["AIFullVolumeRange"] < d["AISilentRange"]
        body = _function_body(_src(COMP_CPP), "UShipEngineAudioComponent::MakeAttenuation(")
        assert "EAttenuationShape::Box" in body
        assert "HullExtent + FVector(AIFullVolumeRange)" in body
        assert "AISilentRange - AIFullVolumeRange" in body

    def test_ai_is_quieter_and_voice_limited(self):
        d = _defaults()
        assert d["AIVolumeScale"] < 1.0
        assert d["MaxAIVoices"] == 2
        arbitrate = _function_body(_src(SUBSYS_CPP), "UShipEngineVoiceSubsystem::ArbitrateAIVoices()")
        assert "MaxAIVoices" in arbitrate and "AISilentRange" in arbitrate

    def test_player_engine_is_2d_and_muffled_on_foot(self):
        cpp = _src(COMP_CPP)
        att = _function_body(cpp, "UShipEngineAudioComponent::MakeAttenuation(")
        assert "bAttenuate = false" in att and "bSpatialize = false" in att
        mode = _function_body(cpp, "UShipEngineAudioComponent::ResolveMode()")
        assert "GetShipLeftOnFoot()" in mode and "PlayerMuffled" in mode
        tick = _function_body(cpp, "UShipEngineAudioComponent::TickComponent(")
        assert "SetLowPassFilterFrequency" in tick and "MuffledVolumeScale" in tick

    def test_parameters_are_smoothed(self):
        tick = _function_body(_src(COMP_CPP), "UShipEngineAudioComponent::TickComponent(")
        for smoothed in ("SmoothThrottle", "SmoothRev", "SmoothBoost", "SmoothMuffle"):
            assert re.search(rf"{smoothed}\s*=\s*FMath::FInterpTo", tick), f"{smoothed} must be interpolated"
