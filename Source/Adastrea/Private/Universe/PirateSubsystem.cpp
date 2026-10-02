#include "Universe/PirateSubsystem.h"
#include "Universe/OrganisationSubsystem.h"
#include "Trading/TradeItemDataAsset.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "AdastreaLog.h"

namespace PirateTuning
{
	/** Credits of goods (at base price) per need point. Fenced goods (cash) are worth least. */
	constexpr float CreditsPerPoint[FPirateNeeds::Num] = { 1000.0f, 1000.0f, 250.0f, 150.0f, 200.0f, 400.0f };
	constexpr float CreditsPerCashPoint = 500.0f;
	/** Need levels lost when one of the gang's ships is disabled. */
	constexpr float ShipLossParts = 12.0f;
	constexpr float ShipLossPeople = 4.0f;
	/** Parts level per ship the gang can field (ship 1 needs this much, ship 2 twice...). */
	constexpr float PartsPerRaider = 30.0f;
	constexpr float MinPartsToRaid = 8.0f;
	constexpr int32 MaxRaiders = 3;

	TAutoConsoleVariable<float> CVarUpkeepScale(
		TEXT("adastrea.PirateUpkeepScale"), 1.0f,
		TEXT("Multiplier on pirate need upkeep (0 freezes the levels, e.g. for tests)."));

	const TCHAR* NeedNames[FPirateNeeds::Num] = { TEXT("people"), TEXT("access"), TEXT("parts"), TEXT("supplies"), TEXT("information"), TEXT("cash") };

	bool ContainsAny(const FString& Text, std::initializer_list<const TCHAR*> Words)
	{
		for (const TCHAR* Word : Words)
		{
			if (Text.Contains(Word))
			{
				return true;
			}
		}
		return false;
	}
}

namespace
{
	static FAutoConsoleCommandWithWorld GPirateInfoCmd(
		TEXT("adastrea.PirateInfo"),
		TEXT("Log every pirate gang's need levels, biggest shortage and desperation."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (const UPirateSubsystem* Pirates = UPirateSubsystem::Get(World))
			{
				Pirates->LogGangs();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GPirateNeedCmd(
		TEXT("adastrea.PirateNeed"),
		TEXT("Set a gang's need level (0-100). Usage: adastrea.PirateNeed GangId|all people|access|parts|supplies|information|cash Level"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UPirateSubsystem* Pirates = UPirateSubsystem::Get(World);
			EPirateNeed Need;
			if (!Pirates || Args.Num() < 3 || !UPirateSubsystem::ParseNeed(Args[1], Need))
			{
				UE_LOG(LogAdastreaCombat, Warning, TEXT("Usage: adastrea.PirateNeed GangId|all Need Level"));
				return;
			}
			const float Level = FCString::Atof(*Args[2]);
			const bool bAll = Args[0].Equals(TEXT("all"), ESearchCase::IgnoreCase);
			for (const FPirateGang& Gang : Pirates->GetGangs())
			{
				if (bAll || Gang.OrgId == FName(*Args[0]))
				{
					FPirateGang* Mutable = Pirates->FindGang(Gang.OrgId);
					Mutable->Needs.Set(Need, Level);
					// A level set by hand starts a fresh shortage clock.
					Mutable->ShortageSeconds = 0.0f;
				}
			}
			Pirates->LogGangs();
		}));

	static FAutoConsoleCommandWithWorldAndArgs GPirateDesperationCmd(
		TEXT("adastrea.PirateDesperation"),
		TEXT("Debug: set how desperate a gang is (0-1). Usage: adastrea.PirateDesperation GangId|all Value"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UPirateSubsystem* Pirates = UPirateSubsystem::Get(World);
			if (!Pirates || Args.Num() < 2)
			{
				UE_LOG(LogAdastreaCombat, Warning, TEXT("Usage: adastrea.PirateDesperation GangId|all Value"));
				return;
			}
			const float Value = FMath::Clamp(FCString::Atof(*Args[1]), 0.0f, 1.0f);
			for (const FPirateGang& Gang : Pirates->GetGangs())
			{
				if (Args[0].Equals(TEXT("all"), ESearchCase::IgnoreCase) || Gang.OrgId == FName(*Args[0]))
				{
					Pirates->FindGang(Gang.OrgId)->ShortageSeconds = Value * UPirateSubsystem::DesperationSeconds;
				}
			}
			Pirates->LogGangs();
		}));

	static FAutoConsoleCommandWithWorldAndArgs GPirateUpkeepCmd(
		TEXT("adastrea.PirateUpkeep"),
		TEXT("Run pirate upkeep ticks now (need levels fall). Usage: adastrea.PirateUpkeep [Count=1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UPirateSubsystem* Pirates = UPirateSubsystem::Get(World))
			{
				Pirates->RunUpkeep(Args.IsEmpty() ? 1 : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 1000));
				Pirates->LogGangs();
			}
		}));
}

