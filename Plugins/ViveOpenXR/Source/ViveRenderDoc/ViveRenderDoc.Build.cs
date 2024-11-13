// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class ViveRenderDoc : ModuleRules
	{
		public ViveRenderDoc(ReadOnlyTargetRules Target)
		: base(Target)
		{
			var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
			PrivateIncludePaths.AddRange(
				new string[] {
					Path.Combine(EngineDir, @"Source\Runtime\VulkanRHI\Private"), //VulkanRHIPrivate.h
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"Engine",
					"OpenXRHMD",
					"RHICore", //For RHICoreShader.h
					"VulkanRHI",
					"RHI"
				}
			);

			AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan"); //For vulkan_core.h

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PrivateIncludePaths.Add(Path.Combine(EngineDir, @"Source/Runtime/VulkanRHI/Private/Windows"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				PrivateIncludePaths.Add(Path.Combine(EngineDir, @"Source/Runtime/VulkanRHI/Private/Android"));
			}
		}
	}
}
