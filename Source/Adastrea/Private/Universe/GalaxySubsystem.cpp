// Copyright Epic Games, Inc. All Rights Reserved.

#include "Universe/GalaxySubsystem.h"
#include "AdastreaLog.h"
#include "SpaceSectorMap.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

namespace
{
	TAutoConsoleVariable<int32> CVarSecurityOverride(
		TEXT("adastrea.Security"),
		-1,
		TEXT("Override the current sector's security for testing: -1 = the sector's own, 0 = None, 1 = Low, 2 = Medium, 3 = High."));

	FString JsonString(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		FString Out;
		Obj->TryGetStringField(Field, Out);
		return Out;
	}

	float JsonNumber(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, float Default)
	{
		double Out = Default;
		return Obj->TryGetNumberField(Field, Out) ? (float)Out : Default;
	}

	TArray<FName> JsonNameArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field)
	{
		TArray<FName> Out;
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Obj->TryGetArrayField(Field, Values))
		{
			for (const TSharedPtr<FJsonValue>& V : *Values)
			{
				FString S;
				if (V.IsValid() && V->TryGetString(S) && !S.IsEmpty())
				{
					Out.AddUnique(FName(*S));
				}
			}
		}
		return Out;
	}

	/** Reads [a, b, ...] into Out; returns the number of components read. */
	int32 JsonFloats(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, float* Out, int32 Max)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!Obj->TryGetArrayField(Field, Values))
		{
			return 0;
		}
		int32 N = 0;
		for (const TSharedPtr<FJsonValue>& V : *Values)
		{
			double D = 0.0;
			if (N < Max && V.IsValid() && V->TryGetNumber(D))
			{
				Out[N++] = (float)D;
			}
		}
		return N;
	}

	/** "/Game/Maps/TestLevel.TestLevel" -> "/Game/Maps/TestLevel". */
	FString NormalizeLevelPath(const FString& Path)
	{
		FString Package = Path;
		int32 Dot = INDEX_NONE;
		if (Package.FindChar(TEXT('.'), Dot))
		{
			Package.LeftInline(Dot);
		}
		return Package;
	}

	static FAutoConsoleCommandWithWorld GReloadGalaxyCmd(
		TEXT("adastrea.ReloadGalaxy"),
		TEXT("Re-read Content/Data/Universe/Galaxy.json into the galaxy map."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World))
			{
				Galaxy->ReloadGalaxy();
			}
		}));

	static FAutoConsoleCommandWithWorld GGalaxyInfoCmd(
		TEXT("adastrea.GalaxyInfo"),
		TEXT("Log the galaxy's systems and sectors, and which sector this level is."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			UGalaxySubsystem* Galaxy = UGalaxySubsystem::Get(World);
			if (!Galaxy)
			{
				return;
			}
			UE_LOG(LogAdastrea, Log, TEXT("Galaxy (%s): %d systems, start '%s', this level = '%s'"),
				Galaxy->IsLoadedFromFile() ? TEXT("Galaxy.json") : TEXT("built-in"),
				Galaxy->GetSystems().Num(), *Galaxy->GetStartSystemId().ToString(),
				*Galaxy->ResolveCurrentSectorId(World).ToString());
			for (const FStarSystemDef& Sys : Galaxy->GetSystems())
			{
				UE_LOG(LogAdastrea, Log, TEXT("  %s '%s' [%s] at (%.1f, %.1f) ly, %d sectors (%d built), lanes: %d"),
					*Sys.Id.ToString(), *Sys.Name.ToString(), *Sys.StarClass, Sys.Position.X, Sys.Position.Y,
					Sys.Sectors.Num(), Sys.NumBuiltSectors(), Sys.JumpLinks.Num());
				for (const FGalaxySectorDef& Sec : Sys.Sectors)
				{
					UE_LOG(LogAdastrea, Log, TEXT("    %s '%s' %s level=%s gates=%d laneGates=%d"),
						*Sec.Id.ToString(), *Sec.Name.ToString(), *Sec.Type,
						Sec.HasLevel() ? *Sec.Level : TEXT("(planned)"), Sec.Gates.Num(), Sec.LaneGates.Num());
				}
			}
		}));
}

void UGalaxySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadGalaxy();
}

UGalaxySubsystem* UGalaxySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UGalaxySubsystem>() : nullptr;
}

FString UGalaxySubsystem::GetGalaxyFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("Data/Universe/Galaxy.json");
}

bool UGalaxySubsystem::ReloadGalaxy()
{
	const FString Path = GetGalaxyFilePath();
	FString Error;
	bLoadedFromFile = LoadFromFile(Path, Error);
	if (!bLoadedFromFile)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: %s (%s). Using the built-in galaxy."), *Error, *Path);
		BuildDefaultGalaxy();
	}
	FinalizeGalaxy();
	++Revision;

	int32 NumSectors = 0;
	for (const FStarSystemDef& Sys : Systems) { NumSectors += Sys.Sectors.Num(); }
	UE_LOG(LogAdastrea, Log, TEXT("Galaxy: loaded %d systems, %d sectors from %s (start system '%s')"),
		Systems.Num(), NumSectors, bLoadedFromFile ? TEXT("Galaxy.json") : TEXT("built-in defaults"),
		*StartSystemId.ToString());
	return bLoadedFromFile;
}

bool UGalaxySubsystem::LoadFromFile(const FString& Path, FString& OutError)
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

	const TArray<TSharedPtr<FJsonValue>>* SystemValues = nullptr;
	if (!Root->TryGetArrayField(TEXT("systems"), SystemValues) || SystemValues->Num() == 0)
	{
		OutError = TEXT("no \"systems\" array");
		return false;
	}

	TArray<FStarSystemDef> Loaded;
	for (const TSharedPtr<FJsonValue>& SysValue : *SystemValues)
	{
		const TSharedPtr<FJsonObject> SysObj = SysValue.IsValid() ? SysValue->AsObject() : nullptr;
		if (!SysObj.IsValid())
		{
			continue;
		}

		FStarSystemDef Sys;
		Sys.Id = FName(*JsonString(SysObj, TEXT("id")));
		if (Sys.Id.IsNone())
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: skipping a system with no id"));
			continue;
		}
		const FString SysName = JsonString(SysObj, TEXT("name"));
		Sys.Name = FText::FromString(SysName.IsEmpty() ? Sys.Id.ToString() : SysName);
		Sys.StarClass = JsonString(SysObj, TEXT("starClass"));
		Sys.Faction = JsonString(SysObj, TEXT("faction"));
		Sys.Description = FText::FromString(JsonString(SysObj, TEXT("description")));
		Sys.JumpLinks = JsonNameArray(SysObj, TEXT("jumpLinks"));

		float Color[3];
		if (JsonFloats(SysObj, TEXT("starColor"), Color, 3) == 3)
		{
			Sys.StarColor = FLinearColor(Color[0], Color[1], Color[2], 1.0f);
		}
		float Pos[2];
		if (JsonFloats(SysObj, TEXT("position"), Pos, 2) == 2)
		{
			Sys.Position = FVector2D(Pos[0], Pos[1]);
		}

		const TArray<TSharedPtr<FJsonValue>>* SectorValues = nullptr;
		if (SysObj->TryGetArrayField(TEXT("sectors"), SectorValues))
		{
			for (const TSharedPtr<FJsonValue>& SecValue : *SectorValues)
			{
				const TSharedPtr<FJsonObject> SecObj = SecValue.IsValid() ? SecValue->AsObject() : nullptr;
				if (!SecObj.IsValid())
				{
					continue;
				}
				FGalaxySectorDef Sec;
				Sec.Id = FName(*JsonString(SecObj, TEXT("id")));
				if (Sec.Id.IsNone())
				{
					UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: skipping a sector with no id in system '%s'"), *Sys.Id.ToString());
					continue;
				}
				const FString SecName = JsonString(SecObj, TEXT("name"));
				Sec.Name = FText::FromString(SecName.IsEmpty() ? Sec.Id.ToString() : SecName);
				Sec.Type = JsonString(SecObj, TEXT("type"));
				Sec.Security = JsonString(SecObj, TEXT("security"));
				Sec.Faction = JsonString(SecObj, TEXT("faction"));
				Sec.Description = FText::FromString(JsonString(SecObj, TEXT("description")));
				Sec.Level = NormalizeLevelPath(JsonString(SecObj, TEXT("level")));
				Sec.OrbitRadius = FMath::Clamp(JsonNumber(SecObj, TEXT("orbitRadius"), 0.5f), 0.1f, 1.0f);
				Sec.OrbitAngle = JsonNumber(SecObj, TEXT("orbitAngle"), 0.0f);
				Sec.Gates = JsonNameArray(SecObj, TEXT("gates"));
				Sec.LaneGates = JsonNameArray(SecObj, TEXT("laneGates"));
				Sys.Sectors.Add(MoveTemp(Sec));
			}
		}
		Loaded.Add(MoveTemp(Sys));
	}

	if (Loaded.Num() == 0)
	{
		OutError = TEXT("no valid systems");
		return false;
	}

	Systems = MoveTemp(Loaded);
	StartSystemId = FName(*JsonString(Root, TEXT("startSystem")));
	return true;
}

