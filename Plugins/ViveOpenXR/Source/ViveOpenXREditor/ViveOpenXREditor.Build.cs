// Copyright HTC Corporation. All Rights Reserved.

using UnrealBuildTool;

public class ViveOpenXREditor : ModuleRules
{
    public ViveOpenXREditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "InputCore",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "EditorWidgets",
                "DesktopWidgets",
                "PropertyEditor",
                "UnrealEd",
                "SharedSettingsWidgets",
                "TargetPlatform",
                "RenderCore",
                "ViveOpenXRRuntimeSettings"
            }
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "Settings"
            }
        );
    }
}