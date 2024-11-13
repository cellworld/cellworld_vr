// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class ViveOpenXRWristTracker : ModuleRules
    {
        public ViveOpenXRWristTracker(ReadOnlyTargetRules Target) 
				: base(Target)
        {
			var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
            PrivateIncludePaths.AddRange(
                new string[] {
                    EngineDir + "/Source/ThirdParty/OpenXR/include",
                    EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                    EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRInput/Private",
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
                    "UMG",
                    "OpenXRHMD",
					"OpenXRInput",
                    "XRBase"
                }
				);

            PublicDependencyModuleNames.Add("EnhancedInput");

            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
                PrivateDependencyModuleNames.Add("InputEditor");
            }
        }
    }
}
