// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StationEditor : ModuleRules
{
	public StationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Disable warnings as errors for UE 5.6 compatibility
		bWarningsAsErrors = false;

		// Include paths for this module
		PublicIncludePaths.AddRange(new string[] 
		{
			"StationEditor",
			"StationEditor/Public",
			"Adastrea/Stations"  // For SpaceStationModule.h
		});

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"Adastrea",  // Dependency on main module for station and faction systems
			"UMG",       // For widget functionality
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
		});
	}
}
