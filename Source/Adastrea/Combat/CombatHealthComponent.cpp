#include "Combat/CombatHealthComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"

UCombatHealthComponent::UCombatHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default configuration
    MaxHullStrength = 1000.0f;
    MaxShieldStrength = 500.0f;
    ShieldRechargeRate = 10.0f;
    ShieldRechargeDelay = 5.0f;
    ArmorRating = 50.0f;
    bUseDirectionalShields = false;

    // Default shield distribution
    ForwardShieldPercentage = 40.0f;
    AftShieldPercentage = 20.0f;
    PortShieldPercentage = 20.0f;
    StarboardShieldPercentage = 20.0f;

    // Default state
    CurrentHullStrength = MaxHullStrength;
    CurrentShieldStrength = MaxShieldStrength;
    bIsDestroyed = false;
    bShieldsOverloaded = false;
    TimeSinceLastDamage = 0.0f;
    bCriticalDamageEventFired = false;
}

void UCombatHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize to max values
    CurrentHullStrength = MaxHullStrength;
    CurrentShieldStrength = MaxShieldStrength;

    // Initialize shield facings
    if (bUseDirectionalShields)
    {
        InitializeShieldFacings();
    }

    // Register for damage events
    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->OnTakeAnyDamage.AddDynamic(this, &UCombatHealthComponent::HandleTakeAnyDamage);
    }
}

void UCombatHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDestroyed)
    {
        return;
    }

    // Update shield recharge
    TimeSinceLastDamage += DeltaTime;
    UpdateShieldRecharge(DeltaTime);
}

void UCombatHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float DamageAmount, 
    const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (bIsDestroyed || DamageAmount <= 0.0f)
    {
        return;
    }

    // Default damage handling - use default penetration values
    const FVector HitDirection = DamageCauser ? 
        (DamagedActor->GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal() : 
        FVector::ForwardVector;

    ApplyTypedDamage(DamageAmount, EDamageType::Kinetic, 50.0f, 50.0f, HitDirection, DamageCauser);
}

float UCombatHealthComponent::ApplyTypedDamage(float BaseDamage, EDamageType DamageType, 
    float ArmorPenetration, float ShieldPenetration, FVector HitDirection, AActor* DamageCauser)
{
    if (bIsDestroyed || BaseDamage <= 0.0f)
    {
        return 0.0f;
    }

    float RemainingDamage = BaseDamage;
    TimeSinceLastDamage = 0.0f;

    // Try to absorb damage with shields first
    if (HasActiveShields())
    {
        if (bUseDirectionalShields)
        {
            // Apply to specific facing
            const EShieldFacing HitFacing = GetHitFacing(HitDirection);
            const float ShieldDamage = ApplyDamageToShieldFacing(HitFacing, RemainingDamage, ShieldPenetration);
            RemainingDamage -= ShieldDamage;
            
            if (ShieldDamage > 0.0f)
            {
                OnShieldDamaged(ShieldDamage, HitFacing, DamageCauser);
            }
        }
        else
        {
            // Omnidirectional shields
            const float ShieldEfficiency = 1.0f - (ShieldPenetration / 100.0f);
            const float ShieldDamage = FMath::Min(RemainingDamage * ShieldEfficiency, CurrentShieldStrength);
            CurrentShieldStrength -= ShieldDamage;
            RemainingDamage -= ShieldDamage;

            if (CurrentShieldStrength <= 0.0f)
            {
                CurrentShieldStrength = 0.0f;
                bShieldsOverloaded = true;
                OnShieldsDepleted(EShieldFacing::Omnidirectional);
            }

            if (ShieldDamage > 0.0f)
            {
                OnShieldDamaged(ShieldDamage, EShieldFacing::Omnidirectional, DamageCauser);
            }
        }
    }

    // Apply remaining damage to hull (with armor reduction)
    if (RemainingDamage > 0.0f)
    {
        const float ArmoredDamage = CalculateArmorReduction(RemainingDamage, ArmorPenetration);
        CurrentHullStrength -= ArmoredDamage;

        OnHullDamaged(ArmoredDamage, DamageCauser);

        // Check for critical damage
        if (!bCriticalDamageEventFired && IsCriticallyDamaged())
        {
            bCriticalDamageEventFired = true;
            OnCriticalDamage();
        }

        // Check for destruction
        if (CurrentHullStrength <= 0.0f)
        {
            CurrentHullStrength = 0.0f;
            DestroyShip();
            
            AController* Killer = DamageCauser ? DamageCauser->GetInstigatorController() : nullptr;
            OnShipDestroyed(Killer);
        }

        return ArmoredDamage;
    }

    return BaseDamage - RemainingDamage;
}

