#!/usr/bin/env python3
"""
UI Widget Generator for Adastrea

Creates User Widget Blueprints for the space flight game interface.
Generates HUD, inventory, trading, quest log, and other UI components.

Usage:
    from UIWidgetGenerator import UIWidgetGenerator
    generator = UIWidgetGenerator()
    generator.generate_all_widgets()
"""

import unreal
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class UIWidgetGenerator:
    """
    Generates UI widget blueprints for Adastrea
    """

    def __init__(self):
        """Initialize the UI widget generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.generated_count = 0

    def generate_all_widgets(self) -> int:
        """
        Generate all UI widget blueprints

        Returns:
            Number of widgets created
        """
        print("[UIWidgetGen] Generating UI widget blueprints...")

        count = 0

        # Core UI widgets
        count += self._create_hud_widget()
        count += self._create_inventory_widget()
        count += self._create_trading_widget()
        count += self._create_quest_log_widget()
        count += self._create_ship_customization_widget()

        # Additional UI widgets
        count += self._create_menu_widgets()
        count += self._create_notification_widgets()

        print(f"[UIWidgetGen] Created {count} UI widget blueprints")
        return count

    def _create_hud_widget(self) -> int:
        """Create main HUD widget"""
        try:
            widget_name = "WBP_HUD_Main"
            widget_path = f"/Game/UI/HUD/{widget_name}"

            if self.editor_asset_lib.does_asset_exist(widget_path):
                print(f"[UIWidgetGen] {widget_name} already exists")
                return 0

            # Create User Widget Blueprint
            widget_bp = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path="/Game/UI/HUD",
                asset_class=unreal.WidgetBlueprint,
                factory=unreal.WidgetBlueprintFactory()
            )

            if widget_bp:
                print(f"[UIWidgetGen] ✓ Created {widget_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[UIWidgetGen] Error creating HUD widget: {e}")
            return 0

    def _create_inventory_widget(self) -> int:
        """Create inventory widget"""
        try:
            widget_name = "WBP_Inventory"
            widget_path = f"/Game/UI/Inventory/{widget_name}"

            if self.editor_asset_lib.does_asset_exist(widget_path):
                print(f"[UIWidgetGen] {widget_name} already exists")
                return 0

            widget_bp = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path="/Game/UI/Inventory",
                asset_class=unreal.WidgetBlueprint,
                factory=unreal.WidgetBlueprintFactory()
            )

            if widget_bp:
                print(f"[UIWidgetGen] ✓ Created {widget_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[UIWidgetGen] Error creating inventory widget: {e}")
            return 0

    def _create_trading_widget(self) -> int:
        """Create trading interface widget"""
        try:
            widget_name = "WBP_Trading_Interface"
            widget_path = f"/Game/UI/Trading/{widget_name}"

            if self.editor_asset_lib.does_asset_exist(widget_path):
                print(f"[UIWidgetGen] {widget_name} already exists")
                return 0

            widget_bp = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path="/Game/UI/Trading",
                asset_class=unreal.WidgetBlueprint,
                factory=unreal.WidgetBlueprintFactory()
            )

            if widget_bp:
                print(f"[UIWidgetGen] ✓ Created {widget_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[UIWidgetGen] Error creating trading widget: {e}")
            return 0

    def _create_quest_log_widget(self) -> int:
        """Create quest log widget"""
        try:
            widget_name = "WBP_Quest_Log"
            widget_path = f"/Game/UI/Quests/{widget_name}"

            if self.editor_asset_lib.does_asset_exist(widget_path):
                print(f"[UIWidgetGen] {widget_name} already exists")
                return 0

            widget_bp = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path="/Game/UI/Quests",
                asset_class=unreal.WidgetBlueprint,
                factory=unreal.WidgetBlueprintFactory()
            )

            if widget_bp:
                print(f"[UIWidgetGen] ✓ Created {widget_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[UIWidgetGen] Error creating quest log widget: {e}")
            return 0

    def _create_ship_customization_widget(self) -> int:
        """Create ship customization widget"""
        try:
            widget_name = "WBP_Ship_Customization"
            widget_path = f"/Game/UI/Ship/{widget_name}"

            if self.editor_asset_lib.does_asset_exist(widget_path):
                print(f"[UIWidgetGen] {widget_name} already exists")
                return 0

            widget_bp = self.asset_tools.create_asset(
                asset_name=widget_name,
                package_path="/Game/UI/Ship",
                asset_class=unreal.WidgetBlueprint,
                factory=unreal.WidgetBlueprintFactory()
            )

            if widget_bp:
                print(f"[UIWidgetGen] ✓ Created {widget_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[UIWidgetGen] Error creating ship customization widget: {e}")
            return 0

    def _create_menu_widgets(self) -> int:
        """Create menu-related widgets"""
        menu_widgets = [
            ("WBP_Main_Menu", "/Game/UI/Menus"),
            ("WBP_Pause_Menu", "/Game/UI/Menus"),
            ("WBP_Settings_Menu", "/Game/UI/Menus"),
            ("WBP_Save_Load_Menu", "/Game/UI/Menus"),
        ]

        count = 0
        for widget_name, package_path in menu_widgets:
            try:
                widget_path = f"{package_path}/{widget_name}"

                if self.editor_asset_lib.does_asset_exist(widget_path):
                    continue

                widget_bp = self.asset_tools.create_asset(
                    asset_name=widget_name,
                    package_path=package_path,
                    asset_class=unreal.WidgetBlueprint,
                    factory=unreal.WidgetBlueprintFactory()
                )

                if widget_bp:
                    print(f"[UIWidgetGen] ✓ Created {widget_name}")
                    count += 1

            except Exception as e:
                print(f"[UIWidgetGen] Error creating {widget_name}: {e}")

        return count

    def _create_notification_widgets(self) -> int:
        """Create notification and popup widgets"""
        notification_widgets = [
            ("WBP_Notification_Popup", "/Game/UI/Notifications"),
            ("WBP_Objective_Tracker", "/Game/UI/HUD"),
            ("WBP_Damage_Indicator", "/Game/UI/HUD"),
            ("WBP_Mini_Map", "/Game/UI/HUD"),
            ("WBP_Chat_Window", "/Game/UI/Communication"),
        ]

        count = 0
        for widget_name, package_path in notification_widgets:
            try:
                widget_path = f"{package_path}/{widget_name}"

                if self.editor_asset_lib.does_asset_exist(widget_path):
                    continue

                widget_bp = self.asset_tools.create_asset(
                    asset_name=widget_name,
                    package_path=package_path,
                    asset_class=unreal.WidgetBlueprint,
                    factory=unreal.WidgetBlueprintFactory()
                )

                if widget_bp:
                    print(f"[UIWidgetGen] ✓ Created {widget_name}")
                    count += 1

            except Exception as e:
                print(f"[UIWidgetGen] Error creating {widget_name}: {e}")

        return count