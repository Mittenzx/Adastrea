// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Combat/TargetingComponent.h"
#include "Combat/WeaponComponent.h"
#include "AdastreaLog.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UTargetingComponent::UTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Default configuration
    MaxTargetingRange = 10000.0f;
    MaxTargetingAngle = 90.0f;
    ScanInterval = 0.5f;
    bAutoRetarget = true;
    bPrioritizeByThreat = true;

    // Default state
    CurrentTarget = nullptr;
    bIsLocked = false;
    TimeSinceLastScan = 0.0f;
    LockTime = 0.0f;
}

void UTargetingComponent::BeginPlay()
{
    Super::BeginPlay();

    // Perform initial scan
    ScanForTargets();
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastScan += DeltaTime;

    // Periodic target scan
    if (TimeSinceLastScan >= ScanInterval)
    {
        ScanForTargets();
        TimeSinceLastScan = 0.0f;
    }

    // Update current target state
    UpdateCurrentTarget();
}

bool UTargetingComponent::SelectTarget(AActor* Target)
{
    if (!Target)
    {
        ClearTarget();
        return false;
    }

    // Validate target
    if (!IsValidTarget(Target))
    {
        return false;
    }

    // Check range
    const AActor* Owner = GetOwner();
    if (Owner)
    {
        const float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
        if (Distance > MaxTargetingRange)
        {
            return false;
        }
    }

    // Lock target
    CurrentTarget = Target;
    bIsLocked = true;
    LockTime = GetWorld()->GetTimeSeconds();
    OnTargetLocked(Target);

    return true;
}

AActor* UTargetingComponent::SelectNextTarget()
{
    if (DetectedTargets.Num() == 0)
    {
        return nullptr;
    }

    int32 CurrentIndex = -1;

    // Find current target in list
    if (CurrentTarget)
    {
        for (int32 i = 0; i < DetectedTargets.Num(); i++)
        {
            if (DetectedTargets[i].Target == CurrentTarget)
            {
                CurrentIndex = i;
                break;
            }
        }
    }

    // Select next target
    const int32 NextIndex = (CurrentIndex + 1) % DetectedTargets.Num();
    if (SelectTarget(DetectedTargets[NextIndex].Target))
    {
        return DetectedTargets[NextIndex].Target;
    }

    return nullptr;
}

AActor* UTargetingComponent::SelectPreviousTarget()
{
    if (DetectedTargets.Num() == 0)
    {
        return nullptr;
    }

    int32 CurrentIndex = -1;

    // Find current target in list
    if (CurrentTarget)
    {
        for (int32 i = 0; i < DetectedTargets.Num(); i++)
        {
            if (DetectedTargets[i].Target == CurrentTarget)
            {
                CurrentIndex = i;
                break;
            }
        }
    }

    // Select previous target
    int32 PrevIndex = CurrentIndex - 1;
    if (PrevIndex < 0)
    {
        PrevIndex = DetectedTargets.Num() - 1;
    }

    if (SelectTarget(DetectedTargets[PrevIndex].Target))
    {
        return DetectedTargets[PrevIndex].Target;
    }

    return nullptr;
}

AActor* UTargetingComponent::SelectNearestTarget()
{
    if (DetectedTargets.Num() == 0)
    {
        return nullptr;
    }

    // Find nearest target
    float MinDistance = MAX_FLT;
    AActor* NearestTarget = nullptr;

    for (const FTargetInfo& TargetInfo : DetectedTargets)
    {
        if (TargetInfo.Distance < MinDistance)
        {
            MinDistance = TargetInfo.Distance;
            NearestTarget = TargetInfo.Target;
        }
    }

    if (SelectTarget(NearestTarget))
    {
        return NearestTarget;
    }

    return nullptr;
}

AActor* UTargetingComponent::SelectHighestThreatTarget()
{
    if (DetectedTargets.Num() == 0)
    {
        return nullptr;
    }

    // Find highest threat target
    float MaxThreat = -1.0f;
    AActor* MostDangerousTarget = nullptr;

    for (const FTargetInfo& TargetInfo : DetectedTargets)
    {
        if (TargetInfo.ThreatLevel > MaxThreat)
        {
            MaxThreat = TargetInfo.ThreatLevel;
            MostDangerousTarget = TargetInfo.Target;
        }
    }

    if (SelectTarget(MostDangerousTarget))
    {
        return MostDangerousTarget;
    }

    return nullptr;
}

