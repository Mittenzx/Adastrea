#include "Combat/RaidSubsystem.h"
#include "AI/AIPilotController.h"
#include "AI/PatrolController.h"
#include "AI/RaiderController.h"
#include "Combat/CombatTestDirector.h"
#include "Combat/ShipHealthComponent.h"
#include "Drones/LootDrone.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Universe/GalaxySubsystem.h"
#include "Universe/OrganisationSubsystem.h"
#include "Universe/OwnershipComponent.h"
#include "Universe/PirateSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace RaidTuning
{
	const FName RaiderTag(TEXT("Raider"));
	const TCHAR* RaiderShip = TEXT("/Game/Blueprints/Ships/BP_Ship_Fighter.BP_Ship_Fighter_C");

	/** AI targets answer a hail after this long (s). */
	constexpr float AIAnswerSeconds = 3.0f;
	/** Give up on a target not reached in this long (s). */
	constexpr float ApproachSeconds = 150.0f;
	/** Give up on a target out of reach (docked, hidden) this long (s). */
	constexpr float LostTargetSeconds = 20.0f;
	/** Give up on a fight this long (s). */
	constexpr float AttackSeconds = 240.0f;
	/** Drones get this long to finish (s). */
	constexpr float DroneSeconds = 90.0f;
	/** A complying target that moves further than this from where it agreed has refused (cm). */
	constexpr float LeashDistance = 100000.0f;

	/** Patrols this close to a target count as its escort (cm). */
	constexpr float EscortRadius = 300000.0f;
	/** Need points a unit of risk costs, before desperation discounts it. */
	constexpr float RiskCost = 4.0f;
	/** A target must be worth at least this many points over its risk. */
	constexpr float MinScore = 2.0f;
	/** Unarmed AI traders are what raiders prefer. */
	constexpr float TraderPreference = 1.25f;

	constexpr float FirstAutoRaidSeconds = 120.0f;
	constexpr float AutoRaidInterval = 60.0f;

	float SecurityRisk(ESectorSecurity Security)
	{
		switch (Security)
		{
		case ESectorSecurity::Low:		return 0.5f;
		case ESectorSecurity::Medium:	return 1.2f;
		case ESectorSecurity::High:		return 2.5f;
		default:						return 0.0f;
		}
	}

	/** How readily a gang raids a sector of this security at all (auto-raids). */
	float SecurityAppetite(ESectorSecurity Security)
	{
		switch (Security)
		{
		case ESectorSecurity::Low:		return 0.8f;
		case ESectorSecurity::Medium:	return 0.4f;
		case ESectorSecurity::High:		return 0.1f;
		default:						return 1.0f;
		}
	}

	FString NeedPhrase(EPirateNeed Need)
	{
		switch (Need)
		{
		case EPirateNeed::Supplies:		return TEXT("We're running short of fuel and food.");
		case EPirateNeed::Parts:		return TEXT("We need parts to keep flying.");
		case EPirateNeed::Information:	return TEXT("We want your data.");
		default:						return TEXT("Times are hard out here.");
		}
	}

	FString DescribeGoods(const FRaidDemand& Demand)
	{
		TArray<FString> Parts;
		for (const FRaidGoods& Line : Demand.Goods)
		{
			if (const UTradeItemDataAsset* Item = Line.Item.Get())
			{
				Parts.Add(FString::Printf(TEXT("%d %s"), Line.Units, *Item->ItemName.ToString()));
			}
		}
		if (Demand.Credits > 0)
		{
			Parts.Add(FString::Printf(TEXT("%d credits"), Demand.Credits));
		}
		if (Parts.Num() <= 1)
		{
			return Parts.IsEmpty() ? FString() : Parts[0];
		}
		const FString Last = Parts.Pop();
		return FString::Join(Parts, TEXT(", ")) + TEXT(" and ") + Last;
	}

	bool IsOnFoot(const ASpaceship* Ship)
	{
		return !Ship || Ship->IsHidden();
	}

	TAutoConsoleVariable<int32> CVarAutoRaids(
		TEXT("adastrea.AutoRaids"), 1,
		TEXT("1: pirate gangs raid the level on their own when a target is worth the risk (not in CombatTest). 0: only adastrea.Raid."));

	TAutoConsoleVariable<int32> CVarAIComply(
		TEXT("adastrea.RaidAIComply"), -1,
		TEXT("How AI traders answer pirate demands: -1 weigh it up, 0 always refuse, 1 always comply."));
}

