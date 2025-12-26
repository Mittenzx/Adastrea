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

		// Note: The Adastrea module has a non-standard directory structure with source files
		// in subdirectories at the module root level (AI/, Combat/, etc.) rather than within
		// Public/ and Private/ subdirectories. These need explicit include paths.
		// Using relative paths (ModuleDirectory) to keep paths short and avoid SetEnv errors.
		PublicIncludePaths.AddRange(new string[] {
			System.IO.Path.Combine(ModuleDirectory, "AI"),
			System.IO.Path.Combine(ModuleDirectory, "Audio"),
			System.IO.Path.Combine(ModuleDirectory, "Characters"),
			System.IO.Path.Combine(ModuleDirectory, "Combat"),
			System.IO.Path.Combine(ModuleDirectory, "Exploration"),
			System.IO.Path.Combine(ModuleDirectory, "Factions"),
			System.IO.Path.Combine(ModuleDirectory, "Input"),
			System.IO.Path.Combine(ModuleDirectory, "Materials"),
			System.IO.Path.Combine(ModuleDirectory, "Navigation"),
			System.IO.Path.Combine(ModuleDirectory, "Performance"),
			System.IO.Path.Combine(ModuleDirectory, "Planets"),
			System.IO.Path.Combine(ModuleDirectory, "Player"),
			System.IO.Path.Combine(ModuleDirectory, "Procedural"),
			System.IO.Path.Combine(ModuleDirectory, "Quest"),
			System.IO.Path.Combine(ModuleDirectory, "Rivals"),
			System.IO.Path.Combine(ModuleDirectory, "Ships"),
			System.IO.Path.Combine(ModuleDirectory, "Stations"),
			System.IO.Path.Combine(ModuleDirectory, "Trading"),
			System.IO.Path.Combine(ModuleDirectory, "Tutorial"),
			System.IO.Path.Combine(ModuleDirectory, "UI"),
			System.IO.Path.Combine(ModuleDirectory, "Way")
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
