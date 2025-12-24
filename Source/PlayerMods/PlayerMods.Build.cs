// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlayerMods : ModuleRules
{
	public PlayerMods(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Warnings as errors enabled to catch potential issues early
		// Engine-level warnings (C4459) are suppressed at target level via /wd4459
		bWarningsAsErrors = true;

		// Include paths for this module
		PublicIncludePaths.Add("PlayerMods/Public");
		PrivateIncludePaths.Add("PlayerMods/Private");

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"Adastrea"  // Dependency on main module for AdastreaLog
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
		});
	}
}
