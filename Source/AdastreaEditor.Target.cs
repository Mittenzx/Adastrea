// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AdastreaEditorTarget : TargetRules
{
	public AdastreaEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		
		// Disable warnings as errors for third-party plugins
		bWarningsAsErrors = false;
		bOverrideBuildEnvironment = true;
		
		// Disable specific warning C4459 (variable name hiding in UE 5.6 engine code)
		AdditionalCompilerArguments = "/wd4459";
		
		ExtraModuleNames.Add("Adastrea");
		ExtraModuleNames.Add("PlayerMods");
		ExtraModuleNames.Add("StationEditor");
	}
}
