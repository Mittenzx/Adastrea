#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Antagonist.h"
#include "AntagonistManager.generated.h"

// Forward declarations
class UFeatDataAsset;

/**
 * World subsystem for managing antagonist spawning and lifecycle.
 * 
 * The Antagonist Manager is responsible for:
 * - Listening to OnFeatCompleted events from the player
 * - Evaluating Feat triggers to determine if a new antagonist should spawn
 * - Creating and initializing new FActiveAntagonist instances
 * - Managing the lifecycle of active antagonists (heat decay, encounters)
 * - Persisting antagonist state across gameplay sessions
 * - Providing query functions for active antagonists
 * 
 * Design Philosophy:
 * - Reactive: Antagonists emerge naturally from player achievements
 * - Personal: Each antagonist has unique motivation tied to specific Feats
 * - Escalating: Heat levels create dynamic tension and pacing
 * - Persistent: Rivals remain consistent across saves and sessions
 * 
 * Integration Points:
 * - Verse System: Listens for OnFeatCompleted broadcasts
 * - AI System: Provides antagonist data for behavior systems
 * - Quest System: Triggers rival-specific encounters
 * - Save System: Persists antagonist roster
 * 
 * Usage:
 * - Automatically created as a world subsystem
 * - Access via UWorld::GetSubsystem<UAntagonistManager>()
 * - Feat system broadcasts completion events
 * - AI queries for active antagonists
 * 
 * Example Flow:
 * 1. Player completes Legendary Feat "Dragon-Slayer"
 * 2. Feat broadcasts OnFeatCompleted event
 * 3. Manager checks Feat for FAntagonistTrigger
 * 4. If trigger exists, spawns new FActiveAntagonist
 * 5. Antagonist begins appearing in relevant gameplay contexts
 */
UCLASS(BlueprintType)
class ADASTREA_API UAntagonistManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UAntagonistManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ====================
	// Antagonist Creation
	// ====================

	/**
	 * Evaluate a completed Feat to determine if it should spawn an antagonist
	 * Called automatically when a Feat is completed
	 * @param CompletedFeat The Feat that was just achieved
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Spawning")
	void OnFeatCompleted(UFeatDataAsset* CompletedFeat);

	/**
	 * Manually spawn an antagonist from a Feat trigger
	 * @param SourceFeat The Feat that triggered this antagonist
	 * @param Goal The antagonist's primary motivation
	 * @param InitialHeat Starting heat level (0-100)
	 * @return The newly created antagonist instance
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Spawning")
	FActiveAntagonist SpawnAntagonist(UFeatDataAsset* SourceFeat, EAntagonistGoal Goal, int32 InitialHeat = 50);

	// ====================
	// Antagonist Management
	// ====================

	/**
	 * Get all currently active antagonists
	 * @return Array of active antagonist instances
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|Query")
	TArray<FActiveAntagonist> GetActiveAntagonists() const;

	/**
	 * Get a specific antagonist by their unique ID
	 * @param AntagonistID The unique identifier to search for
	 * @param OutAntagonist The found antagonist data
	 * @return True if antagonist was found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|Query")
	bool GetAntagonistByID(const FGuid& AntagonistID, FActiveAntagonist& OutAntagonist) const;

	/**
	 * Get all antagonists with a specific goal
	 * @param Goal The goal type to filter by
	 * @return Array of antagonists with matching goal
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|Query")
	TArray<FActiveAntagonist> GetAntagonistsByGoal(EAntagonistGoal Goal) const;

	/**
	 * Get all antagonists above a certain heat threshold
	 * @param MinHeat Minimum heat level to include
	 * @return Array of high-heat antagonists
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|Query")
	TArray<FActiveAntagonist> GetHighHeatAntagonists(int32 MinHeat = 70) const;

	/**
	 * Check if any antagonists are currently active
	 * @return True if at least one antagonist exists
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|Query")
	bool HasActiveAntagonists() const;

	// ====================
	// Antagonist Modification
	// ====================

	/**
	 * Modify the heat level of a specific antagonist
	 * @param AntagonistID The antagonist to modify
	 * @param HeatDelta Amount to change heat by (can be negative)
	 * @return True if antagonist was found and modified
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Modification")
	bool ModifyAntagonistHeat(const FGuid& AntagonistID, int32 HeatDelta);

	/**
	 * Record an encounter with an antagonist
	 * Increments encounter count and may adjust heat
	 * @param AntagonistID The antagonist encountered
	 * @return True if antagonist was found and updated
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Modification")
	bool RecordEncounter(const FGuid& AntagonistID);

	/**
	 * Deactivate an antagonist (e.g., defeated, captured, resolved)
	 * @param AntagonistID The antagonist to deactivate
	 * @return True if antagonist was found and deactivated
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Modification")
	bool DeactivateAntagonist(const FGuid& AntagonistID);

	/**
	 * Reactivate a previously deactivated antagonist
	 * @param AntagonistID The antagonist to reactivate
	 * @return True if antagonist was found and reactivated
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|Modification")
	bool ReactivateAntagonist(const FGuid& AntagonistID);

	// ====================
	// System Functions
	// ====================

	/**
	 * Update all antagonist states (heat decay, activity checks)
	 * Called periodically by the manager
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|System")
	void UpdateAntagonists(float DeltaTime);

	/**
	 * Clear all antagonists (for testing or new game)
	 */
	UFUNCTION(BlueprintCallable, Category="Antagonist|System")
	void ClearAllAntagonists();

	/**
	 * Get the total number of antagonists (active and inactive)
	 * @return Total antagonist count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Antagonist|System")
	int32 GetTotalAntagonistCount() const;

protected:
	/** Array of all antagonists in the current game session */
	UPROPERTY()
	TArray<FActiveAntagonist> Antagonists;

	/** Timer handle for periodic antagonist updates */
	FTimerHandle UpdateTimerHandle;

	/**
	 * Generate a unique name for a new antagonist
	 * @param SourceFeat The Feat that triggered this antagonist
	 * @param Goal The antagonist's primary motivation
	 * @return Generated name for the antagonist
	 */
	FText GenerateAntagonistName(UFeatDataAsset* SourceFeat, EAntagonistGoal Goal) const;

	/**
	 * Apply heat decay to all active antagonists over time
	 * @param DeltaTime Time since last decay update
	 */
	void ApplyHeatDecay(float DeltaTime);

	/**
	 * Start the periodic update timer
	 */
	void StartUpdateTimer();

	/**
	 * Stop the periodic update timer
	 */
	void StopUpdateTimer();
};