namespace
{
	static FAutoConsoleCommandWithWorldAndArgs GRaidCmd(
		TEXT("adastrea.Raid"),
		TEXT("Start a pirate raid now. Usage: adastrea.Raid [GangId=first that can] [TargetName=the gang's choice] [SpawnDistanceM=2500]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			URaidSubsystem* Raids = URaidSubsystem::Get(World);
			const UPirateSubsystem* Pirates = UPirateSubsystem::Get(World);
			if (!Raids || !Pirates)
			{
				return;
			}
			FName Gang = Args.Num() >= 1 ? FName(*Args[0]) : NAME_None;
			if (Gang.IsNone() || Gang == FName(TEXT("any")))
			{
				for (const FPirateGang& Candidate : Pirates->GetGangs())
				{
					if (UPirateSubsystem::GetRaidStrength(Candidate) > 0)
					{
						Gang = Candidate.OrgId;
						break;
					}
				}
			}
			ASpaceship* Target = nullptr;
			if (Args.Num() >= 2 && Args[1] != TEXT("any"))
			{
				for (TActorIterator<ASpaceship> It(World); It; ++It)
				{
					if (It->GetName().Contains(Args[1]) || It->GetShipName().ToString().Contains(Args[1]))
					{
						Target = *It;
						break;
					}
				}
				if (!Target)
				{
					UE_LOG(LogAdastreaCombat, Warning, TEXT("adastrea.Raid: no ship named like '%s'"), *Args[1]);
					return;
				}
			}
			const float Distance = Args.Num() >= 3 ? FCString::Atof(*Args[2]) * 100.0f : 250000.0f;
			Raids->StartRaid(Gang, Target, Distance);
		}));

	static FAutoConsoleCommandWithWorld GClearRaidsCmd(
		TEXT("adastrea.ClearRaids"),
		TEXT("End every raid and remove its raiders and drones."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (URaidSubsystem* Raids = URaidSubsystem::Get(World))
			{
				Raids->ClearRaids();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GRaidTraderCmd(
		TEXT("adastrea.RaidTrader"),
		TEXT("Test: spawn an AI trader (flying the player's ship class) beside the player with cargo. Usage: adastrea.RaidTrader ItemName Units [DistanceM=400]  (ItemName as in DA_TradeItem_<ItemName>, e.g. Helium-3)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
			if (!Cast<ASpaceship>(Player) || Args.Num() < 2)
			{
				UE_LOG(LogAdastreaCombat, Warning, TEXT("Usage (flying a ship): adastrea.RaidTrader ItemName Units [DistanceM]"));
				return;
			}
			const FString Path = FString::Printf(TEXT("/Game/DataAssets/Trading/Items/DA_TradeItem_%s.DA_TradeItem_%s"), *Args[0], *Args[0]);
			UTradeItemDataAsset* Item = LoadObject<UTradeItemDataAsset>(nullptr, *Path);
			if (!Item)
			{
				UE_LOG(LogAdastreaCombat, Warning, TEXT("adastrea.RaidTrader: no item at %s"), *Path);
				return;
			}
			ASpaceship* Ship = AAIPilotController::SpawnAITrader(World, Player->GetClass(), nullptr);
			if (!Ship)
			{
				return;
			}
			const float Distance = Args.Num() >= 3 ? FCString::Atof(*Args[2]) * 100.0f : 40000.0f;
			Ship->SetActorLocation(Player->GetActorLocation() + Player->GetActorRightVector() * Distance, false, nullptr, ETeleportType::TeleportPhysics);
			Ship->SetActorRotation(Player->GetActorRotation());
			const int32 Units = FMath::Max(1, FCString::Atoi(*Args[1]));
			const bool bLoaded = Ship->CargoComponent && Ship->CargoComponent->AddCargo(Item, Units);
			UE_LOG(LogAdastreaCombat, Log, TEXT("adastrea.RaidTrader: %s with %d %s (%s), %.0f m from the player"), *Ship->GetName(), Units,
				*Item->ItemName.ToString(), bLoaded ? TEXT("loaded") : TEXT("NO ROOM"), Distance / 100.0f);
		}));
}

URaidSubsystem* URaidSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<URaidSubsystem>() : nullptr;
}

bool URaidSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId URaidSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URaidSubsystem, STATGROUP_Tickables);
}

void URaidSubsystem::Deinitialize()
{
	Raids.Reset();
	Super::Deinitialize();
}

FString URaidSubsystem::PhaseToString(ERaidPhase Phase)
{
	switch (Phase)
	{
	case ERaidPhase::Approach:	return TEXT("approaching");
	case ERaidPhase::Hail:		return TEXT("hailing");
	case ERaidPhase::Collect:	return TEXT("collecting");
	case ERaidPhase::Attack:	return TEXT("attacking");
	case ERaidPhase::Strip:		return TEXT("stripping");
	default:					return TEXT("retreating");
	}
}

bool URaidSubsystem::IsRaiderShip(const AActor* Actor)
{
	return Actor && Actor->ActorHasTag(RaidTuning::RaiderTag);
}

const FRaid* URaidSubsystem::FindRaid(int32 RaidId) const
{
	return Raids.FindByPredicate([RaidId](const FRaid& R) { return R.Id == RaidId; });
}

FRaid* URaidSubsystem::FindRaidMutable(int32 RaidId)
{
	return Raids.FindByPredicate([RaidId](const FRaid& R) { return R.Id == RaidId; });
}

ERaidPhase URaidSubsystem::GetPhase(int32 RaidId) const
{
	const FRaid* Raid = FindRaid(RaidId);
	return Raid ? Raid->Phase : ERaidPhase::Retreat;
}

ASpaceship* URaidSubsystem::GetRaidTarget(int32 RaidId) const
{
	const FRaid* Raid = FindRaid(RaidId);
	return Raid ? Raid->Target.Get() : nullptr;
}

const FRaid* URaidSubsystem::GetPlayerHail() const
{
	for (const FRaid& Raid : Raids)
	{
		if (Raid.bTargetIsPlayer && (Raid.Phase == ERaidPhase::Hail || Raid.Phase == ERaidPhase::Collect))
		{
			return &Raid;
		}
	}
	return nullptr;
}

float URaidSubsystem::GetPlayerAnswerSecondsLeft() const
{
	const FRaid* Raid = GetPlayerHail();
	return Raid && Raid->Phase == ERaidPhase::Hail ? FMath::Max(0.0f, PlayerAnswerSeconds - Raid->PhaseSeconds) : 0.0f;
}

// ---------------------------------------------------------------------------
// Targets
// ---------------------------------------------------------------------------

