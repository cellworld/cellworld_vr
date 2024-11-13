// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using EpicGames.Core;
using Microsoft.Extensions.Logging;

public class ViveOpenXRSceneUnderstanding : ModuleRules
{
	public ViveOpenXRSceneUnderstanding(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        //PrivatePCHHeaderFile = @"Private\OpenXRCommon.h";

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"OpenXRHMD",
				"OpenXRInput",
				"OpenXRAR",
				"AugmentedReality",
				"Slate",
				"SlateCore",
				"InputCore",
				"RHI",
				"RenderCore",
				"ViveOpenXRRuntimeSettings"
				// ... add private dependencies that you statically link with here ...	
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenXR");
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.Platform == UnrealTargetPlatform.Android)
		{
            ConfigHierarchy Ini = ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine, Target.ProjectFile != null ? Target.ProjectFile.Directory : null, Target.Platform);
            bool bEnableSceneUnderstanding = false;
            string SettingsSection = "/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings";
            Ini.GetBool(SettingsSection, "bEnableSceneUnderstanding", out bEnableSceneUnderstanding);

            Logger.LogInformation("Enable Scene Understanding: {0}", bEnableSceneUnderstanding);

			if (bEnableSceneUnderstanding)
			{
                Logger.LogInformation("Vive OpenXR Scene Understanding is not supported on Vive AIO devices.  For Vive XR Elite, please use Anchor and Plane Detection.");
			}
        }
    }
}