void UTargetingComponent::ClearTarget()
{
    if (CurrentTarget)
    {
        OnTargetLost(TEXT("Manual"));
    }

    CurrentTarget = nullptr;
    bIsLocked = false;
    LockTime = 0.0f;
}

float UTargetingComponent::GetDistanceToTarget() const
{
    if (!CurrentTarget)
    {
        return 0.0f;
    }

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 0.0f;
    }

    return FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
}

float UTargetingComponent::GetAngleToTarget() const
{
    if (!CurrentTarget)
    {
        return 0.0f;
    }

    return CalculateAngleToActor(CurrentTarget);
}

bool UTargetingComponent::IsTargetInFiringArc(float ArcAngle) const
{
    if (!CurrentTarget)
    {
        return false;
    }

    const float Angle = GetAngleToTarget();
    return Angle <= ArcAngle;
}

FVector UTargetingComponent::CalculateLeadPosition(float ProjectileSpeed) const
{
    if (!CurrentTarget || ProjectileSpeed <= 0.0f)
    {
        return CurrentTarget ? CurrentTarget->GetActorLocation() : FVector::ZeroVector;
    }

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return CurrentTarget->GetActorLocation();
    }

    // Get target velocity
    const FVector TargetVelocity = CurrentTarget->GetVelocity();
    const FVector TargetLocation = CurrentTarget->GetActorLocation();
    const FVector ShooterLocation = Owner->GetActorLocation();

    // Calculate time to intercept
    const FVector ToTarget = TargetLocation - ShooterLocation;
    const float Distance = ToTarget.Size();
    const float TimeToIntercept = Distance / ProjectileSpeed;

    // Calculate lead position
    const FVector LeadPosition = TargetLocation + (TargetVelocity * TimeToIntercept);

    return LeadPosition;
}

bool UTargetingComponent::GetCurrentTargetInfo(FTargetInfo& OutTargetInfo) const
{
    if (!CurrentTarget)
    {
        return false;
    }

    // Find target in detected list
    for (const FTargetInfo& TargetInfo : DetectedTargets)
    {
        if (TargetInfo.Target == CurrentTarget)
        {
            OutTargetInfo = TargetInfo;
            return true;
        }
    }

    // Target not in detected list, create info on the fly
    OutTargetInfo.Target = CurrentTarget;
    OutTargetInfo.Distance = GetDistanceToTarget();
    OutTargetInfo.Angle = GetAngleToTarget();
    OutTargetInfo.ThreatLevel = CalculateThreatLevel(CurrentTarget);
    OutTargetInfo.DetectionTime = LockTime;

    return true;
}

TArray<FTargetInfo> UTargetingComponent::GetDetectedTargetsSortedByDistance() const
{
    TArray<FTargetInfo> SortedTargets = DetectedTargets;
    
    SortedTargets.Sort([](const FTargetInfo& A, const FTargetInfo& B)
    {
        return A.Distance < B.Distance;
    });

    return SortedTargets;
}

TArray<FTargetInfo> UTargetingComponent::GetDetectedTargetsSortedByThreat() const
{
    TArray<FTargetInfo> SortedTargets = DetectedTargets;
    
    SortedTargets.Sort([](const FTargetInfo& A, const FTargetInfo& B)
    {
        return A.ThreatLevel > B.ThreatLevel;
    });

    return SortedTargets;
}

int32 UTargetingComponent::GetDetectedTargetCount() const
{
    return DetectedTargets.Num();
}

void UTargetingComponent::OnTargetLocked_Implementation(AActor* Target)
{
    // Default implementation
    UE_LOG(LogAdastreaCombat, Log, TEXT("Target locked: %s"), Target ? *Target->GetName() : TEXT("nullptr"));
}

void UTargetingComponent::OnTargetLost_Implementation(const FString& Reason)
{
    // Default implementation
    UE_LOG(LogAdastreaCombat, Log, TEXT("Target lost: %s"), *Reason);
}