namespace
{
	/** Whether a raid could go after Ship at all. */
	bool IsCandidate(const ASpaceship* Ship)
	{
		if (!Ship || Ship->IsHidden() || URaidSubsystem::IsRaiderShip(Ship) || AHostileFighterController::IsHostileShip(Ship)
			|| APatrolController::IsPatrolShip(Ship))
		{
			return false;
		}
		if (Ship->HealthComponent && Ship->HealthComponent->bTrainingTarget)
		{
			return false;
		}
		if (Ship->IsWrecked())
		{
			return true;
		}
		if (Ship->IsDocked() || Ship->IsDocking())
		{
			return false;
		}
		return Ship->IsPlayerControlled() || Cast<AAIPilotController>(Ship->GetController()) != nullptr;
	}

	/** Need points a demand is worth to the gang: each line's points weighed by how short of that need the gang is. */
	float DemandValue(const FPirateGang& Gang, const FRaidDemand& Demand)
	{
		float Value = 0.0f;
		for (const FRaidGoods& Line : Demand.Goods)
		{
			const EPirateNeed Need = UPirateSubsystem::NeedForItem(Line.Item.Get());
			Value += UPirateSubsystem::PointsForGoods(Line.Item.Get(), Line.Units) * UPirateSubsystem::GetShortage(Gang, Need) * UPirateSubsystem::GetImportance(Need);
		}
		Value += UPirateSubsystem::PointsForCredits(Demand.Credits) * UPirateSubsystem::GetShortage(Gang, EPirateNeed::Cash) * UPirateSubsystem::GetImportance(EPirateNeed::Cash);
		return Value;
	}
}

FRaidDemand URaidSubsystem::BuildDemand(const FPirateGang& Gang, const ASpaceship* Ship, float Desperation) const
{
	FRaidDemand Demand;
	if (!Ship || !Ship->CargoComponent)
	{
		return Demand;
	}
	const bool bWreck = Ship->IsWrecked();
	// A wreck gives up everything; a live ship is asked for a share, more the more desperate the gang is.
	const float Share = bWreck ? 1.0f : 0.5f + 0.5f * Desperation;

	struct FCandidate { FRaidGoods Goods; float Weight; bool bWanted; };
	TArray<FCandidate> Candidates;
	for (const FCargoEntry& Entry : Ship->CargoComponent->CargoInventory)
	{
		if (!Entry.Item || Entry.Quantity <= 0)
		{
			continue;
		}
		const EPirateNeed Need = UPirateSubsystem::NeedForItem(Entry.Item);
		FCandidate& C = Candidates.AddDefaulted_GetRef();
		C.Goods.Item = Entry.Item;
		C.Goods.Units = FMath::Clamp(FMath::CeilToInt(Entry.Quantity * Share), 1, Entry.Quantity);
		C.Weight = UPirateSubsystem::PointsForGoods(Entry.Item, C.Goods.Units) * UPirateSubsystem::GetShortage(Gang, Need) * UPirateSubsystem::GetImportance(Need);
		C.bWanted = bWreck || UPirateSubsystem::GetShortage(Gang, Need) >= 0.25f;
	}
	Candidates.Sort([](const FCandidate& A, const FCandidate& B) { return A.Weight > B.Weight; });
	for (const FCandidate& C : Candidates)
	{
		if (C.bWanted && (bWreck || Demand.Goods.Num() < 3))
		{
			Demand.Goods.Add(C.Goods);
		}
	}
	// Nothing they need aboard: the most valuable cargo, for the fence.
	if (Demand.Goods.IsEmpty() && !Candidates.IsEmpty())
	{
		Demand.Goods.Add(Candidates[0].Goods);
	}

	const UPlayerTraderComponent* Trader = Ship->PlayerTraderComponent;
	if (!bWreck && Trader && Trader->GetCredits() > 0 && UPirateSubsystem::GetShortage(Gang, EPirateNeed::Cash) >= 0.3f)
	{
		const float Cap = 2000.0f + 8000.0f * Desperation;
		const int32 Credits = FMath::FloorToInt(FMath::Min(Trader->GetCredits() * 0.1f * (1.0f + Desperation), Cap) / 100.0f) * 100;
		Demand.Credits = FMath::Max(0, Credits);
	}

	Demand.Need = Demand.Goods.IsEmpty() ? EPirateNeed::Cash : UPirateSubsystem::NeedForItem(Demand.Goods[0].Item.Get());
	if (!Demand.IsEmpty())
	{
		Demand.Line = RaidTuning::NeedPhrase(Demand.Need) + TEXT(" Hand over ") + RaidTuning::DescribeGoods(Demand) + TEXT(" and we let you go.");
		if (Desperation >= 0.6f)
		{
			Demand.Line += TEXT(" Don't test us. We've nothing left to lose.");
		}
	}
	return Demand;
}

int32 URaidSubsystem::CountPatrolsNear(const FVector& Location, float Radius) const
{
	int32 Count = 0;
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		if (APatrolController::IsPatrolShip(*It) && !It->IsWrecked() && FVector::DistSquared(It->GetActorLocation(), Location) <= FMath::Square(Radius))
		{
			++Count;
		}
	}
	return Count;
}

int32 URaidSubsystem::GetHopsFromHome(const FPirateGang& Gang) const
{
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	const FOrgDef* Org = Orgs ? Orgs->FindOrg(Gang.OrgId) : nullptr;
	const FName Here = Galaxy ? Galaxy->ResolveCurrentSectorId(this) : NAME_None;
	if (!Galaxy || !Org || Here.IsNone() || !Galaxy->FindSector(Here))
	{
		// A level outside the galaxy (CombatTest): next door to everyone.
		return 1;
	}
	if (Here == Org->HomeSectorId)
	{
		return 0;
	}
	const TArray<FName> Route = Galaxy->FindRoute(Org->HomeSectorId, Here);
	return Route.IsEmpty() ? 99 : Route.Num();
}

