// Copyright HTC Corporation. All Rights Reserved.

#pragma once

// Must include earlier than Unreal's OpenXR header
#include "ViveOpenXRWrapper.h"
#include "IOpenXRExtensionPlugin.h"
#include "OpenXRHMD.h"

DECLARE_LOG_CATEGORY_EXTERN(ViveOXRAnchor, Log, All);

class FViveOpenXRAnchor : public IModuleInterface, public IOpenXRExtensionPlugin, public IOpenXRCustomAnchorSupport
{
public:
	FViveOpenXRAnchor(){}
	virtual ~FViveOpenXRAnchor(){}

	static FOpenXRHMD* HMD();
	static FViveOpenXRAnchor* Instance();

	/* IModuleInterface */

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IOpenXRExtensionPlugin implementation */

	virtual FString GetDisplayName() override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual const void* OnSyncActions(XrSession InSession, const void* InNext) override;

	virtual IOpenXRCustomAnchorSupport* GetCustomAnchorSupport() override;

	/** IOpenXRCustomAnchorSupport implementation */
	virtual bool OnPinComponent(class UARPin* Pin, XrSession InSession, XrSpace TrackingSpace, XrTime DisplayTime, float worldToMeterScale) override;
	virtual void OnRemovePin(class UARPin* Pin) override;
	virtual void OnUpdatePin(class UARPin* Pin, XrSession InSession, XrSpace TrackingSpace, XrTime DisplayTime, float worldToMeterScale) override;
	virtual bool IsLocalPinSaveSupported() const override;
	virtual bool ArePinsReadyToLoad() override;
	virtual void LoadARPins(XrSession InSession, TFunction<UARPin* (FName)> OnCreatePin) override;
	virtual bool SaveARPin(XrSession InSession, FName InName, UARPin* InPin) override;
	virtual void RemoveSavedARPin(XrSession InSession, FName InName) override;
	virtual void RemoveAllSavedARPins(XrSession InSession) override;

public:
	// Extension functions
	bool CreateSpatialAnchor(const XrSpatialAnchorCreateInfoHTC* createInfo, XrSpace* anchor);
	bool GetSpatialAnchorName(XrSpace anchor, XrSpatialAnchorNameHTC* name);

public:
	// Helper functions
	XrSpatialAnchorCreateInfoHTC MakeCreateInfo(const FVector& InLocation, const FQuat& InRotation, XrSpace baseSpace, FString name, float worldToMeterScale);
	bool LocateAnchor(XrSpace anchor, FRotator& rotation, FVector& translation);
	inline bool IsSupported() { return isAnchorSupported; }

private:
	static FOpenXRHMD* hmd;
	static FViveOpenXRAnchor* instance;
	bool m_bEnableAnchor = false;
	XrSession m_Session = XR_NULL_HANDLE;
	bool isAnchorSupported = false;

	/* OpenXR Function Ptrs */
	PFN_xrCreateSpatialAnchorHTC xrCreateSpatialAnchorHTC = nullptr;
	PFN_xrGetSpatialAnchorNameHTC xrGetSpatialAnchorNameHTC = nullptr;
};
