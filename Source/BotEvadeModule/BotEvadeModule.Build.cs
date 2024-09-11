using UnrealBuildTool; 

public class BotEvadeModule : ModuleRules //Replace class name with your module name 
{
    // ReSharper disable once InconsistentNaming
    public BotEvadeModule(ReadOnlyTargetRules Target) : base(Target) // Replace constructor name with your module name 
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", 
            "InputCore", 
            "EnhancedInput", "HeadMountedDisplay", "UMG", "Slate","SlateCore",
            "OnlineSubsystem","OnlineSubsystemNull", "OnlineSubsystemUtils"
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] { "XRBase"  });

    }
    

}

// Make sure to add whatever module name you use here in your:
// - MyProject.uproject module dependency list (In the project Root)
// AND
// - MyProject.Build.cs file module dependency list (Should be in Source/MyProject/MyProject.Build.cs)

