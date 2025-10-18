// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdAstreaEditor : ModuleRules
{
	public AdAstreaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"Adastrea"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
			"UnrealEd",
			"Slate",
			"SlateCore",
			"PropertyEditor",
			"EditorStyle",
			"DetailCustomizations"
		});
	}
}
