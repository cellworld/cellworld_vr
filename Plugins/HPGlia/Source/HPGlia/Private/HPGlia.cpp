// (c) Copyright 2019-2021 HP Development Company, L.P.

#include "HPGlia.h"
#include "omnicept/Glia.h"
#include "Engine.h"
#include "Core.h"
#include "HPGliaSettings.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif
#include <set>

#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FHPGliaModule"

void FHPGliaModule::StartupModule()
{
    dllsToLoad.push_back( { "libzmq-mt-4_3_3.dll", nullptr } );
    dllsToLoad.push_back( { "log4cplusU.dll", nullptr } );
    dllsToLoad.push_back( { "jsoncpp.dll", nullptr } );

    for (auto dll : dllsToLoad)
    {
        FString path = "";

        path = SearchForDllPath(FPaths::ProjectPluginsDir(), dll.name);

        if (path.IsEmpty())
        {
            path = SearchForDllPath(FPaths::EnginePluginsDir(), dll.name);
        }

        if (path.IsEmpty())
        {
            //Stop loading - plugin required DLL to load successfully
            checkf(false, TEXT("Failed to load dll"));
        }
        else
        {
            dll.handle = FPlatformProcess::GetDllHandle(*path);
        }
    }

    HP::Omnicept::Glia::startModule();
    RegisterSettings();
}

void FHPGliaModule::RegisterSettings()
{
#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings("Project", "Plugins", "HPGliaSettings",
            LOCTEXT("HPGliaSettingsName", "HP Glia Settings"),
            LOCTEXT("HPGliaSettingsDescription", "Configure global settings for the Hp Glia"),
            GetMutableDefault<UHPGliaSettings>());
    }
#endif
}

/**
 * Solution pulled from:
 * https://answers.unrealengine.com/questions/401885/delay-load-dlls-causing-breakpoint-1.html
 */
FString FHPGliaModule::SearchForDllPath(FString _searchBase, FString _dllName)
{
    TArray<FString> directoriesToSkip;
    IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
    PlatformFile.IterateDirectory(*_searchBase, Visitor);

    for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
    {
        const FString file = TimestampIt.Key();
        const FString filePath = FPaths::GetPath(file);
        const FString fileName = FPaths::GetCleanFilename(file);
        if (fileName.Compare(_dllName) == 0)
        {
            return file;
        }
    }
    return "";
}

void FHPGliaModule::ShutdownModule()
{
    HP::Omnicept::Glia::stopModule();

    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    for (auto dll : dllsToLoad)
    {
        FPlatformProcess::FreeDllHandle(dll.handle);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHPGliaModule, HPGlia)
