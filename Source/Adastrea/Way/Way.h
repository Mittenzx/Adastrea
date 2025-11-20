#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Way.generated.h"

/**
 * Precepts represent the core values and philosophies that define a group's identity.
 * These values determine how a group reacts to player accomplishments (Feats).
 * 
 * Examples:
 * - A scientific school might value Discovery and Innovation
 * - A merchant syndicate might value Prosperity and Cunning
 * - A warrior guild might value Honor and Strength
 */
UENUM(BlueprintType)
enum class EPrecept : uint8
{
    // Virtue and Moral Values
    Honor           UMETA(DisplayName = "Honor"),           // Upholding principles and keeping one's word
    Justice         UMETA(DisplayName = "Justice"),         // Fairness and righteousness
    Compassion      UMETA(DisplayName = "Compassion"),      // Mercy and kindness to others
    Loyalty         UMETA(DisplayName = "Loyalty"),         // Dedication and steadfastness
    
    // Achievement and Excellence
    Mastery         UMETA(DisplayName = "Mastery"),         // Pursuit of skill perfection
    Innovation      UMETA(DisplayName = "Innovation"),      // Creating new solutions
    Discovery       UMETA(DisplayName = "Discovery"),       // Uncovering hidden knowledge
    Craftsmanship   UMETA(DisplayName = "Craftsmanship"),   // Excellence in creation
    
    // Power and Influence
    Strength        UMETA(DisplayName = "Strength"),        // Physical and military might
    Dominance       UMETA(DisplayName = "Dominance"),       // Control and supremacy
    Cunning         UMETA(DisplayName = "Cunning"),         // Strategic thinking and cleverness
    Ambition        UMETA(DisplayName = "Ambition"),        // Drive to rise and succeed
    
    // Community and Relationships
    Unity           UMETA(DisplayName = "Unity"),           // Togetherness and cooperation
    Freedom         UMETA(DisplayName = "Freedom"),         // Liberty and independence
    Tradition       UMETA(DisplayName = "Tradition"),       // Preserving heritage and customs
    Progress        UMETA(DisplayName = "Progress"),        // Advancement and change
    
    // Material and Practical
    Prosperity      UMETA(DisplayName = "Prosperity"),      // Wealth and abundance
    Survival        UMETA(DisplayName = "Survival"),        // Endurance and resilience
    Efficiency      UMETA(DisplayName = "Efficiency"),      // Optimal resource use
    Harmony         UMETA(DisplayName = "Harmony")          // Balance and peace
};

/**
 * Reputation levels representing the player's standing with a Way.
 * These qualitative levels are derived from the player's Verse score with that Way.
 */
UENUM(BlueprintType)
enum class EReputationLevel : uint8
{
    Distrusted      UMETA(DisplayName = "Distrusted"),      // Negative reputation, Way opposes player
    Neutral         UMETA(DisplayName = "Neutral"),         // No strong opinion, neutral standing
    Respected       UMETA(DisplayName = "Respected"),       // Positive reputation, Way respects player
    Trusted         UMETA(DisplayName = "Trusted")          // Excellent reputation, Way trusts player
};

/**
 * Industry categories that Ways can specialize in
 * Each Way focuses on specific trade/craft areas
 */
UENUM(BlueprintType)
enum class EWayIndustry : uint8
{
    Mining          UMETA(DisplayName = "Mining"),          // Extract raw materials (ore, gas, ice)
    Refining        UMETA(DisplayName = "Refining"),        // Process raw materials into usable resources
    Manufacturing   UMETA(DisplayName = "Manufacturing"),   // Craft components and parts
    Shipbuilding    UMETA(DisplayName = "Shipbuilding"),    // Build and repair spaceships
    Trading         UMETA(DisplayName = "Trading"),         // Buy and sell goods, merchant services
    Exploration     UMETA(DisplayName = "Exploration"),     // Survey, map, and discover new areas
    Research        UMETA(DisplayName = "Research"),        // Develop new technologies and knowledge
    Agriculture     UMETA(DisplayName = "Agriculture"),     // Produce food and organic materials
    Medical         UMETA(DisplayName = "Medical"),         // Healthcare and pharmaceutical services
    Security        UMETA(DisplayName = "Security"),        // Protection, escort, and defense services
    Salvage         UMETA(DisplayName = "Salvage"),         // Recovery and recycling of materials
    Entertainment   UMETA(DisplayName = "Entertainment"),   // Leisure and cultural services
    Transport       UMETA(DisplayName = "Transport"),       // Cargo hauling and passenger services
    Engineering     UMETA(DisplayName = "Engineering"),     // Technical services and maintenance
    Custom          UMETA(DisplayName = "Custom")           // Unique specialty (define in description)
};

