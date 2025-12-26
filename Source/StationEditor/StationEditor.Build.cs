// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StationEditor : ModuleRules
{
	public StationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Warnings as errors enabled to catch potential issues early
		// Engine-level warnings (C4459) are suppressed at target level via /wd4459
		bWarningsAsErrors = true;

		// Note: PublicIncludePaths removed to fix command line length issues during project generation
		// UE5 automatically includes Public/ and Private/ folders, making explicit paths redundant
		// Cross-module includes (e.g., "Factions/FactionDataAsset.h") work via dependency on Adastrea module

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"Adastrea",  // Dependency on main module for station and faction systems
			"UMG",       // For widget functionality
			"Slate",
			"SlateCore",
			"InputCore"   // Needed for EKeys and input handling in widgets
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
		});
	}
}
