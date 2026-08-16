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

		// Note: No explicit include paths needed - UE5 automatically includes Public/ and Private/ subdirectories
		// The module now follows standard UE5 structure with all source files organized under Public/ and Private/
		// This eliminates the SetEnv environment variable length overflow issue

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

		// Note: AIModule and NavigationSystem were removed (Aug 2026) — their only
		// consumers (NavigationComponent, SimpleAIMovementComponent, AI/Personnel
		// cluster) were dead code and have been stripped. Re-add if AI/nav is re-enabled.

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
