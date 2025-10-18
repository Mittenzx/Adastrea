#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TradeContractDataAsset.generated.h"

// Forward declarations
class UTradeItemDataAsset;
class UFactionDataAsset;

/**
 * Enum for contract types
 */
UENUM(BlueprintType)
enum class ETradeContractType : uint8
{
	Delivery UMETA(DisplayName = "Delivery"),
	Procurement UMETA(DisplayName = "Procurement"),
	Trade UMETA(DisplayName = "Trade"),
	Escort UMETA(DisplayName = "Escort"),
	Smuggling UMETA(DisplayName = "Smuggling"),
	RareGoods UMETA(DisplayName = "Rare Goods"),
	Emergency UMETA(DisplayName = "Emergency Supply"),
	Bulk UMETA(DisplayName = "Bulk Trade")
};

/**
 * Enum for contract difficulty
 */
UENUM(BlueprintType)
enum class EContractDifficulty : uint8
{
	Trivial UMETA(DisplayName = "Trivial"),
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	VeryHard UMETA(DisplayName = "Very Hard"),
	Extreme UMETA(DisplayName = "Extreme")
};

/**
 * Enum for contract status
 */
UENUM(BlueprintType)
enum class EContractStatus : uint8
{
	Available UMETA(DisplayName = "Available"),
	Active UMETA(DisplayName = "Active"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed"),
	Expired UMETA(DisplayName = "Expired"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

/**
 * Contract cargo requirement
 */
USTRUCT(BlueprintType)
struct FContractCargo
{
	GENERATED_BODY()

	// The trade item required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cargo")
	UTradeItemDataAsset* TradeItem;

	// Quantity required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cargo", meta=(ClampMin="1"))
	int32 Quantity;

	// Whether cargo must be delivered in pristine condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cargo")
	bool bRequiresPristineCondition;

	FContractCargo()
		: TradeItem(nullptr)
		, Quantity(1)
		, bRequiresPristineCondition(false)
	{}
};

/**
 * Contract rewards
 */
USTRUCT(BlueprintType)
struct FContractRewards
{
	GENERATED_BODY()

	// Credits paid on completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rewards", meta=(ClampMin="0"))
	int32 Credits;

	// Reputation gain with contract giver faction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rewards", meta=(ClampMin="0"))
	int32 ReputationGain;

	// Bonus items given on completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rewards")
	TArray<FContractCargo> BonusItems;

	// Experience points awarded
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rewards", meta=(ClampMin="0"))
	int32 ExperiencePoints;

	FContractRewards()
		: Credits(0)
		, ReputationGain(0)
		, ExperiencePoints(0)
	{}
};

/**
 * Contract penalties for failure
 */
USTRUCT(BlueprintType)
struct FContractPenalties
{
	GENERATED_BODY()

	// Credits lost on failure
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penalties", meta=(ClampMin="0"))
	int32 CreditPenalty;

	// Reputation lost on failure
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penalties", meta=(ClampMin="0"))
	int32 ReputationPenalty;

	// Collateral deposit required (refunded on success)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penalties", meta=(ClampMin="0"))
	int32 CollateralDeposit;

	FContractPenalties()
		: CreditPenalty(0)
		, ReputationPenalty(0)
		, CollateralDeposit(0)
	{}
};

/**
 * Contract location/destination
 */
USTRUCT(BlueprintType)
struct FContractLocation
{
	GENERATED_BODY()

	// Location name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
	FText LocationName;

	// Location coordinates (for navigation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
	FVector Coordinates;

	// Market ID if at a market
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
	FName MarketID;

	// Faction controlling this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location")
	UFactionDataAsset* ControllingFaction;

	FContractLocation()
		: LocationName(FText::FromString(TEXT("Unknown Location")))
		, Coordinates(FVector::ZeroVector)
		, MarketID(NAME_None)
		, ControllingFaction(nullptr)
	{}
};

/**
 * Trade Contract Data Asset
 * Defines trade missions and contracts for players and AI
 */
UCLASS(BlueprintType)
class ADASTREA_API UTradeContractDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ====================
	// BASIC INFO
	// ====================

	// Display name of the contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info")
	FText ContractName;

	// Description of the contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info", meta=(MultiLine=true))
	FText Description;

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info")
	FName ContractID;

	// Contract type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info")
	ETradeContractType ContractType;

	// Difficulty rating
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info")
	EContractDifficulty Difficulty;

	// Faction offering the contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Info")
	UFactionDataAsset* ContractGiver;

	// ====================
	// CONTRACT DETAILS
	// ====================

	// Cargo requirements
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details")
	TArray<FContractCargo> RequiredCargo;

	// Origin location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details")
	FContractLocation OriginLocation;

	// Destination location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details")
	FContractLocation DestinationLocation;

	// Distance to destination (in light years or game units)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details", meta=(ClampMin="0"))
	float Distance;

	// Time limit in game hours (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details", meta=(ClampMin="0"))
	float TimeLimit;

	// Minimum reputation required to accept (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Contract Details", meta=(ClampMin="-100", ClampMax="100"))
	int32 MinReputationRequired;

	// ====================
	// REWARDS & PENALTIES
	// ====================

	// Rewards for completing the contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	FContractRewards Rewards;

	// Penalties for failing the contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rewards")
	FContractPenalties Penalties;

	// ====================
	// CONTRACT STATE
	// ====================

	// Current status
	UPROPERTY(BlueprintReadWrite, Category="Contract State")
	EContractStatus Status;

	// Time when contract was accepted
	UPROPERTY(BlueprintReadWrite, Category="Contract State")
	float AcceptedTime;

	// Time when contract expires
	UPROPERTY(BlueprintReadWrite, Category="Contract State")
	float ExpirationTime;

	// Player ID who accepted this contract
	UPROPERTY(BlueprintReadWrite, Category="Contract State")
	FName AcceptedByPlayerID;

	// ====================
	// SPECIAL CONDITIONS
	// ====================

	// Whether this is a repeatable contract
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Conditions")
	bool bRepeatable;

	// Whether stealth/smuggling is required
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Conditions")
	bool bRequiresStealth;

	// Whether combat is expected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Conditions")
	bool bCombatExpected;

	// Hostile factions along route
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Conditions")
	TArray<UFactionDataAsset*> HostileFactions;

	// Special tags for filtering and AI behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Special Conditions")
	TArray<FName> ContractTags;

	// ====================
	// Constructor
	// ====================

	UTradeContractDataAsset();

	// ====================
	// Blueprint Callable Functions
	// ====================

	/**
	 * Check if player meets requirements to accept contract
	 * @param PlayerReputation Player's reputation with contract giver
	 * @param PlayerCargoCapacity Player's available cargo space
	 * @return True if player can accept
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool CanPlayerAccept(int32 PlayerReputation, float PlayerCargoCapacity) const;

	/**
	 * Calculate total cargo volume required
	 * @return Total volume in cubic meters
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	float GetTotalCargoVolume() const;

	/**
	 * Calculate total cargo mass required
	 * @return Total mass in kilograms
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	float GetTotalCargoMass() const;

	/**
	 * Get estimated completion time based on distance and average speed
	 * @param AverageSpeed Average travel speed
	 * @return Estimated hours to complete
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	float GetEstimatedCompletionTime(float AverageSpeed) const;

	/**
	 * Calculate profit margin (rewards - costs)
	 * @param CargoAcquisitionCost Cost to acquire the cargo
	 * @param FuelCost Estimated fuel cost
	 * @return Net profit in credits
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	int32 CalculateProfitMargin(int32 CargoAcquisitionCost, int32 FuelCost) const;

	/**
	 * Get remaining time before expiration
	 * @param CurrentGameTime Current game time
	 * @return Hours remaining
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	float GetRemainingTime(float CurrentGameTime) const;

	/**
	 * Check if contract has expired
	 * @param CurrentGameTime Current game time
	 * @return True if contract has expired
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool IsExpired(float CurrentGameTime) const;

	/**
	 * Accept the contract
	 * @param PlayerID Player accepting the contract
	 * @param CurrentGameTime Current game time
	 * @return True if successfully accepted
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool AcceptContract(FName PlayerID, float CurrentGameTime);

	/**
	 * Complete the contract
	 * @param CurrentGameTime Current game time
	 * @return True if successfully completed
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool CompleteContract(float CurrentGameTime);

	/**
	 * Fail the contract
	 * @param Reason Reason for failure
	 * @return True if contract was active and is now failed
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool FailContract(const FText& Reason);

	/**
	 * Check if contract has a specific tag
	 * @param Tag The tag to check for
	 * @return True if contract has the tag
	 */
	UFUNCTION(BlueprintCallable, Category="Trade Contract")
	bool HasTag(FName Tag) const;

	/**
	 * BlueprintNativeEvent: Called when contract is accepted
	 * @param PlayerID Player who accepted
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Trade Contract|Events")
	void OnContractAccepted(FName PlayerID);

	/**
	 * BlueprintNativeEvent: Called when contract is completed
	 * @param PlayerID Player who completed
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Trade Contract|Events")
	void OnContractCompleted(FName PlayerID);

	/**
	 * BlueprintNativeEvent: Called when contract is failed
	 * @param PlayerID Player who failed
	 * @param Reason Reason for failure
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Trade Contract|Events")
	void OnContractFailed(FName PlayerID, const FText& Reason);

	/**
	 * BlueprintNativeEvent: Called when contract expires
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Trade Contract|Events")
	void OnContractExpired();
};
