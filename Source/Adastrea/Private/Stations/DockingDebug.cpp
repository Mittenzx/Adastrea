// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingDebug.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarDockingDebug(
	TEXT("adastrea.Docking.Debug"),
	0,
	TEXT("Show on-screen [DOCKING] debug messages. 0 = off (default), 1 = on."),
	ECVF_Default);

bool AdastreaDockingDebug::IsEnabled()
{
	return CVarDockingDebug.GetValueOnGameThread() != 0;
}

void AdastreaDockingDebug::Print(float Duration, const FColor& Color, const FString& Message)
{
	if (GEngine && IsEnabled())
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
}
