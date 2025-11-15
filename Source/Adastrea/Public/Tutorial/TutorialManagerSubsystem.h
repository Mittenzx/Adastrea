// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tutorial/TutorialStepDataAsset.h"
#include "TutorialManagerSubsystem.generated.h"

/**
 * Tutorial progress structure
 */
USTRUCT(BlueprintType)
struct FTutorialProgress
{
	GENERATED_BODY()

	/** Tutorial ID */
	UPROPERTY(BlueprintReadOnly)
	FString TutorialID;

	/** Current step index */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStepIndex;

	/** Whether tutorial is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bCompleted;

	/** Time spent on current step */
	UPROPERTY(BlueprintReadOnly)
	float TimeOnCurrentStep;

	FTutorialProgress()
		: CurrentStepIndex(0)
		, bCompleted(false)
		, TimeOnCurrentStep(0.0f)
	{
	}
};

/**
 * Tutorial Manager Subsystem for managing tutorial sequences
 * 
 * This subsystem handles tutorial progression, tracking, and state management.
 * 
 * Usage:
 * 1. Access via UGameInstance::GetSubsystem<UTutorialManagerSubsystem>()
 * 2. Call StartTutorial with tutorial ID and steps
 * 3. System manages step progression automatically
 * 4. Listen for events to update UI
 * 
 * Example:
 * - New player starts game
 * - System starts "BasicControls" tutorial
 * - Player completes steps
 * - Progress is tracked and saved
 */
UCLASS()
class ADASTREA_API UTutorialManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTutorialManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// Configuration
	// ========================================================================

	/** Whether tutorials are enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Settings")
	bool bTutorialsEnabled;

	/** Whether to auto-start tutorials for new players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Settings")
	bool bAutoStartTutorials;

	// ========================================================================
	// State
	// ========================================================================

	/** Current active tutorial */
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial State")
	FString ActiveTutorialID;

	/** Current tutorial steps */
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial State")
	TArray<UTutorialStepDataAsset*> ActiveTutorialSteps;

	/** Current tutorial progress */
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial State")
	FTutorialProgress CurrentProgress;

	/** Completed tutorials */
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial State")
	TArray<FString> CompletedTutorials;

	// ========================================================================
	// Blueprint Events
	// ========================================================================

	/**
	 * Event fired when a tutorial starts
	 * @param TutorialID The tutorial that started
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Events")
	void OnTutorialStarted(const FString& TutorialID);

	/**
	 * Event fired when a tutorial step changes
	 * @param StepIndex The new step index
	 * @param Step The tutorial step data
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Events")
	void OnStepChanged(int32 StepIndex, UTutorialStepDataAsset* Step);

	/**
	 * Event fired when a tutorial is completed
	 * @param TutorialID The tutorial that was completed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial Events")
	void OnTutorialCompleted(const FString& TutorialID);

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Start a tutorial sequence
	 * @param TutorialID Unique ID for this tutorial
	 * @param Steps Array of tutorial steps
	 * @return True if tutorial was started
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	bool StartTutorial(const FString& TutorialID, const TArray<UTutorialStepDataAsset*>& Steps);

	/**
	 * Advance to the next tutorial step
	 * @return True if advanced to next step
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	bool AdvanceStep();

	/**
	 * Go back to the previous tutorial step
	 * @return True if went to previous step
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	bool PreviousStep();

	/**
	 * Skip the current tutorial
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	void SkipTutorial();

	/**
	 * Complete the current tutorial step
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	void CompleteCurrentStep();

	/**
	 * Check if a tutorial has been completed
	 * @param TutorialID Tutorial to check
	 * @return True if tutorial was completed
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Manager")
	bool IsTutorialCompleted(const FString& TutorialID) const;

	/**
	 * Get current tutorial step
	 * @return Current step data asset
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Manager")
	UTutorialStepDataAsset* GetCurrentStep() const;

	/**
	 * Get progress percentage for current tutorial (0-100)
	 * @return Progress percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Manager")
	float GetTutorialProgress() const;

	/**
	 * Check if there is an active tutorial
	 * @return True if tutorial is active
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Manager")
	bool HasActiveTutorial() const;

	/**
	 * Reset all tutorial progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Tutorial Manager")
	void ResetAllProgress();

private:
	/** Timer handle for step time tracking */
	FTimerHandle StepTimerHandle;

	/** Update step timer */
	void UpdateStepTimer();

	/** Complete the current tutorial */
	void CompleteTutorial();
};