/**
 * Quality tier for products/services provided by a Way
 * Affects pricing, reputation, and customer satisfaction
 */
UENUM(BlueprintType)
enum class EQualityTier : uint8
{
    Basic           UMETA(DisplayName = "Basic"),           // Cheap, functional, gets the job done
    Standard        UMETA(DisplayName = "Standard"),        // Average quality, fair price
    Quality         UMETA(DisplayName = "Quality"),         // Above average, reliable
    Premium         UMETA(DisplayName = "Premium"),         // High quality, expensive
    Elite           UMETA(DisplayName = "Elite"),           // Exceptional quality, very expensive
    Legendary       UMETA(DisplayName = "Legendary")        // Best in class, priceless reputation
};

/**
 * Structure defining how strongly a group values a particular Precept.
 * Groups can have multiple Precepts with varying importance levels.
 */
USTRUCT(BlueprintType)
struct FPreceptValue
{
    GENERATED_BODY()

    /** The Precept this value represents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept")
    EPrecept Precept;

    /** How strongly this group values this Precept (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept", meta=(ClampMin="0", ClampMax="100"))
    int32 ImportanceValue;

    /** Optional description of why this Precept matters to the group */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Precept", meta=(MultiLine=true))
    FText Description;

    FPreceptValue()
        : Precept(EPrecept::Honor)
        , ImportanceValue(50)
        , Description(FText::FromString(TEXT("A core value of this group.")))
    {}
};

/**
 * Structure defining a resource that a Way produces or consumes
 * Used for supply chain and trade mechanics
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FWayResource
{
    GENERATED_BODY()

    /** Name of the resource (e.g., "Iron Ore", "Ship Components", "Medical Supplies") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
    FText ResourceName;

    /** Unique identifier for this resource type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
    FName ResourceID;

    /** Quantity produced or consumed per cycle (e.g., per day/week) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource", meta=(ClampMin="0"))
    int32 Quantity;

    /** Quality tier of this resource */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
    EQualityTier QualityLevel;

    /** Whether this is a produced resource (true) or consumed resource (false) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource")
    bool bIsProduced;

    /** Optional notes about this resource */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource", meta=(MultiLine=true))
    FText ResourceNotes;

    FWayResource()
        : ResourceName(FText::FromString(TEXT("Unknown Resource")))
        , ResourceID(NAME_None)
        , Quantity(100)
        , QualityLevel(EQualityTier::Standard)
        , bIsProduced(true)
        , ResourceNotes(FText::GetEmpty())
    {}
};

/**
 * Structure defining a trade relationship/supply chain between Ways
 * Example: Mining Way supplies ore to Crafting Way
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FSupplyChainLink
{
    GENERATED_BODY()

    /** The Way that supplies resources */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain")
    UWayDataAsset* SupplierWay;

    /** The Way that receives/consumes resources */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain")
    UWayDataAsset* ConsumerWay;

    /** Resource being supplied */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain")
    FName ResourceID;

    /** Quantity supplied per cycle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain", meta=(ClampMin="0"))
    int32 SupplyQuantity;

    /** Price per unit (can be negotiated) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain", meta=(ClampMin="0"))
    int32 PricePerUnit;

    /** Whether this is an exclusive supply contract */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain")
    bool bIsExclusive;

    /** Contract duration (in game days, 0 = indefinite) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Supply Chain", meta=(ClampMin="0"))
    int32 ContractDuration;

    FSupplyChainLink()
        : SupplierWay(nullptr)
        , ConsumerWay(nullptr)
        , ResourceID(NAME_None)
        , SupplyQuantity(100)
        , PricePerUnit(10)
        , bIsExclusive(false)
        , ContractDuration(0)
    {}
};

/**
 * Business relationship between two Ways
 * Defines trade agreements, partnerships, and operational relationships
 */
