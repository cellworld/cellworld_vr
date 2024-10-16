// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class cellworld_vr : ModuleRules
{
	public cellworld_vr(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "Engine", "InputCore", 
            "EyeTracker","InputCore","UMG","NavigationSystem",
            "TCPMessages", "ExperimentPlugin", "AIModule", "GameplayTasks",
            "Json", "JsonUtilities", "AsyncLoadingScreen","UnrealEngineTemplateModule",
            "MiscUtils"
        });
        PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "XRBase", "Slate", "SlateCore" });

        PublicIncludePaths.AddRange(new string[] {
            // "cellworld_vr/PredatorController",
            "cellworld_vr/Public/Interfaces",
            "cellworld_vr/Public/Test",
            "cellworld_vr/Public"
            //"AsyncLoadingScreen/AsyncLoadingScreenLibrary.h"
        });
        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
