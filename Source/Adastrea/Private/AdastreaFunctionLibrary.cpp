#include "AdastreaFunctionLibrary.h"
#include "Factions/FactionDataAsset.h"
#include "Interfaces/IFactionMember.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ====================
// DISTANCE & POSITION UTILITIES
// ====================

float UAdastreaFunctionLibrary::GetDistance2D(AActor* ActorA, AActor* ActorB)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
    {
        return -1.0f;
    }

    FVector LocationA = ActorA->GetActorLocation();
    FVector LocationB = ActorB->GetActorLocation();
    LocationA.Z = 0.0f;
    LocationB.Z = 0.0f;

    return FVector::Dist(LocationA, LocationB);
}

bool UAdastreaFunctionLibrary::IsWithinRange(AActor* ActorA, AActor* ActorB, float Range)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
    {
        return false;
    }

    float Distance = FVector::Dist(ActorA->GetActorLocation(), ActorB->GetActorLocation());
    return Distance <= Range;
}

AActor* UAdastreaFunctionLibrary::GetClosestActor(AActor* Origin, const TArray<AActor*>& Actors)
{
    if (!IsValid(Origin) || Actors.Num() == 0)
    {
        return nullptr;
    }

    AActor* ClosestActor = nullptr;
    float ClosestDistance = FLT_MAX;
    FVector OriginLocation = Origin->GetActorLocation();

    for (AActor* Actor : Actors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        float Distance = FVector::Dist(OriginLocation, Actor->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestActor = Actor;
        }
    }

    return ClosestActor;
}

TArray<AActor*> UAdastreaFunctionLibrary::GetActorsWithinSphere(
    const UObject* WorldContextObject,
    FVector Center,
    float Radius,
    TSubclassOf<AActor> ActorClass)
{
    TArray<AActor*> FoundActors;

    if (!WorldContextObject)
    {
        return FoundActors;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    if (!World)
    {
        return FoundActors;
    }

    // Get all actors of specified class (or all actors if null)
    TArray<AActor*> AllActors;
    TSubclassOf<AActor> ClassToFind = ActorClass.Get() ? ActorClass : TSubclassOf<AActor>(AActor::StaticClass());
    UGameplayStatics::GetAllActorsOfClass(World, ClassToFind, AllActors);

    // Filter by distance
    for (AActor* Actor : AllActors)
    {
        if (IsValid(Actor))
        {
            float Distance = FVector::Dist(Center, Actor->GetActorLocation());
            if (Distance <= Radius)
            {
                FoundActors.Add(Actor);
            }
        }
    }

    return FoundActors;
}

FVector UAdastreaFunctionLibrary::GetDirectionToActor(AActor* From, AActor* To)
{
    if (!IsValid(From) || !IsValid(To))
    {
        return FVector::ZeroVector;
    }

    FVector Direction = To->GetActorLocation() - From->GetActorLocation();
    Direction.Normalize();
    return Direction;
}

// ====================
// FACTION UTILITIES
// ====================

bool UAdastreaFunctionLibrary::AreFactionsAllied(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB)
{
    int32 Relationship = GetFactionRelationship(FactionA, FactionB);
    return Relationship >= 26; // Friendly or allied threshold
}

bool UAdastreaFunctionLibrary::AreFactionsHostile(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB)
{
    int32 Relationship = GetFactionRelationship(FactionA, FactionB);
    return Relationship <= -26; // Hostile or war threshold
}

int32 UAdastreaFunctionLibrary::GetFactionRelationship(UFactionDataAsset* FactionA, UFactionDataAsset* FactionB)
{
    if (!FactionA || !FactionB)
    {
        return 0; // Neutral if either faction is null
    }

    if (FactionA == FactionB)
    {
        return 100; // Same faction = maximum positive
    }

    // TODO: Query FactionDiplomacyManager for actual relationship value
    // For now return neutral
    return 0;
}

UFactionDataAsset* UAdastreaFunctionLibrary::GetActorFaction(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return nullptr;
    }

    // Check if actor implements IFactionMember
    if (Actor->Implements<UFactionMember>())
    {
        return IFactionMember::Execute_GetFaction(Actor);
    }

    return nullptr;
}

// ====================
// COMBAT UTILITIES
// ====================

float UAdastreaFunctionLibrary::CalculateDamageAfterArmor(float RawDamage, float ArmorValue, EDamageType DamageType)
{
    if (RawDamage <= 0.0f)
    {
        return 0.0f;
    }

    // Basic armor mitigation formula
    // Higher armor = more damage reduction
    // Different damage types could have different penetration in the future
    float ArmorReduction = ArmorValue / (ArmorValue + 100.0f);
    
    // TODO: Apply damage type modifiers
    // - Kinetic: standard penetration
    // - Energy: bypasses some armor
    // - Explosive: reduced by armor
    // - Thermal: minimal armor effect
    // - EMP: ignores armor completely

    float FinalDamage = RawDamage * (1.0f - ArmorReduction);
    return FMath::Max(FinalDamage, 0.0f);
}

FVector UAdastreaFunctionLibrary::CalculateLeadTarget(
    FVector ShooterLocation,
    FVector TargetLocation,
    FVector TargetVelocity,
    float ProjectileSpeed)
{
    if (ProjectileSpeed <= 0.0f)
    {
        return TargetLocation; // Return current location if projectile has no speed
    }

    // Calculate vector to target
    FVector ToTarget = TargetLocation - ShooterLocation;
    float Distance = ToTarget.Size();

    if (Distance <= 0.0f)
    {
        return TargetLocation;
    }

    // Calculate time for projectile to reach target's current position
    float TimeToHit = Distance / ProjectileSpeed;

    // Predict where target will be after that time
    FVector PredictedLocation = TargetLocation + (TargetVelocity * TimeToHit);

    // Refine prediction (one iteration should be sufficient for most cases)
    FVector ToPredicted = PredictedLocation - ShooterLocation;
    float PredictedDistance = ToPredicted.Size();
    TimeToHit = PredictedDistance / ProjectileSpeed;
    PredictedLocation = TargetLocation + (TargetVelocity * TimeToHit);

    return PredictedLocation;
}

