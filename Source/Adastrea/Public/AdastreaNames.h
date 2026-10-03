// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;

/**
 * Names shown to the player. Packaged builds have no actor labels, so anything the
 * HUD prints must come from game data (station names, roster names, ship data
 * assets), never from object names like "BP_TradeStation_C_2".
 */
namespace AdastreaNames
{
	/** "BP_TradeStation_C_2" -> "Trade Station". For last-resort fallbacks. */
	ADASTREA_API FString Readable(const FString& ObjectOrClassName);

	/**
	 * What to call an actor on the HUD: a jump gate's display name, a station's
	 * name, a ship's roster name or ship-type name, else its targetable display
	 * name, else a readable class name.
	 */
	ADASTREA_API FString ForActor(const AActor* Actor);
}
