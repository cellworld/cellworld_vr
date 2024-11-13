// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "OpenXRCommon.h"
#include "CoreMinimal.h"
#include "Tickable.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "XRMotionControllerBase.h"
#include "IOpenXRExtensionPlugin.h"
#include "IInputDeviceModule.h"
#include "IViveOpenXRXrTrackerModule.h"
#include "OpenXRCore.h"
#include "OpenXRHMD.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRXrTracker, Log, All);

class FViveOpenXRXrTracker :
	public IInputDevice,
	public IOpenXRExtensionPlugin,
	public FXRMotionControllerBase
{
public:
	struct FViveXrTracker
	{
	public:
		FViveXrTracker();
		XrPath XrTrackerUserPath; // XrPath SubactionPath
		XrPath xrTrackerPosePath;
		XrAction xrTrackerPoseAction;
		XrSpace XrTrackerSpace; // { XR_NULL_HANDLE };
		XrSpaceLocation XrTrackerSpaceLocation;
		TArray<XrPath> XrTrackerPaths;
		XrActionStatePose XrTrackerStatePose;
		int32 DeviceId;

		void CreateSpace(const XrSession InSession);
		void AddAction(XrActionSet& InActionSet, const FName& InName, const TArray<XrPath>& InSubactionPaths);
		void DestroySpace();

		int32 AddTrackedDevices(class FOpenXRHMD* HMD);
		void GetSuggestedBindings(TArray<XrActionSuggestedBinding>& OutSuggestedBindings);
		void SyncActionStates(XrSession InSession);
	};
public:
	FViveOpenXRXrTracker(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FViveOpenXRXrTracker();

	//OpenXRFunctions
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	//virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	//virtual void PostSyncActions(XrSession InSession) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
	//virtual void OnDestroySession(XrSession InSession) override;

	bool m_EnableXrTracker = false;
	class FOpenXRHMD* OpenXRHMD = nullptr;
	PFN_xrGetSystemProperties xrGetSystemProperties = nullptr;
	PFN_xrEnumeratePathsForInteractionProfileHTC xrEnumeratePathsForInteractionProfileHTC = nullptr;

	static inline FViveOpenXRXrTracker* GetInstance() {
		return m_Instance;
	}

	/************************************************************************/
	/* IMotionController                                                    */
	/************************************************************************/
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator & OutOrientation, FVector & OutPosition, bool& OutbProvidedLinearVelocity, FVector & OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector & OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector & OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual ETrackingStatus GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const override;
	virtual FName GetMotionControllerDeviceTypeName() const override;
	virtual void EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const override;
	//virtual bool SetPlayerMappableInputConfig(TObjectPtr<class UPlayerMappableInputConfig> InputConfig) override;

/************************************************************************/
/* IInputDevice                                                         */
/************************************************************************/
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override
	{
		MessageHandler = InMessageHandler;
	}
	virtual void SendControllerEvents() override {}
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override {}
	virtual void Tick(float DeltaTime) override {}
private:
	/** The recipient of motion controller input events */
	TSharedPtr< FGenericApplicationMessageHandler > MessageHandler;

protected:
	static FViveOpenXRXrTracker* m_Instance;

private:
	XrInstance Instance = XR_NULL_HANDLE;
	bool bSessionStarted = false;
	bool bActionsAttached = false;
	bool bPathEnumerated = false;
	int32 DeviceIndex = 0;
	XrPath xrTrackerInteractionProfile = XR_NULL_PATH;
	XrSession m_Session = XR_NULL_HANDLE;
	XrActionSet XrTrackerActionSet = XR_NULL_HANDLE;
	TArray<XrActionSuggestedBinding> XrTrackerBindings;
	TMap<FName, FViveXrTracker> viveXrTrackerMap;

	////////Tracker User Path//////////
	const TArray<FString> xrTrackerPaths =
	{ FString("/user/xr_tracker_htc/vive_ultimate_tracker_0") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_1") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_2") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_3") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_4") };

	const TArray<FString> xrTrackerPosePaths =
	{ FString("/user/xr_tracker_htc/vive_ultimate_tracker_0/input/entity_htc/pose") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_1/input/entity_htc/pose") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_2/input/entity_htc/pose") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_3/input/entity_htc/pose") ,
		FString("/user/xr_tracker_htc/vive_ultimate_tracker_4/input/entity_htc/pose") };
};

class FViveOpenXRXrTrackerModule : public IViveOpenXRXrTrackerModule
{
public:
	FViveOpenXRXrTrackerModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	FViveOpenXRXrTracker* GetXrTracker();

private:
	TSharedPtr<FViveOpenXRXrTracker> XrTracker;

};