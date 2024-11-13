// Copyright HTC Corporation. All Rights Reserved.

#pragma once

// Must include earlier than Unreal's OpenXR header
#include "ViveOpenXRWrapper.h"
#include "IOpenXRARModule.h"
#include "IOpenXRExtensionPlugin.h"
#include "IOpenXRARTrackedGeometryHolder.h"
#include "OpenXRHMD.h"
#include "ViveOpenXRPlaneDetectionFunctionLibrary.h"

DECLARE_LOG_CATEGORY_EXTERN(ViveOXRPlaneDetection, Log, All);

inline FGuid PlaneIdToFGuid(const uint64_t& planeId)
{
	FGuid OutGuid;
	FMemory::Memcpy(&OutGuid, &planeId, sizeof(FGuid));
	return OutGuid;
}

class FViveOpenXRPlaneDetection : public IModuleInterface, public IOpenXRExtensionPlugin, public IOpenXRCustomCaptureSupport
{
public:
	FViveOpenXRPlaneDetection(){}
	virtual ~FViveOpenXRPlaneDetection(){}

	static FOpenXRHMD* HMD();
	static FViveOpenXRPlaneDetection* Instance();

	/* IModuleInterface */

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IOpenXRExtensionPlugin implementation */

	virtual FString GetDisplayName() override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;		
	virtual void PostCreateSession(XrSession InSession) override;
	virtual void OnStartARSession(class UARSessionConfig* SessionConfig) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual IOpenXRCustomCaptureSupport* GetCustomCaptureSupport(const EARCaptureType CaptureType) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
	virtual const void* OnSyncActions(XrSession InSession, const void* InNext) override;	

public:
	bool IsSupportPlaneDetection() const { return isPlaneDetectionSupported; }
	bool CreatePlaneDetector(const XrPlaneDetectorCreateInfoEXT* createInfo, XrPlaneDetectorEXT* planeDetector);
	bool DestroyPlaneDetector(XrPlaneDetectorEXT planeDetector);
	bool BeginPlaneDetection(XrPlaneDetectorEXT planeDetector, const XrPlaneDetectorBeginInfoEXT* beginInfo);
	bool GetPlaneDetectionState(XrPlaneDetectorEXT planeDetector, XrPlaneDetectionStateEXT* state);
	bool GetPlaneDetections(XrPlaneDetectorEXT planeDetector, const XrPlaneDetectorGetInfoEXT* info, XrPlaneDetectorLocationsEXT* locations);
	bool GetPlanePolygonBuffer(XrPlaneDetectorEXT planeDetector, uint64_t planeId, uint32_t polygonBufferIndex, XrPlaneDetectorPolygonBufferEXT* polygonBuffer);

private:
	static FOpenXRHMD* hmd;
	static FViveOpenXRPlaneDetection* instance;
	static bool isPlaneDetectionSupported;
	bool m_bEnablePlaneDetection = false;
	XrSession m_Session = XR_NULL_HANDLE;
	
	XrPlaneDetectorEXT m_PlaneDetector = XR_NULL_HANDLE;
		
	bool m_bPlaneDetectionBegin = false;
	bool m_bHorizontalPlaneDetection = false;
	bool m_bVerticalPlaneDetection = false;	
	TArray<FGuid> m_PlaneLocationGuids;

	/* OpenXR Function Ptrs */

	PFN_xrCreatePlaneDetectorEXT xrCreatePlaneDetectorEXT = nullptr;
	PFN_xrDestroyPlaneDetectorEXT xrDestroyPlaneDetectorEXT = nullptr;
	PFN_xrBeginPlaneDetectionEXT xrBeginPlaneDetectionEXT = nullptr;
	PFN_xrGetPlaneDetectionStateEXT xrGetPlaneDetectionStateEXT = nullptr;
	PFN_xrGetPlaneDetectionsEXT xrGetPlaneDetectionsEXT = nullptr;
	PFN_xrGetPlanePolygonBufferEXT xrGetPlanePolygonBufferEXT = nullptr;

	/*OpenXRAR Function*/

	IOpenXRARTrackedMeshHolder* m_TrackedMeshHolder = nullptr;

	/* Foreground delegate for resume. */

	void OnApplicationResume();
};
