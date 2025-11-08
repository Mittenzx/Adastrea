#pragma once

#include "CoreMinimal.h"
#include "Antagonist.generated.h"

// Forward declarations
class UFeatDataAsset;

/**
 * Defines the primary motivation and goal of an antagonist.
 * This determines their behavior patterns and how they pursue the player.
 */
UENUM(BlueprintType)
enum class EAntagonistGoal : uint8
{
	Revenge			UMETA(DisplayName = "Revenge"),			// Seeks to defeat/destroy the player
	Competition		UMETA(DisplayName = "Competition"),		// Wants to surpass the player's achievements
	Obsession		UMETA(DisplayName = "Obsession"),		// Fascinated by player, wants to study/capture
	Jealousy		UMETA(DisplayName = "Jealousy"),		// Envious of player's success, seeks to undermine
	Honor			UMETA(DisplayName = "Honor"),			// Must prove superiority due to code/honor
	Curiosity		UMETA(DisplayName = "Curiosity"),		// Intrigued by player's unique abilities/approach
	Greed			UMETA(DisplayName = "Greed"),			// Wants to profit from player's defeat or capture
	Justice			UMETA(DisplayName = "Justice")			// Believes player must answer for their actions
};

/**
 * Represents a live, active antagonist in the game world.
 * 
 * An antagonist is a unique NPC rival spawned in response to the player's
 * legendary accomplishments (Feats). They provide personalized opposition
 * and recurring confrontations throughout the player's journey.
 * 
 * Key Characteristics:
 * - Spawned by completing specific high-tier Feats
 * - Unique generated identity and motivation
 * - Persistent across gameplay sessions
 * - Heat level tracks their current activity/aggression
 * - Can appear in various gameplay contexts (combat, diplomacy, trading)
 * 
 * Design Inspiration:
 * - Manga-style recurring rivals (e.g., Shangri-La Frontier)
 * - Personal nemesis systems from games like Shadow of Mordor
 * - Dynamic reputation-based spawning
 * 
 * Usage:
 * - Created and managed by UAntagonistManager
 * - Stored in game save data for persistence
 * - Referenced by AI systems for behavior
 * - Tracked by quest/event systems for encounters
 */
USTRUCT(BlueprintType)
struct FActiveAntagonist
{
	GENERATED_BODY()

	/** Unique identifier for this antagonist instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
	FGuid AntagonistID;

	/** Generated name for this antagonist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
	FText AntagonistName;

	/** The Feat that triggered this antagonist's creation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Origin")
	UFeatDataAsset* TriggeringFeat;

	/** Primary motivation driving this antagonist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
	EAntagonistGoal PrimaryGoal;

	/** 
	 * Current Heat level (0-100)
	 * Represents how actively this antagonist is pursuing the player
	 * Higher heat = more frequent encounters and aggressive behavior
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior", meta=(ClampMin="0", ClampMax="100"))
	int32 HeatLevel;

	/** Optional: Specific faction this antagonist is affiliated with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Affiliation")
	FName FactionID;

	/** Optional: Custom traits or modifiers for this antagonist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Customization")
	TArray<FName> TraitIDs;

	/** Timestamp of when this antagonist was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking")
	FDateTime CreationTime;

	/** Number of times player has encountered this antagonist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracking")
	int32 EncounterCount;

	/** Whether this antagonist is currently active in the world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bIsActive;

	FActiveAntagonist()
		: AntagonistID(FGuid::NewGuid())
		, AntagonistName(FText::FromString(TEXT("Unknown Rival")))
		, TriggeringFeat(nullptr)
		, PrimaryGoal(EAntagonistGoal::Competition)
		, HeatLevel(50)
		, FactionID(NAME_None)
		, CreationTime(FDateTime::Now())
		, EncounterCount(0)
		, bIsActive(true)
	{}
};
