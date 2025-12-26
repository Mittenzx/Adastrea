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

		// Note: StationEditor has a non-standard directory structure with UI/ subdirectory
		// at the module root level. This needs an explicit include path.
		// Using relative path (ModuleDirectory) to keep paths short and avoid SetEnv errors.
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "UI"));

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
