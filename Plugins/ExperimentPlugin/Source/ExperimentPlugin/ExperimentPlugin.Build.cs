// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExperimentPlugin : ModuleRules
{
	public ExperimentPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"TCPMessages",
				"XRBase",
				"MiscUtils",
				"OculusXRHMD",
				"OculusXRAnchors",
				"OculusXRPassthrough"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Networking",
				"Sockets",
				"InputCore",
				"Json",
				"JsonUtilities",
				"TCPMessages",
				"UMG",
				"HeadMountedDisplay",
				// "OculusXRAnchors",
				// "OculusXRHMD",
				// "OculusXRPassthrough",
				// "OpenXRAR"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
