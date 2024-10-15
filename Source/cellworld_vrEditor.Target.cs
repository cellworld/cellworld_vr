// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class cellworld_vrEditorTarget : TargetRules {
	
	public cellworld_vrEditorTarget( TargetInfo Target) : base(Target)
	{
		// BuildEnvironment = TargetBuildEnvironment.Unique;
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "cellworld_vr" } );
	}
}
