#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Way.h"
#include "WayNetwork.generated.h"

/**
 * Structure defining the relationship between a Way and its Network.
 * Tracks influence level and commitment to network goals.
 */
USTRUCT(BlueprintType)
struct FNetworkMember
{
    GENERATED_BODY()

    /** The Way that is part of this network */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Network Member")
    UWayDataAsset* MemberWay;

    /** 
     * Influence level within the network (0-100)
     * Represents how much weight this Way has in network decisions
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Network Member", meta=(ClampMin="0", ClampMax="100"))
    int32 InfluenceLevel;

    /** 
     * Commitment to network goals (0-100)
     * Affects how strongly this Way responds to network-wide events
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Network Member", meta=(ClampMin="0", ClampMax="100"))
    int32 CommitmentLevel;

    /** Optional: Role description within the network */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Network Member", meta=(MultiLine=true))
    FText RoleDescription;

    FNetworkMember()
        : MemberWay(nullptr)
        , InfluenceLevel(50)
        , CommitmentLevel(50)
        , RoleDescription(FText::FromString(TEXT("Network member")))
    {}
};

/**
 * Data Asset representing a WayNetwork - a micro-alliance of Ways with aligned goals.
 * 
 * Networks form when multiple small guilds/schools/syndicates share common Precepts
 * and decide to work together. This creates emergent political structures where
 * players who earn reputation with one member gain partial reputation with the network.
 * 
 * Design Philosophy:
 * - Small, focused networks (2-5 Ways typical)
 * - Based on Precept alignment (not just convenience)
 * - Dynamic reputation spillover effects
 * - Networks can have their own collective goals
 * - Player actions affect entire network when significant
 * 
 * Usage Examples:
 * - "The Craftsmen's Compact" - Engineers' Guild, Ship-Builders Union, Refinery Collective
 *   Shared Precepts: Craftsmanship, Mastery, Innovation
 * 
 * - "The Frontier Alliance" - Scout's League, Cartographer's Society, Pioneer Traders
 *   Shared Precepts: Discovery, Freedom, Survival
 * 
 * - "The Honor Circle" - Warrior's Guild, Duelist Academy, Protector's Order
 *   Shared Precepts: Honor, Strength, Justice
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on UWayNetworkDataAsset
 * - Add 2-5 member Ways with shared Precepts
 * - Configure network-wide bonuses and reputation spillover
 * - Reference in VerseSubsystem for reputation calculations
 */
UCLASS(BlueprintType)
class ADASTREA_API UWayNetworkDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UWayNetworkDataAsset();

    // ====================
    // Core Identity
    // ====================

    /** Display name of this network (e.g., "The Craftsmen's Compact", "The Frontier Alliance") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText NetworkName;

    /** Description of the network's purpose and goals */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
    FText Description;

    /** Unique identifier for this network */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName NetworkID;

    /** Primary color for UI representation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FLinearColor NetworkColor;

    // ====================
    // Member Ways
    // ====================

    /** Ways that are part of this network */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Members")
    TArray<FNetworkMember> MemberWays;

    // ====================
    // Shared Philosophy
    // ====================

    /** 
     * Core Precepts that unite this network
     * These should be shared by most/all member Ways
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Philosophy")
    TArray<FPreceptValue> SharedPrecepts;

    // ====================
    // Network Mechanics
    // ====================

    /**
     * Reputation spillover percentage (0-100)
     * When player gains reputation with one member, this % applies to all network members
     * Example: 30 = gaining 100 rep with one Way grants 30 rep to other network members
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mechanics", meta=(ClampMin="0", ClampMax="100"))
    int32 ReputationSpilloverPercent;

    /**
     * Network reputation multiplier for feats aligning with SharedPrecepts
     * When player completes a Feat matching network's shared Precepts, multiply rep gain
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mechanics", meta=(ClampMin="1.0", ClampMax="3.0"))
    float NetworkAlignmentBonus;

    /**
     * Minimum individual Way reputation to gain network-wide bonuses
     * Player must have this rep with at least one member to benefit from network effects
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mechanics", meta=(ClampMin="0", ClampMax="100"))
    int32 MinimumReputationThreshold;

    // ====================
    // Network Status
    // ====================

    /** 
     * Whether this network is currently active
     * Networks can form and dissolve based on political changes
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
    bool bIsActive;

    /** Optional: Date when this network was formed (for lore/history) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
    FDateTime FormationDate;

    // ====================
    // Member Query Functions
    // ====================

    /**
     * Get all member Ways in this network
     * @return Array of member Ways
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Members")
    TArray<FNetworkMember> GetMembers() const;

    /**
     * Check if a specific Way is part of this network
     * @param Way The Way to check for
     * @return True if Way is a network member
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Members")
    bool IsMember(const UWayDataAsset* Way) const;

    /**
     * Get the member count for this network
     * @return Number of Ways in the network
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Members")
    int32 GetMemberCount() const;

    /**
     * Get member with highest influence in the network
     * @param OutMember The most influential member
     * @return True if network has members
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Members")
    bool GetMostInfluentialMember(FNetworkMember& OutMember) const;

    /**
     * Get influence level of a specific Way in the network
     * @param Way The Way to query
     * @return Influence level (0-100), returns 0 if not a member
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Members")
    int32 GetMemberInfluence(const UWayDataAsset* Way) const;

    // ====================
    // Precept Alignment Functions
    // ====================

    /**
     * Get all shared Precepts for this network
     * @return Array of Precept values
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Precepts")
    TArray<FPreceptValue> GetSharedPrecepts() const;

    /**
     * Check if this network values a specific Precept
     * @param Precept The Precept to check for
     * @return True if network has the Precept in shared values
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Precepts")
    bool ValuesSharedPrecept(EPrecept Precept) const;

    /**
     * Calculate alignment score between this network and a Feat
     * @param FeatAlignments Precept alignments from a Feat
     * @return Alignment score based on shared Precepts
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Precepts")
    float CalculateNetworkAlignment(const TArray<FFeatPreceptAlignment>& FeatAlignments) const;

    // ====================
    // Reputation Functions
    // ====================

    /**
     * Calculate spillover reputation for network members when one member gains rep
     * @param BaseReputationGain Reputation gained by primary Way
     * @return Spillover reputation for other network members
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Reputation")
    int32 CalculateSpilloverReputation(int32 BaseReputationGain) const;

    /**
     * Check if player qualifies for network-wide bonuses
     * @param PlayerReputation Player's reputation with a network member
     * @return True if player meets minimum threshold
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Reputation")
    bool QualifiesForNetworkBonuses(int32 PlayerReputation) const;

    // ====================
    // Helper Functions
    // ====================

    /**
     * Get summary of network's philosophical alignment
     * @return Text describing the network's shared values
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way Network|Utility")
    FText GetPhilosophySummary() const;
};
