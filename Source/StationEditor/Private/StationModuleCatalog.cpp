// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationModuleCatalog.h"
#include "AdastreaLog.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

UStationModuleCatalog::UStationModuleCatalog()
{
	// Default constructor - entries should be configured in derived Blueprints,
	// or populated at runtime via LoadCatalogFromJson().
	bLoadedFromJson = false;
}

EStationModuleGroup UStationModuleCatalog::MapModuleGroup(const FString& Group)
{
	if (Group == TEXT("Docking"))    return EStationModuleGroup::Docking;
	if (Group == TEXT("Power"))      return EStationModuleGroup::Power;
	if (Group == TEXT("Storage"))    return EStationModuleGroup::Storage;
	if (Group == TEXT("Processing")) return EStationModuleGroup::Processing;
	if (Group == TEXT("Defence"))    return EStationModuleGroup::Defence;
	if (Group == TEXT("Habitation")) return EStationModuleGroup::Habitation;
	if (Group == TEXT("Public"))     return EStationModuleGroup::Public;
	if (Group == TEXT("Connection")) return EStationModuleGroup::Connection;
	// Default: Other
	return EStationModuleGroup::Other;
}

int32 UStationModuleCatalog::LoadCatalogFromJson()
{
	ModuleEntries.Empty();
	bLoadedFromJson = false;

	// Content/Data/StationModuleCatalog.json (mirrors CraftingTree.json location).
	const FString FullPath = FPaths::ProjectContentDir() + TEXT("Data/StationModuleCatalog.json");
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: could not read %s"), *FullPath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: failed to parse JSON"));
		return 0;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArr = nullptr;
	if (!Root->TryGetArrayField(TEXT("entries"), EntriesArr))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: no 'entries' array in JSON"));
		return 0;
	}

	for (const TSharedPtr<FJsonValue>& Val : *EntriesArr)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid())
		{
			continue;
		}

		FStationModuleEntry Entry;

		// Module class: /Script/Adastrea.<ClassName>
		FString ClassPath;
		Obj->TryGetStringField(TEXT("class_path"), ClassPath);
		if (!ClassPath.IsEmpty())
		{
			UClass* ResolvedClass = LoadClass<ASpaceStationModule>(nullptr, *ClassPath);
			if (ResolvedClass)
			{
				Entry.ModuleClass = ResolvedClass;
			}
			else
			{
				UE_LOG(LogAdastreaStations, Warning,
					TEXT("StationModuleCatalog: could not resolve class %s"), *ClassPath);
			}
		}

		FString DisplayName, Description, Group;
		int32 TechLevel = 1, Credits = 0;
		double BuildTime = 60.0;
		Obj->TryGetStringField(TEXT("display_name"), DisplayName);
		Obj->TryGetStringField(TEXT("description"), Description);
		Obj->TryGetStringField(TEXT("group"), Group);
		Obj->TryGetNumberField(TEXT("required_tech_level"), TechLevel);
		Obj->TryGetNumberField(TEXT("build_cost_credits"), Credits);
		Obj->TryGetNumberField(TEXT("build_time_seconds"), BuildTime);

		Entry.DisplayName = FText::FromString(DisplayName);
		Entry.Description = FText::FromString(Description);
		Entry.RequiredTechLevel = TechLevel;
		Entry.ModuleGroup = MapModuleGroup(Group);
		Entry.BuildCost.Credits = Credits;
		Entry.BuildCost.BuildTime = (float)BuildTime;

		// Materials: { "ItemID": qty }
		const TSharedPtr<FJsonObject>* MatsObj = nullptr;
		if (Obj->TryGetObjectField(TEXT("materials"), MatsObj) && MatsObj->IsValid())
		{
			for (const TPair<FString, TSharedPtr<FJsonValue>>& MPair : (*MatsObj)->Values)
			{
				int32 Qty = 0;
				MPair.Value->TryGetNumber(Qty);
				if (Qty > 0)
				{
					Entry.BuildCost.Materials.Add(FName(*MPair.Key), Qty);
				}
			}
		}

		ModuleEntries.Add(Entry);
	}

	bLoadedFromJson = ModuleEntries.Num() > 0;
	UE_LOG(LogAdastreaStations, Log,
		TEXT("StationModuleCatalog: loaded %d module entries from %s"),
		ModuleEntries.Num(), *FullPath);

	LoadFootprintsFromJson();

	return ModuleEntries.Num();
}

