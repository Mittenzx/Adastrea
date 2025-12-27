#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BoardingComponent.generated.h"

/**
 * Enum for boarding action states
 */
UENUM(BlueprintType)
enum class EBoardingState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Approaching     UMETA(DisplayName = "Approaching"),
    Breaching       UMETA(DisplayName = "Breaching"),
    Combat          UMETA(DisplayName = "In Combat"),
    Securing        UMETA(DisplayName = "Securing"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

/**
 * Struct for boarding party data
 */
USTRUCT(BlueprintType)
struct FBoardingParty
{
    GENERATED_BODY()

    /** Number of crew members in boarding party */
    UPROPERTY(BlueprintReadWrite)
    int32 CrewCount;

    /** Average skill level of boarding party (0-100) */
    UPROPERTY(BlueprintReadWrite)
    float SkillLevel;

    /** Equipment quality (0-100) */
    UPROPERTY(BlueprintReadWrite)
    float EquipmentQuality;

    /** Morale level (0-100) */
    UPROPERTY(BlueprintReadWrite)
    float Morale;

    /** Current casualties */
    UPROPERTY(BlueprintReadWrite)
    int32 Casualties;

    FBoardingParty()
        : CrewCount(0)
        , SkillLevel(50.0f)
        , EquipmentQuality(50.0f)
        , Morale(100.0f)
        , Casualties(0)
    {}
};

/**
 * Struct for boarding action data
 */
USTRUCT(BlueprintType)
struct FBoardingAction
{
    GENERATED_BODY()

    /** Target ship being boarded */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> TargetShip;

    /** Current state of boarding action */
    UPROPERTY(BlueprintReadOnly)
    EBoardingState State;

    /** Boarding party data */
    UPROPERTY(BlueprintReadOnly)
    FBoardingParty AttackingParty;

    /** Defending crew data */
    UPROPERTY(BlueprintReadOnly)
    FBoardingParty DefendingParty;

    /** Progress percentage (0-100) */
    UPROPERTY(BlueprintReadOnly)
    float Progress;

    /** Time elapsed in current state */
    UPROPERTY(BlueprintReadOnly)
    float TimeInState;

    /** Whether boarding was initiated by this ship */
    UPROPERTY(BlueprintReadOnly)
    bool bIsAttacker;

    FBoardingAction()
        : TargetShip(nullptr)
        , State(EBoardingState::Idle)
        , Progress(0.0f)
        , TimeInState(0.0f)
        , bIsAttacker(false)
    {}
};

/**
 * Component for managing boarding actions and ship capture mechanics
 * 
 * Handles crew-based combat for boarding enemy ships, defending against
 * boarding attempts, and managing ship capture logic.
 * 
 * Usage:
 * - Add component to ship Blueprint
 * - Configure boarding capability and defense strength
 * - Use InitiateBoardingAction() to start boarding
 * - Monitor boarding progress through Blueprint events
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class ADASTREA_API UBoardingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBoardingComponent();

    // ====================
    // CONFIGURATION
    // ====================

    /** Maximum boarding crew capacity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="0", ClampMax="100"))
    int32 MaxBoardingCrew;

    /** Base boarding skill level (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="0", ClampMax="100"))
    float BaseBoardingSkill;

    /** Defense rating against boarding (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="0", ClampMax="100"))
    float DefenseRating;

    /** Time required to breach hull (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="1", ClampMax="300"))
    float BreachTime;

    /** Base combat resolution interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="1", ClampMax="60"))
    float CombatInterval;

    /** Minimum crew required to maintain ship control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding", meta=(ClampMin="1", ClampMax="100"))
    int32 MinimumCrewForControl;

    /** Can this ship initiate boarding actions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding")
    bool bCanBoard;

    /** Can this ship be boarded by others */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boarding")
    bool bCanBeBoarded;

    // ====================
    // STATE
    // ====================

    /** Active boarding action (if any) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boarding State")
    FBoardingAction ActiveBoardingAction;

    /** Whether currently engaged in boarding action */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boarding State")
    bool bIsBoardingActive;

    /** Whether ship has been captured */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boarding State")
    bool bIsCaptured;

    /** Controller that captured this ship (if captured) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boarding State")
    AController* CapturingController;

protected:
    /** Time since last combat resolution */
    float TimeSinceCombatTick;

    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ====================
    // BOARDING OPERATIONS
    // ====================

    /**
     * Initiate a boarding action against target ship
     * @param TargetShip Ship to board
     * @param BoardingCrewCount Number of crew to send
     * @return True if boarding action started successfully
     */
    UFUNCTION(BlueprintCallable, Category="Boarding")
    bool InitiateBoardingAction(AActor* TargetShip, int32 BoardingCrewCount);

    /**
     * Abort current boarding action
     */
    UFUNCTION(BlueprintCallable, Category="Boarding")
    void AbortBoardingAction();

    /**
     * Surrender ship to attacker
     * @param Attacker Controller taking control
     */
    UFUNCTION(BlueprintCallable, Category="Boarding")
    void SurrenderShip(AController* Attacker);

    /**
     * Recapture ship from captors
     * @param RescuingController Controller reclaiming ship
     * @return True if recapture successful
     */
    UFUNCTION(BlueprintCallable, Category="Boarding")
    bool RecaptureShip(AController* RescuingController);

    // ====================
    // QUERIES
    // ====================

    /**
     * Get boarding progress percentage (0-100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    float GetBoardingProgress() const;

    /**
     * Get current boarding state
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    EBoardingState GetBoardingState() const;

    /**
     * Check if ship can initiate boarding on target
     * @param TargetShip Ship to check
     * @return True if boarding is possible
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    bool CanInitiateBoardingOn(AActor* TargetShip) const;

    /**
     * Estimate boarding success chance
     * @param TargetShip Target to board
     * @param BoardingCrewCount Crew to send
     * @return Estimated success probability (0-1)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    float EstimateBoardingSuccessChance(AActor* TargetShip, int32 BoardingCrewCount) const;

    /**
     * Get attacker combat strength
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    float GetAttackerCombatStrength() const;

    /**
     * Get defender combat strength
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Boarding")
    float GetDefenderCombatStrength() const;

    // ====================
    // BLUEPRINT NATIVE EVENTS
    // ====================

    /**
     * Called when boarding action begins
     * @param TargetShip Ship being boarded
     * @param bAsAttacker True if this ship is boarding
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnBoardingStarted(AActor* TargetShip, bool bAsAttacker);

    /**
     * Called when breaching phase completes
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnBreachComplete();

    /**
     * Called after each combat resolution
     * @param AttackerCasualties Casualties suffered by attackers
     * @param DefenderCasualties Casualties suffered by defenders
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnCombatResolved(int32 AttackerCasualties, int32 DefenderCasualties);

    /**
     * Called when boarding action succeeds
     * @param CapturedShip Ship that was captured
     * @param NewOwner Controller that captured the ship
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnBoardingSuccessful(AActor* CapturedShip, AController* NewOwner);

    /**
     * Called when boarding action fails
     * @param Reason Why boarding failed
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnBoardingFailed(const FString& Reason);

    /**
     * Called when ship is captured
     * @param Captor Controller that captured this ship
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnShipCaptured(AController* Captor);

    /**
     * Called when ship is recaptured
     * @param Rescuer Controller that recaptured this ship
     */
    UFUNCTION(BlueprintNativeEvent, Category="Boarding")
    void OnShipRecaptured(AController* Rescuer);

private:
    /** Update boarding state */
    void UpdateBoardingAction(float DeltaTime);

    /** Process breach phase */
    void ProcessBreachPhase(float DeltaTime);

    /** Process combat phase */
    void ProcessCombatPhase(float DeltaTime);

    /** Resolve combat between boarding party and defenders */
    void ResolveCombat();

    /** Calculate combat outcome casualties */
    void CalculateCasualties(float AttackerStrength, float DefenderStrength, 
                            int32& OutAttackerCasualties, int32& OutDefenderCasualties);

    /** Check if boarding action is complete */
    void CheckBoardingCompletion();

    /** Complete successful boarding */
    void CompleteBoardingSuccess();

    /** Fail boarding action */
    void FailBoardingAction(const FString& Reason);

    /** Get boarding component from target ship */
    UBoardingComponent* GetTargetBoardingComponent() const;
};
