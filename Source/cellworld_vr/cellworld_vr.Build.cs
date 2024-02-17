// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class cellworld_vr : ModuleRules
{
	public cellworld_vr(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "HeadMountedDisplay", "Engine", "InputCore", 
            "EyeTracker","InputCore","UMG","NavigationSystem","HPGlia",
            "TCPMessages", "ExperimentPlugin", "AIModule", "GameplayTasks",
            "Json", "JsonUtilities"
        });
        PrivateDependencyModuleNames.AddRange(new string[] { /*"HeadMountedDisplay",*/ "Slate", "SlateCore" });

        PublicIncludePaths.AddRange(new string[] {
            "cellworld_vr/PredatorController"
        });
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
