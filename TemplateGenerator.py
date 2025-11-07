#!/usr/bin/env python3
"""
Adastrea - Template Generator Script

This script generates YAML template files for various game systems.
Can be run from command line or imported in Unreal Editor Python environment.

Features:
- Generate Spaceship templates
- Generate Personnel templates
- Generate Trading system templates (items, markets, contracts)
- Generate Faction AI templates
- Generate Material templates
- Interactive and batch modes

Usage:
    # Command line - Interactive mode
    python TemplateGenerator.py
    
    # Command line - Batch mode
    python TemplateGenerator.py --type spaceship --name "MyShip" --output Assets/SpaceshipTemplates/
    
    # In Unreal Editor Python console
    import TemplateGenerator
    TemplateGenerator.generate_spaceship_template("MyShip")
"""

import os
import sys
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime


class TemplateGenerator:
    """Generator for creating YAML template files for Adastrea game systems"""
    
    def __init__(self, project_root: Optional[Path] = None):
        """
        Initialize the template generator
        
        Args:
            project_root: Path to project root. If None, uses current working directory
        """
        self.project_root = project_root or Path.cwd()
        self.templates_dir = self.project_root / "Assets"
        
        # Ensure templates directory exists
        self.templates_dir.mkdir(parents=True, exist_ok=True)
    
    def generate_spaceship_template(self, ship_name: str, ship_class: str = "Fighter",
                                   output_dir: Optional[Path] = None) -> Path:
        """
        Generate a spaceship template YAML file
        
        Args:
            ship_name: Name of the ship
            ship_class: Class of ship (Scout, Fighter, Trader, etc.)
            output_dir: Output directory (defaults to Assets/SpaceshipTemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "SpaceshipTemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Clean ship name for filename
        clean_name = ship_name.replace(" ", "")
        filename = f"{ship_class}_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# {ship_name} - {ship_class} Class
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Customize this template for your ship design

BasicInfo:
  ShipName: "{ship_name}"
  Description: "A {ship_class.lower()} class vessel designed for [describe purpose]."
  ShipClass: "{ship_class}"
  ShipID: "{clean_name}"

CoreStats:
  HullStrength: 1000.0          # Durability of the ship's hull
  CargoCapacity: 100.0          # Cargo space in tons
  CrewRequired: 2               # Minimum crew to operate
  MaxCrew: 6                    # Maximum crew capacity
  ModularPoints: 8              # Customization points

CombatStats:
  ArmorRating: 30.0             # Damage reduction from armor
  ShieldStrength: 500.0         # Shield hit points
  ShieldRechargeRate: 10.0      # Shield regeneration per second
  WeaponSlots: 2                # Number of weapon hardpoints
  WeaponPowerCapacity: 1000.0   # Total weapon power budget
  PointDefenseRating: 5.0       # Anti-missile defense capability

MobilityStats:
  MaxSpeed: 700.0               # Top speed in m/s
  Acceleration: 100.0           # Acceleration in m/s²
  Maneuverability: 7            # Turn rate (1-10 scale)
  JumpRange: 20.0               # Maximum FTL jump distance
  FuelCapacity: 2000.0          # Fuel tank capacity
  FuelConsumptionRate: 75.0     # Fuel usage per jump

UtilityStats:
  SensorRange: 15000.0          # Detection range in meters
  SensorResolution: 6           # Sensor quality (1-10 scale)
  StealthRating: 5              # Stealth capability (1-10 scale)
  RepairSystemRating: 5         # Onboard repair capability
  ScienceRating: 4              # Science equipment quality
  MedicalRating: 4              # Medical bay capability

OperationalStats:
  PowerCapacity: 5000.0         # Total power generation
  LifeSupportRating: 7          # Life support quality
  MaintenanceLevel: 4           # Maintenance complexity (lower is easier)
  HangarCapacity: 1             # Small craft capacity
  DroneCapacity: 5              # Drone/probe capacity
  AISystemRating: 5             # AI assistance level

AdvancedStats:
  DiplomacyRating: 4            # Diplomatic capability
  BoardingCapability: 3         # Boarding action capability
  CustomizableSlots: 6          # Module customization slots
  EWarRating: 4                 # Electronic warfare capability
  MiningRating: 0               # Mining equipment quality

Lore:
  Manufacturer: "[Manufacturer Name]"
  YearBuilt: {datetime.now().year}
  RarityTier: "Common"          # Common, Uncommon, Rare, Legendary
  LoreNotes: "[Add ship's backstory and notable history here]"
  PrimaryColor: [0.5, 0.5, 0.5, 1.0]    # RGBA color values
  SecondaryColor: [0.8, 0.8, 0.8, 1.0]  # RGBA color values
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created spaceship template: {filepath}")
        return filepath
    
    def generate_personnel_template(self, name: str, role: str = "CrewMember",
                                   output_dir: Optional[Path] = None) -> Path:
        """
        Generate a personnel template YAML file
        
        Args:
            name: Personnel name or identifier
            role: Role type (Captain, Pilot, Engineer, etc.)
            output_dir: Output directory (defaults to Assets/PersonnelTemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "PersonnelTemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Clean name for filename
        clean_name = name.replace(" ", "")
        filename = f"{role}_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# {name} - {role}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
# Customize this template for your personnel

PersonnelID: "{role}_{clean_name}"
PersonnelName: "{name}"

# Basic Identity
Biography: |
  [Write the character's background story here.
  Include their career history, major achievements, and motivations.
  This can be multiple lines.]
Age: 30
Gender: "Male"              # Male, Female, Non-Binary, Other
Species: "Human"            # Human, Alien, Synthetic, etc.
Nationality: "United Earth"
Tags:
  - "{role}"
  - "Experienced"

# Role and Assignment
PrimaryRole: "{role}"
CurrentAssignment: "[Ship/Station Name]"
Department: "Operations"
ManagerID: "None"
DirectReports: []

# Skills and Experience
OverallSkillLevel: 5        # 1-10 scale
TotalExperience: 5000       # Total experience points earned
Skills:
  - SkillName: "Primary Skill"
    SkillLevel: 6
    ExperiencePoints: 1500
  - SkillName: "Secondary Skill"
    SkillLevel: 4
    ExperiencePoints: 800
  - SkillName: "Tertiary Skill"
    SkillLevel: 3
    ExperiencePoints: 500
Specialties:
  - "[Primary Specialty]"
  - "[Secondary Specialty]"

# Status and Condition
Morale: 70                  # 0-100
Health: 90                  # 0-100
Fatigue: 30                 # 0-100 (lower is better)
Loyalty: 75                 # 0-100
Reputation: 50              # -100 to 100

# Employment
Salary: 5000                # Monthly salary in credits
ContractDuration: 12        # Months (0 = indefinite)
ContractMonthsRemaining: 12

# Personality and Traits
PersonalityType: "Professional"
PersonalityDescription: |
  [Describe the character's personality, work style, and character traits.]
Traits:
  - TraitName: "Skilled Worker"
    TraitDescription: "Performs job duties efficiently"
    TraitID: "SkilledWorker"
    ModifierValue: 1.1      # 10% performance bonus
  - TraitName: "Team Player"
    TraitDescription: "Works well with others"
    TraitID: "TeamPlayer"
    ModifierValue: 1.05     # 5% crew morale bonus

# Relationships
Relationships:
  - TargetPersonnelID: "[OtherPersonID]"
    RelationshipType: "Colleague"
    RelationshipStrength: 50  # -100 to 100
    Notes: "Professional working relationship"

# Performance Metrics
PerformanceMetrics:
  - MetricName: "Job Performance"
    Value: 75               # 0-100
    Trend: 2.0              # Positive = improving
  - MetricName: "Attendance"
    Value: 95
    Trend: 0.0

# Past Assignments
PastAssignments:
  - AssignmentLocation: "[Previous Location]"
    RoleHeld: "{role}"
    DurationMonths: 12
    PerformanceRating: 4    # 1-5 stars
    Notes: "Good performance in previous role"
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created personnel template: {filepath}")
        return filepath
    
    def generate_trade_item_template(self, item_name: str, category: str = "RawMaterials",
                                    output_dir: Optional[Path] = None) -> Path:
        """
        Generate a trade item template YAML file
        
        Args:
            item_name: Name of the trade item
            category: Item category (RawMaterials, Manufactured, Luxury, etc.)
            output_dir: Output directory (defaults to Assets/TradingTemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "TradingTemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = item_name.replace(" ", "")
        filename = f"TradeItem_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# Trade Item: {item_name}
# Category: {category}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

ItemName: "{item_name}"
ItemID: "TradeItem_{clean_name}"
Description: |
  [Describe the trade item, its uses, and importance in the economy]
  
Category: {category}

# Material Data Reference (optional)
MaterialData: "DA_Material_{clean_name}"

# ====================
# PRICING
# ====================
BasePrice: 100.0
PriceVolatility:
  VolatilityMultiplier: 1.0
  MinPriceDeviation: 0.7
  MaxPriceDeviation: 1.5
  
bAffectedBySupplyDemand: true
bAffectedByMarketEvents: true

# ====================
# LEGALITY & RESTRICTIONS
# ====================
LegalityStatus: Legal       # Legal, Restricted, Contraband
TradeRestrictions:
  BannedFactions: []
  RequiresPermitFactions: []
  MinReputationRequired: -100
  
ContrabandFineMultiplier: 0.0

# ====================
# TRADE PROPERTIES
# ====================
VolumePerUnit: 1.0
MassPerUnit: 1.0
StandardLotSize: 100
TypicalMarketStock: 1000
ReplenishmentRate: 100

# ====================
# AI TRADER BEHAVIOR
# ====================
AITradePriority: 5          # 1-10 priority for AI traders
bAIHoardable: false
bAIArbitrageEnabled: true
BehaviorTags:
  - "{category}"
  - "StandardItem"
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created trade item template: {filepath}")
        return filepath
    
    def generate_market_template(self, market_name: str, market_type: str = "Standard",
                                output_dir: Optional[Path] = None) -> Path:
        """
        Generate a market template YAML file
        
        Args:
            market_name: Name of the market
            market_type: Market type (Standard, FreePort, BlackMarket, etc.)
            output_dir: Output directory (defaults to Assets/TradingTemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "TradingTemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = market_name.replace(" ", "")
        filename = f"Market_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# Market: {market_name}
# Type: {market_type}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

MarketName: "{market_name}"
MarketID: "Market_{clean_name}"
MarketType: "{market_type}"

# Location
StationID: "[AssociatedStationID]"
SystemID: "[SystemID]"
FactionID: "[ControllingFactionID]"

# Market Properties
TaxRate: 0.05               # 5% sales tax
LicenseFee: 1000.0          # Cost to obtain trading license
MinReputationToTrade: 0     # Minimum reputation required

# Allowed Categories
AllowedCategories:
  - "RawMaterials"
  - "Manufactured"
  - "Food"
  - "Technology"
  # Add or remove categories as needed

# Banned Categories (for black markets, add Legal items)
BannedCategories: []

# Market Specialization
SpecializedIn:
  - "{market_type}"

# Supply Modifiers
SupplyModifiers:
  RawMaterials: 1.0
  Manufactured: 1.0
  Luxury: 1.0
  Contraband: 0.0

# Demand Modifiers
DemandModifiers:
  RawMaterials: 1.0
  Manufactured: 1.0
  Luxury: 1.0
  Contraband: 0.0

# Operating Hours
bAlwaysOpen: true
OpeningHour: 0
ClosingHour: 24

# Security
SecurityLevel: 5            # 1-10 scale
bScansForContraband: true
bEnforcesReputationLimits: true
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created market template: {filepath}")
        return filepath
    
    def generate_contract_template(self, contract_name: str, contract_type: str = "Delivery",
                                  output_dir: Optional[Path] = None) -> Path:
        """
        Generate a contract template YAML file
        
        Args:
            contract_name: Name/description of the contract
            contract_type: Contract type (Delivery, Combat, Exploration, etc.)
            output_dir: Output directory (defaults to Assets/TradingTemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "TradingTemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = contract_name.replace(" ", "")
        filename = f"Contract_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# Contract: {contract_name}
# Type: {contract_type}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

ContractID: "Contract_{clean_name}"
ContractName: "{contract_name}"
ContractType: "{contract_type}"

# Contract Details
Description: |
  [Describe the contract objectives, requirements, and any special conditions]

IssuerFactionID: "[FactionID]"
ReputationRequired: 0       # Minimum reputation to accept
DifficultyLevel: 3          # 1-10 scale

# Objectives
Objectives:
  - ObjectiveID: "Obj_1"
    Description: "[Primary objective description]"
    bRequired: true
    RewardBonus: 0.0
  - ObjectiveID: "Obj_2"
    Description: "[Optional objective description]"
    bRequired: false
    RewardBonus: 500.0

# Rewards
BaseReward: 5000.0          # Credits
BonusReward: 2000.0         # Additional credits for completing bonus objectives
ReputationGain: 10          # Reputation points with issuer faction
ExperienceGain: 500         # Experience points

# Time Limits
TimeLimit: 72               # Hours to complete (0 = no limit)
ExpirationTime: 168         # Hours until contract expires if not accepted

# Requirements
RequiredCargoSpace: 0       # Minimum cargo capacity needed
RequiredCombatRating: 0     # Minimum combat rating
RequiredShipClass: "Any"    # Specific ship class requirement

# Risk Assessment
DangerRating: 3             # 1-10 scale
bCombatExpected: false
bIllegalActivity: false

# Locations
StartLocationID: "[StationID]"
EndLocationID: "[DestinationStationID]"
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created contract template: {filepath}")
        return filepath
    
    def generate_faction_ai_template(self, faction_name: str,
                                    output_dir: Optional[Path] = None) -> Path:
        """
        Generate a faction AI template YAML file
        
        Args:
            faction_name: Name of the faction
            output_dir: Output directory (defaults to Assets/FactionAITemplates/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir / "FactionAITemplates"
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = faction_name.replace(" ", "")
        filename = f"{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# Faction AI Configuration: {faction_name}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

FactionID: "{clean_name}"
FactionName: "{faction_name}"

# ====================
# AI PERSONALITY
# ====================
AggressionLevel: 5          # 1-10 (1=Peaceful, 10=Hostile)
ExpansionDrive: 5           # 1-10 (1=Isolationist, 10=Expansionist)
EconomicFocus: 5            # 1-10 (1=Military, 10=Trade)
DiplomaticOpenness: 5       # 1-10 (1=Closed, 10=Friendly)
TechnologicalAmbition: 5    # 1-10 (1=Traditional, 10=Cutting-edge)

# ====================
# STRATEGIC PRIORITIES
# ====================
Priorities:
  - Priority: "EconomicGrowth"
    Weight: 30              # Percentage weight
  - Priority: "MilitaryStrength"
    Weight: 25
  - Priority: "TerritoryExpansion"
    Weight: 20
  - Priority: "DiplomaticRelations"
    Weight: 15
  - Priority: "TechnologicalAdvancement"
    Weight: 10

# ====================
# DIPLOMATIC BEHAVIOR
# ====================
DiplomaticStance:
  DefaultRelationship: 0    # -100 to 100
  AllianceThreshold: 60     # Form alliance if relationship above this
  WarThreshold: -60         # Declare war if relationship below this
  TradeThreshold: 20        # Open trade routes if relationship above this

# Faction relationships
PreferredAllies: []
HistoricRivals: []
NeutralTowards: []

# ====================
# ECONOMIC BEHAVIOR
# ====================
EconomicStrategy:
  PreferredTradeGoods:
    - "RawMaterials"
    - "Manufactured"
  InvestmentFocus: "Infrastructure"  # Infrastructure, Military, Technology
  TaxPolicy: 0.10           # 10% tax rate
  SubsidyLevel: 0.05        # 5% subsidy for key industries

# ====================
# MILITARY BEHAVIOR
# ====================
MilitaryStrategy:
  PreferredShipTypes:
    - "Fighter"
    - "Frigate"
  DefensePosture: "Balanced"  # Aggressive, Balanced, Defensive
  FleetComposition:
    Scouts: 20              # Percentage
    Fighters: 40
    Capitals: 30
    Support: 10

# ====================
# EXPANSION BEHAVIOR
# ====================
ExpansionStrategy:
  PreferredSystems:
    - "ResourceRich"
    - "StrategicLocation"
  ColonizationRate: 5       # 1-10 scale
  bAggressiveExpansion: false

# ====================
# SPECIAL BEHAVIORS
# ====================
SpecialBehaviors:
  - "PeacefulEarlyGame"     # Don't attack in first 10 game hours
  - "TradePreference"       # Prefer diplomatic solutions
  
bUseDynamicPriorities: true  # Adjust priorities based on game state
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created faction AI template: {filepath}")
        return filepath
    
    def generate_material_template(self, material_name: str,
                                  output_dir: Optional[Path] = None) -> Path:
        """
        Generate a material template YAML file
        
        Args:
            material_name: Name of the material
            output_dir: Output directory (defaults to Assets/)
            
        Returns:
            Path to the created template file
        """
        if output_dir is None:
            output_dir = self.templates_dir
        
        clean_name = material_name.replace(" ", "")
        filename = f"Material_{clean_name}.yaml"
        filepath = output_dir / filename
        
        template_content = f"""# Material Definition: {material_name}
# Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

MaterialID: "Mat_{clean_name}"
MaterialName: "{material_name}"

# Basic Properties
Category: "Metal"           # Metal, Composite, Organic, Energy, Exotic
Rarity: "Common"            # Common, Uncommon, Rare, Legendary
Description: |
  [Describe the material, its properties, and uses]

# Physical Properties
Density: 7.8                # g/cm³
Hardness: 5.0               # Mohs scale (1-10)
MeltingPoint: 1500          # Celsius
Conductivity: 0.5           # 0-1 scale

# Economic Properties
BaseValue: 100              # Credits per unit
ExtractionDifficulty: 3     # 1-10 scale
ProcessingCost: 50          # Credits to refine per unit
StorageRequirements: "Standard"

# Availability
CommonSources:
  - "Asteroids"
  - "PlanetarySurface"
RichRegions:
  - "[SystemName]"

# Uses
Applications:
  - "ShipConstruction"
  - "StationComponents"
  - "Manufacturing"

# Special Properties
SpecialTraits:
  - TraitName: "Durable"
    Description: "Resists wear and corrosion"
    Modifier: 1.2

# Trade Properties
bTradeable: true
bRenewable: false
bStrategicResource: false
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created material template: {filepath}")
        return filepath
    
    def interactive_mode(self):
        """Run the template generator in interactive mode"""
        print("=" * 60)
        print("Adastrea Template Generator - Interactive Mode")
        print("=" * 60)
        print()
        print("Available template types:")
        print("  1. Spaceship")
        print("  2. Personnel")
        print("  3. Trade Item")
        print("  4. Market")
        print("  5. Contract")
        print("  6. Faction AI")
        print("  7. Material")
        print("  0. Exit")
        print()
        
        while True:
            choice = input("Select template type (0-7): ").strip()
            
            if choice == "0":
                print("Exiting...")
                break
            elif choice == "1":
                name = input("Enter ship name: ").strip()
                ship_class = input("Enter ship class (Fighter/Scout/Trader/etc.): ").strip() or "Fighter"
                self.generate_spaceship_template(name, ship_class)
            elif choice == "2":
                name = input("Enter personnel name: ").strip()
                role = input("Enter role (Captain/Pilot/Engineer/etc.): ").strip() or "CrewMember"
                self.generate_personnel_template(name, role)
            elif choice == "3":
                name = input("Enter item name: ").strip()
                category = input("Enter category (RawMaterials/Manufactured/Luxury/etc.): ").strip() or "RawMaterials"
                self.generate_trade_item_template(name, category)
            elif choice == "4":
                name = input("Enter market name: ").strip()
                market_type = input("Enter market type (Standard/FreePort/BlackMarket/etc.): ").strip() or "Standard"
                self.generate_market_template(name, market_type)
            elif choice == "5":
                name = input("Enter contract name: ").strip()
                contract_type = input("Enter contract type (Delivery/Combat/Exploration/etc.): ").strip() or "Delivery"
                self.generate_contract_template(name, contract_type)
            elif choice == "6":
                name = input("Enter faction name: ").strip()
                self.generate_faction_ai_template(name)
            elif choice == "7":
                name = input("Enter material name: ").strip()
                self.generate_material_template(name)
            else:
                print("Invalid choice. Please select 0-7.")
            
            print()


def main():
    """Main entry point for command line usage"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Generate YAML templates for Adastrea game systems',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Interactive mode
  python TemplateGenerator.py
  
  # Generate specific templates
  python TemplateGenerator.py --type spaceship --name "Striker" --class Fighter
  python TemplateGenerator.py --type personnel --name "John Smith" --role Captain
  python TemplateGenerator.py --type tradeitem --name "Titanium Ore" --category RawMaterials
  python TemplateGenerator.py --type market --name "Central Hub" --market-type Standard
  python TemplateGenerator.py --type contract --name "Delivery Mission" --contract-type Delivery
  python TemplateGenerator.py --type factionai --name "Trade Federation"
  python TemplateGenerator.py --type material --name "Steel Alloy"
        """
    )
    
    parser.add_argument('--type', choices=['spaceship', 'personnel', 'tradeitem', 'market', 
                                          'contract', 'factionai', 'material'],
                       help='Type of template to generate')
    parser.add_argument('--name', help='Name for the template')
    parser.add_argument('--class', dest='ship_class', help='Ship class (for spaceship templates)')
    parser.add_argument('--role', help='Personnel role (for personnel templates)')
    parser.add_argument('--category', help='Item category (for trade item templates)')
    parser.add_argument('--market-type', help='Market type (for market templates)')
    parser.add_argument('--contract-type', help='Contract type (for contract templates)')
    parser.add_argument('--output', help='Output directory (optional)')
    
    args = parser.parse_args()
    
    generator = TemplateGenerator()
    
    # If no arguments provided, run interactive mode
    if not args.type:
        generator.interactive_mode()
        return
    
    # Validate that name is provided for batch mode
    if not args.name:
        print("Error: --name is required when using --type")
        sys.exit(1)
    
    output_dir = Path(args.output) if args.output else None
    
    # Generate based on type
    if args.type == 'spaceship':
        ship_class = args.ship_class or "Fighter"
        generator.generate_spaceship_template(args.name, ship_class, output_dir)
    elif args.type == 'personnel':
        role = args.role or "CrewMember"
        generator.generate_personnel_template(args.name, role, output_dir)
    elif args.type == 'tradeitem':
        category = args.category or "RawMaterials"
        generator.generate_trade_item_template(args.name, category, output_dir)
    elif args.type == 'market':
        market_type = args.market_type or "Standard"
        generator.generate_market_template(args.name, market_type, output_dir)
    elif args.type == 'contract':
        contract_type = args.contract_type or "Delivery"
        generator.generate_contract_template(args.name, contract_type, output_dir)
    elif args.type == 'factionai':
        generator.generate_faction_ai_template(args.name, output_dir)
    elif args.type == 'material':
        generator.generate_material_template(args.name, output_dir)


if __name__ == '__main__':
    main()
