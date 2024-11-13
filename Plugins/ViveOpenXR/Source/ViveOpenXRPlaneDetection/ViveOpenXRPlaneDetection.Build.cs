// Copyright HTC Corporation. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class ViveOpenXRPlaneDetection : ModuleRules
	{
		public ViveOpenXRPlaneDetection(ReadOnlyTargetRules Target) : base(Target)
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
				}
			);
		}
	}
}
