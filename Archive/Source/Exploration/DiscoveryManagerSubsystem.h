#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Exploration/AnomalyDataAsset.h"
#include "Exploration/DiscoveryDataAsset.h"
#include "DiscoveryManagerSubsystem.generated.h"

/**
 * Discovery Record
 * Information about a discovered object
 */
USTRUCT(BlueprintType)
struct FDiscoveryRecord
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	FName DiscoveryID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	FText DiscoveryName;

	/** Type: true = Anomaly, false = Location */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	bool bIsAnomaly;

	/** Anomaly data (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	UAnomalyDataAsset* AnomalyData;

	/** Discovery data (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	UDiscoveryDataAsset* DiscoveryData;

	/** Rarity tier */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	ERarityTier RarityTier;

	/** When discovered */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	FDateTime DiscoveryTime;

	/** Who discovered it */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	FString DiscovererName;

	/** World location where discovered */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	FVector WorldLocation;

	/** Total reward value received */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	int32 RewardValue;

	/** Has been fully explored/interacted with */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	bool bFullyExplored;

	/** Number of interactions */
	UPROPERTY(BlueprintReadOnly, Category="Discovery")
	int32 InteractionCount;

	FDiscoveryRecord()
		: DiscoveryID(NAME_None)
		, bIsAnomaly(false)
		, AnomalyData(nullptr)
		, DiscoveryData(nullptr)
		, RarityTier(ERarityTier::Common)
		, DiscoveryTime(FDateTime::Now())
		, WorldLocation(FVector::ZeroVector)
		, RewardValue(0)
		, bFullyExplored(false)
		, InteractionCount(0)
	{
	}
};

/**
 * Discovery Statistics
 * Player's exploration statistics
 */
USTRUCT(BlueprintType)
struct FDiscoveryStatistics
{
	GENERATED_BODY()

	/** Total discoveries made */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 TotalDiscoveries;

	/** Anomalies discovered */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 AnomaliesDiscovered;

	/** Locations discovered */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 LocationsDiscovered;

	/** Common tier discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 CommonDiscoveries;

	/** Uncommon tier discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 UncommonDiscoveries;

	/** Rare tier discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 RareDiscoveries;

	/** Very Rare tier discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 VeryRareDiscoveries;

	/** Legendary tier discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 LegendaryDiscoveries;

	/** Total reward value earned */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 TotalRewardValue;

	/** Total credits earned from discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 TotalCreditsEarned;

	/** Total experience earned from discoveries */
	UPROPERTY(BlueprintReadOnly, Category="Statistics")
	int32 TotalExperienceEarned;

	FDiscoveryStatistics()
		: TotalDiscoveries(0)
		, AnomaliesDiscovered(0)
		, LocationsDiscovered(0)
		, CommonDiscoveries(0)
		, UncommonDiscoveries(0)
		, RareDiscoveries(0)
		, VeryRareDiscoveries(0)
		, LegendaryDiscoveries(0)
		, TotalRewardValue(0)
		, TotalCreditsEarned(0)
		, TotalExperienceEarned(0)
	{
	}
};

/**
 * Discovery Manager Subsystem
 * 
 * Game-wide manager for tracking all player discoveries including:
 * - Anomalies and spatial phenomena
 * - Archaeological sites and derelict vessels
 * - Discovery statistics and achievements
 * - Persistence across save/load
 * 
 * **Responsibilities:**
 * - Register new discoveries
 * - Track discovery state (discovered, explored, depleted)
 * - Provide discovery queries (by type, rarity, location)
 * - Calculate discovery statistics
 * - Fire events for UI updates and achievements
 * 
 * **Usage:**
 * - Access via UGameInstance::GetSubsystem<UDiscoveryManagerSubsystem>()
 * - DiscoveryComponent automatically registers discoveries
 * - Use Blueprint functions to query discovery state
 * - Subscribe to events for real-time updates
 * 
 * **Integration:**
 * - Works with DiscoveryComponent for registration
 * - Integrates with save system for persistence
 * - Provides data for UI and achievement systems
 * - Can trigger quests based on discovery patterns
 */
