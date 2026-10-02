#include "Combat/ShipHealthComponent.h"
#include "Combat/CombatProjectileSubsystem.h"
#include "Combat/DistressSubsystem.h"
#include "Combat/RaidSubsystem.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Engine/World.h"
#include "AdastreaLog.h"

namespace ShipHealth
{
	/** Armor points that halve hull damage (damage x 100 / (100 + armor)). */
	constexpr float ArmorHalving = 100.0f;
	/** The hit box is the hull's bounding box, which overhangs a tapered hull; pull it in a little. */
	constexpr float HitBoxShrink = 0.9f;
}

UShipHealthComponent::UShipHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	ShieldRechargeDelay = 4.0f;
	ShieldRechargeRate = 20.0f;
	bTrainingTarget = false;
	TrainingRespawnDelay = 5.0f;
	Team = 0;

	CurrentShield = 0.0f;
	LastMaxShield = 0.0f;
	LastHitTime = -1.0e9;
	DestroyedTime = 0.0;
	bDestroyed = false;
	CachedHitBox = FBox(ForceInit);
	bHitBoxCached = false;
}

void UShipHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ASpaceship* Ship = GetShip())
	{
		if (Ship->ShipDataAsset && Ship->ShipDataAsset->ShieldRechargeRate > 0.0f)
		{
			ShieldRechargeRate = Ship->ShipDataAsset->ShieldRechargeRate;
		}
	}
	LastMaxShield = GetMaxShield();
	CurrentShield = LastMaxShield;
	SpawnTransform = GetOwner()->GetActorTransform();

	if (UCombatProjectileSubsystem* Combat = UCombatProjectileSubsystem::Get(this))
	{
		Combat->RegisterTarget(this);
	}
}

void UShipHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCombatProjectileSubsystem* Combat = UCombatProjectileSubsystem::Get(this))
	{
		Combat->UnregisterTarget(this);
	}
	Super::EndPlay(EndPlayReason);
}

ASpaceship* UShipHealthComponent::GetShip() const
{
	return Cast<ASpaceship>(GetOwner());
}

float UShipHealthComponent::GetMaxShield() const
{
	const ASpaceship* Ship = GetShip();
	return Ship ? Ship->GetMaxShieldStrength() : 0.0f;
}

float UShipHealthComponent::GetTimeSinceHit() const
{
	const UWorld* World = GetWorld();
	return World ? static_cast<float>(World->GetTimeSeconds() - LastHitTime) : 1.0e9f;
}

float UShipHealthComponent::ApplyDamage(float Amount, AActor* DamageInstigator)
{
	ASpaceship* Ship = GetShip();
	if (!Ship || bDestroyed || Amount <= 0.0f)
	{
		return 0.0f;
	}
	LastHitTime = GetWorld()->GetTimeSeconds();

	const float ShieldDamage = FMath::Min(CurrentShield, Amount);
	CurrentShield -= ShieldDamage;

	float HullDamage = 0.0f;
	if (const float Through = Amount - ShieldDamage; Through > 0.0f)
	{
		const float Armor = Ship->ShipDataAsset ? FMath::Max(Ship->ShipDataAsset->ArmorRating, 0.0f) : 0.0f;
		HullDamage = FMath::Min(Through * ShipHealth::ArmorHalving / (ShipHealth::ArmorHalving + Armor), Ship->CurrentHullIntegrity);
		Ship->CurrentHullIntegrity -= HullDamage;
	}

	OnDamaged.Broadcast(ShieldDamage, HullDamage, DamageInstigator);
	if (UDistressSubsystem* Distress = UDistressSubsystem::Get(this))
	{
		Distress->ReportAttack(Ship, DamageInstigator);
	}
	if (URaidSubsystem* Raids = URaidSubsystem::Get(this))
	{
		Raids->ReportAttack(Ship, DamageInstigator);
	}

	if (Ship->CurrentHullIntegrity <= 0.0f)
	{
		bDestroyed = true;
		DestroyedTime = GetWorld()->GetTimeSeconds();
		UE_LOG(LogAdastreaCombat, Log, TEXT("%s destroyed by %s"), *Ship->GetName(), *GetNameSafe(DamageInstigator));
		// Ships are never blown up: they're disabled and become drifting wrecks.
		Ship->SetWrecked(true);
		OnDestroyed.Broadcast(DamageInstigator);
	}
	return ShieldDamage + HullDamage;
}

