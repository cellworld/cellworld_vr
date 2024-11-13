// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;

public class ViveOpenXRLoader : ModuleRules
{
	public ViveOpenXRLoader(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		//string LibDir = ModuleDirectory + "/lib";

		//if (Target.Platform == UnrealTargetPlatform.Android)
		//{
		//	PublicAdditionalLibraries.Add(LibDir + "/android/arm64-v8a/libopenxr_loader.so");
		//}
	}
}
