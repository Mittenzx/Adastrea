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

		// Include paths for this module
		PublicIncludePaths.AddRange(new string[] 
		{
			"StationEditor",
			"Adastrea"  // For Stations/ includes
		});

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
