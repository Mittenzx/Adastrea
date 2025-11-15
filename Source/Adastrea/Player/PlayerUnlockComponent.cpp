#include "Player/PlayerUnlockComponent.h"
#include "Player/PlayerProgressionComponent.h"
#include "Player/PlayerReputationComponent.h"
#include "Player/AdastreaGameInstance.h"
#include "AdastreaLog.h"
#include "Kismet/GameplayStatics.h"

UPlayerUnlockComponent::UPlayerUnlockComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerUnlockComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeUnlocks();
}

void UPlayerUnlockComponent::InitializeUnlocks()
{
	// Unlock default content
	for (FUnlockEntry& Unlock : Unlocks)
	{
		if (Unlock.bUnlockedByDefault && !Unlock.bIsUnlocked)
		{
			Unlock.bIsUnlocked = true;
			UnlockedIDs.AddUnique(Unlock.UnlockID);
			
			UE_LOG(LogAdastrea, Log, TEXT("PlayerUnlockComponent: Default unlock: %s"), *Unlock.UnlockID.ToString());
		}
	}

	UE_LOG(LogAdastrea, Log, TEXT("PlayerUnlockComponent: Initialized %d unlocks (%d unlocked by default)"),
		Unlocks.Num(), UnlockedIDs.Num());
}

bool UPlayerUnlockComponent::TryUnlock(FName UnlockID, bool bIgnoreRequirements)
{
	FUnlockEntry* Unlock = FindUnlock(UnlockID);
	if (!Unlock)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Unlock not found: %s"), *UnlockID.ToString());
		OnUnlockFailed.Broadcast(UnlockID, FText::FromString("Unlock not found"));
		return false;
	}

	// Already unlocked
	if (Unlock->bIsUnlocked)
	{
		UE_LOG(LogAdastrea, Log, TEXT("PlayerUnlockComponent: Already unlocked: %s"), *UnlockID.ToString());
		return true;
	}

	// Check requirements
	if (!bIgnoreRequirements && !MeetsRequirements(UnlockID))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Requirements not met for: %s"), *UnlockID.ToString());
		OnUnlockFailed.Broadcast(UnlockID, FText::FromString("Requirements not met"));
		return false;
	}

	// Unlock content
	Unlock->bIsUnlocked = true;
	UnlockedIDs.AddUnique(UnlockID);

	UE_LOG(LogAdastrea, Log, TEXT("PlayerUnlockComponent: Unlocked: %s (%s)"),
		*UnlockID.ToString(), *Unlock->DisplayName.ToString());

	// Broadcast event
	OnContentUnlocked.Broadcast(UnlockID, Unlock->Type);

	return true;
}

void UPlayerUnlockComponent::ForceUnlock(FName UnlockID)
{
	TryUnlock(UnlockID, true);
}

bool UPlayerUnlockComponent::IsUnlocked(FName UnlockID) const
{
	return UnlockedIDs.Contains(UnlockID);
}

bool UPlayerUnlockComponent::MeetsRequirements(FName UnlockID) const
{
	const FUnlockEntry* Unlock = FindUnlock(UnlockID);
	if (!Unlock)
	{
		return false;
	}

	// Check all requirements
	for (const FUnlockRequirement& Requirement : Unlock->Requirements)
	{
		if (!CheckRequirement(Requirement))
		{
			return false;
		}
	}

	return true;
}

float UPlayerUnlockComponent::GetUnlockProgress(FName UnlockID) const
{
	const FUnlockEntry* Unlock = FindUnlock(UnlockID);
	if (!Unlock)
	{
		return 0.0f;
	}

	// Already unlocked
	if (Unlock->bIsUnlocked)
	{
		return 1.0f;
	}

	// Calculate progress based on met requirements
	if (Unlock->Requirements.Num() == 0)
	{
		return 1.0f; // No requirements = ready to unlock
	}

	int32 MetRequirements = 0;
	for (const FUnlockRequirement& Requirement : Unlock->Requirements)
	{
		if (CheckRequirement(Requirement))
		{
			MetRequirements++;
		}
	}

	return static_cast<float>(MetRequirements) / static_cast<float>(Unlock->Requirements.Num());
}

void UPlayerUnlockComponent::AddUnlock(const FUnlockEntry& NewUnlock)
{
	// Check if unlock already exists
	if (FindUnlock(NewUnlock.UnlockID))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Unlock already exists: %s"), *NewUnlock.UnlockID.ToString());
		return;
	}

	Unlocks.Add(NewUnlock);
	UE_LOG(LogAdastrea, Log, TEXT("PlayerUnlockComponent: Added new unlock: %s"), *NewUnlock.UnlockID.ToString());
}

