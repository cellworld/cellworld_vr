// Copyright HTC Corporation. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class ViveOpenXREyeTracker : ModuleRules
    {
        public ViveOpenXREyeTracker(ReadOnlyTargetRules Target)
                : base(Target)
        {
            PublicDependencyModuleNames.Add("EyeTracker");

            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivatePCHHeaderFile = @"Private\OpenXRCommon.h";
            var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);


            PrivateIncludePaths.AddRange(
                new string[] {
                "ViveOpenXREyeTracker/Private/External",
                EngineDir + "/Source/ThirdParty/OpenXR/include",
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRInput/Private",
                    // ... add private include paths required here ...
                }
            );
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "InputDevice",
                "InputCore",
                "HeadMountedDisplay",
                "OpenXRHMD",
                "OpenXRInput",
                "XRBase",
                "Projects"
                }
            );

            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }
    }
}
