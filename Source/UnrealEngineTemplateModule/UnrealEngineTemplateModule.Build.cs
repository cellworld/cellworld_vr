using UnrealBuildTool; 

public class UnrealEngineTemplateModule : ModuleRules //Replace class name with your module name 
{ 
    public UnrealEngineTemplateModule(ReadOnlyTargetRules Target) : base(Target) //Replace constructor name with your module name 
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"}); //Add any extra dependencies here
    }
}

// Make sure to add whatever module name you use here in your:
// - MyProject.uproject module dependency list (In the project Root)
// AND
// - MyProject.Build.cs file module dependency list (Should be in Source/MyProject/MyProject.Build.cs)