UPirateSubsystem* UPirateSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = (WorldContextObject && GEngine) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UPirateSubsystem>() : nullptr;
}

void UPirateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UOrganisationSubsystem::StaticClass());
	Super::Initialize(Collection);
	ReloadGangs();
}

TStatId UPirateSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPirateSubsystem, STATGROUP_Tickables);
}

ETickableTickType UPirateSubsystem::GetTickableTickType() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional;
}

bool UPirateSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return !Gangs.IsEmpty() && World && World->IsGameWorld();
}

void UPirateSubsystem::ReloadGangs()
{
	Gangs.Reset();
	const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	if (Orgs)
	{
		UpkeepSeconds = FMath::Max(1.0f, Orgs->GetEconomyConfig().TickSeconds);
	}

	FString Text;
	TSharedPtr<FJsonObject> Root;
	if (!FFileHelper::LoadFileToString(Text, *UOrganisationSubsystem::GetOrganisationsFilePath())
		|| !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid())
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Pirates: couldn't read Organisations.json"));
		return;
	}
	const TArray<TSharedPtr<FJsonValue>>* OrgValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("organisations"), OrgValues))
	{
		return;
	}
	for (const TSharedPtr<FJsonValue>& Value : *OrgValues)
	{
		const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
		const TSharedPtr<FJsonObject>* PirateObj = nullptr;
		if (!Obj.IsValid() || !Obj->TryGetObjectField(TEXT("pirate"), PirateObj))
		{
			continue;
		}
		FPirateGang Gang;
		Gang.OrgId = FName(Obj->GetStringField(TEXT("id")));
		FString Parent;
		if (Obj->TryGetStringField(TEXT("parent"), Parent) && !Parent.IsEmpty())
		{
			UE_LOG(LogAdastreaCombat, Warning, TEXT("Pirates: '%s' has a parent (%s); pirate gangs are independent"), *Gang.OrgId.ToString(), *Parent);
		}
		const TSharedPtr<FJsonObject>* NeedsObj = nullptr;
		const TSharedPtr<FJsonObject>* UpkeepObj = nullptr;
		(*PirateObj)->TryGetObjectField(TEXT("needs"), NeedsObj);
		(*PirateObj)->TryGetObjectField(TEXT("upkeep"), UpkeepObj);
		for (EPirateNeed Need : TEnumRange<EPirateNeed>())
		{
			double Number = 0.0;
			if (NeedsObj && (*NeedsObj)->TryGetNumberField(NeedToString(Need), Number))
			{
				Gang.Needs.Set(Need, (float)Number);
			}
			if (UpkeepObj && (*UpkeepObj)->TryGetNumberField(NeedToString(Need), Number))
			{
				Gang.Upkeep[(int32)Need] = FMath::Max(0.0f, (float)Number);
			}
		}
		Gangs.Add(Gang);
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Pirates: %d gang(s) loaded"), Gangs.Num());
}

FPirateGang* UPirateSubsystem::FindGang(FName OrgId)
{
	return Gangs.FindByPredicate([OrgId](const FPirateGang& G) { return G.OrgId == OrgId; });
}

const FPirateGang* UPirateSubsystem::FindGang(FName OrgId) const
{
	return Gangs.FindByPredicate([OrgId](const FPirateGang& G) { return G.OrgId == OrgId; });
}

FString UPirateSubsystem::NeedToString(EPirateNeed Need)
{
	return (int32)Need < FPirateNeeds::Num ? PirateTuning::NeedNames[(int32)Need] : TEXT("?");
}

bool UPirateSubsystem::ParseNeed(const FString& Text, EPirateNeed& OutNeed)
{
	for (EPirateNeed Need : TEnumRange<EPirateNeed>())
	{
		if (Text.Equals(NeedToString(Need), ESearchCase::IgnoreCase))
		{
			OutNeed = Need;
			return true;
		}
	}
	return false;
}

float UPirateSubsystem::GetImportance(EPirateNeed Need)
{
	switch (Need)
	{
	case EPirateNeed::People:		return 3.0f;
	case EPirateNeed::Access:		return 2.5f;
	case EPirateNeed::Parts:		return 1.6f;
	case EPirateNeed::Supplies:		return 1.4f;
	case EPirateNeed::Information:	return 1.2f;
	default:						return 0.6f;
	}
}

