// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;

public class ViveOpenXRRuntimeSettings : ModuleRules
{
    public ViveOpenXRRuntimeSettings(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Projects"
            }
        );

        if (Target.Type == TargetRules.TargetType.Editor || Target.Type == TargetRules.TargetType.Program)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "TargetPlatform"
                }
            );
        }
    }
}
