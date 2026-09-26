#!/usr/bin/env python3
"""
Economy contract tests: one wallet, and F5/F9 quicksave/quickload.

Like test_save_load_contract.py, these read the real C++ source and assert the
contracts that are easy to regress without noticing in a build:

- the Station Editor pays from the player's UPlayerTraderComponent (the HUD
  wallet that save v2 stores), not from a separate PlayerCredits budget
- building charges, undo/cancel refunds, and upgrades all go through the trader
- with no trader, costed modules are unaffordable (not free)
- StationEditor may depend on Adastrea, but never the other way round
- F5/F9 are runtime Enhanced Input actions on the player controller (so they
  work on foot and at the helm), calling QuickSave/QuickLoad with visible feedback
- UAdastreaHUDWidget::ShowAlert has a native, visible fallback

Run:  pytest tests/test_economy_wallet_quicksave.py
"""

import re
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent
SRC = PROJECT_ROOT / "Source"
MANAGER_H = SRC / "StationEditor" / "Public" / "StationEditorManager.h"
MANAGER_CPP = SRC / "StationEditor" / "Private" / "StationEditorManager.cpp"
WIDGET_H = SRC / "StationEditor" / "Public" / "UI" / "StationEditorWidgetCpp.h"
WIDGET_CPP = SRC / "StationEditor" / "Public" / "UI" / "StationEditorWidgetCpp.cpp"
STATION_EDITOR_BUILD = SRC / "StationEditor" / "StationEditor.Build.cs"
ADASTREA_BUILD = SRC / "Adastrea" / "Adastrea.Build.cs"
PC_H = SRC / "Adastrea" / "Public" / "Player" / "AdastreaPlayerController.h"
PC_CPP = SRC / "Adastrea" / "Private" / "Player" / "AdastreaPlayerController.cpp"
HUD_H = SRC / "Adastrea" / "Public" / "UI" / "AdastreaHUDWidget.h"
HUD_CPP = SRC / "Adastrea" / "Private" / "UI" / "AdastreaHUDWidget.cpp"
SAVE_SUBSYSTEM_H = SRC / "Adastrea" / "Public" / "Player" / "SaveGameSubsystem.h"


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


def _dependency_names(build_cs: str) -> set:
    code = _strip_comments(build_cs)
    return set(re.findall(r'"([A-Za-z0-9_]+)"', code))


class TestStationEditorUsesTraderWallet:

    def test_no_separate_credit_budget(self):
        """PlayerCredits used to be the editor's own budget; it must not come back."""
        for path in (MANAGER_H, MANAGER_CPP, WIDGET_H, WIDGET_CPP):
            code = _strip_comments(_src(path))
            assert not re.search(r"\bPlayerCredits\b", code), f"{path.name} still uses a PlayerCredits budget"
            assert "DefaultPlayerCredits" not in code, f"{path.name} still seeds a fake wallet"

    def test_trader_is_a_weak_pointer_like_cargo(self):
        h = _src(MANAGER_H)
        assert re.search(r"TWeakObjectPtr<UPlayerTraderComponent>\s+PlayerTrader\s*;", h)
        assert "UPlayerTraderComponent* GetPlayerTrader() const;" in h
        assert "int32 GetPlayerCredits() const;" in h

    def test_trader_resolves_from_the_piloted_ship(self):
        body = _function_body(_src(MANAGER_CPP), "UStationEditorManager::GetPlayerTrader() const")
        assert "PlayerTrader.IsValid()" in body
        assert "bAutoResolvePlayerTrader" in body
        assert "PlayerTraderComponent" in body, "must use the ship's trader (the HUD/save wallet)"

    def test_charge_recharge_refund_upgrade_go_through_the_trader(self):
        cpp = _src(MANAGER_CPP)
        charge = _function_body(cpp, "UStationEditorManager::ChargeForModule(")
        recharge = _function_body(cpp, "UStationEditorManager::RechargeSpend(")
        refund = _function_body(cpp, "UStationEditorManager::RefundSpend(")
        upgrade = _function_body(cpp, "UStationEditorManager::UpgradeModule(")
        assert "RemoveCredits(" in charge
        assert "RemoveCredits(" in recharge
        assert "AddCredits(" in refund
        assert "RemoveCredits(" in upgrade

    def test_affordability_reads_the_trader_and_needs_a_wallet(self):
        body = _function_body(_src(MANAGER_CPP), "UStationEditorManager::CanAffordModule(")
        assert "GetPlayerCredits()" in body
        # Free modules are always affordable; costed ones need credits, and
        # GetPlayerCredits() is 0 with no trader, so they're blocked, not free.
        credits = _function_body(_src(MANAGER_CPP), "UStationEditorManager::GetPlayerCredits() const")
        assert re.search(r"Trader\s*\?\s*Trader->GetCredits\(\)\s*:\s*0", credits)

    def test_widget_shows_the_trader_wallet(self):
        cpp = _src(WIDGET_CPP)
        assert "EditorManager->GetPlayerCredits()" in cpp


