#include "Exploration/DiscoveryManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UDiscoveryManagerSubsystem::UDiscoveryManagerSubsystem()
{
	bStatisticsDirty = true;
}

void UDiscoveryManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DiscoveryRecords.Empty();
	CachedStatistics = FDiscoveryStatistics();
	bStatisticsDirty = true;

	UE_LOG(LogTemp, Log, TEXT("DiscoveryManagerSubsystem initialized."));
}

void UDiscoveryManagerSubsystem::Deinitialize()
{
	DiscoveryRecords.Empty();

	Super::Deinitialize();
}

bool UDiscoveryManagerSubsystem::RegisterDiscovery(
	FName DiscoveryID,
	UAnomalyDataAsset* AnomalyData,
	UDiscoveryDataAsset* DiscoveryData,
	APlayerController* Discoverer,
	FVector WorldLocation,
	int32 RewardValue)
{
	// Validate input
	if (DiscoveryID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register discovery with invalid ID."));
		return false;
	}

	if (!AnomalyData && !DiscoveryData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register discovery without data."));
		return false;
	}

	// Check if already registered
	if (DiscoveryRecords.Contains(DiscoveryID))
	{
		UE_LOG(LogTemp, Log, TEXT("Discovery %s already registered."), *DiscoveryID.ToString());
		return false;
	}

	// Create discovery record
	FDiscoveryRecord Record;
	Record.DiscoveryID = DiscoveryID;
	Record.bIsAnomaly = (AnomalyData != nullptr);
	Record.AnomalyData = AnomalyData;
	Record.DiscoveryData = DiscoveryData;
	Record.DiscoveryTime = FDateTime::Now();
	Record.DiscovererName = GetPlayerName(Discoverer);
	Record.WorldLocation = WorldLocation;
	Record.RewardValue = RewardValue;
	Record.bFullyExplored = false;
	Record.InteractionCount = 0;

	// Set name and rarity
	if (AnomalyData)
	{
		Record.DiscoveryName = AnomalyData->AnomalyName;
		Record.RarityTier = AnomalyData->RarityTier;
	}
	else if (DiscoveryData)
	{
		Record.DiscoveryName = DiscoveryData->DiscoveryName;
		Record.RarityTier = DiscoveryData->RarityTier;
	}

	// Add to map
	int32 OldCount = DiscoveryRecords.Num();
	DiscoveryRecords.Add(DiscoveryID, Record);
	bStatisticsDirty = true;

	// Fire events
	OnDiscoveryRegistered.Broadcast(Record);

	// Check for rare discovery
	if (Record.RarityTier >= ERarityTier::Rare)
	{
		OnRareDiscovery.Broadcast(Record, Record.RarityTier);
	}

	// Check for milestones
	int32 NewCount = DiscoveryRecords.Num();
	CheckMilestones(OldCount, NewCount);

	UE_LOG(LogTemp, Log, TEXT("Registered discovery: %s (%s)"), 
		*Record.DiscoveryName.ToString(),
		*DiscoveryID.ToString());

	return true;
}

bool UDiscoveryManagerSubsystem::RegisterAnomaly(
	UAnomalyDataAsset* AnomalyData,
	APlayerController* Discoverer,
	FVector WorldLocation,
	int32 RewardValue)
{
	if (!AnomalyData)
	{
		return false;
	}

	return RegisterDiscovery(
		AnomalyData->AnomalyID,
		AnomalyData,
		nullptr,
		Discoverer,
		WorldLocation,
		RewardValue
	);
}

bool UDiscoveryManagerSubsystem::RegisterLocation(
	UDiscoveryDataAsset* DiscoveryData,
	APlayerController* Discoverer,
	FVector WorldLocation,
	int32 RewardValue)
{
	if (!DiscoveryData)
	{
		return false;
	}

	return RegisterDiscovery(
		DiscoveryData->DiscoveryID,
		nullptr,
		DiscoveryData,
		Discoverer,
		WorldLocation,
		RewardValue
	);
}

