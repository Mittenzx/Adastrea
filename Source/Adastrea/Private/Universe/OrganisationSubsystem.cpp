// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/OrganisationSubsystem.h"
#include "Universe/GalaxySubsystem.h"
#include "Universe/OwnershipComponent.h"
#include "AdastreaLog.h"
#include "Ships/Spaceship.h"
#include "Stations/SpaceStation.h"
#include "Trading/PlayerTraderComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"

const FName UOrganisationSubsystem::PlayerOrgId(TEXT("player"));

namespace
{
	FString JsonString(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		FString Out;
		Obj->TryGetStringField(Field, Out);
		return Out;
	}

	FName JsonName(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		const FString S = JsonString(Obj, Field);
		return S.IsEmpty() ? NAME_None : FName(*S);
	}

	int32 JsonInt(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, int32 Default)
	{
		double Out = Default;
		return Obj->TryGetNumberField(Field, Out) ? FMath::RoundToInt((float)Out) : Default;
	}

	float JsonNumber(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, float Default)
	{
		double Out = Default;
		return Obj->TryGetNumberField(Field, Out) ? (float)Out : Default;
	}

	/** Reads { "trader": n, "miner": n, "patrol": n } into Out (indexed by EShipRole), keeping defaults for missing roles. */
	void JsonPerRole(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, int32 (&Out)[3])
	{
		const TSharedPtr<FJsonObject>* PerRole = nullptr;
		if (Obj->TryGetObjectField(Field, PerRole))
		{
			Out[(int32)EShipRole::Trader] = FMath::Max(0, JsonInt(*PerRole, TEXT("trader"), Out[(int32)EShipRole::Trader]));
			Out[(int32)EShipRole::Miner]  = FMath::Max(0, JsonInt(*PerRole, TEXT("miner"),  Out[(int32)EShipRole::Miner]));
			Out[(int32)EShipRole::Patrol] = FMath::Max(0, JsonInt(*PerRole, TEXT("patrol"), Out[(int32)EShipRole::Patrol]));
		}
	}

	/** Adds without overflowing int32 (wallets are int32 like UPlayerTraderComponent::Credits). */
	int32 AddClamped(int32 A, int64 B)
	{
		return (int32)FMath::Clamp((int64)A + B, (int64)MIN_int32, (int64)MAX_int32);
	}

	FString Cr(int64 Amount)
	{
		return FText::AsNumber(Amount).ToString() + TEXT(" cr");
	}