void URaidSubsystem::ScoreTarget(const FPirateGang& Gang, const ASpaceship* Ship, float& OutValue, float& OutRisk) const
{
	const float Desperation = UPirateSubsystem::GetDesperation(Gang);
	OutValue = DemandValue(Gang, BuildDemand(Gang, Ship, Desperation));
	if (Ship->IsWrecked())
	{
		OutValue += UPirateSubsystem::WreckSalvagePoints * UPirateSubsystem::GetShortage(Gang, EPirateNeed::Parts) * UPirateSubsystem::GetImportance(EPirateNeed::Parts);
	}
	else if (!Ship->IsPlayerControlled())
	{
		OutValue *= RaidTuning::TraderPreference;
	}

	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	OutRisk = RaidTuning::SecurityRisk(Galaxy ? Galaxy->GetCurrentSecurity(this) : ESectorSecurity::None);
	OutRisk += 0.8f * CountPatrolsNear(Ship->GetActorLocation(), RaidTuning::EscortRadius);
	// The player flies an armed ship; a wreck can't fight back.
	OutRisk += (!Ship->IsWrecked() && Ship->IsPlayerControlled()) ? 1.0f : 0.0f;
	OutRisk += 0.3f * FMath::Max(0, GetHopsFromHome(Gang) - 1);
}

float URaidSubsystem::ComputeScore(const FPirateGang& Gang, const ASpaceship* Ship) const
{
	float Value = 0.0f, Risk = 0.0f;
	ScoreTarget(Gang, Ship, Value, Risk);
	// A desperate gang shrugs off risk it would never take when comfortable.
	return Value - Risk * RaidTuning::RiskCost / (1.0f + 2.0f * UPirateSubsystem::GetDesperation(Gang));
}

float URaidSubsystem::GetTargetScore(FName GangId, ASpaceship* Ship) const
{
	const UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	const FPirateGang* Gang = Pirates ? Pirates->FindGang(GangId) : nullptr;
	return Gang && Ship ? ComputeScore(*Gang, Ship) : 0.0f;
}

TArray<FString> URaidSubsystem::DescribeRaids() const
{
	TArray<FString> Out;
	for (const FRaid& Raid : Raids)
	{
		Out.Add(FString::Printf(TEXT("%d %s %s %s raiders=%d drones=%d"), Raid.Id, *Raid.GangId.ToString(), *PhaseToString(Raid.Phase),
			*GetNameSafe(Raid.Target.Get()), Raid.Raiders.Num(), Raid.Drones.Num()));
	}
	return Out;
}

ASpaceship* URaidSubsystem::ChooseTarget(const FPirateGang& Gang, float& OutScore) const
{
	ASpaceship* Best = nullptr;
	OutScore = RaidTuning::MinScore;
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		ASpaceship* Ship = *It;
		if (!IsCandidate(Ship) || Raids.ContainsByPredicate([Ship](const FRaid& R) { return R.Target.Get() == Ship; }))
		{
			continue;
		}
		const float Score = ComputeScore(Gang, Ship);
		if (Score > OutScore)
		{
			OutScore = Score;
			Best = Ship;
		}
	}
	return Best;
}

// ---------------------------------------------------------------------------
// Raids
// ---------------------------------------------------------------------------

int32 URaidSubsystem::StartRaid(FName GangId, ASpaceship* Target, float SpawnDistance)
{
	UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	const FPirateGang* Gang = Pirates ? Pirates->FindGang(GangId) : nullptr;
	if (!Gang)
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Raid: '%s' is not a pirate gang (adastrea.PirateInfo lists them)"), *GangId.ToString());
		return INDEX_NONE;
	}
	const int32 Strength = UPirateSubsystem::GetRaidStrength(*Gang);
	if (Strength <= 0)
	{
		UE_LOG(LogAdastreaCombat, Log, TEXT("Raid: %s has no ships to spare (parts %.0f)"), *GangId.ToString(), Gang->Needs.Get(EPirateNeed::Parts));
		return INDEX_NONE;
	}
	float Score = 0.0f;
	if (!Target)
	{
		Target = ChooseTarget(*Gang, Score);
	}
	if (!Target)
	{
		UE_LOG(LogAdastreaCombat, Log, TEXT("Raid: %s finds nothing here worth the risk"), *GangId.ToString());
		return INDEX_NONE;
	}

	FRaid& Raid = Raids.AddDefaulted_GetRef();
	Raid.Id = NextRaidId++;
	Raid.GangId = GangId;
	Raid.Target = Target;
	Raid.bTargetIsPlayer = Target->IsPlayerControlled();
	Raid.Desperation = UPirateSubsystem::GetDesperation(*Gang);

	// Lie in wait ahead of a moving target (raiders fly no faster than their prey), else
	// come in from a random bearing, roughly level with it.
	FVector Heading = Target->GetVelocity().SizeSquared() > FMath::Square(500.0f) ? Target->GetVelocity().GetSafeNormal() : FVector::ZeroVector;
	if (const AAIPilotController* Pilot = Cast<AAIPilotController>(Target->GetController()); Heading.IsZero() && Pilot && Pilot->TargetStation)
	{
		Heading = (Pilot->TargetStation->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	}
	const FRotator Bearing = !Heading.IsZero()
		? Heading.Rotation() + FRotator(FMath::FRandRange(-10.0f, 10.0f), FMath::FRandRange(-30.0f, 30.0f), 0.0f)
		: FRotator(FMath::FRandRange(-10.0f, 10.0f), FMath::FRandRange(0.0f, 360.0f), 0.0f);
	const FVector Centre = Target->GetActorLocation() + Bearing.Vector() * SpawnDistance;
	const FRotator Facing = (Target->GetActorLocation() - Centre).Rotation();
	const FVector Side = FRotationMatrix(Facing).GetUnitAxis(EAxis::Y);
	TSubclassOf<ASpaceship> ShipClass = StaticLoadClass(ASpaceship::StaticClass(), nullptr, RaidTuning::RaiderShip);
	for (int32 i = 0; i < Strength; ++i)
	{
		const FVector Location = Centre + Side * (i - (Strength - 1) * 0.5f) * 4000.0f;
		ASpaceship* Ship = AHostileFighterController::SpawnHostile(GetWorld(), ShipClass, Location, Facing, ARaiderController::StaticClass());
		ARaiderController* Pilot = Ship ? Cast<ARaiderController>(Ship->GetController()) : nullptr;
		if (!Pilot)
		{
			continue;
		}
		Ship->Tags.AddUnique(RaidTuning::RaiderTag);
		Pilot->JoinRaid(Raid.Id, GangId);
		UOwnershipComponent* Ownership = NewObject<UOwnershipComponent>(Ship, TEXT("Ownership"));
		Ownership->OwnerId = GangId;
		Ownership->RegisterComponent();
		Raid.Raiders.Add(Pilot);
	}

	float Value = 0.0f, Risk = 0.0f;
	ScoreTarget(*Gang, Target, Value, Risk);
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d: %s sends %d raider(s) at %s (value %.1f, risk %.1f, short of %s, desperation %.2f)"),
		Raid.Id, *GangId.ToString(), Raid.Raiders.Num(), *Target->GetName(), Value, Risk,
		*UPirateSubsystem::NeedToString(UPirateSubsystem::GetBiggestShortage(*Gang)), Raid.Desperation);
	if (Raid.Raiders.IsEmpty())
	{
		Raids.Pop();
		return INDEX_NONE;
	}
	return Raid.Id;
}