float UCombatHealthComponent::RestoreHull(float Amount)
{
    if (bIsDestroyed || Amount <= 0.0f)
    {
        return 0.0f;
    }

    const float OldHull = CurrentHullStrength;
    CurrentHullStrength = FMath::Min(CurrentHullStrength + Amount, MaxHullStrength);
    const float ActualHealed = CurrentHullStrength - OldHull;

    // Clear critical damage flag if healed above threshold
    if (bCriticalDamageEventFired && !IsCriticallyDamaged())
    {
        bCriticalDamageEventFired = false;
    }

    return ActualHealed;
}

float UCombatHealthComponent::RestoreShields(float Amount, EShieldFacing Facing)
{
    if (Amount <= 0.0f)
    {
        return 0.0f;
    }

    float TotalRestored = 0.0f;

    if (bUseDirectionalShields)
    {
        if (Facing == EShieldFacing::Omnidirectional)
        {
            // Restore all facings proportionally
            for (auto& Pair : ShieldFacings)
            {
                FShieldFacingData& FacingData = Pair.Value;
                const float OldStrength = FacingData.CurrentStrength;
                FacingData.CurrentStrength = FMath::Min(FacingData.CurrentStrength + Amount, FacingData.MaxStrength);
                TotalRestored += (FacingData.CurrentStrength - OldStrength);
            }
        }
        else
        {
            // Restore specific facing
            if (ShieldFacings.Contains(Facing))
            {
                FShieldFacingData& FacingData = ShieldFacings[Facing];
                const float OldStrength = FacingData.CurrentStrength;
                FacingData.CurrentStrength = FMath::Min(FacingData.CurrentStrength + Amount, FacingData.MaxStrength);
                TotalRestored = FacingData.CurrentStrength - OldStrength;
            }
        }

        // Update total shield strength
        CurrentShieldStrength = 0.0f;
        for (const auto& Pair : ShieldFacings)
        {
            CurrentShieldStrength += Pair.Value.CurrentStrength;
        }
    }
    else
    {
        // Omnidirectional shields
        const float OldShield = CurrentShieldStrength;
        CurrentShieldStrength = FMath::Min(CurrentShieldStrength + Amount, MaxShieldStrength);
        TotalRestored = CurrentShieldStrength - OldShield;
    }

    // Clear overload flag if shields restored
    if (CurrentShieldStrength > 0.0f)
    {
        bShieldsOverloaded = false;
    }

    return TotalRestored;
}

void UCombatHealthComponent::DestroyShip()
{
    if (bIsDestroyed)
    {
        return;
    }

    bIsDestroyed = true;
    CurrentHullStrength = 0.0f;
    CurrentShieldStrength = 0.0f;

    UE_LOG(LogAdastrea, Warning, TEXT("Ship destroyed: %s"), *GetOwner()->GetName());
}

float UCombatHealthComponent::GetHullPercentage() const
{
    return MaxHullStrength > 0.0f ? CurrentHullStrength / MaxHullStrength : 0.0f;
}

float UCombatHealthComponent::GetShieldPercentage() const
{
    return MaxShieldStrength > 0.0f ? CurrentShieldStrength / MaxShieldStrength : 0.0f;
}

