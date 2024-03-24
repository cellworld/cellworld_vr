// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CellworldVR : ModuleRules
{
	public CellworldVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore","XRBase",
            "EyeTracker","InputCore","UMG","NavigationSystem","HeadMountedDisplay",
            "TCPMessages", "ExperimentPlugin", "AIModule", "GameplayTasks",
            "Json", "JsonUtilities", "AsyncLoadingScreen"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {"Slate", "SlateCore" });
        PublicIncludePaths.AddRange(new string[] {
            "CellworldVR/PredatorController",
            "CellworldVR/Public",
            "AsyncLoadingScreen/AsyncLoadingScreenLibrary.h"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
