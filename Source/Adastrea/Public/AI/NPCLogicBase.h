#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NPCLogicBase.generated.h"

/**
 * Priority level for AI decision making
 * Higher priority actions take precedence over lower priority ones
 */
UENUM(BlueprintType)
enum class EAIPriority : uint8
{
    Critical UMETA(DisplayName = "Critical"),      // Immediate survival threats
    High UMETA(DisplayName = "High"),              // Important but not life-threatening
    Medium UMETA(DisplayName = "Medium"),          // Standard operations
    Low UMETA(DisplayName = "Low"),                // Optional/background tasks
    Idle UMETA(DisplayName = "Idle")               // When nothing else to do
};

/**
 * AI behavior mode for different gameplay situations
 */
UENUM(BlueprintType)
enum class EAIBehaviorMode : uint8
{
    Peaceful UMETA(DisplayName = "Peaceful"),          // Non-aggressive, exploration focused
    Defensive UMETA(DisplayName = "Defensive"),        // Protect territory/assets
    Aggressive UMETA(DisplayName = "Aggressive"),      // Actively seek conflict
    Trading UMETA(DisplayName = "Trading"),            // Commerce focused
    Exploration UMETA(DisplayName = "Exploration"),    // Discovery and mapping
    Diplomatic UMETA(DisplayName = "Diplomatic"),      // Relationship building
    Resource UMETA(DisplayName = "Resource"),          // Gathering/mining focus
    Stealth UMETA(DisplayName = "Stealth")             // Covert operations
};

