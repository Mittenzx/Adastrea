#include "Ships/SpaceshipDataAsset.h"

USpaceshipDataAsset::USpaceshipDataAsset()
{
    // Set default values for basic info
    ShipName = FText::FromString(TEXT("Unknown Vessel"));
    Description = FText::FromString(TEXT("A spaceship operating in the galaxy."));
    ShipClass = FText::FromString(TEXT("Unclassified"));
    ShipID = FName(TEXT("UnknownShip"));

    // Core stats defaults
    HullStrength = 1000.0f;
    CargoCapacity = 100.0f;
    CrewRequired = 5;
    MaxCrew = 10;
    ModularPoints = 5;

    // Combat stats defaults
    ArmorRating = 50.0f;
    ShieldStrength = 500.0f;
    ShieldRechargeRate = 10.0f;
    WeaponSlots = 2;
    WeaponPowerCapacity = 1000.0f;
    PointDefenseRating = 5.0f;

    // Mobility stats defaults
    MaxSpeed = 500.0f;
    Acceleration = 50.0f;
    Maneuverability = 5;
    JumpRange = 10.0f;
    FuelCapacity = 1000.0f;
    FuelConsumptionRate = 100.0f;

    // Utility stats defaults
    SensorRange = 5000.0f;
    SensorResolution = 5;
    StealthRating = 5;
    RepairSystemRating = 3;
    ScienceRating = 3;
    MedicalRating = 3;

    // Operational stats defaults
    PowerCapacity = 5000.0f;
    LifeSupportRating = 5;
    MaintenanceLevel = 5;
    HangarCapacity = 0;
    DroneCapacity = 5;
    AISystemRating = 5;

    // Advanced stats defaults
    DiplomacyRating = 0;
    BoardingCapability = 0;
    CustomizableSlots = 3;
    EWarRating = 0;
    MiningRating = 0;

    // Lore defaults
    Manufacturer = FText::FromString(TEXT("Generic Shipyards"));
    YearBuilt = 2500;
    RarityTier = FText::FromString(TEXT("Common"));
    LoreNotes = FText::FromString(TEXT("A standard vessel with no particular distinguishing features."));
    PrimaryColor = FLinearColor::White;
    SecondaryColor = FLinearColor::Gray;
}

float USpaceshipDataAsset::GetCombatRating() const
{
    // Define rating calculation constants for maintainability
    static constexpr float kArmorDivisor = 10.0f;
    static constexpr float kShieldDivisor = 1000.0f;
    static constexpr float kWeaponDivisor = 500.0f;
    static constexpr float kPDMultiplier = 10.0f;
    
    // Weight factors (must sum to 1.0)
    static constexpr float kArmorWeight = 0.2f;
    static constexpr float kShieldWeight = 0.3f;
    static constexpr float kWeaponWeight = 0.4f;
    static constexpr float kPDWeight = 0.1f;
    
    static constexpr float kMinRating = 0.0f;
    static constexpr float kMaxRating = 100.0f;

    // Calculate combat effectiveness based on multiple factors
    const float ArmorScore = FMath::Clamp(ArmorRating / kArmorDivisor, kMinRating, kMaxRating);
    const float ShieldScore = FMath::Clamp(ShieldStrength / kShieldDivisor, kMinRating, kMaxRating);
    const float WeaponScore = FMath::Clamp((WeaponSlots * WeaponPowerCapacity) / kWeaponDivisor, kMinRating, kMaxRating);
    const float PDScore = FMath::Clamp(PointDefenseRating * kPDMultiplier, kMinRating, kMaxRating);

    // Calculate weighted average
    const float CombatRating = (ArmorScore * kArmorWeight) + 
                               (ShieldScore * kShieldWeight) + 
                               (WeaponScore * kWeaponWeight) + 
                               (PDScore * kPDWeight);
    
    return FMath::Clamp(CombatRating, kMinRating, kMaxRating);
}

