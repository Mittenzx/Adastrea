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
    // Calculate combat effectiveness based on multiple factors
    float ArmorScore = FMath::Clamp(ArmorRating / 10.0f, 0.0f, 100.0f);
    float ShieldScore = FMath::Clamp(ShieldStrength / 1000.0f, 0.0f, 100.0f);
    float WeaponScore = FMath::Clamp((WeaponSlots * WeaponPowerCapacity) / 500.0f, 0.0f, 100.0f);
    float PDScore = FMath::Clamp(PointDefenseRating * 10.0f, 0.0f, 100.0f);

    // Weighted average
    float CombatRating = (ArmorScore * 0.2f) + (ShieldScore * 0.3f) + (WeaponScore * 0.4f) + (PDScore * 0.1f);
    
    return FMath::Clamp(CombatRating, 0.0f, 100.0f);
}

float USpaceshipDataAsset::GetMobilityRating() const
{
    // Calculate mobility based on speed, acceleration, maneuverability, and jump range
    float SpeedScore = FMath::Clamp(MaxSpeed / 100.0f, 0.0f, 100.0f);
    float AccelScore = FMath::Clamp(Acceleration / 10.0f, 0.0f, 100.0f);
    float ManeuverScore = FMath::Clamp(Maneuverability * 10.0f, 0.0f, 100.0f);
    float JumpScore = FMath::Clamp(JumpRange * 10.0f, 0.0f, 100.0f);

    // Weighted average
    float MobilityRating = (SpeedScore * 0.3f) + (AccelScore * 0.2f) + (ManeuverScore * 0.3f) + (JumpScore * 0.2f);
    
    return FMath::Clamp(MobilityRating, 0.0f, 100.0f);
}

float USpaceshipDataAsset::GetUtilityRating() const
{
    // Calculate utility based on sensors, repair, science, and medical systems
    float SensorScore = FMath::Clamp((SensorRange / 1000.0f) + (SensorResolution * 5.0f), 0.0f, 100.0f);
    float RepairScore = FMath::Clamp(RepairSystemRating * 10.0f, 0.0f, 100.0f);
    float ScienceScore = FMath::Clamp(ScienceRating * 10.0f, 0.0f, 100.0f);
    float MedicalScore = FMath::Clamp(MedicalRating * 10.0f, 0.0f, 100.0f);

    // Weighted average
    float UtilityRating = (SensorScore * 0.4f) + (RepairScore * 0.2f) + (ScienceScore * 0.2f) + (MedicalScore * 0.2f);
    
    return FMath::Clamp(UtilityRating, 0.0f, 100.0f);
}

bool USpaceshipDataAsset::IsSuitableForRole(const FString& Role) const
{
    FString LowerRole = Role.ToLower();

    if (LowerRole == TEXT("combat") || LowerRole == TEXT("military"))
    {
        return GetCombatRating() >= 60.0f;
    }
    else if (LowerRole == TEXT("trade") || LowerRole == TEXT("cargo"))
    {
        return CargoCapacity >= 500.0f;
    }
    else if (LowerRole == TEXT("exploration") || LowerRole == TEXT("scout"))
    {
        return (SensorRange >= 10000.0f && JumpRange >= 15.0f);
    }
    else if (LowerRole == TEXT("mining"))
    {
        return MiningRating >= 5 && CargoCapacity >= 1000.0f;
    }
    else if (LowerRole == TEXT("medical") || LowerRole == TEXT("hospital"))
    {
        return MedicalRating >= 7;
    }
    else if (LowerRole == TEXT("science") || LowerRole == TEXT("research"))
    {
        return ScienceRating >= 7;
    }
    else if (LowerRole == TEXT("carrier"))
    {
        return HangarCapacity >= 5;
    }
    else if (LowerRole == TEXT("stealth") || LowerRole == TEXT("infiltration"))
    {
        return StealthRating >= 7;
    }
    else if (LowerRole == TEXT("command") || LowerRole == TEXT("flagship"))
    {
        return (DiplomacyRating >= 5 && MaxCrew >= 100 && GetCombatRating() >= 50.0f);
    }

    // Unknown role or doesn't meet any criteria
    return false;
}

FString USpaceshipDataAsset::GetSizeCategory() const
{
    // Categorize ship size based on hull strength and crew capacity
    float SizeScore = (HullStrength / 1000.0f) + (MaxCrew / 10.0f);

    if (SizeScore <= 5.0f)
    {
        return TEXT("Fighter");
    }
    else if (SizeScore <= 15.0f)
    {
        return TEXT("Corvette");
    }
    else if (SizeScore <= 40.0f)
    {
        return TEXT("Frigate");
    }
    else if (SizeScore <= 100.0f)
    {
        return TEXT("Cruiser");
    }
    else if (SizeScore <= 250.0f)
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
    // Calculate daily operational cost based on crew, maintenance, and power
    float CrewCost = CrewRequired * 10.0f; // 10 credits per crew member per day
    float MaintenanceCost = HullStrength * 0.01f * MaintenanceLevel; // Hull size affects maintenance
    float PowerCost = PowerCapacity * 0.005f; // Power generation upkeep
    
    return CrewCost + MaintenanceCost + PowerCost;
}
