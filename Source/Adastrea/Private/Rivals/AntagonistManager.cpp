// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rivals/AntagonistManager.h"
#include "Way/Feat.h"
#include "AdastreaLog.h"
#include "TimerManager.h"
#include "Engine/World.h"

// File-scope constants for antagonist name generation
namespace AntagonistNames
{
	static const TArray<FString>& GetPrefixes()
	{
		static TArray<FString> Prefixes = []()
		{
			TArray<FString> Arr;
			Arr.Reserve(10);
			Arr.Append({
				TEXT("Captain"), TEXT("Commander"), TEXT("Admiral"), TEXT("Lord"), TEXT("Lady"),
				TEXT("Baron"), TEXT("Baroness"), TEXT("Director"), TEXT("Overseer"), TEXT("Warlord")
			});
			return Arr;
		}();
		return Prefixes;
	}
}

UAntagonistManager::UAntagonistManager()
{
}

void UAntagonistManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Start the periodic update timer
	StartUpdateTimer();
}

void UAntagonistManager::Deinitialize()
{
	// Stop the update timer
	StopUpdateTimer();
	
	Super::Deinitialize();
}

void UAntagonistManager::OnFeatCompleted(UFeatDataAsset* CompletedFeat)
{
	if (!CompletedFeat)
	{
		return;
	}
	
	// TODO: Check if the Feat has an antagonist trigger
	// For now, this is a placeholder implementation
	UE_LOG(LogAdastrea, Log, TEXT("Feat completed: %s - checking for antagonist triggers"), *CompletedFeat->GetName());
}

FActiveAntagonist UAntagonistManager::SpawnAntagonist(UFeatDataAsset* SourceFeat, EAntagonistGoal Goal, int32 InitialHeat)
{
	FActiveAntagonist NewAntagonist;
	NewAntagonist.AntagonistID = FGuid::NewGuid();
	NewAntagonist.TriggeringFeat = SourceFeat;
	NewAntagonist.PrimaryGoal = Goal;
	NewAntagonist.HeatLevel = FMath::Clamp(InitialHeat, 0, 100);
	NewAntagonist.AntagonistName = GenerateAntagonistName(SourceFeat, Goal);
	NewAntagonist.CreationTime = FDateTime::Now();
	NewAntagonist.EncounterCount = 0;
	NewAntagonist.bIsActive = true;
	
	// Add to the list of antagonists
	Antagonists.Add(NewAntagonist);
	
	UE_LOG(LogAdastrea, Log, TEXT("Spawned new antagonist: %s (Heat: %d)"), 
		*NewAntagonist.AntagonistName.ToString(), NewAntagonist.HeatLevel);
	
	return NewAntagonist;
}

TArray<FActiveAntagonist> UAntagonistManager::GetActiveAntagonists() const
{
	TArray<FActiveAntagonist> ActiveList;
	
	for (const FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.bIsActive)
		{
			ActiveList.Add(Antagonist);
		}
	}
	
	return ActiveList;
}

bool UAntagonistManager::GetAntagonistByID(const FGuid& AntagonistID, FActiveAntagonist& OutAntagonist) const
{
	for (const FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.AntagonistID == AntagonistID)
		{
			OutAntagonist = Antagonist;
			return true;
		}
	}
	
	return false;
}

TArray<FActiveAntagonist> UAntagonistManager::GetAntagonistsByGoal(EAntagonistGoal Goal) const
{
	TArray<FActiveAntagonist> FilteredList;
	
	for (const FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.PrimaryGoal == Goal && Antagonist.bIsActive)
		{
			FilteredList.Add(Antagonist);
		}
	}
	
	return FilteredList;
}

TArray<FActiveAntagonist> UAntagonistManager::GetHighHeatAntagonists(int32 MinHeat) const
{
	TArray<FActiveAntagonist> HighHeatList;
	
	for (const FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.HeatLevel >= MinHeat && Antagonist.bIsActive)
		{
			HighHeatList.Add(Antagonist);
		}
	}
	
	return HighHeatList;
}

bool UAntagonistManager::HasActiveAntagonists() const
{
	for (const FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.bIsActive)
		{
			return true;
		}
	}
	
	return false;
}

bool UAntagonistManager::ModifyAntagonistHeat(const FGuid& AntagonistID, int32 HeatDelta)
{
	for (FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.AntagonistID == AntagonistID)
		{
			Antagonist.HeatLevel = FMath::Clamp(Antagonist.HeatLevel + HeatDelta, 0, 100);
			UE_LOG(LogAdastrea, Log, TEXT("Modified antagonist %s heat by %d to %d"), 
				*Antagonist.AntagonistName.ToString(), HeatDelta, Antagonist.HeatLevel);
			return true;
		}
	}
	
	return false;
}

bool UAntagonistManager::RecordEncounter(const FGuid& AntagonistID)
{
	for (FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.AntagonistID == AntagonistID)
		{
			Antagonist.EncounterCount++;
			// Increase heat slightly on encounter
			Antagonist.HeatLevel = FMath::Clamp(Antagonist.HeatLevel + 5, 0, 100);
			UE_LOG(LogAdastrea, Log, TEXT("Recorded encounter with antagonist %s (Total: %d)"), 
				*Antagonist.AntagonistName.ToString(), Antagonist.EncounterCount);
			return true;
		}
	}
	
	return false;
}

