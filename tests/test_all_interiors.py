#!/usr/bin/env python3
"""
Test All Interiors

Enumerates every interior mesh in the Adastrea kit and asserts (without needing
a running editor):
  1. Every interior asset file exists on disk.
  2. The interaction-significant "shell" meshes (the ones ConfigureInterior can be
     handed) map to a known interior type, and every companion part a family mounts
     in C++ actually has a matching asset on disk (so no part mounts as a dead end).
  3. The families defined in C++ (CommandBridge / CrewQuarters / Hab) cover the
     shell assets that carry companion parts.

Run: pytest tests/test_all_interiors.py
"""

import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
INTERIORS_DIR = PROJECT_ROOT / "Plugins" / "AdastreaShips" / "Content" / "Meshes" / "Interiors"
INTERIOR_CPP = PROJECT_ROOT / "Source" / "Adastrea" / "Private" / "Ships" / "SpaceshipInterior.cpp"

# The complete set of interior meshes shipped in the kit.
EXPECTED_INTERIORS = [
    "SM_Int_CommandBridge_Console",
    "SM_Int_CommandBridge_Deck",
    "SM_Int_CommandBridge_Hatch",
    "SM_Int_CommandBridge_Lights",
    "SM_Int_CommandBridge_Shell",
    "SM_Int_CommandBridge_Stations",
    "SM_Int_CommandBridge_Viewport",
    # Bespoke corvette bridge (CorvetteBridge family)
    "SM_Int_Corvette_Bridge_Console",
    "SM_Int_Corvette_Bridge_Deck",
    "SM_Int_Corvette_Bridge_Hatch",
    "SM_Int_Corvette_Bridge_Lights",
    "SM_Int_Corvette_Bridge_Shell",
    "SM_Int_Corvette_Bridge_Stations",
    "SM_Int_Corvette_Bridge_Viewport",
    "SM_Int_Fighter_Cabin",
    # v3 fighter cabin zones (Fighter family) + the retired v1 pair still on disk
    "SM_Int_Fighter_Cabin_Console",
    "SM_Int_Fighter_Cabin_Lights",
    "SM_Int_Fighter_Cabin_Shell",
    "SM_Int_Fighter_Cabin_Viewport",
    "SM_Int_Fighter_Cockpit",
    "SM_Int_Fighter_EmptyRoom",
    "SM_Int_Freighter_CrewQuarters",
    "SM_Int_Freighter_CrewQuarters_Bunks",
    "SM_Int_Freighter_CrewQuarters_Desks",
    "SM_Int_Freighter_CrewQuarters_Galley",
    "SM_Int_Freighter_CrewQuarters_Hatch",
    "SM_Int_Freighter_CrewQuarters_Lights",
    "SM_Int_Freighter_CrewQuarters_Mess",
    "SM_Int_Freighter_CrewQuarters_Shell",
    "SM_Int_Freighter_CrewQuarters_Vents",
    "SM_Int_Freighter_EngineRoom",
    "SM_Int_Generationship_Hab",
    "SM_Int_Generationship_Hab_Bunks",
    "SM_Int_Generationship_Hab_Desks",
    "SM_Int_Generationship_Hab_Galley",
    "SM_Int_Generationship_Hab_Hatch",
    "SM_Int_Generationship_Hab_Lights",
    "SM_Int_Generationship_Hab_Mess",
    "SM_Int_Generationship_Hab_Shell",
    "SM_Int_Generationship_Hab_Vents",
    "SM_Int_Standard_Airlock",
    "SM_Int_Standard_Corridor",
    "SM_Int_Xenomorph_AlienHold",
    # Capital bridges (BattleshipBridge / CommandXLBridge families), Roster art-gap
    # pass; imported by Tools/import_art_gap_assets.py.
    "SM_Int_Battleship_Bridge_Console",
    "SM_Int_Battleship_Bridge_Deck",
    "SM_Int_Battleship_Bridge_Hatch",
    "SM_Int_Battleship_Bridge_Lights",
    "SM_Int_Battleship_Bridge_Shell",
    "SM_Int_Battleship_Bridge_Stations",
    "SM_Int_Battleship_Bridge_Viewport",
    "SM_Int_CommandXL_Bridge_Console",
    "SM_Int_CommandXL_Bridge_Deck",
    "SM_Int_CommandXL_Bridge_Hatch",
    "SM_Int_CommandXL_Bridge_Lights",
    "SM_Int_CommandXL_Bridge_Shell",
    "SM_Int_CommandXL_Bridge_Stations",
    "SM_Int_CommandXL_Bridge_Viewport",
]

# C++ family string (MountInteriorParts) -> mesh prefix. A family that isn't in
# the C++ yet is simply never looked up.
FAMILY_PREFIX = {
    "CommandBridge": "SM_Int_CommandBridge",
    "CorvetteBridge": "SM_Int_Corvette_Bridge",
    "CrewQuarters": "SM_Int_Freighter_CrewQuarters",
    "Hab": "SM_Int_Generationship_Hab",
    "Fighter": "SM_Int_Fighter_Cabin",
    "BattleshipBridge": "SM_Int_Battleship_Bridge",
    "CommandXLBridge": "SM_Int_CommandXL_Bridge",
}

