// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Characters/DeckNavGrid.h"
#include "NPCPopulationSubsystem.generated.h"

class AAdastreaNPC;
class ASpaceshipAvatar;

/**
 * Puts people aboard the interior the player is walking, and only that one:
 * when the player's avatar has settled in a ship interior, the subsystem bakes
 * that deck's FDeckNavGrid and spawns its crew (the ship's CrewRequired, capped
 * by adastrea.NPC.Max); when the player leaves, the crew is removed again.
 * Everyone else in the galaxy stays plain numbers until the player is there.
 *
 * It watches the player's pawn each tick instead of hooking the player
 * controller's interior transitions, so it needs nothing from them.
 */
UCLASS()
class ADASTREA_API UNPCPopulationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Deinitialize() override;

	/** Walkable grid of an interior the player is in (null if not baked). */
	const FDeckNavGrid* GetGrid(const AActor* Interior) const;

	/** Fixtures (consoles, monitors, ...) inside an interior, for NPCs to go and work at. */
	const TArray<TWeakObjectPtr<AActor>>& GetFixtures(const AActor* Interior) const;

	/** Spawn Count more crew in the player's current interior (test/console use). */
	int32 SpawnCrew(int32 Count);

	/** Remove everyone this subsystem spawned. */
	void ClearCrew();

	/** Draw the grid and current NPC paths for a few seconds. */
	void DrawDebug(float Duration) const;

	/** Log the walkable grid at the player's deck level. */
	void DumpGrid() const;

	/** Log how the bake sees one column of the current interior (interior-local cm). */
	void ProbeColumn(float LocalX, float LocalY) const { Grid.ProbeColumn(GetWorld(), LocalX, LocalY); }

	const TArray<TWeakObjectPtr<AAdastreaNPC>>& GetCrew() const { return Crew; }

private:
	void Populate(ASpaceshipAvatar* Avatar, AActor* Interior);
	AActor* GetPlayerInterior(ASpaceshipAvatar*& OutAvatar) const;

	TWeakObjectPtr<AActor> CurrentInterior;
	float SettleTime = 0.0f;
	bool bPopulated = false;

	FDeckNavGrid Grid;
	TArray<TWeakObjectPtr<AActor>> Fixtures;
	TArray<TWeakObjectPtr<AAdastreaNPC>> Crew;
	FRandomStream Random;
};
