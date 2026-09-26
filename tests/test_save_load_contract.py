#!/usr/bin/env python3
"""
Save/Load Contract Tests

The save system is C++ (USaveGameSubsystem / UAdastreaSaveGame), so like
test_interiors_walk_scales.py this suite reads the real source and asserts the
contracts that keep saves round-tripping:

- save versioning: older saves still load, and version-1 files (which never wrote
  SaveVersion, because SaveGameToSlot skips values equal to the class default)
  keep reading back as version 1
- the live-loop state is collected and applied: flown ship class + data asset,
  trader credits, cargo (incl. transient crafting-tree ore), docking, and
  player-built stations
- player-built stations/modules are tagged where they are created
- station layouts use the one Station Editor blueprint format, which the Python
  plan-mode tool (docs/11-TECHNICAL_SPECS/generate_station_builder.py) parses

Run:  pytest tests/test_save_load_contract.py
"""

import importlib.util
import re
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
SRC = PROJECT_ROOT / "Source"
SAVE_GAME_H = SRC / "Adastrea" / "Public" / "Player" / "AdastreaSaveGame.h"
SAVE_GAME_CPP = SRC / "Adastrea" / "Private" / "Player" / "AdastreaSaveGame.cpp"
SUBSYSTEM_H = SRC / "Adastrea" / "Public" / "Player" / "SaveGameSubsystem.h"
SUBSYSTEM_CPP = SRC / "Adastrea" / "Private" / "Player" / "SaveGameSubsystem.cpp"
STATION_H = SRC / "Adastrea" / "Public" / "Stations" / "SpaceStation.h"
STATION_CPP = SRC / "Adastrea" / "Private" / "Stations" / "SpaceStation.cpp"
PLAYER_CONTROLLER_CPP = SRC / "Adastrea" / "Private" / "Player" / "AdastreaPlayerController.cpp"
EDITOR_MANAGER_CPP = SRC / "StationEditor" / "Private" / "StationEditorManager.cpp"
STATION_BUILDER_PY = PROJECT_ROOT / "docs" / "11-TECHNICAL_SPECS" / "generate_station_builder.py"