void UShipHealthComponent::Restore()
{
	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return;
	}
	Ship->CurrentHullIntegrity = Ship->MaxHullIntegrity;
	LastMaxShield = GetMaxShield();
	CurrentShield = LastMaxShield;
	LastHitTime = -1.0e9;
	if (bDestroyed)
	{
		bDestroyed = false;
		Ship->SetWrecked(false);
		// Training targets go back to where they were placed (wrecks tumble and drift).
		if (bTrainingTarget)
		{
			Ship->SetActorTransform(SpawnTransform);
		}
	}
}

void UShipHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDestroyed)
	{
		if (bTrainingTarget && GetWorld()->GetTimeSeconds() - DestroyedTime >= TrainingRespawnDelay)
		{
			Restore();
		}
		return;
	}

	// Shield upgrades change the maximum: keep the same fraction charged.
	const float MaxShield = GetMaxShield();
	if (!FMath::IsNearlyEqual(MaxShield, LastMaxShield))
	{
		CurrentShield = LastMaxShield > 0.0f ? CurrentShield * (MaxShield / LastMaxShield) : MaxShield;
		LastMaxShield = MaxShield;
	}

	if (CurrentShield < MaxShield && GetTimeSinceHit() >= ShieldRechargeDelay)
	{
		CurrentShield = FMath::Min(MaxShield, CurrentShield + ShieldRechargeRate * DeltaTime);
	}
}

const FBox& UShipHealthComponent::GetHitBox() const
{
	if (!bHitBoxCached)
	{
		if (const ASpaceship* Ship = GetShip())
		{
			const FBox Hull = Ship->GetHullLocalBounds();
			if (Hull.IsValid)
			{
				CachedHitBox = FBox::BuildAABB(Hull.GetCenter(), Hull.GetExtent() * ShipHealth::HitBoxShrink);
				bHitBoxCached = true;
			}
		}
	}
	return CachedHitBox;
}

bool UShipHealthComponent::SegmentHit(const FVector& A, const FVector& B, FVector& OutHit, float& OutTime) const
{
	const ASpaceship* Ship = GetShip();
	if (!Ship || !Ship->ShipRoot)
	{
		return false;
	}
	const FBox& Box = GetHitBox();
	if (!Box.IsValid)
	{
		return false;
	}

	FTransform Frame = Ship->ShipRoot->GetComponentTransform();
	Frame.SetScale3D(FVector::OneVector);
	const FVector LA = Frame.InverseTransformPosition(A);
	const FVector LB = Frame.InverseTransformPosition(B);

	// Slab test of the local segment against the box.
	const FVector D = LB - LA;
	float TMin = 0.0f, TMax = 1.0f;
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		if (FMath::Abs(D[Axis]) < KINDA_SMALL_NUMBER)
		{
			if (LA[Axis] < Box.Min[Axis] || LA[Axis] > Box.Max[Axis])
			{
				return false;
			}
			continue;
		}
		float T1 = (Box.Min[Axis] - LA[Axis]) / D[Axis];
		float T2 = (Box.Max[Axis] - LA[Axis]) / D[Axis];
		if (T1 > T2)
		{
			Swap(T1, T2);
		}
		TMin = FMath::Max(TMin, T1);
		TMax = FMath::Min(TMax, T2);
		if (TMin > TMax)
		{
			return false;
		}
	}
	OutTime = TMin;
	OutHit = A + (B - A) * TMin;
	return true;
}
