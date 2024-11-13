// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRRuntimeSettings.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY(LogViveOpenXRSettings);

IMPLEMENT_MODULE(FDefaultModuleImpl, ViveOpenXRRuntimeSettings);

UViveOpenXRRuntimeSettings* UViveOpenXRRuntimeSettings::ViveOpenXRRuntimeSettingsSingleton = nullptr;

void UViveOpenXRRuntimeSettings::PostInitProperties()
{
        Super::PostInitProperties();

	TSharedPtr<IPlugin> OpenXRHandTracking = IPluginManager::Get().FindPlugin(FString(TEXT("OpenXRHandTracking")));
	bEnableHandTracking = (OpenXRHandTracking.IsValid() && OpenXRHandTracking->IsEnabled()) ? true : false;

	TSharedPtr<IPlugin> OpenXREyeTracker = IPluginManager::Get().FindPlugin(FString(TEXT("OpenXREyeTracker")));
	bEnableEyeTracking = (OpenXREyeTracker.IsValid() && OpenXREyeTracker->IsEnabled()) ? true : false;

	TryUpdateDefaultConfigFile();
}

UViveOpenXRRuntimeSettings* UViveOpenXRRuntimeSettings::Get()
{
	if (ViveOpenXRRuntimeSettingsSingleton == nullptr && GetTransientPackage() != nullptr)
	{
		static const TCHAR* SettingsContainerName = TEXT("ViveOpenXRRuntimeSettingsContainer");

		ViveOpenXRRuntimeSettingsSingleton = FindObject<UViveOpenXRRuntimeSettings>((UObject*)GetTransientPackage(), SettingsContainerName);

		if (ViveOpenXRRuntimeSettingsSingleton == nullptr)
		{
			ViveOpenXRRuntimeSettingsSingleton = NewObject<UViveOpenXRRuntimeSettings>(
				(UObject*)GetTransientPackage(), UViveOpenXRRuntimeSettings::StaticClass(), SettingsContainerName);
			ViveOpenXRRuntimeSettingsSingleton->AddToRoot();
		}
	}
	return ViveOpenXRRuntimeSettingsSingleton;
}

#if WITH_EDITOR
void UViveOpenXRRuntimeSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	GConfig->Flush(false);
}
#endif
