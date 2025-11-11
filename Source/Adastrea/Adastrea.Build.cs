// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Adastrea : ModuleRules
{
	public Adastrea(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
			"Adastrea/Way",
			"Adastrea/Rivals"
		});

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
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