float UCombatHealthComponent::GetShieldFacingPercentage(EShieldFacing Facing) const
{
    if (!bUseDirectionalShields || !ShieldFacings.Contains(Facing))
    {
        return GetShieldPercentage();
    }

    const FShieldFacingData& FacingData = ShieldFacings[Facing];
    return FacingData.MaxStrength > 0.0f ? FacingData.CurrentStrength / FacingData.MaxStrength : 0.0f;
}

bool UCombatHealthComponent::HasActiveShields() const
{
    return CurrentShieldStrength > 0.0f && !bShieldsOverloaded;
}

bool UCombatHealthComponent::IsCriticallyDamaged() const
{
    return GetHullPercentage() < 0.25f;
}

EShieldFacing UCombatHealthComponent::GetHitFacing(FVector HitDirection) const
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return EShieldFacing::Forward;
    }

    // Normalize hit direction
    HitDirection.Normalize();

    // Transform to local space
    const FVector Forward = Owner->GetActorForwardVector();
    const FVector Right = Owner->GetActorRightVector();

    const float ForwardDot = FVector::DotProduct(HitDirection, Forward);
    const float RightDot = FVector::DotProduct(HitDirection, Right);

    // Determine facing based on dot products
    const float AbsForward = FMath::Abs(ForwardDot);
    const float AbsRight = FMath::Abs(RightDot);

    if (AbsForward > AbsRight)
    {
        return ForwardDot > 0.0f ? EShieldFacing::Forward : EShieldFacing::Aft;
    }
    else
    {
        return RightDot > 0.0f ? EShieldFacing::Starboard : EShieldFacing::Port;
    }
}

void UCombatHealthComponent::OnHullDamaged_Implementation(float DamageAmount, AActor* DamageCauser)
{
    UE_LOG(LogAdastrea, Log, TEXT("Hull damaged: %.1f (%.1f%% remaining)"), 
        DamageAmount, GetHullPercentage() * 100.0f);
}

void UCombatHealthComponent::OnShieldDamaged_Implementation(float DamageAmount, EShieldFacing Facing, AActor* DamageCauser)
{
    UE_LOG(LogAdastrea, Log, TEXT("Shield damaged: %.1f to %s facing (%.1f%% remaining)"), 
        DamageAmount, *UEnum::GetValueAsString(Facing), GetShieldFacingPercentage(Facing) * 100.0f);
}

void UCombatHealthComponent::OnShieldsDepleted_Implementation(EShieldFacing Facing)
{
    UE_LOG(LogAdastrea, Warning, TEXT("Shields depleted: %s"), *UEnum::GetValueAsString(Facing));
}

void UCombatHealthComponent::OnShieldsRecharged_Implementation(EShieldFacing Facing)
{
    UE_LOG(LogAdastrea, Log, TEXT("Shields recharged: %s"), *UEnum::GetValueAsString(Facing));
}

void UCombatHealthComponent::OnCriticalDamage_Implementation()
{
    UE_LOG(LogAdastrea, Warning, TEXT("Ship critically damaged!"));
}

void UCombatHealthComponent::OnShipDestroyed_Implementation(AController* Killer)
{
    UE_LOG(LogAdastrea, Error, TEXT("Ship destroyed by %s"), 
        Killer ? *Killer->GetName() : TEXT("Unknown"));
}