	TArray<FString> JsonStringArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		TArray<FString> Out;
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Obj->TryGetArrayField(Field, Values))
		{
			for (const TSharedPtr<FJsonValue>& V : *Values)
			{
				FString S;
				if (V.IsValid() && V->TryGetString(S) && !S.IsEmpty())
				{
					Out.AddUnique(S);
				}
			}
		}
		return Out;
	}

	TArray<FName> JsonNameArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		TArray<FName> Out;
		for (const FString& S : JsonStringArray(Obj, Field))
		{
			Out.AddUnique(FName(*S));
		}
		return Out;
	}

	const TArray<TSharedPtr<FJsonValue>>& JsonObjectArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		static const TArray<TSharedPtr<FJsonValue>> Empty;
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		return Obj->TryGetArrayField(Field, Values) ? *Values : Empty;
	}

	bool ParseKind(const FString& S, EOrgKind& Out)
	{
		if (S.Equals(TEXT("person"), ESearchCase::IgnoreCase))    { Out = EOrgKind::Person;    return true; }
		if (S.Equals(TEXT("outfit"), ESearchCase::IgnoreCase))    { Out = EOrgKind::Outfit;    return true; }
		if (S.Equals(TEXT("guild"), ESearchCase::IgnoreCase))     { Out = EOrgKind::Guild;     return true; }
		if (S.Equals(TEXT("authority"), ESearchCase::IgnoreCase)) { Out = EOrgKind::Authority; return true; }
		return false;
	}

	bool ParseRole(const FString& S, EShipRole& Out)
	{
		if (S.Equals(TEXT("trader"), ESearchCase::IgnoreCase)) { Out = EShipRole::Trader; return true; }
		if (S.Equals(TEXT("miner"), ESearchCase::IgnoreCase))  { Out = EShipRole::Miner;  return true; }
		if (S.Equals(TEXT("patrol"), ESearchCase::IgnoreCase)) { Out = EShipRole::Patrol; return true; }
		return false;
	}

	void LogOrgTree(const UOrganisationSubsystem& Orgs, const TMultiMap<FName, FName>& Children, FName OrgId, int32 Depth)
	{
		const FOrgDef* Org = Orgs.FindOrg(OrgId);
		if (!Org || Depth > 8)
		{
			return;
		}
		FString Extra;
		if (Org->Kind == EOrgKind::Authority && Org->Governs.Num() > 0)
		{
			TArray<FString> Sectors;
			for (const FName& S : Org->Governs) { Sectors.Add(S.ToString()); }
			Extra = FString::Printf(TEXT(" governs [%s]"), *FString::Join(Sectors, TEXT(", ")));
		}
		const FOrgLedger Ledger = Orgs.GetLedger(Org->Id);
		UE_LOG(LogAdastrea, Log, TEXT("  %s%s '%s' (%s) ships=%d wallet=%s%s | earned %s, tax paid %s, tax in %s, upkeep %s, bought %d, sold %d"),
			*FString::ChrN(Depth * 2, TEXT(' ')), *Org->Id.ToString(), *Org->Name.ToString(), *UOrganisationSubsystem::KindToString(Org->Kind),
			Orgs.CountShipsOwnedBy(Org->Id), *Cr(Org->Credits), *Extra, *Cr(Ledger.Earned), *Cr(Ledger.TaxPaid),
			*Cr(Ledger.TaxCollected), *Cr(Ledger.Upkeep), Ledger.ShipsBought, Ledger.ShipsSold);

		TArray<FName> Kids;
		Children.MultiFind(OrgId, Kids, true);
		for (const FName& Kid : Kids)
		{
			LogOrgTree(Orgs, Children, Kid, Depth + 1);
		}
	}

	void LogSector(UOrganisationSubsystem& Orgs, FName SectorId)
	{
		const FName Authority = Orgs.GetSectorAuthorityId(SectorId);
		const TArray<FShipRecord*> InSector = Orgs.GetShipsInSector(SectorId);
		UE_LOG(LogAdastrea, Log, TEXT("Sector '%s': governed by %s, %d roster ships"), *SectorId.ToString(),
			Authority.IsNone() ? TEXT("nobody (independent space)") : *Orgs.GetOrgDisplayName(Authority).ToString(), InSector.Num());
		for (const FShipRecord* Ship : InSector)
		{
			UE_LOG(LogAdastrea, Log, TEXT("  %s '%s' %s  earned %s  owner: %s  %s"), *Ship->Id.ToString(), *Ship->Name.ToString(),
				*UOrganisationSubsystem::RoleToString(Ship->Role), *Cr(Ship->Earnings), *Orgs.DescribeAllegiance(Ship->OwnerId),
				Ship->SpawnedShip.IsValid() ? *FString::Printf(TEXT("[spawned as %s]"), *Ship->SpawnedShip->GetName()) : TEXT("[not spawned]"));
		}
	}

	static FAutoConsoleCommandWithWorld GReloadOrgsCmd(
		TEXT("adastrea.ReloadOrgs"),
		TEXT("Re-read Content/Data/Universe/Organisations.json (ships already in the level keep flying but lose their roster link)."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World))
			{
				Orgs->ReloadOrganisations();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GOrgInfoCmd(
		TEXT("adastrea.OrgInfo"),
		TEXT("Log the organisation tree and the roster of a sector. Usage: adastrea.OrgInfo [SectorId=this level]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World);
			if (!Orgs)
			{
				return;
			}
			if (!Args.IsEmpty())
			{
				LogSector(*Orgs, FName(*Args[0]));
				return;
			}

			UE_LOG(LogAdastrea, Log, TEXT("Organisations (%s): %d orgs, %d ships, next economy tick in %.0fs"),
				Orgs->IsLoadedFromFile() ? TEXT("Organisations.json") : TEXT("player only"),
				Orgs->GetOrganisations().Num(), Orgs->GetShips().Num(), Orgs->GetSecondsToNextTick());

			TMultiMap<FName, FName> Children;
			TArray<FName> Roots;
			for (const FOrgDef& Org : Orgs->GetOrganisations())
			{
				if (Org.ParentId.IsNone() || !Orgs->FindOrg(Org.ParentId))
				{
					Roots.Add(Org.Id);
				}
				else
				{
					Children.Add(Org.ParentId, Org.Id);
				}
			}
			for (const FName& Root : Roots)
			{
				LogOrgTree(*Orgs, Children, Root, 0);
			}

			const UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World);
			const FName Here = Galaxy ? Galaxy->ResolveCurrentSectorId(World) : NAME_None;
			if (!Here.IsNone())
			{
				LogSector(*Orgs, Here);
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GEconomyTickCmd(
		TEXT("adastrea.EconomyTick"),
		TEXT("Run organisation economy ticks now (income, tax, upkeep, ship buying/selling). Usage: adastrea.EconomyTick [Count=1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World))
			{
				Orgs->RunEconomyTicks(Args.IsEmpty() ? 1 : FMath::Clamp(FCString::Atoi(*Args[0]), 1, 10000));
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GOrgGrantCmd(
		TEXT("adastrea.OrgGrant"),
		TEXT("Debug: add credits to an organisation's wallet (negative to take). Usage: adastrea.OrgGrant OrgId Amount"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World);
			if (!Orgs || Args.Num() < 2 || !Orgs->GrantCredits(FName(*Args[0]), FCString::Atoi(*Args[1])))
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Usage: adastrea.OrgGrant OrgId Amount (OrgId from adastrea.OrgInfo)"));
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GOrgStandingCmd(
		TEXT("adastrea.OrgStanding"),
		TEXT("Log the standing between two organisations. Usage: adastrea.OrgStanding OrgA OrgB"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(World);
			if (!Orgs || Args.Num() < 2)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Usage: adastrea.OrgStanding OrgA OrgB"));
				return;
			}
			const FName A(*Args[0]);
			const FName B(*Args[1]);
			UE_LOG(LogAdastrea, Log, TEXT("Standing %s <-> %s = %d"), *Orgs->DescribeAllegiance(A), *Orgs->DescribeAllegiance(B),
				Orgs->GetStanding(A, B));
		}));
}

void UOrganisationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Validation checks governed sectors against the galaxy.
	Collection.InitializeDependency(UGalaxySubsystem::StaticClass());
	Super::Initialize(Collection);
	ReloadOrganisations();
}

UOrganisationSubsystem* UOrganisationSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UOrganisationSubsystem>() : nullptr;
}

FString UOrganisationSubsystem::GetOrganisationsFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("Data/Universe/Organisations.json");
}

int32 UOrganisationSubsystem::GetShipCap(EOrgKind Kind)
{
	switch (Kind)
	{
	case EOrgKind::Person:    return 2;
	case EOrgKind::Outfit:    return 6;
	case EOrgKind::Guild:     return 8;
	case EOrgKind::Authority: return 20;
	}
	return 0;
}

FString UOrganisationSubsystem::KindToString(EOrgKind Kind)
{
	switch (Kind)
	{
	case EOrgKind::Person:    return TEXT("Person");
	case EOrgKind::Outfit:    return TEXT("Outfit");
	case EOrgKind::Guild:     return TEXT("Guild");
	case EOrgKind::Authority: return TEXT("Authority");
	}
	return FString();
}

FString UOrganisationSubsystem::RoleToString(EShipRole Role)
{
	switch (Role)
	{
	case EShipRole::Trader: return TEXT("Trader");
	case EShipRole::Miner:  return TEXT("Miner");
	case EShipRole::Patrol: return TEXT("Patrol");
	}
	return FString();
}

bool UOrganisationSubsystem::ReloadOrganisations()
{
	Orgs.Reset();
	Ships.Reset();
	Relations.Reset();
	Ledgers.Reset();
	Economy = FOrgEconomyConfig();
	TickAccumulator = 0.0f;

	const FString Path = GetOrganisationsFilePath();
	FString Error;
	bLoadedFromFile = LoadFromFile(Path, Error);
	if (!bLoadedFromFile)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("Organisations: %s (%s). Only the player exists."), *Error, *Path);
	}
	Finalize();
	++Revision;

	UE_LOG(LogAdastrea, Log, TEXT("Organisations: loaded %d orgs, %d ships, %d relations from %s"),
		Orgs.Num(), Ships.Num(), Relations.Num(), bLoadedFromFile ? TEXT("Organisations.json") : TEXT("defaults"));
	return bLoadedFromFile;
}

