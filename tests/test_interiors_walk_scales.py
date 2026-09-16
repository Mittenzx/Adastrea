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
        """FitVolumeToMesh clamps the walk footprint below a minimum so the
        walk box is never degenerate, even for a tiny/malformed shell mesh."""
        src = self._interior_cpp()
        assert "FMath::Max(Extent.X, 50.0f)" in src, \
            "X walk half-extent (HalfDepth) must have a floor"
        assert "FMath::Max(Extent.Y, 50.0f)" in src, \
            "Y walk half-extent (HalfWidth) must have a floor"
        assert "FMath::Max(Extent.Z, 100.0f)" in src, \
            "Z walk half-extent (HalfHeight) must have a floor"

    def test_exit_trigger_front_of_room(self):
        """Exit trigger sits ~60% toward the front of the walk depth."""
        src = self._interior_cpp()
        assert "HalfDepth * 0.6f" in src, "Exit trigger should sit near the room front"

    def test_walls_block_pawn_confining_the_room(self):
        """Four wall BoxComponents provide the real confinement (the avatar's
        own swept movement stops at them) — this is what replaced the old
        manual per-tick position clamp."""
        src = self._interior_cpp()
        for wall in ("WallNorth", "WallSouth", "WallEast", "WallWest"):
            assert f"{wall} = MakeWall" in src, f"{wall} must be constructed"
        assert 'Wall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block)' in src, \
            "Walls must block the Pawn channel to confine the avatar"


class TestAvatarFloorAltitude:
    """The avatar stands at human height and is held on the floor / confined to
    the room by real collision, not a manual position clamp (see MoveSafe/
    SnapToFloor in SpaceshipAvatar.cpp and the wall BoxComponents in
    SpaceshipInterior.cpp — the old GetLocalHalfExtents + per-tick clamp
    approach was replaced by swept movement against real wall/floor colliders)."""

    def _avatar_cpp(self):
        return _src(AVATAR_CPP)

    def test_capsule_is_human_standing(self):
        """~96u capsule half-height (~192u total) so the avatar reads as a
        standing human, not sunk or towering."""
        src = self._avatar_cpp()
        r = _const(src, "InitCapsuleSize", None)
        # InitCapsuleSize(Radius, HalfHeight) — pull the HalfHeight (2nd arg).
        m = re.search(r"InitCapsuleSize\(\s*[-0-9.]+f?\s*,\s*([-0-9.]+)f?\s*\)", src)
        assert m is not None, "Avatar must set its capsule size in the constructor"
        half_height = float(m.group(1))
        assert 60 <= half_height <= 140, f"Capsule half-height should be ~96u standing, got {half_height}"

    def test_avatar_moves_via_swept_collision(self):
        """Movement is a direct swept capsule translation (MoveSafe), which
        naturally stops at real wall/floor colliders — no manual room-extent
        clamp is needed or present."""
        src = self._avatar_cpp()
        assert "AddActorWorldOffset(WorldDelta, true, &Hit)" in src, \
            "MoveSafe must sweep the capsule so it collides with real walls/floor"
        assert "GetLocalHalfExtents" not in src, \
            "Manual room-extent clamping was replaced by real wall collision — should not reappear"

    def test_avatar_holds_floor_height(self):
        """SnapToFloor holds standing height via a downward line trace against
        the real floor collider, not a fixed/hardcoded altitude constant."""
        src = self._avatar_cpp()
        assert "LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn" in src, \
            "SnapToFloor must trace down against Pawn-blocking floor collision"
        assert "Hit.Location.Z + HalfHeight" in src, \
            "SnapToFloor must place the capsule center HalfHeight above the traced floor"


class TestEntryPoint:
    """The entry point drops the avatar inside the room at a standing height."""

    def _interior_cpp(self):
        return _src(INTERIOR_CPP)

    def test_entry_is_away_from_seat_trigger_standing(self):
        """Entry local point is offset toward the opposite end of the room from
        the seat/exit trigger (which sits at +0.6*HalfDepth), so the avatar
        doesn't spawn a couple steps from instantly re-triggering the
        return-to-ship overlap, and stands at a sane height (floor-relative,
        so it can't float/sink as the room's actual floor height varies).
        Offset by Origin.X/Y too — the mesh's bounding-box centre is often not
        at its pivot, so (0,0) alone would misplace the entry point relative
        to the actual visible geometry."""
        src = self._interior_cpp()
        assert "EntryLocation = FVector(Origin.X - HalfDepth * 0.6f, Origin.Y," in src, \
            "Entry should sit opposite the seat trigger (X = Origin.X - 0.6*HalfDepth), offset by Origin.Y"
        assert "FloorZ + 100.0f)" in src, \
            "Entry Z should be floor-relative (FloorZ + standing offset)"


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