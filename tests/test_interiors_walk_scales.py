#!/usr/bin/env python3
"""
Interior Walk-Scale Test Suite

Confirms the interior walk geometry stays in a human-walkable regime and that the
scale contracts enforced by the C++ interior/avatar code are consistent. Because
these scales are defined in C++ (computed at runtime from mesh bounds), this
suite reads the real source and asserts the source-of-truth constants and the
math around them, so a regression in the numbers fails loudly.

Run:  pytest tests/test_interiors_walk_scales.py
"""

import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
INTERIOR_CPP = PROJECT_ROOT / "Source" / "Adastrea" / "Private" / "Ships" / "SpaceshipInterior.cpp"
INTERIOR_H   = PROJECT_ROOT / "Source" / "Adastrea" / "Public" / "Ships" / "SpaceshipInterior.h"
AVATAR_CPP   = PROJECT_ROOT / "Source" / "Adastrea" / "Private" / "Ships" / "SpaceshipAvatar.cpp"
AVATAR_H     = PROJECT_ROOT / "Source" / "Adastrea" / "Public" / "Ships" / "SpaceshipAvatar.h"


def _src(path: Path) -> str:
    if not path.exists():
        raise FileNotFoundError(f"Missing source file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def _const(text: str, name: str, default=None):
    """Pull a named constant/float assignment out of C++ source."""
    # matches:  const float NAME = <val>;   or   NAME = <val>;   (incl. default member init)
    pats = [
        rf"const\s+float\s+{name}\s*=\s*([-0-9.]+f?)",
        rf"{name}\s*=\s*([-0-9.]+f?)",
        rf"{name}\s*=\s*([-0-9.]+f?)\s*;",
    ]
    for p in pats:
        for m in re.finditer(p, text, re.IGNORECASE):
            return float(m.group(1).rstrip("f"))
    return default


class TestInteriorScaleNormalization:
    """The shell is normalized to a human-walkable radius (~650 units)."""

    def _scale_src(self):
        return _src(INTERIOR_CPP)

    def test_target_radius_present(self):
        src = self._scale_src()
        r = _const(src, "TargetRadius")
        assert r is not None, "TargetRadius constant not found in ConfigureInterior"
        assert r == 650.0, f"TargetRadius expected 650.0, got {r}"

    def test_scale_is_radius_ratio(self):
        """Scale = TargetRadius / shell sphere radius; prevents a giant void."""
        src = self._scale_src()
        assert "TargetRadius / RawBounds.SphereRadius" in src, \
            "Scale must be TargetRadius divided by the mesh sphere radius"

    def test_scale_sets_relative_scale3d(self):
        """The computed scale is applied to the shell mesh uniformly."""
        src = self._scale_src()
        assert "SetRelativeScale3D(FVector(Scale, Scale, Scale))" in src, \
            "Shell must be scaled uniformly (x,y,z)"


class TestWalkExtentContracts:
    """The walk volume / confinement must stay inside a room-sized box."""

    def _interior_cpp(self):
        return _src(INTERIOR_CPP)

    def test_floor_defaults_sane(self):
        """Floor forward depth > width > human scale, ceiling a few metres."""
        h = _src(INTERIOR_H)
        fwd = _const(h, "FloorForwardDepth", 0)
        wid = _const(h, "FloorWidth", 0)
        ceil = _const(h, "CeilingHeight", 0)
        assert fwd > 500, f"FloorForwardDepth too small: {fwd}"
        assert wid > 300, f"FloorWidth too small: {wid}"
        assert 200 <= ceil <= 800, f"CeilingHeight out of human range: {ceil}"

    def test_walk_extents_have_floor(self):
        """GetLocalHalfExtents clamps below a 100-unit minimum so the walk box
        is never degenerate."""
        src = self._interior_cpp()
        assert "FMath::Max(Bounds.BoxExtent.X, 100.0f)" in src, \
            "X walk half-extent must have a 100u floor"
        assert "FMath::Max(Bounds.BoxExtent.Y, 100.0f)" in src, \
            "Y walk half-extent must have a 100u floor"

    def test_exit_trigger_front_of_room(self):
        """Exit trigger sits ~60% toward the front of the walk depth."""
        src = self._interior_cpp()
        assert "HalfDepth * 0.6f" in src, "Exit trigger should sit near the room front"


class TestAvatarFloorAltitude:
    """The avatar is held at a standing altitude (~96u) while walking an interior."""

    def _avatar_h(self):
        return _src(AVATAR_H)

    def test_interior_floor_altitude_present(self):
        src = self._avatar_h()
        assert "InteriorFloorAltitude" in src, "Avatar must expose an interior floor altitude"

    def test_floor_altitude_is_human_standing(self):
        """~96u (~0.5x the 192u capsule) so eyes sit near eye height, not sunk
        or hovering."""
        r = _const(self._avatar_h(), "InteriorFloorAltitude", -1)
        assert r is not None
        assert 60 <= r <= 140, f"InteriorFloorAltitude should be ~96u standing, got {r}"

    def test_avatar_confined_in_tick(self):
        """The avatar clamps position to the room each tick (can't walk through
        the shell into space)."""
        src = _src(AVATAR_CPP)
        assert "GetLocalHalfExtents(InteriorFloorAltitude, HP)" in src, \
            "Avatar Tick must query the room's local half-extents"
        assert "FMath::Clamp(LocalPos.X" in src and "FMath::Clamp(LocalPos.Y" in src, \
            "Avatar must clamp X and Y to the room extent"


class TestEntryPoint:
    """The entry point drops the avatar inside the room at a standing height."""

    def _interior_cpp(self):
        return _src(INTERIOR_CPP)

    def test_entry_is_room_center_standing(self):
        """Entry local point is (0,0,~200): centered, a couple units up so the
        avatar is standing (not embedded in the floor)."""
        src = self._interior_cpp()
        assert "FVector(0.0f, 0.0f, 200.0f)" in src, "Entry should be centered at Z=200 standing height"


class TestCompanionPartFamilies:
    """Multi-piece interiors mount their kit parts so rooms aren't empty shells."""

    def _interior_cpp(self):
        return _src(INTERIOR_CPP)

    def test_command_bridge_parts(self):
        src = self._interior_cpp()
        for part in ["Console", "Deck", "Lights", "Stations", "Viewport", "Hatch"]:
            assert f'TryPart(TEXT("{part}"))' in src, \
                f"CommandBridge should mount {part}"

    def test_crew_quarters_parts(self):
        src = self._interior_cpp()
        for part in ["Bunks", "Desks", "Galley", "Lights", "Mess", "Vents", "Hatch"]:
            assert f'TryPart(TEXT("{part}"))' in src, \
                f"CrewQuarters should mount {part}"

    def test_hab_parts(self):
        src = self._interior_cpp()
        for part in ["Bunks", "Desks", "Galley", "Lights", "Mess", "Vents", "Hatch"]:
            assert f'TryPart(TEXT("{part}"))' in src, \
                f"Hab should mount {part}"

    def test_parts_attached_to_scene_root(self):
        src = self._interior_cpp()
        assert "SetupAttachment(SceneRoot)" in src, "Parts must attach to SceneRoot"
        assert "InteriorParts.Add(Comp)" in src, "Parts must be recorded in InteriorParts"


class TestRevealUnhidesParts:
    """RevealInterior un-hides the shell and every mounted part."""

    def test_reveal_iterates_parts(self):
        src = _src(INTERIOR_CPP)
        assert "for (TObjectPtr<UStaticMeshComponent> Part : InteriorParts)" in src, \
            "RevealInterior must iterate and un-hide InteriorParts"
        assert 'Part->SetHiddenInGame(false)' in src, \
            "RevealInterior must un-hide each part"


if __name__ == "__main__":
    # Minimal standalone runner (no pytest needed)
    import traceback
    import unittest

    class _Loader(unittest.TestLoader):
        def getTestCaseNames(self, testCaseClass):
            return [n for n in unittest.TestCase.getTestCaseNames(self, testCaseClass)]

    suite = _Loader().discover(str(Path(__file__).parent), pattern=Path(__file__).name)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    sys.exit(0 if result.wasSuccessful() else 1)