void URaidSubsystem::ClearRaids()
{
	for (FRaid& Raid : Raids)
	{
		ReleaseTarget(Raid);
		for (const TWeakObjectPtr<ARaiderController>& Weak : Raid.Raiders)
		{
			if (ARaiderController* Pilot = Weak.Get())
			{
				APawn* Ship = Pilot->GetPawn();
				Pilot->UnPossess();
				Pilot->Destroy();
				if (Ship)
				{
					Ship->Destroy();
				}
			}
		}
		for (const TWeakObjectPtr<ALootDrone>& Drone : Raid.Drones)
		{
			if (Drone.IsValid())
			{
				Drone->Destroy();
			}
		}
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raids cleared (%d)"), Raids.Num());
	Raids.Reset();
}

ASpaceship* URaidSubsystem::GetLeadShip(const FRaid& Raid) const
{
	const ASpaceship* Target = Raid.Target.Get();
	ASpaceship* Lead = nullptr;
	float BestDist = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<ARaiderController>& Weak : Raid.Raiders)
	{
		const ARaiderController* Pilot = Weak.Get();
		ASpaceship* Ship = Pilot ? Cast<ASpaceship>(Pilot->GetPawn()) : nullptr;
		if (!Ship || Ship->IsWrecked() || Pilot->IsFleeing())
		{
			continue;
		}
		const float Dist = Target ? FVector::Dist(Ship->GetActorLocation(), Target->GetActorLocation()) : 0.0f;
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Lead = Ship;
		}
	}
	return Lead;
}

bool URaidSubsystem::AllRaidersFleeing(const FRaid& Raid) const
{
	for (const TWeakObjectPtr<ARaiderController>& Weak : Raid.Raiders)
	{
		if (Weak.IsValid() && !Weak->IsFleeing())
		{
			return false;
		}
	}
	return true;
}

void URaidSubsystem::SetPhase(FRaid& Raid, ERaidPhase Phase)
{
	if (Raid.Phase == Phase)
	{
		return;
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d (%s): %s -> %s"), Raid.Id, *Raid.GangId.ToString(), *PhaseToString(Raid.Phase), *PhaseToString(Phase));
	Raid.Phase = Phase;
	Raid.PhaseSeconds = 0.0f;
	Raid.LostTargetSeconds = 0.0f;
	if (Phase == ERaidPhase::Retreat)
	{
		ReleaseTarget(Raid);
	}
}

void URaidSubsystem::ReleaseTarget(FRaid& Raid)
{
	if (ASpaceship* Target = Raid.Target.Get())
	{
		if (AAIPilotController* Pilot = Cast<AAIPilotController>(Target->GetController()); Pilot && Pilot->IsHeldByRaiders())
		{
			Pilot->SetHeldByRaiders(false);
		}
	}
}

void URaidSubsystem::TellPlayer(const FRaid& Raid, const FString& Text, float Seconds, bool bWarning) const
{
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d: %s"), Raid.Id, *Text);
	if (!Raid.bTargetIsPlayer)
	{
		return;
	}
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowHUDMessage(Text, Seconds, bWarning);
	}
}

bool URaidSubsystem::DecideAIComply(const FRaid& Raid) const
{
	const int32 Forced = RaidTuning::CVarAIComply.GetValueOnGameThread();
	if (Forced >= 0)
	{
		return Forced > 0;
	}
	const ASpaceship* Target = Raid.Target.Get();
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	const ESectorSecurity Security = Galaxy ? Galaxy->GetCurrentSecurity(this) : ESectorSecurity::None;
	// A trader complies unless help is close: patrols nearby, or a sector that answers distress calls fast.
	float Chance = 0.8f + 0.1f * (Raid.Raiders.Num() - 1);
	Chance -= 0.25f * (Target ? CountPatrolsNear(Target->GetActorLocation(), RaidTuning::EscortRadius) : 0);
	Chance -= Security == ESectorSecurity::High ? 0.3f : (Security == ESectorSecurity::Medium ? 0.15f : 0.0f);
	return FMath::FRand() < Chance;
}