USTRUCT(BlueprintType)
struct ADASTREA_API FWayRelationship
{
    GENERATED_BODY()

    /** The Way this relationship is with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships")
    FName TargetWayID;

    /** Relationship status (-100 to 100, negative is competitive, positive is cooperative) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships", meta=(ClampMin="-100", ClampMax="100"))
    int32 RelationshipValue;

    /** Whether there is an active partnership agreement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships")
    bool bHasPartnership;

    /** Whether there is an active business conflict/rivalry */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships")
    bool bInCompetition;

    /** Trade agreement multiplier (1.0 = normal, higher = better rates) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships", meta=(ClampMin="0.0", ClampMax="3.0"))
    float TradeModifier;

    /** Whether this Way shares resources with the target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships")
    bool bSharesResources;

    /** Optional notes about the business relationship */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationships", meta=(MultiLine=true))
    FText RelationshipNotes;

    FWayRelationship()
        : TargetWayID(NAME_None)
        , RelationshipValue(0)
        , bHasPartnership(false)
        , bInCompetition(false)
        , TradeModifier(1.0f)
        , bSharesResources(false)
        , RelationshipNotes(FText::GetEmpty())
    {}
};

/**
 * Data Asset representing a Way - the identity and philosophy of a group.
 * A Way defines who a group is (School or Syndicate) and what they value (Precepts).
 * 
 * Schools: Organizations focused on knowledge, skill, or craft (e.g., Engineers' Guild, Star-Navigators Academy)
 * Syndicates: Organizations focused on commerce, influence, or territory (e.g., Merchant Coalition, Mining Cartel)
 * 
 * Usage:
 * - Create a Blueprint Data Asset based on UWayDataAsset
 * - Configure the group's identity and core Precepts
 * - Use in gameplay to determine how the group reacts to player Feats
 * - Higher Precept values mean stronger reactions to related accomplishments
 * 
 * Example:
 * - "The Stellar Engineers" school values Craftsmanship (90), Innovation (80), Mastery (70)
 * - When player earns "Master Ship-Builder" title, this group responds positively
 * - Groups with different Precepts might ignore or even disapprove of the same title
 */
