#include "Ships/SpaceshipDataAsset.h"
#include "Misc/DataValidation.h"
#include "AdastreaLog.h"

USpaceshipDataAsset::USpaceshipDataAsset()
{
    // Phase 2: Initialize cache variables
    bRatingsCacheDirty = true;
    CachedCombatRating = 0.0f;
    CachedMobilityRating = 0.0f;
    CachedUtilityRating = 0.0f;

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
    RotationRateMultiplier = 0.5f;  // Medium agility by default
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
    // Phase 2: Use cached value if available
    if (bRatingsCacheDirty)
    {
        UpdateRatingsCache();
    }
    return CachedCombatRating;
}

float USpaceshipDataAsset::CalculateCombatRatingInternal() const
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
    // Phase 2: Use cached value if available
    if (bRatingsCacheDirty)
    {
        UpdateRatingsCache();
    }
    return CachedMobilityRating;
}

float USpaceshipDataAsset::CalculateMobilityRatingInternal() const
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
    // Phase 2: Use cached value if available
    if (bRatingsCacheDirty)
    {
        UpdateRatingsCache();
    }
    return CachedUtilityRating;
}

float USpaceshipDataAsset::CalculateUtilityRatingInternal() const
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

// ====================
// Phase 2: Calculation Caching Implementation
// ====================

void USpaceshipDataAsset::InvalidateRatingsCache()
{
    bRatingsCacheDirty = true;
}

void USpaceshipDataAsset::UpdateRatingsCache() const
{
    // Calculate all ratings at once for efficiency
    CachedCombatRating = CalculateCombatRatingInternal();
    CachedMobilityRating = CalculateMobilityRatingInternal();
    CachedUtilityRating = CalculateUtilityRatingInternal();
    
    bRatingsCacheDirty = false;
    
    UE_LOG(LogAdastrea, Verbose, TEXT("SpaceshipDataAsset: Updated ratings cache for %s (Combat: %.1f, Mobility: %.1f, Utility: %.1f)"),
        *ShipName.ToString(), CachedCombatRating, CachedMobilityRating, CachedUtilityRating);
}

#if WITH_EDITOR
EDataValidationResult USpaceshipDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult Result = Super::IsDataValid(Context);

    // Validate basic info
    if (ShipName.IsEmpty())
    {
        Context.AddError(FText::FromString(TEXT("Ship Name is empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (ShipClass.IsEmpty())
    {
        Context.AddError(FText::FromString(TEXT("Ship Class is empty")));
        Result = EDataValidationResult::Invalid;
    }

    if (ShipID.IsNone())
    {
        Context.AddError(FText::FromString(TEXT("Ship ID is not set")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate core stats
    if (HullStrength <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Hull Strength must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (CrewRequired < 1)
    {
        Context.AddError(FText::FromString(TEXT("Crew Required must be at least 1")));
        Result = EDataValidationResult::Invalid;
    }

    if (MaxCrew < CrewRequired)
    {
        Context.AddError(FText::Format(
            FText::FromString(TEXT("Max Crew ({0}) must be greater than or equal to Crew Required ({1})")),
            FText::AsNumber(MaxCrew), FText::AsNumber(CrewRequired)
        ));
        Result = EDataValidationResult::Invalid;
    }

    // Validate mobility stats
    if (MaxSpeed <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Max Speed must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    if (Acceleration <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Acceleration must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate fuel economy
    if (FuelCapacity > 0.0f && FuelConsumptionRate <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Fuel Consumption Rate must be greater than 0 if Fuel Capacity is set")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate shield stats
    if (ShieldStrength > 0.0f && ShieldRechargeRate < 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Shield Recharge Rate cannot be negative if Shield Strength is set")));
        Result = EDataValidationResult::Invalid;
    }

    // Validate operational stats
    if (PowerCapacity <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("Power Capacity must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }

    // Warn about potential balance issues
    float CombatRating = GetCombatRating();
    if (CombatRating > 100.0f)
    {
        Context.AddWarning(FText::Format(
            FText::FromString(TEXT("Combat Rating is very high ({0}). Consider rebalancing stats.")),
            FText::AsNumber(CombatRating)
        ));
        // Don't mark as invalid, just a warning
    }

    return Result;
}

void USpaceshipDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Invalidate cache when any property changes in editor
    if (PropertyChangedEvent.Property)
    {
        // Invalidate cache for any property change to be safe
        InvalidateRatingsCache();
        FName PropertyName = PropertyChangedEvent.Property->GetFName();
        UE_LOG(LogAdastrea, Verbose, TEXT("SpaceshipDataAsset: Property %s changed, invalidating ratings cache for %s"),
            *PropertyName.ToString(), *ShipName.ToString());
    }
    
    // Validate properties to catch configuration errors at edit-time
    // Helps avoid invalid ship configurations and improves designer experience
    ValidateShipProperties();
}

void USpaceshipDataAsset::ValidateShipProperties()
{
    // Use the DataAssetValidation utilities for consistent validation
    // More comprehensive validation is handled by IsDataValid()
    
    // Validate and auto-correct hull strength to be positive
    if (HullStrength <= 0.0f)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("SpaceshipDataAsset [%s]: HullStrength (%.2f) should be positive! Auto-correcting to 1.0."), 
            *ShipName.ToString(), HullStrength);
        HullStrength = 1.0f;
    }
    
    // Validate and auto-correct cargo capacity to be non-negative
    if (CargoCapacity < 0.0f)
    {
        UE_LOG(LogAdastrea, Warning, TEXT("SpaceshipDataAsset [%s]: CargoCapacity (%.2f) cannot be negative! Auto-correcting to 0.0."), 
            *ShipName.ToString(), CargoCapacity);
        CargoCapacity = 0.0f;
    }
    
    // Validate ship name is set and provide default if empty
    if (ShipName.IsEmpty())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("SpaceshipDataAsset: ShipName is empty! Auto-assigning default name 'Unnamed Ship'."));
        ShipName = FText::FromString(TEXT("Unnamed Ship"));
    }
}
#endif