bool UOrganisationSubsystem::LoadFromFile(const FString& Path, FString& OutError)
{
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		OutError = TEXT("file not found");
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = FString::Printf(TEXT("invalid JSON: %s"), *Reader->GetErrorMessage());
		return false;
	}

	TSet<FName> SeenOrgs;
	for (const TSharedPtr<FJsonValue>& Value : JsonObjectArray(Root, TEXT("organisations")))
	{
		const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!Obj.IsValid())
		{
			continue;
		}
		FOrgDef Org;
		Org.Id = JsonName(Obj, TEXT("id"));
		if (Org.Id.IsNone() || SeenOrgs.Contains(Org.Id))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: skipping an organisation with a missing or duplicate id '%s'"), *Org.Id.ToString());
			continue;
		}
		const FString KindText = JsonString(Obj, TEXT("kind"));
		if (!ParseKind(KindText, Org.Kind))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: '%s' has unknown kind '%s' (person, outfit, guild, authority); treating as person"),
				*Org.Id.ToString(), *KindText);
		}
		const FString Name = JsonString(Obj, TEXT("name"));
		Org.Name = FText::FromString(Name.IsEmpty() ? Org.Id.ToString() : Name);
		Org.ParentId = JsonName(Obj, TEXT("parent"));
		Org.Governs = JsonNameArray(Obj, TEXT("governs"));
		Org.Seat = JsonString(Obj, TEXT("seat"));
		Org.HomeSectorId = JsonName(Obj, TEXT("home"));
		Org.Industries = JsonStringArray(Obj, TEXT("industries"));
		Org.Description = FText::FromString(JsonString(Obj, TEXT("description")));
		Org.Credits = FMath::Max(0, JsonInt(Obj, TEXT("credits"), 0));
		Org.TaxRate = FMath::Clamp(JsonNumber(Obj, TEXT("taxRate"), Org.TaxRate), 0.0f, 0.5f);
		// -1: filled from the roster in Finalize().
		Org.TargetFleetSize = JsonInt(Obj, TEXT("targetFleet"), -1);

		const TArray<TSharedPtr<FJsonValue>>& Colour = JsonObjectArray(Obj, TEXT("colour"));
		if (Colour.Num() >= 3)
		{
			Org.Colour = FLinearColor((float)Colour[0]->AsNumber(), (float)Colour[1]->AsNumber(), (float)Colour[2]->AsNumber(), 1.0f);
		}

		SeenOrgs.Add(Org.Id);
		Orgs.Add(MoveTemp(Org));
	}

	// Ships: one entry per ship, or "count": N for N numbered ships sharing a name.
	TMap<FName, int32> NextShipNumber;
	TSet<FName> SeenShips;
	for (const TSharedPtr<FJsonValue>& Value : JsonObjectArray(Root, TEXT("ships")))
	{
		const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!Obj.IsValid())
		{
			continue;
		}
		FShipRecord Base;
		Base.OwnerId = JsonName(Obj, TEXT("owner"));
		Base.SectorId = JsonName(Obj, TEXT("sector"));
		const FString RoleText = JsonString(Obj, TEXT("role"));
		if (!RoleText.IsEmpty() && !ParseRole(RoleText, Base.Role))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: ship of '%s' has unknown role '%s' (trader, miner, patrol); treating as trader"),
				*Base.OwnerId.ToString(), *RoleText);
		}
		const FString ClassPath = JsonString(Obj, TEXT("class"));
		if (!ClassPath.IsEmpty())
		{
			Base.ShipClass = TSoftClassPtr<ASpaceship>(FSoftObjectPath(ClassPath));
		}
		if (Base.OwnerId.IsNone())
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: skipping a ship with no owner"));
			continue;
		}

		const FString Name = JsonString(Obj, TEXT("name"));
		const FName ExplicitId = JsonName(Obj, TEXT("id"));
		const int32 Count = FMath::Clamp(JsonInt(Obj, TEXT("count"), 1), 1, 50);
		for (int32 i = 1; i <= Count; ++i)
		{
			FShipRecord Ship = Base;
			int32& Number = NextShipNumber.FindOrAdd(Base.OwnerId);
			++Number;
			Ship.Id = (!ExplicitId.IsNone() && Count == 1) ? ExplicitId
				: FName(*FString::Printf(TEXT("%s_%d"), *Base.OwnerId.ToString(), Number));
			const FString ShipName = Name.IsEmpty() ? Ship.Id.ToString() : (Count > 1 ? FString::Printf(TEXT("%s %d"), *Name, i) : Name);
			Ship.Name = FText::FromString(ShipName);
			if (SeenShips.Contains(Ship.Id))
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Organisations: skipping duplicate ship id '%s'"), *Ship.Id.ToString());
				continue;
			}
			SeenShips.Add(Ship.Id);
			Ships.Add(MoveTemp(Ship));
		}
	}

	for (const TSharedPtr<FJsonValue>& Value : JsonObjectArray(Root, TEXT("relations")))
	{
		const TSharedPtr<FJsonObject> Obj = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!Obj.IsValid())
		{
			continue;
		}
		FOrgRelation Rel;
		Rel.A = JsonName(Obj, TEXT("a"));
		Rel.B = JsonName(Obj, TEXT("b"));
		Rel.Standing = FMath::Clamp(JsonInt(Obj, TEXT("standing"), 0), -100, 100);
		Rel.Note = JsonString(Obj, TEXT("note"));
		if (!Rel.A.IsNone() && !Rel.B.IsNone() && Rel.A != Rel.B)
		{
			Relations.Add(MoveTemp(Rel));
		}
	}

	LoadEconomy(Root);
	return true;
}

