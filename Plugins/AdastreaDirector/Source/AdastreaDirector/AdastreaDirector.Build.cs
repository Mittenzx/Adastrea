// Copyright (c) 2025 Mittenzx. All Rights Reserved.

using UnrealBuildTool;

public class AdastreaDirector : ModuleRules
{
	public AdastreaDirector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Sockets",
				"Networking",
				"ImageWrapper",      // For PNG encoding
				"RenderCore",        // For rendering thread operations
				"RHI",               // For viewport pixel reading
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",          // For GEditor and editor functionality
					"LevelEditor",       // For level editor access
				}
			);
		}
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"Json",
				"JsonUtilities",
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AssetTools",        // For asset import and creation
					"AssetRegistry",     // For asset queries
					"EditorScriptingUtilities", // For EditorAssetLibrary
				}
			);
		}
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
