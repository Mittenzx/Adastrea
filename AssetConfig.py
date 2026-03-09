"""
Adastrea - Asset Configuration Definitions

Copyright (c) 2025 Mittenzx. Licensed under MIT.

Shared configuration for asset types, column mappings, and data paths.
Used by both Google Sheets and CSV integration modules.
"""

from typing import Dict, Any, List


def get_spaceship_columns() -> List[Dict[str, str]]:
    """Get column definitions for spaceship data"""
    return [
        # Basic Info
        {'header': 'Ship ID', 'path': 'BasicInfo.ShipID', 'type': 'string'},
        {'header': 'Ship Name', 'path': 'BasicInfo.ShipName', 'type': 'string'},
        {'header': 'Description', 'path': 'BasicInfo.Description', 'type': 'string'},
        {'header': 'Ship Class', 'path': 'BasicInfo.ShipClass', 'type': 'string'},

        # Core Stats
        {'header': 'Hull Strength', 'path': 'CoreStats.HullStrength', 'type': 'number'},
        {'header': 'Cargo Capacity', 'path': 'CoreStats.CargoCapacity', 'type': 'number'},
        {'header': 'Crew Required', 'path': 'CoreStats.CrewRequired', 'type': 'integer'},
        {'header': 'Max Crew', 'path': 'CoreStats.MaxCrew', 'type': 'integer'},
        {'header': 'Modular Points', 'path': 'CoreStats.ModularPoints', 'type': 'integer'},

        # Combat Stats
        {'header': 'Armor Rating', 'path': 'CombatStats.ArmorRating', 'type': 'number'},
        {'header': 'Shield Strength', 'path': 'CombatStats.ShieldStrength', 'type': 'number'},
        {'header': 'Shield Recharge Rate', 'path': 'CombatStats.ShieldRechargeRate', 'type': 'number'},
        {'header': 'Weapon Slots', 'path': 'CombatStats.WeaponSlots', 'type': 'integer'},
        {'header': 'Weapon Power Capacity', 'path': 'CombatStats.WeaponPowerCapacity', 'type': 'number'},
        {'header': 'Point Defense Rating', 'path': 'CombatStats.PointDefenseRating', 'type': 'number'},

        # Mobility Stats
        {'header': 'Max Speed', 'path': 'MobilityStats.MaxSpeed', 'type': 'number'},
        {'header': 'Acceleration', 'path': 'MobilityStats.Acceleration', 'type': 'number'},
        {'header': 'Maneuverability', 'path': 'MobilityStats.Maneuverability', 'type': 'integer'},
        {'header': 'Jump Range', 'path': 'MobilityStats.JumpRange', 'type': 'number'},
        {'header': 'Fuel Capacity', 'path': 'MobilityStats.FuelCapacity', 'type': 'number'},
        {'header': 'Fuel Consumption Rate', 'path': 'MobilityStats.FuelConsumptionRate', 'type': 'number'},

        # Utility Stats
        {'header': 'Sensor Range', 'path': 'UtilityStats.SensorRange', 'type': 'number'},
        {'header': 'Sensor Resolution', 'path': 'UtilityStats.SensorResolution', 'type': 'integer'},
        {'header': 'Stealth Rating', 'path': 'UtilityStats.StealthRating', 'type': 'integer'},
        {'header': 'Repair System Rating', 'path': 'UtilityStats.RepairSystemRating', 'type': 'integer'},
        {'header': 'Science Rating', 'path': 'UtilityStats.ScienceRating', 'type': 'integer'},
        {'header': 'Medical Rating', 'path': 'UtilityStats.MedicalRating', 'type': 'integer'},

        # Operational Stats
        {'header': 'Power Capacity', 'path': 'OperationalStats.PowerCapacity', 'type': 'number'},
        {'header': 'Life Support Rating', 'path': 'OperationalStats.LifeSupportRating', 'type': 'integer'},
        {'header': 'Maintenance Level', 'path': 'OperationalStats.MaintenanceLevel', 'type': 'integer'},
        {'header': 'Hangar Capacity', 'path': 'OperationalStats.HangarCapacity', 'type': 'integer'},
        {'header': 'Drone Capacity', 'path': 'OperationalStats.DroneCapacity', 'type': 'integer'},
        {'header': 'AI System Rating', 'path': 'OperationalStats.AISystemRating', 'type': 'integer'},

        # Advanced Stats
        {'header': 'Diplomacy Rating', 'path': 'AdvancedStats.DiplomacyRating', 'type': 'integer'},
        {'header': 'Boarding Capability', 'path': 'AdvancedStats.BoardingCapability', 'type': 'integer'},
        {'header': 'Customizable Slots', 'path': 'AdvancedStats.CustomizableSlots', 'type': 'integer'},
        {'header': 'EWar Rating', 'path': 'AdvancedStats.EWarRating', 'type': 'integer'},
        {'header': 'Mining Rating', 'path': 'AdvancedStats.MiningRating', 'type': 'integer'},

        # Lore
        {'header': 'Manufacturer', 'path': 'Lore.Manufacturer', 'type': 'string'},
        {'header': 'Year Built', 'path': 'Lore.YearBuilt', 'type': 'integer'},
        {'header': 'Rarity Tier', 'path': 'Lore.RarityTier', 'type': 'string'},
        {'header': 'Lore Notes', 'path': 'Lore.LoreNotes', 'type': 'string'},
    ]