float USpaceshipDataAsset::GetMobilityRating() const
{
    // Define rating calculation constants
    static constexpr float kSpeedDivisor = 100.0f;
    static constexpr float kAccelDivisor = 10.0f;
    static constexpr float kManeuverMultiplier = 10.0f;
    static constexpr float kJumpMultiplier = 10.0f;
    
    // Weight factors (must sum to 1.0)
    static constexpr float kSpeedWeight = 0.3f;
    static constexpr float kAccelWeight = 0.2f;
    static constexpr float kManeuverWeight = 0.3f;
    static constexpr float kJumpWeight = 0.2f;
    
    static constexpr float kMinRating = 0.0f;
    static constexpr float kMaxRating = 100.0f;

    // Calculate mobility based on speed, acceleration, maneuverability, and jump range
    const float SpeedScore = FMath::Clamp(MaxSpeed / kSpeedDivisor, kMinRating, kMaxRating);
    const float AccelScore = FMath::Clamp(Acceleration / kAccelDivisor, kMinRating, kMaxRating);
    const float ManeuverScore = FMath::Clamp(Maneuverability * kManeuverMultiplier, kMinRating, kMaxRating);
    const float JumpScore = FMath::Clamp(JumpRange * kJumpMultiplier, kMinRating, kMaxRating);

    // Calculate weighted average
    const float MobilityRating = (SpeedScore * kSpeedWeight) + 
                                 (AccelScore * kAccelWeight) + 
                                 (ManeuverScore * kManeuverWeight) + 
                                 (JumpScore * kJumpWeight);
    
    return FMath::Clamp(MobilityRating, kMinRating, kMaxRating);
}

float USpaceshipDataAsset::GetUtilityRating() const
{
    // Define rating calculation constants
    static constexpr float kSensorRangeDivisor = 1000.0f;
    static constexpr float kSensorResolutionMultiplier = 5.0f;
    static constexpr float kSystemRatingMultiplier = 10.0f;
    
    // Weight factors (must sum to 1.0)
    static constexpr float kSensorWeight = 0.4f;
    static constexpr float kRepairWeight = 0.2f;
    static constexpr float kScienceWeight = 0.2f;
    static constexpr float kMedicalWeight = 0.2f;
    
    static constexpr float kMinRating = 0.0f;
    static constexpr float kMaxRating = 100.0f;

    // Calculate utility based on sensors, repair, science, and medical systems
    const float SensorScore = FMath::Clamp((SensorRange / kSensorRangeDivisor) + 
                                          (SensorResolution * kSensorResolutionMultiplier), 
                                          kMinRating, kMaxRating);
    const float RepairScore = FMath::Clamp(RepairSystemRating * kSystemRatingMultiplier, kMinRating, kMaxRating);
    const float ScienceScore = FMath::Clamp(ScienceRating * kSystemRatingMultiplier, kMinRating, kMaxRating);
    const float MedicalScore = FMath::Clamp(MedicalRating * kSystemRatingMultiplier, kMinRating, kMaxRating);

    // Calculate weighted average
    const float UtilityRating = (SensorScore * kSensorWeight) + 
                                (RepairScore * kRepairWeight) + 
                                (ScienceScore * kScienceWeight) + 
                                (MedicalScore * kMedicalWeight);
    
    return FMath::Clamp(UtilityRating, kMinRating, kMaxRating);
}

