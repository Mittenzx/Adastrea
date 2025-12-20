#!/usr/bin/env python3
"""
Mass Content Generator for Adastrea
Generates extensive game content including assets, blueprints, and level population
"""

import unreal
import json
import random
import math
from pathlib import Path
from typing import Dict, List, Any, Optional
import sys
import os

# Add the project root to Python path for imports
sys.path.append(str(Path(__file__).parent))

try:
    from ScenePopulator import ScenePopulator, PlacementPattern
    SCENE_POPULATOR_AVAILABLE = True
except ImportError:
    SCENE_POPULATOR_AVAILABLE = False
    print("Warning: ScenePopulator not available")

class MassContentGenerator:
    """Generates massive amounts of content for the Adastrea game"""

    def __init__(self):
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.generated_content = {
            "data_assets": [],
            "blueprints": [],
            "actors": [],
            "materials": [],
            "particles": []
        }

    def generate_ship_variants(self, base_ship: unreal.USpaceshipDataAsset, count: int = 20) -> List[unreal.USpaceshipDataAsset]:
        """Generate many variants of a base spaceship"""
        variants = []

        for i in range(count):
            variant = self.asset_tools.create_asset(
                asset_name=f"{base_ship.get_name()}_Variant_{i+1}",
                package_path=f"/Game/Adastrea/DataAssets/Ships/Variants/{base_ship.get_name()}_Variant_{i+1}",
                asset_class=unreal.USpaceshipDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Copy and randomize base properties
            variant.display_name = unreal.Text(f"{base_ship.display_name} Mk-{random.randint(1, 10)}")
            variant.core_rating = max(1, min(100, base_ship.core_rating + random.randint(-30, 30)))
            variant.combat_rating = max(1, min(100, base_ship.combat_rating + random.randint(-25, 25)))
            variant.mobility_rating = max(1, min(100, base_ship.mobility_rating + random.randint(-25, 25)))
            variant.utility_rating = max(1, min(100, base_ship.utility_rating + random.randint(-25, 25)))

            # Randomize physical properties
            variant.mass = base_ship.mass * random.uniform(0.7, 1.4)
            variant.max_speed = base_ship.max_speed * random.uniform(0.8, 1.3)
            variant.acceleration = base_ship.acceleration * random.uniform(0.8, 1.3)

            # Randomize crew and cost
            variant.min_crew = max(1, base_ship.min_crew + random.randint(-3, 3))
            variant.max_crew = max(variant.min_crew, base_ship.max_crew + random.randint(-8, 8))
            variant.base_cost = max(50000, base_ship.base_cost + random.randint(-200000, 200000))

            unreal.EditorAssetLibrary.save_asset(variant.get_path_name())
            variants.append(variant)
            self.generated_content["data_assets"].append(variant.get_path_name())

        return variants

    def generate_weapon_series(self, base_weapon: unreal.UWeaponDataAsset, series_name: str, count: int = 15) -> List[unreal.UWeaponDataAsset]:
        """Generate a series of related weapons"""
        weapons = []

        weapon_types = ["Cannon", "Beam", "Launcher", "Torpedo", "Railgun", "Plasma", "Laser", "Particle"]

        for i in range(count):
            weapon_type = random.choice(weapon_types)

            weapon = self.asset_tools.create_asset(
                asset_name=f"{series_name}_{weapon_type}_{i+1}",
                package_path=f"/Game/Adastrea/DataAssets/Weapons/{series_name}/{series_name}_{weapon_type}_{i+1}",
                asset_class=unreal.UWeaponDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Set weapon properties based on type
            if weapon_type == "Cannon":
                weapon.base_damage = random.randint(30, 80)
                weapon.range = random.randint(500, 2000)
                weapon.fire_rate = random.uniform(1.0, 3.0)
            elif weapon_type == "Beam":
                weapon.base_damage = random.randint(40, 100)
                weapon.range = random.randint(800, 2500)
                weapon.fire_rate = random.uniform(0.5, 2.0)
            elif weapon_type == "Launcher":
                weapon.base_damage = random.randint(100, 300)
                weapon.range = random.randint(1000, 4000)
                weapon.fire_rate = random.uniform(0.2, 1.0)

            weapon.damage_type = weapon_type
            weapon.accuracy = random.uniform(0.75, 0.98)
            weapon.energy_cost = random.randint(20, 200)
            weapon.ammo_capacity = random.randint(50, 1000)
            weapon.reload_time = random.uniform(2.0, 15.0)
            weapon.weight = random.randint(500, 5000)
            weapon.base_cost = random.randint(50000, 2000000)

            unreal.EditorAssetLibrary.save_asset(weapon.get_path_name())
            weapons.append(weapon)
            self.generated_content["data_assets"].append(weapon.get_path_name())

        return weapons

    def generate_faction_fleets(self, faction: unreal.UFactionDataAsset, fleet_sizes: Dict[str, int]) -> Dict[str, List[unreal.USpaceshipDataAsset]]:
        """Generate complete fleets for a faction"""
        fleets = {}

        for fleet_type, ship_count in fleet_sizes.items():
            fleet_ships = []

            for i in range(ship_count):
                # Create ship for this fleet
                ship_class = fleet_type.split('_')[0]  # Extract class from fleet type

                ship = self.asset_tools.create_asset(
                    asset_name=f"{faction.faction_name}_{fleet_type}_{i+1}",
                    package_path=f"/Game/Adastrea/DataAssets/Ships/{faction.faction_name}/{fleet_type}_{i+1}",
                    asset_class=unreal.USpaceshipDataAsset,
                    factory=unreal.DataAssetFactory()
                )

                # Set faction-specific properties
                ship.display_name = unreal.Text(f"{faction.faction_name} {ship_class} {i+1}")
                ship.faction_affiliation = faction.faction_name

                # Adjust stats based on fleet type
                if "fighter" in fleet_type.lower():
                    ship.combat_rating = random.randint(70, 95)
                    ship.mobility_rating = random.randint(80, 100)
                    ship.size_category = "Small"
                elif "cruiser" in fleet_type.lower():
                    ship.combat_rating = random.randint(75, 90)
                    ship.utility_rating = random.randint(70, 90)
                    ship.size_category = "Large"
                elif "freighter" in fleet_type.lower():
                    ship.utility_rating = random.randint(85, 100)
                    ship.combat_rating = random.randint(20, 40)
                    ship.size_category = "Large"

                unreal.EditorAssetLibrary.save_asset(ship.get_path_name())
                fleet_ships.append(ship)
                self.generated_content["data_assets"].append(ship.get_path_name())

            fleets[fleet_type] = fleet_ships

        return fleets

    def generate_station_blueprints(self, station_types: List[str], count_per_type: int = 5) -> List[str]:
        """Generate station blueprints for different types"""
        blueprints = []

        for station_type in station_types:
            for i in range(count_per_type):
                blueprint_name = f"BP_Station_{station_type}_{i+1}"
                blueprint_path = f"/Game/Adastrea/Blueprints/Stations/{station_type}/{blueprint_name}"

                # Create blueprint based on station type
                if station_type == "Trading":
                    parent_class = unreal.AActor  # Would be actual station class
                elif station_type == "Military":
                    parent_class = unreal.AActor
                else:
                    parent_class = unreal.AActor

                blueprint = self.asset_tools.create_asset(
                    asset_name=blueprint_name,
                    package_path=blueprint_path,
                    asset_class=unreal.Blueprint,
                    factory=unreal.BlueprintFactory()
                )

                blueprints.append(blueprint_path)
                self.generated_content["blueprints"].append(blueprint_path)

        return blueprints

    def generate_trading_commodities(self, count: int = 50) -> List[Dict[str, Any]]:
        """Generate trading commodities with economic properties"""
        commodities = []

        categories = ["Metals", "Minerals", "Technology", "Food", "Medicine", "Luxury", "Fuel", "Weapons", "Ship Parts"]
        rarities = ["Common", "Uncommon", "Rare", "Very Rare", "Legendary"]

        for i in range(count):
            category = random.choice(categories)
            rarity = random.choice(rarities)

            commodity = {
                "name": f"{category}_{random.randint(100, 999)}",
                "category": category,
                "rarity": rarity,
                "base_price": self._calculate_base_price(category, rarity),
                "volatility": self._calculate_volatility(rarity),
                "legality": random.choice(["Legal", "Restricted", "Illegal", "Contraband"]),
                "description": f"A {rarity.lower()} {category.lower()} commodity"
            }

            commodities.append(commodity)

        # Save as data assets
        for commodity in commodities:
            asset = self.asset_tools.create_asset(
                asset_name=f"DA_Commodity_{commodity['name']}",
                package_path=f"/Game/Adastrea/DataAssets/Trading/Commodities/DA_Commodity_{commodity['name']}",
                asset_class=unreal.UDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Set properties (would need actual UCommodityDataAsset class)
            asset.commodity_name = unreal.Text(commodity["name"])
            asset.category = commodity["category"]
            asset.base_price = commodity["base_price"]

            unreal.EditorAssetLibrary.save_asset(asset.get_path_name())
            self.generated_content["data_assets"].append(asset.get_path_name())

        return commodities

    def _calculate_base_price(self, category: str, rarity: str) -> int:
        """Calculate base price based on category and rarity"""
        base_prices = {
            "Metals": 100,
            "Minerals": 200,
            "Technology": 1000,
            "Food": 50,
            "Medicine": 500,
            "Luxury": 2000,
            "Fuel": 150,
            "Weapons": 1500,
            "Ship Parts": 800
        }

        rarity_multipliers = {
            "Common": 1.0,
            "Uncommon": 2.5,
            "Rare": 6.0,
            "Very Rare": 15.0,
            "Legendary": 40.0
        }

        base = base_prices.get(category, 100)
        multiplier = rarity_multipliers.get(rarity, 1.0)

        return int(base * multiplier * random.uniform(0.8, 1.2))

    def _calculate_volatility(self, rarity: str) -> float:
        """Calculate price volatility based on rarity"""
        volatility_map = {
            "Common": 0.1,
            "Uncommon": 0.2,
            "Rare": 0.4,
            "Very Rare": 0.7,
            "Legendary": 1.0
        }
        return volatility_map.get(rarity, 0.1) * random.uniform(0.8, 1.2)

    def generate_quest_database(self, count: int = 100) -> List[Dict[str, Any]]:
        """Generate a comprehensive quest database"""
        quests = []

        quest_templates = [
            {"type": "Delivery", "difficulty_range": (1, 5), "reward_range": (1000, 50000)},
            {"type": "Combat", "difficulty_range": (2, 8), "reward_range": (5000, 100000)},
            {"type": "Exploration", "difficulty_range": (1, 6), "reward_range": (2000, 75000)},
            {"type": "Trading", "difficulty_range": (1, 4), "reward_range": (3000, 60000)},
            {"type": "Espionage", "difficulty_range": (3, 9), "reward_range": (10000, 200000)},
            {"type": "Mining", "difficulty_range": (2, 7), "reward_range": (4000, 80000)}
        ]

        for i in range(count):
            template = random.choice(quest_templates)

            quest = {
                "id": f"quest_{i+1:04d}",
                "title": self._generate_quest_title(template["type"]),
                "type": template["type"],
                "difficulty": random.randint(template["difficulty_range"][0], template["difficulty_range"][1]),
                "description": self._generate_quest_description(template["type"]),
                "objectives": self._generate_quest_objectives(template["type"]),
                "rewards": {
                    "credits": random.randint(template["reward_range"][0], template["reward_range"][1]),
                    "reputation": random.randint(5, 50),
                    "items": random.sample(["Weapon Upgrade", "Ship Module", "Rare Commodity", "Faction Favor", "Experience Boost"], random.randint(0, 3))
                },
                "requirements": {
                    "min_level": max(1, template["difficulty_range"][0] - 1),
                    "skills": random.sample(["Piloting", "Combat", "Trading", "Diplomacy", "Engineering", "Navigation"], random.randint(0, 3)),
                    "faction_standing": random.choice(["Neutral", "Friendly", "Allied"])
                },
                "time_limit": random.randint(1, 30),  # days
                "failure_consequences": random.choice(["Reputation Loss", "Faction Hostility", "Financial Penalty", "Mission Lockout"])
            }

            quests.append(quest)

        # Save quest data
        quest_data_asset = self.asset_tools.create_asset(
            asset_name="DA_QuestDatabase",
            package_path="/Game/Adastrea/DataAssets/Quests/DA_QuestDatabase",
            asset_class=unreal.UDataAsset,
            factory=unreal.DataAssetFactory()
        )

        # Would set quest data here
        unreal.EditorAssetLibrary.save_asset(quest_data_asset.get_path_name())
        self.generated_content["data_assets"].append(quest_data_asset.get_path_name())

        return quests

    def _generate_quest_title(self, quest_type: str) -> str:
        """Generate a quest title based on type"""
        title_templates = {
            "Delivery": ["The Package", "Urgent Delivery", "Special Cargo", "Time-Sensitive Shipment"],
            "Combat": ["Eliminate Threat", "Defend Territory", "Pirate Hunt", "Hostile Forces"],
            "Exploration": ["Unknown Anomaly", "Chart New Territory", "Ancient Ruins", "Mysterious Signal"],
            "Trading": ["Market Opportunity", "Bulk Cargo", "Rare Goods", "Trade Negotiations"],
            "Espionage": ["Gather Intelligence", "Infiltrate Base", "Steal Data", "Sabotage Mission"],
            "Mining": ["Rich Deposit", "Mineral Survey", "Resource Extraction", "Mining Operation"]
        }

        return random.choice(title_templates.get(quest_type, ["Generic Quest"]))

    def _generate_quest_description(self, quest_type: str) -> str:
        """Generate quest description"""
        return f"A {quest_type.lower()} mission requiring careful planning and execution."

    def _generate_quest_objectives(self, quest_type: str) -> List[str]:
        """Generate quest objectives"""
        objectives_map = {
            "Delivery": ["Pick up cargo", "Navigate to destination", "Deliver safely", "Return for payment"],
            "Combat": ["Locate targets", "Engage hostiles", "Eliminate threats", "Return for debriefing"],
            "Exploration": ["Travel to location", "Scan area", "Gather data", "Return with findings"],
            "Trading": ["Acquire goods", "Transport cargo", "Negotiate sale", "Complete transaction"],
            "Espionage": ["Infiltrate facility", "Gather intelligence", "Extract data", "Escape undetected"],
            "Mining": ["Travel to site", "Deploy equipment", "Extract resources", "Return with cargo"]
        }

        base_objectives = objectives_map.get(quest_type, ["Complete objective"])
        return random.sample(base_objectives, random.randint(2, len(base_objectives)))

    def generate_ai_behaviors(self, count: int = 30) -> List[Dict[str, Any]]:
        """Generate AI behavior profiles"""
        behaviors = []

        personality_traits = ["Aggressive", "Cautious", "Diplomatic", "Ruthless", "Honorable", "Deceptive", "Brave", "Intelligent"]
        combat_styles = ["Hit-and-Run", "Siege", "Defensive", "Overwhelm", "Stealth"]
        trading_styles = ["Fair", "Exploitative", "Generous", "Bartering", "Hostile"]

        for i in range(count):
            behavior = {
                "name": f"AI_Profile_{i+1}",
                "personality": random.sample(personality_traits, random.randint(2, 5)),
                "combat_style": random.choice(combat_styles),
                "trading_style": random.choice(trading_styles),
                "risk_tolerance": random.uniform(0.1, 1.0),
                "aggression_level": random.uniform(0.0, 1.0),
                "caution_level": random.uniform(0.0, 1.0),
                "decision_weights": {
                    "profit": random.uniform(0.1, 1.0),
                    "risk": random.uniform(0.1, 1.0),
                    "ethics": random.uniform(0.1, 1.0),
                    "loyalty": random.uniform(0.1, 1.0)
                },
                "behavior_patterns": {
                    "patrol_frequency": random.uniform(0.1, 1.0),
                    "engagement_distance": random.uniform(1000, 10000),
                    "retreat_threshold": random.uniform(0.1, 0.8),
                    "ally_support": random.uniform(0.0, 1.0)
                }
            }

            behaviors.append(behavior)

        return behaviors

    def populate_massive_scene(self) -> Dict[str, Any]:
        """Populate a scene with massive amounts of content"""
        if not SCENE_POPULATOR_AVAILABLE:
            print("ScenePopulator not available for scene population")
            return {}

        scene_content = {
            "ships": [],
            "stations": [],
            "asteroids": [],
            "effects": []
        }

        # Create multiple sectors
        sectors = [
            {"center": (0, 0, 0), "radius": 10000, "ships": 50, "stations": 8},
            {"center": (25000, 0, 0), "radius": 8000, "ships": 30, "stations": 5},
            {"center": (-25000, 0, 0), "radius": 8000, "ships": 30, "stations": 5},
            {"center": (0, 25000, 0), "radius": 8000, "ships": 30, "stations": 5},
            {"center": (0, -25000, 0), "radius": 8000, "ships": 30, "stations": 5}
        ]

        for sector in sectors:
            # Populate ships
            ship_positions = PlacementPattern.orbit(sector["ships"], sector["radius"] * 0.8, sector["center"])
            for pos in ship_positions[:10]:  # Limit for performance
                ship = ScenePopulator().spawn_actor("/Game/Adastrea/Blueprints/Ships/Fighter", pos, (0, 0, 0))
                if ship:
                    scene_content["ships"].append(ship)

            # Populate stations
            station_positions = PlacementPattern.cluster(sector["stations"], sector["stations"], 2000, sector["radius"] * 0.6, sector["center"])
            for pos in station_positions:
                station = ScenePopulator().spawn_actor("/Game/Adastrea/Blueprints/Stations/TradingHub", pos, (0, 0, 0))
                if station:
                    scene_content["stations"].append(station)

        # Create massive asteroid fields
        asteroid_fields = [
            ((15000, 15000, 0), 5000, 8000, 200),
            ((-15000, -15000, 0), 6000, 9000, 180),
            ((0, 0, 0), 12000, 15000, 300)
        ]

        for center, inner, outer, count in asteroid_fields:
            from ScenePopulator import create_asteroid_belt
            asteroids = create_asteroid_belt(center, inner, outer, count)
            scene_content["asteroids"].extend(asteroids)

        # Create environmental effects
        from ScenePopulator import create_environment_effects
        nebulae = create_environment_effects((10000, 10000, 0), "nebula", 10000, 30)
        scene_content["effects"].extend(nebulae)

        total_actors = sum(len(actors) for actors in scene_content.values())
        print(f"Mass scene population complete: {total_actors} actors created")

        return scene_content

    def generate_everything(self) -> Dict[str, Any]:
        """Generate ALL content types"""
        print("Starting massive content generation...")

        # Generate data assets
        print("Generating spaceship variants...")
        # Would need actual base ships to generate variants from

        print("Generating weapon series...")
        # Would need base weapons

        print("Generating faction fleets...")
        # Would need faction data

        print("Generating trading commodities...")
        commodities = self.generate_trading_commodities(50)

        print("Generating quest database...")
        quests = self.generate_quest_database(100)

        print("Generating AI behaviors...")
        ai_behaviors = self.generate_ai_behaviors(30)

        print("Populating massive scene...")
        scene_content = self.populate_massive_scene()

        result = {
            "commodities_generated": len(commodities),
            "quests_generated": len(quests),
            "ai_behaviors_generated": len(ai_behaviors),
            "scene_actors_created": sum(len(actors) for actors in scene_content.values()),
            "total_assets_created": len(self.generated_content["data_assets"]) + len(self.generated_content["blueprints"])
        }

        print("Mass content generation complete!")
        print(f"Results: {json.dumps(result, indent=2)}")

        return result

# Blueprint function library for Unreal Editor
@unreal.uclass()
class MassContentGeneratorLibrary(unreal.BlueprintFunctionLibrary):
    """Blueprint-accessible mass content generation functions"""

    @unreal.ufunction(static=True, meta=dict(Category="Mass Content Generation"))
    def generate_all_content():
        """Generate massive amounts of all content types"""
        generator = MassContentGenerator()
        return generator.generate_everything()

    @unreal.ufunction(static=True, meta=dict(Category="Mass Content Generation"))
    def generate_commodities_only(count=50):
        """Generate only trading commodities"""
        generator = MassContentGenerator()
        return generator.generate_trading_commodities(count)

    @unreal.ufunction(static=True, meta=dict(Category="Mass Content Generation"))
    def generate_quests_only(count=100):
        """Generate only quest content"""
        generator = MassContentGenerator()
        return generator.generate_quest_database(count)

    @unreal.ufunction(static=True, meta=dict(Category="Mass Content Generation"))
    def populate_massive_scene():
        """Populate the current scene with massive content"""
        generator = MassContentGenerator()
        return generator.populate_massive_scene()

if __name__ == "__main__":
    # Command line usage
    import argparse

    parser = argparse.ArgumentParser(description="Mass Content Generator for Adastrea")
    parser.add_argument("--generate-all", action="store_true", help="Generate all content types")
    parser.add_argument("--commodities", type=int, help="Generate trading commodities")
    parser.add_argument("--quests", type=int, help="Generate quest database")
    parser.add_argument("--populate-scene", action="store_true", help="Populate scene with content")

    args = parser.parse_args()

    generator = MassContentGenerator()

    if args.generate_all:
        result = generator.generate_everything()
        print(f"Generation complete: {result}")
    elif args.commodities:
        commodities = generator.generate_trading_commodities(args.commodities)
        print(f"Generated {len(commodities)} commodities")
    elif args.quests:
        quests = generator.generate_quest_database(args.quests)
        print(f"Generated {len(quests)} quests")
    elif args.populate_scene:
        scene_content = generator.populate_massive_scene()
        total_actors = sum(len(actors) for actors in scene_content.values())
        print(f"Populated scene with {total_actors} actors")
    else:
        parser.print_help()