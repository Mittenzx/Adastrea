// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Adastrea : ModuleRules
{
	public Adastrea(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// TODO [MVP]: Re-enable warnings as errors after addressing UE 5.6 compatibility warnings
		// This is currently disabled which hides potential issues (Anti-Pattern #14)
		// See: docs/development/SIMPLIFIED_BUILD_GUIDE.md for build configuration
		bWarningsAsErrors = false;

		// Expose public headers for cross-module includes
		// Add base "Adastrea" path to support folder-prefixed includes like "Stations/SpaceStationModule.h"
		PublicIncludePaths.AddRange(new string[] 
		{
			"Adastrea",
			"Adastrea/Public",
			"Adastrea/Public/AI",
			"Adastrea/Public/Ships",
			"Adastrea/Public/Factions",
			"Adastrea/Public/Combat",
			"Adastrea/Public/Navigation",
			"Adastrea/Public/Quest",
			"Adastrea/Public/Input",
			"Adastrea/Public/Audio",
			"Adastrea/Public/Performance",
			"Adastrea/Public/UI",
			"Adastrea/Public/Tutorial",
			"Adastrea/Public/Exploration",
			"Adastrea/Way",
			"Adastrea/Rivals"
		});

		// Note: StationEditor dependency removed to fix circular dependency
		// StationEditor depends on Adastrea, so Adastrea cannot depend on StationEditor
		// Station Editor widget implementations moved to StationEditor module
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
			"AIModule",
			"NavigationSystem"
		});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
