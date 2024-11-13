// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class ViveOpenXRDisplayRefreshRate : ModuleRules
	{

		public ViveOpenXRDisplayRefreshRate(ReadOnlyTargetRules Target) : base(Target)
		{
            var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
            PrivateIncludePaths.AddRange(
                new string[] {
                    EngineDir + "/Source/ThirdParty/OpenXR/include",
                    EngineDir + "/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                }
                );

            PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"ViveOpenXRHMD",
					"OpenXRHMD",
				}
			);
		}
	}
}
