// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaGameState.h"
#include "Net/UnrealNetwork.h"

AAdastreaGameState::AAdastreaGameState()
{
	CurrentGameDay = 0;
}

void AAdastreaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAdastreaGameState, CurrentGameDay);
	DOREPLIFETIME(AAdastreaGameState, ActiveEvents);
	DOREPLIFETIME(AAdastreaGameState, ActiveTraders);
}

void AAdastreaGameState::AdvanceDay()
{
	CurrentGameDay++;

	// Update event durations
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
	{
		if (ActiveEvents[i].DaysRemaining > 0)
		{
			ActiveEvents[i].DaysRemaining--;
			
			// Remove events that have expired
			if (ActiveEvents[i].DaysRemaining == 0)
			{
				ActiveEvents.RemoveAt(i);
			}
		}
	}

	OnDayAdvanced();
}

bool AAdastreaGameState::IsEventActive(const FName& EventName) const
{
	for (const FActiveEvent& Event : ActiveEvents)
	{
		if (Event.EventName == EventName)
		{
			return true;
		}
	}
	return false;
}

void AAdastreaGameState::ActivateEvent(const FName& EventName, int32 Duration)
{
	// Check if event already exists
	for (FActiveEvent& Event : ActiveEvents)
	{
		if (Event.EventName == EventName)
		{
			// Update existing event duration
			Event.DaysRemaining = Duration;
			return;
		}
	}

	// Add new event
	ActiveEvents.Add(FActiveEvent(EventName, Duration));
}

void AAdastreaGameState::DeactivateEvent(const FName& EventName)
{
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
	{
		if (ActiveEvents[i].EventName == EventName)
		{
			ActiveEvents.RemoveAt(i);
			return;
		}
	}
}

void AAdastreaGameState::OnDayAdvanced()
{
	// Override in Blueprint or derived classes to add custom day-change logic
}
