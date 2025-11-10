// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AdastreaGameState.generated.h"

/**
 * Structure representing an active galactic event
 * Used for replication instead of TMap (which is not supported)
 */
USTRUCT(BlueprintType)
struct FActiveEvent
{
	GENERATED_BODY()

	/** Name of the active event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Events")
	FName EventName;

	/** Days remaining for the event (0 = indefinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Events")
	int32 DaysRemaining;

	FActiveEvent()
		: EventName(NAME_None)
		, DaysRemaining(0)
	{}

	FActiveEvent(FName InEventName, int32 InDaysRemaining)
		: EventName(InEventName)
		, DaysRemaining(InDaysRemaining)
	{}
};

/**
 * Game State for Adastrea
 * 
 * This class stores replicated game state information that should be synchronized
 * across all clients in multiplayer scenarios. It tracks the current state of the
 * game session, active events, and dynamic world conditions.
 * 
 * Key Responsibilities:
 * - Tracking current mission/quest state
 * - Managing active galactic events (wars, trade embargoes, etc.)
 * - Storing dynamic market conditions and prices
 * - Coordinating time progression and day/night cycles
 * - Managing active NPCs and AI factions in the current session
 * 
 * Replication:
 * - All properties marked for replication are synchronized to clients
 * - Use this for state that all players need to see
 * - Non-replicated local state should go in PlayerController or GameInstance
 * 
 * Usage:
 * - Access via UGameplayStatics::GetGameState()
 * - Store multiplayer-synchronized state here
 * - Use replicated properties for client visibility
 * - Create Blueprints derived from this class for project-specific state
 * 
 * Integration Points:
 * - Trading system for market state updates
 * - Faction system for galactic events
 * - Quest/mission system for active objectives
 * - Time management for in-game calendar
 */
UCLASS(BlueprintType)
class ADASTREA_API AAdastreaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAdastreaGameState();

	/**
	 * Get the current in-game time (days since game start)
	 * @return The number of in-game days elapsed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Time")
	int32 GetCurrentGameDay() const { return CurrentGameDay; }

	/**
	 * Advance the in-game time by one day
	 * Triggers events and updates that occur on day change
	 */
	UFUNCTION(BlueprintCallable, Category="Time")
	void AdvanceDay();

	/**
	 * Check if a specific galactic event is currently active
	 * @param EventName The name of the event to check
	 * @return True if the event is active
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Events")
	bool IsEventActive(const FName& EventName) const;

	/**
	 * Activate a galactic event (war, trade embargo, festival, etc.)
	 * @param EventName The name of the event to activate
	 * @param Duration How many days the event should last (0 = indefinite)
	 */
	UFUNCTION(BlueprintCallable, Category="Events")
	void ActivateEvent(const FName& EventName, int32 Duration);

	/**
	 * Deactivate a currently active event
	 * @param EventName The name of the event to deactivate
	 */
	UFUNCTION(BlueprintCallable, Category="Events")
	void DeactivateEvent(const FName& EventName);

protected:
	/**
	 * Current in-game day (time progression)
	 * Replicated to all clients
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Time")
	int32 CurrentGameDay;

	/**
	 * Array of active galactic events and their remaining durations
	 * Replicated to all clients
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Events")
	TArray<FActiveEvent> ActiveEvents;

	/**
	 * List of all active NPC trader ships in the current session
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="NPCs")
	TArray<class AActor*> ActiveTraders;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called when a day advances
	 * Update event durations and trigger day-change logic
	 */
	virtual void OnDayAdvanced();
};
