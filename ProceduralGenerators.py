#!/usr/bin/env python3
"""
Adastrea - Procedural Content Generation Script

This script generates randomized game content for various systems using
procedural generation techniques. Creates Data Asset-compatible YAML files.

Features:
- Galaxy and star system generation with celestial bodies
- Random NPC/Personnel generation with names, backstories, traits
- Quest/Mission/Contract generation
- Spaceship variant generation
- Configurable randomness and rules-based generation

Usage:
    # Command line - Interactive mode
    python ProceduralGenerators.py
    
    # Command line - Generate specific content
    python ProceduralGenerators.py --type galaxy --count 5
    python ProceduralGenerators.py --type npc --count 20 --role Captain
    python ProceduralGenerators.py --type quest --count 10
    
    # In Unreal Editor Python console
    import ProceduralGenerators
    ProceduralGenerators.generate_galaxy("Alpha Sector", num_systems=10)
    ProceduralGenerators.generate_npcs(count=50)
"""

import os
import sys
import random
import argparse
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime


class NameGenerator:
    """Generates realistic names for NPCs, ships, locations, etc."""
    
    # Name components for procedural generation
    FIRST_NAMES_MALE = [
        "James", "John", "Robert", "Michael", "William", "David", "Richard", "Joseph",
        "Thomas", "Charles", "Christopher", "Daniel", "Matthew", "Anthony", "Mark",
        "Donald", "Steven", "Paul", "Andrew", "Joshua", "Kenneth", "Kevin", "Brian",
        "Edward", "Ronald", "Timothy", "Jason", "Jeffrey", "Ryan", "Jacob", "Gary",
        "Nicolas", "Eric", "Jonathan", "Stephen", "Larry", "Justin", "Scott", "Brandon",
        "Raymond", "Gregory", "Samuel", "Benjamin", "Patrick", "Jack", "Dennis", "Jerry",
        "Tyler", "Aaron", "Henry", "Douglas", "Peter", "Adam", "Nathan", "Zachary"
    ]
    
    FIRST_NAMES_FEMALE = [
        "Mary", "Patricia", "Jennifer", "Linda", "Barbara", "Elizabeth", "Susan",
        "Jessica", "Sarah", "Karen", "Nancy", "Lisa", "Betty", "Margaret", "Sandra",
        "Ashley", "Kimberly", "Emily", "Donna", "Michelle", "Dorothy", "Carol", "Amanda",
        "Melissa", "Deborah", "Stephanie", "Rebecca", "Sharon", "Laura", "Cynthia",
        "Kathleen", "Amy", "Angela", "Shirley", "Anna", "Brenda", "Pamela", "Emma",
        "Nicole", "Helen", "Samantha", "Katherine", "Christine", "Debra", "Rachel",
        "Carolyn", "Janet", "Catherine", "Maria", "Heather", "Diane", "Ruth", "Julie"
    ]
    
    LAST_NAMES = [
        "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
        "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson",
        "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Perez", "Thompson",
        "White", "Harris", "Sanchez", "Clark", "Ramirez", "Lewis", "Robinson", "Walker",
        "Young", "Allen", "King", "Wright", "Scott", "Torres", "Nguyen", "Hill", "Flores",
        "Green", "Adams", "Nelson", "Baker", "Hall", "Rivera", "Campbell", "Mitchell",
        "Carter", "Roberts", "Gomez", "Phillips", "Evans", "Turner", "Diaz", "Parker",
        "Cruz", "Edwards", "Collins", "Reyes", "Stewart", "Morris", "Morales", "Murphy",
        "Cook", "Rogers", "Morgan", "Peterson", "Cooper", "Reed", "Bailey", "Bell",
        "Gomez", "Kelly", "Howard", "Ward", "Cox", "Diaz", "Richardson", "Wood", "Watson",
        "Brooks", "Bennett", "Gray", "James", "Reyes", "Cruz", "Hughes", "Price", "Myers"
    ]
    
    STAR_PREFIXES = ["Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta",
                     "Nova", "Lux", "Stellar", "Astro", "Celestia", "Cosmo", "Nebula"]
    
    STAR_SUFFIXES = ["Prime", "Major", "Minor", "Centauri", "Proxima", "Secundus", "Tertius",
                     "Australis", "Borealis", "Ascendant", "Radiant", "Luminous"]
    
    PLANET_PREFIXES = ["New", "Terra", "Aqua", "Arid", "Frost", "Lava", "Verde", "Azure",
                       "Crimson", "Golden", "Silver", "Crystal", "Ember", "Storm"]
    
    PLANET_SUFFIXES = ["World", "Prime", "Haven", "Colony", "Outpost", "Station", "Base",
                       "Settlement", "Refuge", "Sanctuary", "Frontier", "Expanse"]
    
    @classmethod
    def generate_person_name(cls, gender: Optional[str] = None) -> str:
        """Generate a random person name"""
        if gender is None:
            gender = random.choice(["Male", "Female"])
        
        if gender == "Male":
            first = random.choice(cls.FIRST_NAMES_MALE)
        else:
            first = random.choice(cls.FIRST_NAMES_FEMALE)
        
        last = random.choice(cls.LAST_NAMES)
        
        # Sometimes add middle initial
        if random.random() < 0.3:
            middle = random.choice("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
            return f"{first} {middle}. {last}"
        
        return f"{first} {last}"
    
    @classmethod
    def generate_star_name(cls) -> str:
        """Generate a star system name"""
        prefix = random.choice(cls.STAR_PREFIXES)
        
        if random.random() < 0.5:
            suffix = random.choice(cls.STAR_SUFFIXES)
            return f"{prefix} {suffix}"
        else:
            # Add a number
            num = random.randint(1, 999)
            return f"{prefix} {num}"
    
    @classmethod
    def generate_planet_name(cls) -> str:
        """Generate a planet name"""
        prefix = random.choice(cls.PLANET_PREFIXES)
        suffix = random.choice(cls.PLANET_SUFFIXES)
        
        if random.random() < 0.3:
            # Add roman numeral
            roman = random.choice(["I", "II", "III", "IV", "V", "VI", "VII", "VIII"])
            return f"{prefix} {suffix} {roman}"
        
        return f"{prefix} {suffix}"
    
    @classmethod
    def generate_ship_name(cls) -> str:
        """Generate a ship name"""
        adjectives = ["Valiant", "Swift", "Eternal", "Crimson", "Azure", "Golden",
                     "Silent", "Thunder", "Storm", "Star", "Void", "Phoenix",
                     "Dragon", "Eagle", "Falcon", "Raven", "Wolf", "Tiger"]
        
        nouns = ["Voyager", "Explorer", "Pathfinder", "Pioneer", "Venture", "Quest",
                "Seeker", "Hunter", "Guardian", "Defender", "Sentinel", "Warden",
                "Ranger", "Striker", "Interceptor", "Vanguard", "Herald", "Odyssey"]
        
        if random.random() < 0.7:
            return f"{random.choice(adjectives)} {random.choice(nouns)}"
        else:
            # Single word name
            return random.choice(adjectives + nouns)


class BiographyGenerator:
    """Generates procedural backstories and biographies"""
    
    CAREER_PATHS = [
        "started as a cargo hauler and worked their way up through the ranks",
        "served in the military for {years} years before retiring to private sector",
        "graduated from the academy with honors and has been rising steadily",
        "comes from a long line of spacers dating back three generations",
        "was a renowned scientist before transitioning to space operations",
        "spent years as a freelance pilot before joining organized service",
        "worked as a mining surveyor in the outer colonies",
        "was a trader on the frontier worlds before seeking more stable employment",
        "served as a medical officer during the border conflicts",
        "trained as an engineer at the prestigious Stellar Institute"
    ]
    
    PERSONALITY_TRAITS = [
        "known for their calm demeanor under pressure",
        "has a reputation for being tough but fair",
        "respected for their tactical brilliance",
        "admired for their unwavering loyalty",
        "feared by enemies and respected by allies",
        "considered a natural born leader",
        "valued for their diplomatic skills",
        "recognized for their technical expertise",
        "celebrated for their innovative problem-solving",
        "noted for their strong moral compass"
    ]
    
    ACHIEVEMENTS = [
        "successfully completed {count} deep space missions",
        "saved their crew from certain disaster during a reactor failure",
        "negotiated a peace treaty between two warring factions",
        "discovered a new jump route saving weeks of travel time",
        "defended a civilian station against pirate raiders",
        "led a successful rescue operation in hostile territory",
        "developed an improved navigation protocol still in use today",
        "earned multiple commendations for bravery and service",
        "published research that advanced the field of {field}",
        "trained the next generation of {role}s at the academy"
    ]
    
    @classmethod
    def generate_biography(cls, role: str, age: int) -> str:
        """Generate a character biography"""
        # Ensure minimum years calculation doesn't go negative
        max_years = max(1, min(age - 25, 30))
        years = random.randint(1, max_years) if max_years >= 1 else 1
        count = random.randint(10, 100)
        
        fields = ["astrogation", "ship systems", "trade economics", "xenobiology"]
        field = random.choice(fields)
        
        career = random.choice(cls.CAREER_PATHS).format(years=years)
        personality = random.choice(cls.PERSONALITY_TRAITS)
        achievement = random.choice(cls.ACHIEVEMENTS).format(count=count, field=field, role=role)
        
        return f"This individual {career}. They are {personality} and {achievement}."


class ProceduralGenerators:
    """Main class for procedural content generation"""
    
    def __init__(self, project_root: Optional[Path] = None):
        """Initialize the procedural generator"""
        self.project_root = project_root or Path.cwd()
        self.output_dir = self.project_root / "Assets" / "ProceduralContent"
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Create subdirectories
        (self.output_dir / "Galaxy").mkdir(exist_ok=True)
        (self.output_dir / "Personnel").mkdir(exist_ok=True)
        (self.output_dir / "Quests").mkdir(exist_ok=True)
        (self.output_dir / "Ships").mkdir(exist_ok=True)
    
    def generate_star_system(self, system_name: Optional[str] = None,
                            num_planets: Optional[int] = None) -> Dict[str, Any]:
        """
        Generate a procedural star system with planets, asteroids, etc.
        
        Args:
            system_name: Name of the star system (generated if None)
            num_planets: Number of planets (random 1-8 if None)
            
        Returns:
            Dictionary containing star system data
        """
        if system_name is None:
            system_name = NameGenerator.generate_star_name()
        
        if num_planets is None:
            num_planets = random.randint(1, 8)
        
        # Star properties
        star_types = ["Red Dwarf", "Yellow Dwarf", "Blue Giant", "Red Giant", "White Dwarf"]
        star_type = random.choice(star_types)
        
        star_mass = random.uniform(0.5, 3.0)
        star_luminosity = random.uniform(0.1, 100.0)
        
        # Generate planets
        planets = []
        for i in range(num_planets):
            planet = self._generate_planet(i + 1)
            planets.append(planet)
        
        # Asteroid belts
        num_asteroid_belts = random.randint(0, 3)
        asteroid_belts = []
        for i in range(num_asteroid_belts):
            belt = {
                "Name": f"{system_name} Asteroid Belt {chr(65 + i)}",
                "OrbitDistance": random.uniform(150000, 500000),
                "Density": random.choice(["Sparse", "Moderate", "Dense"]),
                "Resources": random.sample(["Iron", "Titanium", "Platinum", "Rare Earth"],
                                         k=random.randint(1, 3))
            }
            asteroid_belts.append(belt)
        
        # Space stations (0-2)
        num_stations = random.randint(0, 2)
        stations = []
        for i in range(num_stations):
            station_types = ["Trading Hub", "Mining Station", "Research Facility", "Military Outpost"]
            station = {
                "Name": f"{system_name} Station {i + 1}",
                "Type": random.choice(station_types),
                "OrbitDistance": random.uniform(100000, 800000),
                "Population": random.randint(50, 5000)
            }
            stations.append(station)
        
        system_data = {
            "SystemName": system_name,
            "SystemID": system_name.replace(" ", ""),
            "StarType": star_type,
            "StarMass": round(star_mass, 2),
            "StarLuminosity": round(star_luminosity, 2),
            "Coordinates": {
                "X": random.randint(-10000, 10000),
                "Y": random.randint(-10000, 10000),
                "Z": random.randint(-1000, 1000)
            },
            "Planets": planets,
            "AsteroidBelts": asteroid_belts,
            "Stations": stations,
            "Factions": random.sample(["Independent", "United Earth", "Free Traders",
                                      "Mining Guild", "Research Consortium"],
                                     k=random.randint(1, 3)),
            "DangerLevel": random.choice(["Safe", "Low", "Moderate", "High", "Extreme"]),
            "TrafficLevel": random.choice(["None", "Light", "Moderate", "Heavy"]),
            "bHasJumpGate": random.choice([True, False])
        }
        
        return system_data
    
    def _generate_planet(self, orbit_number: int) -> Dict[str, Any]:
        """Generate a single planet's properties"""
        planet_types = ["Rocky", "Gas Giant", "Ice", "Desert", "Ocean", "Lava", "Terran"]
        planet_type = random.choice(planet_types)
        
        # Type-based properties
        if planet_type == "Gas Giant":
            size = random.uniform(50000, 150000)
            habitability = 0
            has_atmosphere = True
        elif planet_type in ["Rocky", "Desert"]:
            size = random.uniform(5000, 15000)
            habitability = random.randint(0, 50)
            has_atmosphere = random.choice([True, False])
        elif planet_type == "Terran":
            size = random.uniform(10000, 14000)
            habitability = random.randint(60, 95)
            has_atmosphere = True
        else:
            size = random.uniform(5000, 30000)
            habitability = random.randint(0, 40)
            has_atmosphere = random.choice([True, False])
        
        planet_data = {
            "Name": NameGenerator.generate_planet_name(),
            "Type": planet_type,
            "OrbitNumber": orbit_number,
            "OrbitDistance": orbit_number * random.uniform(50000, 100000),
            "Size": round(size, 2),
            "Mass": round(random.uniform(0.1, 10.0), 2),
            "Gravity": round(random.uniform(0.3, 2.5), 2),
            "Temperature": random.randint(-200, 500),
            "HasAtmosphere": has_atmosphere,
            "Habitability": habitability,
            "Population": random.randint(0, 1000000) if habitability > 30 else 0,
            "Resources": random.sample(["Water", "Minerals", "Organics", "Rare Metals",
                                       "Helium-3", "Uranium", "Exotic Matter"],
                                      k=random.randint(1, 4)),
            "HasMoons": random.choice([True, False]),
            "NumberOfMoons": random.randint(0, 5) if random.choice([True, False]) else 0
        }
        
        return planet_data
    
    def save_star_system(self, system_data: Dict[str, Any]) -> Path:
        """Save star system data to YAML file"""
        filename = f"System_{system_data['SystemID']}.yaml"
        filepath = self.output_dir / "Galaxy" / filename
        
        yaml_content = f"""# Star System: {system_data['SystemName']}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Procedurally generated galaxy content

SystemName: "{system_data['SystemName']}"
SystemID: "{system_data['SystemID']}"

# ====================
# STAR PROPERTIES
# ====================
StarType: "{system_data['StarType']}"
StarMass: {system_data['StarMass']}  # Solar masses
StarLuminosity: {system_data['StarLuminosity']}  # Solar luminosity

# ====================
# COORDINATES
# ====================
Coordinates:
  X: {system_data['Coordinates']['X']}
  Y: {system_data['Coordinates']['Y']}
  Z: {system_data['Coordinates']['Z']}

# ====================
# PLANETS
# ====================
Planets:
"""
        
        for planet in system_data['Planets']:
            yaml_content += f"""  - Name: "{planet['Name']}"
    Type: "{planet['Type']}"
    OrbitNumber: {planet['OrbitNumber']}
    OrbitDistance: {planet['OrbitDistance']}  # km from star
    Size: {planet['Size']}  # km diameter
    Mass: {planet['Mass']}  # Earth masses
    Gravity: {planet['Gravity']}  # Earth gravity
    Temperature: {planet['Temperature']}  # Celsius
    HasAtmosphere: {str(planet['HasAtmosphere']).lower()}
    Habitability: {planet['Habitability']}  # 0-100 scale
    Population: {planet['Population']}
    Resources:
"""
            for resource in planet['Resources']:
                yaml_content += f'      - "{resource}"\n'
            yaml_content += f"""    HasMoons: {str(planet['HasMoons']).lower()}
    NumberOfMoons: {planet['NumberOfMoons']}

"""
        
        yaml_content += """# ====================
# ASTEROID BELTS
# ====================
AsteroidBelts:
"""
        
        for belt in system_data['AsteroidBelts']:
            yaml_content += f"""  - Name: "{belt['Name']}"
    OrbitDistance: {belt['OrbitDistance']}
    Density: "{belt['Density']}"
    Resources:
"""
            for resource in belt['Resources']:
                yaml_content += f'      - "{resource}"\n'
            yaml_content += '\n'
        
        yaml_content += """# ====================
# SPACE STATIONS
# ====================
Stations:
"""
        
        for station in system_data['Stations']:
            yaml_content += f"""  - Name: "{station['Name']}"
    Type: "{station['Type']}"
    OrbitDistance: {station['OrbitDistance']}
    Population: {station['Population']}

"""
        
        yaml_content += f"""# ====================
# SYSTEM INFO
# ====================
Factions:
"""
        for faction in system_data['Factions']:
            yaml_content += f'  - "{faction}"\n'
        
        yaml_content += f"""
DangerLevel: "{system_data['DangerLevel']}"
TrafficLevel: "{system_data['TrafficLevel']}"
bHasJumpGate: {str(system_data['bHasJumpGate']).lower()}
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(yaml_content)
        
        print(f"✓ Created star system: {filepath}")
        return filepath
    
    def generate_galaxy(self, galaxy_name: str = "Procedural Galaxy",
                       num_systems: int = 10) -> List[Path]:
        """
        Generate multiple star systems forming a galaxy sector
        
        Args:
            galaxy_name: Name of the galaxy/sector
            num_systems: Number of star systems to generate
            
        Returns:
            List of paths to created system files
        """
        print(f"\nGenerating {galaxy_name} with {num_systems} star systems...")
        
        created_files = []
        for i in range(num_systems):
            system_data = self.generate_star_system()
            filepath = self.save_star_system(system_data)
            created_files.append(filepath)
        
        print(f"\n✓ Generated {num_systems} star systems in {self.output_dir / 'Galaxy'}")
        return created_files
    
    def generate_npc(self, role: Optional[str] = None,
                    gender: Optional[str] = None) -> Dict[str, Any]:
        """
        Generate a random NPC with varied characteristics
        
        Args:
            role: Personnel role (Captain, Engineer, etc.) - random if None
            gender: Gender (Male, Female) - random if None
            
        Returns:
            Dictionary containing NPC data
        """
        # Role options
        all_roles = [
            "Captain", "Pilot", "Navigator", "Engineer", "Gunner",
            "Trader", "Scientist", "MedicalOfficer", "SecurityOfficer",
            "CargoSpecialist", "Communications", "SensorOperator"
        ]
        
        if role is None:
            role = random.choice(all_roles)
        
        if gender is None:
            gender = random.choice(["Male", "Female"])
        
        name = NameGenerator.generate_person_name(gender)
        age = random.randint(25, 65)
        
        # Generate biography
        biography = BiographyGenerator.generate_biography(role, age)
        
        # Generate skills based on role
        skills = self._generate_skills_for_role(role)
        
        # Generate traits
        traits = self._generate_random_traits()
        
        # Generate relationships (placeholder)
        num_relationships = random.randint(0, 3)
        
        npc_data = {
            "PersonnelID": f"{role}_{name.replace(' ', '')}",
            "PersonnelName": name,
            "Biography": biography,
            "Age": age,
            "Gender": gender,
            "Species": "Human",  # Could be expanded
            "Nationality": random.choice(["United Earth", "Colonial Alliance", "Free Systems",
                                         "Outer Rim", "Core Worlds"]),
            "Tags": [role, f"Level{random.randint(1, 10)}"],
            "PrimaryRole": role,
            "Department": self._get_department_for_role(role),
            "OverallSkillLevel": random.randint(5, 10),
            "TotalExperience": random.randint(1000, 50000),
            "Skills": skills,
            "Morale": random.randint(50, 100),
            "Health": random.randint(70, 100),
            "Fatigue": random.randint(0, 50),
            "Loyalty": random.randint(40, 95),
            "Reputation": random.randint(-20, 80),
            "Salary": random.randint(1000, 20000),
            "PersonalityType": random.choice(["Leader", "Analytical", "Adventurous",
                                             "Cautious", "Diplomatic", "Aggressive"]),
            "Traits": traits,
            "NumRelationships": num_relationships
        }
        
        return npc_data
    
    def _generate_skills_for_role(self, role: str) -> List[Dict[str, Any]]:
        """Generate appropriate skills for a role"""
        skill_templates = {
            "Captain": ["Leadership", "Tactical", "Navigation", "Diplomacy"],
            "Pilot": ["Piloting", "Navigation", "Reaction", "Maneuvering"],
            "Engineer": ["Engineering", "Repair", "Systems", "PowerManagement"],
            "Gunner": ["Gunnery", "Targeting", "Tactical", "WeaponSystems"],
            "Navigator": ["Navigation", "Astrogation", "Sensors", "Cartography"],
            "Trader": ["Trading", "Negotiation", "Economics", "Appraisal"],
            "Scientist": ["Research", "Analysis", "Biology", "Physics"],
            "MedicalOfficer": ["Medicine", "Surgery", "Biology", "Chemistry"],
            "SecurityOfficer": ["Combat", "Security", "Tactics", "Investigation"],
            "CargoSpecialist": ["Logistics", "Organization", "Trading", "LoadBalancing"],
            "Communications": ["Communications", "Linguistics", "Diplomacy", "Signals"],
            "SensorOperator": ["Sensors", "Detection", "Analysis", "Tracking"]
        }
        
        base_skills = skill_templates.get(role, ["General", "Competence", "Expertise"])
        
        skills = []
        for skill_name in base_skills:
            skill_level = random.randint(3, 10)
            xp = skill_level * random.randint(100, 500)
            skills.append({
                "SkillName": skill_name,
                "SkillLevel": skill_level,
                "ExperiencePoints": xp
            })
        
        return skills
    
    def _get_department_for_role(self, role: str) -> str:
        """Get department based on role"""
        dept_map = {
            "Captain": "Command",
            "Pilot": "Flight",
            "Navigator": "Flight",
            "Engineer": "Engineering",
            "Gunner": "Tactical",
            "Trader": "Operations",
            "Scientist": "Science",
            "MedicalOfficer": "Medical",
            "SecurityOfficer": "Security",
            "CargoSpecialist": "Operations",
            "Communications": "Communications",
            "SensorOperator": "Operations"
        }
        return dept_map.get(role, "General")
    
    def _generate_random_traits(self) -> List[Dict[str, Any]]:
        """Generate random personality traits"""
        available_traits = [
            {"Name": "Natural Leader", "Modifier": 1.2, "Description": "Inspires crew"},
            {"Name": "Combat Veteran", "Modifier": 1.15, "Description": "Experienced in combat"},
            {"Name": "Quick Learner", "Modifier": 1.1, "Description": "Gains XP faster"},
            {"Name": "Cautious", "Modifier": 0.9, "Description": "Reduced risk taking"},
            {"Name": "Brave", "Modifier": 1.1, "Description": "Performs well under pressure"},
            {"Name": "Technical Genius", "Modifier": 1.25, "Description": "Engineering bonus"},
            {"Name": "Diplomat", "Modifier": 1.1, "Description": "Better negotiations"},
            {"Name": "Lucky", "Modifier": 1.05, "Description": "Occasional bonuses"},
        ]
        
        num_traits = random.randint(1, 3)
        return random.sample(available_traits, k=num_traits)
    
    def save_npc(self, npc_data: Dict[str, Any]) -> Path:
        """Save NPC data to YAML file"""
        filename = f"{npc_data['PrimaryRole']}_{npc_data['PersonnelID']}.yaml"
        filepath = self.output_dir / "Personnel" / filename
        
        yaml_content = f"""# Personnel: {npc_data['PersonnelName']}
# Role: {npc_data['PrimaryRole']}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Procedurally generated NPC

PersonnelID: "{npc_data['PersonnelID']}"
PersonnelName: "{npc_data['PersonnelName']}"

# Basic Identity
Biography: |
  {npc_data['Biography']}
  
Age: {npc_data['Age']}
Gender: "{npc_data['Gender']}"
Species: "{npc_data['Species']}"
Nationality: "{npc_data['Nationality']}"
Tags:
"""
        for tag in npc_data['Tags']:
            yaml_content += f'  - "{tag}"\n'
        
        yaml_content += f"""
# Role and Assignment
PrimaryRole: "{npc_data['PrimaryRole']}"
Department: "{npc_data['Department']}"
CurrentAssignment: "Unassigned"
ManagerID: "None"
DirectReports: []

# Skills and Experience
OverallSkillLevel: {npc_data['OverallSkillLevel']}
TotalExperience: {npc_data['TotalExperience']}
Skills:
"""
        
        for skill in npc_data['Skills']:
            yaml_content += f"""  - SkillName: "{skill['SkillName']}"
    SkillLevel: {skill['SkillLevel']}
    ExperiencePoints: {skill['ExperiencePoints']}
"""
        
        yaml_content += f"""
Specialties:
  - "{npc_data['PrimaryRole']} Specialist"

# Status and Condition
Morale: {npc_data['Morale']}
Health: {npc_data['Health']}
Fatigue: {npc_data['Fatigue']}
Loyalty: {npc_data['Loyalty']}
Reputation: {npc_data['Reputation']}

# Employment
Salary: {npc_data['Salary']}
ContractDuration: 0
ContractMonthsRemaining: 0

# Personality and Traits
PersonalityType: "{npc_data['PersonalityType']}"
PersonalityDescription: |
  {npc_data['PersonalityType']} personality with focus on {npc_data['PrimaryRole'].lower()} duties.
  
Traits:
"""
        
        for trait in npc_data['Traits']:
            yaml_content += f"""  - TraitName: "{trait['Name']}"
    TraitDescription: "{trait['Description']}"
    TraitID: "{trait['Name'].replace(' ', '')}"
    ModifierValue: {trait['Modifier']}
"""
        
        yaml_content += """
# Relationships
Relationships: []

# Performance Metrics
PerformanceMetrics:
  - MetricName: "Overall Performance"
    Value: 75
    Trend: 0.0

# Past Assignments
PastAssignments: []
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(yaml_content)
        
        print(f"✓ Created NPC: {npc_data['PersonnelName']} ({npc_data['PrimaryRole']}) -> {filepath.name}")
        return filepath
    
    def generate_npcs(self, count: int = 10, role: Optional[str] = None) -> List[Path]:
        """
        Generate multiple random NPCs
        
        Args:
            count: Number of NPCs to generate
            role: Specific role (None for varied roles)
            
        Returns:
            List of paths to created NPC files
        """
        print(f"\nGenerating {count} random NPCs...")
        
        created_files = []
        for i in range(count):
            npc_data = self.generate_npc(role=role)
            filepath = self.save_npc(npc_data)
            created_files.append(filepath)
        
        print(f"\n✓ Generated {count} NPCs in {self.output_dir / 'Personnel'}")
        return created_files
    
    def generate_quest(self, quest_type: Optional[str] = None) -> Dict[str, Any]:
        """
        Generate a random quest/mission/contract
        
        Args:
            quest_type: Type of quest (Delivery, Combat, Exploration, etc.)
            
        Returns:
            Dictionary containing quest data
        """
        quest_types = ["Delivery", "Procurement", "Escort", "Combat", "Exploration",
                      "Rescue", "Investigation", "Smuggling", "Mining"]
        
        if quest_type is None:
            quest_type = random.choice(quest_types)
        
        quest_name = self._generate_quest_name(quest_type)
        difficulty = random.choice(["Easy", "Medium", "Hard", "VeryHard"])
        
        # Generate locations
        origin = NameGenerator.generate_star_name()
        destination = NameGenerator.generate_star_name()
        
        # Generate rewards based on difficulty
        difficulty_multipliers = {"Easy": 1, "Medium": 2, "Hard": 3, "VeryHard": 5}
        mult = difficulty_multipliers[difficulty]
        
        base_reward = random.randint(1000, 5000) * mult
        reputation = random.randint(5, 20) * mult
        
        quest_data = {
            "ContractName": quest_name,
            "ContractID": f"Quest_{quest_type}_{random.randint(1000, 9999)}",
            "Description": self._generate_quest_description(quest_type, origin, destination),
            "ContractType": quest_type,
            "Difficulty": difficulty,
            "OriginLocation": origin,
            "DestinationLocation": destination,
            "Distance": round(random.uniform(1000, 50000), 2),
            "TimeLimit": random.randint(24, 168),  # Hours
            "MinReputationRequired": random.randint(-10, 30),
            "CreditReward": base_reward,
            "ReputationReward": reputation,
            "ExperienceReward": random.randint(50, 500) * mult,
            "bRepeatable": random.choice([True, False]),
            "bCombatExpected": quest_type in ["Combat", "Escort", "Smuggling"],
            "bRequiresStealth": quest_type == "Smuggling",
            "Tags": [quest_type, difficulty, "Procedural"]
        }
        
        return quest_data
    
    def _generate_quest_name(self, quest_type: str) -> str:
        """Generate a quest name based on type"""
        delivery_names = ["Urgent Delivery", "Supply Run", "Critical Shipment", "Express Cargo"]
        combat_names = ["Eliminate Threat", "Clear Sector", "Pirate Hunt", "Defense Mission"]
        exploration_names = ["Survey Mission", "Uncharted Territory", "Mapping Run", "Discovery"]
        rescue_names = ["Rescue Operation", "Emergency Evac", "Distress Call", "Save Mission"]
        escort_names = ["VIP Transport", "Convoy Escort", "Protection Detail", "Safe Passage"]
        
        name_map = {
            "Delivery": delivery_names,
            "Procurement": ["Acquisition Request", "Resource Gathering", "Supply Acquisition"],
            "Combat": combat_names,
            "Exploration": exploration_names,
            "Rescue": rescue_names,
            "Escort": escort_names,
            "Investigation": ["Investigate Anomaly", "Research Mission", "Intelligence Gathering"],
            "Smuggling": ["Covert Delivery", "Discrete Transport", "Shadow Run"],
            "Mining": ["Mining Operation", "Resource Extraction", "Ore Collection"]
        }
        
        names = name_map.get(quest_type, ["Mission"])
        return random.choice(names)
    
    def _generate_quest_description(self, quest_type: str, origin: str, destination: str) -> str:
        """Generate quest description"""
        templates = {
            "Delivery": f"Transport cargo from {origin} to {destination}. Ensure timely delivery.",
            "Combat": f"Eliminate hostile forces in the {destination} sector. High danger expected.",
            "Exploration": f"Survey the {destination} system and report findings to {origin}.",
            "Rescue": f"Respond to distress call from {destination}. Lives are at stake.",
            "Escort": f"Protect convoy traveling from {origin} to {destination}.",
            "Investigation": f"Investigate unusual activity near {destination} on behalf of {origin}.",
            "Smuggling": f"Discretely deliver contraband from {origin} to {destination}. Avoid detection.",
            "Mining": f"Extract and deliver resources from asteroid field near {destination}.",
            "Procurement": f"Acquire specified goods and deliver to {origin}."
        }
        
        return templates.get(quest_type, f"Complete mission from {origin} to {destination}.")
    
    def save_quest(self, quest_data: Dict[str, Any]) -> Path:
        """Save quest data to YAML file"""
        filename = f"Quest_{quest_data['ContractType']}_{quest_data['ContractID'].split('_')[-1]}.yaml"
        filepath = self.output_dir / "Quests" / filename
        
        yaml_content = f"""# Quest: {quest_data['ContractName']}
# Type: {quest_data['ContractType']}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Procedurally generated quest/mission

ContractName: "{quest_data['ContractName']}"
ContractID: "{quest_data['ContractID']}"
Description: |
  {quest_data['Description']}
  
ContractType: {quest_data['ContractType']}
Difficulty: {quest_data['Difficulty']}

# Contract Giver (placeholder - assign appropriate faction)
ContractGiver: "DA_Faction_Independent"

# ====================
# LOCATIONS
# ====================
OriginLocation:
  LocationName: "{quest_data['OriginLocation']}"
  Coordinates: [{random.randint(0, 10000)}, {random.randint(0, 10000)}, {random.randint(0, 1000)}]
  MarketID: "Market_Origin"
  ControllingFaction: null

DestinationLocation:
  LocationName: "{quest_data['DestinationLocation']}"
  Coordinates: [{random.randint(0, 10000)}, {random.randint(0, 10000)}, {random.randint(0, 1000)}]
  MarketID: "Market_Destination"
  ControllingFaction: null

Distance: {quest_data['Distance']}
TimeLimit: {quest_data['TimeLimit']}
MinReputationRequired: {quest_data['MinReputationRequired']}

# ====================
# REWARDS & PENALTIES
# ====================
Rewards:
  Credits: {quest_data['CreditReward']}
  ReputationGain: {quest_data['ReputationReward']}
  ExperiencePoints: {quest_data['ExperienceReward']}
  BonusItems: []

Penalties:
  CreditPenalty: {quest_data['CreditReward'] // 4}
  ReputationPenalty: {quest_data['ReputationReward'] // 2}
  CollateralDeposit: {quest_data['CreditReward'] // 10}

# ====================
# SPECIAL CONDITIONS
# ====================
bRepeatable: {str(quest_data['bRepeatable']).lower()}
bRequiresStealth: {str(quest_data['bRequiresStealth']).lower()}
bCombatExpected: {str(quest_data['bCombatExpected']).lower()}
HostileFactions: []
ContractTags:
"""
        
        for tag in quest_data['Tags']:
            yaml_content += f'  - "{tag}"\n'
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(yaml_content)
        
        print(f"✓ Created quest: {quest_data['ContractName']} ({quest_data['Difficulty']}) -> {filepath.name}")
        return filepath
    
    def generate_quests(self, count: int = 10, quest_type: Optional[str] = None) -> List[Path]:
        """
        Generate multiple random quests
        
        Args:
            count: Number of quests to generate
            quest_type: Specific type (None for varied types)
            
        Returns:
            List of paths to created quest files
        """
        print(f"\nGenerating {count} random quests...")
        
        created_files = []
        for i in range(count):
            quest_data = self.generate_quest(quest_type=quest_type)
            filepath = self.save_quest(quest_data)
            created_files.append(filepath)
        
        print(f"\n✓ Generated {count} quests in {self.output_dir / 'Quests'}")
        return created_files
    
    def generate_ship_variant(self, base_class: str = "Fighter") -> Dict[str, Any]:
        """
        Generate a ship variant with randomized stats
        
        Args:
            base_class: Base ship class (Fighter, Scout, Trader, etc.)
            
        Returns:
            Dictionary containing ship data
        """
        ship_name = NameGenerator.generate_ship_name()
        
        # Base stats by class
        class_stats = {
            "Fighter": {
                "HullStrength": (800, 1200), "CargoCapacity": (50, 150),
                "MaxSpeed": (600, 900), "WeaponSlots": (2, 4)
            },
            "Scout": {
                "HullStrength": (600, 1000), "CargoCapacity": (30, 100),
                "MaxSpeed": (800, 1100), "WeaponSlots": (1, 2)
            },
            "Trader": {
                "HullStrength": (1000, 1500), "CargoCapacity": (500, 2000),
                "MaxSpeed": (400, 600), "WeaponSlots": (0, 2)
            },
            "Gunship": {
                "HullStrength": (1200, 1800), "CargoCapacity": (100, 300),
                "MaxSpeed": (500, 700), "WeaponSlots": (4, 8)
            },
            "Transport": {
                "HullStrength": (1500, 2500), "CargoCapacity": (1000, 5000),
                "MaxSpeed": (300, 500), "WeaponSlots": (0, 1)
            }
        }
        
        stats = class_stats.get(base_class, class_stats["Fighter"])
        
        ship_data = {
            "ShipName": ship_name,
            "ShipClass": base_class,
            "ShipID": ship_name.replace(" ", ""),
            "HullStrength": random.uniform(*stats["HullStrength"]),
            "CargoCapacity": random.uniform(*stats["CargoCapacity"]),
            "MaxSpeed": random.uniform(*stats["MaxSpeed"]),
            "WeaponSlots": random.randint(*stats["WeaponSlots"]),
            "ArmorRating": random.uniform(20, 80),
            "ShieldStrength": random.uniform(300, 800),
            "Maneuverability": random.randint(3, 10),
            "CrewRequired": random.randint(1, 8),
            "MaxCrew": random.randint(4, 20),
            "ModularPoints": random.randint(4, 16),
            "Manufacturer": random.choice(["Vanguard Aerospace", "Stellar Dynamics",
                                          "Nova Industries", "Frontier Shipworks"]),
            "RarityTier": random.choice(["Common", "Uncommon", "Rare"])
        }
        
        return ship_data
    
    def save_ship_variant(self, ship_data: Dict[str, Any]) -> Path:
        """Save ship variant to YAML file"""
        filename = f"{ship_data['ShipClass']}_{ship_data['ShipID']}.yaml"
        filepath = self.output_dir / "Ships" / filename
        
        yaml_content = f"""# {ship_data['ShipName']} - {ship_data['ShipClass']} Class
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Procedurally generated ship variant

BasicInfo:
  ShipName: "{ship_data['ShipName']}"
  Description: "A {ship_data['ShipClass'].lower()} class vessel with unique characteristics."
  ShipClass: "{ship_data['ShipClass']}"
  ShipID: "{ship_data['ShipID']}"

CoreStats:
  HullStrength: {ship_data['HullStrength']:.1f}
  CargoCapacity: {ship_data['CargoCapacity']:.1f}
  CrewRequired: {ship_data['CrewRequired']}
  MaxCrew: {ship_data['MaxCrew']}
  ModularPoints: {ship_data['ModularPoints']}

CombatStats:
  ArmorRating: {ship_data['ArmorRating']:.1f}
  ShieldStrength: {ship_data['ShieldStrength']:.1f}
  ShieldRechargeRate: 10.0
  WeaponSlots: {ship_data['WeaponSlots']}
  WeaponPowerCapacity: 1000.0
  PointDefenseRating: 5.0

MobilityStats:
  MaxSpeed: {ship_data['MaxSpeed']:.1f}
  Acceleration: 100.0
  Maneuverability: {ship_data['Maneuverability']}
  JumpRange: 15.0
  FuelCapacity: 1000.0
  FuelConsumptionRate: 80.0

UtilityStats:
  SensorRange: 10000.0
  SensorResolution: 5
  StealthRating: 3
  RepairSystemRating: 3
  ScienceRating: 3
  MedicalRating: 2

OperationalStats:
  PowerCapacity: 2500.0
  LifeSupportRating: 5
  MaintenanceLevel: 3
  HangarCapacity: 0
  DroneCapacity: 5
  AISystemRating: 5

AdvancedStats:
  DiplomacyRating: 2
  BoardingCapability: 2
  CustomizableSlots: 3
  EWarRating: 3
  MiningRating: 0

Lore:
  Manufacturer: "{ship_data['Manufacturer']}"
  YearBuilt: {random.randint(2450, 2500)}
  RarityTier: "{ship_data['RarityTier']}"
  LoreNotes: "A procedurally generated variant of the {ship_data['ShipClass']} class."
  PrimaryColor: [{random.uniform(0, 1):.2f}, {random.uniform(0, 1):.2f}, {random.uniform(0, 1):.2f}, 1.0]
  SecondaryColor: [{random.uniform(0, 1):.2f}, {random.uniform(0, 1):.2f}, {random.uniform(0, 1):.2f}, 1.0]
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(yaml_content)
        
        print(f"✓ Created ship variant: {ship_data['ShipName']} ({ship_data['ShipClass']}) -> {filepath.name}")
        return filepath
    
    def generate_ship_variants(self, count: int = 5, ship_class: Optional[str] = None) -> List[Path]:
        """Generate multiple ship variants"""
        print(f"\nGenerating {count} ship variants...")
        
        created_files = []
        for i in range(count):
            ship_data = self.generate_ship_variant(base_class=ship_class or "Fighter")
            filepath = self.save_ship_variant(ship_data)
            created_files.append(filepath)
        
        print(f"\n✓ Generated {count} ship variants in {self.output_dir / 'Ships'}")
        return created_files


def interactive_menu():
    """Interactive menu for procedural generation"""
    generator = ProceduralGenerators()
    
    while True:
        print("\n" + "=" * 60)
        print("Adastrea Procedural Content Generator")
        print("=" * 60)
        print("\n1. Generate Galaxy/Star Systems")
        print("2. Generate Random NPCs")
        print("3. Generate Quests/Missions")
        print("4. Generate Ship Variants")
        print("5. Generate Everything (Batch)")
        print("0. Exit")
        print()
        
        choice = input("Select option: ").strip()
        
        if choice == "1":
            count = int(input("Number of star systems to generate (default 5): ") or "5")
            generator.generate_galaxy(num_systems=count)
            
        elif choice == "2":
            count = int(input("Number of NPCs to generate (default 10): ") or "10")
            role = input("Specific role (leave empty for varied): ").strip() or None
            generator.generate_npcs(count=count, role=role)
            
        elif choice == "3":
            count = int(input("Number of quests to generate (default 10): ") or "10")
            quest_type = input("Specific type (leave empty for varied): ").strip() or None
            generator.generate_quests(count=count, quest_type=quest_type)
            
        elif choice == "4":
            count = int(input("Number of ship variants to generate (default 5): ") or "5")
            ship_class = input("Ship class (Fighter/Scout/Trader/Gunship, default Fighter): ").strip() or "Fighter"
            generator.generate_ship_variants(count=count, ship_class=ship_class)
            
        elif choice == "5":
            print("\nGenerating comprehensive content pack...")
            generator.generate_galaxy(num_systems=5)
            generator.generate_npcs(count=20)
            generator.generate_quests(count=15)
            generator.generate_ship_variants(count=8)
            print("\n✓ Batch generation complete!")
            
        elif choice == "0":
            print("Exiting...")
            break
        else:
            print("Invalid option!")


def main():
    """Main entry point for command-line usage"""
    parser = argparse.ArgumentParser(description="Adastrea Procedural Content Generator")
    parser.add_argument("--type", choices=["galaxy", "npc", "quest", "ship", "all"],
                       help="Type of content to generate")
    parser.add_argument("--count", type=int, default=10,
                       help="Number of items to generate")
    parser.add_argument("--role", type=str,
                       help="Specific role for NPC generation")
    parser.add_argument("--quest-type", type=str,
                       help="Specific quest type")
    parser.add_argument("--ship-class", type=str, default="Fighter",
                       help="Ship class for variant generation")
    parser.add_argument("--menu", action="store_true",
                       help="Run interactive menu")
    
    args = parser.parse_args()
    
    if args.menu or not args.type:
        interactive_menu()
        return
    
    generator = ProceduralGenerators()
    
    if args.type == "galaxy":
        generator.generate_galaxy(num_systems=args.count)
    elif args.type == "npc":
        generator.generate_npcs(count=args.count, role=args.role)
    elif args.type == "quest":
        generator.generate_quests(count=args.count, quest_type=args.quest_type)
    elif args.type == "ship":
        generator.generate_ship_variants(count=args.count, ship_class=args.ship_class)
    elif args.type == "all":
        print("\nGenerating comprehensive content pack...")
        generator.generate_galaxy(num_systems=5)
        generator.generate_npcs(count=20)
        generator.generate_quests(count=15)
        generator.generate_ship_variants(count=8)
        print("\n✓ Batch generation complete!")


if __name__ == "__main__":
    main()
