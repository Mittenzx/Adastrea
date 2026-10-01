#pragma once

#include "CoreMinimal.h"

/**
 * Who fights whom. Bolts never hit a ship on the shooter's own team, except Neutral,
 * which never matches (so neutral targets like the CombatTest dummies can be shot by
 * anyone).
 */
namespace CombatTeam
{
	/** Unaligned: hit by everyone, including other neutrals. */
	constexpr int32 Neutral = 0;
	/** Hostile fighters. */
	constexpr int32 Hostile = 1;
	/** The player, AI traders and miners, and station defences. */
	constexpr int32 Civil = 2;
}
