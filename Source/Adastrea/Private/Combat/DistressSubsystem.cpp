#include "Combat/DistressSubsystem.h"
#include "AI/HostileFighterController.h"
#include "AI/PatrolController.h"
#include "Combat/CombatTeams.h"
#include "Combat/ShipHealthComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Universe/OrganisationSubsystem.h"
#include "Universe/OwnershipComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace Distress
{
	/** A call closes once no hostile has been near it for this long (s)... */
	constexpr double QuietSecondsToClose = 8.0;
	/** ...or this long after the last shot, even if hostiles hang about (s). */
	constexpr double MaxSecondsSinceAttack = 180.0;
}

UDistressSubsystem* UDistressSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UDistressSubsystem>() : nullptr;
}

bool UDistressSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UDistressSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDistressSubsystem, STATGROUP_Tickables);
}

void UDistressSubsystem::GetResponse(ESectorSecurity Security, int32& OutMaxPatrols, float& OutDispatchDelay)
{
	switch (Security)
	{
	case ESectorSecurity::High:		OutMaxPatrols = 3; OutDispatchDelay = 0.0f; break;
	case ESectorSecurity::Medium:	OutMaxPatrols = 2; OutDispatchDelay = 15.0f; break;
	case ESectorSecurity::Low:		OutMaxPatrols = 1; OutDispatchDelay = 40.0f; break;
	default:						OutMaxPatrols = 0; OutDispatchDelay = 0.0f; break;
	}
}

void UDistressSubsystem::ReportAttack(ASpaceship* Victim, AActor* Attacker)
{
	if (!Victim || !Victim->HealthComponent || Victim->HealthComponent->Team != CombatTeam::Civil
		|| !AHostileFighterController::IsHostileShip(Attacker))
	{
		return;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	for (FDistressCall& Call : Calls)
	{
		if (Call.Victim.Get() == Victim)
		{
			Call.LastAttackTime = Now;
			Call.LastHostileTime = Now;
			return;
		}
	}

	FDistressCall& Call = Calls.AddDefaulted_GetRef();
	Call.Id = NextCallId++;
	Call.Victim = Victim;
	Call.Location = Victim->GetActorLocation();
	Call.LastAttackTime = Now;
	Call.LastHostileTime = Now;
	Call.bPlayerCall = Victim->IsPlayerControlled();
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	Call.Security = Galaxy ? Galaxy->GetCurrentSecurity(this) : ESectorSecurity::None;

	int32 MaxPatrols = 0;
	float Delay = 0.0f;
	GetResponse(Call.Security, MaxPatrols, Delay);
	Call.DispatchTime = Now + Delay;

	UE_LOG(LogAdastreaCombat, Log, TEXT("Distress call %d: %s attacked by %s (security %s)"), Call.Id, *Victim->GetName(),
		*GetNameSafe(Attacker), *UGalaxySubsystem::SecurityToString(Call.Security));
	if (MaxPatrols == 0)
	{
		Call.bDispatched = true;
		Message(Call, TEXT("Distress call sent. No one answers out here."), 5.0f, true);
	}
	else if (Delay > 0.0f)
	{
		Message(Call, FString::Printf(TEXT("Distress call sent. %s security: patrols in about %.0f s."),
			*UGalaxySubsystem::SecurityToString(Call.Security), Delay), 5.0f);
	}
}

bool UDistressSubsystem::GetCallLocation(int32 CallId, FVector& OutLocation) const
{
	for (const FDistressCall& Call : Calls)
	{
		if (Call.Id == CallId)
		{
			OutLocation = Call.Location;
			return true;
		}
	}
	return false;
}

bool UDistressSubsystem::AnyHostileNear(const FVector& Location) const
{
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		if (!It->IsWrecked() && AHostileFighterController::IsHostileShip(*It)
			&& FVector::DistSquared(It->GetActorLocation(), Location) <= FMath::Square(CallRadius))
		{
			return true;
		}
	}
	return false;
}