class TestModuleDependencies:

    def test_station_editor_depends_on_adastrea(self):
        assert "Adastrea" in _dependency_names(_src(STATION_EDITOR_BUILD))

    def test_adastrea_does_not_depend_on_station_editor(self):
        assert "StationEditor" not in _dependency_names(_src(ADASTREA_BUILD)), \
            "Adastrea -> StationEditor would create a module cycle"


class TestQuickSaveQuickLoad:

    def test_runtime_actions_and_context_on_the_controller(self):
        h = _src(PC_H)
        for name in ("SystemMappingContext", "QuickSaveAction", "QuickLoadAction"):
            assert re.search(rf"UPROPERTY\(Transient\)\s*TObjectPtr<class \w+>\s+{name}\s*;", h), name

        cpp = _src(PC_CPP)
        create = _function_body(cpp, "AAdastreaPlayerController::CreateSystemInput()")
        assert "NewObject<UInputAction>" in create
        assert "NewObject<UInputMappingContext>" in create
        assert "MapKey(QuickSaveAction, EKeys::F5)" in create
        assert "MapKey(QuickLoadAction, EKeys::F9)" in create

    def test_bound_and_context_survives_pawn_swaps(self):
        cpp = _src(PC_CPP)
        setup = _function_body(cpp, "AAdastreaPlayerController::SetupInputComponent()")
        assert "BindAction(QuickSaveAction" in setup
        assert "BindAction(QuickLoadAction" in setup
        # Controller-owned, re-added on possess: works on foot and at the helm.
        assert "AddSystemMappingContext()" in _function_body(cpp, "AAdastreaPlayerController::BeginPlay()")
        assert "AddSystemMappingContext()" in _function_body(cpp, "AAdastreaPlayerController::OnPossess(")

    def test_handlers_call_the_save_subsystem_with_feedback(self):
        cpp = _src(PC_CPP)
        save = _function_body(cpp, "AAdastreaPlayerController::HandleQuickSave()")
        load = _function_body(cpp, "AAdastreaPlayerController::HandleQuickLoad()")
        assert "QuickSave()" in save and "Game saved" in save
        assert "QuickLoad()" in load and "Game loaded" in load
        assert "DoesSaveExist" in load and "No quicksave" in load
        assert "GetLoadBlocker()" in load

    def test_load_blocker_is_public(self):
        h = _src(SAVE_SUBSYSTEM_H)
        public_part = h[:h.index("protected:")]
        assert "FString GetLoadBlocker() const;" in public_part


class TestHUDAlertFallback:

    def test_show_alert_has_a_native_body(self):
        body = _function_body(_src(HUD_CPP), "UAdastreaHUDWidget::ShowAlert_Implementation(")
        code = _strip_comments(body)
        assert "SetText(Message)" in code, "ShowAlert must display something natively"
        assert "SetTimer" in code, "the toast must hide after its duration"
        assert "AlertTextBlock" in _src(HUD_H)
