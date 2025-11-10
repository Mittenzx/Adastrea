// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * StationEditor Module
 * 
 * Runtime module providing in-game, player-facing space station editor features.
 * This module contains all UI and gameplay systems for the station construction editor
 * that players interact with during gameplay.
 * 
 * This is separate from AdastreaEditor which contains developer/designer tools
 * for the Unreal Editor.
 */
class FStationEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