bool UDiscoveryManagerSubsystem::MarkAsExplored(FName DiscoveryID)
{
	FDiscoveryRecord* Record = DiscoveryRecords.Find(DiscoveryID);
	if (!Record)
	{
		return false;
	}

	if (Record->bFullyExplored)
	{
		return false; // Already explored
	}

	Record->bFullyExplored = true;
	bStatisticsDirty = true;

	OnDiscoveryExplored.Broadcast(DiscoveryID);

	UE_LOG(LogTemp, Log, TEXT("Discovery marked as explored: %s"), *DiscoveryID.ToString());

	return true;
}

bool UDiscoveryManagerSubsystem::IncrementInteractionCount(FName DiscoveryID)
{
	FDiscoveryRecord* Record = DiscoveryRecords.Find(DiscoveryID);
	if (!Record)
	{
		return false;
	}

	Record->InteractionCount++;
	bStatisticsDirty = true;

	return true;
}

bool UDiscoveryManagerSubsystem::IsDiscovered(FName DiscoveryID) const
{
	return DiscoveryRecords.Contains(DiscoveryID);
}

bool UDiscoveryManagerSubsystem::GetDiscoveryRecord(FName DiscoveryID, FDiscoveryRecord& OutRecord) const
{
	const FDiscoveryRecord* Record = DiscoveryRecords.Find(DiscoveryID);
	if (Record)
	{
		OutRecord = *Record;
		return true;
	}
	return false;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetAllDiscoveries() const
{
	TArray<FDiscoveryRecord> AllDiscoveries;
	DiscoveryRecords.GenerateValueArray(AllDiscoveries);
	return AllDiscoveries;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetDiscoveriesByRarity(ERarityTier RarityTier) const
{
	TArray<FDiscoveryRecord> FilteredDiscoveries;

	for (const auto& Pair : DiscoveryRecords)
	{
		if (Pair.Value.RarityTier == RarityTier)
		{
			FilteredDiscoveries.Add(Pair.Value);
		}
	}

	return FilteredDiscoveries;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetAllAnomalies() const
{
	TArray<FDiscoveryRecord> Anomalies;

	for (const auto& Pair : DiscoveryRecords)
	{
		if (Pair.Value.bIsAnomaly)
		{
			Anomalies.Add(Pair.Value);
		}
	}

	return Anomalies;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetAllLocations() const
{
	TArray<FDiscoveryRecord> Locations;

	for (const auto& Pair : DiscoveryRecords)
	{
		if (!Pair.Value.bIsAnomaly)
		{
			Locations.Add(Pair.Value);
		}
	}

	return Locations;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetDiscoveriesByType(FName TypeName) const
{
	TArray<FDiscoveryRecord> FilteredDiscoveries;

	// Filter discoveries by type name
	// TypeName should match the discovery type enum value (e.g., "ArchaeologicalSite", "DerelictShip")
	for (const TPair<FName, FDiscoveryRecord>& Pair : DiscoveryRecords)
	{
		const FDiscoveryRecord& Record = Pair.Value;
		
		// Check if the type matches (compare TypeName with the discovery's type)
		// This allows Blueprint to filter by type string
		if (Record.DiscoveryType == TypeName || TypeName == NAME_None)
		{
			FilteredDiscoveries.Add(Record);
		}
	}

	return FilteredDiscoveries;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetExploredDiscoveries() const
{
	TArray<FDiscoveryRecord> ExploredDiscoveries;

	for (const auto& Pair : DiscoveryRecords)
	{
		if (Pair.Value.bFullyExplored)
		{
			ExploredDiscoveries.Add(Pair.Value);
		}
	}

	return ExploredDiscoveries;
}

TArray<FDiscoveryRecord> UDiscoveryManagerSubsystem::GetDiscoveriesNearLocation(FVector Location, float Radius) const
{
	TArray<FDiscoveryRecord> NearbyDiscoveries;
	float RadiusSquared = Radius * Radius;

	for (const auto& Pair : DiscoveryRecords)
	{
		float DistanceSquared = FVector::DistSquared(Pair.Value.WorldLocation, Location);
		if (DistanceSquared <= RadiusSquared)
		{
			NearbyDiscoveries.Add(Pair.Value);
		}
	}

	return NearbyDiscoveries;
}

FDiscoveryStatistics UDiscoveryManagerSubsystem::GetStatistics() const
{
	if (bStatisticsDirty)
	{
		const_cast<UDiscoveryManagerSubsystem*>(this)->UpdateStatistics();
	}

	return CachedStatistics;
}

int32 UDiscoveryManagerSubsystem::GetTotalDiscoveryCount() const
{
	return DiscoveryRecords.Num();
}

int32 UDiscoveryManagerSubsystem::GetAnomalyCount() const
{
	int32 Count = 0;
	for (const auto& Pair : DiscoveryRecords)
	{
		if (Pair.Value.bIsAnomaly)
		{
			Count++;
		}
	}
	return Count;
}

int32 UDiscoveryManagerSubsystem::GetLocationCount() const
{
	int32 Count = 0;
	for (const auto& Pair : DiscoveryRecords)
	{
		if (!Pair.Value.bIsAnomaly)
		{
			Count++;
		}
	}
	return Count;
}

int32 UDiscoveryManagerSubsystem::GetRarityCount(ERarityTier RarityTier) const
{
	int32 Count = 0;
	for (const auto& Pair : DiscoveryRecords)
	{
		if (Pair.Value.RarityTier == RarityTier)
		{
			Count++;
		}
	}
	return Count;
}

float UDiscoveryManagerSubsystem::GetCompletionPercentage(int32 TotalAvailable) const
{
	if (TotalAvailable <= 0)
	{
		return 0.0f;
	}

	return (static_cast<float>(DiscoveryRecords.Num()) / static_cast<float>(TotalAvailable)) * 100.0f;
}

void UDiscoveryManagerSubsystem::UpdateStatistics()
{
	CachedStatistics = FDiscoveryStatistics();

	for (const auto& Pair : DiscoveryRecords)
	{
		const FDiscoveryRecord& Record = Pair.Value;

		// Total count
		CachedStatistics.TotalDiscoveries++;

		// Type counts
		if (Record.bIsAnomaly)
		{
			CachedStatistics.AnomaliesDiscovered++;
		}
		else
		{
			CachedStatistics.LocationsDiscovered++;
		}

		// Rarity counts
		switch (Record.RarityTier)
		{
			case ERarityTier::Common:
				CachedStatistics.CommonDiscoveries++;
				break;
			case ERarityTier::Uncommon:
				CachedStatistics.UncommonDiscoveries++;
				break;
			case ERarityTier::Rare:
				CachedStatistics.RareDiscoveries++;
				break;
			case ERarityTier::VeryRare:
				CachedStatistics.VeryRareDiscoveries++;
				break;
			case ERarityTier::Legendary:
				CachedStatistics.LegendaryDiscoveries++;
				break;
		}

		// Reward totals
		CachedStatistics.TotalRewardValue += Record.RewardValue;

		// Calculate credits and experience (approximate from data assets)
		if (Record.AnomalyData)
		{
			CachedStatistics.TotalCreditsEarned += Record.AnomalyData->CreditsReward;
			CachedStatistics.TotalExperienceEarned += Record.AnomalyData->ExperienceReward;
		}
		else if (Record.DiscoveryData)
		{
			CachedStatistics.TotalCreditsEarned += Record.DiscoveryData->CreditsReward;
			CachedStatistics.TotalExperienceEarned += Record.DiscoveryData->ExperienceReward;
		}
	}

	bStatisticsDirty = false;
}

void UDiscoveryManagerSubsystem::CheckMilestones(int32 OldCount, int32 NewCount)
{
	// Milestone thresholds
	const TArray<int32> Milestones = { 1, 10, 25, 50, 100, 250, 500, 1000 };

	for (int32 Milestone : Milestones)
	{
		if (OldCount < Milestone && NewCount >= Milestone)
		{
			OnDiscoveryMilestone.Broadcast(Milestone);
			UE_LOG(LogTemp, Log, TEXT("Discovery milestone reached: %d discoveries!"), Milestone);
		}
	}
}

FString UDiscoveryManagerSubsystem::GetPlayerName(APlayerController* Player) const
{
	if (!Player)
	{
		return TEXT("Unknown");
	}

	APlayerState* PlayerState = Player->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			return PlayerName;
		}
	}

	return TEXT("Player");
}