bool UPirateSubsystem::IsRaidable(EPirateNeed Need)
{
	return Need != EPirateNeed::People && Need != EPirateNeed::Access && Need != EPirateNeed::Count;
}

EPirateNeed UPirateSubsystem::NeedForItem(const UTradeItemDataAsset* Item)
{
	if (!Item)
	{
		return EPirateNeed::Cash;
	}
	switch (Item->Category)
	{
	case ETradeItemCategory::Food:
	case ETradeItemCategory::Medical:
		return EPirateNeed::Supplies;
	case ETradeItemCategory::Components:
	case ETradeItemCategory::Technology:
	case ETradeItemCategory::Military:
		return EPirateNeed::Parts;
	case ETradeItemCategory::Data:
		return EPirateNeed::Information;
	default:
		break;
	}
	// Raw and refined goods: fuel and water keep a base running, alloys keep a fleet flying.
	const FString Words = (Item->ItemID.ToString() + TEXT(" ") + Item->ItemName.ToString() + TEXT(" ") + Item->GetName()).ToLower();
	if (PirateTuning::ContainsAny(Words, { TEXT("fuel"), TEXT("helium"), TEXT("power"), TEXT("water"), TEXT("ice"), TEXT("protein"), TEXT("ration"), TEXT("food"), TEXT("medic") }))
	{
		return EPirateNeed::Supplies;
	}
	if (PirateTuning::ContainsAny(Words, { TEXT("component"), TEXT("alloy"), TEXT("steel"), TEXT("electronic"), TEXT("processor"), TEXT("sensor"), TEXT("construction"), TEXT("metal") }))
	{
		return EPirateNeed::Parts;
	}
	if (PirateTuning::ContainsAny(Words, { TEXT("data"), TEXT("manifest"), TEXT("chart") }))
	{
		return EPirateNeed::Information;
	}
	return EPirateNeed::Cash;
}

float UPirateSubsystem::PointsForGoods(const UTradeItemDataAsset* Item, int32 Units)
{
	if (!Item || Units <= 0)
	{
		return 0.0f;
	}
	const EPirateNeed Need = NeedForItem(Item);
	return FMath::Max(1.0f, Item->BasePrice) * Units / PirateTuning::CreditsPerPoint[(int32)Need];
}

float UPirateSubsystem::PointsForCredits(int32 Credits)
{
	return FMath::Max(0, Credits) / PirateTuning::CreditsPerCashPoint;
}

EPirateNeed UPirateSubsystem::GetBiggestShortage(const FPirateGang& Gang)
{
	EPirateNeed Best = EPirateNeed::Cash;
	float BestWant = -1.0f;
	for (EPirateNeed Need : TEnumRange<EPirateNeed>())
	{
		const float Want = GetShortage(Gang, Need) * GetImportance(Need);
		if (IsRaidable(Need) && Want > BestWant)
		{
			BestWant = Want;
			Best = Need;
		}
	}
	return Best;
}

int32 UPirateSubsystem::GetRaidStrength(const FPirateGang& Gang)
{
	const float Parts = Gang.Needs.Get(EPirateNeed::Parts);
	if (Parts < PirateTuning::MinPartsToRaid)
	{
		return 0;
	}
	return FMath::Clamp(1 + FMath::FloorToInt(Parts / PirateTuning::PartsPerRaider), 1, PirateTuning::MaxRaiders);
}

void UPirateSubsystem::DeliverGoods(FName OrgId, UTradeItemDataAsset* Item, int32 Units)
{
	if (FPirateGang* Gang = FindGang(OrgId); Gang && Item && Units > 0)
	{
		const EPirateNeed Need = NeedForItem(Item);
		const float Points = PointsForGoods(Item, Units);
		Gang->Needs.Add(Need, Points);
		UE_LOG(LogAdastreaCombat, Log, TEXT("Pirates: %s brought home %d %s (+%.1f %s, now %.0f)"), *OrgId.ToString(), Units,
			*Item->ItemName.ToString(), Points, *NeedToString(Need), Gang->Needs.Get(Need));
	}
}

void UPirateSubsystem::DeliverCredits(FName OrgId, int32 Credits)
{
	if (FPirateGang* Gang = FindGang(OrgId); Gang && Credits > 0)
	{
		Gang->Needs.Add(EPirateNeed::Cash, PointsForCredits(Credits));
		if (UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this))
		{
			Orgs->GrantCredits(OrgId, Credits);
		}
		UE_LOG(LogAdastreaCombat, Log, TEXT("Pirates: %s brought home %d credits (cash now %.0f)"), *OrgId.ToString(), Credits, Gang->Needs.Get(EPirateNeed::Cash));
	}
}

