// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdastreaEditor : ModuleRules
{
	public AdastreaEditor(ReadOnlyTargetRules Target) : base(Target)
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
