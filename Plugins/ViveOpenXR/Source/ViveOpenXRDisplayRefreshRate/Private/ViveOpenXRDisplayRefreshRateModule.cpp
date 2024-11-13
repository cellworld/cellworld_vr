// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRDisplayRefreshRateModule.h"
#include "OpenXRCore.h"
#include "ViveOpenXRDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogViveOpenXRDisplayRefreshRate);

void FViveOpenXRDisplayRefreshRate::StartupModule()
{
	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableDisplayRefreshRate"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableDisplayRefreshRate = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableDisplayRefreshRate = true;
		}
	}

	if (m_bEnableDisplayRefreshRate)
	{
		UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("Enable DisplayRefreshRate."));
	}
	else
	{
		UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("Disable DisplayRefreshRate."));
		return;
	}

	RegisterOpenXRExtensionModularFeature();
	UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("StartupModule() Finished."));
}

bool FViveOpenXRDisplayRefreshRate::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableDisplayRefreshRate)
	{
		UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("GetRequiredExtensions() Add DisplayRefreshRate Extension Name %s."), ANSI_TO_TCHAR(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME));
		OutExtensions.Add(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME);
	}
	return true;
}

void FViveOpenXRDisplayRefreshRate::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (InHeader->type == XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB) {
		const XrEventDataDisplayRefreshRateChangedFB& EventDataDisplayRefreshRateChanged = *reinterpret_cast<const XrEventDataDisplayRefreshRateChangedFB*>(InHeader);
		FViveOpenXRDelegates::DisplayRefreshRateChanged.Broadcast(EventDataDisplayRefreshRateChanged.fromDisplayRefreshRate, EventDataDisplayRefreshRateChanged.toDisplayRefreshRate);
		UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("OnEvent XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB from: %f to %f"), EventDataDisplayRefreshRateChanged.fromDisplayRefreshRate , EventDataDisplayRefreshRateChanged.toDisplayRefreshRate);
	}
}

const void* FViveOpenXRDisplayRefreshRate::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!m_bEnableDisplayRefreshRate) return InNext;

	UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("Entry DisplayRefreshRate OnCreateSession."));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateDisplayRefreshRatesFB", (PFN_xrVoidFunction*)&xrEnumerateDisplayRefreshRatesFB));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetDisplayRefreshRateFB", (PFN_xrVoidFunction*)&xrGetDisplayRefreshRateFB));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrRequestDisplayRefreshRateFB", (PFN_xrVoidFunction*)&xrRequestDisplayRefreshRateFB));

	return InNext;
}

void FViveOpenXRDisplayRefreshRate::PostCreateSession(XrSession InSession)
{
	UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("Entry DisplayRefreshRate PostCreateSession InSession %lu."), InSession);
	m_Session = InSession;
}

bool FViveOpenXRDisplayRefreshRate::EnumerateDisplayRefreshRates(TArray<float>& OutDisplayRefreshRates)
{
	if (!m_bEnableDisplayRefreshRate || xrEnumerateDisplayRefreshRatesFB == nullptr) return false;

	if (m_DisplayRefreshRates.Num() == 0)
	{
		uint32_t DisplayRefreshRateCount = 0;
		XR_ENSURE(xrEnumerateDisplayRefreshRatesFB(m_Session, 0, &DisplayRefreshRateCount, nullptr));
		if (DisplayRefreshRateCount > 0)
		{
			m_DisplayRefreshRates.Init(0.0f, DisplayRefreshRateCount);
			XR_ENSURE(xrEnumerateDisplayRefreshRatesFB(m_Session, DisplayRefreshRateCount, &DisplayRefreshRateCount, m_DisplayRefreshRates.GetData()));
			for (size_t x = 0; x < m_DisplayRefreshRates.Num(); x++) {
				UE_LOG(LogViveOpenXRDisplayRefreshRate, Log, TEXT("DisplayRefreshRates index %d is %f"), x, m_DisplayRefreshRates[x]);
			}
		}
		else
		{
			UE_LOG(LogViveOpenXRDisplayRefreshRate, Warning, TEXT("DisplayRefreshRateCount is zero"));
			return false;
		}
	}
	OutDisplayRefreshRates = m_DisplayRefreshRates;
	return !OutDisplayRefreshRates.IsEmpty();
}

bool FViveOpenXRDisplayRefreshRate::GetDisplayRefreshRate(float& OutDisplayRefreshRate)
{
	if (!m_bEnableDisplayRefreshRate || xrGetDisplayRefreshRateFB == nullptr) return false;
	return XR_ENSURE(xrGetDisplayRefreshRateFB(m_Session, &OutDisplayRefreshRate));
}

bool FViveOpenXRDisplayRefreshRate::RequestDisplayRefreshRate(float InDisplayRefreshRate)
{
	if (!m_bEnableDisplayRefreshRate || xrRequestDisplayRefreshRateFB == nullptr) return false;
	return XR_ENSURE(xrRequestDisplayRefreshRateFB(m_Session, InDisplayRefreshRate));
}

IMPLEMENT_MODULE(FViveOpenXRDisplayRefreshRate, ViveOpenXRDisplayRefreshRate)