UCLASS(BlueprintType)
class ADASTREA_API UWayDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UWayDataAsset();

    // ====================
    // Core Identity
    // ====================

    /** Display name of this Way (e.g., "The Stellar Engineers", "Nebula Trade Syndicate") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText WayName;

    /** Description of this group's purpose and philosophy */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity", meta=(MultiLine=true))
    FText Description;

    /** Unique identifier for this Way */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName WayID;

    /** Whether this is a School (knowledge/skill focused) or Syndicate (commerce/influence focused) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    bool bIsSchool;

    /** Primary color for UI representation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FLinearColor PrimaryColor;

    /** Secondary color for UI representation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FLinearColor SecondaryColor;

    // ====================
    // Organizational Attributes
    // ====================

    /** Technology level of this Way (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 TechnologyLevel;

    /** Military/Combat strength of this Way (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 MilitaryStrength;

    /** Economic power/wealth of this Way (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 EconomicPower;

    /** Influence/political power of this Way (1-10) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 InfluencePower;

    /** Territory sectors controlled by this Way */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Territory")
    TArray<FName> ControlledSectors;

    /** Home base or headquarters location */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Territory")
    FName HomeBaseID;

    // ====================
    // Trade Specialization
    // ====================

    /** Primary industry this Way specializes in */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Specialization")
    EWayIndustry PrimaryIndustry;

    /** Secondary industries (if any) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Specialization")
    TArray<EWayIndustry> SecondaryIndustries;

    /** Overall quality tier for this Way's products/services */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Specialization")
    EQualityTier QualityReputation;

    /** Detailed description of what this Way does */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Specialization", meta=(MultiLine=true))
    FText SpecializationDescription;

    /** Resources this Way produces */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade")
    TArray<FWayResource> ProducedResources;

    /** Resources this Way consumes/requires */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade")
    TArray<FWayResource> ConsumedResources;

    /** Established supply chain relationships */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trade")
    TArray<FSupplyChainLink> SupplyChains;

    /** Approximate member count of this Way */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Organization", meta=(ClampMin="1", ClampMax="10000"))
    int32 MemberCount;

    // ====================
    // Core Values (Precepts)
    // ====================

    /** The Precepts this group values, determining their reaction to player Feats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Philosophy")
    TArray<FPreceptValue> CorePrecepts;

    // ====================
    // Business Relationships
    // ====================

    /** Relationships with other Ways (trade agreements, partnerships, competitions) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Relationships")
    TArray<FWayRelationship> WayRelationships;

    // ====================
    // Sector Governance Participation
    // ====================

    /** 
     * Whether this Way participates in sector council governance
     * Ways with presence in a sector automatically get representation
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Governance")
    bool bParticipatesInCouncils;

    /**
     * Default voting weight for this Way's representative in councils (0-100)
     * Based on the Way's influence, member count, and resources
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Governance", meta=(ClampMin="0", ClampMax="100"))
    int32 BaseVotingWeight;

    /**
     * Sectors where this Way has significant presence and council representation
     * Automatically grants a council seat in these sectors
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Governance")
    TArray<FName> RepresentedSectors;

    // ====================
    // Precept Query Functions
    // ====================

    /**
     * Get all Precepts valued by this Way
     * @return Array of Precept values
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    const TArray<FPreceptValue>& GetPrecepts() const;

    /**
     * Check if this Way values a specific Precept
     * @param Precept The Precept to check for
     * @return True if this Way has the Precept defined
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    bool HasPrecept(EPrecept Precept) const;

    /**
     * Get the importance value for a specific Precept
     * @param Precept The Precept to query
     * @return Importance value (0-100), returns 0 if Precept not valued
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    int32 GetPreceptValue(EPrecept Precept) const;

    /**
     * Get the most highly valued Precept for this Way
     * @param OutPrecept The most valued Precept
     * @return True if Way has any Precepts defined
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    bool GetPrimaryPrecept(EPrecept& OutPrecept) const;

    /**
     * Get all Precepts with importance above a threshold
     * @param Threshold Minimum importance value to include
     * @return Array of Precept values meeting the threshold
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    TArray<FPreceptValue> GetPreceptsAboveThreshold(int32 Threshold) const;

    // ====================
    // Helper Functions
    // ====================

    /**
     * Get display name for a Precept enum value
     * @param Precept The Precept to get name for
     * @return Localized display name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Utility")
    static FText GetPreceptDisplayName(EPrecept Precept);

    /**
     * Get description for a Precept enum value
     * @param Precept The Precept to get description for
     * @return Localized description explaining the Precept
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Utility")
    static FText GetPreceptDescription(EPrecept Precept);

    // ====================
    // Relationship Query Functions
    // ====================

    /**
     * Get all relationships this Way has with other Ways
     * @return Array of Way relationships
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    TArray<FWayRelationship> GetRelationships() const;

    /**
     * Get relationship with a specific Way
     * @param OtherWayID The Way to get relationship with
     * @param OutRelationship The relationship data (if found)
     * @return True if relationship exists
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    bool GetRelationship(FName OtherWayID, FWayRelationship& OutRelationship) const;

    /**
     * Check if this Way has a partnership with another
     * @param OtherWayID The Way to check partnership with
     * @return True if partnered
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    bool HasPartnership(FName OtherWayID) const;

    /**
     * Check if this Way is in competition with another
     * @param OtherWayID The Way to check competition with
     * @return True if in competition
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    bool IsCompeting(FName OtherWayID) const;

    /**
     * Get the relationship value with another Way
     * @param OtherWayID The Way to get relationship value with
     * @return Relationship value (-100 to 100), returns 0 if no relationship exists
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    int32 GetRelationshipValue(FName OtherWayID) const;

    /**
     * Get trade modifier with another Way
     * @param OtherWayID The Way to get trade modifier with
     * @return Trade modifier (1.0 = normal), returns 1.0 if no relationship exists
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Relationships")
    float GetTradeModifier(FName OtherWayID) const;

    // ====================
    // Governance Functions
    // ====================

    /**
     * Check if this Way has representation in a specific sector
     * @param SectorID The sector to check
     * @return True if Way is represented in the sector council
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Governance")
    bool IsRepresentedInSector(FName SectorID) const;

    /**
     * Get all sectors where this Way has council representation
     * @return Array of sector IDs
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Governance")
    TArray<FName> GetRepresentedSectors() const;

    /**
     * Get this Way's voting weight for council decisions
     * @return Base voting weight (0-100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Governance")
    int32 GetVotingWeight() const;

    // ====================
    // Trade Specialization Functions
    // ====================

    /**
     * Get the primary industry this Way specializes in
     * @return Primary industry enum
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Specialization")
    EWayIndustry GetPrimaryIndustry() const;

    /**
     * Get all industries this Way operates in (primary + secondary)
     * @return Array of industries
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Specialization")
    TArray<EWayIndustry> GetAllIndustries() const;

    /**
     * Check if this Way operates in a specific industry
     * @param Industry The industry to check
     * @return True if Way operates in this industry
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Specialization")
    bool OperatesInIndustry(EWayIndustry Industry) const;

    /**
     * Get this Way's quality reputation
     * @return Quality tier enum
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Specialization")
    EQualityTier GetQualityReputation() const;

    /**
     * Get all resources this Way produces
     * @return Array of produced resources
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    TArray<FWayResource> GetProducedResources() const;

    /**
     * Get all resources this Way consumes
     * @return Array of consumed resources
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    TArray<FWayResource> GetConsumedResources() const;

    /**
     * Check if this Way produces a specific resource
     * @param ResourceID The resource to check
     * @return True if Way produces this resource
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    bool ProducesResource(FName ResourceID) const;

    /**
     * Check if this Way consumes a specific resource
     * @param ResourceID The resource to check
     * @return True if Way consumes this resource
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    bool ConsumesResource(FName ResourceID) const;

    /**
     * Get production quantity for a specific resource
     * @param ResourceID The resource to query
     * @return Quantity produced per cycle, 0 if not produced
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    int32 GetProductionQuantity(FName ResourceID) const;

    /**
     * Get consumption quantity for a specific resource
     * @param ResourceID The resource to query
     * @return Quantity consumed per cycle, 0 if not consumed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    int32 GetConsumptionQuantity(FName ResourceID) const;

    /**
     * Get all supply chain links for this Way (both as supplier and consumer)
     * @return Array of supply chain relationships
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    TArray<FSupplyChainLink> GetSupplyChains() const;

    /**
     * Get Ways that supply resources to this Way
     * @return Array of supplier Ways
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    TArray<UWayDataAsset*> GetSuppliers() const;

    /**
     * Get Ways that this Way supplies resources to
     * @return Array of customer Ways
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    TArray<UWayDataAsset*> GetCustomers() const;

    /**
     * Check if this Way has a supply relationship with another Way
     * @param OtherWay The Way to check
     * @return True if there's a supply chain connection
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Trade")
    bool HasSupplyRelationship(const UWayDataAsset* OtherWay) const;

    /**
     * Get member count of this Way
     * @return Number of members
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Organization")
    int32 GetMemberCount() const;

    /**
     * Get display name for an industry type
     * @param Industry The industry to get name for
     * @return Localized display name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Utility")
    static FText GetIndustryDisplayName(EWayIndustry Industry);

    /**
     * Get display name for a quality tier
     * @param Quality The quality tier to get name for
     * @return Localized display name
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Utility")
    static FText GetQualityDisplayName(EQualityTier Quality);
};