int32 UStationModuleCatalog::LoadFootprintsFromJson()
{
	// Content/Data/StationModuleBuilderData.json - the X4-style plan-mode builder's
	// per-module grid size/faces/power table (see STATION_BUILDER.md). Reused here so
	// the real-time editor's collision/adjacency checks respect each module's actual
	// footprint instead of treating every module as the same size.
	const FString FullPath = FPaths::ProjectContentDir() + TEXT("Data/StationModuleBuilderData.json");
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FullPath))
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: could not read %s (module footprints default to 1x1x1)"), *FullPath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: failed to parse %s"), *FullPath);
		return 0;
	}

	const TSharedPtr<FJsonObject>* ModulesObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("Modules"), ModulesObj) || !ModulesObj->IsValid())
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog: no 'Modules' object in %s"), *FullPath);
		return 0;
	}

	int32 NumMatched = 0;
	for (FStationModuleEntry& Entry : ModuleEntries)
	{
		if (!Entry.ModuleClass)
		{
			continue;
		}

		// ItemID keys in the JSON match the native class's short name (e.g.
		// "CorridorModule" for ACorridorModule - UClass::GetName() already drops
		// the 'A' prefix, same as the class_path convention in StationModuleCatalog.json).
		const FString ItemID = Entry.ModuleClass->GetName();
		const TSharedPtr<FJsonObject>* ModuleObj = nullptr;
		if (!(*ModulesObj)->TryGetObjectField(ItemID, ModuleObj) || !ModuleObj->IsValid())
		{
			continue;
		}

		const TArray<TSharedPtr<FJsonValue>>* SizeArr = nullptr;
		bool bMatchedThisEntry = false;
		if ((*ModuleObj)->TryGetArrayField(TEXT("size"), SizeArr) && SizeArr && SizeArr->Num() >= 3)
		{
			const int32 SX = FMath::Max(1, (int32)(*SizeArr)[0]->AsNumber());
			const int32 SY = FMath::Max(1, (int32)(*SizeArr)[1]->AsNumber());
			const int32 SZ = FMath::Max(1, (int32)(*SizeArr)[2]->AsNumber());
			Entry.GridFootprint = FIntVector(SX, SY, SZ);
			bMatchedThisEntry = true;
		}

		// "faces": "all" (string, most modules) leaves ConnectionFaces empty
		// (empty == unrestricted); "faces": ["W"] (array, e.g. SolarArrayModule)
		// records the specific faces it connects through.
		Entry.ConnectionFaces.Empty();
		const TArray<TSharedPtr<FJsonValue>>* FacesArr = nullptr;
		if ((*ModuleObj)->TryGetArrayField(TEXT("faces"), FacesArr) && FacesArr)
		{
			for (const TSharedPtr<FJsonValue>& FaceVal : *FacesArr)
			{
				FString FaceStr;
				if (FaceVal->TryGetString(FaceStr))
				{
					Entry.ConnectionFaces.Add(FName(*FaceStr));
				}
			}
		}
		// else: "faces" is the string "all" (or absent) - ConnectionFaces stays empty/unrestricted.

		if (bMatchedThisEntry)
		{
			++NumMatched;
		}
	}

	UE_LOG(LogAdastreaStations, Log,
		TEXT("StationModuleCatalog: matched footprints for %d/%d module entries from %s"),
		NumMatched, ModuleEntries.Num(), *FullPath);
	return NumMatched;
}

TArray<FStationModuleEntry> UStationModuleCatalog::GetAllModules() const
{
	return ModuleEntries;
}

TArray<FStationModuleEntry> UStationModuleCatalog::GetModulesByGroup(EStationModuleGroup Group) const
{
	TArray<FStationModuleEntry> FilteredEntries;

	// If requesting all modules, return copy of entire array
	if (Group == EStationModuleGroup::All)
	{
		return ModuleEntries;
	}

	// Filter by group
	FilteredEntries.Reserve(ModuleEntries.Num() / 4); // Estimate 25% might match

	for (const FStationModuleEntry& Entry : ModuleEntries)
	{
		if (Entry.ModuleGroup == Group)
		{
			FilteredEntries.Add(Entry);
		}
	}

	return FilteredEntries;
}

TArray<FStationModuleEntry> UStationModuleCatalog::GetModulesByTechLevel(int32 MaxTechLevel) const
{
	TArray<FStationModuleEntry> FilteredEntries;
	FilteredEntries.Reserve(ModuleEntries.Num());

	for (const FStationModuleEntry& Entry : ModuleEntries)
	{
		if (Entry.RequiredTechLevel <= MaxTechLevel)
		{
			FilteredEntries.Add(Entry);
		}
	}

	return FilteredEntries;
}

TArray<FStationModuleEntry> UStationModuleCatalog::GetModulesFiltered(EStationModuleGroup Group, int32 MaxTechLevel) const
{
	TArray<FStationModuleEntry> FilteredEntries;
	FilteredEntries.Reserve(ModuleEntries.Num() / 2); // Estimate 50% might match

	const bool bFilterByGroup = (Group != EStationModuleGroup::All);

	for (const FStationModuleEntry& Entry : ModuleEntries)
	{
		// Check tech level first (always checked)
		if (Entry.RequiredTechLevel > MaxTechLevel)
		{
			continue;
		}

		// Check group if filtering is enabled
		if (bFilterByGroup && Entry.ModuleGroup != Group)
		{
			continue;
		}

		FilteredEntries.Add(Entry);
	}

	return FilteredEntries;
}

bool UStationModuleCatalog::FindModuleByClass(TSubclassOf<ASpaceStationModule> ModuleClass, FStationModuleEntry& OutEntry) const
{
	if (!ModuleClass)
	{
		UE_LOG(LogAdastreaStations, Warning, TEXT("StationModuleCatalog::FindModuleByClass - Invalid module class"));
		return false;
	}

	for (const FStationModuleEntry& Entry : ModuleEntries)
	{
		if (Entry.ModuleClass == ModuleClass)
		{
			OutEntry = Entry;
			return true;
		}
	}

	return false;
}

int32 UStationModuleCatalog::GetModuleCount() const
{
	return ModuleEntries.Num();
}

bool UStationModuleCatalog::ContainsModule(TSubclassOf<ASpaceStationModule> ModuleClass) const
{
	if (!ModuleClass)
	{
		return false;
	}

	for (const FStationModuleEntry& Entry : ModuleEntries)
	{
		if (Entry.ModuleClass == ModuleClass)
		{
			return true;
		}
	}

	return false;
}
