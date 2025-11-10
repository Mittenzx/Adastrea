#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SectorCouncil.generated.h"

// Forward declarations
class UWayDataAsset;

/**
 * Types of sector policies that can be enacted by councils
 */
UENUM(BlueprintType)
enum class ESectorPolicyType : uint8
{
    SecurityBudget      UMETA(DisplayName = "Security Budget"),      // Funding for sector policing
    TradeRegulation     UMETA(DisplayName = "Trade Regulation"),     // Trade rules and tariffs
    ResourceAllocation  UMETA(DisplayName = "Resource Allocation"),  // How shared resources are distributed
    InfrastructureFund  UMETA(DisplayName = "Infrastructure Fund"),  // Station and facility maintenance
    EmergencyReserve    UMETA(DisplayName = "Emergency Reserve"),    // Emergency response funding
    PatrolFrequency     UMETA(DisplayName = "Patrol Frequency")      // Security patrol scheduling
};

/**
 * Structure representing a sector policy decision
 */
USTRUCT(BlueprintType)
struct FSectorPolicy
{
    GENERATED_BODY()

    /** Type of policy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy")
    ESectorPolicyType PolicyType;

    /** Name of the policy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy")
    FText PolicyName;

    /** Description of what this policy does */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy", meta=(MultiLine=true))
    FText Description;

    /** Numerical value for the policy (e.g., tax rate %, budget amount) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy", meta=(ClampMin="0", ClampMax="100"))
    int32 PolicyValue;

    /** Whether this policy is currently active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy")
    bool bIsActive;

    /** When this policy was enacted */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Policy")
    FDateTime EnactedDate;

    FSectorPolicy()
        : PolicyType(ESectorPolicyType::SecurityBudget)
        , PolicyName(FText::FromString(TEXT("Unnamed Policy")))
        , Description(FText::GetEmpty())
        , PolicyValue(50)
        , bIsActive(false)
        , EnactedDate(FDateTime::MinValue())
    {}
};

/**
 * Structure representing a Way's representative on a sector council
 */
USTRUCT(BlueprintType)
struct FCouncilRepresentative
{
    GENERATED_BODY()

    /** The Way this representative belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Representative")
    UWayDataAsset* RepresentingWay;

    /** Name of the representative (can be NPC or player-assigned) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Representative")
    FText RepresentativeName;

    /** Voting weight for this representative (influenced by Way's power in sector) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Representative", meta=(ClampMin="0", ClampMax="100"))
    int32 VotingWeight;

    /** Years of service on the council */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Representative")
    int32 YearsOfService;

    /** Whether this representative is the player or player-controlled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Representative")
    bool bIsPlayerControlled;

    FCouncilRepresentative()
        : RepresentingWay(nullptr)
        , RepresentativeName(FText::FromString(TEXT("Unknown Representative")))
        , VotingWeight(10)
        , YearsOfService(0)
        , bIsPlayerControlled(false)
    {}
};

/**
 * Structure representing a vote on a sector issue
 */
USTRUCT(BlueprintType)
struct FCouncilVote
{
    GENERATED_BODY()

    /** The policy or issue being voted on */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    FText IssueDescription;

    /** Proposed policy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    FSectorPolicy ProposedPolicy;

    /** Votes in favor (weighted sum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    int32 VotesFor;

    /** Votes against (weighted sum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    int32 VotesAgainst;

    /** Votes abstaining (weighted sum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    int32 VotesAbstain;

    /** Whether the vote passed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    bool bPassed;

    /** When the vote took place */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vote")
    FDateTime VoteDate;

    FCouncilVote()
        : IssueDescription(FText::GetEmpty())
        , VotesFor(0)
        , VotesAgainst(0)
        , VotesAbstain(0)
        , bPassed(false)
        , VoteDate(FDateTime::MinValue())
    {}
};

/**
 * Data Asset representing a Sector Council.
 * 
 * Each sector has a council where representatives from Ways with presence
 * in the sector make administrative decisions about taxes, policing budget,
 * trade regulations, and other sector-wide concerns.
 * 
 * This is NOT politics - it's practical governance and administration.
 * 
 * Usage Example:
 * - Frontier Sector Council
 *   - Representatives: Scout's League, Mining Cooperative, Freight Haulers
 *   - Security Tax: 15% (needs high patrols due to raiders)
 *   - Trade Tax: 5% (low to encourage commerce)
 *   - Security Budget: 60% (prioritize protection)
 */
UCLASS(BlueprintType)
class ADASTREA_API USectorCouncilDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    USectorCouncilDataAsset();

    // ====================
    // Core Identity
    // ====================

    /** Name of the sector this council governs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText SectorName;

    /** Unique identifier for this sector */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName SectorID;

    /** Description of the sector and its characteristics */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
    FText Description;

    // ====================
    // Council Membership
    // ====================

    /** Representatives from Ways operating in this sector */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Council")
    TArray<FCouncilRepresentative> Representatives;