void URaidSubsystem::BeginHail(FRaid& Raid)
{
	const UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	const FPirateGang* Gang = Pirates ? Pirates->FindGang(Raid.GangId) : nullptr;
	Raid.Demand = Gang ? BuildDemand(*Gang, Raid.Target.Get(), Raid.Desperation) : FRaidDemand();
	if (Raid.Demand.IsEmpty())
	{
		// Nothing aboard worth asking for: take it by force, or rather not bother.
		UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d: %s has nothing worth demanding; leaving"), Raid.Id, *GetNameSafe(Raid.Target.Get()));
		SetPhase(Raid, ERaidPhase::Retreat);
		return;
	}
	SetPhase(Raid, ERaidPhase::Hail);
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d hails %s: \"%s\""), Raid.Id, *GetNameSafe(Raid.Target.Get()), *Raid.Demand.Line);
}

void URaidSubsystem::AnswerPlayerHail(bool bComply)
{
	for (FRaid& Raid : Raids)
	{
		if (Raid.bTargetIsPlayer && Raid.Phase == ERaidPhase::Hail)
		{
			Answer(Raid, bComply, TEXT("the player answered"));
			return;
		}
	}
}

void URaidSubsystem::Answer(FRaid& Raid, bool bComply, const TCHAR* Why)
{
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d: %s %s (%s)"), Raid.Id, *GetNameSafe(Raid.Target.Get()), bComply ? TEXT("complies") : TEXT("refuses"), Why);
	if (bComply)
	{
		SetPhase(Raid, ERaidPhase::Collect);
		Raid.ComplyLocation = Raid.Target.IsValid() ? Raid.Target->GetActorLocation() : FVector::ZeroVector;
		if (AAIPilotController* Pilot = Raid.Target.IsValid() ? Cast<AAIPilotController>(Raid.Target->GetController()) : nullptr)
		{
			Pilot->SetHeldByRaiders(true);
		}
		return;
	}
	ReleaseTarget(Raid);
	SetPhase(Raid, ERaidPhase::Attack);
	TellPlayer(Raid, TEXT("Demand refused. The raiders are attacking!"), 4.0f, true);
}

void URaidSubsystem::LaunchDrones(FRaid& Raid, bool bStrip)
{
	Raid.bDronesLaunched = true;
	ASpaceship* Home = GetLeadShip(Raid);
	ASpaceship* Target = Raid.Target.Get();
	if (!Home || !Target)
	{
		return;
	}
	if (bStrip)
	{
		// Everything aboard the wreck.
		const UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
		const FPirateGang* Gang = Pirates ? Pirates->FindGang(Raid.GangId) : nullptr;
		Raid.Demand = Gang ? BuildDemand(*Gang, Target, 1.0f) : FRaidDemand();
	}
	const int32 Count = bStrip ? 2 : FMath::Clamp(Raid.Demand.Goods.Num() + (Raid.Demand.Credits > 0 ? 1 : 0), 1, 2);
	Raid.DroneCount = Count;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (int32 Slot = 0; Slot < Count; ++Slot)
	{
		const FVector Start = Home->GetActorLocation() - Home->GetActorUpVector() * 300.0f;
		if (ALootDrone* Drone = GetWorld()->SpawnActor<ALootDrone>(ALootDrone::StaticClass(), Start, Home->GetActorRotation(), Params))
		{
			Drone->StartJob(Raid.Id, Home, Target, Slot);
			Raid.Drones.Add(Drone);
		}
	}
	if (!bStrip)
	{
		TellPlayer(Raid, TEXT("Their drones are coming for the cargo. Hold still."), 4.0f, false);
	}
}

void URaidSubsystem::NotifyDroneAtTarget(ALootDrone* Drone)
{
	FRaid* Raid = Drone ? FindRaidMutable(Drone->GetRaidId()) : nullptr;
	ASpaceship* Target = Drone ? Drone->GetTarget() : nullptr;
	if (!Raid || !Target)
	{
		return;
	}
	const int32 Count = FMath::Max(1, Raid->DroneCount);
	const int32 Slot = Drone->GetSlot();
	UCargoComponent* Cargo = Target->CargoComponent;
	TArray<FString> Taken;
	// Cargo lines are dealt out to the drones in turn, skipping the credits drone while there are enough others.
	const bool bCreditsDrone = Raid->Demand.Credits > 0 && !Target->IsWrecked() && Count > 1;
	const int32 CargoDrones = bCreditsDrone ? Count - 1 : Count;
	for (int32 i = 0; i < Raid->Demand.Goods.Num(); ++i)
	{
		UTradeItemDataAsset* Item = Raid->Demand.Goods[i].Item.Get();
		if (i % CargoDrones != Slot || !Item || !Cargo)
		{
			continue;
		}
		const int32 Units = FMath::Min(Raid->Demand.Goods[i].Units, Cargo->GetItemQuantity(Item));
		if (Units > 0 && Cargo->RemoveCargo(Item, Units))
		{
			Drone->Goods.Add({ Item, Units });
			Taken.Add(FString::Printf(TEXT("%d %s"), Units, *Item->ItemName.ToString()));
		}
	}
	// Credits go with the last drone, so a demand of one cargo line and credits keeps both drones busy.
	if (Slot == Count - 1 && Raid->Demand.Credits > 0 && !Target->IsWrecked())
	{
		if (UPlayerTraderComponent* Trader = Target->PlayerTraderComponent)
		{
			const int32 Credits = FMath::Min(Raid->Demand.Credits, Trader->GetCredits());
			if (Credits > 0 && Trader->RemoveCredits(Credits))
			{
				Drone->Credits = Credits;
				Taken.Add(FString::Printf(TEXT("%d credits"), Credits));
			}
		}
	}
	if (Target->IsWrecked() && Slot == Count - 1)
	{
		Drone->SalvagePoints = UPirateSubsystem::WreckSalvagePoints;
		Taken.Add(TEXT("salvage"));
	}
	const FString What = Taken.IsEmpty() ? FString(TEXT("nothing")) : FString::Join(Taken, TEXT(", "));
	TellPlayer(*Raid, FString::Printf(TEXT("A pirate drone took %s."), *What), 4.0f, true);
}

