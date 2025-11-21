#include "Exploration/DiscoveryComponent.h"
#include "Exploration/DiscoveryManagerSubsystem.h"
#include "Exploration/ScannableObjectComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDiscoveryComponent::UDiscoveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Initialize defaults
	AnomalyData = nullptr;
	DiscoveryData = nullptr;
	bIsUnique = true;
	bPersistent = true;
	bDebugShowState = false;

	// State defaults
	DiscoveryState = EDiscoveryState::Undiscovered;
	bDiscovered = false;
	DiscoveryTime = 0.0f;
	DiscoveredBy = nullptr;
	CurrentInteraction = EInteractionType::None;
	bInteractionInProgress = false;
	InteractionProgress = 0.0f;
	InteractionStartTime = 0.0f;
	InteractionCount = 0;
	InteractionCooldownRemaining = 0.0f;
}

void UDiscoveryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeDiscovery();
}

void UDiscoveryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update interaction progress
	if (bInteractionInProgress)
	{
		UpdateInteraction(DeltaTime);
	}

	// Update cooldown
	if (InteractionCooldownRemaining > 0.0f)
	{
		InteractionCooldownRemaining -= DeltaTime;
		if (InteractionCooldownRemaining < 0.0f)
		{
			InteractionCooldownRemaining = 0.0f;
		}
	}
}

void UDiscoveryComponent::InitializeDiscovery()
{
	// Validate configuration
	if (!AnomalyData && !DiscoveryData)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscoveryComponent on %s has no data assigned!"), *GetOwner()->GetName());
		return;
	}

	// Ensure ScannableObjectComponent exists
	UScannableObjectComponent* ScannableComp = GetOwner()->FindComponentByClass<UScannableObjectComponent>();
	if (!ScannableComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscoveryComponent on %s requires ScannableObjectComponent for detection!"), *GetOwner()->GetName());
	}
}

bool UDiscoveryComponent::MarkAsDiscovered(APlayerController* Discoverer)
{
	// Check if can discover
	FText Reason;
	if (!CanDiscover(Reason))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot discover %s: %s"), *GetDiscoveryName().ToString(), *Reason.ToString());
		return false;
	}

	// Check if already discovered
	if (bDiscovered)
	{
		UE_LOG(LogTemp, Log, TEXT("%s has already been discovered."), *GetDiscoveryName().ToString());
		return false;
	}

	// Mark as discovered
	bDiscovered = true;
	DiscoveryState = EDiscoveryState::Discovered;
	DiscoveryTime = GetWorld()->GetTimeSeconds();
	DiscoveredBy = Discoverer;

	// Register with manager
	RegisterDiscovery();

	// Give rewards
	GiveRewards(Discoverer);

	// Fire Blueprint event
	OnDiscovered(Discoverer);

	UE_LOG(LogTemp, Log, TEXT("Discovery made: %s"), *GetDiscoveryName().ToString());

	return true;
}

bool UDiscoveryComponent::CanDiscover(FText& OutReason) const
{
	// Must have data
	if (!AnomalyData && !DiscoveryData)
	{
		OutReason = FText::FromString("No discovery data configured.");
		return false;
	}

	// Check if unique and already discovered globally
	if (bIsUnique)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameInstance* GameInstance = World->GetGameInstance();
			if (GameInstance)
			{
				UDiscoveryManagerSubsystem* DiscoveryManager = GameInstance->GetSubsystem<UDiscoveryManagerSubsystem>();
				if (DiscoveryManager && DiscoveryManager->IsDiscovered(GetDiscoveryID()))
				{
					OutReason = FText::FromString("This discovery has already been made.");
					return false;
				}
			}
		}
	}

	OutReason = FText::GetEmpty();
	return true;
}

FText UDiscoveryComponent::GetDiscoveryName() const
{
	if (AnomalyData)
	{
		return AnomalyData->AnomalyName;
	}
	else if (DiscoveryData)
	{
		return DiscoveryData->DiscoveryName;
	}
	return FText::FromString("Unknown Discovery");
}

FText UDiscoveryComponent::GetDiscoveryDescription() const
{
	if (AnomalyData)
	{
		return AnomalyData->Description;
	}
	else if (DiscoveryData)
	{
		return DiscoveryData->Description;
	}
	return FText::GetEmpty();
}