void UPirateSubsystem::DeliverSalvage(FName OrgId, float Points)
{
	if (FPirateGang* Gang = FindGang(OrgId))
	{
		Gang->Needs.Add(EPirateNeed::Parts, Points);
		UE_LOG(LogAdastreaCombat, Log, TEXT("Pirates: %s brought home salvage (+%.1f parts, now %.0f)"), *OrgId.ToString(), Points, Gang->Needs.Get(EPirateNeed::Parts));
	}
}

void UPirateSubsystem::RecordRaid(FName OrgId, bool bBroughtSomethingHome)
{
	if (FPirateGang* Gang = FindGang(OrgId))
	{
		++Gang->Raids;
		Gang->RaidsWon += bBroughtSomethingHome ? 1 : 0;
	}
}

void UPirateSubsystem::RecordShipLost(FName OrgId)
{
	if (FPirateGang* Gang = FindGang(OrgId))
	{
		++Gang->ShipsLost;
		Gang->Needs.Add(EPirateNeed::Parts, -PirateTuning::ShipLossParts);
		Gang->Needs.Add(EPirateNeed::People, -PirateTuning::ShipLossPeople);
		UE_LOG(LogAdastreaCombat, Log, TEXT("Pirates: %s lost a ship (parts %.0f, people %.0f)"), *OrgId.ToString(),
			Gang->Needs.Get(EPirateNeed::Parts), Gang->Needs.Get(EPirateNeed::People));
	}
}

void UPirateSubsystem::Tick(float DeltaTime)
{
	for (FPirateGang& Gang : Gangs)
	{
		const EPirateNeed Worst = GetBiggestShortage(Gang);
		if (Gang.Needs.Get(Worst) < SevereLevel)
		{
			Gang.ShortageSeconds = FMath::Min(Gang.ShortageSeconds + DeltaTime, DesperationSeconds);
		}
		else
		{
			// Relief comes faster than desperation built up.
			Gang.ShortageSeconds = FMath::Max(0.0f, Gang.ShortageSeconds - DeltaTime * 2.0f);
		}
	}

	UpkeepAccumulator += DeltaTime;
	int32 Runs = 0;
	while (UpkeepAccumulator >= UpkeepSeconds && Runs < 4)
	{
		UpkeepAccumulator -= UpkeepSeconds;
		RunUpkeepTick();
		++Runs;
	}
	UpkeepAccumulator = FMath::Min(UpkeepAccumulator, UpkeepSeconds);
}

void UPirateSubsystem::RunUpkeep(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		RunUpkeepTick();
	}
}

void UPirateSubsystem::RunUpkeepTick()
{
	for (FPirateGang& Gang : Gangs)
	{
		for (EPirateNeed Need : TEnumRange<EPirateNeed>())
		{
			Gang.Needs.Add(Need, -Gang.Upkeep[(int32)Need] * FMath::Max(0.0f, PirateTuning::CVarUpkeepScale.GetValueOnGameThread()));
		}
	}
}

void UPirateSubsystem::LogGangs() const
{
	const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	for (const FPirateGang& Gang : Gangs)
	{
		FString Levels;
		for (EPirateNeed Need : TEnumRange<EPirateNeed>())
		{
			Levels += FString::Printf(TEXT(" %s=%.0f"), *NeedToString(Need), Gang.Needs.Get(Need));
		}
		UE_LOG(LogAdastreaCombat, Log, TEXT("Pirate gang %s (%s):%s | short of %s, desperation %.2f, strength %d, raids %d (%d won), ships lost %d"),
			*Gang.OrgId.ToString(), Orgs ? *Orgs->GetOrgDisplayName(Gang.OrgId).ToString() : TEXT("?"), *Levels,
			*NeedToString(GetBiggestShortage(Gang)), GetDesperation(Gang), GetRaidStrength(Gang), Gang.Raids, Gang.RaidsWon, Gang.ShipsLost);
	}
}

void UPirateSubsystem::ExportState(TMap<FName, FPirateNeeds>& Out) const
{
	Out.Reset();
	for (const FPirateGang& Gang : Gangs)
	{
		Out.Add(Gang.OrgId, Gang.Needs);
	}
}

void UPirateSubsystem::ImportState(const TMap<FName, FPirateNeeds>& In)
{
	for (FPirateGang& Gang : Gangs)
	{
		if (const FPirateNeeds* Saved = In.Find(Gang.OrgId))
		{
			Gang.Needs = *Saved;
			Gang.ShortageSeconds = 0.0f;
		}
	}
}
