// Copyright Mittenzx. All Rights Reserved.

#include "Tutorial/TutorialStepDataAsset.h"

UTutorialStepDataAsset::UTutorialStepDataAsset()
	: StepType(ETutorialStepType::Message)
	, TimeLimit(0.0f)
	, bCanSkip(true)
	, bPauseGame(false)
	, VoiceoverAudio(nullptr)
	, BackgroundImage(nullptr)
{
	StepTitle = FText::FromString("Tutorial Step");
	StepDescription = FText::FromString("Configure this tutorial step");
}

FString UTutorialStepDataAsset::GetStepTypeString() const
{
	switch (StepType)
	{
	case ETutorialStepType::Message:
		return TEXT("Message");
	case ETutorialStepType::HighlightUI:
		return TEXT("Highlight UI");
	case ETutorialStepType::WaitForAction:
		return TEXT("Wait For Action");
	case ETutorialStepType::WaitForCondition:
		return TEXT("Wait For Condition");
	case ETutorialStepType::PlayAnimation:
		return TEXT("Play Animation");
	case ETutorialStepType::SpawnActor:
		return TEXT("Spawn Actor");
	default:
		return TEXT("Unknown");
	}
}
