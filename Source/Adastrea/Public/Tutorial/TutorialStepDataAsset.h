// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TutorialStepDataAsset.generated.h"

/**
 * Tutorial step types
 */
UENUM(BlueprintType)
enum class ETutorialStepType : uint8
{
	Message UMETA(DisplayName = "Show Message"),
	HighlightUI UMETA(DisplayName = "Highlight UI Element"),
	WaitForAction UMETA(DisplayName = "Wait For Action"),
	WaitForCondition UMETA(DisplayName = "Wait For Condition"),
	PlayAnimation UMETA(DisplayName = "Play Animation"),
	SpawnActor UMETA(DisplayName = "Spawn Actor")
};

/**
 * Tutorial Step Data Asset for defining tutorial sequences
 * 
 * This class provides a designer-friendly way to create tutorial steps
 * that guide players through game mechanics.
 * 
 * Usage:
 * 1. Create a Blueprint based on this class
 * 2. Configure step properties and text
 * 3. Chain steps together in tutorial sequences
 * 4. Reference from tutorial manager
 * 
 * Example:
 * - Create DA_Tutorial_Movement_Step1
 * - Set type to Message with movement instructions
 * - Add to tutorial sequence for new players
 */
UCLASS(BlueprintType)
class ADASTREA_API UTutorialStepDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UTutorialStepDataAsset();

	// ========================================================================
	// Basic Info
	// ========================================================================

	/** Display name for this tutorial step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	FText StepTitle;

	/** Detailed description shown to player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info", meta = (MultiLine = true))
	FText StepDescription;

	/** Type of tutorial step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
	ETutorialStepType StepType;

	// ========================================================================
	// Step Configuration
	// ========================================================================

	/** Maximum time to complete this step (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration",
		meta = (ClampMin = 0.0, UIMin = 0.0))
	float TimeLimit;

	/** Whether this step can be skipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration")
	bool bCanSkip;

	/** Whether to pause the game during this step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration")
	bool bPauseGame;

	/** UI element name to highlight (for HighlightUI type) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration",
		meta = (EditCondition = "StepType == ETutorialStepType::HighlightUI"))
	FString UIElementToHighlight;

	/** Action name to wait for (for WaitForAction type) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration",
		meta = (EditCondition = "StepType == ETutorialStepType::WaitForAction"))
	FString ActionToWaitFor;

	/** Actor class to spawn (for SpawnActor type) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Configuration",
		meta = (EditCondition = "StepType == ETutorialStepType::SpawnActor"))
	TSubclassOf<AActor> ActorToSpawn;

	// ========================================================================
	// Audio/Visual
	// ========================================================================

	/** Optional voiceover audio for this step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Visual")
	class USoundBase* VoiceoverAudio;

	/** Optional background image or video */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Visual")
	class UTexture2D* BackgroundImage;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Check if this step has a time limit
	 * @return True if there is a time limit
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Step")
	bool HasTimeLimit() const { return TimeLimit > 0.0f; }

	/**
	 * Get step type as string
	 * @return String representation of step type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial Step")
	FString GetStepTypeString() const;
};
