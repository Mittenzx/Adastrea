#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerUnlockComponent.generated.h"

/**
 * Types of unlockable content
 */
UENUM(BlueprintType)
enum class EUnlockType : uint8
{
	Ship        UMETA(DisplayName = "Ship"),          // Spaceship
	Module      UMETA(DisplayName = "Module"),        // Station/Ship module
	Weapon      UMETA(DisplayName = "Weapon"),        // Weapon type
	Equipment   UMETA(DisplayName = "Equipment"),     // Equipment item
	Blueprint   UMETA(DisplayName = "Blueprint"),     // Crafting blueprint
	Feature     UMETA(DisplayName = "Feature")        // Game feature/ability
};

/**
 * Unlock requirement types
 */
UENUM(BlueprintType)
enum class EUnlockRequirementType : uint8
{
	Level           UMETA(DisplayName = "Level"),              // Player level
	Achievement     UMETA(DisplayName = "Achievement"),        // Achievement completion
	Quest           UMETA(DisplayName = "Quest"),              // Quest completion
	Reputation      UMETA(DisplayName = "Reputation"),         // Faction reputation
	Credits         UMETA(DisplayName = "Credits"),            // Credit cost
	Item            UMETA(DisplayName = "Item")                // Requires specific item
};

/**
 * Individual unlock requirement
 */
USTRUCT(BlueprintType)
struct FUnlockRequirement
{
	GENERATED_BODY()

	/** Type of requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Requirement")
	EUnlockRequirementType Type;

	/** Required value (level, reputation value, credit cost, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Requirement")
	int32 RequiredValue;

	/** Required ID (faction name, quest ID, achievement ID, item ID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Requirement")
	FName RequiredID;

	/** Requirement description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Requirement")
	FText Description;

	FUnlockRequirement()
		: Type(EUnlockRequirementType::Level)
		, RequiredValue(1)
		, RequiredID(NAME_None)
		, Description(FText::GetEmpty())
	{}
};

/**
 * Unlockable content entry
 */
USTRUCT(BlueprintType)
struct FUnlockEntry
{
	GENERATED_BODY()

	/** Unique ID for this unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	FName UnlockID;

	/** Type of unlockable content */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	EUnlockType Type;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock", meta=(MultiLine=true))
	FText Description;

	/** Requirements to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	TArray<FUnlockRequirement> Requirements;

	/** Whether this is unlocked by default (starting equipment) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	bool bUnlockedByDefault;

	/** Whether this is currently unlocked */
	UPROPERTY(BlueprintReadOnly, Category="Unlock")
	bool bIsUnlocked;

