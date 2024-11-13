// Copyright HTC Corporation. All Rights Reserved.

using System;
using System.IO;
using Microsoft.Extensions.Logging;

namespace UnrealBuildTool.Rules
{
	public class ViveOpenXRHMD : ModuleRules
	{
		public ViveOpenXRHMD(ReadOnlyTargetRules Target) : base(Target)
		{
            Logger.LogInformation("Execute ViveOpenXRHMD.Build.cs");
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

			PrivateIncludePaths.AddRange(
				new string[] {
                    EngineDir+"/Plugins/Runtime/OpenXR/Source/OpenXRHMD/Private",
                    EngineDir+"/Source/ThirdParty/OpenXR/include",
					//OpenXR/Source/OpenXRHMD/Public/OpenXRPlatformRHI.h need them.
					Path.Combine(EngineDir, @"Source/Runtime/OpenGLDrv/Private"), //OpenGLDrvPrivate.h
					Path.Combine(EngineDir, @"Source/Runtime/VulkanRHI/Private"), //VulkanRHIPrivate.h
				}
			);

            PublicIncludePathModuleNames.AddRange(
			new string[]
			{
               "OpenXRHMD",
			});

            PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"HeadMountedDisplay",
				"OpenGLDrv",
				"VulkanRHI",
				"ViveOpenXRLoader",
				"InputCore",
				"UMG",
                "XRBase",
                "Projects"
            });

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"OpenXRHMD",
					"EyeTracker",
				}
			);

			AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan"); //VulkanRHI/Private/VulkanLoader.h need it.

			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.Add("UnrealEd");
			}

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PrivateIncludePaths.AddRange(
					new string[] {
						Path.Combine(EngineDir, @"Source/Runtime/Windows/D3D11RHI/Private"),
						Path.Combine(EngineDir, @"Source/Runtime/Windows/D3D11RHI/Private/Windows"),
						Path.Combine(EngineDir, @"Source/Runtime/D3D12RHI/Private"),
						Path.Combine(EngineDir, @"Source/Runtime/VulkanRHI/Private/Windows"),
					}
				);

				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"D3D11RHI",
						"D3D12RHI",
					}
				);

				AddEngineThirdPartyPrivateStaticDependencies(Target, "NVAftermath");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "IntelMetricsDiscovery");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "IntelExtensionsFramework");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
			}
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
                RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "../ViveOpenXRLoader/lib/android/arm64-v8a/libVIVEopenxr_loader.so"));
                PrivateIncludePaths.Add(Path.Combine(EngineDir, @"Source/Runtime/VulkanRHI/Private/Android"));
				{
					string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
					AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "ViveOpenXRAndroid_UPL.xml"));
				}
			}
		}
	}
}
