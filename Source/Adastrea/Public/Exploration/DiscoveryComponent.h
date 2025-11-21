#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Exploration/AnomalyDataAsset.h"
#include "Exploration/DiscoveryDataAsset.h"
#include "DiscoveryComponent.generated.h"

/**
 * Discovery State
 * Current state of a discoverable object
 */
UENUM(BlueprintType)
enum class EDiscoveryState : uint8
{
	Undiscovered		UMETA(DisplayName = "Undiscovered"),		// Not yet found
	Detected			UMETA(DisplayName = "Detected"),			// Detected but not fully scanned
	Discovered			UMETA(DisplayName = "Discovered"),			// Fully discovered
	Explored			UMETA(DisplayName = "Explored"),			// Explored/interacted with
	Depleted			UMETA(DisplayName = "Depleted")				// Resources exhausted
};

/**
 * Interaction Type
 * Type of interaction with a discovery
 */
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None				UMETA(DisplayName = "None"),
	Scan				UMETA(DisplayName = "Scan"),
	Harvest				UMETA(DisplayName = "Harvest"),
	Explore				UMETA(DisplayName = "Explore"),
	Salvage				UMETA(DisplayName = "Salvage"),
	Research			UMETA(DisplayName = "Research")
};

/**
 * Discovery Component
 * 
 * Makes an object discoverable through the exploration system. Attach this component
 * to actors that represent:
 * - Anomalies and spatial phenomena
 * - Archaeological sites
 * - Derelict ships and stations
 * - Hidden caches and artifacts
 * 
 * **Discovery Flow:**
 * 1. Object detected by ScannerComponent (via ScannableObjectComponent)
 * 2. Player performs scan at required detail level
 * 3. DiscoveryComponent marks object as discovered
 * 4. Discovery registered with DiscoveryManagerSubsystem
 * 5. Rewards granted to player
 * 6. Events fired for UI updates, quests, etc.
 * 
 * **State Tracking:**
 * - Undiscovered: Not yet found
 * - Detected: Scanners see it but not analyzed
 * - Discovered: Fully scanned and catalogued
 * - Explored: Player has interacted/entered
 * - Depleted: Resources exhausted (if harvestable)
 * 
 * **Usage:**
 * 1. Add component to actor Blueprint
 * 2. Assign either AnomalyData or DiscoveryData
 * 3. Component automatically integrates with Scanner and Discovery Manager
 * 4. Override Blueprint events for custom behavior
 * 
 * **Integration:**
 * - Works alongside ScannableObjectComponent for detection
 * - Registers discoveries with DiscoveryManagerSubsystem
 * - Triggers quest objectives when appropriate
 * - Grants rewards through player systems
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UDiscoveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDiscoveryComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================
	// CONFIGURATION
	// ====================

	/** Anomaly data (if this is an anomaly) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discovery")
	UAnomalyDataAsset* AnomalyData;

	/** Discovery data (if this is a discoverable location) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discovery")
	UDiscoveryDataAsset* DiscoveryData;

	/** Is this discovery unique (can only be found once)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discovery")
	bool bIsUnique;

	/** Should this discovery persist across save/load? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discovery")
	bool bPersistent;

	/** Debug: Show discovery state in world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discovery|Debug")
	bool bDebugShowState;

	// ====================
	// STATE
	// ====================

	/** Current state of this discovery */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	EDiscoveryState DiscoveryState;

	/** Has this been discovered? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	bool bDiscovered;

	/** Time when discovered (game time) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	float DiscoveryTime;

	/** Player who discovered this */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	APlayerController* DiscoveredBy;

	/** Current interaction in progress */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	EInteractionType CurrentInteraction;

	/** Is interaction currently in progress? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	bool bInteractionInProgress;

	/** Interaction progress (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	float InteractionProgress;

	/** Interaction start time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	float InteractionStartTime;

	/** Number of times this has been interacted with */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	int32 InteractionCount;

	/** Time until next interaction is allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Discovery State")
	float InteractionCooldownRemaining;

	// ====================
	// DISCOVERY FUNCTIONS
	// ====================

	/**
	 * Mark this object as discovered
	 * @param Discoverer Player who discovered this
	 * @return True if discovery was successful
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery")
	bool MarkAsDiscovered(APlayerController* Discoverer);

	/**
	 * Check if this discovery can be made
	 * @param OutReason Reason if cannot discover
	 * @return True if can be discovered
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	bool CanDiscover(FText& OutReason) const;

	/**
	 * Get the discovery name
	 * @return Display name of this discovery
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FText GetDiscoveryName() const;

	/**
	 * Get the discovery description
	 * @return Description text
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FText GetDiscoveryDescription() const;

	/**
	 * Get the rarity tier
	 * @return Rarity of this discovery
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	ERarityTier GetRarityTier() const;

	/**
	 * Get discovery unique ID
	 * @return FName identifier for this discovery
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	FName GetDiscoveryID() const;

	/**
	 * Is this an anomaly?
	 * @return True if this uses AnomalyData
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	bool IsAnomaly() const;

	/**
	 * Is this a discoverable location?
	 * @return True if this uses DiscoveryData
	 */
	UFUNCTION(BlueprintPure, Category="Discovery")
	bool IsDiscoverableLocation() const;

	// ====================
	// INTERACTION FUNCTIONS
	// ====================

	/**
	 * Start an interaction with this discovery
	 * @param InteractionType Type of interaction to perform
	 * @param Interactor Player controller initiating interaction
	 * @return True if interaction started successfully
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery|Interaction")
	bool StartInteraction(EInteractionType InteractionType, APlayerController* Interactor);

	/**
	 * Cancel current interaction
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery|Interaction")
	void CancelInteraction();

	/**
	 * Check if can interact with this discovery
	 * @param InteractionType Type of interaction to check
	 * @param OutReason Reason if cannot interact
	 * @return True if can interact
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery|Interaction")
	bool CanInteract(EInteractionType InteractionType, FText& OutReason) const;

	/**
	 * Get interaction progress (0-1)
	 * @return Progress of current interaction
	 */
	UFUNCTION(BlueprintPure, Category="Discovery|Interaction")
	float GetInteractionProgress() const;

	/**
	 * Get time remaining for current interaction
	 * @return Seconds remaining
	 */
	UFUNCTION(BlueprintPure, Category="Discovery|Interaction")
	float GetInteractionTimeRemaining() const;

	// ====================
	// REWARD FUNCTIONS
	// ====================

	/**
	 * Give discovery rewards to player
	 * @param Player Player to give rewards to
	 * @return True if rewards were given
	 */
	UFUNCTION(BlueprintCallable, Category="Discovery|Rewards")
	bool GiveRewards(APlayerController* Player);

	/**
	 * Get total reward value
	 * @return Estimated credit value of all rewards
	 */
	UFUNCTION(BlueprintPure, Category="Discovery|Rewards")
	int32 GetTotalRewardValue() const;

	// ====================
	// BLUEPRINT EVENTS
	// ====================

	/**
	 * Called when this discovery is made
	 * @param Discoverer Player who discovered this
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Discovery")
	void OnDiscovered(APlayerController* Discoverer);

	/**
	 * Called when interaction starts
	 * @param InteractionType Type of interaction
	 * @param Interactor Player starting interaction
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Discovery")
	void OnInteractionStarted(EInteractionType InteractionType, APlayerController* Interactor);

	/**
	 * Called when interaction completes successfully
	 * @param InteractionType Type of interaction completed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Discovery")
	void OnInteractionCompleted(EInteractionType InteractionType);

	/**
	 * Called when interaction is cancelled or fails
	 * @param Reason Why interaction failed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Discovery")
	void OnInteractionFailed(const FText& Reason);

	/**
	 * Called when rewards are given
	 * @param Player Player receiving rewards
	 * @param Credits Credit amount
	 * @param Experience Experience amount
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Discovery")
	void OnRewardsGiven(APlayerController* Player, int32 Credits, int32 Experience);

protected:
	/**
	 * Update interaction progress
	 */
	void UpdateInteraction(float DeltaTime);

	/**
	 * Complete the current interaction
	 */
	void CompleteInteraction();

	/**
	 * Register this discovery with the manager
	 */
	void RegisterDiscovery();

	/**
	 * Initialize discovery component
	 */
	void InitializeDiscovery();

	/**
	 * Get interaction duration for type
	 */
	float GetInteractionDuration(EInteractionType InteractionType) const;

	/**
	 * Check if discovery is on cooldown
	 */
	bool IsOnCooldown() const;
};