/**
 * Base class for all NPC AI logic
 * Provides common functionality and Blueprint extension points for AI behaviors
 * 
 * This class uses BlueprintNativeEvents extensively to allow designers to:
 * 1. Override behavior in Blueprints without C++ knowledge
 * 2. Call parent implementation when needed
 * 3. Mix C++ performance with Blueprint flexibility
 * 
 * Usage in Blueprints:
 * - Create a Blueprint class based on this (or FactionLogic/PersonnelLogic)
 * - Override any "Event" functions (e.g., "Event On Tick AI")
 * - Access all UPROPERTY variables directly
 * - Use Blueprint-callable functions to query state
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ADASTREA_API UNPCLogicBase : public UObject
{
    GENERATED_BODY()

public:
    UNPCLogicBase();

    // ====================
    // Core AI Properties (Blueprint-Editable)
    // ====================

    /** Current AI behavior mode - determines overall strategy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    EAIBehaviorMode CurrentBehaviorMode;

    /** Whether this AI is currently active and processing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
    bool bIsActive;

    /** AI update interval in seconds (performance optimization) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Performance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float UpdateInterval;

    /** Time accumulated since last AI tick */
    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    float TimeSinceLastUpdate;

    // ====================
    // BlueprintNativeEvent Hooks - Core AI Loop
    // These functions can be overridden in Blueprints for custom behavior
    // ====================

    /**
     * Called to initialize the AI logic
     * Override in Blueprint to set up initial state, priorities, etc.
     * 
     * Example Blueprint usage:
     * - Set initial behavior mode
     * - Configure update intervals
     * - Set up references to data assets
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Lifecycle")
    void InitializeAI();
    virtual void InitializeAI_Implementation();

    /**
     * Main AI tick function - called at UpdateInterval rate
     * Override in Blueprint to implement custom AI decision making
     * 
     * @param DeltaTime Time since last tick in seconds
     * 
     * Example Blueprint usage:
     * - Evaluate current priorities
     * - Make decisions based on state
     * - Update behavior mode if needed
     * - Execute actions
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Lifecycle")
    void OnTickAI(float DeltaTime);
    virtual void OnTickAI_Implementation(float DeltaTime);

    /**
     * Called when AI is activated
     * Override in Blueprint for custom activation logic
     * 
     * Example Blueprint usage:
     * - Resume paused behaviors
     * - Re-evaluate priorities
     * - Announce activation to game systems
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Lifecycle")
    void OnActivateAI();
    virtual void OnActivateAI_Implementation();

    /**
     * Called when AI is deactivated
     * Override in Blueprint for custom deactivation logic
     * 
     * Example Blueprint usage:
     * - Store current state
     * - Cancel ongoing actions
     * - Clean up resources
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Lifecycle")
    void OnDeactivateAI();
    virtual void OnDeactivateAI_Implementation();

    /**
     * Called when behavior mode changes
     * Override in Blueprint to react to mode changes
     * 
     * @param OldMode The previous behavior mode
     * @param NewMode The new behavior mode
     * 
     * Example Blueprint usage:
     * - Adjust priorities for new mode
     * - Cancel incompatible actions
     * - Trigger mode-specific setup
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Behavior")
    void OnBehaviorModeChanged(EAIBehaviorMode OldMode, EAIBehaviorMode NewMode);
    virtual void OnBehaviorModeChanged_Implementation(EAIBehaviorMode OldMode, EAIBehaviorMode NewMode);

    // ====================
    // BlueprintNativeEvent Hooks - Decision Making
    // ====================

    /**
     * Evaluate and return the current priority task
     * Override in Blueprint to implement custom priority logic
     * 
     * @return The priority level of the most important current task
     * 
     * Example Blueprint usage:
     * - Check for threats (Critical)
     * - Check resource needs (High/Medium)
     * - Check exploration opportunities (Low)
     * - Return Idle if nothing to do
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Decision")
    EAIPriority EvaluateCurrentPriority();
    virtual EAIPriority EvaluateCurrentPriority_Implementation();

    /**
     * Decide on the next action to take
     * Override in Blueprint to implement custom decision logic
     * 
     * @param CurrentPriority The current priority level
     * @return True if an action was decided upon
     * 
     * Example Blueprint usage:
     * - Switch on CurrentPriority
     * - Check available options for that priority
     * - Select best action based on state
     * - Return true if action selected
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Decision")
    bool DecideNextAction(EAIPriority CurrentPriority);
    virtual bool DecideNextAction_Implementation(EAIPriority CurrentPriority);

    /**
     * Check if the AI should change its behavior mode
     * Override in Blueprint for custom mode switching logic
     * 
     * @return The new behavior mode, or current mode if no change
     * 
     * Example Blueprint usage:
     * - Check if under attack -> Defensive
     * - Check if trade opportunity -> Trading
     * - Check if exploration complete -> return to Peaceful
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Decision")
    EAIBehaviorMode ShouldChangeBehaviorMode();
    virtual EAIBehaviorMode ShouldChangeBehaviorMode_Implementation();

    // ====================
    // Blueprint-Callable Utility Functions
    // These are helper functions that can be called from Blueprint
    // ====================

    /**
     * Activate the AI logic
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Control")
    void ActivateAI();

    /**
     * Deactivate the AI logic
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Control")
    void DeactivateAI();

    /**
     * Set the behavior mode and trigger mode change event
     * @param NewMode The behavior mode to switch to
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Control")
    void SetBehaviorMode(EAIBehaviorMode NewMode);

    /**
     * Check if AI is in a specific behavior mode
     * @param Mode The mode to check
     * @return True if in the specified mode
     */
    UFUNCTION(BlueprintPure, Category = "AI|Query")
    bool IsInBehaviorMode(EAIBehaviorMode Mode) const;

    /**
     * Check if AI is in peaceful/non-aggressive mode
     * @return True if in Peaceful, Trading, Exploration, or Diplomatic mode
     */
    UFUNCTION(BlueprintPure, Category = "AI|Query")
    bool IsPeaceful() const;

    /**
     * Check if AI is in aggressive/combat mode
     * @return True if in Aggressive or Defensive mode
     */
    UFUNCTION(BlueprintPure, Category = "AI|Query")
    bool IsAggressive() const;

    /**
     * Get a human-readable description of the current AI state
     * @return Text description of current state
     */
    UFUNCTION(BlueprintPure, Category = "AI|Query")
    FString GetAIStateDescription() const;

    /**
     * Main update function - call this from owner's Tick
     * This handles timing and calls OnTickAI when appropriate
     * @param DeltaTime Time since last update in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Control")
    void UpdateAI(float DeltaTime);
};