FString UDistressSubsystem::DescribeResponders(const TArray<APatrolController*>& Patrols) const
{
	// "2 Adastrea Trade Compact patrols are", or "2 patrols are" when they have no owner.
	FString Owner;
	if (const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this))
	{
		if (const UOwnershipComponent* Ownership = UOwnershipComponent::Find(Patrols[0]->GetPawn()); Ownership && !Ownership->OwnerId.IsNone())
		{
			Owner = Orgs->GetOrgDisplayName(Ownership->OwnerId).ToString();
		}
	}
	if (Owner.IsEmpty())
	{
		return Patrols.Num() == 1 ? FString(TEXT("A patrol is")) : FString::Printf(TEXT("%d patrols are"), Patrols.Num());
	}
	return Patrols.Num() == 1 ? FString::Printf(TEXT("One %s patrol is"), *Owner) : FString::Printf(TEXT("%d %s patrols are"), Patrols.Num(), *Owner);
}

void UDistressSubsystem::Dispatch(FDistressCall& Call)
{
	Call.bDispatched = true;
	int32 MaxPatrols = 0;
	float Delay = 0.0f;
	GetResponse(Call.Security, MaxPatrols, Delay);

	// Free patrols, nearest first.
	TArray<APatrolController*> Free;
	for (TActorIterator<APatrolController> It(GetWorld()); It; ++It)
	{
		const APawn* Ship = It->GetPawn();
		if (Ship && It->GetCallId() == INDEX_NONE)
		{
			Free.Add(*It);
		}
	}
	Free.Sort([&Call](const APatrolController& A, const APatrolController& B)
	{
		return FVector::DistSquared(A.GetPawn()->GetActorLocation(), Call.Location) < FVector::DistSquared(B.GetPawn()->GetActorLocation(), Call.Location);
	});
	if (Free.Num() > MaxPatrols)
	{
		Free.SetNum(MaxPatrols);
	}

	if (Free.IsEmpty())
	{
		UE_LOG(LogAdastreaCombat, Log, TEXT("Distress call %d: no free patrols"), Call.Id);
		Message(Call, TEXT("Distress call sent. No patrols near enough to answer."), 5.0f, true);
		return;
	}
	for (APatrolController* Patrol : Free)
	{
		Patrol->AnswerCall(Call.Id);
		Call.Responders.Add(Patrol);
	}
	const float Km = FVector::Dist(Free[0]->GetPawn()->GetActorLocation(), Call.Location) / 100000.0f;
	Message(Call, FString::Printf(TEXT("%s answering your distress call, %.1f km out."), *DescribeResponders(Free), Km), 5.0f);
}

void UDistressSubsystem::Message(const FDistressCall& Call, const FString& Text, float Seconds, bool bWarning) const
{
	UE_LOG(LogAdastreaCombat, Log, TEXT("Distress %d: %s"), Call.Id, *Text);
	if (!Call.bPlayerCall)
	{
		return;
	}
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowHUDMessage(Text, Seconds, bWarning);
	}
}

void UDistressSubsystem::Tick(float DeltaTime)
{
	const double Now = GetWorld()->GetTimeSeconds();
	for (int32 i = Calls.Num() - 1; i >= 0; --i)
	{
		FDistressCall& Call = Calls[i];
		if (const ASpaceship* Victim = Call.Victim.Get())
		{
			Call.Location = Victim->GetActorLocation();
		}
		if (AnyHostileNear(Call.Location))
		{
			Call.LastHostileTime = Now;
		}
		if (!Call.bDispatched && Now >= Call.DispatchTime)
		{
			Dispatch(Call);
		}

		const bool bQuiet = Now - Call.LastHostileTime >= Distress::QuietSecondsToClose;
		if (bQuiet || Now - Call.LastAttackTime >= Distress::MaxSecondsSinceAttack)
		{
			UE_LOG(LogAdastreaCombat, Log, TEXT("Distress call %d closed (%s)"), Call.Id, bQuiet ? TEXT("area clear") : TEXT("timed out"));
			if (bQuiet && !Call.Responders.IsEmpty())
			{
				Message(Call, TEXT("Patrol: area secure. Distress call closed."), 4.0f);
			}
			Calls.RemoveAt(i);
		}
	}
}
