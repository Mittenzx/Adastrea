#!/usr/bin/env python3
"""
Blueprint Generator for Adastrea

Generates Blueprint assets programmatically using Unreal Engine's Python API.
Creates essential Blueprints for the space flight game including ships, stations,
AI controllers, and game systems.

Usage:
    from BlueprintGenerator import BlueprintGenerator
    generator = BlueprintGenerator()
    generator.generate_essential_blueprints()
"""

import unreal
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class BlueprintGenerator:
    """
    Generates Blueprint assets for the Adastrea project
    """

    def __init__(self):
        """Initialize the blueprint generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.generated_count = 0

    def generate_essential_blueprints(self) -> int:
        """
        Generate only essential blueprints required for basic functionality

        Returns:
            Number of blueprints created
        """
        print("[BlueprintGen] Generating essential blueprints...")

        count = 0

        # Essential game mode and player setup
        count += self._create_game_mode_blueprint()
        count += self._create_player_controller_blueprint()
        count += self._create_player_ship_blueprint()

        # Basic AI and NPC systems
        count += self._create_ai_controller_blueprint()
        count += self._create_faction_ai_blueprint()

        # Station and environment
        count += self._create_space_station_blueprint()
        count += self._create_station_module_blueprint()

        print(f"[BlueprintGen] Created {count} essential blueprints")
        return count

    def generate_all_blueprints(self) -> int:
        """
        Generate all blueprints including advanced systems

        Returns:
            Number of blueprints created
        """
        print("[BlueprintGen] Generating all blueprints...")

        count = self.generate_essential_blueprints()

        # Advanced systems
        count += self._create_advanced_ship_blueprints()
        count += self._create_weapon_system_blueprints()
        count += self._create_trading_system_blueprints()
        count += self._create_quest_system_blueprints()
        count += self._create_ui_system_blueprints()

        print(f"[BlueprintGen] Created {count} total blueprints")
        return count

    def _create_game_mode_blueprint(self) -> int:
        """Create BP_TestGameMode blueprint"""
        try:
            # Check if it already exists
            asset_path = "/Game/Blueprints/BP_TestGameMode"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_TestGameMode already exists")
                return 0

            # Create new Blueprint based on GameModeBase
            # Use the correct factory for Blueprint creation
            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.GameModeBase)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_TestGameMode",
                package_path="/Game/Blueprints",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_TestGameMode")
                return 1
            else:
                print(f"[BlueprintGen] ✗ Failed to create BP_TestGameMode")
                return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating game mode: {e}")
            return 0

    def _create_player_controller_blueprint(self) -> int:
        """Create BP_PlayerController blueprint"""
        try:
            asset_path = "/Game/Blueprints/Controllers/BP_PlayerController"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_PlayerController already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.PlayerController)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_PlayerController",
                package_path="/Game/Blueprints/Controllers",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_PlayerController")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating player controller: {e}")
            return 0

    def _create_player_ship_blueprint(self) -> int:
        """Create BP_PlayerShip blueprint"""
        try:
            asset_path = "/Game/Blueprints/Ships/BP_PlayerShip"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_PlayerShip already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.Pawn)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_PlayerShip",
                package_path="/Game/Blueprints/Ships",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_PlayerShip")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating player ship: {e}")
            return 0

    def _create_ai_controller_blueprint(self) -> int:
        """Create BP_AIController blueprint"""
        try:
            asset_path = "/Game/Blueprints/AI/BP_AIController"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_AIController already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.AIController)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_AIController",
                package_path="/Game/Blueprints/AI",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_AIController")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating AI controller: {e}")
            return 0

    def _create_faction_ai_blueprint(self) -> int:
        """Create BP_FactionAI blueprint"""
        try:
            asset_path = "/Game/Blueprints/AI/BP_FactionAI"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_FactionAI already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.Actor)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_FactionAI",
                package_path="/Game/Blueprints/AI",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_FactionAI")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating faction AI: {e}")
            return 0

    def _create_space_station_blueprint(self) -> int:
        """Create BP_SpaceStation blueprint"""
        try:
            asset_path = "/Game/Blueprints/Stations/BP_SpaceStation"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_SpaceStation already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.Actor)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_SpaceStation",
                package_path="/Game/Blueprints/Stations",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_SpaceStation")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating space station: {e}")
            return 0

    def _create_station_module_blueprint(self) -> int:
        """Create BP_StationModule blueprint"""
        try:
            asset_path = "/Game/Blueprints/Stations/BP_StationModule"
            if self.editor_asset_lib.does_asset_exist(asset_path):
                print(f"[BlueprintGen] BP_StationModule already exists")
                return 0

            factory = unreal.BlueprintFactory()
            factory.set_editor_property('parent_class', unreal.Actor)

            blueprint = self.asset_tools.create_asset(
                asset_name="BP_StationModule",
                package_path="/Game/Blueprints/Stations",
                asset_class=unreal.Blueprint,
                factory=factory
            )

            if blueprint:
                print(f"[BlueprintGen] ✓ Created BP_StationModule")
                return 1
            return 0

        except Exception as e:
            print(f"[BlueprintGen] Error creating station module: {e}")
            return 0

    def _create_advanced_ship_blueprints(self) -> int:
        """Create various ship type blueprints"""
        ship_types = [
            ("BP_Ship_Fighter", unreal.Pawn),
            ("BP_Ship_Cruiser", unreal.Pawn),
            ("BP_Ship_Freighter", unreal.Pawn),
            ("BP_Ship_Corvette", unreal.Pawn),
            ("BP_Ship_Destroyer", unreal.Pawn),
        ]

        count = 0
        for ship_name, parent_class in ship_types:
            try:
                asset_path = f"/Game/Blueprints/Ships/{ship_name}"
                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                factory = unreal.BlueprintFactory()
                factory.set_editor_property('parent_class', parent_class)

                blueprint = self.asset_tools.create_asset(
                    asset_name=ship_name,
                    package_path="/Game/Blueprints/Ships",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )

                if blueprint:
                    print(f"[BlueprintGen] ✓ Created {ship_name}")
                    count += 1

            except Exception as e:
                print(f"[BlueprintGen] Error creating {ship_name}: {e}")

        return count

    def _create_weapon_system_blueprints(self) -> int:
        """Create weapon system blueprints"""
        weapon_types = [
            ("BP_Weapon_Laser", unreal.ActorComponent),
            ("BP_Weapon_Plasma", unreal.ActorComponent),
            ("BP_Weapon_Missile", unreal.ActorComponent),
            ("BP_Weapon_Railgun", unreal.ActorComponent),
        ]

        count = 0
        for weapon_name, parent_class in weapon_types:
            try:
                asset_path = f"/Game/Blueprints/Weapons/{weapon_name}"
                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                factory = unreal.BlueprintFactory()
                factory.set_editor_property('parent_class', parent_class)

                blueprint = self.asset_tools.create_asset(
                    asset_name=weapon_name,
                    package_path="/Game/Blueprints/Weapons",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )

                if blueprint:
                    print(f"[BlueprintGen] ✓ Created {weapon_name}")
                    count += 1

            except Exception as e:
                print(f"[BlueprintGen] Error creating {weapon_name}: {e}")

        return count

    def _create_trading_system_blueprints(self) -> int:
        """Create trading system blueprints"""
        trading_types = [
            ("BP_Trading_Station", unreal.Actor),
            ("BP_Commodity_Market", unreal.Actor),
            ("BP_Trade_Route", unreal.Actor),
        ]

        count = 0
        for trade_name, parent_class in trading_types:
            try:
                asset_path = f"/Game/Blueprints/Trading/{trade_name}"
                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                factory = unreal.BlueprintFactory()
                factory.set_editor_property('parent_class', parent_class)

                blueprint = self.asset_tools.create_asset(
                    asset_name=trade_name,
                    package_path="/Game/Blueprints/Trading",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )

                if blueprint:
                    print(f"[BlueprintGen] ✓ Created {trade_name}")
                    count += 1

            except Exception as e:
                print(f"[BlueprintGen] Error creating {trade_name}: {e}")

        return count

    def _create_quest_system_blueprints(self) -> int:
        """Create quest system blueprints"""
        quest_types = [
            ("BP_Quest_Manager", unreal.Actor),
            ("BP_Quest_Objectives", unreal.Actor),
            ("BP_NPC_Quest_Giver", unreal.Actor),
        ]

        count = 0
        for quest_name, parent_class in quest_types:
            try:
                asset_path = f"/Game/Blueprints/Quests/{quest_name}"
                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                factory = unreal.BlueprintFactory()
                factory.set_editor_property('parent_class', parent_class)

                blueprint = self.asset_tools.create_asset(
                    asset_name=quest_name,
                    package_path="/Game/Blueprints/Quests",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )

                if blueprint:
                    print(f"[BlueprintGen] ✓ Created {quest_name}")
                    count += 1

            except Exception as e:
                print(f"[BlueprintGen] Error creating {quest_name}: {e}")

        return count

    def _create_ui_system_blueprints(self) -> int:
        """Create UI system blueprints"""
        ui_types = [
            ("BP_UI_HUD", unreal.UserWidget),
            ("BP_UI_Inventory", unreal.UserWidget),
            ("BP_UI_Trading", unreal.UserWidget),
            ("BP_UI_Quest_Log", unreal.UserWidget),
            ("BP_UI_Ship_Customization", unreal.UserWidget),
        ]

        count = 0
        for ui_name, parent_class in ui_types:
            try:
                asset_path = f"/Game/Blueprints/UI/{ui_name}"
                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                factory = unreal.BlueprintFactory()
                factory.set_editor_property('parent_class', parent_class)

                blueprint = self.asset_tools.create_asset(
                    asset_name=ui_name,
                    package_path="/Game/Blueprints/UI",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )

                if blueprint:
                    print(f"[BlueprintGen] ✓ Created {ui_name}")
                    count += 1

            except Exception as e:
                print(f"[BlueprintGen] Error creating {ui_name}: {e}")

        return count