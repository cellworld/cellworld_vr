// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "IOpenXRExtensionPlugin.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRDisplayRefreshRate, Log, All);

class FViveOpenXRDisplayRefreshRate : public IModuleInterface, public IOpenXRExtensionPlugin
{
public:
	FViveOpenXRDisplayRefreshRate(){}
	virtual ~FViveOpenXRDisplayRefreshRate(){}

	/************************************************************************/
	/* IModuleInterface                                                     */
	/************************************************************************/
	virtual void StartupModule() override;
	virtual void ShutdownModule() override
	{
		UnregisterOpenXRExtensionModularFeature();
	}

	/** IOpenXRExtensionPlugin implementation */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ViveOpenXRDisplayRefreshRate"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void PostCreateSession(XrSession InSession) override;

public:
	bool EnumerateDisplayRefreshRates(TArray<float>& OutDisplayRefreshRates);
	bool GetDisplayRefreshRate(float& OutDisplayRefreshRate);
	bool RequestDisplayRefreshRate(float InDisplayRefreshRate);

private:
	bool m_bEnableDisplayRefreshRate = false;
	XrSession m_Session = XR_NULL_HANDLE;

	TArray<float> m_DisplayRefreshRates;

	//OpenXR Function Ptrs
	PFN_xrEnumerateDisplayRefreshRatesFB xrEnumerateDisplayRefreshRatesFB = nullptr;
	PFN_xrGetDisplayRefreshRateFB xrGetDisplayRefreshRateFB = nullptr;
	PFN_xrRequestDisplayRefreshRateFB xrRequestDisplayRefreshRateFB = nullptr;
};
