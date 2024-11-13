// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class ViveCustomHandGesture : ModuleRules
	{
		public ViveCustomHandGesture(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			PrivateDependencyModuleNames.AddRange(
			new string[]
				{
					"CoreUObject",
					"Engine",
					"Core",
					"HeadMountedDisplay"
				}
			);
		}
	}
}
