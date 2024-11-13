// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class ViveOpenXRPassthrough: ModuleRules
    {

        public ViveOpenXRPassthrough(ReadOnlyTargetRules Target) 
				: base(Target)
        {
            //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivatePCHHeaderFile = @"Private\OpenXRCommon.h";

            var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
            PrivateIncludePaths.AddRange(
                new string[] {
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                    // ... add public include paths required here ...
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "OpenXRHMD",
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "ViveOpenXRHMD",
                    "InputCore",
                    "HeadMountedDisplay",
                    "OpenXRHMD",
                    "ViveOpenXRWrapper",
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