bool UAdastreaFunctionLibrary::IsInFiringArc(
    FVector ShooterLocation,
    FRotator ShooterRotation,
    FVector TargetLocation,
    float ArcAngleDegrees)
{
    // Get direction to target
    FVector ToTarget = TargetLocation - ShooterLocation;
    ToTarget.Normalize();

    // Get shooter's forward vector
    FVector Forward = ShooterRotation.Vector();

    // Calculate angle between forward and target direction
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    // Check if within half of the arc angle (since arc is a full cone)
    return AngleDegrees <= (ArcAngleDegrees * 0.5f);
}

float UAdastreaFunctionLibrary::CalculateTimeToImpact(float Distance, float ProjectileSpeed)
{
    if (ProjectileSpeed <= 0.0f || Distance <= 0.0f)
    {
        return 0.0f;
    }

    return Distance / ProjectileSpeed;
}

// ====================
// DATA VALIDATION UTILITIES
// ====================

bool UAdastreaFunctionLibrary::ValidateDataAsset(UDataAsset* DataAsset, FString& OutErrorMessage)
{
    if (!IsValid(DataAsset))
    {
        OutErrorMessage = TEXT("Data Asset is null or invalid");
        return false;
    }

    // Basic validation - more specific validation should be done in the Data Asset's IsDataValid() method
    OutErrorMessage = TEXT("");
    return true;
}

float UAdastreaFunctionLibrary::ClampValue(float Value, float Min, float Max)
{
    return FMath::Clamp(Value, Min, Max);
}

int32 UAdastreaFunctionLibrary::ClampValueInt(int32 Value, int32 Min, int32 Max)
{
    return FMath::Clamp(Value, Min, Max);
}

// ====================
// STRING UTILITIES
// ====================

FString UAdastreaFunctionLibrary::FormatLargeNumber(int32 Number)
{
    FString NumStr = FString::FromInt(FMath::Abs(Number));
    FString Result;

    int32 Count = 0;
    for (int32 i = NumStr.Len() - 1; i >= 0; i--)
    {
        if (Count > 0 && Count % 3 == 0)
        {
            Result.InsertAt(0, TEXT(","));
        }
        Result.InsertAt(0, FString::Chr(NumStr[i]));
        Count++;
    }

    if (Number < 0)
    {
        Result.InsertAt(0, TEXT("-"));
    }

    return Result;
}

FString UAdastreaFunctionLibrary::FormatLargeNumberFloat(float Number, int32 DecimalPlaces)
{
    bool bIsNegative = Number < 0.0f;
    float AbsNumber = FMath::Abs(Number);
    
    // Split into integer and decimal parts
    int32 IntegerPart = FMath::FloorToInt(AbsNumber);
    float DecimalPart = AbsNumber - IntegerPart;

    // Format integer part with commas (using positive value)
    FString IntegerStr = FormatLargeNumber(IntegerPart);

    // Format decimal part
    FString DecimalStr;
    if (DecimalPlaces > 0)
    {
        DecimalPart = FMath::RoundToFloat(DecimalPart * FMath::Pow(10.0f, DecimalPlaces));
        DecimalStr = FString::Printf(TEXT(".%0*d"), DecimalPlaces, FMath::RoundToInt(DecimalPart));
    }

    FString Result = IntegerStr + DecimalStr;

    if (bIsNegative)
    {
        Result.InsertAt(0, TEXT("-"));
    }

    return Result;
}

FString UAdastreaFunctionLibrary::FormatPercentage(float Percentage, int32 DecimalPlaces)
{
    float PercentValue = Percentage * 100.0f;
    
    if (DecimalPlaces == 0)
    {
        return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(PercentValue));
    }
    else
    {
        FString Format = FString::Printf(TEXT("%%.%df%%%%"), DecimalPlaces);
        return Format.Replace(TEXT("%f"), *FString::SanitizeFloat(PercentValue));
    }
}

FString UAdastreaFunctionLibrary::FormatDuration(float Seconds)
{
    int32 TotalSeconds = FMath::RoundToInt(Seconds);
    
    int32 Hours = TotalSeconds / 3600;
    int32 Minutes = (TotalSeconds % 3600) / 60;
    int32 Secs = TotalSeconds % 60;

    FString Result;

    if (Hours > 0)
    {
        Result += FString::Printf(TEXT("%dh "), Hours);
    }

    if (Minutes > 0 || Hours > 0)
    {
        Result += FString::Printf(TEXT("%dm "), Minutes);
    }

    Result += FString::Printf(TEXT("%ds"), Secs);

    return Result.TrimEnd();
}

// ====================
// MATH UTILITIES
// ====================

float UAdastreaFunctionLibrary::RemapValue(float Value, float InMin, float InMax, float OutMin, float OutMax)
{
    if (FMath::IsNearlyEqual(InMax, InMin))
    {
        return OutMin; // Avoid division by zero
    }

    float Normalized = (Value - InMin) / (InMax - InMin);
    return OutMin + (Normalized * (OutMax - OutMin));
}

float UAdastreaFunctionLibrary::GetPercentageInRange(float Value, float Min, float Max)
{
    if (FMath::IsNearlyEqual(Max, Min))
    {
        return 0.0f; // Avoid division by zero
    }

    float Percentage = (Value - Min) / (Max - Min);
    return FMath::Clamp(Percentage, 0.0f, 1.0f);
}
