// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HostileFighterController.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Ships/Spaceship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace HostileFighter
{
	constexpr int32 Team = 1;
	/** Throttle on an attack run once inside three break-off distances: slow enough to track. */
	constexpr float CloseThrottle = 70.0f;
	constexpr float IdleThrottle = 20.0f;
	/** Break-off point: this far past the target, and this far off to the side (cm). */
	constexpr float BreakPast = 3000.0f;
	constexpr float BreakSide = 4500.0f;
}

AHostileFighterController::AHostileFighterController()
{
	// Hostiles don't trade or dock.
	bTradeAtStations = false;
}

void AHostileFighterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return;
	}
	if (Ship->HealthComponent)
	{
		Ship->HealthComponent->Team = HostileFighter::Team;
	}
	if (Ship->WeaponComponent)
	{
		Ship->WeaponComponent->BoltDamage *= DamageScale;
		Ship->WeaponComponent->SpreadDegrees = GunSpreadDegrees;
	}
	// Each pilot flies a little differently, so a wave doesn't move in lockstep.
	MaxAttackSeconds *= FMath::FRandRange(0.75f, 1.25f);
	BreakOffSeconds *= FMath::FRandRange(0.8f, 1.3f);
	BreakOffDistance *= FMath::FRandRange(0.8f, 1.2f);
	BurstSeconds *= FMath::FRandRange(0.8f, 1.2f);

	CombatState = EHostileFighterState::Idle;
	// A random start delay staggers the wave's first approach.
	StateSeconds = -FMath::FRandRange(0.0f, 1.5f);
	BurstClock = FMath::FRandRange(0.0f, BurstSeconds + BurstPause);
	UE_LOG(LogAdastreaCombat, Log, TEXT("Hostile pilot took %s"), *Ship->GetName());
}

void AHostileFighterController::OnUnPossess()
{
	SetFiring(false);
	Super::OnUnPossess();
}

bool AHostileFighterController::IsHostileShip(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Cast<AHostileFighterController>(Pawn->GetController()) != nullptr;
}

ASpaceship* AHostileFighterController::SpawnHostile(UWorld* World, TSubclassOf<ASpaceship> ShipClass, const FVector& Location, const FRotator& Facing)
{
	if (!World || !ShipClass)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASpaceship* Ship = World->SpawnActor<ASpaceship>(ShipClass, Location, Facing, Params);
	if (!Ship)
	{
		return nullptr;
	}
	AHostileFighterController* Pilot = World->SpawnActor<AHostileFighterController>(AHostileFighterController::StaticClass(), FTransform::Identity, Params);
	if (!Pilot)
	{
		Ship->Destroy();
		return nullptr;
	}
	Pilot->Possess(Ship);
	return Ship;
}

FString AHostileFighterController::GetObjectiveDescription() const
{
	const ASpaceship* Victim = Target.Get();
	switch (CombatState)
	{
	case EHostileFighterState::Attack:
		return FString::Printf(TEXT("Attacking %s"), Victim ? *Victim->GetShipName().ToString() : TEXT("target"));
	case EHostileFighterState::BreakOff:
		return TEXT("Breaking off");
	default:
		return TEXT("Hunting");
	}
}

ASpaceship* AHostileFighterController::FindTarget() const
{
	const ASpaceship* Ship = GetShip();
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	ASpaceship* Player = PC ? Cast<ASpaceship>(PC->GetPawn()) : nullptr;
	if (!Ship || !Player || Player->IsDocked() || Player->IsDocking())
	{
		return nullptr;
	}
	if (Player->HealthComponent && Player->HealthComponent->IsDestroyed())
	{
		return nullptr;
	}
	return FVector::Dist(Ship->GetActorLocation(), Player->GetActorLocation()) <= EngageRange ? Player : nullptr;
}

void AHostileFighterController::SetFiring(bool bFire)
{
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->WeaponComponent)
	{
		Ship->WeaponComponent->SetTriggerHeld(bFire);
	}
}