ERarityTier UDiscoveryComponent::GetRarityTier() const
{
	if (AnomalyData)
	{
		return AnomalyData->RarityTier;
	}
	else if (DiscoveryData)
	{
		return DiscoveryData->RarityTier;
	}
	return ERarityTier::Common;
}

FName UDiscoveryComponent::GetDiscoveryID() const
{
	if (AnomalyData)
	{
		return AnomalyData->AnomalyID;
	}
	else if (DiscoveryData)
	{
		return DiscoveryData->DiscoveryID;
	}
	return NAME_None;
}

bool UDiscoveryComponent::IsAnomaly() const
{
	return AnomalyData != nullptr;
}

bool UDiscoveryComponent::IsDiscoverableLocation() const
{
	return DiscoveryData != nullptr;
}

bool UDiscoveryComponent::StartInteraction(EInteractionType InteractionType, APlayerController* Interactor)
{
	// Check if can interact
	FText Reason;
	if (!CanInteract(InteractionType, Reason))
	{
		OnInteractionFailed(Reason);
		return false;
	}

	// Start interaction
	CurrentInteraction = InteractionType;
	bInteractionInProgress = true;
	InteractionProgress = 0.0f;
	InteractionStartTime = GetWorld()->GetTimeSeconds();

	// Fire Blueprint event
	OnInteractionStarted(InteractionType, Interactor);

	UE_LOG(LogTemp, Log, TEXT("Started interaction with %s"), *GetDiscoveryName().ToString());

	return true;
}

void UDiscoveryComponent::CancelInteraction()
{
	if (!bInteractionInProgress)
	{
		return;
	}

	bInteractionInProgress = false;
	InteractionProgress = 0.0f;
	CurrentInteraction = EInteractionType::None;

	OnInteractionFailed(FText::FromString("Interaction cancelled."));

	UE_LOG(LogTemp, Log, TEXT("Interaction cancelled with %s"), *GetDiscoveryName().ToString());
}

bool UDiscoveryComponent::CanInteract(EInteractionType InteractionType, FText& OutReason) const
{
	// Must be discovered first
	if (!bDiscovered)
	{
		OutReason = FText::FromString("Must discover this object first.");
		return false;
	}

	// Check if on cooldown
	if (IsOnCooldown())
	{
		OutReason = FText::Format(
			FText::FromString("Interaction on cooldown. Wait {0} seconds."),
			FText::AsNumber(FMath::CeilToInt(InteractionCooldownRemaining))
		);
		return false;
	}

	// Check if already interacting
	if (bInteractionInProgress)
	{
		OutReason = FText::FromString("Already interacting.");
		return false;
	}

	// Check if depleted
	if (DiscoveryState == EDiscoveryState::Depleted)
	{
		OutReason = FText::FromString("This discovery has been depleted.");
		return false;
	}

	// Type-specific checks
	switch (InteractionType)
	{
		case EInteractionType::Harvest:
			if (AnomalyData && !AnomalyData->bCanBeHarvested)
			{
				OutReason = FText::FromString("This anomaly cannot be harvested.");
				return false;
			}
			break;

		case EInteractionType::Explore:
			if (DiscoveryData && !DiscoveryData->bCanBeExplored)
			{
				OutReason = FText::FromString("This location cannot be explored.");
				return false;
			}
			break;

		case EInteractionType::Salvage:
			if (DiscoveryData && !DiscoveryData->bCanBeSalvaged)
			{
				OutReason = FText::FromString("This location cannot be salvaged.");
				return false;
			}
			break;

		default:
			break;
	}

	OutReason = FText::GetEmpty();
	return true;
}

float UDiscoveryComponent::GetInteractionProgress() const
{
	return InteractionProgress;
}

float UDiscoveryComponent::GetInteractionTimeRemaining() const
{
	if (!bInteractionInProgress)
	{
		return 0.0f;
	}

	float Duration = GetInteractionDuration(CurrentInteraction);
	float Elapsed = GetWorld()->GetTimeSeconds() - InteractionStartTime;
	return FMath::Max(0.0f, Duration - Elapsed);
}

