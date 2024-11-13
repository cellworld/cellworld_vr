// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ViveOpenXRXrTracker : ModuleRules
{
	public ViveOpenXRXrTracker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = @"Private\OpenXRCommon.h";
        var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

        PrivateIncludePaths.AddRange(
			new string[] {
                "ViveOpenXRTracker/ViveOpenXRXrTracker/Private/External",
                EngineDir + "/Source/ThirdParty/OpenXR/include",
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRInput/Private",
				// ... add public include paths required here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "InputDevice",
                "InputCore",
                "Engine",
				"Slate",
				"SlateCore",
                "HeadMountedDisplay",
                "OpenXRHMD",
                "OpenXRInput",
                "XRBase"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