void AHostileFighterController::BeginBreakOff(const ASpaceship* Victim)
{
	const ASpaceship* Ship = GetShip();
	const FVector Along = (Victim->GetActorLocation() - Ship->GetActorLocation()).GetSafeNormal();
	// A random direction square to the approach: peel off to some side of the target.
	FVector Side = FVector::CrossProduct(Along, FMath::VRand()).GetSafeNormal();
	if (Side.IsNearlyZero())
	{
		Side = FVector::CrossProduct(Along, FVector::UpVector).GetSafeNormal();
	}
	BreakPoint = Victim->GetActorLocation() + Along * HostileFighter::BreakPast + Side * HostileFighter::BreakSide;
	CombatState = EHostileFighterState::BreakOff;
	StateSeconds = 0.0f;
	SetFiring(false);
}

void AHostileFighterController::TickPilot(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return;
	}

	// Shot down: remove the ship and its pilot (wreck and debris come with ship destruction).
	if (Ship->HealthComponent && Ship->HealthComponent->IsDestroyed())
	{
		UE_LOG(LogAdastreaCombat, Log, TEXT("Hostile %s shot down"), *Ship->GetName());
		SetFiring(false);
		UnPossess();
		Ship->Destroy();
		Destroy();
		return;
	}

	ASpaceship* Victim = FindTarget();
	Target = Victim;
	if (Ship->WeaponComponent)
	{
		Ship->WeaponComponent->SetAimTarget(Victim);
	}
	if (!Victim)
	{
		CombatState = EHostileFighterState::Idle;
		SetFiring(false);
		Ship->SetThrottle(HostileFighter::IdleThrottle);
		return;
	}

	StateSeconds += DeltaSeconds;
	const float Distance = FVector::Dist(Ship->GetActorLocation(), Victim->GetActorLocation());

	switch (CombatState)
	{
	case EHostileFighterState::Idle:
		FaceToward(Victim->GetActorLocation(), DeltaSeconds);
		Ship->SetThrottle(CruiseThrottle);
		if (StateSeconds >= 0.0f)
		{
			CombatState = EHostileFighterState::Attack;
			StateSeconds = 0.0f;
		}
		break;

	case EHostileFighterState::Attack:
	{
		FVector Aim = Victim->GetActorLocation();
		if (Ship->WeaponComponent)
		{
			Ship->WeaponComponent->GetLeadPoint(Victim, Aim);
		}
		const float Alignment = FaceToward(Aim, DeltaSeconds);
		Ship->SetThrottle(Distance > BreakOffDistance * 3.0f ? CruiseThrottle : HostileFighter::CloseThrottle);

		if (Distance < BreakOffDistance || StateSeconds > MaxAttackSeconds)
		{
			BeginBreakOff(Victim);
			break;
		}

		// Fire in bursts while the lead point is in the cone and in range.
		const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Alignment, -1.0f, 1.0f)));
		const float GunRange = Ship->WeaponComponent ? Ship->WeaponComponent->Range : 0.0f;
		if (AngleDeg <= FireConeDegrees && Distance <= GunRange * 0.9f)
		{
			BurstClock += DeltaSeconds;
			SetFiring(FMath::Fmod(BurstClock, BurstSeconds + BurstPause) < BurstSeconds);
		}
		else
		{
			BurstClock = 0.0f;
			SetFiring(false);
		}
		break;
	}

	case EHostileFighterState::BreakOff:
		FaceToward(BreakPoint, DeltaSeconds);
		Ship->SetThrottle(CruiseThrottle);
		if (StateSeconds > BreakOffSeconds || FVector::Dist(Ship->GetActorLocation(), BreakPoint) < BreakOffDistance)
		{
			CombatState = EHostileFighterState::Attack;
			StateSeconds = 0.0f;
			BurstClock = 0.0f;
		}
		break;
	}
}
