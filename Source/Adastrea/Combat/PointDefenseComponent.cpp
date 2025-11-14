// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Combat/PointDefenseComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UPointDefenseComponent::UPointDefenseComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_DuringPhysics;

    // Default configuration
    DetectionRange = 5000.0f;
    ScanInterval = 0.25f;
    MaxSimultaneousTargets = 4;
    bAutoEngage = true;
    bPrioritizeMissiles = true;

    // Default tags
    MissileTags.Add(FName(TEXT("Missile")));
    MissileTags.Add(FName(TEXT("Torpedo")));
    FighterTags.Add(FName(TEXT("Fighter")));
    FighterTags.Add(FName(TEXT("Bomber")));

    // Default state
    TimeSinceLastScan = 0.0f;
}

void UPointDefenseComponent::BeginPlay()
{
    Super::BeginPlay();

    // Perform initial scan
    if (IsActive())
    {
        ScanForThreats();
    }
}

void UPointDefenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsActive())
    {
        return;
    }

    TimeSinceLastScan += DeltaTime;

    // Periodic threat scan
    if (TimeSinceLastScan >= ScanInterval)
    {
        ScanForThreats();
        TimeSinceLastScan = 0.0f;
    }

    // Update existing threats
    UpdateThreats();

    // Assign weapons to threats
    if (bAutoEngage)
    {
        AssignWeaponsToThreats();
    }

    // Update weapon firing
    UpdateWeaponFiring(DeltaTime);
}

void UPointDefenseComponent::Activate(bool bReset)
{
    Super::Activate(bReset);
    UE_LOG(LogAdastreaCombat, Log, TEXT("Point defense activated"));
}

void UPointDefenseComponent::Deactivate()
{
    Super::Deactivate();
    ClearAllThreats();
    UE_LOG(LogAdastreaCombat, Log, TEXT("Point defense deactivated"));
}

bool UPointDefenseComponent::AddPointDefenseWeapon(UWeaponComponent* Weapon)
{
    if (!Weapon || !Weapon->WeaponData)
    {
        return false;
    }

    // Check if weapon is point defense capable
    if (!Weapon->WeaponData->bPointDefenseCapable)
    {
        UE_LOG(LogAdastreaCombat, Warning, TEXT("Weapon %s is not point defense capable"), 
            *Weapon->WeaponData->WeaponName.ToString());
        return false;
    }

    if (!PointDefenseWeapons.Contains(Weapon))
    {
        PointDefenseWeapons.Add(Weapon);
        return true;
    }

    return false;
}

void UPointDefenseComponent::RemovePointDefenseWeapon(UWeaponComponent* Weapon)
{
    PointDefenseWeapons.Remove(Weapon);
    WeaponAssignments.Remove(Weapon);
}

bool UPointDefenseComponent::EngageThreat(AActor* Threat)
{
    if (!Threat || !IsValidThreat(Threat))
    {
        return false;
    }

    // Check if already engaged
    if (EngagedThreats.Contains(Threat))
    {
        return true;
    }

    // Check if we can engage more targets
    if (EngagedThreats.Num() >= MaxSimultaneousTargets)
    {
        return false;
    }

    // Find available weapon
    for (UWeaponComponent* Weapon : PointDefenseWeapons)
    {
        if (!Weapon || !Weapon->WeaponData)
        {
            continue;
        }

        // Check if weapon is already assigned
        if (WeaponAssignments.Contains(Weapon))
        {
            continue;
        }

        // Check if weapon can engage this threat
        if (!CanWeaponEngageThreat(Weapon, Threat))
        {
            continue;
        }

        // Assign weapon to threat
        WeaponAssignments.Add(Weapon, Threat);
        EngagedThreats.Add(Threat);
        
        // Mark threat as engaged
        for (FIncomingThreat& ThreatInfo : TrackedThreats)
        {
            if (ThreatInfo.ThreatActor == Threat)
            {
                ThreatInfo.bIsEngaged = true;
                break;
            }
        }

        OnThreatEngaged(Threat, Weapon);
        return true;
    }

    return false;
}

void UPointDefenseComponent::DisengageThreat(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }

    // Remove from engaged list
    EngagedThreats.Remove(Threat);

    // Remove weapon assignments
    TArray<UWeaponComponent*> WeaponsToUnassign;
    for (const auto& Pair : WeaponAssignments)
    {
        if (Pair.Value == Threat)
        {
            WeaponsToUnassign.Add(Pair.Key);
        }
    }

    for (UWeaponComponent* Weapon : WeaponsToUnassign)
    {
        WeaponAssignments.Remove(Weapon);
    }

    // Update tracked threat
    for (FIncomingThreat& ThreatInfo : TrackedThreats)
    {
        if (ThreatInfo.ThreatActor == Threat)
        {
            ThreatInfo.bIsEngaged = false;
            break;
        }
    }
}

void UPointDefenseComponent::ClearAllThreats()
{
    EngagedThreats.Empty();
    WeaponAssignments.Empty();
    TrackedThreats.Empty();
}

