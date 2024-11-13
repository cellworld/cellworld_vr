// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "vulkan_core.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveRenderDoc, Log, All);

class FViveRenderDoc : public IOpenXRExtensionPlugin, public IModuleInterface
{
public:
	FViveRenderDoc();
	virtual ~FViveRenderDoc(){}

	// IModuleInterface
	virtual void StartupModule() override { RegisterOpenXRExtensionModularFeature(); }
	virtual void ShutdownModule() override { UnregisterOpenXRExtensionModularFeature(); }

	// IOpenXRExtensionPlugin
	virtual FString GetDisplayName() override { return FString(TEXT("ViveRenderDoc")); }
	virtual const void* OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext) override;
private:
	void InsertDebugUtilsLabel();
	bool IsLaunchByRenderDoc();
	void DumpVulkanInfos();
private:
	PFN_vkCmdInsertDebugUtilsLabelEXT InsertDebugUtilsLabelEXT = nullptr;
};