void UOrganisationSubsystem::LoadEconomy(const TSharedPtr<FJsonObject>& Root)
{
	const TSharedPtr<FJsonObject>* EconomyObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("economy"), EconomyObj))
	{
		return;
	}
	const TSharedPtr<FJsonObject>& E = *EconomyObj;
	Economy.TickSeconds = FMath::Max(1.0f, JsonNumber(E, TEXT("tickSeconds"), Economy.TickSeconds));
	JsonPerRole(E, TEXT("shipPrice"), Economy.ShipPrice);
	JsonPerRole(E, TEXT("upkeep"), Economy.Upkeep);
	JsonPerRole(E, TEXT("offscreenIncome"), Economy.OffscreenIncome);
	Economy.SectorIncome = FMath::Max(0, JsonInt(E, TEXT("sectorIncome"), Economy.SectorIncome));
	Economy.ResaleFraction = FMath::Clamp(JsonNumber(E, TEXT("resaleFraction"), Economy.ResaleFraction), 0.0f, 1.0f);
	Economy.ReserveFraction = FMath::Max(0.0f, JsonNumber(E, TEXT("reserveFraction"), Economy.ReserveFraction));
}

void UOrganisationSubsystem::Finalize()
{
	if (!Orgs.ContainsByPredicate([](const FOrgDef& Org) { return Org.Id == PlayerOrgId; }))
	{
		FOrgDef Player;
		Player.Id = PlayerOrgId;
		Player.Name = NSLOCTEXT("Organisations", "PlayerOrg", "You");
		Player.Kind = EOrgKind::Person;
		Player.Colour = FLinearColor(0.3f, 0.8f, 1.0f, 1.0f);
		Orgs.Add(MoveTemp(Player));
	}

	OrgIndex.Reset();
	for (int32 i = 0; i < Orgs.Num(); ++i)
	{
		OrgIndex.Add(Orgs[i].Id, i);
	}
	for (FOrgDef& Org : Orgs)
	{
		if (!Org.ParentId.IsNone() && !OrgIndex.Contains(Org.ParentId))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: '%s' has unknown parent '%s'; treating as independent"),
				*Org.Id.ToString(), *Org.ParentId.ToString());
			Org.ParentId = NAME_None;
		}
	}

	RebuildShipIndex();

	// Authorities keep the fleet they start with unless the JSON says otherwise.
	for (FOrgDef& Org : Orgs)
	{
		if (Org.TargetFleetSize < 0)
		{
			Org.TargetFleetSize = Org.Kind == EOrgKind::Authority ? CountShipsOwnedBy(Org.Id) : 0;
		}
	}

	// First authority to claim a sector wins; Validate() reports the rest.
	SectorAuthority.Reset();
	for (const FOrgDef& Org : Orgs)
	{
		if (Org.Kind != EOrgKind::Authority)
		{
			continue;
		}
		for (const FName& Sector : Org.Governs)
		{
			if (!SectorAuthority.Contains(Sector))
			{
				SectorAuthority.Add(Sector, Org.Id);
			}
		}
	}

	Validate();
}

