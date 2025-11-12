// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rivals/AntagonistManager.h"
#include "Way/Feat.h"
#include "AdastreaLog.h"
#include "TimerManager.h"
#include "Engine/World.h"

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
	// Simple name generation based on goal
	// TODO: Implement more sophisticated name generation
	FString BaseName = TEXT("Rival");
	
	switch (Goal)
	{
		case EAntagonistGoal::Revenge:
			BaseName = TEXT("The Avenger");
			break;
		case EAntagonistGoal::Competition:
			BaseName = TEXT("The Challenger");
			break;
		case EAntagonistGoal::Obsession:
			BaseName = TEXT("The Stalker");
			break;
		case EAntagonistGoal::Jealousy:
			BaseName = TEXT("The Envious");
			break;
		case EAntagonistGoal::Honor:
			BaseName = TEXT("The Honorbound");
			break;
		case EAntagonistGoal::Curiosity:
			BaseName = TEXT("The Seeker");
			break;
		case EAntagonistGoal::Greed:
			BaseName = TEXT("The Profiteer");
			break;
		case EAntagonistGoal::Justice:
			BaseName = TEXT("The Justicar");
			break;
	}
	
	return FText::FromString(BaseName);
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
		World->GetTimerManager().SetTimer(UpdateTimerHandle, [this]()
		{
			UpdateAntagonists(60.0f);
		}, 60.0f, true);
	}
}

void UAntagonistManager::StopUpdateTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
}
