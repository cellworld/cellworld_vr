// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRRuntimeSettings.h"
#include "Modules/ModuleInterface.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ViveOpenXRSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "FViveOpenXREditorModule"

class FViveOpenXREditorModule
    : public IModuleInterface
{
    virtual void StartupModule() override
    {

      // register settings detail panel customization
      FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
      PropertyModule.RegisterCustomClassLayout(
        UViveOpenXRRuntimeSettings::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FViveOpenXRSettingsCustomization::MakeInstance)
      );
      PropertyModule.NotifyCustomizationModuleChanged();

        ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

        if (SettingsModule != nullptr)
        {
            SettingsModule->RegisterSettings("Project", "Plugins", "ViveOpenXR",
                LOCTEXT("RuntimeSettingsName", "Vive OpenXR"),
                LOCTEXT("RuntimeSettingsDescription", "Project settings for Vive OpenXR plugins."),
                GetMutableDefault<UViveOpenXRRuntimeSettings>()
            );
        }
    }

    virtual void ShutdownModule() override
    {
        ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

        if (SettingsModule != nullptr)
        {
            SettingsModule->UnregisterSettings("Project", "Plugins", "ViveOpenXR");
        }
    }
};

IMPLEMENT_MODULE(FViveOpenXREditorModule, ViveOpenXREditor);

#undef LOCTEXT_NAMESPACE