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
    // Core Values (Precepts)
    // ====================

    /** The Precepts this group values, determining their reaction to player Feats */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Philosophy")
    TArray<FPreceptValue> CorePrecepts;

    // ====================
    // Precept Query Functions
    // ====================

    /**
     * Get all Precepts valued by this Way
     * @return Array of Precept values
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Way|Precepts")
    TArray<FPreceptValue> GetPrecepts() const;

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
};
