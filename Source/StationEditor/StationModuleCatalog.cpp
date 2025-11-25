// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "StationModuleCatalog.h"
#include "AdastreaLog.h"

UStationModuleCatalog::UStationModuleCatalog()
{
	// Default constructor - entries should be configured in derived Blueprints
}

const TArray<FStationModuleEntry>& UStationModuleCatalog::GetAllModules() const
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
