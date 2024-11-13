// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class ViveOpenXRFacialTracking: ModuleRules
    {

        public ViveOpenXRFacialTracking(ReadOnlyTargetRules Target) 
				: base(Target)
        {
            //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivatePCHHeaderFile = @"Private\OpenXRCommon.h";

            PublicDependencyModuleNames.AddRange(
			   new string[]
			   {
					"InputDevice",
					"LiveLink",
					"LiveLinkInterface"
			   }
		   );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
					"Slate",
					"HeadMountedDisplay",
					"SlateCore",
					"LiveLink",
					"LiveLinkInterface",
					"OpenXRHMD",
					"OpenXRInput"
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