void URaidSubsystem::NotifyDroneHome(ALootDrone* Drone)
{
	FRaid* Raid = Drone ? FindRaidMutable(Drone->GetRaidId()) : nullptr;
	UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	if (!Raid || !Pirates)
	{
		return;
	}
	for (const FRaidGoods& Goods : Drone->Goods)
	{
		Pirates->DeliverGoods(Raid->GangId, Goods.Item.Get(), Goods.Units);
	}
	Pirates->DeliverCredits(Raid->GangId, Drone->Credits);
	if (Drone->SalvagePoints > 0.0f)
	{
		Pirates->DeliverSalvage(Raid->GangId, Drone->SalvagePoints);
	}
	Raid->bBroughtSomething |= Drone->IsCarrying();
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d: loot drone %s landed%s"), Raid->Id, *Drone->GetName(), Drone->IsCarrying() ? TEXT(" with its load") : TEXT(" empty"));
}

ASpaceship* URaidSubsystem::GetDroneHome(int32 RaidId, ASpaceship* Preferred) const
{
	const FRaid* Raid = FindRaid(RaidId);
	if (!Raid)
	{
		return nullptr;
	}
	if (Preferred && !Preferred->IsWrecked() && !Preferred->IsActorBeingDestroyed()
		&& Raid->Raiders.ContainsByPredicate([Preferred](const TWeakObjectPtr<ARaiderController>& R) { return R.IsValid() && R->GetPawn() == Preferred; }))
	{
		return Preferred;
	}
	for (const TWeakObjectPtr<ARaiderController>& Weak : Raid->Raiders)
	{
		ASpaceship* Ship = Weak.IsValid() ? Cast<ASpaceship>(Weak->GetPawn()) : nullptr;
		if (Ship && !Ship->IsWrecked())
		{
			return Ship;
		}
	}
	return nullptr;
}

void URaidSubsystem::ReportAttack(ASpaceship* Victim, AActor* Attacker)
{
	ARaiderController* Pilot = Victim ? Cast<ARaiderController>(Victim->GetController()) : nullptr;
	if (!Pilot || !Attacker)
	{
		return;
	}
	Pilot->NotifyAttackedBy(Attacker);
	FRaid* Raid = FindRaidMutable(Pilot->GetRaidId());
	if (Raid && Raid->Target.Get() == Attacker
		&& (Raid->Phase == ERaidPhase::Approach || Raid->Phase == ERaidPhase::Hail || Raid->Phase == ERaidPhase::Collect))
	{
		Answer(*Raid, false, TEXT("it opened fire"));
	}
}

void URaidSubsystem::NotifyRaiderWrecked(ARaiderController* Raider)
{
	if (UPirateSubsystem* Pirates = UPirateSubsystem::Get(this))
	{
		Pirates->RecordShipLost(Raider->GetGangId());
	}
	if (FRaid* Raid = FindRaidMutable(Raider->GetRaidId()))
	{
		Raid->Raiders.Remove(Raider);
	}
}

void URaidSubsystem::NotifyRaiderLeft(ARaiderController* Raider)
{
	if (FRaid* Raid = FindRaidMutable(Raider->GetRaidId()))
	{
		Raid->Raiders.Remove(Raider);
	}
}

void URaidSubsystem::EndRaid(FRaid& Raid)
{
	ReleaseTarget(Raid);
	if (UPirateSubsystem* Pirates = UPirateSubsystem::Get(this))
	{
		Pirates->RecordRaid(Raid.GangId, Raid.bBroughtSomething);
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Raid %d (%s) over: %s, %s"), Raid.Id, *Raid.GangId.ToString(),
		Raid.Outcome.IsEmpty() ? TEXT("no outcome") : *Raid.Outcome, Raid.bBroughtSomething ? TEXT("loot brought home") : TEXT("empty-handed"));
}

void URaidSubsystem::Tick(float DeltaTime)
{
	for (int32 i = Raids.Num() - 1; i >= 0; --i)
	{
		FRaid& Raid = Raids[i];
		Raid.Raiders.RemoveAll([](const TWeakObjectPtr<ARaiderController>& R) { return !R.IsValid(); });
		Raid.Drones.RemoveAll([](const TWeakObjectPtr<ALootDrone>& D) { return !D.IsValid(); });
		if (Raid.Raiders.IsEmpty() && Raid.Drones.IsEmpty())
		{
			EndRaid(Raid);
			Raids.RemoveAt(i);
			continue;
		}
		TickRaid(Raid, DeltaTime);
	}
	TickAutoRaids(DeltaTime);
}

