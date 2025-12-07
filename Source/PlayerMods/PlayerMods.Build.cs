// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlayerMods : ModuleRules
{
	public PlayerMods(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Disable warnings as errors for UE 5.6 compatibility
		bWarningsAsErrors = false;

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