void UGalaxySubsystem::BuildDefaultGalaxy()
{
	Systems.Reset();

	FStarSystemDef Home;
	Home.Id = TEXT("adastrea");
	Home.Name = FText::FromString(TEXT("Adastrea"));
	Home.StarClass = TEXT("G2V");
	Home.StarColor = FLinearColor(1.0f, 0.86f, 0.55f, 1.0f);
	Home.Description = FText::FromString(TEXT("Built-in fallback galaxy: Galaxy.json is missing or invalid."));
	Home.JumpLinks = { TEXT("kestrel") };
	{
		FGalaxySectorDef Prime;
		Prime.Id = TEXT("adastrea_prime");
		Prime.Name = FText::FromString(TEXT("Adastrea Prime"));
		Prime.Type = TEXT("Trade Hub");
		Prime.Security = TEXT("High");
		Prime.Level = TEXT("/Game/Maps/TestLevel");
		Prime.OrbitRadius = 0.4f;
		Prime.OrbitAngle = 200.0f;
		Prime.Gates = { TEXT("adastrea_outer") };
		Home.Sectors.Add(Prime);

		FGalaxySectorDef Outer;
		Outer.Id = TEXT("adastrea_outer");
		Outer.Name = FText::FromString(TEXT("Outer Relay"));
		Outer.Type = TEXT("Frontier");
		Outer.Security = TEXT("Low");
		Outer.OrbitRadius = 0.85f;
		Outer.OrbitAngle = 30.0f;
		Home.Sectors.Add(Outer);
	}
	Systems.Add(Home);

	FStarSystemDef Kestrel;
	Kestrel.Id = TEXT("kestrel");
	Kestrel.Name = FText::FromString(TEXT("Kestrel"));
	Kestrel.StarClass = TEXT("K5V");
	Kestrel.StarColor = FLinearColor(1.0f, 0.62f, 0.32f, 1.0f);
	Kestrel.Position = FVector2D(9.0f, 4.0f);
	Systems.Add(Kestrel);

	StartSystemId = Home.Id;
}