void UOrganisationSubsystem::Validate() const
{
	const UGalaxySubsystem* Galaxy = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySubsystem>() : nullptr;

	for (const FOrgDef& Org : Orgs)
	{
		if (Org.Kind != EOrgKind::Authority)
		{
			if (Org.Governs.Num() > 0)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Organisations: '%s' is a %s; only authorities govern sectors (ignored)"),
					*Org.Id.ToString(), *KindToString(Org.Kind));
			}
		}
		else
		{
			if (Org.Governs.Num() > MaxGovernedSectors)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Organisations: authority '%s' governs %d sectors (small-scale rule: at most %d)"),
					*Org.Id.ToString(), Org.Governs.Num(), MaxGovernedSectors);
			}
			FName System;
			for (const FName& SectorId : Org.Governs)
			{
				const FName Holder = SectorAuthority.FindRef(SectorId);
				if (Holder != Org.Id)
				{
					UE_LOG(LogAdastrea, Warning, TEXT("Organisations: sector '%s' is claimed by both '%s' and '%s'; '%s' keeps it"),
						*SectorId.ToString(), *Holder.ToString(), *Org.Id.ToString(), *Holder.ToString());
				}
				const FGalaxySectorDef* Sector = Galaxy ? Galaxy->FindSector(SectorId) : nullptr;
				if (Galaxy && !Sector)
				{
					UE_LOG(LogAdastrea, Warning, TEXT("Organisations: authority '%s' governs unknown sector '%s'"), *Org.Id.ToString(), *SectorId.ToString());
					continue;
				}
				if (Sector)
				{
					if (!System.IsNone() && Sector->SystemId != System)
					{
						UE_LOG(LogAdastrea, Warning, TEXT("Organisations: authority '%s' spans systems '%s' and '%s' (small-scale rule: one system)"),
							*Org.Id.ToString(), *System.ToString(), *Sector->SystemId.ToString());
					}
					System = Sector->SystemId;
					if (!Sector->Faction.IsEmpty() && Sector->Faction != Org.Name.ToString())
					{
						UE_LOG(LogAdastrea, Log, TEXT("Organisations: Galaxy.json lists sector '%s' faction as '%s' but it is governed by '%s'"),
							*SectorId.ToString(), *Sector->Faction, *Org.Name.ToString());
					}
				}
			}
		}

		// Parent chains longer than the org count must loop.
		FName Cursor = Org.ParentId;
		for (int32 Steps = 0; !Cursor.IsNone(); ++Steps)
		{
			if (Cursor == Org.Id || Steps > Orgs.Num())
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Organisations: '%s' is its own ancestor (parent loop)"), *Org.Id.ToString());
				break;
			}
			const FOrgDef* Parent = FindOrg(Cursor);
			Cursor = Parent ? Parent->ParentId : NAME_None;
		}

		const int32 Owned = CountShipsOwnedBy(Org.Id);
		if (Owned > GetShipCap(Org.Kind))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: %s '%s' owns %d ships (small-scale cap: %d)"),
				*KindToString(Org.Kind), *Org.Id.ToString(), Owned, GetShipCap(Org.Kind));
		}
	}

	for (const FShipRecord& Ship : Ships)
	{
		if (!FindOrg(Ship.OwnerId))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: ship '%s' has unknown owner '%s'"), *Ship.Id.ToString(), *Ship.OwnerId.ToString());
		}
		if (Galaxy && !Ship.SectorId.IsNone() && !Galaxy->FindSector(Ship.SectorId))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: ship '%s' is in unknown sector '%s'"), *Ship.Id.ToString(), *Ship.SectorId.ToString());
		}
	}

	for (const FOrgRelation& Rel : Relations)
	{
		if (!FindOrg(Rel.A) || !FindOrg(Rel.B))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: relation '%s' <-> '%s' names an unknown organisation"), *Rel.A.ToString(), *Rel.B.ToString());
		}
	}
}

const FOrgDef* UOrganisationSubsystem::FindOrg(FName OrgId) const
{
	const int32* Index = OrgIndex.Find(OrgId);
	return Index ? &Orgs[*Index] : nullptr;
}

bool UOrganisationSubsystem::GetOrganisation(FName OrgId, FOrgDef& OutOrg) const
{
	if (const FOrgDef* Org = FindOrg(OrgId))
	{
		OutOrg = *Org;
		return true;
	}
	return false;
}

FText UOrganisationSubsystem::GetOrgDisplayName(FName OrgId) const
{
	const FOrgDef* Org = FindOrg(OrgId);
	return Org ? Org->Name : NSLOCTEXT("Organisations", "Unowned", "Unowned");
}

FName UOrganisationSubsystem::GetSectorAuthorityId(FName SectorId) const
{
	return SectorAuthority.FindRef(SectorId);
}

TArray<FName> UOrganisationSubsystem::GetAllegianceChain(FName OrgId) const
{
	TArray<FName> Chain;
	for (const FOrgDef* Org = FindOrg(OrgId); Org && !Chain.Contains(Org->Id); Org = FindOrg(Org->ParentId))
	{
		Chain.Add(Org->Id);
	}
	return Chain;
}

int32 UOrganisationSubsystem::GetStanding(FName A, FName B) const
{
	if (A == B)
	{
		return 100;
	}
	const TArray<FName> ChainA = GetAllegianceChain(A);
	const TArray<FName> ChainB = GetAllegianceChain(B);

	// Nearest pair first: (A,B), then (A,parent B) / (parent A,B), ...
	const int32 MaxDepth = ChainA.Num() + ChainB.Num() - 2;
	for (int32 Depth = 0; Depth <= MaxDepth; ++Depth)
	{
		for (int32 i = 0; i <= Depth; ++i)
		{
			const int32 j = Depth - i;
			if (!ChainA.IsValidIndex(i) || !ChainB.IsValidIndex(j))
			{
				continue;
			}
			const FName X = ChainA[i];
			const FName Y = ChainB[j];
			if (X == Y)
			{
				return 50;
			}
			for (const FOrgRelation& Rel : Relations)
			{
				if ((Rel.A == X && Rel.B == Y) || (Rel.A == Y && Rel.B == X))
				{
					return Rel.Standing;
				}
			}
		}
	}
	return 0;
}