UCLASS()
class ADASTREA_API UDiscoveryManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDiscoveryManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// DISCOVERY REGISTRATION
	// ====================

	/**
	 * Register a new discovery
	 * @param DiscoveryID Unique identifier
	 * @param AnomalyData Anomaly data (if anomaly)
	 * @param DiscoveryData Discovery data (if location)
	 * @param Discoverer Player who discovered
	 * @param WorldLocation Location in world
	 * @param RewardValue Total reward value
	 * @return True if registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool RegisterDiscovery(
		FName DiscoveryID,
		UAnomalyDataAsset* AnomalyData,
		UDiscoveryDataAsset* DiscoveryData,
		APlayerController* Discoverer,
		FVector WorldLocation,
		int32 RewardValue
	);

	/**
	 * Register anomaly discovery
	 * @param AnomalyData Anomaly data asset
	 * @param Discoverer Player who discovered
	 * @param WorldLocation Location in world
	 * @param RewardValue Total reward value
	 * @return True if registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool RegisterAnomaly(
		UAnomalyDataAsset* AnomalyData,
		APlayerController* Discoverer,
		FVector WorldLocation,
		int32 RewardValue
	);

	/**
	 * Register location discovery
	 * @param DiscoveryData Discovery data asset
	 * @param Discoverer Player who discovered
	 * @param WorldLocation Location in world
	 * @param RewardValue Total reward value
	 * @return True if registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool RegisterLocation(
		UDiscoveryDataAsset* DiscoveryData,
		APlayerController* Discoverer,
		FVector WorldLocation,
		int32 RewardValue
	);

	/**
	 * Mark discovery as fully explored
	 * @param DiscoveryID Discovery to mark
	 * @return True if updated successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool MarkAsExplored(FName DiscoveryID);

	/**
	 * Increment interaction count for discovery
	 * @param DiscoveryID Discovery to update
	 * @return True if updated successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool IncrementInteractionCount(FName DiscoveryID);

	// ====================
	// DISCOVERY QUERIES
	// ====================

	/**
	 * Check if discovery has been made
	 * @param DiscoveryID Discovery to check
	 * @return True if already discovered
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	bool IsDiscovered(FName DiscoveryID) const;

	/**
	 * Get discovery record by ID
	 * @param DiscoveryID Discovery to find
	 * @param OutRecord Record data if found
	 * @return True if found
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	bool GetDiscoveryRecord(FName DiscoveryID, FDiscoveryRecord& OutRecord) const;

	/**
	 * Get all discoveries
	 * @return Array of all discovery records
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetAllDiscoveries() const;

	/**
	 * Get discoveries by rarity
	 * @param RarityTier Rarity to filter by
	 * @return Array of discoveries with that rarity
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetDiscoveriesByRarity(ERarityTier RarityTier) const;

	/**
	 * Get all anomalies
	 * @return Array of anomaly records
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetAllAnomalies() const;

	/**
	 * Get all discoverable locations
	 * @return Array of location records
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetAllLocations() const;

	/**
	 * Get discoveries by type (anomaly type or discovery type)
	 * @param TypeName Name of the type to filter by
	 * @return Array of matching discoveries
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetDiscoveriesByType(FName TypeName) const;

	/**
	 * Get fully explored discoveries
	 * @return Array of explored discoveries
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetExploredDiscoveries() const;

	/**
	 * Get discoveries near a location
	 * @param Location Center location
	 * @param Radius Search radius in cm
	 * @return Array of discoveries within radius
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	TArray<FDiscoveryRecord> GetDiscoveriesNearLocation(FVector Location, float Radius) const;

	// ====================
	// STATISTICS
	// ====================

	/**
	 * Get discovery statistics
	 * @return Current discovery statistics
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	FDiscoveryStatistics GetStatistics() const;

	/**
	 * Get total discovery count
	 * @return Number of discoveries made
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	int32 GetTotalDiscoveryCount() const;

	/**
	 * Get anomaly count
	 * @return Number of anomalies discovered
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	int32 GetAnomalyCount() const;

	/**
	 * Get location count
	 * @return Number of locations discovered
	 */
	UFUNCTION(BlueprintPure, Category="Discovery Manager")
	int32 GetLocationCount() const;

	/**
	 * Get rarity count
	 * @param RarityTier Rarity to count
	 * @return Number of discoveries of that rarity
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	int32 GetRarityCount(ERarityTier RarityTier) const;

	/**
	 * Get discovery completion percentage
	 * @param TotalAvailable Total discoveries available in game
	 * @return Percentage (0-100)
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery Manager")
	float GetCompletionPercentage(int32 TotalAvailable) const;

	// ====================
	// EVENTS
	// ====================

	/** Called when new discovery is registered */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscoveryRegistered, const FDiscoveryRecord&, DiscoveryRecord);
	UPROPERTY(BlueprintAssignable, Category="Discovery Manager|Events")
	FOnDiscoveryRegistered OnDiscoveryRegistered;

	/** Called when discovery is marked as explored */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscoveryExplored, FName, DiscoveryID);
	UPROPERTY(BlueprintAssignable, Category="Discovery Manager|Events")
	FOnDiscoveryExplored OnDiscoveryExplored;

	/** Called when rare discovery is made */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRareDiscovery, const FDiscoveryRecord&, DiscoveryRecord, ERarityTier, RarityTier);
	UPROPERTY(BlueprintAssignable, Category="Discovery Manager|Events")
	FOnRareDiscovery OnRareDiscovery;

	/** Called when milestone is reached (10, 25, 50, 100 discoveries) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscoveryMilestone, int32, TotalDiscoveries);
	UPROPERTY(BlueprintAssignable, Category="Discovery Manager|Events")
	FOnDiscoveryMilestone OnDiscoveryMilestone;

protected:
	/** Map of all discoveries (ID -> Record) */
	UPROPERTY()
	TMap<FName, FDiscoveryRecord> DiscoveryRecords;

	/** Cached statistics */
	UPROPERTY()
	mutable FDiscoveryStatistics CachedStatistics;

	/** Should statistics be recalculated? */
	mutable bool bStatisticsDirty;

private:
	/**
	 * Update cached statistics
	 */
	void UpdateStatistics() const;

	/**
	 * Check and fire milestone events
	 */
	void CheckMilestones(int32 OldCount, int32 NewCount);

	/**
	 * Get player name from controller
	 */
	FString GetPlayerName(APlayerController* Player) const;
};
