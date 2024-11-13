// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRAnchorModule.h"
#include "OpenXRCore.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "ARBlueprintLibrary.h"
#include "IOpenXRARModule.h"
#include "ARPin.h"
#include <string>

DEFINE_LOG_CATEGORY(ViveOXRAnchor);

FOpenXRHMD* FViveOpenXRAnchor::hmd = nullptr;
FViveOpenXRAnchor* FViveOpenXRAnchor::instance = nullptr;

FOpenXRHMD* FViveOpenXRAnchor::HMD() {
	if (hmd != nullptr)
		return hmd;
	if (GEngine->XRSystem.IsValid())
	{
		hmd = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());
	}
	return hmd;
}

FViveOpenXRAnchor* FViveOpenXRAnchor::Instance()
{
	if (instance != nullptr)
	{
		return instance;
	}
	else
	{
		if (GEngine->XRSystem.IsValid() && HMD() != nullptr)
		{
			for (IOpenXRExtensionPlugin* Module : HMD()->GetExtensionPlugins())
			{
				if (Module->GetDisplayName() == TEXT("ViveOpenXRAnchor"))
				{
					instance = static_cast<FViveOpenXRAnchor*>(Module);
					break;
				}
			}
		}
		return instance;
	}
}


void FViveOpenXRAnchor::StartupModule()
{
	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableAnchor"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableAnchor = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableAnchor = true;
		}
	}

	if (m_bEnableAnchor)
	{
		UE_LOG(ViveOXRAnchor, Log, TEXT("Enable Anchor."));
		instance = this;
	}
	else
	{
		UE_LOG(ViveOXRAnchor, Log, TEXT("Disable Anchor."));
		instance = nullptr;
		return;
	}

	RegisterOpenXRExtensionModularFeature();
	UE_LOG(ViveOXRAnchor, Log, TEXT("StartupModule() Finished."));
}

void FViveOpenXRAnchor::ShutdownModule()
{
	instance = nullptr;
	UnregisterOpenXRExtensionModularFeature();
}

FString FViveOpenXRAnchor::GetDisplayName()
{
	return FString(TEXT("ViveOpenXRAnchor"));
}

bool FViveOpenXRAnchor::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableAnchor)
	{
		UE_LOG(ViveOXRAnchor, Log, TEXT("GetRequiredExtensions() Add Anchor Extension Name %s."), ANSI_TO_TCHAR(XR_HTC_ANCHOR_EXTENSION_NAME));
		OutExtensions.Add(XR_HTC_ANCHOR_EXTENSION_NAME);
	}
	return true;
}

const void* FViveOpenXRAnchor::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!m_bEnableAnchor) return InNext;

	UE_LOG(ViveOXRAnchor, Log, TEXT("Entry Anchor OnCreateSession."));

	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSpatialAnchorHTC", (PFN_xrVoidFunction*)&xrCreateSpatialAnchorHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSpatialAnchorNameHTC", (PFN_xrVoidFunction*)&xrGetSpatialAnchorNameHTC));

	XrSystemAnchorPropertiesHTC systemAnchorProperties = {};
	systemAnchorProperties.type = XR_TYPE_SYSTEM_ANCHOR_PROPERTIES_HTC;
	systemAnchorProperties.next = nullptr;
	XrSystemProperties systemProperties = {};
	systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	systemProperties.next = &systemAnchorProperties;
	XrResult result = xrGetSystemProperties(InInstance, InSystem, &systemProperties);
	if (XR_FAILED(result))
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("OnCreateSession() xrGetSystemProperties failed with result %d."), result);
	}
	else
	{
		isAnchorSupported = systemAnchorProperties.supportsAnchor > 0;
		UE_LOG(ViveOXRAnchor, Log, TEXT("OnCreateSession() Is Anchor support: %d."), systemAnchorProperties.supportsAnchor);
	}

	return InNext;
}

void FViveOpenXRAnchor::PostCreateSession(XrSession InSession)
{
	UE_LOG(ViveOXRAnchor, Log, TEXT("Entry Anchor PostCreateSession InSession %lu."), InSession);
	m_Session = InSession;
}

// This will be called every frame.
const void* FViveOpenXRAnchor::OnSyncActions(XrSession InSession, const void* InNext)
{
	return InNext;
}

IOpenXRCustomAnchorSupport* FViveOpenXRAnchor::GetCustomAnchorSupport()
{
	return this;
}

bool FViveOpenXRAnchor::OnPinComponent(UARPin* Pin, XrSession InSession, XrSpace TrackingSpace, XrTime DisplayTime, float worldToMeterScale)
{
	auto t = Pin->GetLocalToTrackingTransform();
	auto loc = t.GetLocation();
	auto rot = t.GetRotation();
	auto name = Pin->GetDebugName().ToString();
	XrSpatialAnchorCreateInfoHTC createInfo = MakeCreateInfo(loc, rot, TrackingSpace, name, worldToMeterScale);
	XrSpace anchor = 0;
	if (!CreateSpatialAnchor(&createInfo, &anchor)) return false;
	Pin->SetNativeResource(anchor);
	return true;
}