FName UOrganisationSubsystem::GetOwnerIdOf(const AActor* Actor) const
{
	if (!Actor)
	{
		return NAME_None;
	}
	if (const UOwnershipComponent* Ownership = UOwnershipComponent::Find(Actor); Ownership && !Ownership->OwnerId.IsNone())
	{
		return Ownership->OwnerId;
	}
	if (const APawn* Pawn = Cast<APawn>(Actor); Pawn && Pawn->IsPlayerControlled())
	{
		return PlayerOrgId;
	}
	if (Actor->IsA<ASpaceStation>())
	{
		const UGalaxySubsystem* Galaxy = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySubsystem>() : nullptr;
		return Galaxy ? GetSectorAuthorityId(Galaxy->ResolveCurrentSectorId(Actor)) : NAME_None;
	}
	return NAME_None;
}

FString UOrganisationSubsystem::DescribeAllegiance(FName OrgId) const
{
	const TArray<FName> Chain = GetAllegianceChain(OrgId);
	if (Chain.IsEmpty())
	{
		return OrgId.IsNone() ? TEXT("Unowned") : FString::Printf(TEXT("%s (unknown)"), *OrgId.ToString());
	}
	TArray<FString> Parts;
	for (const FName& Id : Chain)
	{
		const FOrgDef* Org = FindOrg(Id);
		Parts.Add(FString::Printf(TEXT("%s (%s)"), *Org->Name.ToString(), *KindToString(Org->Kind)));
	}
	return FString::Join(Parts, TEXT(" > "));
}

FShipRecord* UOrganisationSubsystem::FindShip(FName ShipId)
{
	const int32* Index = ShipIndex.Find(ShipId);
	return Index ? &Ships[*Index] : nullptr;
}

const FShipRecord* UOrganisationSubsystem::FindShip(FName ShipId) const
{
	const int32* Index = ShipIndex.Find(ShipId);
	return Index ? &Ships[*Index] : nullptr;
}

TArray<FShipRecord*> UOrganisationSubsystem::GetShipsInSector(FName SectorId)
{
	TArray<FShipRecord*> Out;
	for (FShipRecord& Ship : Ships)
	{
		if (Ship.SectorId == SectorId && !Ship.bDestroyed)
		{
			Out.Add(&Ship);
		}
	}
	return Out;
}

int32 UOrganisationSubsystem::CountShipsOwnedBy(FName OrgId) const
{
	int32 N = 0;
	for (const FShipRecord& Ship : Ships)
	{
		N += (Ship.OwnerId == OrgId && !Ship.bDestroyed) ? 1 : 0;
	}
	return N;
}

void UOrganisationSubsystem::BindSpawnedShip(FName ShipId, ASpaceship* Ship)
{
	if (FShipRecord* Record = FindShip(ShipId))
	{
		Record->SpawnedShip = Ship;
	}
}

void UOrganisationSubsystem::RebuildShipIndex()
{
	ShipIndex.Reset();
	for (int32 i = 0; i < Ships.Num(); ++i)
	{
		ShipIndex.Add(Ships[i].Id, i);
	}
}

FOrgDef* UOrganisationSubsystem::FindOrgMutable(FName OrgId)
{
	const int32* Index = OrgIndex.Find(OrgId);
	return Index ? &Orgs[*Index] : nullptr;
}

// ---------------------------------------------------------------------------
// Economy
// ---------------------------------------------------------------------------

void UOrganisationSubsystem::Tick(float DeltaTime)
{
	TickAccumulator += DeltaTime;
	// A long hitch shouldn't replay dozens of ticks at once.
	int32 Runs = 0;
	while (TickAccumulator >= Economy.TickSeconds && Runs < 4)
	{
		TickAccumulator -= Economy.TickSeconds;
		RunEconomyTick();
		++Runs;
	}
	if (TickAccumulator >= Economy.TickSeconds)
	{
		TickAccumulator = 0.0f;
	}
}

TStatId UOrganisationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UOrganisationSubsystem, STATGROUP_Tickables);
}

ETickableTickType UOrganisationSubsystem::GetTickableTickType() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Conditional;
}

bool UOrganisationSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return bLoadedFromFile && World && World->IsGameWorld();
}

bool UOrganisationSubsystem::GrantCredits(FName OrgId, int32 Amount)
{
	FOrgDef* Org = FindOrgMutable(OrgId);
	if (!Org)
	{
		return false;
	}
	Org->Credits = AddClamped(Org->Credits, Amount);
	UE_LOG(LogAdastrea, Log, TEXT("Economy: granted %s to %s; wallet now %s"), *Cr(Amount), *Org->Name.ToString(), *Cr(Org->Credits));
	return true;
}

void UOrganisationSubsystem::RunEconomyTicks(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		RunEconomyTick();
	}
	UE_LOG(LogAdastrea, Log, TEXT("Economy: ran %d tick(s); adastrea.OrgInfo for wallets"), Count);
}

int32 UOrganisationSubsystem::CollectTax(FName OwnerId, FName SectorId, int32 Amount)
{
	const FName AuthorityId = GetSectorAuthorityId(SectorId);
	FOrgDef* Authority = FindOrgMutable(AuthorityId);
	if (!Authority || AuthorityId == OwnerId || Amount <= 0)
	{
		return 0;
	}
	const int32 Tax = FMath::RoundToInt(Amount * Authority->TaxRate);
	Authority->Credits = AddClamped(Authority->Credits, Tax);
	Ledgers.FindOrAdd(AuthorityId).TaxCollected += Tax;
	Ledgers.FindOrAdd(OwnerId).TaxPaid += Tax;
	return Tax;
}

