// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdastreaEditor : ModuleRules
{
	public AdastreaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Warnings as errors to match the rest of the project.
		bWarningsAsErrors = true;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Adastrea",          // mesh data asset types (UStaticMesh comes from Engine)
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DataValidation",    // UEditorValidatorBase
			"UnrealEd",          // editor-only editing support
			"MeshDescription",   // FMeshDescription
			"StaticMeshDescription", // FStaticMeshAttributes
			"Slate",
			"SlateCore",
		});
	}
}