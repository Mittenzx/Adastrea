// Copyright Mittenzx. All Rights Reserved.

#include "Tutorial/TutorialManagerSubsystem.h"
#include "TimerManager.h"

UTutorialManagerSubsystem::UTutorialManagerSubsystem()
	: bTutorialsEnabled(true)
	, bAutoStartTutorials(true)
{
}

void UTutorialManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTutorialManagerSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StepTimerHandle);
	}

	Super::Deinitialize();
}

bool UTutorialManagerSubsystem::StartTutorial(const FString& TutorialID, const TArray<UTutorialStepDataAsset*>& Steps)
{
	if (!bTutorialsEnabled || Steps.Num() == 0)
	{
		return false;
	}

	// Check if already completed
	if (IsTutorialCompleted(TutorialID))
	{
		return false;
	}

	// Stop any active tutorial
	if (HasActiveTutorial())
	{
		SkipTutorial();
	}

	// Set up new tutorial
	ActiveTutorialID = TutorialID;
	ActiveTutorialSteps = Steps;
	CurrentProgress = FTutorialProgress();
	CurrentProgress.TutorialID = TutorialID;

	// Start step timer
	GetWorld()->GetTimerManager().SetTimer(StepTimerHandle, this,
		&UTutorialManagerSubsystem::UpdateStepTimer, 1.0f, true);

	// Fire events
	OnTutorialStarted(TutorialID);
	if (Steps.Num() > 0)
	{
		OnStepChanged(0, Steps[0]);
	}

	return true;
}

bool UTutorialManagerSubsystem::AdvanceStep()
{
	if (!HasActiveTutorial())
	{
		return false;
	}

	int32 NextStep = CurrentProgress.CurrentStepIndex + 1;
	if (NextStep >= ActiveTutorialSteps.Num())
	{
		// Tutorial complete
		CompleteTutorial();
		return false;
	}

	// Move to next step
	CurrentProgress.CurrentStepIndex = NextStep;
	CurrentProgress.TimeOnCurrentStep = 0.0f;

	OnStepChanged(NextStep, ActiveTutorialSteps[NextStep]);
	return true;
}

bool UTutorialManagerSubsystem::PreviousStep()
{
	if (!HasActiveTutorial() || CurrentProgress.CurrentStepIndex == 0)
	{
		return false;
	}

	CurrentProgress.CurrentStepIndex--;
	CurrentProgress.TimeOnCurrentStep = 0.0f;

	OnStepChanged(CurrentProgress.CurrentStepIndex, 
		ActiveTutorialSteps[CurrentProgress.CurrentStepIndex]);
	return true;
}

void UTutorialManagerSubsystem::SkipTutorial()
{
	if (!HasActiveTutorial())
	{
		return;
	}

	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(StepTimerHandle);

	// Clear state
	ActiveTutorialID.Empty();
	ActiveTutorialSteps.Empty();
	CurrentProgress = FTutorialProgress();
}

void UTutorialManagerSubsystem::CompleteCurrentStep()
{
	AdvanceStep();
}

bool UTutorialManagerSubsystem::IsTutorialCompleted(const FString& TutorialID) const
{
	return CompletedTutorials.Contains(TutorialID);
}

UTutorialStepDataAsset* UTutorialManagerSubsystem::GetCurrentStep() const
{
	if (!HasActiveTutorial() || CurrentProgress.CurrentStepIndex >= ActiveTutorialSteps.Num())
	{
		return nullptr;
	}

	return ActiveTutorialSteps[CurrentProgress.CurrentStepIndex];
}

float UTutorialManagerSubsystem::GetTutorialProgress() const
{
	if (!HasActiveTutorial() || ActiveTutorialSteps.Num() == 0)
	{
		return 0.0f;
	}

	return (static_cast<float>(CurrentProgress.CurrentStepIndex) / 
		static_cast<float>(ActiveTutorialSteps.Num())) * 100.0f;
}

bool UTutorialManagerSubsystem::HasActiveTutorial() const
{
	return !ActiveTutorialID.IsEmpty() && ActiveTutorialSteps.Num() > 0;
}

void UTutorialManagerSubsystem::ResetAllProgress()
{
	SkipTutorial();
	CompletedTutorials.Empty();
}

void UTutorialManagerSubsystem::UpdateStepTimer()
{
	if (!HasActiveTutorial())
	{
		return;
	}

	CurrentProgress.TimeOnCurrentStep += 1.0f;

	// Check for time limit
	UTutorialStepDataAsset* CurrentStep = GetCurrentStep();
	if (CurrentStep && CurrentStep->HasTimeLimit())
	{
		if (CurrentProgress.TimeOnCurrentStep >= CurrentStep->TimeLimit)
		{
			// Time limit exceeded - advance automatically or fail
			AdvanceStep();
		}
	}
}

void UTutorialManagerSubsystem::CompleteTutorial()
{
	if (!HasActiveTutorial())
	{
		return;
	}

	// Mark as completed
	CompletedTutorials.AddUnique(ActiveTutorialID);
	CurrentProgress.bCompleted = true;

	// Fire completion event
	OnTutorialCompleted(ActiveTutorialID);

	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(StepTimerHandle);

	// Clear state
	FString CompletedID = ActiveTutorialID;
	ActiveTutorialID.Empty();
	ActiveTutorialSteps.Empty();
}
