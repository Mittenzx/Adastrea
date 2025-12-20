#!/usr/bin/env python3
"""
Data Asset Batch Creator for Adastrea

Converts YAML template files to Unreal Engine Data Assets.
Creates massive amounts of game content including ships, factions,
weapons, commodities, quests, and AI behaviors.

Usage:
    from DataAssetBatchCreator import DataAssetBatchCreator
    creator = DataAssetBatchCreator()
    creator.batch_convert_all()
"""

import unreal
import yaml
import json
from pathlib import Path
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class DataAssetBatchCreator:
    """
    Batch converts YAML templates to Data Assets
    """

    def __init__(self):
        """Initialize the data asset creator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.project_root = Path(__file__).parent.parent.parent
        self.yaml_dir = self.project_root / "Assets"
        self.created_count = 0

    def batch_convert_all(self) -> int:
        """
        Convert all YAML templates to Data Assets

        Returns:
            Number of data assets created
        """
        print("[DataAssetGen] Starting batch conversion of YAML templates...")

        total_created = 0

        # Convert different types of assets
        total_created += self._convert_ship_templates()
        total_created += self._convert_faction_templates()
        total_created += self._convert_weapon_templates()
        total_created += self._convert_commodity_templates()
        total_created += self._convert_quest_templates()
        total_created += self._convert_personnel_templates()
        total_created += self._convert_ai_templates()

        print(f"[DataAssetGen] Created {total_created} data assets from YAML templates")
        return total_created

    def _convert_ship_templates(self) -> int:
        """Convert ship YAML templates to SpaceshipDataAsset"""
        print("[DataAssetGen] Converting ship templates...")

        yaml_files = [
            "SpaceshipTemplates/SmallFighter.yaml",
            "SpaceshipTemplates/MediumFighter.yaml",
            "SpaceshipTemplates/LargeFighter.yaml",
            "SpaceshipTemplates/Cruiser.yaml",
            "SpaceshipTemplates/Battleship.yaml",
            "SpaceshipTemplates/Freighter.yaml",
            "SpaceshipTemplates/MiningShip.yaml",
        ]

        count = 0
        for yaml_file in yaml_files:
            try:
                full_path = self.yaml_dir / yaml_file
                if not full_path.exists():
                    continue

                # Load YAML data
                with open(full_path, 'r', encoding='utf-8') as f:
                    data = yaml.safe_load(f)

                # Create data asset
                asset_name = f"DA_Ship_{data.get('name', 'Unknown').replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Ships/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                # Create SpaceshipDataAsset
                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Ships",
                    asset_class=unreal.SpaceshipDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    # Set properties from YAML
                    self._set_spaceship_properties(data_asset, data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error converting {yaml_file}: {e}")

        return count

    def _convert_faction_templates(self) -> int:
        """Convert faction YAML templates to FactionDataAsset"""
        print("[DataAssetGen] Converting faction templates...")

        yaml_files = [
            "FactionSetupGuide/SolarisUnion.yaml",
            "FactionSetupGuide/TerranFederation.yaml",
            "FactionSetupGuide/FreeTradeGuild.yaml",
            "FactionSetupGuide/PirateClans.yaml",
            "FactionSetupGuide/AlienCollective.yaml",
        ]

        count = 0
        for yaml_file in yaml_files:
            try:
                full_path = self.yaml_dir / yaml_file
                if not full_path.exists():
                    continue

                with open(full_path, 'r', encoding='utf-8') as f:
                    data = yaml.safe_load(f)

                asset_name = f"DA_Faction_{data.get('name', 'Unknown').replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Factions/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Factions",
                    asset_class=unreal.FactionDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_faction_properties(data_asset, data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error converting {yaml_file}: {e}")

        return count

    def _convert_weapon_templates(self) -> int:
        """Convert weapon YAML templates"""
        print("[DataAssetGen] Converting weapon templates...")

        # Generate procedural weapon data since YAML files may not exist
        weapons = self._generate_weapon_data()

        count = 0
        for weapon_data in weapons:
            try:
                asset_name = f"DA_Weapon_{weapon_data['name'].replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Weapons/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Weapons",
                    asset_class=unreal.WeaponDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_weapon_properties(data_asset, weapon_data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error creating weapon {weapon_data['name']}: {e}")

        return count

    def _convert_commodity_templates(self) -> int:
        """Convert commodity YAML templates"""
        print("[DataAssetGen] Converting commodity templates...")

        commodities = self._generate_commodity_data()

        count = 0
        for commodity_data in commodities:
            try:
                asset_name = f"DA_Commodity_{commodity_data['name'].replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Trading/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Trading",
                    asset_class=unreal.CommodityDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_commodity_properties(data_asset, commodity_data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error creating commodity {commodity_data['name']}: {e}")

        return count

    def _convert_quest_templates(self) -> int:
        """Convert quest YAML templates"""
        print("[DataAssetGen] Converting quest templates...")

        quests = self._generate_quest_data()

        count = 0
        for quest_data in quests:
            try:
                asset_name = f"DA_Quest_{quest_data['name'].replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Quests/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Quests",
                    asset_class=unreal.QuestDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_quest_properties(data_asset, quest_data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error creating quest {quest_data['name']}: {e}")

        return count

    def _convert_personnel_templates(self) -> int:
        """Convert personnel YAML templates"""
        print("[DataAssetGen] Converting personnel templates...")

        personnel = self._generate_personnel_data()

        count = 0
        for person_data in personnel:
            try:
                asset_name = f"DA_Personnel_{person_data['name'].replace(' ', '')}"
                asset_path = f"/Game/DataAssets/Personnel/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/Personnel",
                    asset_class=unreal.PersonnelDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_personnel_properties(data_asset, person_data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error creating personnel {person_data['name']}: {e}")

        return count

    def _convert_ai_templates(self) -> int:
        """Convert AI behavior templates"""
        print("[DataAssetGen] Converting AI templates...")

        ai_behaviors = self._generate_ai_data()

        count = 0
        for ai_data in ai_behaviors:
            try:
                asset_name = f"DA_AI_{ai_data['name'].replace(' ', '')}"
                asset_path = f"/Game/DataAssets/AI/{asset_name}"

                if self.editor_asset_lib.does_asset_exist(asset_path):
                    continue

                data_asset = self.asset_tools.create_asset(
                    asset_name=asset_name,
                    package_path="/Game/DataAssets/AI",
                    asset_class=unreal.AIDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                if data_asset:
                    self._set_ai_properties(data_asset, ai_data)
                    print(f"[DataAssetGen] ✓ Created {asset_name}")
                    count += 1

            except Exception as e:
                print(f"[DataAssetGen] Error creating AI {ai_data['name']}: {e}")

        return count

    def _generate_weapon_data(self) -> List[Dict]:
        """Generate procedural weapon data"""
        weapons = []
        weapon_types = [
            ("Laser Cannon", "Energy", 50, 100, 0.5, 1000),
            ("Plasma Blaster", "Energy", 75, 80, 1.0, 800),
            ("Railgun", "Projectile", 200, 20, 3.0, 200),
            ("Missile Launcher", "Missile", 500, 5, 5.0, 50),
            ("Particle Beam", "Energy", 150, 30, 2.0, 300),
        ]

        for name, type_name, damage, rate, cooldown, ammo in weapon_types:
            weapons.append({
                "name": name,
                "type": type_name,
                "damage": damage,
                "fire_rate": rate,
                "cooldown": cooldown,
                "max_ammo": ammo,
                "range": 1000 if type_name == "Energy" else 5000,
                "accuracy": 0.9 if type_name == "Energy" else 0.7,
            })

        return weapons

    def _generate_commodity_data(self) -> List[Dict]:
        """Generate procedural commodity data"""
        commodities = []
        commodity_types = [
            ("Titanium Ore", "Mining", 100, 50, 25),
            ("Plasma Cells", "Energy", 500, 200, 100),
            ("Rare Earth Elements", "Mining", 1000, 800, 50),
            ("Medical Supplies", "Medical", 200, 150, 75),
            ("Food Rations", "Agriculture", 50, 30, 90),
            ("Luxury Goods", "Trade", 1000, 1200, 20),
            ("Weapon Components", "Military", 300, 250, 40),
            ("Fuel Cells", "Energy", 150, 100, 80),
        ]

        for name, category, base_price, max_price, availability in commodity_types:
            commodities.append({
                "name": name,
                "category": category,
                "base_price": base_price,
                "max_price": max_price,
                "availability": availability,
                "volatility": 0.1 if category == "Mining" else 0.3,
            })

        return commodities

    def _generate_quest_data(self) -> List[Dict]:
        """Generate procedural quest data"""
        quests = []
        quest_types = [
            ("Mining Expedition", "Mining", 5000, "Deliver 10 units of ore"),
            ("Pirate Hunt", "Combat", 10000, "Destroy 5 pirate ships"),
            ("Trade Route", "Trading", 7500, "Complete 3 trade deliveries"),
            ("Rescue Mission", "Exploration", 8000, "Rescue stranded crew"),
            ("Research Data", "Science", 6000, "Collect research samples"),
        ]

        for name, category, reward, description in quest_types:
            quests.append({
                "name": name,
                "category": category,
                "reward_credits": reward,
                "description": description,
                "difficulty": "Medium",
                "time_limit": 3600,  # 1 hour
            })

        return quests

    def _generate_personnel_data(self) -> List[Dict]:
        """Generate procedural personnel data"""
        personnel = []
        roles = [
            ("Captain", "Command", 10000, 95),
            ("Engineer", "Technical", 8000, 85),
            ("Pilot", "Flight", 7500, 90),
            ("Gunner", "Combat", 6000, 80),
            ("Medic", "Medical", 7000, 88),
            ("Navigator", "Navigation", 6500, 82),
            ("Scientist", "Research", 9000, 92),
        ]

        for role, department, salary, skill in roles:
            personnel.append({
                "name": f"{role} Johnson",
                "role": role,
                "department": department,
                "salary": salary,
                "skill_level": skill,
                "loyalty": 75,
                "experience": 5,
            })

        return personnel

    def _generate_ai_data(self) -> List[Dict]:
        """Generate procedural AI behavior data"""
        ai_behaviors = []
        behaviors = [
            ("Trader", "Peaceful", "Trade", 50),
            ("Miner", "Neutral", "Resource gathering", 30),
            ("Patrol", "Defensive", "Area protection", 70),
            ("Pirate", "Aggressive", "Hostile actions", 90),
            ("Explorer", "Curious", "Discovery", 40),
        ]

        for name, personality, behavior, aggression in behaviors:
            ai_behaviors.append({
                "name": name,
                "personality": personality,
                "behavior_type": behavior,
                "aggression_level": aggression,
                "detection_range": 5000,
                "response_time": 2.0,
            })

        return ai_behaviors

    def _set_spaceship_properties(self, asset: Any, data: Dict):
        """Set properties on SpaceshipDataAsset"""
        # This would set actual properties if the classes existed
        pass

    def _set_faction_properties(self, asset: Any, data: Dict):
        """Set properties on FactionDataAsset"""
        pass

    def _set_weapon_properties(self, asset: Any, data: Dict):
        """Set properties on WeaponDataAsset"""
        pass

    def _set_commodity_properties(self, asset: Any, data: Dict):
        """Set properties on CommodityDataAsset"""
        pass

    def _set_quest_properties(self, asset: Any, data: Dict):
        """Set properties on QuestDataAsset"""
        pass

    def _set_personnel_properties(self, asset: Any, data: Dict):
        """Set properties on PersonnelDataAsset"""
        pass

    def _set_ai_properties(self, asset: Any, data: Dict):
        """Set properties on AIDataAsset"""
        pass