    /** Total number of council seats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Council", meta=(ClampMin="1", ClampMax="20"))
    int32 TotalSeats;

    /** Minimum voting weight needed to pass a policy (% of total votes) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Council", meta=(ClampMin="50", ClampMax="100"))
    int32 PassThreshold;

    // ====================
    // Taxation
    // ====================

    /** Tax rate for security/policing (% of trade value) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Taxation", meta=(ClampMin="0", ClampMax="50"))
    int32 SecurityTaxRate;

    /** Tax rate for infrastructure maintenance (% of trade value) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Taxation", meta=(ClampMin="0", ClampMax="50"))
    int32 InfrastructureTaxRate;

    /** Tax rate for general sector administration (% of trade value) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Taxation", meta=(ClampMin="0", ClampMax="50"))
    int32 AdministrationTaxRate;

    // ====================
    // Budget Allocation
    // ====================

    /** Percentage of budget allocated to security/policing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Budget", meta=(ClampMin="0", ClampMax="100"))
    int32 SecurityBudgetPercent;

    /** Percentage of budget allocated to infrastructure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Budget", meta=(ClampMin="0", ClampMax="100"))
    int32 InfrastructureBudgetPercent;

    /** Percentage of budget allocated to emergency reserves */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Budget", meta=(ClampMin="0", ClampMax="100"))
    int32 EmergencyReservePercent;

    /** Current funds in sector treasury */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Budget")
    int64 TreasuryFunds;

    // ====================
    // Active Policies
    // ====================

    /** Currently active policies in this sector */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Policies")
    TArray<FSectorPolicy> ActivePolicies;

    /** Historical record of votes taken by this council */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Policies")
    TArray<FCouncilVote> VoteHistory;

    // ====================
    // Council Functions
    // ====================

    /**
     * Get all representatives on this council
     * @return Array of council representatives
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Members")
    TArray<FCouncilRepresentative> GetRepresentatives() const;

    /**
     * Get representative for a specific Way
     * @param Way The Way to find representative for
     * @param OutRepresentative The representative (if found)
     * @return True if Way has a representative
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Members")
    bool GetRepresentativeForWay(const UWayDataAsset* Way, FCouncilRepresentative& OutRepresentative) const;

    /**
     * Check if a Way has representation on this council
     * @param Way The Way to check
     * @return True if Way has a representative
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Members")
    bool HasRepresentative(const UWayDataAsset* Way) const;

    /**
     * Get total voting weight of all representatives
     * @return Sum of all voting weights
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Members")
    int32 GetTotalVotingWeight() const;

    // ====================
    // Tax Functions
    // ====================

    /**
     * Calculate total tax rate for this sector
     * @return Combined tax rate percentage
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Taxation")
    int32 GetTotalTaxRate() const;

    /**
     * Calculate tax owed on a transaction
     * @param TransactionValue The value being taxed
     * @return Tax amount owed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Taxation")
    int64 CalculateTaxOwed(int64 TransactionValue) const;

    /**
     * Calculate security tax specifically
     * @param TransactionValue The value being taxed
     * @return Security tax amount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Taxation")
    int64 CalculateSecurityTax(int64 TransactionValue) const;

    // ====================
    // Budget Functions
    // ====================

    /**
     * Calculate allocated security budget from current treasury
     * @return Security budget amount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Budget")
    int64 GetSecurityBudget() const;

    /**
     * Calculate allocated infrastructure budget from current treasury
     * @return Infrastructure budget amount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Budget")
    int64 GetInfrastructureBudget() const;

    /**
     * Calculate emergency reserve funds
     * @return Emergency reserve amount
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Budget")
    int64 GetEmergencyReserve() const;

    /**
     * Check if budget allocations add up to 100%
     * @return True if budget is properly allocated
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Budget")
    bool IsBudgetBalanced() const;

    // ====================
    // Policy Functions
    // ====================

    /**
     * Get all active policies
     * @return Array of active policies
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Policies")
    TArray<FSectorPolicy> GetActivePolicies() const;

    /**
     * Get active policy of a specific type
     * @param PolicyType The type of policy to find
     * @param OutPolicy The policy (if found)
     * @return True if policy of this type exists
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Policies")
    bool GetPolicyByType(ESectorPolicyType PolicyType, FSectorPolicy& OutPolicy) const;

    /**
     * Check if a specific policy type is active
     * @param PolicyType The policy type to check
     * @return True if policy is active
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Policies")
    bool HasActivePolicy(ESectorPolicyType PolicyType) const;

    // ====================
    // Voting Functions
    // ====================

    /**
     * Simulate a council vote on a proposed policy
     * @param ProposedPolicy The policy being voted on
     * @param WaysInFavor Ways that vote in favor
     * @param WaysAgainst Ways that vote against
     * @return Vote result structure
     */
    UFUNCTION(BlueprintCallable, Category="Sector Council|Voting")
    FCouncilVote SimulateVote(const FSectorPolicy& ProposedPolicy, 
        const TArray<UWayDataAsset*>& WaysInFavor, 
        const TArray<UWayDataAsset*>& WaysAgainst) const;

    /**
     * Check if a vote would pass with given support
     * @param VotesFor Weighted votes in favor
     * @param VotesAgainst Weighted votes against
     * @return True if vote meets pass threshold
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Sector Council|Voting")
    bool WouldVotePass(int32 VotesFor, int32 VotesAgainst) const;
};
