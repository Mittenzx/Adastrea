// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OwnershipComponent.generated.h"

class UTradeItemDataAsset;

/**
 * Marks which organisation owns an actor (see UOrganisationSubsystem).
 *
 * AAIShipPopulator adds one to every roster ship it spawns. On a ship with a
 * UPlayerTraderComponent it reports each sale, so the profit reaches the owner.
 * Add one to a placed station to give it an owner other than the sector's
 * authority (the default).
 */
UCLASS(ClassGroup=(Adastrea), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UOwnershipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOwnershipComponent();

	/** Organisation id from Organisations.json. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ownership")
	FName OwnerId;

	/** Roster record this ship stands in for (ships spawned from the roster only). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ownership")
	FName ShipRecordId;

	/** The ship's name from the roster ("Juno's Wake"); empty for non-roster actors. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ownership")
	FText RecordName;

	/** Credits the ship trades with (its trader's balance when this began play); anything above it after a sale is profit. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ownership")
	int32 WorkingCapital = 0;

	/** The actor's ownership component, if it has one. */
	static UOwnershipComponent* Find(const AActor* Actor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleItemSold(UTradeItemDataAsset* Item, int32 Quantity, int32 TotalValue);
};