def _src(path: Path) -> str:
    if not path.exists():
        raise FileNotFoundError(f"Missing source file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def _int_const(text: str, name: str) -> int:
    m = re.search(rf"static\s+constexpr\s+int32\s+{name}\s*=\s*(\d+)\s*;", text)
    assert m, f"{name} not found"
    return int(m.group(1))


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


class TestSaveVersioning:

    def test_version_constants_are_ordered(self):
        h = _src(SAVE_GAME_H)
        current = _int_const(h, "CURRENT_SAVE_VERSION")
        minimum = _int_const(h, "MIN_SUPPORTED_SAVE_VERSION")
        legacy = _int_const(h, "LEGACY_SAVE_VERSION")
        ship_stations = _int_const(h, "SHIP_AND_STATIONS_SAVE_VERSION")
        assert current >= 2, "ship/cargo/station data needs save version 2+"
        assert minimum <= legacy <= current
        assert minimum <= ship_stations <= current
        assert minimum == 1, "version-1 saves must keep loading"

    def test_constructor_default_is_legacy_not_current(self):
        """SaveGameToSlot delta-serializes against the CDO: a default of CURRENT would
        make every old file (which never wrote SaveVersion) read back as current."""
        cpp = _src(SAVE_GAME_CPP)
        ctor = _function_body(cpp, "UAdastreaSaveGame::UAdastreaSaveGame()")
        init_list = cpp[cpp.index("UAdastreaSaveGame::UAdastreaSaveGame()"):cpp.index(ctor)]
        assert "SaveVersion(LEGACY_SAVE_VERSION)" in init_list
        assert "SaveVersion(CURRENT_SAVE_VERSION)" not in init_list

    def test_save_stamps_current_version(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "bool USaveGameSubsystem::SaveGame(")
        assert "SaveVersion = UAdastreaSaveGame::CURRENT_SAVE_VERSION" in body

    def test_compatibility_is_a_range(self):
        body = _function_body(_src(SAVE_GAME_CPP), "bool UAdastreaSaveGame::IsCompatibleVersion()")
        assert "MIN_SUPPORTED_SAVE_VERSION" in body and "CURRENT_SAVE_VERSION" in body
        assert "SaveVersion == CURRENT_SAVE_VERSION" not in body

    def test_new_data_only_applied_for_new_saves(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "void USaveGameSubsystem::ApplyGameState(")
        assert "SHIP_AND_STATIONS_SAVE_VERSION" in body
        # Every v2 restore step sits behind that gate.
        gate = body.index("bHasShipAndStations =")
        for call in ("ApplyStations(", "ApplyPlayerShipClass(", "ApplyShipCreditsAndCargo(", "ApplyDocking("):
            assert call in body, f"{call} not applied on load"
            assert body.index(call) > gate


class TestLiveLoopCoverage:

    def test_save_game_has_ship_and_station_records(self):
        h = _src(SAVE_GAME_H)
        for struct in ("FSavedCargoEntry", "FSavedPlayerShip", "FSavedStation"):
            assert re.search(rf"struct\s+{struct}\b", h), f"{struct} missing"
        assert re.search(r"FSavedPlayerShip\s+PlayerShip\s*;", h)
        assert re.search(r"TArray<FSavedStation>\s+Stations\s*;", h)
        for field in ("ShipClass", "ShipDataAsset", "Credits", "StartingCredits", "Cargo",
                      "bDocked", "DockedStationName", "DockedSavedStationIndex", "DockedBayIndex",
                      "ItemAsset", "StationTransform", "Blueprint"):
            assert re.search(rf"\b{field}\s*;", h), f"saved field {field} missing"

    def test_collect_records_ship_and_stations(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "void USaveGameSubsystem::CollectGameState(")
        # Stations first: docking refers to saved stations by index.
        assert body.index("CollectStations(") < body.index("CollectPlayerShip(")

    def test_ship_record_uses_live_wallet_and_hold(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "void USaveGameSubsystem::CollectPlayerShip(")
        assert "PlayerTraderComponent" in body, "credits must come from the trader component the HUD uses"
        assert "CargoComponent" in body
        assert "ItemID" in body and "IsAsset()" in body, \
            "transient crafting-tree items have no asset path and must be saved by ID"
        assert "IsDocked()" in body

    def test_cargo_resolves_transient_items_by_id(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "UTradeItemDataAsset* USaveGameSubsystem::ResolveCargoItem(")
        assert "TryLoad()" in body
        assert "GetTradeItem(" in body, "crafting-tree ore (e.g. IronOre) must resolve via UCraftingTreeLoader"

    def test_ship_swap_sets_data_asset_before_begin_play(self):
        body = _function_body(_src(SUBSYSTEM_CPP), "ASpaceship* USaveGameSubsystem::ApplyPlayerShipClass(")
        assert "SpawnActorDeferred" in body
        assert body.index("ShipDataAsset = DataAsset") < body.index("FinishSpawning(")

    def test_load_refused_mid_edit(self):
        cpp = _src(SUBSYSTEM_CPP)
        assert "IsStationEditorOpen()" in _function_body(cpp, "FString USaveGameSubsystem::GetLoadBlocker()")
        assert "GetLoadBlocker()" in _function_body(cpp, "bool USaveGameSubsystem::LoadGame(")

    def test_console_commands_registered(self):
        cpp = _src(SUBSYSTEM_CPP)
        assert 'TEXT("adastrea.SaveGame")' in cpp
        assert 'TEXT("adastrea.LoadGame")' in cpp


class TestPlayerBuiltTagging:

    def test_tag_declared_once(self):
        assert "static const FName PlayerBuiltTag;" in _src(STATION_H)
        assert re.search(r'const FName ASpaceStation::PlayerBuiltTag\(TEXT\("PlayerBuilt"\)\);', _src(STATION_CPP))

    def test_builder_station_is_tagged(self):
        body = _function_body(_src(PLAYER_CONTROLLER_CPP), "ASpaceStation* AAdastreaPlayerController::SpawnStationForBuilder()")
        assert "ASpaceStation::PlayerBuiltTag" in body

    def test_editor_placed_modules_are_tagged(self):
        """Every placement path (place, import, construction queue) goes through SpawnModuleInternal."""
        body = _function_body(_src(EDITOR_MANAGER_CPP), "ASpaceStationModule* UStationEditorManager::SpawnModuleInternal(")
        assert "ASpaceStation::PlayerBuiltTag" in body

    def test_restored_modules_are_tagged(self):
        body = _function_body(_src(STATION_CPP), "int32 ASpaceStation::BuildFromBlueprintString(")
        assert "PlayerBuiltTag" in body


class TestStationBlueprintFormat:

    def _load_python_tool(self):
        spec = importlib.util.spec_from_file_location("generate_station_builder", STATION_BUILDER_PY)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module

    def test_editor_export_uses_shared_codec(self):
        cpp = _src(EDITOR_MANAGER_CPP)
        export = _function_body(cpp, "FString UStationEditorManager::ExportStationBlueprint()")
        assert "ExportBlueprintString(" in export
        importer = _function_body(cpp, "int32 UStationEditorManager::ImportStationBlueprint(")
        assert "ASpaceStation::ParseBlueprintString(" in importer

    def test_cpp_format_parses_in_python_tool(self):
        """Render the C++ printf formats with sample values and parse them with the
        Python plan-mode tool, so neither side can drift from the other."""
        body = _function_body(_src(STATION_CPP), "FString ASpaceStation::ExportBlueprintString(")
        header = re.search(r'TEXT\("(1\.0\.0;[^"]*)"\)\s*\+\s*FString::FromInt', body)
        entry = re.search(r'FString::Printf\(TEXT\("(;M%d:[^"]*)"\)', body)
        assert header and entry, "blueprint header/entry format not found"

        spacing = 10
        modules = [("CargoBayModule", -3, 0, 12, 270, 1), ("DockingBayModule", 40, -8, 0, 90, 0)]
        blob = header.group(1) + str(spacing)
        for i, (item, gx, gy, gz, rot, core) in enumerate(modules):
            blob += entry.group(1) % (i + 1, item, gx, gy, gz, rot, core)

        tool = self._load_python_tool()
        layout, errors = tool.blueprint_to_layout(blob)
        assert not errors, errors
        assert layout["GridSpacing"] == spacing
        parsed = [(m["ItemID"], *m["GridPos"], m["Rotation"], int(m["IsCore"])) for m in layout["Modules"]]
        assert parsed == modules

        # And back: the Python writer produces what the C++ parser expects.
        assert tool.layout_to_blueprint(layout) == blob
