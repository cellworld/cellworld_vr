// (c) Copyright 2019-2021 HP Development Company, L.P.

using System;
using System.Diagnostics;
using System.IO;
using UnrealBuildTool;

public class HPGlia : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get
        {
            String gliaInstallPath = Environment.GetEnvironmentVariable("HP_OMNICEPT_INSTALL");
            return Path.GetFullPath(gliaInstallPath);
        }
    }

    private const string libzmqDll = "libzmq-mt-4_3_3.dll";
    private const string jsoncppDll = "jsoncpp.dll";

    public HPGlia(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
                Path.Combine(ThirdPartyPath, "include"),
            }
            );

        System.Console.WriteLine(PublicIncludePaths);

        //Don't put any paths in here, just *.dll name
        PublicDelayLoadDLLs.Add(libzmqDll);
        PublicDelayLoadDLLs.Add(jsoncppDll);

        PublicSystemLibraries.Add("wintrust.lib");
        PublicSystemLibraries.Add("bcrypt.lib");
        PublicSystemLibraries.Add("crypt32.lib");

        try
        {
            foreach (var dir in Directory.GetDirectories(Path.Combine(ThirdPartyPath, "lib", "Release"), "msvc*", SearchOption.TopDirectoryOnly))
            {
                PublicAdditionalLibraries.Add(Path.Combine(dir, "hp_omnicept.lib"));
            }
        }
        catch (System.Exception e)
        {
            Debug.WriteLine("Failed to locate Omnicept library: {0}", e.ToString());
            throw;
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
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
                "Projects",
                // ... add private dependencies that you statically link with here ...
            }
            );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string basePath = Path.GetDirectoryName(RulesCompiler.GetFileNameFromType(GetType())); //Plugin source folder
            string win64DllPath = System.IO.Path.Combine(basePath, "..", "..", "Binaries", "Win64");

            RuntimeDependencies.Add(Path.Combine(win64DllPath, libzmqDll));
            RuntimeDependencies.Add(Path.Combine(win64DllPath, jsoncppDll));
        }
    }
}
