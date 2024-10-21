// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class cellworld_vr : ModuleRules
{
	public cellworld_vr(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// PCHUsage = PCHUsageMode.UseSharedPCHs;
		// PrivatePCHHeaderFile = "cellworld_vr.h";
		
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "Engine", "InputCore", "UMG","NavigationSystem",
            "TCPMessages", "ExperimentPlugin", "AIModule", "GameplayTasks",
            "Json", "JsonUtilities", "AsyncLoadingScreen","UnrealEngineTemplateModule",
            "MiscUtils","AndroidPermission","OculusXRAnchors","OculusXRHMD", "OculusXRPassthrough",
            "RHI","RenderCore"
		});
        PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "AugmentedReality", "XRBase", "Slate", "SlateCore" });

        PublicIncludePaths.AddRange(new string[] {
            "cellworld_vr/Public/Interfaces",
            "cellworld_vr/Public/Test",
            "cellworld_vr/Public"
        });
        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
