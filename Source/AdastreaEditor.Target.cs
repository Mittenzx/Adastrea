// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AdastreaEditorTarget : TargetRules
{
	public AdastreaEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Adastrea");
		ExtraModuleNames.Add("PlayerMods");
		ExtraModuleNames.Add("StationEditor");
	}
}