void UGalaxySubsystem::FinalizeGalaxy()
{
	SystemIndex.Reset();
	SectorIndex.Reset();

	// Index, dropping duplicate IDs (first one wins).
	for (int32 SysIdx = 0; SysIdx < Systems.Num(); )
	{
		FStarSystemDef& Sys = Systems[SysIdx];
		if (SystemIndex.Contains(Sys.Id))
		{
			UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: duplicate system id '%s' ignored"), *Sys.Id.ToString());
			Systems.RemoveAt(SysIdx);
			continue;
		}
		for (int32 SecIdx = 0; SecIdx < Sys.Sectors.Num(); )
		{
			FGalaxySectorDef& Sec = Sys.Sectors[SecIdx];
			if (SectorIndex.Contains(Sec.Id))
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: duplicate sector id '%s' ignored"), *Sec.Id.ToString());
				Sys.Sectors.RemoveAt(SecIdx);
				continue;
			}
			Sec.SystemId = Sys.Id;
			++SecIdx;
		}
		++SysIdx;
	}
	for (int32 SysIdx = 0; SysIdx < Systems.Num(); ++SysIdx)
	{
		SystemIndex.Add(Systems[SysIdx].Id, SysIdx);
		for (int32 SecIdx = 0; SecIdx < Systems[SysIdx].Sectors.Num(); ++SecIdx)
		{
			SectorIndex.Add(Systems[SysIdx].Sectors[SecIdx].Id, TPair<int32, int32>(SysIdx, SecIdx));
		}
	}

	// Jump lanes: drop dangling/self links, then make every lane two-way.
	for (FStarSystemDef& Sys : Systems)
	{
		Sys.JumpLinks.RemoveAll([&](FName Link)
		{
			const bool bBad = Link == Sys.Id || !SystemIndex.Contains(Link);
			if (bBad)
			{
				UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: system '%s' jump link to unknown system '%s' dropped"), *Sys.Id.ToString(), *Link.ToString());
			}
			return bBad;
		});
	}
	for (int32 SysIdx = 0; SysIdx < Systems.Num(); ++SysIdx)
	{
		for (const FName Link : TArray<FName>(Systems[SysIdx].JumpLinks))
		{
			Systems[SystemIndex[Link]].JumpLinks.AddUnique(Systems[SysIdx].Id);
		}
	}

	// Gates: same-system sectors only, two-way.
	for (FStarSystemDef& Sys : Systems)
	{
		for (FGalaxySectorDef& Sec : Sys.Sectors)
		{
			Sec.Gates.RemoveAll([&](FName Gate)
			{
				const TPair<int32, int32>* Where = SectorIndex.Find(Gate);
				const bool bBad = Gate == Sec.Id || !Where || Systems[Where->Key].Id != Sys.Id;
				if (bBad)
				{
					UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: sector '%s' gate to '%s' dropped (unknown or in another system; use jumpLinks between systems)"),
						*Sec.Id.ToString(), *Gate.ToString());
				}
				return bBad;
			});
		}
		for (int32 SecIdx = 0; SecIdx < Sys.Sectors.Num(); ++SecIdx)
		{
			for (const FName Gate : TArray<FName>(Sys.Sectors[SecIdx].Gates))
			{
				Sys.Sectors[SectorIndex[Gate].Value].Gates.AddUnique(Sys.Sectors[SecIdx].Id);
			}
		}
	}

	// Lane gates: to a sector in a lane-linked system only, two-way.
	for (FStarSystemDef& Sys : Systems)
	{
		for (FGalaxySectorDef& Sec : Sys.Sectors)
		{
			Sec.LaneGates.RemoveAll([&](FName Gate)
			{
				const TPair<int32, int32>* Where = SectorIndex.Find(Gate);
				const bool bBad = !Where || Systems[Where->Key].Id == Sys.Id || !Sys.JumpLinks.Contains(Systems[Where->Key].Id);
				if (bBad)
				{
					UE_LOG(LogAdastrea, Warning, TEXT("Galaxy: sector '%s' lane gate to '%s' dropped (unknown, same system, or no jump lane between the systems)"),
						*Sec.Id.ToString(), *Gate.ToString());
				}
				return bBad;
			});
		}
	}
	for (int32 SysIdx = 0; SysIdx < Systems.Num(); ++SysIdx)
	{
		for (int32 SecIdx = 0; SecIdx < Systems[SysIdx].Sectors.Num(); ++SecIdx)
		{
			const FName FromId = Systems[SysIdx].Sectors[SecIdx].Id;
			for (const FName Gate : TArray<FName>(Systems[SysIdx].Sectors[SecIdx].LaneGates))
			{
				const TPair<int32, int32> Where = SectorIndex[Gate];
				Systems[Where.Key].Sectors[Where.Value].LaneGates.AddUnique(FromId);
			}
		}
	}

	if (!SystemIndex.Contains(StartSystemId) && Systems.Num() > 0)
	{
		StartSystemId = Systems[0].Id;
	}
}

