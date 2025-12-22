#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PersonnelDataAsset.generated.h"

/**
 * Enumeration of all available personnel roles in the game
 */
UENUM(BlueprintType)
enum class EPersonnelRole : uint8
{
    Captain UMETA(DisplayName = "Captain"),
    XO UMETA(DisplayName = "Executive Officer"),
    Pilot UMETA(DisplayName = "Pilot"),
    Navigator UMETA(DisplayName = "Navigator"),
    CommunicationsOfficer UMETA(DisplayName = "Communications Officer"),
    ScienceOfficer UMETA(DisplayName = "Science Officer"),
    Engineer UMETA(DisplayName = "Engineer"),
    MedicalOfficer UMETA(DisplayName = "Medical Officer"),
    SecurityOfficer UMETA(DisplayName = "Security Officer"),
    Quartermaster UMETA(DisplayName = "Quartermaster"),
    Chef UMETA(DisplayName = "Chef"),
    Bartender UMETA(DisplayName = "Bartender"),
    Janitor UMETA(DisplayName = "Janitor"),
    Diplomat UMETA(DisplayName = "Diplomat"),
    AI_Operator UMETA(DisplayName = "AI Operator"),
    Researcher UMETA(DisplayName = "Researcher"),
    SalvageSpecialist UMETA(DisplayName = "Salvage Specialist"),
    Smuggler UMETA(DisplayName = "Smuggler"),
    Entertainment UMETA(DisplayName = "Entertainment"),
    Civilian UMETA(DisplayName = "Civilian"),
    Ambassador UMETA(DisplayName = "Ambassador"),
    CrewMember UMETA(DisplayName = "Crew Member"),
    GunneryChief UMETA(DisplayName = "Gunnery Chief"),
    Gunner UMETA(DisplayName = "Gunner"),
    MiningSpecialist UMETA(DisplayName = "Mining Specialist"),
    Trader UMETA(DisplayName = "Trader"),
    StationManager UMETA(DisplayName = "Station Manager"),
    StationStaff UMETA(DisplayName = "Station Staff"),
    ModuleStaff UMETA(DisplayName = "Module Staff"),
    Other UMETA(DisplayName = "Other")
};

/**
 * Personnel trait structure for unique characteristics and bonuses
 */
USTRUCT(BlueprintType)
struct FPersonnelTrait
{
    GENERATED_BODY()

    // Display name of the trait
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    FText TraitName;

    // Description of the trait's effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait", meta=(MultiLine=true))
    FText TraitDescription;

    // Unique identifier for the trait
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    FName TraitID;

    // Modifier value for gameplay effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trait")
    float ModifierValue;

    FPersonnelTrait()
        : TraitName(FText::FromString(TEXT("Unknown Trait")))
        , TraitDescription(FText::FromString(TEXT("No description available.")))
        , TraitID(NAME_None)
        , ModifierValue(0.0f)
    {}
};

/**
 * Relationship structure between personnel
 */
USTRUCT(BlueprintType)
struct FPersonnelRelationship
{
    GENERATED_BODY()

    // The personnel this relationship is with
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationship")
    FName TargetPersonnelID;

    // Type of relationship (Friend, Rival, Mentor, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationship")
    FString RelationshipType;

    // Relationship strength (-100 to 100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationship", meta=(ClampMin="-100", ClampMax="100"))
    int32 RelationshipStrength;

    // Additional notes about the relationship
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Relationship", meta=(MultiLine=true))
    FText Notes;

    FPersonnelRelationship()
        : TargetPersonnelID(NAME_None)
        , RelationshipType(TEXT("Neutral"))
        , RelationshipStrength(0)
        , Notes(FText::GetEmpty())
    {}
};

/**
 * Performance metric structure for tracking personnel effectiveness
 */
USTRUCT(BlueprintType)
struct FPerformanceMetric
{
    GENERATED_BODY()

    // Name of the metric
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance")
    FString MetricName;

    // Current value of the metric (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance", meta=(ClampMin="0", ClampMax="100"))
    float Value;

    // Historical trend (positive = improving, negative = declining)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Performance")
    float Trend;

    FPerformanceMetric()
        : MetricName(TEXT("Unknown"))
        , Value(50.0f)
        , Trend(0.0f)
    {}
};

/**
 * Past assignment record structure
 */
USTRUCT(BlueprintType)
struct FPastAssignment
{
    GENERATED_BODY()

    // Name of the ship, station, or organization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assignment")
    FString AssignmentLocation;

    // Role held during this assignment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assignment")
    FString RoleHeld;

