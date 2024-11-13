// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include "IOpenXRExtensionPlugin.h"
#include "Modules/ModuleInterface.h"
#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRHMD, Log, All);

class FViveOpenXRHMD : public IOpenXRExtensionPlugin, public IModuleInterface
{

public:
	FViveOpenXRHMD() : LoaderHandle(nullptr) {}

	virtual ~FViveOpenXRHMD(){}

	virtual void StartupModule() override
	{
		RegisterOpenXRExtensionModularFeature();

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ViveOpenXR"));
		if (Plugin.IsValid())
		{
			UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR plugin version: %s"), *Plugin->GetDescriptor().VersionName);
		}
	}

	virtual void ShutdownModule() override
	{
		if (LoaderHandle)
		{
			FPlatformProcess::FreeDllHandle(LoaderHandle);
			LoaderHandle = nullptr;
		}
		UnregisterOpenXRExtensionModularFeature();
	}

	virtual FString GetDisplayName() override
	{
		return FString(TEXT("Vive OpenXR"));
	}

	virtual bool GetCustomLoader(PFN_xrGetInstanceProcAddr* OutGetProcAddr) override;
	virtual bool IsStandaloneStereoOnlyDevice() override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;
	virtual bool GetSpectatorScreenController(FHeadMountedDisplayBase* InHMDBase, TUniquePtr<FDefaultSpectatorScreenController>& OutSpectatorScreenController) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	virtual const void* OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext) override;
	virtual const void* OnGetSystem(XrInstance InInstance, const void* InNext) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual const void* OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext) override;
	virtual const void* OnBeginProjectionView(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext) override;
	virtual const void* OnBeginDepthInfo(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext) override;
	virtual const void* OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags) override;
	virtual const void* OnEndFrame(XrSession InSession, XrTime DisplayTime, const void* InNext) override;
	virtual const void* OnSyncActions(XrSession InSession, const void* InNext) override;
	virtual void PostSyncActions(XrSession InSession) override;

private:
	void *LoaderHandle;
};