float UTargetingComponent::CalculateThreatLevel_Implementation(AActor* Target) const
{
    if (!Target)
    {
        return 0.0f;
    }

    // Base threat calculation
    float ThreatLevel = 50.0f;

    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return ThreatLevel;
    }

    // Distance factor: Closer targets are more threatening (up to +25)
    const float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    const float DistanceFactor = 1.0f - FMath::Clamp(Distance / MaxTargetingRange, 0.0f, 1.0f);
    ThreatLevel += DistanceFactor * 25.0f;

    // Velocity factor: Targets moving toward us are more threatening (up to +15)
    const FVector ToTarget = Target->GetActorLocation() - Owner->GetActorLocation();
    const FVector TargetVelocity = Target->GetVelocity();
    const float ApproachSpeed = FVector::DotProduct(TargetVelocity.GetSafeNormal(), ToTarget.GetSafeNormal());
    if (ApproachSpeed < 0.0f) // Negative means approaching
    {
        ThreatLevel += FMath::Abs(ApproachSpeed) * 15.0f;
    }

    // Check if target has combat components (weapon, targeting, health)
    if (Target->FindComponentByClass<UWeaponComponent>())
    {
        ThreatLevel += 10.0f; // Armed targets are more dangerous
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 100.0f);
}

void UTargetingComponent::ScanForTargets()
{
    DetectedTargets.Empty();

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

    // Get all actors in range
    TArray<AActor*> OverlappingActors;
    const FVector OwnerLocation = Owner->GetActorLocation();

    // Use sphere overlap to find nearby actors
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    World->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(MaxTargetingRange),
        QueryParams
    );

    const float CurrentTime = World->GetTimeSeconds();

    // Process overlapping actors
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* PotentialTarget = Overlap.GetActor();
        
        if (!IsValidTarget(PotentialTarget))
        {
            continue;
        }

        // Calculate target info
        FTargetInfo TargetInfo;
        TargetInfo.Target = PotentialTarget;
        TargetInfo.Distance = FVector::Dist(OwnerLocation, PotentialTarget->GetActorLocation());
        TargetInfo.Angle = CalculateAngleToActor(PotentialTarget);
        TargetInfo.ThreatLevel = CalculateThreatLevel(PotentialTarget);
        TargetInfo.DetectionTime = CurrentTime;

        // Check angle constraint
        if (TargetInfo.Angle <= MaxTargetingAngle)
        {
            DetectedTargets.Add(TargetInfo);
        }
    }

    // Sort by priority
    if (bPrioritizeByThreat)
    {
        DetectedTargets.Sort([](const FTargetInfo& A, const FTargetInfo& B)
        {
            return A.ThreatLevel > B.ThreatLevel;
        });
    }
    else
    {
        DetectedTargets.Sort([](const FTargetInfo& A, const FTargetInfo& B)
        {
            return A.Distance < B.Distance;
        });
    }
}

void UTargetingComponent::UpdateCurrentTarget()
{
    if (!CurrentTarget)
    {
        return;
    }

    // Check if target is still valid
    if (!IsValidTarget(CurrentTarget))
    {
        ClearTarget();
        OnTargetLost(TEXT("Destroyed"));
        
        if (bAutoRetarget)
        {
            SelectNearestTarget();
        }
        return;
    }

    // Check if target is out of range
    const float Distance = GetDistanceToTarget();
    if (Distance > MaxTargetingRange)
    {
        ClearTarget();
        OnTargetLost(TEXT("OutOfRange"));
        
        if (bAutoRetarget)
        {
            SelectNearestTarget();
        }
        return;
    }

    // Check if target is outside angle
    const float Angle = GetAngleToTarget();
    if (Angle > MaxTargetingAngle)
    {
        ClearTarget();
        OnTargetLost(TEXT("OutOfArc"));
        
        if (bAutoRetarget)
        {
            SelectNearestTarget();
        }
        return;
    }
}

bool UTargetingComponent::IsValidTarget(AActor* Target) const
{
    if (!Target || Target == GetOwner())
    {
        return false;
    }

    // Check if target is valid (not pending kill, not destroyed)
    if (!IsValid(Target))
    {
        return false;
    }

    // If target tags specified, check for matching tags
    if (TargetTags.Num() > 0)
    {
        bool bHasValidTag = false;
        for (const FName& Tag : TargetTags)
        {
            if (Target->Tags.Contains(Tag))
            {
                bHasValidTag = true;
                break;
            }
        }
        
        if (!bHasValidTag)
        {
            return false;
        }
    }

    return true;
}

float UTargetingComponent::CalculateAngleToActor(AActor* Target) const
{
    const AActor* Owner = GetOwner();
    if (!Owner || !Target)
    {
        return 0.0f;
    }

    const FVector Forward = Owner->GetActorForwardVector();
    const FVector ToTarget = (Target->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    
    const float DotProduct = FVector::DotProduct(Forward, ToTarget);
    const float AngleRadians = FMath::Acos(DotProduct);
    const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

    return AngleDegrees;
}