	/** Icon/thumbnail for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlock")
	class UTexture2D* Icon;

	FUnlockEntry()
		: UnlockID(NAME_None)
		, Type(EUnlockType::Ship)
		, DisplayName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, bUnlockedByDefault(false)
		, bIsUnlocked(false)
		, Icon(nullptr)
	{}
};

/**
 * Player unlock component managing unlockable content.
 * Tracks ships, modules, weapons, equipment, and features unlocked by the player.
 * 
 * Usage:
 * - Attach to player pawn or controller
 * - Configure unlockables via Data Assets or Blueprint
 * - Check requirements and unlock content as player progresses
 * - Query unlock status for UI and gameplay restrictions
 * 
 * Example:
 * - Check if ship is unlocked: IsUnlocked("Ship_Warhammer")
 * - Try to unlock: TryUnlock("Ship_Warhammer")
 * - Get unlock progress: GetUnlockProgress("Ship_Warhammer")
 * - Query all unlocked ships: GetUnlockedByType(EUnlockType::Ship)
 * 
 * Integration:
 * - PlayerProgressionComponent provides level requirements
 * - AchievementManagerSubsystem provides achievement requirements
 * - QuestManagerSubsystem provides quest requirements
 * - PlayerReputationComponent provides reputation requirements
 * - Game Instance provides credit checks
 */
UCLASS(BlueprintType, ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UPlayerUnlockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerUnlockComponent();

	// ====================
	// Core Properties
	// ====================

	/** All unlockable content entries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unlocks")
	TArray<FUnlockEntry> Unlocks;

	/** Unlocked content IDs (for save/load) */
	UPROPERTY(BlueprintReadOnly, Category="Unlocks")
	TArray<FName> UnlockedIDs;

	// ====================
	// Unlock Management
	// ====================

	/**
	 * Try to unlock content (checks requirements)
	 * @param UnlockID ID of content to unlock
	 * @param bIgnoreRequirements Bypass requirement checks (for cheats/rewards)
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category="Unlocks")
	bool TryUnlock(FName UnlockID, bool bIgnoreRequirements = false);

	/**
	 * Force unlock content (bypass all requirements)
	 * @param UnlockID ID of content to unlock
	 */
	UFUNCTION(BlueprintCallable, Category="Unlocks")
	void ForceUnlock(FName UnlockID);

	/**
	 * Check if content is unlocked
	 * @param UnlockID ID of content to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks")
	bool IsUnlocked(FName UnlockID) const;

	/**
	 * Check if all requirements are met for unlock
	 * @param UnlockID ID of content to check
	 * @return True if all requirements met
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks")
	bool MeetsRequirements(FName UnlockID) const;

	/**
	 * Get unlock progress (0.0 to 1.0)
	 * @param UnlockID ID of content to check
	 * @return Progress percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks")
	float GetUnlockProgress(FName UnlockID) const;

	/**
	 * Add new unlockable content at runtime
	 * @param NewUnlock Unlock entry to add
	 */
	UFUNCTION(BlueprintCallable, Category="Unlocks")
	void AddUnlock(const FUnlockEntry& NewUnlock);

	// ====================
	// Query Functions
	// ====================

	/**
	 * Get all unlocked content of a specific type
	 * @param Type Content type to filter
	 * @return Array of unlocked content IDs
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	TArray<FName> GetUnlockedByType(EUnlockType Type) const;

	/**
	 * Get all unlocks of a specific type (locked and unlocked)
	 * @param Type Content type to filter
	 * @return Array of unlock entries
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	TArray<FUnlockEntry> GetUnlocksByType(EUnlockType Type) const;

	/**
	 * Get unlock entry by ID
	 * @param UnlockID ID of content to find
	 * @param OutUnlock Found unlock entry
	 * @return True if found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	bool GetUnlockEntry(FName UnlockID, FUnlockEntry& OutUnlock) const;

	/**
	 * Get total number of unlocks
	 * @return Total unlock count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	int32 GetTotalUnlocksCount() const { return Unlocks.Num(); }

	/**
	 * Get number of unlocked items
	 * @return Unlocked count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	int32 GetUnlockedCount() const { return UnlockedIDs.Num(); }

	/**
	 * Get unlock completion percentage
	 * @return Completion as 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unlocks|Query")
	float GetUnlockCompletionPercentage() const;

	/**
	 * Get unmet requirements for an unlock
	 * @param UnlockID ID of content to check
	 * @return Array of unmet requirements
	 */
	UFUNCTION(BlueprintCallable, Category="Unlocks|Query")
	TArray<FUnlockRequirement> GetUnmetRequirements(FName UnlockID) const;

	// ====================
	// Events
	// ====================

	/** Event fired when content is unlocked */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContentUnlocked, FName, UnlockID, EUnlockType, Type);
	UPROPERTY(BlueprintAssignable, Category="Unlocks|Events")
	FOnContentUnlocked OnContentUnlocked;

	/** Event fired when unlock attempt fails */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnlockFailed, FName, UnlockID, FText, Reason);
	UPROPERTY(BlueprintAssignable, Category="Unlocks|Events")
	FOnUnlockFailed OnUnlockFailed;

protected:
	virtual void BeginPlay() override;

	/**
	 * Initialize default unlocks
	 */
	void InitializeUnlocks();

	/**
	 * Find unlock entry by ID
	 * @param UnlockID ID to find
	 * @return Pointer to unlock entry, or nullptr
	 */
	FUnlockEntry* FindUnlock(FName UnlockID);

	/**
	 * Find unlock entry (const version)
	 * @param UnlockID ID to find
	 * @return Pointer to unlock entry, or nullptr
	 */
	const FUnlockEntry* FindUnlock(FName UnlockID) const;

	/**
	 * Check if a single requirement is met
	 * @param Requirement Requirement to check
	 * @return True if requirement is met
	 */
	bool CheckRequirement(const FUnlockRequirement& Requirement) const;
};
