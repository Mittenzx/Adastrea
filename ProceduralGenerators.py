#!/usr/bin/env python3
"""
Procedural Generators for Adastrea Game

This module provides procedural generation utilities for:
- Character names
- Star system names
- Planet names
- Other procedural content
"""

import random
from typing import List, Optional


class NameGenerator:
    """Generator for procedural names in the Adastrea universe."""
    
    # First name lists
    FIRST_NAMES_MALE = [
        "Alex", "Ben", "Chris", "David", "Ethan", "Frank", "Greg", "Henry",
        "Ian", "Jack", "Kyle", "Leo", "Mark", "Nathan", "Oscar", "Paul",
        "Quinn", "Ryan", "Steve", "Tom", "Victor", "William", "Xander", "Zane"
    ]
    
    FIRST_NAMES_FEMALE = [
        "Alice", "Beth", "Clara", "Diana", "Eva", "Fiona", "Grace", "Helen",
        "Iris", "Jane", "Kate", "Luna", "Maya", "Nora", "Olivia", "Paula",
        "Quinn", "Rose", "Sara", "Tina", "Uma", "Vera", "Wendy", "Xena", "Zoe"
    ]
    
    # Last name list
    LAST_NAMES = [
        "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller",
        "Davis", "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez",
        "Wilson", "Anderson", "Thomas", "Taylor", "Moore", "Jackson", "Martin",
        "Lee", "Perez", "Thompson", "White", "Harris", "Sanchez", "Clark",
        "Ramirez", "Lewis", "Robinson", "Walker", "Young", "Allen", "King",
        "Wright", "Scott", "Torres", "Nguyen", "Hill", "Flores", "Green",
        "Adams", "Nelson", "Baker", "Hall", "Rivera", "Campbell", "Mitchell",
        "Carter", "Roberts"
    ]
    
    # Star name components
    STAR_PREFIXES = [
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta",
        "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho",
        "Sigma", "Tau", "Upsilon", "Phi", "Chi", "Psi", "Omega"
    ]
    
    STAR_SUFFIXES = [
        "Centauri", "Draconis", "Orionis", "Lyrae", "Cygni", "Aquilae",
        "Bootis", "Cassiopeiae", "Persei", "Tauri", "Geminorum", "Leonis",
        "Virginis", "Librae", "Scorpii", "Sagittarii", "Capricorni",
        "Aquarii", "Piscium", "Arietis"
    ]
    
    # Planet name components
    PLANET_PREFIXES = [
        "New", "Old", "Great", "Little", "Upper", "Lower", "East", "West",
        "North", "South", "Central", "Outer", "Inner", "Far", "Near",
        "Ancient", "Young", "Bright", "Dark", "Cold", "Hot", "Dry", "Wet"
    ]
    
    PLANET_SUFFIXES = [
        "Prime", "Secundus", "Tertius", "Quartus", "Quintus", "Sextus",
        "Septimus", "Octavus", "Nonus", "Decimus", "Major", "Minor",
        "Superior", "Inferior", "Alpha", "Beta", "Gamma", "Delta"
    ]
    
    @classmethod
    def generate_person_name(cls, gender: Optional[str] = None) -> str:
        """
        Generate a procedural person name.
        
        Args:
            gender: Optional gender specification ("Male" or "Female")
            
        Returns:
            A generated name string
        """
        if gender == "Male":
            first_name = random.choice(cls.FIRST_NAMES_MALE)
        elif gender == "Female":
            first_name = random.choice(cls.FIRST_NAMES_FEMALE)
        else:
            # Randomly choose from either list
            first_name = random.choice(cls.FIRST_NAMES_MALE + cls.FIRST_NAMES_FEMALE)
        
        last_name = random.choice(cls.LAST_NAMES)
        return f"{first_name} {last_name}"
    
    @classmethod
    def generate_star_name(cls) -> str:
        """
        Generate a procedural star system name.
        
        Returns:
            A generated star name string
        """
        prefix = random.choice(cls.STAR_PREFIXES)
        suffix = random.choice(cls.STAR_SUFFIXES)
        
        # Sometimes add a Greek letter prefix
        if random.random() < 0.3:
            greek_letters = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon"]
            prefix = f"{random.choice(greek_letters)} {prefix}"
        
        return f"{prefix} {suffix}"
    
    @classmethod
    def generate_planet_name(cls) -> str:
        """
        Generate a procedural planet name.
        
        Returns:
            A generated planet name string
        """
        prefix = random.choice(cls.PLANET_PREFIXES)
        suffix = random.choice(cls.PLANET_SUFFIXES)
        
        # Sometimes add a descriptive element
        if random.random() < 0.4:
            descriptors = ["Desert", "Ocean", "Forest", "Mountain", "Ice", "Volcanic"]
            descriptor = random.choice(descriptors)
            return f"{prefix} {descriptor} {suffix}"
        
        return f"{prefix} {suffix}"
    
    @classmethod
    def generate_ship_name(cls) -> str:
        """
        Generate a procedural ship name.
        
        Returns:
            A generated ship name string
        """
        ship_types = [
            "Voyager", "Explorer", "Pioneer", "Discoverer", "Pathfinder",
            "Navigator", "Trailblazer", "Surveyor", "Scout", "Ranger",
            "Guardian", "Protector", "Defender", "Sentinel", "Watchman",
            "Challenger", "Conqueror", "Victory", "Triumph", "Glory"
        ]
        
        prefixes = ["USS", "HMS", "ISS", "CSS", "RSS", "GCS"]
        
        ship_type = random.choice(ship_types)
        prefix = random.choice(prefixes)
        
        # Add a number sometimes
        if random.random() < 0.5:
            number = random.randint(1, 999)
            return f"{prefix} {ship_type} {number}"
        
        return f"{prefix} {ship_type}"
    
    @classmethod
    def generate_multiple_names(cls, count: int = 10, name_type: str = "person") -> List[str]:
        """
        Generate multiple names of the specified type.
        
        Args:
            count: Number of names to generate
            name_type: Type of names to generate ("person", "star", "planet", "ship")
            
        Returns:
            List of generated name strings
        """
        name_generators = {
            "person": cls.generate_person_name,
            "star": cls.generate_star_name,
            "planet": cls.generate_planet_name,
            "ship": cls.generate_ship_name
        }
        
        if name_type not in name_generators:
            raise ValueError(f"Invalid name_type: {name_type}. Must be one of: {list(name_generators.keys())}")
        
        generator = name_generators[name_type]
        return [generator() for _ in range(count)]


# Example usage
if __name__ == "__main__":
    print("Example name generation:")
    print(f"Person: {NameGenerator.generate_person_name()}")
    print(f"Male: {NameGenerator.generate_person_name('Male')}")
    print(f"Female: {NameGenerator.generate_person_name('Female')}")
    print(f"Star: {NameGenerator.generate_star_name()}")
    print(f"Planet: {NameGenerator.generate_planet_name()}")
    print(f"Ship: {NameGenerator.generate_ship_name()}")
    
    print("\nBulk generation example:")
    print("5 person names:", NameGenerator.generate_multiple_names(5, "person"))
    print("3 star names:", NameGenerator.generate_multiple_names(3, "star"))