// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AdastreaTarget : TargetRules
{
	public AdastreaTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		
		// Disable warnings as errors for third-party plugins
		bUseUnityBuild = false;
		bWarningsAsErrors = false;
		
		// Disable specific warning C4459 (variable name hiding in UE 5.6 engine code)
		AdditionalCompilerArguments = "/wd4459";
		
		ExtraModuleNames.Add("Adastrea");
		ExtraModuleNames.Add("PlayerMods");
		ExtraModuleNames.Add("StationEditor");
	}
}