int32 UPointDefenseComponent::GetThreatCount() const
{
    return TrackedThreats.Num();
}

int32 UPointDefenseComponent::GetEngagedThreatCount() const
{
    return EngagedThreats.Num();
}

AActor* UPointDefenseComponent::GetHighestPriorityThreat() const
{
    float HighestPriority = -1.0f;
    AActor* MostDangerous = nullptr;

    for (const FIncomingThreat& ThreatInfo : TrackedThreats)
    {
        // Skip engaged threats
        if (ThreatInfo.bIsEngaged)
        {
            continue;
        }

        if (ThreatInfo.Priority > HighestPriority)
        {
            HighestPriority = ThreatInfo.Priority;
            MostDangerous = ThreatInfo.ThreatActor;
        }
    }

    return MostDangerous;
}

float UPointDefenseComponent::CalculateInterceptionProbability(AActor* Threat) const
{
    if (!Threat)
    {
        return 0.0f;
    }

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 0.0f;
    }

    // Base probability
    float Probability = 0.5f;

    // Distance factor (closer = higher probability)
    const float Distance = FVector::Dist(Owner->GetActorLocation(), Threat->GetActorLocation());
    const float DistanceFactor = 1.0f - FMath::Clamp(Distance / DetectionRange, 0.0f, 1.0f);
    Probability += DistanceFactor * 0.3f;

    // Number of weapons engaging (more = higher probability)
    int32 EngagedWeapons = 0;
    for (const auto& Pair : WeaponAssignments)
    {
        if (Pair.Value == Threat)
        {
            EngagedWeapons++;
        }
    }
    Probability += FMath::Min(EngagedWeapons * 0.15f, 0.3f);

    return FMath::Clamp(Probability, 0.0f, 1.0f);
}

bool UPointDefenseComponent::GetThreatInfo(AActor* Threat, FIncomingThreat& OutThreatInfo) const
{
    for (const FIncomingThreat& ThreatInfo : TrackedThreats)
    {
        if (ThreatInfo.ThreatActor == Threat)
        {
            OutThreatInfo = ThreatInfo;
            return true;
        }
    }

    return false;
}

bool UPointDefenseComponent::IsValidThreat(AActor* Actor) const
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }

    // Check if actor is valid (not pending kill, not null, not destroyed)
    if (!IsValid(Actor))
    {
        return false;
    }

    // Check for threat tags
    for (const FName& MissileTag : MissileTags)
    {
        if (Actor->Tags.Contains(MissileTag))
        {
            return true;
        }
    }

    for (const FName& FighterTag : FighterTags)
    {
        if (Actor->Tags.Contains(FighterTag))
        {
            return true;
        }
    }

    return false;
}

void UPointDefenseComponent::OnThreatDetected_Implementation(AActor* Threat)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Point defense threat detected: %s"), *Threat->GetName());
}

void UPointDefenseComponent::OnThreatEngaged_Implementation(AActor* Threat, UWeaponComponent* Weapon)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Point defense engaging: %s"), *Threat->GetName());
}

void UPointDefenseComponent::OnThreatNeutralized_Implementation(AActor* Threat, bool bDestroyed)
{
    UE_LOG(LogAdastreaCombat, Log, TEXT("Threat neutralized: %s (destroyed: %s)"), 
        *Threat->GetName(), bDestroyed ? TEXT("yes") : TEXT("no"));
}

float UPointDefenseComponent::CalculateThreatPriority_Implementation(AActor* Threat) const
{
    if (!Threat)
    {
        return 0.0f;
    }

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 50.0f;
    }

    float Priority = 50.0f;

    // Check threat type
    bool bIsMissile = false;
    for (const FName& Tag : MissileTags)
    {
        if (Threat->Tags.Contains(Tag))
        {
            bIsMissile = true;
            break;
        }
    }

    // Missiles have higher base priority
    if (bIsMissile)
    {
        Priority += 20.0f;
        
        if (bPrioritizeMissiles)
        {
            Priority += 10.0f;
        }
    }

    // Distance factor (closer = higher priority)
    const float Distance = FVector::Dist(Owner->GetActorLocation(), Threat->GetActorLocation());
    const float DistanceFactor = 1.0f - FMath::Clamp(Distance / DetectionRange, 0.0f, 1.0f);
    Priority += DistanceFactor * 30.0f;

    return FMath::Clamp(Priority, 0.0f, 100.0f);
}

