// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ViveOpenXRAnchor : ModuleRules
{
    public ViveOpenXRAnchor(ReadOnlyTargetRules Target) : base(Target)
    {
        var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
        PrivateIncludePaths.AddRange(
            new string[] {
                EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
				// ... add public include paths required here ...
			}
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ViveOpenXRHMD",
                "ViveOpenXRWrapper",
                "OpenXRHMD",
                "OpenXRAR",
                "AugmentedReality",
                "XRBase",
            }
        );

    }
}






























