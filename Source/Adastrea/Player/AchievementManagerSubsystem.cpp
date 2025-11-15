#include "Player/AchievementManagerSubsystem.h"
#include "Player/PlayerProgressionComponent.h"
#include "Player/PlayerUnlockComponent.h"
#include "Player/AdastreaGameInstance.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UAchievementManagerSubsystem::UAchievementManagerSubsystem()
{
}

void UAchievementManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Initialized"));
}

void UAchievementManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Deinitialized"));
}

void UAchievementManagerSubsystem::RegisterAchievement(UAchievementDataAsset* Achievement)
{
	if (!Achievement)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AchievementManagerSubsystem: Cannot register null achievement"));
		return;
	}

	// Check if already registered
	if (FindAchievementTracker(Achievement->AchievementID))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AchievementManagerSubsystem: Achievement already registered: %s"),
			*Achievement->AchievementID.ToString());
		return;
	}

	// Register achievement
	FAchievementTracker Tracker(Achievement);
	RegisteredAchievements.Add(Tracker);

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Registered achievement: %s (%s)"),
		*Achievement->AchievementID.ToString(), *Achievement->DisplayName.ToString());
}

void UAchievementManagerSubsystem::RegisterAchievements(const TArray<UAchievementDataAsset*>& Achievements)
{
	for (UAchievementDataAsset* Achievement : Achievements)
	{
		RegisterAchievement(Achievement);
	}
}

void UAchievementManagerSubsystem::UpdateAchievementProgress(FName AchievementID, int32 ProgressIncrement, bool bAutoAward)
{
	FAchievementTracker* Tracker = FindAchievementTracker(AchievementID);
	if (!Tracker || !Tracker->Achievement)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AchievementManagerSubsystem: Achievement not found: %s"),
			*AchievementID.ToString());
		return;
	}

	// Already completed
	if (Tracker->Progress.bIsCompleted)
	{
		return;
	}

	// Update progress
	Tracker->Progress.CurrentProgress = FMath::Min(
		Tracker->Progress.CurrentProgress + ProgressIncrement,
		Tracker->Progress.TargetProgress
	);

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Updated achievement progress: %s (%d/%d)"),
		*AchievementID.ToString(), Tracker->Progress.CurrentProgress, Tracker->Progress.TargetProgress);

	// Broadcast progress update
	OnAchievementProgressUpdated.Broadcast(
		AchievementID,
		Tracker->Progress.CurrentProgress,
		Tracker->Progress.TargetProgress
	);

	// Check if completed
	if (bAutoAward && Tracker->Progress.CurrentProgress >= Tracker->Progress.TargetProgress)
	{
		AwardAchievement(AchievementID, true);
	}
}

void UAchievementManagerSubsystem::UpdateAchievementStat(FName StatName, int32 StatIncrement, bool bAutoAwardAchievements)
{
	// Update stat value
	int32& StatValue = AchievementStats.FindOrAdd(StatName);
	StatValue += StatIncrement;

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Updated stat: %s = %d"),
		*StatName.ToString(), StatValue);

	// Broadcast stat update
	OnStatUpdated.Broadcast(StatName, StatValue);

	// Check stat-based achievements
	if (bAutoAwardAchievements)
	{
		CheckStatBasedAchievements(StatName);
	}
}

bool UAchievementManagerSubsystem::AwardAchievement(FName AchievementID, bool bGrantRewards)
{
	FAchievementTracker* Tracker = FindAchievementTracker(AchievementID);
	if (!Tracker || !Tracker->Achievement)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AchievementManagerSubsystem: Cannot award - achievement not found: %s"),
			*AchievementID.ToString());
		return false;
	}

	// Already completed
	if (Tracker->Progress.bIsCompleted)
	{
		UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Achievement already completed: %s"),
			*AchievementID.ToString());
		return false;
	}

	// Mark as completed
	Tracker->Progress.bIsCompleted = true;
	Tracker->Progress.CurrentProgress = Tracker->Progress.TargetProgress;
	Tracker->Progress.CompletedTimestamp = FDateTime::Now();
	CompletedAchievements.AddUnique(AchievementID);

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Achievement unlocked: %s (%s)"),
		*AchievementID.ToString(), *Tracker->Achievement->DisplayName.ToString());

	// Broadcast achievement unlocked
	OnAchievementUnlocked.Broadcast(AchievementID, Tracker->Achievement);

	// Grant rewards
	if (bGrantRewards)
	{
		GrantAchievementRewards(Tracker->Achievement);
	}

	return true;
}