def get_trade_item_columns() -> List[Dict[str, str]]:
    """Get column definitions for trade item data"""
    return [
        {'header': 'Item ID', 'path': 'ItemID', 'type': 'string'},
        {'header': 'Item Name', 'path': 'ItemName', 'type': 'string'},
        {'header': 'Description', 'path': 'Description', 'type': 'string'},
        {'header': 'Category', 'path': 'Category', 'type': 'string'},
        {'header': 'Material Data', 'path': 'MaterialData', 'type': 'string'},
        {'header': 'Base Price', 'path': 'BasePrice', 'type': 'number'},
        {'header': 'Volatility Multiplier', 'path': 'PriceVolatility.VolatilityMultiplier', 'type': 'number'},
        {'header': 'Min Price Deviation', 'path': 'PriceVolatility.MinPriceDeviation', 'type': 'number'},
        {'header': 'Max Price Deviation', 'path': 'PriceVolatility.MaxPriceDeviation', 'type': 'number'},
        {'header': 'Affected By Supply/Demand', 'path': 'bAffectedBySupplyDemand', 'type': 'boolean'},
        {'header': 'Affected By Market Events', 'path': 'bAffectedByMarketEvents', 'type': 'boolean'},
        {'header': 'Legality Status', 'path': 'LegalityStatus', 'type': 'string'},
        {'header': 'Min Reputation Required', 'path': 'TradeRestrictions.MinReputationRequired', 'type': 'integer'},
        {'header': 'Contraband Fine Multiplier', 'path': 'ContrabandFineMultiplier', 'type': 'number'},
        {'header': 'Volume Per Unit', 'path': 'VolumePerUnit', 'type': 'number'},
        {'header': 'Mass Per Unit', 'path': 'MassPerUnit', 'type': 'number'},
        {'header': 'Standard Lot Size', 'path': 'StandardLotSize', 'type': 'integer'},
        {'header': 'Typical Market Stock', 'path': 'TypicalMarketStock', 'type': 'integer'},
        {'header': 'Replenishment Rate', 'path': 'ReplenishmentRate', 'type': 'integer'},
        {'header': 'AI Trade Priority', 'path': 'AITradePriority', 'type': 'integer'},
        {'header': 'AI Hoardable', 'path': 'bAIHoardable', 'type': 'boolean'},
        {'header': 'AI Arbitrage Enabled', 'path': 'bAIArbitrageEnabled', 'type': 'boolean'},
    ]