bool UAntagonistManager::DeactivateAntagonist(const FGuid& AntagonistID)
{
	for (FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.AntagonistID == AntagonistID)
		{
			Antagonist.bIsActive = false;
			UE_LOG(LogAdastrea, Log, TEXT("Deactivated antagonist: %s"), 
				*Antagonist.AntagonistName.ToString());
			return true;
		}
	}
	
	return false;
}

bool UAntagonistManager::ReactivateAntagonist(const FGuid& AntagonistID)
{
	for (FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.AntagonistID == AntagonistID)
		{
			Antagonist.bIsActive = true;
			UE_LOG(LogAdastrea, Log, TEXT("Reactivated antagonist: %s"), 
				*Antagonist.AntagonistName.ToString());
			return true;
		}
	}
	
	return false;
}

void UAntagonistManager::UpdateAntagonists(float DeltaTime)
{
	ApplyHeatDecay(DeltaTime);
}

void UAntagonistManager::ClearAllAntagonists()
{
	Antagonists.Empty();
	UE_LOG(LogAdastrea, Log, TEXT("Cleared all antagonists"));
}

int32 UAntagonistManager::GetTotalAntagonistCount() const
{
	return Antagonists.Num();
}

FText UAntagonistManager::GenerateAntagonistName(UFeatDataAsset* SourceFeat, EAntagonistGoal Goal) const
{
	// Surnames based on goal type
	TArray<FString> Surnames;
	switch (Goal)
	{
		case EAntagonistGoal::Revenge:
			Surnames = { TEXT("Vex"), TEXT("Rancor"), TEXT("Vendetta"), TEXT("Fury"), TEXT("Wraith") };
			break;
		case EAntagonistGoal::Competition:
			Surnames = { TEXT("Challenger"), TEXT("Rival"), TEXT("Defiant"), TEXT("Victor"), TEXT("Ascendant") };
			break;
		case EAntagonistGoal::Obsession:
			Surnames = { TEXT("Stalker"), TEXT("Hunter"), TEXT("Shadow"), TEXT("Watcher"), TEXT("Pursuer") };
			break;
		case EAntagonistGoal::Jealousy:
			Surnames = { TEXT("Envious"), TEXT("Covetous"), TEXT("Desirous"), TEXT("Grudge"), TEXT("Spite") };
			break;
		case EAntagonistGoal::Honor:
			Surnames = { TEXT("Honorbound"), TEXT("Duelist"), TEXT("Oath"), TEXT("Vanguard"), TEXT("Sentinel") };
			break;
		case EAntagonistGoal::Curiosity:
			Surnames = { TEXT("Seeker"), TEXT("Scholar"), TEXT("Inquirer"), TEXT("Delver"), TEXT("Explorer") };
			break;
		case EAntagonistGoal::Greed:
			Surnames = { TEXT("Profiteer"), TEXT("Hoarder"), TEXT("Tycoon"), TEXT("Mogul"), TEXT("Raider") };
			break;
		case EAntagonistGoal::Justice:
			Surnames = { TEXT("Justicar"), TEXT("Lawbringer"), TEXT("Arbiter"), TEXT("Judge"), TEXT("Enforcer") };
			break;
		default:
			Surnames = { TEXT("Nemesis"), TEXT("Adversary"), TEXT("Rival"), TEXT("Antagonist") };
			UE_LOG(LogAdastrea, Warning, TEXT("Unknown antagonist goal type (%d), using default names"), static_cast<int32>(Goal));
			break;
	}
	
	// Randomly select prefix and surname for variation
	const TArray<FString>& Prefixes = AntagonistNames::GetPrefixes();
	const int32 PrefixIndex = FMath::RandRange(0, Prefixes.Num() - 1);
	const int32 SurnameIndex = FMath::RandRange(0, Surnames.Num() - 1);
	
	const FString GeneratedName = FString::Printf(TEXT("%s %s"), *Prefixes[PrefixIndex], *Surnames[SurnameIndex]);
	return FText::FromString(GeneratedName);
}

void UAntagonistManager::ApplyHeatDecay(float DeltaTime)
{
	// Decay heat over time for all active antagonists
	const float HeatDecayRate = 0.5f; // Heat points per second
	const float HeatDecay = HeatDecayRate * DeltaTime;
	
	for (FActiveAntagonist& Antagonist : Antagonists)
	{
		if (Antagonist.bIsActive && Antagonist.HeatLevel > 0)
		{
			Antagonist.HeatLevel = FMath::Max(0, Antagonist.HeatLevel - FMath::RoundToInt(HeatDecay));
		}
	}
}

void UAntagonistManager::StartUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		// Update every 60 seconds
		const float UpdateInterval = 60.0f;
		World->GetTimerManager().SetTimer(UpdateTimerHandle, [this, UpdateInterval]()
		{
			UpdateAntagonists(UpdateInterval);
		}, UpdateInterval, true);
	}
}

void UAntagonistManager::StopUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
}
