#include "Way/SectorCouncil.h"
#include "Way/Way.h"

USectorCouncilDataAsset::USectorCouncilDataAsset()
{
    // Default identity
    SectorName = FText::FromString(TEXT("Unnamed Sector"));
    SectorID = FName(TEXT("UnknownSector"));
    Description = FText::FromString(TEXT("A sector governed by council representatives from local Ways."));
    
    // Default council settings
    TotalSeats = 10;
    PassThreshold = 51; // 51% needed to pass
    
    // Default tax rates (moderate taxation)
    SecurityTaxRate = 10;
    InfrastructureTaxRate = 5;
    AdministrationTaxRate = 3;
    
    // Default budget allocation
    SecurityBudgetPercent = 50;       // 50% to security
    InfrastructureBudgetPercent = 35; // 35% to infrastructure
    EmergencyReservePercent = 15;     // 15% emergency reserve
    
    TreasuryFunds = 100000; // Starting funds
}

// ====================
// Council Functions
// ====================

TArray<FCouncilRepresentative> USectorCouncilDataAsset::GetRepresentatives() const
{
    return Representatives;
}

bool USectorCouncilDataAsset::GetRepresentativeForWay(const UWayDataAsset* Way, FCouncilRepresentative& OutRepresentative) const
{
    if (!Way)
    {
        return false;
    }
    
    for (const FCouncilRepresentative& Rep : Representatives)
    {
        if (Rep.RepresentingWay == Way)
        {
            OutRepresentative = Rep;
            return true;
        }
    }
    return false;
}

bool USectorCouncilDataAsset::HasRepresentative(const UWayDataAsset* Way) const
{
    if (!Way)
    {
        return false;
    }
    
    for (const FCouncilRepresentative& Rep : Representatives)
    {
        if (Rep.RepresentingWay == Way)
        {
            return true;
        }
    }
    return false;
}

int32 USectorCouncilDataAsset::GetTotalVotingWeight() const
{
    int32 TotalWeight = 0;
    for (const FCouncilRepresentative& Rep : Representatives)
    {
        TotalWeight += Rep.VotingWeight;
    }
    return TotalWeight;
}

// ====================
// Tax Functions
// ====================

int32 USectorCouncilDataAsset::GetTotalTaxRate() const
{
    return SecurityTaxRate + InfrastructureTaxRate + AdministrationTaxRate;
}

int64 USectorCouncilDataAsset::CalculateTaxOwed(int64 TransactionValue) const
{
    if (TransactionValue <= 0)
    {
        return 0;
    }
    
    // Calculate tax as percentage of transaction
    float TaxRate = static_cast<float>(GetTotalTaxRate()) / 100.0f;
    return FMath::RoundToInt64(static_cast<float>(TransactionValue) * TaxRate);
}

int64 USectorCouncilDataAsset::CalculateSecurityTax(int64 TransactionValue) const
{
    if (TransactionValue <= 0)
    {
        return 0;
    }
    
    float TaxRate = static_cast<float>(SecurityTaxRate) / 100.0f;
    return FMath::RoundToInt64(static_cast<float>(TransactionValue) * TaxRate);
}

// ====================
// Budget Functions
// ====================

int64 USectorCouncilDataAsset::GetSecurityBudget() const
{
    float Allocation = static_cast<float>(SecurityBudgetPercent) / 100.0f;
    return FMath::RoundToInt64(static_cast<float>(TreasuryFunds) * Allocation);
}

int64 USectorCouncilDataAsset::GetInfrastructureBudget() const
{
    float Allocation = static_cast<float>(InfrastructureBudgetPercent) / 100.0f;
    return FMath::RoundToInt64(static_cast<float>(TreasuryFunds) * Allocation);
}

int64 USectorCouncilDataAsset::GetEmergencyReserve() const
{
    float Allocation = static_cast<float>(EmergencyReservePercent) / 100.0f;
    return FMath::RoundToInt64(static_cast<float>(TreasuryFunds) * Allocation);
}

bool USectorCouncilDataAsset::IsBudgetBalanced() const
{
    int32 TotalAllocation = SecurityBudgetPercent + InfrastructureBudgetPercent + EmergencyReservePercent;
    return TotalAllocation == 100;
}

// ====================
// Policy Functions
// ====================

TArray<FSectorPolicy> USectorCouncilDataAsset::GetActivePolicies() const
{
    TArray<FSectorPolicy> Active;
    for (const FSectorPolicy& Policy : ActivePolicies)
    {
        if (Policy.bIsActive)
        {
            Active.Add(Policy);
        }
    }
    return Active;
}

bool USectorCouncilDataAsset::GetPolicyByType(ESectorPolicyType PolicyType, FSectorPolicy& OutPolicy) const
{
    for (const FSectorPolicy& Policy : ActivePolicies)
    {
        if (Policy.PolicyType == PolicyType && Policy.bIsActive)
        {
            OutPolicy = Policy;
            return true;
        }
    }
    return false;
}

bool USectorCouncilDataAsset::HasActivePolicy(ESectorPolicyType PolicyType) const
{
    for (const FSectorPolicy& Policy : ActivePolicies)
    {
        if (Policy.PolicyType == PolicyType && Policy.bIsActive)
        {
            return true;
        }
    }
    return false;
}

// ====================
// Voting Functions
// ====================

FCouncilVote USectorCouncilDataAsset::SimulateVote(const FSectorPolicy& ProposedPolicy, 
    const TArray<UWayDataAsset*>& WaysInFavor, 
    const TArray<UWayDataAsset*>& WaysAgainst) const
{
    FCouncilVote Vote;
    Vote.IssueDescription = FText::Format(
        FText::FromString(TEXT("Vote on: {0}")),
        ProposedPolicy.PolicyName
    );
    Vote.ProposedPolicy = ProposedPolicy;
    Vote.VoteDate = FDateTime::Now();
    
    // Count weighted votes for each side
    for (const UWayDataAsset* Way : WaysInFavor)
    {
        if (Way)
        {
            FCouncilRepresentative Rep;
            if (GetRepresentativeForWay(Way, Rep))
            {
                Vote.VotesFor += Rep.VotingWeight;
            }
        }
    }
    
    for (const UWayDataAsset* Way : WaysAgainst)
    {
        if (Way)
        {
            FCouncilRepresentative Rep;
            if (GetRepresentativeForWay(Way, Rep))
            {
                Vote.VotesAgainst += Rep.VotingWeight;
            }
        }
    }
    
    // Calculate abstentions
    int32 TotalWeight = GetTotalVotingWeight();
    Vote.VotesAbstain = TotalWeight - Vote.VotesFor - Vote.VotesAgainst;
    
    // Determine if vote passed
    Vote.bPassed = WouldVotePass(Vote.VotesFor, Vote.VotesAgainst);
    
    return Vote;
}

bool USectorCouncilDataAsset::WouldVotePass(int32 VotesFor, int32 VotesAgainst) const
{
    int32 TotalVotes = VotesFor + VotesAgainst;
    if (TotalVotes == 0)
    {
        return false; // No votes cast
    }
    
    // Calculate percentage of votes in favor
    float PercentFor = (static_cast<float>(VotesFor) / static_cast<float>(TotalVotes)) * 100.0f;
    
    return PercentFor >= static_cast<float>(PassThreshold);
}