bool USpaceshipDataAsset::IsSuitableForRole(const FString& Role) const
{
    // Input validation
    if (Role.IsEmpty())
    {
        return false;
    }

    // Define role requirement constants
    static constexpr float kCombatRatingThreshold = 60.0f;
    static constexpr float kCargoCapacityTradeThreshold = 500.0f;
    static constexpr float kSensorRangeExplorationThreshold = 10000.0f;
    static constexpr float kJumpRangeExplorationThreshold = 15.0f;
    static constexpr int32 kMiningRatingThreshold = 5;
    static constexpr float kCargoCapacityMiningThreshold = 1000.0f;
    static constexpr int32 kMedicalRatingThreshold = 7;
    static constexpr int32 kScienceRatingThreshold = 7;
    static constexpr int32 kHangarCapacityCarrierThreshold = 5;
    static constexpr int32 kStealthRatingThreshold = 7;
    static constexpr int32 kDiplomacyRatingThreshold = 5;
    static constexpr int32 kMaxCrewCommandThreshold = 100;
    static constexpr float kCombatRatingCommandThreshold = 50.0f;

    // Convert to lowercase for case-insensitive comparison
    const FString LowerRole = Role.ToLower();

    // Use static FName for efficient comparisons
    static const FName CombatRole(TEXT("combat"));
    static const FName MilitaryRole(TEXT("military"));
    static const FName TradeRole(TEXT("trade"));
    static const FName CargoRole(TEXT("cargo"));
    static const FName ExplorationRole(TEXT("exploration"));
    static const FName ScoutRole(TEXT("scout"));
    static const FName MiningRole(TEXT("mining"));
    static const FName MedicalRole(TEXT("medical"));
    static const FName HospitalRole(TEXT("hospital"));
    static const FName ScienceRole(TEXT("science"));
    static const FName ResearchRole(TEXT("research"));
    static const FName CarrierRole(TEXT("carrier"));
    static const FName StealthRole(TEXT("stealth"));
    static const FName InfiltrationRole(TEXT("infiltration"));
    static const FName CommandRole(TEXT("command"));
    static const FName FlagshipRole(TEXT("flagship"));

    const FName RoleName(*LowerRole);

    // Combat/Military roles
    if (RoleName == CombatRole || RoleName == MilitaryRole)
    {
        return GetCombatRating() >= kCombatRatingThreshold;
    }
    // Trade/Cargo roles
    else if (RoleName == TradeRole || RoleName == CargoRole)
    {
        return CargoCapacity >= kCargoCapacityTradeThreshold;
    }
    // Exploration/Scout roles
    else if (RoleName == ExplorationRole || RoleName == ScoutRole)
    {
        return (SensorRange >= kSensorRangeExplorationThreshold && 
                JumpRange >= kJumpRangeExplorationThreshold);
    }
    // Mining role
    else if (RoleName == MiningRole)
    {
        return (MiningRating >= kMiningRatingThreshold && 
                CargoCapacity >= kCargoCapacityMiningThreshold);
    }
    // Medical/Hospital roles
    else if (RoleName == MedicalRole || RoleName == HospitalRole)
    {
        return MedicalRating >= kMedicalRatingThreshold;
    }
    // Science/Research roles
    else if (RoleName == ScienceRole || RoleName == ResearchRole)
    {
        return ScienceRating >= kScienceRatingThreshold;
    }
    // Carrier role
    else if (RoleName == CarrierRole)
    {
        return HangarCapacity >= kHangarCapacityCarrierThreshold;
    }
    // Stealth/Infiltration roles
    else if (RoleName == StealthRole || RoleName == InfiltrationRole)
    {
        return StealthRating >= kStealthRatingThreshold;
    }
    // Command/Flagship roles
    else if (RoleName == CommandRole || RoleName == FlagshipRole)
    {
        return (DiplomacyRating >= kDiplomacyRatingThreshold && 
                MaxCrew >= kMaxCrewCommandThreshold && 
                GetCombatRating() >= kCombatRatingCommandThreshold);
    }

    // Unknown role or doesn't meet any criteria
    return false;
}

FString USpaceshipDataAsset::GetSizeCategory() const
{
    // Define ship size category thresholds
    static constexpr float kHullStrengthDivisor = 1000.0f;
    static constexpr float kMaxCrewDivisor = 10.0f;
    static constexpr float kFighterThreshold = 5.0f;
    static constexpr float kCorvetteThreshold = 15.0f;
    static constexpr float kFrigateThreshold = 40.0f;
    static constexpr float kCruiserThreshold = 100.0f;
    static constexpr float kBattleshipThreshold = 250.0f;

    // Categorize ship size based on hull strength and crew capacity
    const float SizeScore = (HullStrength / kHullStrengthDivisor) + (MaxCrew / kMaxCrewDivisor);

    if (SizeScore <= kFighterThreshold)
    {
        return TEXT("Fighter");
    }
    else if (SizeScore <= kCorvetteThreshold)
    {
        return TEXT("Corvette");
    }
    else if (SizeScore <= kFrigateThreshold)
    {
        return TEXT("Frigate");
    }
    else if (SizeScore <= kCruiserThreshold)
    {
        return TEXT("Cruiser");
    }
    else if (SizeScore <= kBattleshipThreshold)
    {
        return TEXT("Battleship");
    }
    else
    {
        return TEXT("Capital Ship");
    }
}

float USpaceshipDataAsset::GetOperationalCost() const
{
    // Define operational cost constants
    static constexpr float kCreditsPerCrewPerDay = 10.0f;
    static constexpr float kHullMaintenanceMultiplier = 0.01f;
    static constexpr float kPowerUpkeepMultiplier = 0.005f;

    // Calculate daily operational cost based on crew, maintenance, and power
    const float CrewCost = CrewRequired * kCreditsPerCrewPerDay;
    const float MaintenanceCost = HullStrength * kHullMaintenanceMultiplier * MaintenanceLevel;
    const float PowerCost = PowerCapacity * kPowerUpkeepMultiplier;
    
    return CrewCost + MaintenanceCost + PowerCost;
}
