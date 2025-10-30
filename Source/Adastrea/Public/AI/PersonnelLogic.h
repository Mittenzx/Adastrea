#pragma once

#include "CoreMinimal.h"
#include "AI/NPCLogicBase.h"
#include "Characters/PersonnelDataAsset.h"
#include "PersonnelLogic.generated.h"

/**
 * Personnel task priority for individual AI decisions
 */
UENUM(BlueprintType)
enum class EPersonnelTask : uint8
{
    Emergency UMETA(DisplayName = "Emergency"),        // Life-threatening situations
    DutyStation UMETA(DisplayName = "Duty Station"),   // Primary job responsibilities
    Maintenance UMETA(DisplayName = "Maintenance"),    // Upkeep and repairs
    Social UMETA(DisplayName = "Social"),              // Crew interactions
    Personal UMETA(DisplayName = "Personal"),          // Self-care, rest, recreation
    Training UMETA(DisplayName = "Training"),          // Skill improvement
    Exploration UMETA(DisplayName = "Exploration")     // Discovery activities
};

/**
 * Disposition type affects how personnel react to situations
 */
UENUM(BlueprintType)
enum class EPersonnelDisposition : uint8
{
    Cautious UMETA(DisplayName = "Cautious"),          // Risk-averse, methodical
    Bold UMETA(DisplayName = "Bold"),                  // Risk-taking, decisive
    Analytical UMETA(DisplayName = "Analytical"),      // Logical, calculated
    Empathetic UMETA(DisplayName = "Empathetic"),      // People-focused, supportive
    Pragmatic UMETA(DisplayName = "Pragmatic"),        // Results-oriented, efficient
    Curious UMETA(DisplayName = "Curious"),            // Inquisitive, exploratory
    Disciplined UMETA(DisplayName = "Disciplined"),    // By-the-book, orderly
    Creative UMETA(DisplayName = "Creative")           // Innovative, unconventional
};

/**
 * Personnel Logic - Handles micro-level AI for individual personnel/crew
 * 
 * This class manages individual personnel behavior, including:
 * - Daily task prioritization
 * - Personality-driven decision making
 * - Social interactions with other crew
 * - Skill development and training
 * - Morale and well-being management
 * 
 * Designed to work with PersonnelDataAsset for data-driven character AI
 * 
 * Blueprint Usage:
 * 1. Create a Blueprint based on UPersonnelLogic
 * 2. Assign a PersonnelDataAsset reference
 * 3. Configure disposition and personality
 * 4. Override Event functions to customize behavior
 * 5. Add to character actors or management systems
 */
UCLASS(Blueprintable, BlueprintType)
class ADASTREA_API UPersonnelLogic : public UNPCLogicBase
{
    GENERATED_BODY()

public:
    UPersonnelLogic();

    // ====================
    // Personnel-Specific Properties
    // ====================

    /** Reference to the personnel's data asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personnel|Data")
    UPersonnelDataAsset* PersonnelData;

    /** Personality disposition affecting decision making */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personnel|Personality")
    EPersonnelDisposition Disposition;

    /** Current task being performed */
    UPROPERTY(BlueprintReadOnly, Category = "Personnel|State")
    EPersonnelTask CurrentTask;

    /** Whether personnel is currently on duty */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|State")
    bool bOnDuty;

    /** Whether personnel is currently resting/sleeping */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|State")
    bool bIsResting;

    /** Current location/station assignment */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|Assignment")
    FString CurrentLocation;

    /** Time until next scheduled duty shift (in hours) */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|Schedule", meta = (ClampMin = "0", ClampMax = "24"))
    float HoursUntilNextShift;

    /** Time until current task completes (in hours) */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|Task", meta = (ClampMin = "0"))
    float HoursUntilTaskComplete;

    /** List of personnel IDs this person is friends with */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|Social")
    TArray<FName> FriendsList;

    /** List of personnel IDs this person has conflicts with */
    UPROPERTY(BlueprintReadWrite, Category = "Personnel|Social")
    TArray<FName> ConflictList;

    // ====================
    // BlueprintNativeEvent Hooks - Personnel-Specific
    // ====================

