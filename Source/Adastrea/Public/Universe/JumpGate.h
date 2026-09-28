// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "JumpGate.generated.h"

class UInstancedStaticMeshComponent;
class UPointLightComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
struct FGalaxySectorDef;

/**
 * A jump gate: fly the player's ship through the ring to travel to TargetSectorId
 * (a sector id in Content/Data/Universe/Galaxy.json). The jump itself is
 * UGalaxySubsystem::BeginJump (loads the target sector's level).
 *
 * Gates are spawned automatically for every gate/lane gate of the current sector
 * (UJumpGateWorldSubsystem), so levels need no edits. A gate placed by hand in
 * a level with the same TargetSectorId replaces the automatic one.
 *
 * The ring faces along the actor's forward (X) axis; ships arriving through
 * this gate come out ExitDistance in front of it, facing forward.
 * A gate to a planned sector (no level yet) shows as offline (amber) and
 * doesn't jump.
 */
UCLASS()
class ADASTREA_API AJumpGate : public AActor
{
	GENERATED_BODY()

public:
	AJumpGate();

	/** Galaxy sector this gate leads to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Gate")
	FName TargetSectorId;

	/** Inner radius of the ring (cm). The ship must pass inside it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Gate", meta=(ClampMin="1000"))
	float RingRadius = 4000.0f;

	/** How far in front of the gate arriving ships appear (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Jump Gate", meta=(ClampMin="2000"))
	float ExitDistance = 15000.0f;

	/** Set when UJumpGateWorldSubsystem spawned this gate (not placed in the level). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Jump Gate")
	bool bAutoSpawned = false;

	UFUNCTION(BlueprintCallable, Category="Jump Gate")
	void SetTargetSector(FName InTargetSectorId);

	/** True if the target sector has a level to jump to. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jump Gate")
	bool IsOnline() const;

	/** Destination sector name ("Alpha Reach"), or the raw id if unknown. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jump Gate")
	FString GetDestinationName() const;

	/** Name for HUD / targeting: "Gate: Alpha Reach". */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Jump Gate")
	FString GetDisplayName() const;

	FVector GetArrivalLocation() const;
	FRotator GetArrivalRotation() const;

	/** Don't trigger again until the ship has left the ring area (used on arrival). */
	void Disarm() { bArmed = false; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UInstancedStaticMeshComponent> Ring;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

private:
	void BuildRing();
	void RefreshAppearance();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMaterial;

	bool bArmed = true;
};

/**
 * Per-level gate setup: spawns the current sector's jump gates when the level
 * starts playing, and finishes a jump that is arriving in this level.
 */
UCLASS()
class ADASTREA_API UJumpGateWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Distance of automatic gates from the sector centre (cm): ~70 s at cruise, ~35 s boosting. */
	static constexpr float AutoGateDistance = 200000.0f;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** The gate in this level leading to SectorId, or null. */
	AJumpGate* FindGateTo(FName SectorId) const;

	/** Where automatic gates are laid out around: sector marker, else the stations' centroid, else the origin. */
	FVector GetSectorCenter(FName SectorId) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void SpawnGates(FName SectorId);
	void TryCompleteArrival();

	FTimerHandle ArrivalTimer;
	float ArrivalWaited = 0.0f;
};