void FViveOpenXRAnchor::OnRemovePin(UARPin* Pin)
{
	XrSpace anchor = (XrSpace)Pin->GetNativeResource();
	xrDestroySpace(anchor);
	Pin->SetPinnedComponent(nullptr);
	Pin->SetNativeResource(nullptr);
}

void FViveOpenXRAnchor::OnUpdatePin(UARPin* Pin, XrSession InSession, XrSpace TrackingSpace, XrTime DisplayTime, float worldToMeterScale)
{
	FRotator rotation;
	FVector translation;
	if (!LocateAnchor((XrSpace)Pin->GetNativeResource(), rotation, translation))
		return;
	Pin->OnTransformUpdated(FTransform(rotation, translation, FVector(1, 1, 1)));
}

bool FViveOpenXRAnchor::IsLocalPinSaveSupported() const
{
	// Wait for persistent spatial anchor extension
	return false;
}

bool FViveOpenXRAnchor::ArePinsReadyToLoad()
{
	return false;
}

void FViveOpenXRAnchor::LoadARPins(XrSession InSession, TFunction<UARPin* (FName)> OnCreatePin)
{
}

bool FViveOpenXRAnchor::SaveARPin(XrSession InSession, FName InName, UARPin* InPin)
{
	return false;
}

void FViveOpenXRAnchor::RemoveSavedARPin(XrSession InSession, FName InName)
{
}

void FViveOpenXRAnchor::RemoveAllSavedARPins(XrSession InSession)
{
}

bool FViveOpenXRAnchor::CreateSpatialAnchor(const XrSpatialAnchorCreateInfoHTC* createInfo, XrSpace* anchor)
{
	UE_LOG(ViveOXRAnchor, Log, TEXT("CreateSpatialAnchor()"));

	if (xrCreateSpatialAnchorHTC == nullptr)
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("CreateSpatialAnchor() xrCreateSpatialAnchorHTC is nullptr."));
		return false;
	}

	if (anchor == nullptr)
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("CreateSpatialAnchor() anchor is nullptr."));
		return false;
	}

	*anchor = 0;

	XrResult result = xrCreateSpatialAnchorHTC(m_Session, createInfo, anchor);
	if (XR_FAILED(result))
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("CreateSpatialAnchor() xrCreateSpatialAnchorHTC failed with result %d."), result);
		return false;
	}
	return true;
}

bool FViveOpenXRAnchor::GetSpatialAnchorName(XrSpace anchor, XrSpatialAnchorNameHTC* name)
{
	if (xrGetSpatialAnchorNameHTC == nullptr)
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("GetSpatialAnchorName() xrGetSpatialAnchorNameHTC is nullptr."));
		return false;
	}

	if (name == nullptr)
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("GetSpatialAnchorName() name is nullptr."));
		return false;
	}

	name->name[0] = 0;

	XrResult result = xrGetSpatialAnchorNameHTC(anchor, name);
	if (XR_FAILED(result))
	{
		UE_LOG(ViveOXRAnchor, Error, TEXT("GetSpatialAnchorName() xrGetSpatialAnchorNameHTC failed with result %d."), result);
		return false;
	}
	return true;
}

XrSpatialAnchorCreateInfoHTC FViveOpenXRAnchor::MakeCreateInfo(const FVector& loc, const FQuat& rot, XrSpace baseSpace, FString name, float worldToMeterScale)
{
	XrSpatialAnchorCreateInfoHTC createInfo = {};
	createInfo.type = XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_HTC;
	createInfo.next = nullptr;
	createInfo.space = baseSpace;
	createInfo.poseInSpace = XrPosef{ ToXrQuat(rot), ToXrVector(loc, worldToMeterScale) };
	createInfo.name.name[0] = 0;
	std::string str = TCHAR_TO_ANSI(*name);
	int l = str.length();
	size_t m = l < XR_MAX_SPATIAL_ANCHOR_NAME_SIZE_HTC - 1 ? l : XR_MAX_SPATIAL_ANCHOR_NAME_SIZE_HTC - 1;

#if PLATFORM_ANDROID
	strncpy(createInfo.name.name, str.c_str(), m);
#else
	strncpy_s(createInfo.name.name, str.c_str(), m);
#endif
	createInfo.name.name[m] = 0;

	return createInfo;
}

bool FViveOpenXRAnchor::LocateAnchor(XrSpace anchor, FRotator& rotation, FVector& translation)
{
	if (!HMD()) return false;
	XrTime time = hmd->GetDisplayTime();
	XrSpace baseSpace = hmd->GetTrackingSpace();

	XrSpaceLocation loc{};

	auto result = xrLocateSpace((XrSpace)anchor, baseSpace, time, &loc);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRAnchor, Error, TEXT("LocateAnchor() xrLocateSpace failed.  result=%d."), result);
		return false;
	}

	if ((loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) == 0 ||
		(loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) == 0) {
		UE_LOG(ViveOXRAnchor, Error, TEXT("LocateAnchor() locationFlags has not valid bits. Flags=%016llX"), loc.locationFlags);
		return false;
	}

	translation = ToFVector(loc.pose.position, hmd->GetWorldToMetersScale());
	auto rot = ToFQuat(loc.pose.orientation);
	rot.Normalize();
	rotation = FRotator(rot);
	return true;
}

IMPLEMENT_MODULE(FViveOpenXRAnchor, ViveOpenXRAnchor)