void UCombatHealthComponent::UpdateShieldRecharge(float DeltaTime)
{
    if (bShieldsOverloaded || MaxShieldStrength <= 0.0f)
    {
        return;
    }

    // Wait for recharge delay
    if (TimeSinceLastDamage < ShieldRechargeDelay)
    {
        return;
    }

    const float RechargeAmount = ShieldRechargeRate * DeltaTime;

    if (bUseDirectionalShields)
    {
        // Recharge all facings proportionally
        for (auto& Pair : ShieldFacings)
        {
            FShieldFacingData& FacingData = Pair.Value;
            
            if (FacingData.CurrentStrength < FacingData.MaxStrength)
            {
                const float OldStrength = FacingData.CurrentStrength;
                FacingData.CurrentStrength = FMath::Min(FacingData.CurrentStrength + RechargeAmount, FacingData.MaxStrength);
                
                // Fire event when facing fully recharged
                if (OldStrength < FacingData.MaxStrength && FacingData.CurrentStrength >= FacingData.MaxStrength)
                {
                    OnShieldsRecharged(Pair.Key);
                }
            }
        }

        // Update total shield strength
        CurrentShieldStrength = 0.0f;
        for (const auto& Pair : ShieldFacings)
        {
            CurrentShieldStrength += Pair.Value.CurrentStrength;
        }
    }
    else
    {
        // Omnidirectional recharge
        if (CurrentShieldStrength < MaxShieldStrength)
        {
            const float OldStrength = CurrentShieldStrength;
            CurrentShieldStrength = FMath::Min(CurrentShieldStrength + RechargeAmount, MaxShieldStrength);
            
            // Fire event when fully recharged
            if (OldStrength < MaxShieldStrength && CurrentShieldStrength >= MaxShieldStrength)
            {
                OnShieldsRecharged(EShieldFacing::Omnidirectional);
            }
        }
    }
}

void UCombatHealthComponent::InitializeShieldFacings()
{
    ShieldFacings.Empty();

    // Calculate max strength for each facing
    const float ForwardMax = MaxShieldStrength * (ForwardShieldPercentage / 100.0f);
    const float AftMax = MaxShieldStrength * (AftShieldPercentage / 100.0f);
    const float PortMax = MaxShieldStrength * (PortShieldPercentage / 100.0f);
    const float StarboardMax = MaxShieldStrength * (StarboardShieldPercentage / 100.0f);

    // Initialize facings
    FShieldFacingData ForwardData;
    ForwardData.MaxStrength = ForwardMax;
    ForwardData.CurrentStrength = ForwardMax;
    ShieldFacings.Add(EShieldFacing::Forward, ForwardData);

    FShieldFacingData AftData;
    AftData.MaxStrength = AftMax;
    AftData.CurrentStrength = AftMax;
    ShieldFacings.Add(EShieldFacing::Aft, AftData);

    FShieldFacingData PortData;
    PortData.MaxStrength = PortMax;
    PortData.CurrentStrength = PortMax;
    ShieldFacings.Add(EShieldFacing::Port, PortData);

    FShieldFacingData StarboardData;
    StarboardData.MaxStrength = StarboardMax;
    StarboardData.CurrentStrength = StarboardMax;
    ShieldFacings.Add(EShieldFacing::Starboard, StarboardData);

    // Update total
    CurrentShieldStrength = ForwardMax + AftMax + PortMax + StarboardMax;
}

float UCombatHealthComponent::CalculateArmorReduction(float Damage, float ArmorPenetration) const
{
    const float EffectiveArmor = FMath::Max(0.0f, ArmorRating - ArmorPenetration);
    const float DamageMultiplier = 1.0f / (1.0f + (EffectiveArmor / 100.0f));
    return Damage * DamageMultiplier;
}

float UCombatHealthComponent::ApplyDamageToShieldFacing(EShieldFacing Facing, float Damage, float ShieldPenetration)
{
    if (!ShieldFacings.Contains(Facing))
    {
        return 0.0f;
    }

    FShieldFacingData& FacingData = ShieldFacings[Facing];
    
    if (FacingData.CurrentStrength <= 0.0f)
    {
        return 0.0f;
    }

    const float ShieldEfficiency = 1.0f - (ShieldPenetration / 100.0f);
    const float ShieldDamage = FMath::Min(Damage * ShieldEfficiency, FacingData.CurrentStrength);
    FacingData.CurrentStrength -= ShieldDamage;

    if (FacingData.CurrentStrength <= 0.0f)
    {
        FacingData.CurrentStrength = 0.0f;
        OnShieldsDepleted(Facing);
    }

    // Update total shield strength
    CurrentShieldStrength = 0.0f;
    for (const auto& Pair : ShieldFacings)
    {
        CurrentShieldStrength += Pair.Value.CurrentStrength;
    }

    if (CurrentShieldStrength <= 0.0f)
    {
        bShieldsOverloaded = true;
    }

    return ShieldDamage;
}