TArray<FName> UPlayerUnlockComponent::GetUnlockedByType(EUnlockType Type) const
{
	TArray<FName> Result;

	for (const FUnlockEntry& Unlock : Unlocks)
	{
		if (Unlock.Type == Type && Unlock.bIsUnlocked)
		{
			Result.Add(Unlock.UnlockID);
		}
	}

	return Result;
}

TArray<FUnlockEntry> UPlayerUnlockComponent::GetUnlocksByType(EUnlockType Type) const
{
	TArray<FUnlockEntry> Result;

	for (const FUnlockEntry& Unlock : Unlocks)
	{
		if (Unlock.Type == Type)
		{
			Result.Add(Unlock);
		}
	}

	return Result;
}

bool UPlayerUnlockComponent::GetUnlockEntry(FName UnlockID, FUnlockEntry& OutUnlock) const
{
	const FUnlockEntry* Found = FindUnlock(UnlockID);
	if (Found)
	{
		OutUnlock = *Found;
		return true;
	}
	return false;
}

float UPlayerUnlockComponent::GetUnlockCompletionPercentage() const
{
	if (Unlocks.Num() == 0)
	{
		return 1.0f;
	}

	return static_cast<float>(UnlockedIDs.Num()) / static_cast<float>(Unlocks.Num());
}

TArray<FUnlockRequirement> UPlayerUnlockComponent::GetUnmetRequirements(FName UnlockID) const
{
	TArray<FUnlockRequirement> UnmetRequirements;

	const FUnlockEntry* Unlock = FindUnlock(UnlockID);
	if (!Unlock)
	{
		return UnmetRequirements;
	}

	for (const FUnlockRequirement& Requirement : Unlock->Requirements)
	{
		if (!CheckRequirement(Requirement))
		{
			UnmetRequirements.Add(Requirement);
		}
	}

	return UnmetRequirements;
}

FUnlockEntry* UPlayerUnlockComponent::FindUnlock(FName UnlockID)
{
	for (FUnlockEntry& Unlock : Unlocks)
	{
		if (Unlock.UnlockID == UnlockID)
		{
			return &Unlock;
		}
	}
	return nullptr;
}

const FUnlockEntry* UPlayerUnlockComponent::FindUnlock(FName UnlockID) const
{
	for (const FUnlockEntry& Unlock : Unlocks)
	{
		if (Unlock.UnlockID == UnlockID)
		{
			return &Unlock;
		}
	}
	return nullptr;
}

bool UPlayerUnlockComponent::CheckRequirement(const FUnlockRequirement& Requirement) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	switch (Requirement.Type)
	{
		case EUnlockRequirementType::Level:
		{
			// Check player level
			UPlayerProgressionComponent* ProgressionComp = Owner->FindComponentByClass<UPlayerProgressionComponent>();
			if (ProgressionComp)
			{
				return ProgressionComp->MeetsLevelRequirement(Requirement.RequiredValue);
			}
			return false;
		}

		case EUnlockRequirementType::Achievement:
		{
			// TODO: Check achievement completion via AchievementManagerSubsystem
			// For now, return false as achievement system isn't implemented yet
			UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Achievement requirement check not yet implemented"));
			return false;
		}

		case EUnlockRequirementType::Quest:
		{
			// TODO: Check quest completion via QuestManagerSubsystem
			// For now, return false as quest tracking integration isn't complete
			UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Quest requirement check not yet implemented"));
			return false;
		}

		case EUnlockRequirementType::Reputation:
		{
			// Check faction reputation
			UPlayerReputationComponent* ReputationComp = Owner->FindComponentByClass<UPlayerReputationComponent>();
			if (ReputationComp)
			{
				return ReputationComp->IsReputationAtLeast(Requirement.RequiredID, Requirement.RequiredValue);
			}
			return false;
		}

		case EUnlockRequirementType::Credits:
		{
			// Check player credits
			UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
			if (GameInstance)
			{
				return GameInstance->GetPlayerCredits() >= Requirement.RequiredValue;
			}
			return false;
		}

		case EUnlockRequirementType::Item:
		{
			// TODO: Check inventory for required item
			// For now, return false as inventory integration isn't complete
			UE_LOG(LogAdastrea, Warning, TEXT("PlayerUnlockComponent: Item requirement check not yet implemented"));
			return false;
		}

		default:
			return false;
	}
}