# Companion parts the C++ mounts per shell family (build from the source to stay
# in sync, but fall back to this if the source read fails).


def _cpp():
    return INTERIOR_CPP.read_text(encoding="utf-8", errors="replace")


def _families_from_cpp():
    """Return {family: [part,...]} parsed from MountInteriorParts.

    Handles combined branches such as
    'if (Family == TEXT("CommandBridge") || Family == TEXT("CorvetteBridge"))',
    which the old single-family regex silently skipped."""
    src = _cpp()
    families = {}
    blocks = re.findall(
        r'if \(((?:\s*(?:\|\|)?\s*Family == TEXT\("\w+"\))+)\)\s*\{(.*?)\n    \}', src, re.DOTALL)
    for cond, body in blocks:
        parts = re.findall(r'TryPart\(TEXT\("(\w+)"\)\)', body)
        for fam in re.findall(r'Family == TEXT\("(\w+)"\)', cond):
            families[fam] = parts
    return families


class TestAllInteriorsPresent:
    """Every interior mesh asset ships in the kit."""

    def test_expected_interiors_exist_on_disk(self):
        missing = [i for i in EXPECTED_INTERIORS if not (INTERIORS_DIR / f"{i}.uasset").exists()]
        assert not missing, f"Missing interior assets: {missing}"

    def test_no_unexpected_interior_only_assets_regressed(self):
        """The kit should not gain interiors outside the expected set without the
        test being extended (keeping the kit bounded)."""
        on_disk = sorted(p.name.removesuffix(".uasset") for p in INTERIORS_DIR.glob("SM_Int_*.uasset"))
        unexpected = [i for i in on_disk if i not in EXPECTED_INTERIORS]
        assert not unexpected, f"Unexpected interiors not in the test set: {unexpected}"

    def test_count_matches(self):
        on_disk = list(INTERIORS_DIR.glob("SM_Int_*.uasset"))
        assert len(on_disk) == len(EXPECTED_INTERIORS), (
            f"Count mismatch: {len(on_disk)} on disk vs {len(EXPECTED_INTERIORS)} expected"
        )


class TestEachInteriorIsEitherShellOrPart:
    """Every interior is either a selectable shell, or a companion part of a family."""

    def _shell_families(self):
        """Return {(prefix, family)} that the C++ mounts parts for."""
        return {fam: "/AdastreaShips/Meshes/Interiors/" + prefix
                for fam, prefix in FAMILY_PREFIX.items()}

    def test_every_companion_part_has_a_known_family_prefix(self):
        """A companion part (name with _<Part> suffix) must belong to a family the
        C++ mounts, else it's dead/unmountable."""
        parts = {
            "Bunks", "Desks", "Galley", "Hatch", "Lights", "Mess",
            "Vents", "Console", "Deck", "Stations", "Viewport",
        }
        for asset in EXPECTED_INTERIORS:
            # Identify companion parts by suffix.
            for part in parts:
                if asset.endswith("_" + part) or asset.endswith(part):
                    # It must belong to a family that mounts it.
                    ok = False
                    for family, prefix in self._shell_families().items():
                        if prefix.split("_")[-1] in asset:  # family token in the name
                            ok = True
                    assert ok, f"{asset} is a companion part but not tied to a known family"


class TestCompanionAssetsExistForMountedParts:
    """Every part the C++ family code mounts must have an asset on disk."""

    def test_mounted_parts_have_assets(self):
        families = _families_from_cpp()
        assert families, "Could not parse families from C++"
        for family, parts in families.items():
            base = FAMILY_PREFIX.get(family)
            assert base, f"Family {family} has no known mesh prefix"
            for part in parts:
                asset_name = f"{base}_{part}"
                assert (INTERIORS_DIR / f"{asset_name}.uasset").exists(), (
                    f"C++ mounts {asset_name} for {family} but the asset is missing"
                )


class TestShellMeshesAreSelectable:
    """The 'shell' style meshes (that the walk volume is fitted to) resolve for
    each multi-piece family, plus the singletons."""

    def test_each_family_has_a_shell_asset(self):
        shells = {
            "SM_Int_CommandBridge_Shell",
            "SM_Int_Freighter_CrewQuarters_Shell",
            "SM_Int_Generationship_Hab_Shell",
        }
        for s in shells:
            assert (INTERIORS_DIR / f"{s}.uasset").exists(), f"Missing shell {s}"

    def test_singleton_interiors_present(self):
        for s in [
            "SM_Int_Fighter_Cabin",
            "SM_Int_Freighter_EngineRoom",
            "SM_Int_Standard_Airlock",
            "SM_Int_Standard_Corridor",
            "SM_Int_Xenomorph_AlienHold",
        ]:
            assert (INTERIORS_DIR / f"{s}.uasset").exists(), f"Missing singleton {s}"


if __name__ == "__main__":
    import unittest

    loader = unittest.TestLoader()
    suite = loader.discover(str(Path(__file__).parent), pattern=Path(__file__).name)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    sys.exit(0 if result.wasSuccessful() else 1)