void UOrganisationSubsystem::HandleShipSale(FName ShipId, UPlayerTraderComponent* Trader, int32 WorkingCapital)
{
	FShipRecord* Record = FindShip(ShipId);
	FOrgDef* Owner = Record ? FindOrgMutable(Record->OwnerId) : nullptr;
	if (!Owner || !Trader || Owner->Id == PlayerOrgId)
	{
		return;
	}
	const int32 Profit = Trader->GetCredits() - WorkingCapital;
	if (Profit <= 0 || !Trader->RemoveCredits(Profit))
	{
		return;
	}
	const int32 Tax = CollectTax(Owner->Id, Record->SectorId, Profit);
	const int32 Net = Profit - Tax;
	Owner->Credits = AddClamped(Owner->Credits, Net);
	Record->Earnings = AddClamped(Record->Earnings, Net);
	Ledgers.FindOrAdd(Owner->Id).Earned += Net;

	UE_LOG(LogAdastrea, Log, TEXT("Economy: '%s' made %s profit: %s tax to %s, %s to %s (wallet %s)"),
		*Record->Name.ToString(), *Cr(Profit), *Cr(Tax), *GetOrgDisplayName(GetSectorAuthorityId(Record->SectorId)).ToString(),
		*Cr(Net), *Owner->Name.ToString(), *Cr(Owner->Credits));
}

void UOrganisationSubsystem::RunEconomyTick()
{
	// Authorities' base income from the sectors they govern.
	for (FOrgDef& Org : Orgs)
	{
		if (Org.Kind == EOrgKind::Authority && Org.Governs.Num() > 0)
		{
			const int32 Income = Economy.SectorIncome * Org.Governs.Num();
			Org.Credits = AddClamped(Org.Credits, Income);
			Ledgers.FindOrAdd(Org.Id).TaxCollected += Income;
		}
	}

	// Ships: off-screen income (spawned ships earn by actually trading), then upkeep.
	for (FShipRecord& Ship : Ships)
	{
		FOrgDef* Owner = Ship.bDestroyed ? nullptr : FindOrgMutable(Ship.OwnerId);
		if (!Owner || Owner->Id == PlayerOrgId)
		{
			continue;
		}
		const int32 Role = (int32)Ship.Role;
		if (!Ship.SpawnedShip.IsValid() && Economy.OffscreenIncome[Role] > 0)
		{
			const int32 Income = Economy.OffscreenIncome[Role];
			const int32 Net = Income - CollectTax(Owner->Id, Ship.SectorId, Income);
			Owner->Credits = AddClamped(Owner->Credits, Net);
			Ship.Earnings = AddClamped(Ship.Earnings, Net);
			Ledgers.FindOrAdd(Owner->Id).Earned += Net;
		}
		Owner->Credits = AddClamped(Owner->Credits, -(int64)Economy.Upkeep[Role]);
		Ledgers.FindOrAdd(Owner->Id).Upkeep += Economy.Upkeep[Role];
	}

	// At most one purchase or sale per organisation per tick.
	bool bRosterChanged = false;
	for (FOrgDef& Org : Orgs)
	{
		if (Org.Id == PlayerOrgId)
		{
			continue;
		}
		bRosterChanged |= Org.Credits < 0 ? SellShip(Org) : BuyShip(Org);
	}

	if (bRosterChanged)
	{
		++Revision;
		OnRosterChanged.Broadcast();
	}
}

bool UOrganisationSubsystem::SellShip(FOrgDef& Org)
{
	// Newest first, and never one in view: removing a live actor would leave its interior and berth behind.
	for (int32 i = Ships.Num() - 1; i >= 0; --i)
	{
		const FShipRecord& Ship = Ships[i];
		if (Ship.OwnerId != Org.Id || Ship.bDestroyed || Ship.SpawnedShip.IsValid())
		{
			continue;
		}
		const int32 Resale = FMath::RoundToInt(Economy.ShipPrice[(int32)Ship.Role] * Economy.ResaleFraction);
		Org.Credits = AddClamped(Org.Credits, Resale);
		++Ledgers.FindOrAdd(Org.Id).ShipsSold;
		UE_LOG(LogAdastrea, Log, TEXT("Economy: %s is in debt and sold '%s' (%s) for %s; wallet now %s"),
			*Org.Name.ToString(), *Ship.Name.ToString(), *RoleToString(Ship.Role), *Cr(Resale), *Cr(Org.Credits));
		Ships.RemoveAt(i);
		RebuildShipIndex();
		return true;
	}
	return false;
}

EShipRole UOrganisationSubsystem::ChooseRoleToBuy(const FOrgDef& Org) const
{
	int32 Counts[3] = { 0, 0, 0 };
	for (const FShipRecord& Ship : Ships)
	{
		if (Ship.OwnerId == Org.Id && !Ship.bDestroyed)
		{
			++Counts[(int32)Ship.Role];
		}
	}
	int32 Best = -1;
	for (int32 Role = 0; Role < 3; ++Role)
	{
		if (Counts[Role] > 0 && (Best < 0 || Counts[Role] > Counts[Best]))
		{
			Best = Role;
		}
	}
	if (Best >= 0)
	{
		return (EShipRole)Best;
	}
	if (Org.Kind == EOrgKind::Authority)
	{
		return EShipRole::Patrol;
	}
	return Org.Industries.ContainsByPredicate([](const FString& S) { return S.Equals(TEXT("Mining"), ESearchCase::IgnoreCase); })
		? EShipRole::Miner : EShipRole::Trader;
}

