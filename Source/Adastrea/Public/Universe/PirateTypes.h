#pragma once

#include "CoreMinimal.h"
#include "PirateTypes.generated.h"

/**
 * What a pirate gang needs, roughly most valuable first (see PIRACY_AND_LAW.md 2a).
 * Each gang keeps a level of 0..100 for every need.
 */
UENUM(BlueprintType)
enum class EPirateNeed : uint8
{
	/** Crew. Filled by captives and pods (later steps). */
	People,
	/** Someone who can dock at lawful stations for them (a captured player, later steps). */
	Access,
	/** Hulls, components, salvage. */
	Parts,
	/** Fuel, food, medicine, water. */
	Supplies,
	/** Data, logs, manifests. */
	Information,
	/** Credits, and goods only good for selling at a fence. */
	Cash,
	Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EPirateNeed, EPirateNeed::Count);

/** A gang's level for each need (0 = has none, 100 = well stocked). */
USTRUCT(BlueprintType)
struct ADASTREA_API FPirateNeeds
{
	GENERATED_BODY()

	static constexpr int32 Num = 6;

	/** Indexed by EPirateNeed. Saved, not exposed to Blueprint (static arrays can't be). */
	UPROPERTY(EditAnywhere, Category="Pirates")
	float Levels[Num] = { 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f };

	float Get(EPirateNeed Need) const { return Levels[(int32)Need]; }
	void Set(EPirateNeed Need, float Value) { Levels[(int32)Need] = FMath::Clamp(Value, 0.0f, 100.0f); }
	void Add(EPirateNeed Need, float Delta) { Set(Need, Get(Need) + Delta); }
};

static_assert(FPirateNeeds::Num == (int32)EPirateNeed::Count, "FPirateNeeds::Levels must have one entry per EPirateNeed");
