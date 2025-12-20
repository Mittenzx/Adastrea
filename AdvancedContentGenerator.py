#!/usr/bin/env python3
"""
Advanced Content Generator for Adastrea
Creates comprehensive game content including Data Assets, Blueprints, and procedural content
"""

import unreal
import json
import random
import math
from pathlib import Path
from typing import Dict, List, Any, Optional

class AdastreaContentGenerator:
    """Advanced content generator for the Adastrea space game"""

    def __init__(self):
        self.content_root = "/Game/Adastrea"
        self.generated_assets = []

    def create_spaceship_data_assets(self, count: int = 50) -> List[unreal.USpaceshipDataAsset]:
        """Create spaceship data assets with varied stats"""
        spaceships = []

        ship_templates = {
            "Fighter": {"combat": 80, "mobility": 90, "utility": 30, "size": "Small"},
            "Frigate": {"combat": 70, "mobility": 60, "utility": 50, "size": "Medium"},
            "Cruiser": {"combat": 85, "mobility": 40, "utility": 70, "size": "Large"},
            "Destroyer": {"combat": 90, "mobility": 30, "utility": 60, "size": "Large"},
            "Freighter": {"combat": 20, "mobility": 40, "utility": 90, "size": "Large"},
            "Explorer": {"combat": 40, "mobility": 70, "utility": 80, "size": "Medium"},
            "Corvette": {"combat": 60, "mobility": 80, "utility": 40, "size": "Small"},
            "Capital": {"combat": 95, "mobility": 20, "utility": 85, "size": "Capital"}
        }

        factions = ["Solaris Union", "Nebula Corporation", "Free Traders", "Imperial Navy", "Pirates"]

        for i in range(count):
            # Select random template
            template_name = random.choice(list(ship_templates.keys()))
            template = ship_templates[template_name]

            # Create asset
            asset_name = f"DA_Ship_{template_name}_{i+1}"
            asset_path = f"{self.content_root}/DataAssets/Ships/{asset_name}"

            spaceship = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name=asset_name,
                package_path=asset_path,
                asset_class=unreal.USpaceshipDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Set basic properties
            spaceship.display_name = unreal.Text(f"{random.choice(factions)} {template_name} Mk-{random.randint(1, 10)}")
            spaceship.description = unreal.Text(f"A {template_name.lower()} class spaceship with balanced capabilities.")

            # Set ratings with some randomization
            spaceship.core_rating = random.randint(40, 100)
            spaceship.combat_rating = max(1, min(100, template["combat"] + random.randint(-20, 20)))
            spaceship.mobility_rating = max(1, min(100, template["mobility"] + random.randint(-20, 20)))
            spaceship.utility_rating = max(1, min(100, template["utility"] + random.randint(-20, 20)))
            spaceship.operational_rating = random.randint(30, 100)
            spaceship.advanced_rating = random.randint(20, 100)
            spaceship.lore_rating = random.randint(10, 100)

            # Set physical properties
            spaceship.size_category = template["size"]
            spaceship.mass = random.randint(100, 10000)
            spaceship.max_speed = random.randint(50, 500)
            spaceship.acceleration = random.uniform(5.0, 50.0)

            # Set operational properties
            spaceship.min_crew = random.randint(1, 20)
            spaceship.max_crew = random.randint(spaceship.min_crew, spaceship.min_crew + 50)
            spaceship.power_capacity = random.randint(100, 10000)
            spaceship.shield_capacity = random.randint(100, 10000)
            spaceship.hull_integrity = random.randint(100, 10000)

            # Set economic properties
            spaceship.base_cost = random.randint(10000, 1000000)
            spaceship.maintenance_cost = random.randint(100, 10000)

            # Set capabilities
            spaceship.primary_role = random.choice(["Combat", "Trade", "Exploration", "Mining", "Patrol"])
            spaceship.secondary_roles = random.sample(["Combat", "Trade", "Exploration", "Mining", "Patrol", "Transport"], random.randint(0, 3))

            # Save asset
            unreal.EditorAssetLibrary.save_asset(asset_path)
            spaceships.append(spaceship)
            self.generated_assets.append(asset_path)

        return spaceships

    def create_faction_data_assets(self, count: int = 8) -> List[unreal.UFactionDataAsset]:
        """Create faction data assets with relationships and traits"""
        factions = []

        faction_templates = [
            {"name": "Solaris Union", "type": "Military", "ethos": "Order"},
            {"name": "Nebula Corporation", "type": "Corporate", "ethos": "Profit"},
            {"name": "Free Traders", "type": "Merchant", "ethos": "Freedom"},
            {"name": "Imperial Navy", "type": "Military", "ethos": "Dominance"},
            {"name": "Pirates", "type": "Criminal", "ethos": "Chaos"},
            {"name": "Research Alliance", "type": "Scientific", "ethos": "Knowledge"},
            {"name": "Colonial Republic", "type": "Democratic", "ethos": "Liberty"},
            {"name": "Void Nomads", "type": "Nomadic", "ethos": "Survival"}
        ]

        for i in range(min(count, len(faction_templates))):
            template = faction_templates[i]

            asset_name = f"DA_Faction_{template['name'].replace(' ', '')}"
            asset_path = f"{self.content_root}/DataAssets/Factions/{asset_name}"

            faction = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name=asset_name,
                package_path=asset_path,
                asset_class=unreal.UFactionDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Set basic properties
            faction.faction_name = unreal.Text(template["name"])
            faction.description = unreal.Text(f"The {template['name']}, a {template['type'].lower()} faction driven by {template['ethos'].lower()}.")
            faction.faction_type = template["type"]
            faction.ethos = template["ethos"]

            # Set economic properties
            faction.economic_power = random.randint(10, 100)
            faction.technological_level = random.randint(10, 100)
            faction.military_strength = random.randint(10, 100)

            # Set traits
            faction.traits = [
                unreal.Text("Diplomatic"),
                unreal.Text("Aggressive"),
                unreal.Text("Technological"),
                unreal.Text("Economic")
            ]

            # Set colors
            faction.primary_color = unreal.LinearColor(random.random(), random.random(), random.random(), 1.0)
            faction.secondary_color = unreal.LinearColor(random.random(), random.random(), random.random(), 1.0)

            # Save asset
            unreal.EditorAssetLibrary.save_asset(asset_path)
            factions.append(faction)
            self.generated_assets.append(asset_path)

        # Create faction relationships
        self.create_faction_relationships(factions)

        return factions

    def create_faction_relationships(self, factions: List[unreal.UFactionDataAsset]):
        """Create relationships between factions"""
        for faction_a in factions:
            for faction_b in factions:
                if faction_a != faction_b:
                    # Create some logic for relationships
                    relationship = random.randint(-100, 100)

                    # Some factions are natural allies/enemies
                    if faction_a.faction_type == faction_b.faction_type:
                        relationship += random.randint(10, 30)  # Same type = more likely allies
                    elif faction_a.faction_type in ["Military", "Corporate"] and faction_b.faction_type == "Pirates":
                        relationship -= random.randint(20, 50)  # Military/Corporate hate pirates
                    elif faction_a.faction_type == "Merchant" and faction_b.faction_type == "Corporate":
                        relationship += random.randint(10, 30)  # Merchants like corporations

                    # Store relationship (this would need to be implemented in the faction system)
                    # faction_a.relationships[faction_b] = relationship

    def create_weapon_data_assets(self, count: int = 30) -> List[unreal.UWeaponDataAsset]:
        """Create weapon data assets"""
        weapons = []

        weapon_templates = {
            "Laser Cannon": {"damage": 50, "range": 1000, "type": "Energy"},
            "Plasma Gun": {"damage": 80, "range": 800, "type": "Plasma"},
            "Railgun": {"damage": 120, "range": 1500, "type": "Kinetic"},
            "Missile Launcher": {"damage": 200, "range": 2000, "type": "Missile"},
            "Particle Beam": {"damage": 150, "range": 1200, "type": "Energy"},
            "Torpedo Tube": {"damage": 300, "range": 2500, "type": "Missile"}
        }

        for i in range(count):
            template_name = random.choice(list(weapon_templates.keys()))
            template = weapon_templates[template_name]

            asset_name = f"DA_Weapon_{template_name.replace(' ', '')}_{i+1}"
            asset_path = f"{self.content_root}/DataAssets/Weapons/{asset_name}"

            weapon = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name=asset_name,
                package_path=asset_path,
                asset_class=unreal.UWeaponDataAsset,
                factory=unreal.DataAssetFactory()
            )

            # Set basic properties
            weapon.weapon_name = unreal.Text(f"{template_name} Mk-{random.randint(1, 5)}")
            weapon.description = unreal.Text(f"A {template['type'].lower()} weapon with {template['damage']} base damage.")

            # Set combat properties
            weapon.base_damage = max(1, template["damage"] + random.randint(-20, 20))
            weapon.damage_type = template["type"]
            weapon.range = max(100, template["range"] + random.randint(-200, 200))
            weapon.fire_rate = random.uniform(0.5, 5.0)
            weapon.accuracy = random.uniform(0.7, 1.0)

            # Set resource properties
            weapon.energy_cost = random.randint(10, 100)
            weapon.ammo_capacity = random.randint(10, 1000)
            weapon.reload_time = random.uniform(1.0, 10.0)

            # Set physical properties
            weapon.weight = random.randint(100, 1000)
            weapon.size_category = random.choice(["Small", "Medium", "Large"])

            # Set economic properties
            weapon.base_cost = random.randint(1000, 100000)

            # Save asset
            unreal.EditorAssetLibrary.save_asset(asset_path)
            weapons.append(weapon)
            self.generated_assets.append(asset_path)

        return weapons

    def create_station_blueprints(self, count: int = 20):
        """Create station blueprints"""
        stations = []

        station_types = ["Trading Hub", "Military Base", "Mining Outpost", "Research Station", "Shipyard", "Refueling Depot"]

        for i in range(count):
            station_type = random.choice(station_types)
            blueprint_name = f"BP_Station_{station_type.replace(' ', '')}_{i+1}"
            blueprint_path = f"{self.content_root}/Blueprints/Stations/{blueprint_name}"

            # Create blueprint from SpaceStation base class
            blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name=blueprint_name,
                package_path=blueprint_path,
                asset_class=unreal.Blueprint,
                factory=unreal.BlueprintFactory()
            )

            # Set parent class (assuming ASpaceStation exists)
            # This would need to be implemented based on the actual class hierarchy

            stations.append(blueprint)
            self.generated_assets.append(blueprint_path)

        return stations

    def create_procedural_content(self):
        """Create procedural content like asteroid fields, nebulae, etc."""
        # This would create procedural actors and environments
        # For now, just log the intent
        unreal.log("Creating procedural content: asteroid fields, nebulae, jump gates...")

    def generate_all_content(self):
        """Generate all content types"""
        unreal.log("Starting comprehensive content generation for Adastrea...")

        # Create directories
        self.ensure_directories()

        # Generate content
        spaceships = self.create_spaceship_data_assets(50)
        unreal.log(f"Created {len(spaceships)} spaceship data assets")

        factions = self.create_faction_data_assets(8)
        unreal.log(f"Created {len(factions)} faction data assets")

        weapons = self.create_weapon_data_assets(30)
        unreal.log(f"Created {len(weapons)} weapon data assets")

        stations = self.create_station_blueprints(20)
        unreal.log(f"Created {len(stations)} station blueprints")

        self.create_procedural_content()

        unreal.log(f"Content generation complete! Created {len(self.generated_assets)} assets total")
        return self.generated_assets

    def ensure_directories(self):
        """Ensure all necessary directories exist"""
        directories = [
            f"{self.content_root}/DataAssets/Ships",
            f"{self.content_root}/DataAssets/Factions",
            f"{self.content_root}/DataAssets/Weapons",
            f"{self.content_root}/Blueprints/Stations",
            f"{self.content_root}/Blueprints/Procedural"
        ]

        for directory in directories:
            unreal.EditorAssetLibrary.make_directory(directory)

# Global generator instance
content_generator = AdastreaContentGenerator()

@unreal.uclass()
class ContentGeneratorLibrary(unreal.BlueprintFunctionLibrary):
    """Blueprint-accessible content generation functions"""

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_all_game_content():
        """Generate comprehensive game content"""
        return content_generator.generate_all_content()

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_spaceships_only(count=50):
        """Generate only spaceship content"""
        return content_generator.create_spaceship_data_assets(count)

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_factions_only(count=8):
        """Generate only faction content"""
        return content_generator.create_faction_data_assets(count)

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_weapons_only(count=30):
        """Generate only weapon content"""
        return content_generator.create_weapon_data_assets(count)

if __name__ == "__main__":
    # This script can be run from Unreal Engine's Python environment
    generator = AdastreaContentGenerator()
    assets = generator.generate_all_content()
    print(f"Generated {len(assets)} assets:")
    for asset in assets[:10]:  # Show first 10
        print(f"  - {asset}")
    if len(assets) > 10:
        print(f"  ... and {len(assets) - 10} more")