void URaidSubsystem::TickRaid(FRaid& Raid, float DeltaTime)
{
	Raid.PhaseSeconds += DeltaTime;
	if (Raid.Phase == ERaidPhase::Retreat)
	{
		return;
	}
	if (AllRaidersFleeing(Raid))
	{
		Raid.Outcome = TEXT("driven off");
		TellPlayer(Raid, TEXT("The raiders are breaking off."), 4.0f, false);
		SetPhase(Raid, ERaidPhase::Retreat);
		return;
	}

	ASpaceship* Target = Raid.Target.Get();
	if (!Target)
	{
		Raid.Outcome = TEXT("target gone");
		SetPhase(Raid, ERaidPhase::Retreat);
		return;
	}
	const ASpaceship* Lead = GetLeadShip(Raid);
	const float LeadDistance = Lead ? FVector::Dist(Lead->GetActorLocation(), Target->GetActorLocation()) : TNumericLimits<float>::Max();
	const bool bReachable = !Target->IsHidden() && !Target->IsDocked() && !Target->IsDocking();
	Raid.LostTargetSeconds = bReachable ? 0.0f : Raid.LostTargetSeconds + DeltaTime;
	if (Raid.LostTargetSeconds > RaidTuning::LostTargetSeconds)
	{
		Raid.Outcome = TEXT("target got away");
		SetPhase(Raid, ERaidPhase::Retreat);
		return;
	}
	// A target that's been disabled is stripped whatever was going on.
	if (Target->IsWrecked() && Raid.Phase != ERaidPhase::Strip)
	{
		SetPhase(Raid, ERaidPhase::Strip);
		Raid.bDronesLaunched = false;
		Raid.Drones.Reset();
	}

	switch (Raid.Phase)
	{
	case ERaidPhase::Approach:
		if (LeadDistance <= HailRange)
		{
			BeginHail(Raid);
		}
		else if (Raid.PhaseSeconds > RaidTuning::ApproachSeconds)
		{
			Raid.Outcome = TEXT("never caught the target");
			SetPhase(Raid, ERaidPhase::Retreat);
		}
		break;

	case ERaidPhase::Hail:
		if (Raid.bTargetIsPlayer)
		{
			if (Raid.PhaseSeconds > PlayerAnswerSeconds)
			{
				Answer(Raid, false, TEXT("no answer"));
			}
		}
		else if (Raid.PhaseSeconds > RaidTuning::AIAnswerSeconds)
		{
			Answer(Raid, DecideAIComply(Raid), TEXT("weighed it up"));
		}
		break;

	case ERaidPhase::Collect:
		if (!Raid.bDronesLaunched && LeadDistance <= DroneRange)
		{
			LaunchDrones(Raid, false);
		}
		if (FVector::Dist(Target->GetActorLocation(), Raid.ComplyLocation) > RaidTuning::LeashDistance)
		{
			TellPlayer(Raid, TEXT("Raiders: \"Running, are we? Then we do this the hard way.\""), 4.0f, true);
			Answer(Raid, false, TEXT("ran off while complying"));
		}
		else if ((Raid.bDronesLaunched && Raid.Drones.IsEmpty()) || Raid.PhaseSeconds > RaidTuning::DroneSeconds)
		{
			Raid.Outcome = TEXT("the target paid");
			TellPlayer(Raid, TEXT("The raiders have what they came for and are leaving."), 4.0f, false);
			SetPhase(Raid, ERaidPhase::Retreat);
		}
		break;

	case ERaidPhase::Attack:
		if (Raid.PhaseSeconds > RaidTuning::AttackSeconds)
		{
			Raid.Outcome = TEXT("couldn't disable the target");
			SetPhase(Raid, ERaidPhase::Retreat);
		}
		break;

	case ERaidPhase::Strip:
		if (!Raid.bDronesLaunched && LeadDistance <= DroneRange)
		{
			LaunchDrones(Raid, true);
		}
		if (Raid.bDronesLaunched && (Raid.Drones.IsEmpty() || Raid.PhaseSeconds > RaidTuning::DroneSeconds))
		{
			Raid.Outcome = TEXT("stripped the wreck");
			SetPhase(Raid, ERaidPhase::Retreat);
		}
		else if (!Raid.bDronesLaunched && Raid.PhaseSeconds > RaidTuning::ApproachSeconds)
		{
			Raid.Outcome = TEXT("never reached the wreck");
			SetPhase(Raid, ERaidPhase::Retreat);
		}
		break;

	default:
		break;
	}
}

void URaidSubsystem::TickAutoRaids(float DeltaTime)
{
	if (RaidTuning::CVarAutoRaids.GetValueOnGameThread() == 0 || ACombatTestDirector::Find(GetWorld()))
	{
		return;
	}
	AutoRaidClock += DeltaTime;
	if (AutoRaidClock < RaidTuning::FirstAutoRaidSeconds)
	{
		return;
	}
	AutoRaidClock = RaidTuning::FirstAutoRaidSeconds - RaidTuning::AutoRaidInterval;
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Cast<ASpaceship>(Player))
	{
		// Raids happen around the player: not while walking a station or ship interior.
		return;
	}
	const UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(this);
	if (!Pirates)
	{
		return;
	}
	const ESectorSecurity Security = Galaxy ? Galaxy->GetCurrentSecurity(this) : ESectorSecurity::None;
	for (const FPirateGang& Gang : Pirates->GetGangs())
	{
		if (UPirateSubsystem::GetRaidStrength(Gang) <= 0 || Raids.ContainsByPredicate([&Gang](const FRaid& R) { return R.GangId == Gang.OrgId; }))
		{
			continue;
		}
		const float Desperation = UPirateSubsystem::GetDesperation(Gang);
		// Desperate gangs range further from home and into safer space.
		const int32 Reach = 2 + FMath::RoundToInt(2.0f * Desperation);
		if (GetHopsFromHome(Gang) > Reach)
		{
			continue;
		}
		const float Chance = 0.25f * (1.0f + 2.0f * Desperation) * RaidTuning::SecurityAppetite(Security);
		if (FMath::FRand() >= Chance)
		{
			continue;
		}
		if (StartRaid(Gang.OrgId, nullptr, 300000.0f) != INDEX_NONE)
		{
			// One new raid per check.
			return;
		}
	}
}
