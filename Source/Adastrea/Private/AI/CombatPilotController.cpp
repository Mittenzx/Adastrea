// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/CombatPilotController.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Ships/Spaceship.h"
#include "Engine/World.h"
#include "AdastreaLog.h"

namespace CombatPilot
{
	/** Throttle on an attack run once inside three break-off distances: slow enough to track. */
	constexpr float CloseThrottle = 70.0f;
	constexpr float IdleThrottle = 20.0f;
	/** Break-off point: this far past the target, and this far off to the side (cm). */
	constexpr float BreakPast = 3000.0f;
	constexpr float BreakSide = 4500.0f;
	/** Switch from the current target only to one nearer than this share of its distance. */
	constexpr float SwitchTargetRatio = 0.6f;
}

ACombatPilotController::ACombatPilotController()
{
	// Fighters don't trade.
	bTradeAtStations = false;
}

void ACombatPilotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return;
	}
	if (Ship->HealthComponent)
	{
		Ship->HealthComponent->Team = GetCombatTeam();
	}
	if (Ship->WeaponComponent)
	{
		Ship->WeaponComponent->BoltDamage *= DamageScale;
		Ship->WeaponComponent->SpreadDegrees = GunSpreadDegrees;
	}
	// Each pilot flies a little differently, so a wing doesn't move in lockstep.
	MaxAttackSeconds *= FMath::FRandRange(0.75f, 1.25f);
	BreakOffSeconds *= FMath::FRandRange(0.8f, 1.3f);
	BreakOffDistance *= FMath::FRandRange(0.8f, 1.2f);
	BurstSeconds *= FMath::FRandRange(0.8f, 1.2f);

	CombatState = ECombatPilotState::Idle;
	// A random start delay staggers a wing's first approach.
	StateSeconds = -FMath::FRandRange(0.0f, 1.5f);
	BurstClock = FMath::FRandRange(0.0f, BurstSeconds + BurstPause);
	UE_LOG(LogAdastreaCombat, Log, TEXT("%s took %s (team %d)"), *GetClass()->GetName(), *Ship->GetName(), GetCombatTeam());
}

void ACombatPilotController::OnShipWrecked()
{
	UE_LOG(LogAdastreaCombat, Log, TEXT("%s: %s disabled"), *GetClass()->GetName(), GetShip() ? *GetShip()->GetName() : TEXT("?"));
	SetFiring(false);
	Super::OnShipWrecked();
}

void ACombatPilotController::OnUnPossess()
{
	SetFiring(false);
	Super::OnUnPossess();
}

FString ACombatPilotController::GetObjectiveDescription() const
{
	const ASpaceship* Victim = Target.Get();
	switch (CombatState)
	{
	case ECombatPilotState::Attack:
		return FString::Printf(TEXT("Attacking %s"), Victim ? *Victim->GetShipName().ToString() : TEXT("target"));
	case ECombatPilotState::BreakOff:
		return TEXT("Breaking off");
	default:
		return TEXT("Hunting");
	}
}

bool ACombatPilotController::IsAttackable(const ASpaceship* Ship)
{
	if (!Ship || Ship->IsWrecked() || Ship->IsHidden() || Ship->IsDocked() || Ship->IsDocking())
	{
		return false;
	}
	return !(Ship->HealthComponent && Ship->HealthComponent->IsDestroyed());
}

ASpaceship* ACombatPilotController::PickNearest(const TArray<ASpaceship*>& Candidates) const
{
	const ASpaceship* Ship = GetShip();
	if (!Ship)
	{
		return nullptr;
	}
	ASpaceship* Best = nullptr;
	float BestDist = EngageRange;
	float CurrentDist = -1.0f;
	for (ASpaceship* Candidate : Candidates)
	{
		const float Dist = FVector::Dist(Ship->GetActorLocation(), Candidate->GetActorLocation());
		if (Candidate == Target.Get() && Dist <= EngageRange)
		{
			CurrentDist = Dist;
		}
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Best = Candidate;
		}
	}
	// Stick with the current target unless another is much nearer, so pilots don't dither.
	if (CurrentDist >= 0.0f && BestDist > CurrentDist * CombatPilot::SwitchTargetRatio)
	{
		return Target.Get();
	}
	return Best;
}

void ACombatPilotController::SetFiring(bool bFire)
{
	if (const ASpaceship* Ship = GetShip(); Ship && Ship->WeaponComponent)
	{
		Ship->WeaponComponent->SetTriggerHeld(bFire);
	}
}

void ACombatPilotController::BeginBreakOff(const ASpaceship* Victim)
{
	const ASpaceship* Ship = GetShip();
	const FVector Along = (Victim->GetActorLocation() - Ship->GetActorLocation()).GetSafeNormal();
	// A random direction square to the approach: peel off to some side of the target.
	FVector Side = FVector::CrossProduct(Along, FMath::VRand()).GetSafeNormal();
	if (Side.IsNearlyZero())
	{
		Side = FVector::CrossProduct(Along, FVector::UpVector).GetSafeNormal();
	}
	BreakPoint = Victim->GetActorLocation() + Along * CombatPilot::BreakPast + Side * CombatPilot::BreakSide;
	CombatState = ECombatPilotState::BreakOff;
	StateSeconds = 0.0f;
	SetFiring(false);
}

void ACombatPilotController::TickNoTarget(float DeltaSeconds)
{
	if (ASpaceship* Ship = GetShip())
	{
		Ship->SetThrottle(CombatPilot::IdleThrottle);
	}
}

void ACombatPilotController::TickPilot(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	if (!Ship)
	{
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
		CombatState = ECombatPilotState::Idle;
		SetFiring(false);
		TickNoTarget(DeltaSeconds);
		return;
	}

	StateSeconds += DeltaSeconds;
	const float Distance = FVector::Dist(Ship->GetActorLocation(), Victim->GetActorLocation());

	switch (CombatState)
	{
	case ECombatPilotState::Idle:
		FaceToward(Victim->GetActorLocation(), DeltaSeconds);
		Ship->SetThrottle(CruiseThrottle);
		if (StateSeconds >= 0.0f)
		{
			CombatState = ECombatPilotState::Attack;
			StateSeconds = 0.0f;
		}
		break;

	case ECombatPilotState::Attack:
	{
		FVector Aim = Victim->GetActorLocation();
		if (Ship->WeaponComponent)
		{
			Ship->WeaponComponent->GetLeadPoint(Victim, Aim);
		}
		const float Alignment = FaceToward(Aim, DeltaSeconds);
		Ship->SetThrottle(Distance > BreakOffDistance * 3.0f ? CruiseThrottle : CombatPilot::CloseThrottle);

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

	case ECombatPilotState::BreakOff:
		FaceToward(BreakPoint, DeltaSeconds);
		Ship->SetThrottle(CruiseThrottle);
		if (StateSeconds > BreakOffSeconds || FVector::Dist(Ship->GetActorLocation(), BreakPoint) < BreakOffDistance)
		{
			CombatState = ECombatPilotState::Attack;
			StateSeconds = 0.0f;
			BurstClock = 0.0f;
		}
		break;
	}
}
