// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "AIPilotController.generated.h"

class ASpaceship;
class ASpaceStation;
class ADockingBayModule;
class UMarketDataAsset;

/** What the AI pilot is currently doing. */
UENUM(BlueprintType)
enum class EAIPilotState : uint8
{
	/** Pick the next station (and cargo) to run to. */
	Choosing,
	/** Flying to the destination station's docking bay. */
	Flying,
	/** Docked: trading, then waiting out the dwell time. */
	Docked
};

/**
 * AI "player": pilots an ASpaceship through the same flight, docking and trading
 * code the human player uses (throttle, RequestDocking, PlayerTraderComponent).
 *
 * Loop: choose the most profitable station -> fly there -> dock -> sell cargo ->
 * buy the best-margin cargo for the next hop -> dwell -> undock -> repeat.
 * Deliberately simple (straight-line flight, no obstacle avoidance) so ships can
 * populate the world and exercise docking/trading/economy while testing.
 *
 * Deliberately derives from AController (not AAIController) so the AIModule
 * dependency stays out of the build. Subclasses (e.g. AAIMinerController) override
 * TickPilot and reuse the steering/docking helpers below.
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API AAIPilotController : public AController
{
	GENERATED_BODY()

public:
	AAIPilotController();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Seconds spent docked (trading/idling) before flying to the next station. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Pilot", meta=(ClampMin="0.0"))
	float DwellTime = 8.0f;

	/** Cruise throttle (0-100) while far from the destination. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Pilot", meta=(ClampMin="0.0", ClampMax="100.0"))
	float CruiseThrottle = 100.0f;

	/** Buy/sell cargo at each dock. When off the ship just shuttles between stations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Pilot")
	bool bTradeAtStations = true;

	UPROPERTY(BlueprintReadOnly, Category="AI Pilot")
	EAIPilotState State = EAIPilotState::Choosing;

	/** The station we're flying to (or docked at). */
	UPROPERTY(BlueprintReadOnly, Category="AI Pilot")
	TObjectPtr<ASpaceStation> TargetStation;

	/** One-line summary of what the pilot is doing right now (e.g. "Flying to Station_Agricultural"), for debug UI. */
	virtual FString GetObjectiveDescription() const;

	/** Display name for a station: its StationName, else the actor's name. */
	static FString GetStationDisplayName(const ASpaceStation* Station);

	/** Spawn an AI-piloted ship of ShipClass near Station, run by a controller of PilotClass. */
	static ASpaceship* SpawnPilotedShip(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station, TSubclassOf<AAIPilotController> PilotClass);

	/** Spawn an AI trader (AAIPilotController) near Station and start it running. */
	static ASpaceship* SpawnAITrader(UWorld* World, TSubclassOf<ASpaceship> ShipClass, ASpaceStation* Station);

protected:
	/** Per-frame behaviour; the base class is the station-to-station trader. */
	virtual void TickPilot(float DeltaSeconds);

	enum class EApproachResult : uint8
	{
		Flying,
		/** Ship is docked (bay slot taken, ship stopped, DockPoint set). */
		Docked,
		/** In range but the bay wouldn't accept us for MaxBlockedSeconds. */
		Blocked
	};

	/**
	 * Fly toward Bay and dock the way the player does. Uses DockedSeconds as the
	 * blocked-bay timer, so reset it before the first call of a leg.
	 */
	EApproachResult ApproachAndDock(ADockingBayModule* Bay, float DeltaSeconds);

	/** Undock from the current bay (freeing its slot) and push the ship clear. */
	void UndockShip();

	/** Rotate toward Location at the ship's turn rate without touching the throttle.
	 * @return Dot product of the ship's forward vector with the direction to Location (1 = dead ahead). */
	float FaceToward(const FVector& Location, float DeltaSeconds);

	/** Turn toward and cruise at Location, easing the throttle off on approach. */
	void SteerToward(const FVector& Location, float DeltaSeconds);

	ASpaceship* GetShip() const;

	/** The station's market if AI traders are allowed to use it, else null. */
	UMarketDataAsset* GetMarket(const ASpaceStation* Station) const;

	/** Where the ship should end up: the bay's free docking point, else the bay itself. */
	FVector GetDockLocation(const ADockingBayModule* Bay) const;

	/** Docking point we're parked at; the ship is pinned to it while docked. */
	UPROPERTY()
	TObjectPtr<USceneComponent> DockPoint;

	UPROPERTY()
	TObjectPtr<ASpaceStation> LastStation;

	/** Seconds in the current state (dwell timer while docked, blocked timer while approaching). */
	float DockedSeconds = 0.0f;

private:
	void TickChoosing();
	void TickFlying(float DeltaSeconds);
	void TickDocked(float DeltaSeconds);

	/** Choose the next destination, preferring the best cargo margin from the current market. */
	void ChooseDestination();

	/** Sell everything, then load the item with the best margin to another station. */
	void TradeAtCurrentStation();

	/** Station the current cargo run was planned for (set by TradeAtCurrentStation). */
	UPROPERTY()
	TObjectPtr<ASpaceStation> PlannedDestination;
};
