// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlayerMods : ModuleRules
{
	public PlayerMods(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
