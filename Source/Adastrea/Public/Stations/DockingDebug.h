// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"

/** On-screen docking debug messages. Off by default; enable with the console variable adastrea.Docking.Debug 1. */
namespace AdastreaDockingDebug
{
	ADASTREA_API bool IsEnabled();
	ADASTREA_API void Print(float Duration, const FColor& Color, const FString& Message);
}