FName UOrganisationSubsystem::ChooseSectorForNewShip(const FOrgDef& Org) const
{
	// Where most of its ships already work, else its home.
	TMap<FName, int32> PerSector;
	for (const FShipRecord& Ship : Ships)
	{
		if (Ship.OwnerId == Org.Id && !Ship.bDestroyed && !Ship.SectorId.IsNone())
		{
			++PerSector.FindOrAdd(Ship.SectorId);
		}
	}
	FName Best = Org.HomeSectorId;
	int32 BestCount = 0;
	for (const TPair<FName, int32>& Pair : PerSector)
	{
		if (Pair.Value > BestCount)
		{
			Best = Pair.Key;
			BestCount = Pair.Value;
		}
	}
	return Best;
}

bool UOrganisationSubsystem::BuyShip(FOrgDef& Org)
{
	const int32 Owned = CountShipsOwnedBy(Org.Id);
	const int32 Limit = Org.Kind == EOrgKind::Authority ? FMath::Min(Org.TargetFleetSize, GetShipCap(Org.Kind)) : GetShipCap(Org.Kind);
	if (Owned >= Limit)
	{
		return false;
	}
	const EShipRole Role = ChooseRoleToBuy(Org);
	const int32 Price = Economy.ShipPrice[(int32)Role];
	if (Org.Credits < FMath::RoundToInt(Price * (1.0f + Economy.ReserveFraction)))
	{
		return false;
	}
	const FName SectorId = ChooseSectorForNewShip(Org);
	if (SectorId.IsNone())
	{
		return false;
	}

	FShipRecord Ship;
	for (int32 N = 1; ; ++N)
	{
		Ship.Id = FName(*FString::Printf(TEXT("%s_%d"), *Org.Id.ToString(), N));
		if (!ShipIndex.Contains(Ship.Id))
		{
			break;
		}
	}
	int32 SameRole = 0;
	for (const FShipRecord& Other : Ships)
	{
		if (Other.OwnerId == Org.Id && Other.Role == Role)
		{
			++SameRole;
			// Same Blueprint as its sister ships, if they name one.
			if (Ship.ShipClass.IsNull())
			{
				Ship.ShipClass = Other.ShipClass;
			}
		}
	}
	Ship.Name = FText::FromString(FString::Printf(TEXT("%s %s %d"), *Org.Name.ToString(), *RoleToString(Role), SameRole + 1));
	Ship.OwnerId = Org.Id;
	Ship.Role = Role;
	Ship.SectorId = SectorId;

	Org.Credits -= Price;
	++Ledgers.FindOrAdd(Org.Id).ShipsBought;
	UE_LOG(LogAdastrea, Log, TEXT("Economy: %s bought '%s' (%s) for %s in sector '%s'; wallet now %s"),
		*Org.Name.ToString(), *Ship.Name.ToString(), *RoleToString(Role), *Cr(Price), *SectorId.ToString(), *Cr(Org.Credits));

	Ships.Add(MoveTemp(Ship));
	RebuildShipIndex();
	return true;
}

// ---------------------------------------------------------------------------
// Save / load
// ---------------------------------------------------------------------------

void UOrganisationSubsystem::ExportState(FSavedOrganisations& Out) const
{
	Out.bValid = true;
	Out.Wallets.Reset();
	for (const FOrgDef& Org : Orgs)
	{
		Out.Wallets.Add(Org.Id, Org.Credits);
	}
	Out.Ledgers = Ledgers;
	Out.Ships = Ships;
	Out.TickProgress = TickAccumulator;
}

void UOrganisationSubsystem::ImportState(const FSavedOrganisations& In)
{
	if (!In.bValid)
	{
		return;
	}

	for (FOrgDef& Org : Orgs)
	{
		if (const int32* Credits = In.Wallets.Find(Org.Id))
		{
			Org.Credits = *Credits;
		}
	}
	Ledgers.Reset();
	for (const TPair<FName, FOrgLedger>& Pair : In.Ledgers)
	{
		if (FindOrg(Pair.Key))
		{
			Ledgers.Add(Pair.Key, Pair.Value);
		}
	}

	TArray<FShipRecord> Restored;
	TSet<FName> RestoredIds;
	for (const FShipRecord& Saved : In.Ships)
	{
		if (!FindOrg(Saved.OwnerId))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Organisations: dropping saved ship '%s' of unknown owner '%s'"),
				*Saved.Id.ToString(), *Saved.OwnerId.ToString());
			continue;
		}
		FShipRecord Ship = Saved;
		const FShipRecord* Live = FindShip(Ship.Id);
		Ship.SpawnedShip = (Live && !Ship.bDestroyed) ? Live->SpawnedShip : nullptr;
		RestoredIds.Add(Ship.Id);
		Restored.Add(MoveTemp(Ship));
	}
	int32 KeptLive = 0;
	for (const FShipRecord& Live : Ships)
	{
		if (Live.SpawnedShip.IsValid() && !RestoredIds.Contains(Live.Id))
		{
			Restored.Add(Live);
			++KeptLive;
		}
	}

	Ships = MoveTemp(Restored);
	RebuildShipIndex();
	TickAccumulator = FMath::Clamp(In.TickProgress, 0.0f, Economy.TickSeconds);
	++Revision;
	UE_LOG(LogAdastrea, Log, TEXT("Organisations: restored %d wallets, %d ships from the save (%d ships in view kept)"),
		In.Wallets.Num(), Ships.Num(), KeptLive);
	OnRosterChanged.Broadcast();
}
