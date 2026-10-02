#include "AI/RaiderController.h"
#include "Combat/RaidSubsystem.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Ships/Spaceship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace Raider
{
	/** Fight back against whoever last shot us for this long (s). */
	constexpr double RetaliateSeconds = 6.0;
	/** Within this share of HoldDistance, stop closing on the target. */
	constexpr float HoldBand = 1.0f;
}

ARaiderController::ARaiderController()
{
	// Raiders are out to disable, not to dogfight: hit harder than the CombatTest wave
	// fighters so an unarmed trader goes down before its shields recharge.
	DamageScale = 0.6f;
}

void ARaiderController::JoinRaid(int32 InRaidId, FName InGangId)
{
	RaidId = InRaidId;
	GangId = InGangId;
}

FString ARaiderController::GetObjectiveDescription() const
{
	if (bFleeing)
	{
		return TEXT("Running for home");
	}
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	const ERaidPhase Phase = Raids ? Raids->GetPhase(RaidId) : ERaidPhase::Retreat;
	if (Phase == ERaidPhase::Attack && GetCombatTarget())
	{
		return Super::GetObjectiveDescription();
	}
	return FString::Printf(TEXT("Raiding for %s: %s"), *GangId.ToString(), *URaidSubsystem::PhaseToString(Phase));
}

void ARaiderController::NotifyAttackedBy(AActor* Attacker)
{
	Aggressor = Attacker;
	AggressorTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
}

void ARaiderController::Flee(const TCHAR* Why)
{
	if (bFleeing)
	{
		return;
	}
	bFleeing = true;
	FleeSeconds = 0.0f;
	SetFiring(false);
	ASpaceship* Ship = GetShip();
	if (Ship && Ship->WeaponComponent)
	{
		Ship->WeaponComponent->SetAimTarget(nullptr);
	}
	// Away from the fight: from the raid's target, else from the player.
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	const AActor* From = Raids ? Raids->GetRaidTarget(RaidId) : nullptr;
	if (!From)
	{
		From = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	if (Ship)
	{
		FleeDirection = From ? (Ship->GetActorLocation() - From->GetActorLocation()).GetSafeNormal() : Ship->GetActorForwardVector();
		if (FleeDirection.IsNearlyZero())
		{
			FleeDirection = Ship->GetActorForwardVector();
		}
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raider %s (%s) breaks off: %s"), Ship ? *Ship->GetName() : *GetName(), *GangId.ToString(), Why);
}

ASpaceship* ARaiderController::FindTarget() const
{
	if (bFleeing)
	{
		return nullptr;
	}
	// Fight back against whoever is shooting at us.
	if (ASpaceship* Shooter = Cast<ASpaceship>(Aggressor.Get()); Shooter && IsAttackable(Shooter)
		&& GetWorld()->GetTimeSeconds() - AggressorTime < Raider::RetaliateSeconds)
	{
		return Shooter;
	}
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	if (Raids && Raids->GetPhase(RaidId) == ERaidPhase::Attack)
	{
		ASpaceship* Victim = Raids->GetRaidTarget(RaidId);
		return IsAttackable(Victim) ? Victim : nullptr;
	}
	return nullptr;
}

void ARaiderController::TickPilot(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	if (!bFleeing && Ship && Ship->HealthComponent && Ship->HealthComponent->GetMaxShield() > 0.0f && Ship->HealthComponent->GetShield() <= 0.0f)
	{
		Flee(TEXT("shields failed"));
	}
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	if (!bFleeing && (!Raids || !Raids->FindRaid(RaidId) || Raids->GetPhase(RaidId) == ERaidPhase::Retreat))
	{
		Flee(TEXT("the raid is over"));
	}
	if (bFleeing)
	{
		TickFlee(DeltaSeconds);
		return;
	}
	Super::TickPilot(DeltaSeconds);
}

void ARaiderController::TickNoTarget(float DeltaSeconds)
{
	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	const ASpaceship* RaidTarget = Raids ? Raids->GetRaidTarget(RaidId) : nullptr;
	if (!RaidTarget)
	{
		Super::TickNoTarget(DeltaSeconds);
		return;
	}
	// Approach, hail, collect, strip, or a target that ducked out of reach: close in and hold off it.
	TickHold(RaidTarget, DeltaSeconds);
}

void ARaiderController::TickHold(const ASpaceship* RaidTarget, float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	const float Distance = FVector::Dist(Ship->GetActorLocation(), RaidTarget->GetActorLocation());
	const float Hold = URaidSubsystem::HoldDistance;
	const float Alignment = FaceToward(RaidTarget->GetActorLocation(), DeltaSeconds);
	const float Closing = FMath::Clamp((Distance - Hold) / (Hold * Raider::HoldBand), 0.0f, 1.0f);
	// Slow while turning so the ship doesn't overshoot.
	Ship->SetThrottle(CruiseThrottle * Closing * (Alignment > 0.5f ? 1.0f : 0.4f));
}

void ARaiderController::TickFlee(float DeltaSeconds)
{
	ASpaceship* Ship = GetShip();
	FleeSeconds += DeltaSeconds;
	FaceToward(Ship->GetActorLocation() + FleeDirection * 1000000.0f, DeltaSeconds);
	Ship->SetThrottle(CruiseThrottle);

	const URaidSubsystem* Raids = URaidSubsystem::Get(this);
	const AActor* RaidTarget = Raids ? Raids->GetRaidTarget(RaidId) : nullptr;
	const AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const bool bClearOfTarget = !RaidTarget || FVector::Dist(RaidTarget->GetActorLocation(), Ship->GetActorLocation()) > LeaveDistance;
	const bool bClearOfPlayer = !Player || FVector::Dist(Player->GetActorLocation(), Ship->GetActorLocation()) > LeaveDistance;
	if ((bClearOfTarget && bClearOfPlayer) || FleeSeconds > MaxFleeSeconds)
	{
		LeaveLevel();
	}
}

void ARaiderController::LeaveLevel()
{
	ASpaceship* Ship = GetShip();
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raider %s (%s) got away"), Ship ? *Ship->GetName() : *GetName(), *GangId.ToString());
	if (URaidSubsystem* Raids = URaidSubsystem::Get(this))
	{
		Raids->NotifyRaiderLeft(this);
	}
	SetFiring(false);
	UnPossess();
	if (Ship)
	{
		Ship->Destroy();
	}
	Destroy();
}

void ARaiderController::OnShipWrecked()
{
	if (URaidSubsystem* Raids = URaidSubsystem::Get(this))
	{
		Raids->NotifyRaiderWrecked(this);
	}
	Super::OnShipWrecked();
}
