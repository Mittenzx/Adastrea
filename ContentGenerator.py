#!/usr/bin/env python3
"""
Adastrea Content Generator
Generates comprehensive game content for the space flight game
"""

import unreal
import json
import random
from typing import Dict, List, Any
from pathlib import Path

@unreal.uclass()
class ContentGenerator(unreal.BlueprintFunctionLibrary):
    """Blueprint function library for content generation"""

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_spaceship_variants(base_ship_asset, num_variants=10):
        """Generate multiple variants of a spaceship with randomized stats"""
        variants = []

        for i in range(num_variants):
            variant = unreal.new_object(unreal.USpaceshipDataAsset, unreal.CoreUObject.get_transient_package())

            # Copy base properties
            variant.display_name = f"{base_ship_asset.display_name} Variant {i+1}"
            variant.description = f"Auto-generated variant of {base_ship_asset.display_name}"

            # Randomize stats with some constraints
            variant.core_rating = max(1, min(100, base_ship_asset.core_rating + random.randint(-20, 20)))
            variant.combat_rating = max(1, min(100, base_ship_asset.combat_rating + random.randint(-15, 15)))
            variant.mobility_rating = max(1, min(100, base_ship_asset.mobility_rating + random.randint(-15, 15)))
            variant.utility_rating = max(1, min(100, base_ship_asset.utility_rating + random.randint(-15, 15)))
            variant.operational_rating = max(1, min(100, base_ship_asset.operational_rating + random.randint(-15, 15)))
            variant.advanced_rating = max(1, min(100, base_ship_asset.advanced_rating + random.randint(-15, 15)))
            variant.lore_rating = max(1, min(100, base_ship_asset.lore_rating + random.randint(-10, 10)))

            # Randomize size and role
            sizes = ["Small", "Medium", "Large", "Capital"]
            roles = ["Combat", "Trade", "Exploration", "Mining", "Patrol", "Transport"]
            variant.size_category = random.choice(sizes)
            variant.primary_role = random.choice(roles)

            # Randomize crew and cost
            variant.min_crew = max(1, base_ship_asset.min_crew + random.randint(-2, 2))
            variant.max_crew = max(variant.min_crew, base_ship_asset.max_crew + random.randint(-5, 5))
            variant.base_cost = max(1000, base_ship_asset.base_cost + random.randint(-50000, 50000))

            variants.append(variant)

        return variants

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_faction_content(faction_asset, num_stations=5, num_ships=10):
        """Generate stations and ships for a faction"""
        content = {
            "stations": [],
            "ships": []
        }

        # Generate stations
        station_types = ["Trading Hub", "Military Base", "Mining Outpost", "Research Station", "Shipyard"]
        for i in range(num_stations):
            station = {
                "name": f"{faction_asset.faction_name} {random.choice(station_types)} {i+1}",
                "type": random.choice(station_types),
                "size": random.choice(["Small", "Medium", "Large"]),
                "security_level": random.randint(1, 10),
                "economic_value": random.randint(1000, 100000),
                "faction_affiliation": faction_asset.faction_name
            }
            content["stations"].append(station)

        # Generate faction-specific ships
        ship_classes = ["Fighter", "Frigate", "Cruiser", "Destroyer", "Freighter", "Explorer"]
        for i in range(num_ships):
            ship = {
                "name": f"{faction_asset.faction_name} {random.choice(ship_classes)} {i+1}",
                "class": random.choice(ship_classes),
                "combat_rating": random.randint(10, 100),
                "faction_loyalty": random.randint(50, 100),
                "special_abilities": random.sample(["Stealth", "Heavy Armor", "Fast Engines", "Advanced Sensors", "Heavy Weapons"], random.randint(0, 3))
            }
            content["ships"].append(ship)

        return content

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_trading_economy(num_commodities=20, num_routes=15):
        """Generate a complete trading economy"""
        economy = {
            "commodities": [],
            "trade_routes": []
        }

        # Generate commodities
        commodity_types = ["Metals", "Minerals", "Technology", "Food", "Medicine", "Luxury Goods", "Fuel", "Weapons", "Ship Parts", "Rare Elements"]
        for i in range(num_commodities):
            commodity = {
                "name": f"{random.choice(commodity_types)} {random.randint(100, 999)}",
                "type": random.choice(commodity_types),
                "base_price": random.randint(10, 10000),
                "volatility": random.uniform(0.1, 2.0),
                "rarity": random.choice(["Common", "Uncommon", "Rare", "Very Rare", "Legendary"]),
                "legal_status": random.choice(["Legal", "Restricted", "Illegal"])
            }
            economy["commodities"].append(commodity)

        # Generate trade routes
        station_names = [f"Station {i+1}" for i in range(20)]
        for i in range(num_routes):
            route = {
                "start_station": random.choice(station_names),
                "end_station": random.choice([s for s in station_names if s != route["start_station"]]),
                "distance": random.randint(100, 10000),
                "danger_level": random.randint(1, 10),
                "profit_margin": random.uniform(0.1, 3.0),
                "commodities": random.sample([c["name"] for c in economy["commodities"]], random.randint(1, 5))
            }
            economy["trade_routes"].append(route)

        return economy

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_quest_content(num_quests=25):
        """Generate quest content"""
        quests = []

        quest_types = ["Delivery", "Escort", "Combat", "Exploration", "Mining", "Diplomacy", "Smuggling"]
        difficulties = ["Easy", "Medium", "Hard", "Very Hard", "Extreme"]

        for i in range(num_quests):
            quest = {
                "title": f"Quest {i+1}: {random.choice(['The Lost', 'Shadow', 'Final', 'Ancient', 'Forgotten'])} {random.choice(['Artifact', 'Mission', 'Contract', 'Operation', 'Assignment'])}",
                "type": random.choice(quest_types),
                "difficulty": random.choice(difficulties),
                "description": f"A {quest['difficulty'].lower()} {quest['type'].lower()} mission that requires careful planning and execution.",
                "objectives": [
                    f"Complete the {quest['type'].lower()} objective",
                    f"Return to quest giver within {random.randint(1, 30)} days",
                    f"Achieve success rate of at least {random.randint(70, 100)}%"
                ],
                "rewards": {
                    "credits": random.randint(1000, 100000),
                    "reputation": random.randint(1, 50),
                    "items": random.sample(["Weapon Upgrade", "Ship Module", "Rare Commodity", "Faction Favor"], random.randint(0, 3))
                },
                "requirements": {
                    "min_level": random.randint(1, 20),
                    "required_skills": random.sample(["Piloting", "Combat", "Trading", "Diplomacy", "Engineering"], random.randint(0, 3)),
                    "faction_standing": random.choice(["Neutral", "Friendly", "Allied"])
                }
            }
            quests.append(quest)

        return quests

    @unreal.ufunction(static=True, meta=dict(Category="Content Generation"))
    def generate_ai_personalities(num_personalities=15):
        """Generate AI personality profiles for NPCs"""
        personalities = []

        traits = ["Aggressive", "Cautious", "Diplomatic", "Ruthless", "Honorable", "Deceptive", "Brave", "Cowardly", "Intelligent", "Reckless"]
        dispositions = ["Hostile", "Neutral", "Friendly", "Allied", "Enemy"]

        for i in range(num_personalities):
            personality = {
                "name": f"Personality {i+1}",
                "traits": random.sample(traits, random.randint(2, 5)),
                "base_disposition": random.choice(dispositions),
                "aggression_level": random.uniform(0.0, 1.0),
                "caution_level": random.uniform(0.0, 1.0),
                "intelligence_level": random.uniform(0.0, 1.0),
                "behavior_patterns": {
                    "combat_style": random.choice(["Aggressive", "Defensive", "Hit-and-Run", "Siege"]),
                    "trading_style": random.choice(["Fair", "Exploitative", "Generous", "Bartering"]),
                    "social_style": random.choice(["Friendly", "Distant", "Manipulative", "Honest"])
                },
                "decision_weights": {
                    "profit": random.uniform(0.1, 1.0),
                    "risk": random.uniform(0.1, 1.0),
                    "ethics": random.uniform(0.1, 1.0),
                    "loyalty": random.uniform(0.1, 1.0)
                }
            }
            personalities.append(personality)

        return personalities

def generate_all_content():
    """Generate comprehensive game content"""
    print("Generating Adastrea game content...")

    # This would be called from within Unreal Engine
    # For now, we'll create the structure that can be used

    content_structure = {
        "spaceships": {
            "fighters": 50,
            "freighters": 30,
            "explorers": 20,
            "capital_ships": 10
        },
        "factions": {
            "major_factions": 5,
            "minor_factions": 15
        },
        "stations": {
            "trading_hubs": 20,
            "military_bases": 15,
            "mining_outposts": 25,
            "research_stations": 10
        },
        "commodities": 100,
        "trade_routes": 50,
        "quests": 100,
        "ai_personalities": 30
    }

    print(f"Content generation plan: {json.dumps(content_structure, indent=2)}")
    return content_structure

if __name__ == "__main__":
    generate_all_content()