void UAchievementManagerSubsystem::ResetAchievement(FName AchievementID)
{
	FAchievementTracker* Tracker = FindAchievementTracker(AchievementID);
	if (!Tracker || !Tracker->Achievement)
	{
		return;
	}

	// Reset progress
	Tracker->Progress.CurrentProgress = 0;
	Tracker->Progress.bIsCompleted = false;
	Tracker->Progress.CompletedTimestamp = FDateTime::MinValue();
	CompletedAchievements.Remove(AchievementID);

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Reset achievement: %s"), *AchievementID.ToString());
}

void UAchievementManagerSubsystem::ResetAllAchievements()
{
	for (FAchievementTracker& Tracker : RegisteredAchievements)
	{
		Tracker.Progress.CurrentProgress = 0;
		Tracker.Progress.bIsCompleted = false;
		Tracker.Progress.CompletedTimestamp = FDateTime::MinValue();
	}

	CompletedAchievements.Empty();
	AchievementStats.Empty();

	UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: All achievements reset"));
}

bool UAchievementManagerSubsystem::IsAchievementCompleted(FName AchievementID) const
{
	return CompletedAchievements.Contains(AchievementID);
}

bool UAchievementManagerSubsystem::GetAchievementProgress(FName AchievementID, FAchievementProgress& OutProgress) const
{
	const FAchievementTracker* Tracker = FindAchievementTracker(AchievementID);
	if (Tracker)
	{
		OutProgress = Tracker->Progress;
		return true;
	}
	return false;
}

TArray<FAchievementTracker> UAchievementManagerSubsystem::GetAchievementsByCategory(EAchievementCategory Category) const
{
	TArray<FAchievementTracker> Result;

	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Achievement && Tracker.Achievement->Category == Category)
		{
			Result.Add(Tracker);
		}
	}

	return Result;
}

TArray<FAchievementTracker> UAchievementManagerSubsystem::GetAchievementsByRarity(EAchievementRarity Rarity) const
{
	TArray<FAchievementTracker> Result;

	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Achievement && Tracker.Achievement->Rarity == Rarity)
		{
			Result.Add(Tracker);
		}
	}

	return Result;
}

TArray<FAchievementTracker> UAchievementManagerSubsystem::GetCompletedAchievements() const
{
	TArray<FAchievementTracker> Result;

	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Progress.bIsCompleted)
		{
			Result.Add(Tracker);
		}
	}

	return Result;
}

TArray<FAchievementTracker> UAchievementManagerSubsystem::GetInProgressAchievements() const
{
	TArray<FAchievementTracker> Result;

	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (!Tracker.Progress.bIsCompleted && Tracker.Progress.CurrentProgress > 0)
		{
			Result.Add(Tracker);
		}
	}

	return Result;
}

float UAchievementManagerSubsystem::GetOverallCompletionPercentage() const
{
	if (RegisteredAchievements.Num() == 0)
	{
		return 1.0f;
	}

	return static_cast<float>(CompletedAchievements.Num()) / static_cast<float>(RegisteredAchievements.Num());
}

int32 UAchievementManagerSubsystem::GetTotalAchievementPoints() const
{
	int32 TotalPoints = 0;

	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Progress.bIsCompleted && Tracker.Achievement)
		{
			TotalPoints += GetPointsForRarity(Tracker.Achievement->Rarity);
		}
	}

	return TotalPoints;
}

int32 UAchievementManagerSubsystem::GetStatValue(FName StatName) const
{
	const int32* Value = AchievementStats.Find(StatName);
	return Value ? *Value : 0;
}

