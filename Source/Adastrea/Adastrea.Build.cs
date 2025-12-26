// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Adastrea : ModuleRules
{
	public Adastrea(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Warnings as errors enabled to catch potential issues early
		// Engine-level warnings (C4459) are suppressed at target level via /wd4459
		// See: Anti-Pattern #15 (Build System Fragility) - fix warnings, don't disable them
		bWarningsAsErrors = true;

		// Note: PublicIncludePaths removed to fix command line length issues during project generation
		// UE5 automatically includes Public/ and Private/ folders, making explicit paths redundant
		// Existing includes like "Ships/SpaceshipDataAsset.h" continue to work via UE5's automatic include path resolution

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
