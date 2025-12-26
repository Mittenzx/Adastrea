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

		// Note: No explicit include paths needed - UE5 automatically includes Public/ and Private/ subdirectories
		// The module now follows standard UE5 structure with all source files organized under Public/ and Private/

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