const FStarSystemDef* UGalaxySubsystem::FindSystem(FName SystemId) const
{
	const int32* Idx = SystemIndex.Find(SystemId);
	return Idx ? &Systems[*Idx] : nullptr;
}

const FGalaxySectorDef* UGalaxySubsystem::FindSector(FName SectorId) const
{
	const TPair<int32, int32>* Where = SectorIndex.Find(SectorId);
	return Where ? &Systems[Where->Key].Sectors[Where->Value] : nullptr;
}

bool UGalaxySubsystem::GetStarSystem(FName SystemId, FStarSystemDef& OutSystem) const
{
	if (const FStarSystemDef* Sys = FindSystem(SystemId))
	{
		OutSystem = *Sys;
		return true;
	}
	return false;
}

bool UGalaxySubsystem::GetSector(FName SectorId, FGalaxySectorDef& OutSector) const
{
	if (const FGalaxySectorDef* Sec = FindSector(SectorId))
	{
		OutSector = *Sec;
		return true;
	}
	return false;
}

FName UGalaxySubsystem::ResolveCurrentSectorId(const UObject* WorldContextObject) const
{
	UWorld* World = (WorldContextObject && GEngine)
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return NAME_None;
	}

	// 1. Sector markers placed in the level.
	const APlayerController* PC = World->GetFirstPlayerController();
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	FName FirstMarked = NAME_None;
	for (TActorIterator<ASpaceSectorMap> It(World); It; ++It)
	{
		const FName Id = It->SectorId;
		if (Id.IsNone() || !FindSector(Id))
		{
			continue;
		}
		if (Pawn && It->IsPositionInSector(Pawn->GetActorLocation()))
		{
			return Id;
		}
		if (FirstMarked.IsNone())
		{
			FirstMarked = Id;
		}
	}
	if (!FirstMarked.IsNone())
	{
		return FirstMarked;
	}

	// 2. The level itself is listed as a sector's level.
	const FString MapPackage = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
	for (const FStarSystemDef& Sys : Systems)
	{
		for (const FGalaxySectorDef& Sec : Sys.Sectors)
		{
			if (Sec.HasLevel() && Sec.Level.Equals(MapPackage, ESearchCase::IgnoreCase))
			{
				return Sec.Id;
			}
		}
	}
	return NAME_None;
}

ESectorSecurity UGalaxySubsystem::ParseSecurity(const FString& Text)
{
	if (Text.Equals(TEXT("High"), ESearchCase::IgnoreCase))
	{
		return ESectorSecurity::High;
	}
	if (Text.Equals(TEXT("Medium"), ESearchCase::IgnoreCase))
	{
		return ESectorSecurity::Medium;
	}
	if (Text.Equals(TEXT("Low"), ESearchCase::IgnoreCase))
	{
		return ESectorSecurity::Low;
	}
	return ESectorSecurity::None;
}

FString UGalaxySubsystem::SecurityToString(ESectorSecurity Level)
{
	switch (Level)
	{
	case ESectorSecurity::High:		return TEXT("High");
	case ESectorSecurity::Medium:	return TEXT("Medium");
	case ESectorSecurity::Low:		return TEXT("Low");
	default:						return TEXT("None");
	}
}

ESectorSecurity UGalaxySubsystem::GetCurrentSecurity(const UObject* WorldContextObject) const
{
	const int32 Override = CVarSecurityOverride.GetValueOnGameThread();
	if (Override >= 0)
	{
		return static_cast<ESectorSecurity>(FMath::Min(Override, static_cast<int32>(ESectorSecurity::High)));
	}
	const FGalaxySectorDef* Sector = FindSector(ResolveCurrentSectorId(WorldContextObject));
	return Sector ? ParseSecurity(Sector->Security) : ESectorSecurity::None;
}
