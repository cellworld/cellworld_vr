// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MiscUtils : ModuleRules
{
	public MiscUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// "MiscUtils/Source/Timer"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
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
				"JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		PublicDependencyModuleNames.AddRange(new string[]  {
			"Core", "CoreUObject", 
			"Engine", 
			"InputCore",
			"Slate", "SlateCore"
		});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