bool UDiscoveryComponent::GiveRewards(APlayerController* Player)
{
	if (!Player)
	{
		return false;
	}

	int32 Credits = 0;
	int32 Experience = 0;

	if (AnomalyData)
	{
		Credits = AnomalyData->CreditsReward;
		Experience = AnomalyData->ExperienceReward;
	}
	else if (DiscoveryData)
	{
		Credits = DiscoveryData->CreditsReward;
		Experience = DiscoveryData->ExperienceReward;
	}

	// TODO: Implement actual reward system integration
	// For now, just fire the event
	OnRewardsGiven(Player, Credits, Experience);

	UE_LOG(LogTemp, Log, TEXT("Gave rewards for %s: %d credits, %d XP"), *GetDiscoveryName().ToString(), Credits, Experience);

	return true;
}

int32 UDiscoveryComponent::GetTotalRewardValue() const
{
	if (AnomalyData)
	{
		return AnomalyData->GetTotalRewardValue(true);
	}
	else if (DiscoveryData)
	{
		return DiscoveryData->GetTotalRewardValue(true, true);
	}
	return 0;
}

void UDiscoveryComponent::UpdateInteraction(float DeltaTime)
{
	if (!bInteractionInProgress)
	{
		return;
	}

	float Duration = GetInteractionDuration(CurrentInteraction);
	float Elapsed = GetWorld()->GetTimeSeconds() - InteractionStartTime;
	
	InteractionProgress = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);

	// Check if complete
	if (InteractionProgress >= 1.0f)
	{
		CompleteInteraction();
	}
}

void UDiscoveryComponent::CompleteInteraction()
{
	if (!bInteractionInProgress)
	{
		return;
	}

	EInteractionType CompletedType = CurrentInteraction;

	// Update state
	bInteractionInProgress = false;
	InteractionProgress = 1.0f;
	CurrentInteraction = EInteractionType::None;
	InteractionCount++;

	// Update discovery state
	if (DiscoveryState == EDiscoveryState::Discovered)
	{
		DiscoveryState = EDiscoveryState::Explored;
	}

	// Set cooldown if not reusable
	bool bIsReusable = false;
	float Cooldown = 0.0f;

	if (AnomalyData)
	{
		bIsReusable = AnomalyData->bIsReusable;
		Cooldown = AnomalyData->InteractionCooldown;
	}
	else if (DiscoveryData)
	{
		bIsReusable = DiscoveryData->bCanRevisit;
		Cooldown = 300.0f; // Default 5 minute cooldown
	}

	if (!bIsReusable)
	{
		DiscoveryState = EDiscoveryState::Depleted;
	}
	else
	{
		InteractionCooldownRemaining = Cooldown;
	}

	// Fire Blueprint event
	OnInteractionCompleted(CompletedType);

	UE_LOG(LogTemp, Log, TEXT("Completed interaction with %s"), *GetDiscoveryName().ToString());
}

void UDiscoveryComponent::RegisterDiscovery()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UDiscoveryManagerSubsystem* DiscoveryManager = GameInstance->GetSubsystem<UDiscoveryManagerSubsystem>();
	if (!DiscoveryManager)
	{
		return;
	}

	FVector Location = GetOwner()->GetActorLocation();
	int32 RewardValue = GetTotalRewardValue();

	DiscoveryManager->RegisterDiscovery(
		GetDiscoveryID(),
		AnomalyData,
		DiscoveryData,
		DiscoveredBy,
		Location,
		RewardValue
	);
}

float UDiscoveryComponent::GetInteractionDuration(EInteractionType InteractionType) const
{
	switch (InteractionType)
	{
		case EInteractionType::Harvest:
			if (AnomalyData)
			{
				return AnomalyData->HarvestTime;
			}
			break;

		case EInteractionType::Explore:
			if (DiscoveryData)
			{
				return DiscoveryData->ExplorationTime;
			}
			break;

		case EInteractionType::Salvage:
			return 120.0f; // 2 minutes default

		case EInteractionType::Research:
			return 60.0f; // 1 minute default

		case EInteractionType::Scan:
			return 30.0f; // 30 seconds default

		default:
			break;
	}

	return 60.0f; // Default 1 minute
}

bool UDiscoveryComponent::IsOnCooldown() const
{
	return InteractionCooldownRemaining > 0.0f;
}