void UPointDefenseComponent::ScanForThreats()
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector OwnerLocation = Owner->GetActorLocation();
    const float CurrentTime = World->GetTimeSeconds();

    // Find potential threats
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    World->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(DetectionRange),
        QueryParams
    );

    // Process detected actors
    TArray<AActor*> CurrentThreats;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* PotentialThreat = Overlap.GetActor();
        
        if (!IsValidThreat(PotentialThreat))
        {
            continue;
        }

        CurrentThreats.Add(PotentialThreat);

        // Check if this is a new threat
        bool bIsNewThreat = true;
        for (const FIncomingThreat& ExistingThreat : TrackedThreats)
        {
            if (ExistingThreat.ThreatActor == PotentialThreat)
            {
                bIsNewThreat = false;
                break;
            }
        }

        if (bIsNewThreat)
        {
            // Create new threat entry
            FIncomingThreat NewThreat;
            NewThreat.ThreatActor = PotentialThreat;
            NewThreat.Distance = FVector::Dist(OwnerLocation, PotentialThreat->GetActorLocation());
            NewThreat.TimeToImpact = CalculateTimeToImpact(PotentialThreat);
            NewThreat.Priority = CalculateThreatPriority(PotentialThreat);
            NewThreat.DetectionTime = CurrentTime;
            NewThreat.bIsEngaged = false;

            TrackedThreats.Add(NewThreat);
            OnThreatDetected(PotentialThreat);
        }
    }

    // Remove threats that are no longer in range
    for (int32 i = TrackedThreats.Num() - 1; i >= 0; i--)
    {
        if (!CurrentThreats.Contains(TrackedThreats[i].ThreatActor))
        {
            AActor* LostThreat = TrackedThreats[i].ThreatActor;
            TrackedThreats.RemoveAt(i);
            DisengageThreat(LostThreat);
            OnThreatNeutralized(LostThreat, false);
        }
    }
}

void UPointDefenseComponent::UpdateThreats()
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const FVector OwnerLocation = Owner->GetActorLocation();

    // Update threat information
    for (int32 i = TrackedThreats.Num() - 1; i >= 0; i--)
    {
        FIncomingThreat& ThreatInfo = TrackedThreats[i];
        AActor* Threat = ThreatInfo.ThreatActor;

        // Check if threat is still valid
        if (!IsValid(Threat))
        {
            DisengageThreat(Threat);
            TrackedThreats.RemoveAt(i);
            if (Threat)
            {
                OnThreatNeutralized(Threat, true);
            }
            continue;
        }

        // Update threat data
        ThreatInfo.Distance = FVector::Dist(OwnerLocation, Threat->GetActorLocation());
        ThreatInfo.TimeToImpact = CalculateTimeToImpact(Threat);
        ThreatInfo.Priority = CalculateThreatPriority(Threat);
    }

    // Sort by priority
    TrackedThreats.Sort([](const FIncomingThreat& A, const FIncomingThreat& B)
    {
        return A.Priority > B.Priority;
    });
}

void UPointDefenseComponent::AssignWeaponsToThreats()
{
    // Remove assignments for weapons that are no longer available
    TArray<UWeaponComponent*> InvalidWeapons;
    for (const auto& Pair : WeaponAssignments)
    {
        if (!Pair.Key || !PointDefenseWeapons.Contains(Pair.Key))
        {
            InvalidWeapons.Add(Pair.Key);
        }
    }
    for (UWeaponComponent* Weapon : InvalidWeapons)
    {
        WeaponAssignments.Remove(Weapon);
    }

    // Try to engage highest priority unengaged threats
    for (const FIncomingThreat& ThreatInfo : TrackedThreats)
    {
        // Skip engaged threats
        if (ThreatInfo.bIsEngaged)
        {
            continue;
        }

        // Check engagement limit
        if (EngagedThreats.Num() >= MaxSimultaneousTargets)
        {
            break;
        }

        // Try to engage
        EngageThreat(ThreatInfo.ThreatActor);
    }
}

void UPointDefenseComponent::UpdateWeaponFiring(float DeltaTime)
{
    // Fire assigned weapons at their targets
    for (const auto& Pair : WeaponAssignments)
    {
        UWeaponComponent* Weapon = Pair.Key;
        AActor* Target = Pair.Value;

        if (!Weapon || !Target)
        {
            continue;
        }

        // Check if weapon can still engage target
        if (!CanWeaponEngageThreat(Weapon, Target))
        {
            DisengageThreat(Target);
            continue;
        }

        // Fire weapon
        Weapon->Fire(Target);
    }
}

bool UPointDefenseComponent::CanWeaponEngageThreat(UWeaponComponent* Weapon, AActor* Threat) const
{
    if (!Weapon || !Threat || !Weapon->WeaponData)
    {
        return false;
    }

    // Check if weapon is ready to fire
    if (!Weapon->CanFire())
    {
        return false;
    }

    // Check if threat is in range
    if (!Weapon->IsTargetInRange(Threat))
    {
        return false;
    }

    return true;
}

float UPointDefenseComponent::CalculateTimeToImpact(AActor* Threat) const
{
    if (!Threat)
    {
        return -1.0f;
    }

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return -1.0f;
    }

    const FVector ThreatVelocity = Threat->GetVelocity();
    const FVector ToOwner = Owner->GetActorLocation() - Threat->GetActorLocation();
    const float Distance = ToOwner.Size();

    // If threat is moving away, return large value
    if (FVector::DotProduct(ThreatVelocity.GetSafeNormal(), ToOwner.GetSafeNormal()) < 0.0f)
    {
        return 999999.0f;
    }

    const float Speed = ThreatVelocity.Size();
    if (Speed <= 0.0f)
    {
        return 999999.0f;
    }

    return Distance / Speed;
}