def get_faction_columns() -> List[Dict[str, str]]:
    """Get column definitions for faction data"""
    return [
        # Basic Info
        {'header': 'Faction ID', 'path': 'FactionID', 'type': 'string'},
        {'header': 'Faction Name', 'path': 'FactionName', 'type': 'string'},
        {'header': 'Description', 'path': 'Description', 'type': 'string'},
        {'header': 'Faction Type', 'path': 'FactionType', 'type': 'string'},
        {'header': 'Ethos', 'path': 'Ethos', 'type': 'string'},
        
        # Economic Properties
        {'header': 'Economic Power', 'path': 'EconomicPower', 'type': 'integer'},
        {'header': 'Technological Level', 'path': 'TechnologicalLevel', 'type': 'integer'},
        {'header': 'Military Strength', 'path': 'MilitaryStrength', 'type': 'integer'},
        
        # Traits (comma-separated)
        {'header': 'Traits', 'path': 'Traits', 'type': 'string'},
        
        # Colors
        {'header': 'Primary Color R', 'path': 'PrimaryColor.R', 'type': 'number'},
        {'header': 'Primary Color G', 'path': 'PrimaryColor.G', 'type': 'number'},
        {'header': 'Primary Color B', 'path': 'PrimaryColor.B', 'type': 'number'},
        {'header': 'Primary Color A', 'path': 'PrimaryColor.A', 'type': 'number'},
        {'header': 'Secondary Color R', 'path': 'SecondaryColor.R', 'type': 'number'},
        {'header': 'Secondary Color G', 'path': 'SecondaryColor.G', 'type': 'number'},
        {'header': 'Secondary Color B', 'path': 'SecondaryColor.B', 'type': 'number'},
        {'header': 'Secondary Color A', 'path': 'SecondaryColor.A', 'type': 'number'},
        
        # Relationships (comma-separated faction IDs with status)
        {'header': 'Allies', 'path': 'Allies', 'type': 'string'},
        {'header': 'Enemies', 'path': 'Enemies', 'type': 'string'},
        {'header': 'Neutrals', 'path': 'Neutrals', 'type': 'string'},
        
        # Home System
        {'header': 'Home System ID', 'path': 'HomeSystemID', 'type': 'string'},
        {'header': 'Territory Size', 'path': 'TerritorySize', 'type': 'integer'},
        
        # Gameplay
        {'header': 'Starting Reputation', 'path': 'StartingReputation', 'type': 'integer'},
        {'header': 'Trade Modifier', 'path': 'TradeModifier', 'type': 'number'},
        {'header': 'Mission Frequency', 'path': 'MissionFrequency', 'type': 'number'},
    ]