    // Duration in months
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assignment")
    int32 DurationMonths;

    // Performance rating (1-5 stars)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assignment", meta=(ClampMin="1", ClampMax="5"))
    int32 PerformanceRating;

    // Additional notes or achievements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assignment", meta=(MultiLine=true))
    FText Notes;

    FPastAssignment()
        : AssignmentLocation(TEXT("Unknown"))
        , RoleHeld(TEXT("Unknown"))
        , DurationMonths(0)
        , PerformanceRating(3)
        , Notes(FText::GetEmpty())
    {}
};

/**
 * Skill structure for personnel abilities
 */
USTRUCT(BlueprintType)
struct FPersonnelSkill
{
    GENERATED_BODY()

    // Name of the skill
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
    FString SkillName;

    // Skill level (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill", meta=(ClampMin="1", ClampMax="10"))
    int32 SkillLevel;

    // Experience points in this skill
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
    int32 ExperiencePoints;

    FPersonnelSkill()
        : SkillName(TEXT("Unknown"))
        , SkillLevel(1)
        , ExperiencePoints(0)
    {}
};

/**
 * Data Asset for storing personnel/crew member information.
 * This allows designers to create personnel configurations as Blueprint Data Assets.
 */
UCLASS(BlueprintType)
class ADASTREA_API UPersonnelDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ====================
    // Basic Identity
    // ====================

    // Display name of the personnel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    FText PersonnelName;

    // Unique identifier for the personnel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    FName PersonnelID;

    // Biography and background
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity", meta=(MultiLine=true))
    FText Biography;

    // Age of the personnel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity", meta=(ClampMin="18", ClampMax="200"))
    int32 Age;

    // Gender
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    FString Gender;

    // Species/Race
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    FString Species;

    // Nationality or origin
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    FString Nationality;

    // Custom tags for filtering and search
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Identity")
    TArray<FString> Tags;

    // ====================
    // Role and Assignment
    // ====================

    // Primary role of the personnel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Role")
    EPersonnelRole PrimaryRole;

    // Current assignment (ship name, station name, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Role")
    FString CurrentAssignment;

    // Department they belong to
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Role")
    FString Department;

    // Manager's personnel ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Role")
    FName ManagerID;

    // Array of direct report personnel IDs
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Role")
    TArray<FName> DirectReports;

    // ====================
    // Skills and Experience
    // ====================

    // Overall skill level (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Skills", meta=(ClampMin="1", ClampMax="10"))
    int32 OverallSkillLevel;

    // Total experience points
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Skills")
    int32 TotalExperience;

    // Detailed skill breakdown
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Skills")
    TArray<FPersonnelSkill> Skills;

    // Areas of specialization
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Skills")
    TArray<FString> Specialties;

    // ====================
    // Status and Condition
    // ====================

    // Current morale (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Status", meta=(ClampMin="0", ClampMax="100"))
    float Morale;

    // Current health (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Status", meta=(ClampMin="0", ClampMax="100"))
    float Health;

    // Current fatigue level (0-100, higher = more tired)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Status", meta=(ClampMin="0", ClampMax="100"))
    float Fatigue;

    // Loyalty to player/organization (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Status", meta=(ClampMin="0", ClampMax="100"))
    float Loyalty;

    // Reputation score
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Status", meta=(ClampMin="-100", ClampMax="100"))
    int32 Reputation;

    // ====================
    // Employment
    // ====================

    // Monthly salary
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Employment")
    int32 Salary;

    // Contract duration in months (0 = indefinite)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Employment")
    int32 ContractDuration;

    // Months remaining on contract
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Employment")
    int32 ContractMonthsRemaining;

    // ====================
    // Personality and Traits
    // ====================

    // Personality type or description
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Personality")
    FString PersonalityType;

    // Personality description
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Personality", meta=(MultiLine=true))
    FText PersonalityDescription;

    // Personnel traits that provide unique bonuses or characteristics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Personality")
    TArray<FPersonnelTrait> Traits;

    // ====================
    // Relationships
    // ====================

    // Relationships with other personnel
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Relationships")
    TArray<FPersonnelRelationship> Relationships;

    // ====================
    // Performance and History
    // ====================

    // Performance metrics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Performance")
    TArray<FPerformanceMetric> PerformanceMetrics;

    // Past assignments and career history
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Personnel|Performance")
    TArray<FPastAssignment> PastAssignments;

    UPersonnelDataAsset();

    // ====================
    // Trait System Hooks
    // ====================

    /**
     * Get all traits for this personnel
     * @return Array of personnel traits
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Traits")
    TArray<FPersonnelTrait> GetTraits() const;

    /**
     * Check if personnel has a specific trait
     * @param TraitID The trait identifier to check for
     * @return True if personnel has the trait
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Traits")
    bool HasTrait(FName TraitID) const;

    /**
     * Get a specific trait by ID
     * @param TraitID The trait identifier to retrieve
     * @param OutTrait The found trait (if any)
     * @return True if trait was found
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Traits")
    bool GetTraitByID(FName TraitID, FPersonnelTrait& OutTrait) const;

    /**
     * Get the combined modifier value for a specific trait type
     * @param TraitID The trait identifier to get modifiers for
     * @return Total modifier value from all matching traits
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Traits")
    float GetTraitModifier(FName TraitID) const;

    // ====================
    // Skill System Hooks
    // ====================

    /**
     * Get a specific skill by name
     * @param SkillName The name of the skill to retrieve
     * @param OutSkill The found skill (if any)
     * @return True if skill was found
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Skills")
    bool GetSkillByName(const FString& SkillName, FPersonnelSkill& OutSkill) const;

    /**
     * Get the level of a specific skill
     * @param SkillName The name of the skill
     * @return Skill level (1-10), returns 0 if skill not found
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Skills")
    int32 GetSkillLevel(const FString& SkillName) const;

    /**
     * Check if personnel has a specific specialty
     * @param Specialty The specialty to check for
     * @return True if personnel has the specialty
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Skills")
    bool HasSpecialty(const FString& Specialty) const;

    // ====================
    // Relationship System Hooks
    // ====================

    /**
     * Get relationship with another personnel
     * @param OtherPersonnelID The personnel to get relationship with
     * @param OutRelationship The relationship data (if found)
     * @return True if relationship exists
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Relationships")
    bool GetRelationship(FName OtherPersonnelID, FPersonnelRelationship& OutRelationship) const;

    /**
     * Get the relationship strength with another personnel
     * @param OtherPersonnelID The personnel to get relationship strength with
     * @return Relationship strength (-100 to 100), returns 0 if no relationship exists
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Relationships")
    int32 GetRelationshipStrength(FName OtherPersonnelID) const;

    /**
     * Check if this personnel considers another as a friend
     * @param OtherPersonnelID The personnel to check friendship with
     * @return True if they are friends (positive relationship)
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Relationships")
    bool IsFriendsWith(FName OtherPersonnelID) const;

    /**
     * Check if this personnel considers another as a rival
     * @param OtherPersonnelID The personnel to check rivalry with
     * @return True if they are rivals (negative relationship)
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Relationships")
    bool IsRivalWith(FName OtherPersonnelID) const;

    // ====================
    // Performance System Hooks
    // ====================

    /**
     * Get a specific performance metric by name
     * @param MetricName The name of the metric to retrieve
     * @param OutMetric The found metric (if any)
     * @return True if metric was found
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Performance")
    bool GetPerformanceMetric(const FString& MetricName, FPerformanceMetric& OutMetric) const;

    /**
     * Get the average of all performance metrics
     * @return Average performance value (0-100)
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Performance")
    float GetAveragePerformance() const;

    /**
     * Check if personnel is performing well (average > 70)
     * @return True if performance is good
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Performance")
    bool IsPerformingWell() const;

    // ====================
    // Status System Hooks
    // ====================

    /**
     * Check if personnel is in good condition (health > 70, fatigue < 30)
     * @return True if in good condition
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Status")
    bool IsInGoodCondition() const;

    /**
     * Check if personnel needs rest (fatigue > 70)
     * @return True if needs rest
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Status")
    bool NeedsRest() const;

    /**
     * Check if personnel is loyal (loyalty > 70)
     * @return True if loyal
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Status")
    bool IsLoyal() const;

    /**
     * Check if personnel has low morale (morale < 30)
     * @return True if morale is low
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Status")
    bool HasLowMorale() const;

    // ====================
    // Role System Hooks
    // ====================

    /**
     * Check if personnel is in a leadership role
     * @return True if role is Captain, XO, or Manager type
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Role")
    bool IsInLeadershipRole() const;

    /**
     * Check if personnel is in a combat role
     * @return True if role is combat-related
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Role")
    bool IsInCombatRole() const;

    /**
     * Get the number of direct reports
     * @return Count of direct reports
     */
    UFUNCTION(BlueprintCallable, Category="Personnel|Role")
    int32 GetDirectReportCount() const;

#if WITH_EDITOR
    /**
     * Validate personnel data asset properties
     * Checks for required fields and logical constraints
     */
    virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
