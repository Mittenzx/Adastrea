#include "Combat/WeaponDataAsset.h"
#include "AdastreaLog.h"

UWeaponDataAsset::UWeaponDataAsset()
{
    // Basic info defaults
    WeaponName = FText::FromString(TEXT("Generic Weapon"));
    Description = FText::FromString(TEXT("A standard weapon system."));
    WeaponID = FName(TEXT("GenericWeapon"));
    WeaponType = EWeaponType::Energy;
    DamageType = EDamageType::Energy;
    MountSize = EWeaponMountSize::Small;

    // Damage stats defaults
    BaseDamage = 100.0f;
    ArmorPenetration = 50.0f;
    ShieldPenetration = 50.0f;
    CriticalChance = 0.05f;
    CriticalMultiplier = 2.0f;

    // Firing characteristics defaults
    RateOfFire = 1.0f;
    MaxRange = 5000.0f;
    OptimalRange = 2500.0f;
    ProjectileSpeed = 1000.0f;
    TrackingSpeed = 45.0f;
    Accuracy = 75.0f;

    // Resource requirements defaults
    PowerPerShot = 50.0f;
    AmmoCapacity = 0;  // Energy weapons have no ammo
    ReloadTime = 0.0f;
    HeatGeneration = 10.0f;
    CoolingRate = 5.0f;

    // Special properties defaults
    bPointDefenseCapable = false;
    AOERadius = 0.0f;
    EMPStrength = 0.0f;
    HomingRating = 0;

    // Lore defaults
    Manufacturer = FText::FromString(TEXT("Generic Arms"));
    TechLevel = 3;
    RarityTier = FText::FromString(TEXT("Common"));
}

float UWeaponDataAsset::GetDPS() const
{
    return BaseDamage * RateOfFire;
}

float UWeaponDataAsset::GetEffectiveDamage(float ArmorRating, float ShieldStrength) const
{
    float EffectiveDamage = BaseDamage;

    // Apply shield penetration if target has shields
    if (ShieldStrength > 0.0f)
    {
        const float ShieldReduction = FMath::Clamp(1.0f - (ShieldPenetration / 100.0f), 0.0f, 1.0f);
        EffectiveDamage *= (1.0f - ShieldReduction * 0.5f);  // Shields reduce damage by up to 50%
    }
    else
    {
        // Apply armor penetration only if shields are down
        const float ArmorReduction = FMath::Max(0.0f, ArmorRating - ArmorPenetration);
        const float DamageMultiplier = 1.0f / (1.0f + (ArmorReduction / 100.0f));
        EffectiveDamage *= DamageMultiplier;
    }

    return EffectiveDamage;
}

float UWeaponDataAsset::GetTimeToKill(float TargetHealth) const
{
    const float DPS = GetDPS();
    
    if (DPS <= 0.0f)
    {
        return -1.0f;  // Invalid: cannot kill target
    }

    return TargetHealth / DPS;
}

bool UWeaponDataAsset::IsSuitableForRole(const FString& Role) const
{
    if (Role.IsEmpty())
    {
        return false;
    }

    const FString LowerRole = Role.ToLower();

    // Anti-ship: High damage, good range
    if (LowerRole.Contains(TEXT("antiship")) || LowerRole.Contains(TEXT("anti-ship")))
    {
        return BaseDamage >= 200.0f && MaxRange >= 5000.0f;
    }
    // Anti-missile/Point defense: High rate of fire, point defense capable
    else if (LowerRole.Contains(TEXT("antimissile")) || LowerRole.Contains(TEXT("anti-missile")) || 
             LowerRole.Contains(TEXT("pointdefense")) || LowerRole.Contains(TEXT("point-defense")))
    {
        return bPointDefenseCapable && RateOfFire >= 5.0f;
    }
    // Anti-armor: High armor penetration
    else if (LowerRole.Contains(TEXT("antiarmor")) || LowerRole.Contains(TEXT("anti-armor")))
    {
        return ArmorPenetration >= 70.0f;
    }
    // Anti-shield: High shield penetration
    else if (LowerRole.Contains(TEXT("antishield")) || LowerRole.Contains(TEXT("anti-shield")))
    {
        return ShieldPenetration >= 70.0f;
    }
    // Close range: Short optimal range, high damage
    else if (LowerRole.Contains(TEXT("closerange")) || LowerRole.Contains(TEXT("close-range")))
    {
        return OptimalRange <= 2000.0f && BaseDamage >= 150.0f;
    }
    // Long range: Extended range, good accuracy
    else if (LowerRole.Contains(TEXT("longrange")) || LowerRole.Contains(TEXT("long-range")))
    {
        return MaxRange >= 10000.0f && Accuracy >= 70.0f;
    }
    // Bombardment: Area of effect capability
    else if (LowerRole.Contains(TEXT("bombardment")) || LowerRole.Contains(TEXT("aoe")))
    {
        return AOERadius >= 100.0f;
    }
    // Disable: EMP capability
    else if (LowerRole.Contains(TEXT("disable")) || LowerRole.Contains(TEXT("emp")))
    {
        return EMPStrength >= 50.0f;
    }

    return false;
}