    /**
     * Evaluate what task the personnel should prioritize
     * Override to implement custom task selection logic
     * 
     * @return The task type to focus on
     * 
     * Example Blueprint usage:
     * - Check health/fatigue for Personal needs
     * - Check duty status for DutyStation
     * - Check social needs for Social interactions
     * - Consider personality when choosing
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Tasks")
    EPersonnelTask EvaluateTaskPriority();
    virtual EPersonnelTask EvaluateTaskPriority_Implementation();

    /**
     * Handle interaction with another crew member
     * Override to customize social behavior
     * 
     * @param OtherPersonnelID The person to interact with
     * @param InteractionType Type of interaction (chat, collaborate, etc.)
     * 
     * Example Blueprint usage:
     * - Check relationship with other person
     * - Consider disposition (Empathetic = more social)
     * - Update morale based on interaction
     * - Potentially update relationship
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Social")
    void HandleSocialInteraction(FName OtherPersonnelID, const FString& InteractionType);
    virtual void HandleSocialInteraction_Implementation(FName OtherPersonnelID, const FString& InteractionType);

    /**
     * Called when personnel needs to make a decision
     * Override to implement personality-driven decision making
     * 
     * @param DecisionContext Description of the decision to make
     * @param Options Array of possible choices
     * @return Index of chosen option (-1 if none)
     * 
     * Example Blueprint usage:
     * - Consider disposition (Cautious = safe option)
     * - Check morale/loyalty for risk-taking
     * - Factor in skills for competence decisions
     * - Use traits to modify preferences
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Decision")
    int32 MakePersonalityDrivenDecision(const FString& DecisionContext, const TArray<FString>& Options);
    virtual int32 MakePersonalityDrivenDecision_Implementation(const FString& DecisionContext, const TArray<FString>& Options);

    /**
     * Called when personnel completes a task
     * Override to handle task completion and skill gains
     * 
     * @param CompletedTask The task that was completed
     * @param Success Whether the task was successful
     * 
     * Example Blueprint usage:
     * - Grant experience points
     * - Update morale based on success
     * - Improve relevant skills
     * - Check for promotions/recognition
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Tasks")
    void OnTaskCompleted(EPersonnelTask CompletedTask, bool Success);
    virtual void OnTaskCompleted_Implementation(EPersonnelTask CompletedTask, bool Success);

    /**
     * Update personnel daily routine
     * Override to customize daily behavior patterns
     * 
     * Example Blueprint usage:
     * - Check time of day for sleep schedule
     * - Manage duty shift transitions
     * - Handle meal times
     * - Schedule social activities during off-hours
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Schedule")
    void UpdateDailyRoutine();
    virtual void UpdateDailyRoutine_Implementation();

    /**
     * Handle personnel stress and morale changes
     * Override to customize stress response
     * 
     * @param StressAmount Amount of stress to add/remove (-100 to 100)
     * 
     * Example Blueprint usage:
     * - Update morale based on stress
     * - Check for burnout conditions
     * - Apply trait modifiers (some handle stress better)
     * - Trigger rest needs if too stressed
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Wellbeing")
    void HandleStressChange(float StressAmount);
    virtual void HandleStressChange_Implementation(float StressAmount);

    /**
     * Evaluate if personnel should pursue personal development
     * Override to customize skill training behavior
     * 
     * @return True if should pursue training/development
     * 
     * Example Blueprint usage:
     * - Check if skills below role requirements
     * - Consider Curious/Analytical disposition (more likely)
     * - Check available free time
     * - Factor in career ambitions
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Personnel|Development")
    bool ShouldPursueTraining();
    virtual bool ShouldPursueTraining_Implementation();

    // ====================
    // Blueprint-Callable Utility Functions
    // ====================

    /**
     * Check if personnel is friends with another person
     * @param OtherPersonnelID The person to check
     * @return True if they are friends
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Social")
    bool IsFriendsWith(FName OtherPersonnelID) const;

    /**
     * Check if personnel has conflict with another person
     * @param OtherPersonnelID The person to check
     * @return True if they have a conflict
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Social")
    bool HasConflictWith(FName OtherPersonnelID) const;

    /**
     * Add someone to friends list
     * @param OtherPersonnelID The person to befriend
     */
    UFUNCTION(BlueprintCallable, Category = "Personnel|Social")
    void MakeFriend(FName OtherPersonnelID);

    /**
     * Remove someone from friends list
     * @param OtherPersonnelID The person to unfriend
     */
    UFUNCTION(BlueprintCallable, Category = "Personnel|Social")
    void RemoveFriend(FName OtherPersonnelID);

    /**
     * Add someone to conflict list
     * @param OtherPersonnelID The person to add conflict with
     */
    UFUNCTION(BlueprintCallable, Category = "Personnel|Social")
    void AddConflict(FName OtherPersonnelID);

    /**
     * Remove someone from conflict list
     * @param OtherPersonnelID The person to resolve conflict with
     */
    UFUNCTION(BlueprintCallable, Category = "Personnel|Social")
    void ResolveConflict(FName OtherPersonnelID);

    /**
     * Check if personnel needs rest
     * @return True if fatigue is high or morale is low
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Wellbeing")
    bool NeedsRest() const;

    /**
     * Check if personnel is fit for duty
     * @return True if health and morale are adequate
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Status")
    bool IsFitForDuty() const;

    /**
     * Check if personnel is suited for a specific task based on disposition
     * @param Task The task to check
     * @return True if disposition is compatible with task
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Tasks")
    bool IsSuitedForTask(EPersonnelTask Task) const;

    /**
     * Get personnel name from data asset
     * @return Personnel name or "Unknown" if no data asset
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Info")
    FText GetPersonnelName() const;

    /**
     * Get a description of current personnel state
     * @return Text description of personnel's current state and activity
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Info")
    FString GetPersonnelStateDescription() const;

    /**
     * Calculate how personality affects a given modifier
     * @param BaseModifier The base modifier value
     * @return Modified value based on disposition
     */
    UFUNCTION(BlueprintPure, Category = "Personnel|Personality")
    float ApplyPersonalityModifier(float BaseModifier) const;

    // ====================
    // Overridden from UNPCLogicBase
    // ====================

    virtual void InitializeAI_Implementation() override;
    virtual void OnTickAI_Implementation(float DeltaTime) override;
    virtual EAIPriority EvaluateCurrentPriority_Implementation() override;
};
