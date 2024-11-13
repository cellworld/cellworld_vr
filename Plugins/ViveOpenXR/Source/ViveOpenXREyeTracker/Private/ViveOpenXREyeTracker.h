// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "IViveOpenXREyeTrackerModule.h"
#include "IEyeTracker.h"
#include "IOpenXRExtensionPlugin.h"
#include "HTCEyeTrackerTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXREyeTracker, Log, All);

class AHUD;
class FDebugDisplayInfo;
class UCanvas;

struct FHTCEyeTrackerExtesionDispatchTable
{
	PFN_xrGetSystemProperties xrGetSystemProperties;
	PFN_xrCreateEyeTrackerHTC xrCreateEyeTrackerHTC;
	PFN_xrDestroyEyeTrackerHTC xrDestroyEyeTrackerHTC;
	PFN_xrGetEyeGazeDataHTC xrGetEyeGazeDataHTC;
	PFN_xrGetEyePupilDataHTC xrGetEyePupilDataHTC;
	PFN_xrGetEyeGeometricDataHTC xrGetEyeGeometricDataHTC;
};

class FViveOpenXREyeTracker : public IEyeTracker, public IOpenXRExtensionPlugin
{
public:
	FViveOpenXREyeTracker();
	virtual ~FViveOpenXREyeTracker();

	/************************************************************************/
	/* IEyeTracker                                                          */
	/************************************************************************/
	void Destroy();
	virtual void SetEyeTrackedPlayer(APlayerController* PlayerController) override {};
	virtual bool GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const override;
	virtual bool GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override;
	virtual EEyeTrackerStatus GetEyeTrackerStatus() const override;
	virtual bool IsStereoGazeDataAvailable() const override;
	
	/************************************************************************/
	/* IOpenXRExtensionPlugin                                               */
	/************************************************************************/

	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ViveOpenXREyeTracker"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;
	virtual void PostSyncActions(XrSession InSession) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;

	void DrawDebug(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

public:
	XrEyeTrackerHTC EyeTrackerHTC = XR_NULL_HANDLE;

	XrEyeGazeDataHTC EyeGazesHTC{ XR_TYPE_EYE_GAZE_DATA_HTC, nullptr };
	XrEyePupilDataHTC PupilDatasHTC{ XR_TYPE_EYE_PUPIL_DATA_HTC, nullptr };
	XrEyeGeometricDataHTC EyeGeometricDatasHTC{ XR_TYPE_EYE_GEOMETRIC_DATA_HTC, nullptr };

	FXrGazeDataHTC GetEyeGazeValidDatas();
	FXrPupilDataHTC GetPupilDatas();
	FXrEyeGeometricDataHTC GetEyeGeometricDatas();

private:
	bool isHTCEyeTrackerSupported = false;
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	XrInstance Instance = XR_NULL_HANDLE;
	XrSession Session = XR_NULL_HANDLE;
	XrSystemId SystemId;
	bool bSessionStarted = false;
	XrActionsSyncInfo SyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
	XrAction EyeTrackerAction = XR_NULL_HANDLE;
	XrActionSet EyeTrackerActionSet = XR_NULL_HANDLE;
	XrSpace GazeActionSpace = XR_NULL_HANDLE;
	XrActionStatePose ActionStatePose{ XR_TYPE_ACTION_STATE_POSE };

	// EyeTracker cached data
	XrSpaceLocation EyeTrackerSpaceLocation{ XR_TYPE_SPACE_LOCATION };

	FHTCEyeTrackerExtesionDispatchTable FHTCEyeTracker_ext{};
};

class FViveOpenXREyeTrackerModule : public IViveOpenXREyeTrackerModule
{
	/************************************************************************/
	/* IInputDeviceModule                                                   */
	/************************************************************************/
public:
	FViveOpenXREyeTrackerModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > CreateEyeTracker() override;

	/************************************************************************/
	/* IEyeTrackerModule													*/
	/************************************************************************/

	virtual bool IsEyeTrackerConnected() const override;

private:
	TSharedPtr<FViveOpenXREyeTracker, ESPMode::ThreadSafe> EyeTracker;
	FDelegateHandle OnDrawDebugHandle;

	void OnDrawDebug(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