TArray<FAchievementTracker> UAchievementManagerSubsystem::GetRecentlyCompletedAchievements(int32 MaxCount) const
{
	TArray<FAchievementTracker> Completed = GetCompletedAchievements();

	// Sort by completion timestamp (most recent first)
	Completed.Sort([](const FAchievementTracker& A, const FAchievementTracker& B)
	{
		return A.Progress.CompletedTimestamp > B.Progress.CompletedTimestamp;
	});

	// Return only MaxCount results
	if (Completed.Num() > MaxCount)
	{
		Completed.SetNum(MaxCount);
	}

	return Completed;
}

FAchievementTracker* UAchievementManagerSubsystem::FindAchievementTracker(FName AchievementID)
{
	for (FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Achievement && Tracker.Achievement->AchievementID == AchievementID)
		{
			return &Tracker;
		}
	}
	return nullptr;
}

const FAchievementTracker* UAchievementManagerSubsystem::FindAchievementTracker(FName AchievementID) const
{
	for (const FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (Tracker.Achievement && Tracker.Achievement->AchievementID == AchievementID)
		{
			return &Tracker;
		}
	}
	return nullptr;
}

void UAchievementManagerSubsystem::GrantAchievementRewards(UAchievementDataAsset* Achievement)
{
	if (!Achievement)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FAchievementReward& Rewards = Achievement->Rewards;

	// Award credits
	if (Rewards.Credits > 0)
	{
		UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(UGameplayStatics::GetGameInstance(World));
		if (GameInstance)
		{
			GameInstance->ModifyPlayerCredits(Rewards.Credits);
			UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Awarded %d credits"), Rewards.Credits);
		}
	}

	// Award XP
	if (Rewards.ExperiencePoints > 0)
	{
		// Find player progression component
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (PlayerPawn)
			{
				UPlayerProgressionComponent* ProgressionComp = PlayerPawn->FindComponentByClass<UPlayerProgressionComponent>();
				if (ProgressionComp)
				{
					ProgressionComp->AwardExperience(Rewards.ExperiencePoints, true);
					UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Awarded %d XP"), Rewards.ExperiencePoints);
				}
			}
		}
	}

	// Unlock content
	if (Rewards.UnlockID != NAME_None)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (PlayerPawn)
			{
				UPlayerUnlockComponent* UnlockComp = PlayerPawn->FindComponentByClass<UPlayerUnlockComponent>();
				if (UnlockComp)
				{
					UnlockComp->ForceUnlock(Rewards.UnlockID);
					UE_LOG(LogAdastrea, Log, TEXT("AchievementManagerSubsystem: Unlocked %s"), *Rewards.UnlockID.ToString());
				}
			}
		}
	}

	// TODO: Award items via inventory system
}

void UAchievementManagerSubsystem::CheckStatBasedAchievements(FName StatName)
{
	int32 StatValue = GetStatValue(StatName);

	// Check all achievements that track this stat
	for (FAchievementTracker& Tracker : RegisteredAchievements)
	{
		if (!Tracker.Achievement || !Tracker.Achievement->bAutoTrackedByStat)
		{
			continue;
		}

		if (Tracker.Achievement->TrackingStatName != StatName)
		{
			continue;
		}

		if (Tracker.Progress.bIsCompleted)
		{
			continue;
		}

		// Update progress based on stat value
		if (StatValue >= Tracker.Progress.TargetProgress)
		{
			AwardAchievement(Tracker.Achievement->AchievementID, true);
		}
		else
		{
			Tracker.Progress.CurrentProgress = StatValue;
			OnAchievementProgressUpdated.Broadcast(
				Tracker.Achievement->AchievementID,
				Tracker.Progress.CurrentProgress,
				Tracker.Progress.TargetProgress
			);
		}
	}
}

int32 UAchievementManagerSubsystem::GetPointsForRarity(EAchievementRarity Rarity) const
{
	switch (Rarity)
	{
		case EAchievementRarity::Common:
			return 10;
		case EAchievementRarity::Uncommon:
			return 25;
		case EAchievementRarity::Rare:
			return 50;
		case EAchievementRarity::Epic:
			return 100;
		case EAchievementRarity::Legendary:
			return 250;
		default:
			return 0;
	}
}