def get_personnel_columns() -> List[Dict[str, str]]:
    """Get column definitions for personnel data"""
    return [
        # Basic Info
        {'header': 'Personnel ID', 'path': 'PersonnelID', 'type': 'string'},
        {'header': 'Display Name', 'path': 'DisplayName', 'type': 'string'},
        {'header': 'Full Name', 'path': 'FullName', 'type': 'string'},
        {'header': 'Title', 'path': 'Title', 'type': 'string'},
        {'header': 'Description', 'path': 'Description', 'type': 'string'},
        {'header': 'Biography', 'path': 'Biography', 'type': 'string'},
        
        # Core Attributes
        {'header': 'Role', 'path': 'Role', 'type': 'string'},
        {'header': 'Specialization', 'path': 'Specialization', 'type': 'string'},
        {'header': 'Experience Level', 'path': 'ExperienceLevel', 'type': 'integer'},
        {'header': 'Skill Points', 'path': 'SkillPoints', 'type': 'integer'},
        
        # Skills (0-100 scale)
        {'header': 'Piloting Skill', 'path': 'Skills.Piloting', 'type': 'integer'},
        {'header': 'Gunnery Skill', 'path': 'Skills.Gunnery', 'type': 'integer'},
        {'header': 'Engineering Skill', 'path': 'Skills.Engineering', 'type': 'integer'},
        {'header': 'Science Skill', 'path': 'Skills.Science', 'type': 'integer'},
        {'header': 'Medical Skill', 'path': 'Skills.Medical', 'type': 'integer'},
        {'header': 'Diplomacy Skill', 'path': 'Skills.Diplomacy', 'type': 'integer'},
        {'header': 'Trade Skill', 'path': 'Skills.Trade', 'type': 'integer'},
        {'header': 'Security Skill', 'path': 'Skills.Security', 'type': 'integer'},
        
        # Stats
        {'header': 'Health', 'path': 'Stats.Health', 'type': 'integer'},
        {'header': 'Stamina', 'path': 'Stats.Stamina', 'type': 'integer'},
        {'header': 'Morale', 'path': 'Stats.Morale', 'type': 'integer'},
        {'header': 'Loyalty', 'path': 'Stats.Loyalty', 'type': 'integer'},
        
        # Faction & Location
        {'header': 'Faction ID', 'path': 'FactionID', 'type': 'string'},
        {'header': 'Home Station ID', 'path': 'HomeStationID', 'type': 'string'},
        {'header': 'Current Station ID', 'path': 'CurrentStationID', 'type': 'string'},
        {'header': 'Current Ship ID', 'path': 'CurrentShipID', 'type': 'string'},
        
        # Economic
        {'header': 'Salary', 'path': 'Salary', 'type': 'number'},
        {'header': 'Hire Cost', 'path': 'HireCost', 'type': 'number'},
        {'header': 'Contract Length', 'path': 'ContractLength', 'type': 'integer'},
        
        # Traits (comma-separated)
        {'header': 'Personality Traits', 'path': 'PersonalityTraits', 'type': 'string'},
        {'header': 'Professional Traits', 'path': 'ProfessionalTraits', 'type': 'string'},
        
        # Availability
        {'header': 'Is Available', 'path': 'bIsAvailable', 'type': 'boolean'},
        {'header': 'Is Unique', 'path': 'bIsUnique', 'type': 'boolean'},
        {'header': 'Is Story Character', 'path': 'bIsStoryCharacter', 'type': 'boolean'},
        
        # Visual
        {'header': 'Portrait Path', 'path': 'PortraitPath', 'type': 'string'},
        {'header': 'Voice Set ID', 'path': 'VoiceSetID', 'type': 'string'},
    ]


def get_asset_configs() -> Dict[str, Dict[str, Any]]:
    """
    Get configuration for all supported asset types

    Returns:
        Dictionary of asset type configurations
    """
    return {
        'spaceship': {
            'schema_name': 'spaceship',
            'template_dir': 'Assets/SpaceshipTemplates',
            'content_path': '/Game/Spaceships/DataAssets',
            'id_field': 'BasicInfo.ShipID',
            'name_field': 'BasicInfo.ShipName',
            'sheet_name': 'Spaceships',
            'columns': get_spaceship_columns()
        },
        'trade_item': {
            'schema_name': 'trade_item',
            'template_dir': 'Assets/TradingTemplates',
            'content_path': '/Game/Trading/DataAssets',
            'id_field': 'ItemID',
            'name_field': 'ItemName',
            'sheet_name': 'TradeItems',
            'columns': get_trade_item_columns()
        },
        'faction': {
            'schema_name': 'faction',
            'template_dir': 'Assets/FactionTemplates',
            'content_path': '/Game/Factions/DataAssets',
            'id_field': 'FactionID',
            'name_field': 'FactionName',
            'sheet_name': 'Factions',
            'columns': get_faction_columns()
        },
        'personnel': {
            'schema_name': 'personnel',
            'template_dir': 'Assets/PersonnelTemplates',
            'content_path': '/Game/Personnel/DataAssets',
            'id_field': 'PersonnelID',
            'name_field': 'DisplayName',
            'sheet_name': 'Personnel',
            'columns': get_personnel_columns()
        }
    }
