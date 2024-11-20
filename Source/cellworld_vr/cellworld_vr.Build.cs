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
<<<<<<< HEAD
            "Json", "JsonUtilities", "AsyncLoadingScreen", "MiscUtils", 
            "BotEvadeModule"
        });
        PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "XRBase", "Slate", "SlateCore" });
=======
            "Json", "JsonUtilities", "AsyncLoadingScreen","UnrealEngineTemplateModule",
            "MiscUtils","AndroidPermission","OculusXRAnchors","OculusXRHMD", "OculusXRPassthrough",
            "RHI","RenderCore"
		});
        PrivateDependencyModuleNames.AddRange(new string[] { "HeadMountedDisplay", "AugmentedReality", "XRBase", "Slate", "SlateCore" });
>>>>>>> main

        PublicIncludePaths.AddRange(new string[] {
            "cellworld_vr/Public/Interfaces",
            "cellworld_vr/Public/Test",
<<<<<<< HEAD
            "cellworld_vr/Public",
            //"AsyncLoadingScreen/AsyncLoadingScreenLibrary.h"
=======
            "cellworld_vr/Public"
>>>>>>> main
        });
    }
}
