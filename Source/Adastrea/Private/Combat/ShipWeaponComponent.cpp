#include "Combat/ShipWeaponComponent.h"
#include "Combat/CombatProjectileSubsystem.h"
#include "Combat/ShipHealthComponent.h"
#include "Ships/Spaceship.h"
#include "Player/AdastreaPlayerController.h"
#include "AdastreaLog.h"

namespace ShipGuns
{
	/** Guns converge no closer than this, so a target in your face doesn't cross them wildly. */
	constexpr float MinConvergence = 1500.0f;
}

UShipWeaponComponent::UShipWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BoltDamage = 15.0f;
	FireRate = 6.0f;
	BoltSpeed = 30000.0f;
	Range = 30000.0f;
	BoltLength = 1100.0f;
	SpreadDegrees = 0.15f;
	ConvergenceDistance = 8000.0f;
	GunCount = 2;

	HullCentre = FVector::ZeroVector;
	Cooldown = 0.0f;
	NextGun = 0;
	ShotsFired = 0;
	bTriggerHeld = false;
}

ASpaceship* UShipWeaponComponent::GetShip() const
{
	return Cast<ASpaceship>(GetOwner());
}

FTransform UShipWeaponComponent::GetShipFrame() const
{
	const ASpaceship* Ship = GetShip();
	FTransform Frame = Ship && Ship->ShipRoot ? Ship->ShipRoot->GetComponentTransform() : GetOwner()->GetActorTransform();
	Frame.SetScale3D(FVector::OneVector);
	return Frame;
}

void UShipWeaponComponent::SetTriggerHeld(bool bHeld)
{
	bTriggerHeld = bHeld;
}

void UShipWeaponComponent::BuildMuzzles()
{
	Muzzles.Reset();
	const ASpaceship* Ship = GetShip();
	const FBox Hull = Ship ? Ship->GetHullLocalBounds() : FBox(ForceInit);
	if (!Hull.IsValid)
	{
		HullCentre = FVector::ZeroVector;
		Muzzles.Add(FVector(200.0f, 0.0f, 0.0f));
		return;
	}

	// No weapon sockets on the hulls yet: spread the guns across the front of the
	// hull box, slightly below the centreline.
	HullCentre = Hull.GetCenter();
	const FVector Ext = Hull.GetExtent();
	const float X = Hull.Max.X * 0.9f;
	const float Z = HullCentre.Z - Ext.Z * 0.1f;
	const int32 Count = FMath::Max(GunCount, 1);
	for (int32 i = 0; i < Count; ++i)
	{
		const float Side = Count == 1 ? 0.0f : FMath::Lerp(-1.0f, 1.0f, i / float(Count - 1));
		Muzzles.Add(FVector(X, HullCentre.Y + Side * Ext.Y * 0.45f, Z));
	}
}

FVector UShipWeaponComponent::GetConvergencePoint() const
{
	const FTransform Frame = GetShipFrame();
	const FVector Centre = Frame.TransformPosition(HullCentre);
	float Distance = ConvergenceDistance;
	if (const AActor* Target = AimTarget.Get())
	{
		Distance = FMath::Clamp(FVector::Dist(Centre, Target->GetActorLocation()), ShipGuns::MinConvergence, Range);
	}
	return Centre + Frame.GetUnitAxis(EAxis::X) * Distance;
}

bool UShipWeaponComponent::ComputeLeadPoint(const FVector& Shooter, const FVector& ShooterVel, const FVector& Target,
	const FVector& TargetVel, float ProjectileSpeed, FVector& OutPoint)
{
	// Bolts inherit the shooter's velocity, so solve in the shooter's frame:
	// |R + V t| = s t  ->  (V.V - s^2) t^2 + 2 (R.V) t + R.R = 0
	const FVector R = Target - Shooter;
	const FVector V = TargetVel - ShooterVel;
	const double A = FVector::DotProduct(V, V) - double(ProjectileSpeed) * ProjectileSpeed;
	const double B = 2.0 * FVector::DotProduct(R, V);
	const double C = FVector::DotProduct(R, R);

	double T;
	if (FMath::Abs(A) < 1.0e-6)
	{
		if (FMath::Abs(B) < 1.0e-6)
		{
			return false;
		}
		T = -C / B;
	}
	else
	{
		const double Disc = B * B - 4.0 * A * C;
		if (Disc < 0.0)
		{
			return false;
		}
		const double Root = FMath::Sqrt(Disc);
		const double T1 = (-B - Root) / (2.0 * A);
		const double T2 = (-B + Root) / (2.0 * A);
		T = (T1 > 0.0 && (T1 < T2 || T2 <= 0.0)) ? T1 : T2;
	}
	if (T <= 0.0)
	{
		return false;
	}
	OutPoint = Target + V * T;
	return true;
}

bool UShipWeaponComponent::GetLeadPoint(const AActor* Target, FVector& OutPoint) const
{
	const AActor* Owner = GetOwner();
	if (!Target || !Owner)
	{
		return false;
	}
	const FVector Centre = GetShipFrame().TransformPosition(HullCentre);
	return ComputeLeadPoint(Centre, Owner->GetVelocity(), Target->GetActorLocation(), Target->GetVelocity(), BoltSpeed, OutPoint);
}

void UShipWeaponComponent::FireNextGun(float Age)
{
	UCombatProjectileSubsystem* Combat = UCombatProjectileSubsystem::Get(this);
	ASpaceship* Ship = GetShip();
	if (!Combat || !Ship || Muzzles.Num() == 0)
	{
		return;
	}

	NextGun %= Muzzles.Num();
	const FVector Muzzle = GetShipFrame().TransformPosition(Muzzles[NextGun]);
	NextGun = (NextGun + 1) % Muzzles.Num();

	FVector Dir = (GetConvergencePoint() - Muzzle).GetSafeNormal();
	if (SpreadDegrees > 0.0f)
	{
		Dir = FMath::VRandCone(Dir, FMath::DegreesToRadians(SpreadDegrees));
	}
	const float Damage = BoltDamage * Ship->GetWeaponDamageMultiplier();
	const int32 Team = Ship->HealthComponent ? Ship->HealthComponent->Team : 0;
	Combat->FireBolt(Muzzle, Dir * BoltSpeed + Ship->GetVelocity(), Damage, Range / BoltSpeed, BoltLength, Ship, Team, Age);
	++ShotsFired;
}

void UShipWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// The hull bounds aren't final until the ship has set up its meshes in BeginPlay.
	if (Muzzles.Num() == 0)
	{
		BuildMuzzles();
	}

	ASpaceship* Ship = GetShip();
	if (Ship && (Ship->IsDocked() || Ship->IsDocking() || (Ship->HealthComponent && Ship->HealthComponent->IsDestroyed())))
	{
		bTriggerHeld = false;
	}
	// The pilot's guns converge on whatever they have locked.
	if (const AAdastreaPlayerController* PC = Ship ? Cast<AAdastreaPlayerController>(Ship->GetController()) : nullptr)
	{
		AimTarget = PC->GetLockedTarget();
	}

	if (!bTriggerHeld)
	{
		Cooldown = FMath::Max(Cooldown - DeltaTime, 0.0f);
		return;
	}

	// The guns take turns, so the stream is steady: Muzzles x FireRate shots a second.
	// A shot that fell due part-way through this frame is aged by how long ago that was.
	const float Interval = 1.0f / (FireRate * Muzzles.Num());
	// A fresh pull (guns cooled) fires at once; otherwise count down through this frame.
	Cooldown = Cooldown <= 0.0f ? 0.0f : Cooldown - DeltaTime;
	while (Cooldown <= 0.0f)
	{
		FireNextGun(FMath::Min(-Cooldown, DeltaTime));
		Cooldown += Interval;
	}
}
