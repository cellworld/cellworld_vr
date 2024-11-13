// Copyright Epic Games, Inc. All Rights Reserved.

#include "ViveOpenXREyeTracker.h"
#include "EyeTrackerTypes.h"
#include "IXRTrackingSystem.h"
#include "GameFramework/HUD.h"
#include "OpenXRCore.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
#endif

IMPLEMENT_MODULE(FViveOpenXREyeTrackerModule, ViveOpenXREyeTracker);

DEFINE_LOG_CATEGORY(LogViveOpenXREyeTracker);

static TAutoConsoleVariable<int32> CVarEnableOpenXREyetrackingDebug(TEXT("OpenXR.debug.EnableEyetrackingDebug"), 1, TEXT("0 - Eyetracking debug visualizations are disabled. 1 - Eyetracking debug visualizations are enabled."));

FViveOpenXREyeTracker::FViveOpenXREyeTracker()
{
	RegisterOpenXRExtensionModularFeature();
}

FViveOpenXREyeTracker::~FViveOpenXREyeTracker()
{
	Destroy();
}

void FViveOpenXREyeTracker::Destroy()
{
}

bool FViveOpenXREyeTracker::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	TArray<XrApiLayerProperties> Properties;
	uint32_t Count = 0;
	XR_ENSURE(xrEnumerateApiLayerProperties(0, &Count, nullptr));
	Properties.SetNum(Count);
	for (auto& Prop : Properties)
	{
		Prop = XrApiLayerProperties{ XR_TYPE_API_LAYER_PROPERTIES };
	}
	XR_ENSURE(xrEnumerateApiLayerProperties(Count, &Count, Properties.GetData()));

	// Some API layers can crash the loader when enabled, if they're present we shouldn't enable the extension
	for (const XrApiLayerProperties& Layer : Properties)
	{
		if (FCStringAnsi::Strstr(Layer.layerName, "XR_APILAYER_VIVE_eye_tracking") &&
			Layer.layerVersion <= 1)
		{
			return false;
		}
	}

	OutExtensions.Add("XR_EXT_eye_gaze_interaction");
	OutExtensions.Add("XR_HTC_eye_tracker");
	
	return true;
}

void FViveOpenXREyeTracker::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
}

bool FViveOpenXREyeTracker::GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
{
	OutKeyPrefix = "EyeTracker";
	OutHasHaptics = false;
	return xrStringToPath(InInstance, "/interaction_profiles/ext/eye_gaze_interaction", &OutPath) == XR_SUCCESS;
}

void FViveOpenXREyeTracker::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	SystemId = InSystem;
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSystemProperties", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrGetSystemProperties));

	XrSystemEyeTrackingPropertiesHTC eyeTrackingSystemProperties{ XR_TYPE_SYSTEM_EYE_TRACKING_PROPERTIES_HTC };
	eyeTrackingSystemProperties.next = NULL;
	XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES, &eyeTrackingSystemProperties };

	XrResult result = FHTCEyeTracker_ext.xrGetSystemProperties(Instance, SystemId, &systemProperties);
	XR_ENSURE(result);

	if (!eyeTrackingSystemProperties.supportsEyeTracking) {
		// The system does not support eye tracking.
		isHTCEyeTrackerSupported = false;
	}
	else
		isHTCEyeTrackerSupported = true;
}

void FViveOpenXREyeTracker::PostCreateSession(XrSession InSession)
{
	Session = InSession;
}

const void* FViveOpenXREyeTracker::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!isHTCEyeTrackerSupported) return InNext;
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateEyeTrackerHTC", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrCreateEyeTrackerHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyEyeTrackerHTC", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrDestroyEyeTrackerHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetEyeGazeDataHTC", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrGetEyeGazeDataHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetEyePupilDataHTC", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrGetEyePupilDataHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetEyeGeometricDataHTC", (PFN_xrVoidFunction*)&FHTCEyeTracker_ext.xrGetEyeGeometricDataHTC));
	return InNext;
}

void FViveOpenXREyeTracker::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	check(Instance != XR_NULL_HANDLE);

	// This is a bit of a pain right now.  Hopefully future refactors will make it better.
	// We are creating an action set for our eye tracking pose.  An action set can have session create/destroy
	// lifetime.  However currently the OpenXRInput module is loaded well after the OpenXRHMDModule creates 
	// the session, so we can't just setup all this input system right then.  OpenXRInput instead checks if it is live and if 
	// not destroys any existing sets and then creates new ones near xrBeginSession (where the session starts running).
	// It marks them as dead near xrDestroySession so that they will be destroyed on the BeginSession of the next created session, 
	// if any.
	//
	// To mirror that lifetime we are destroying and creating in AttachActionSets and marking as dead in OnDestroySession.
	//
	// Note the ActionSpace is easier because it is never used outside of this ExtensionPlugin.  We are creating it, if necessary, 
	// in OnBeginSession and destroying it in OnDestroySession.  If we had a good CreateSession hook we could create it then, along with
	// the ActionSet and Action.

	// We could have an action set from a previous session.  If so it needs to go away.
	if (EyeTrackerActionSet != XR_NULL_HANDLE)
	{
		xrDestroyActionSet(EyeTrackerActionSet);
		EyeTrackerActionSet = XR_NULL_HANDLE;
		EyeTrackerAction = XR_NULL_HANDLE;
	}

	{
		XrActionSetCreateInfo Info;
		Info.type = XR_TYPE_ACTION_SET_CREATE_INFO;
		Info.next = nullptr;
		FCStringAnsi::Strcpy(Info.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "openxreyetrackeractionset");
		FCStringAnsi::Strcpy(Info.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "OpenXR Eye Tracker Action Set");
		Info.priority = 0;
		XR_ENSURE(xrCreateActionSet(Instance, &Info, &EyeTrackerActionSet));
	}

	{
		check(EyeTrackerAction == XR_NULL_HANDLE);
		XrActionCreateInfo Info;
		Info.type = XR_TYPE_ACTION_CREATE_INFO;
		Info.next = nullptr;
		Info.countSubactionPaths = 0;
		Info.subactionPaths = nullptr;
		FCStringAnsi::Strcpy(Info.actionName, XR_MAX_ACTION_NAME_SIZE, "openxreyetrackeraction");
		FCStringAnsi::Strcpy(Info.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, "OpenXR Eye Tracker Action");
		Info.actionType = XR_ACTION_TYPE_POSE_INPUT;
		XR_ENSURE(xrCreateAction(EyeTrackerActionSet, &Info, &EyeTrackerAction));
	}

	{
		XrPath EyeGazeInteractionProfilePath = XR_NULL_PATH;
		XR_ENSURE(xrStringToPath(Instance, "/interaction_profiles/ext/eye_gaze_interaction", &EyeGazeInteractionProfilePath));

		XrPath GazePosePath = XR_NULL_PATH;
		XR_ENSURE(xrStringToPath(Instance, "/user/eyes_ext/input/gaze_ext/pose", &GazePosePath));

		XrActionSuggestedBinding Bindings;
		Bindings.action = EyeTrackerAction;
		Bindings.binding = GazePosePath;

		XrInteractionProfileSuggestedBinding SuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		SuggestedBindings.interactionProfile = EyeGazeInteractionProfilePath;
		SuggestedBindings.suggestedBindings = &Bindings;
		SuggestedBindings.countSuggestedBindings = 1;
		XR_ENSURE(xrSuggestInteractionProfileBindings(Instance, &SuggestedBindings));
	}

	OutActionSets.Add(EyeTrackerActionSet);
}

const void* FViveOpenXREyeTracker::OnBeginSession(XrSession InSession, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}

	if (GazeActionSpace == XR_NULL_HANDLE)
	{
		GazeActionSpace = XR_NULL_HANDLE;
		XrActionSpaceCreateInfo CreateActionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
		check(EyeTrackerAction != XR_NULL_HANDLE);
		CreateActionSpaceInfo.action = EyeTrackerAction;
		CreateActionSpaceInfo.poseInActionSpace = ToXrPose(FTransform::Identity);
		XR_ENSURE(xrCreateActionSpace(InSession, &CreateActionSpaceInfo, &GazeActionSpace));

		SyncInfo.countActiveActionSets = 0;
		SyncInfo.activeActionSets = XR_NULL_HANDLE;
	}

	if (isHTCEyeTrackerSupported)
	{
		XrResult result = XrResult::XR_ERROR_HANDLE_INVALID;

		XrEyeTrackerCreateInfoHTC createInfo{ XR_TYPE_EYE_TRACKER_CREATE_INFO_HTC };
		result = FHTCEyeTracker_ext.xrCreateEyeTrackerHTC(Session, &createInfo, &EyeTrackerHTC);

		XR_ENSURE(result);
	}

	bSessionStarted = true;

	return InNext;
}

void FViveOpenXREyeTracker::OnDestroySession(XrSession InSession)
{
	if (GazeActionSpace)
	{
		XR_ENSURE(xrDestroySpace(GazeActionSpace));
	}
	GazeActionSpace = XR_NULL_HANDLE;

	XrResult result = XrResult::XR_ERROR_HANDLE_INVALID;
	if (isHTCEyeTrackerSupported)
	{
		if (EyeTrackerHTC == XR_NULL_HANDLE)
		{
			UE_LOG(LogViveOpenXREyeTracker, Error, TEXT("[DestroyEyeTrackerHTC] eyeTracker is XR_NULL_HANDLE."));
		}
		else
		{
			result = FHTCEyeTracker_ext.xrDestroyEyeTrackerHTC(EyeTrackerHTC);
			EyeTrackerHTC = XR_NULL_HANDLE;
		}
		XR_ENSURE(result);
		if (result != XR_SUCCESS)
		{
			UE_LOG(LogViveOpenXREyeTracker, Error, TEXT("[DestroyEyeTrackerHTC] DestroyEyeTrackerHTC failed: %s"), OpenXRResultToString(result));
		}
	}
}

void FViveOpenXREyeTracker::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	check(EyeTrackerActionSet != XR_NULL_HANDLE);

	OutActiveSets.Add(XrActiveActionSet{ EyeTrackerActionSet, XR_NULL_PATH });
}

void FViveOpenXREyeTracker::PostSyncActions(XrSession InSession)
{
	check(EyeTrackerAction != XR_NULL_HANDLE);

	XrActionStateGetInfo GetActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	GetActionStateInfo.action = EyeTrackerAction;
	XR_ENSURE(xrGetActionStatePose(InSession, &GetActionStateInfo, &ActionStatePose));
}

void FViveOpenXREyeTracker::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (ActionStatePose.isActive) 
	{
		check(GazeActionSpace != XR_NULL_HANDLE);
		XR_ENSURE(xrLocateSpace(GazeActionSpace, TrackingSpace, DisplayTime, &EyeTrackerSpaceLocation));
	}
	else
	{
		// Clear the tracked bits if the action is not active
		const XrSpaceLocationFlags TrackedFlags = XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
		EyeTrackerSpaceLocation.locationFlags &= ~TrackedFlags;
	}

	if (isHTCEyeTrackerSupported)
	{
		const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
		XrResult result = XrResult::XR_ERROR_HANDLE_INVALID;

		// Query Gaze information
		XrEyeGazeDataInfoHTC gazesInfo{ XR_TYPE_EYE_GAZE_DATA_INFO_HTC };
		gazesInfo.baseSpace = TrackingSpace;
		gazesInfo.time = DisplayTime;
		result = FHTCEyeTracker_ext.xrGetEyeGazeDataHTC(EyeTrackerHTC, &gazesInfo, &EyeGazesHTC);
		XR_ENSURE(result);
		if (result != XR_SUCCESS)
		{
			UE_LOG(LogViveOpenXREyeTracker, Error, TEXT("[GetEyeGazesHTC] xrGetEyeGazesHTC failed: %s"), OpenXRResultToString(result));
		}

		// Query Pupil information
		XrEyePupilDataInfoHTC pupilDatasInfo{ XR_TYPE_EYE_PUPIL_DATA_INFO_HTC };
		result = FHTCEyeTracker_ext.xrGetEyePupilDataHTC(EyeTrackerHTC, &pupilDatasInfo, &PupilDatasHTC);
		XR_ENSURE(result);
		if (result != XR_SUCCESS)
		{
			UE_LOG(LogViveOpenXREyeTracker, Error, TEXT("[GetPupilDatasHTC] xrGetPupilDatasHTC failed: %s"), OpenXRResultToString(result));
		}

		// Query Geometric information
		XrEyeGeometricDataInfoHTC eyeGeometricDatasInfo{ XR_TYPE_EYE_GEOMETRIC_DATA_INFO_HTC };
		result = FHTCEyeTracker_ext.xrGetEyeGeometricDataHTC(EyeTrackerHTC, &eyeGeometricDatasInfo, &EyeGeometricDatasHTC);
		XR_ENSURE(result);
		if (result != XR_SUCCESS)
		{
			UE_LOG(LogViveOpenXREyeTracker, Error, TEXT("[GetEyeGeometricDatasHTC] xrGetEyeGeometricDatasHTC failed: %s"), OpenXRResultToString(result));
		}
	}
}

bool FViveOpenXREyeTracker::GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const
{
	if (!bSessionStarted)
	{
		OutGazeData = FEyeTrackerGazeData();
		return false;
	}

	const XrSpaceLocationFlags ValidFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;
	const XrSpaceLocationFlags TrackedFlags = XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

	if ((EyeTrackerSpaceLocation.locationFlags & ValidFlags) != ValidFlags)
	{
		// Either Orientation or position are invalid
		OutGazeData = FEyeTrackerGazeData();
		return false;
	}
	else if ((EyeTrackerSpaceLocation.locationFlags & TrackedFlags) != TrackedFlags)
	{
		// Orientation and/or position are old or an estimate of some kind, confidence is low.
		OutGazeData.ConfidenceValue = 0.0f;
	}
	else
	{
		// Both orientation and position are fully tracked now, confidence is high.
		OutGazeData.ConfidenceValue = 1.0f;
	}

	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
	const XrPosef& Pose = EyeTrackerSpaceLocation.pose;
	const FTransform EyeTrackerTransform = ToFTransform(Pose, WorldToMetersScale);

	const FTransform& TrackingToWoldTransform = XRTrackingSystem->GetTrackingToWorldTransform();
	const FTransform EyeTransform = EyeTrackerTransform * TrackingToWoldTransform;

	OutGazeData.GazeDirection = EyeTransform.TransformVector(FVector::ForwardVector);
	OutGazeData.GazeOrigin = EyeTransform.GetLocation();
	OutGazeData.FixationPoint = FVector::ZeroVector; //not supported

	return true;
}

// Integrate with HTC EyeTracker
bool FViveOpenXREyeTracker::GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutStereoGazeData) const
{
	if (!isHTCEyeTrackerSupported) return false;
	FEyeTrackerStereoGazeData OutGazeDataHTC;
	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
	const FTransform& TrackingToWoldTransform = XRTrackingSystem->GetTrackingToWorldTransform();

	if ((bool)EyeGazesHTC.gazes[XR_EYE_POSITION_RIGHT_HTC].isValid && (bool)EyeGazesHTC.gazes[XR_EYE_POSITION_LEFT_HTC].isValid)
		OutGazeDataHTC.ConfidenceValue = 1.0f;
	else if ((bool)EyeGazesHTC.gazes[XR_EYE_POSITION_RIGHT_HTC].isValid || (bool)EyeGazesHTC.gazes[XR_EYE_POSITION_LEFT_HTC].isValid)
		OutGazeDataHTC.ConfidenceValue = 0.5f;
	else
		OutGazeDataHTC.ConfidenceValue = 0.0f;

	XrPosef RightPose = EyeGazesHTC.gazes[XR_EYE_POSITION_RIGHT_HTC].gazePose;
	const FTransform RightEyeTrackerTransform = ToFTransform(RightPose, WorldToMetersScale);
	const FTransform RightEyeTransform = RightEyeTrackerTransform * TrackingToWoldTransform;

	OutGazeDataHTC.RightEyeDirection = RightEyeTransform.TransformVector(FVector::ForwardVector);
	OutGazeDataHTC.RightEyeOrigin = RightEyeTransform.GetLocation();

	XrPosef LeftPose = EyeGazesHTC.gazes[XR_EYE_POSITION_LEFT_HTC].gazePose;
	const FTransform LeftEyeTrackerTransform = ToFTransform(LeftPose, WorldToMetersScale);
	const FTransform LeftEyeTransform = LeftEyeTrackerTransform * TrackingToWoldTransform;

	OutGazeDataHTC.LeftEyeDirection = LeftEyeTransform.TransformVector(FVector::ForwardVector);
	OutGazeDataHTC.LeftEyeOrigin = LeftEyeTransform.GetLocation();

	OutGazeDataHTC.FixationPoint = FVector::ZeroVector; //not supported

	OutStereoGazeData = OutGazeDataHTC;
	return true;
}

EEyeTrackerStatus FViveOpenXREyeTracker::GetEyeTrackerStatus() const
{
	if (!bSessionStarted)
	{
		return EEyeTrackerStatus::NotConnected;
	}

	const XrSpaceLocationFlags ValidFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;
	const XrSpaceLocationFlags TrackedFlags = XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

	if ((EyeTrackerSpaceLocation.locationFlags & ValidFlags) != ValidFlags)
	{
		return EEyeTrackerStatus::NotTracking;
	}

	if ((EyeTrackerSpaceLocation.locationFlags & TrackedFlags) != TrackedFlags)
	{
		return EEyeTrackerStatus::NotTracking;
	}
	else
	{
		return EEyeTrackerStatus::Tracking;
	}
}

bool FViveOpenXREyeTracker::IsStereoGazeDataAvailable() const
{
	return false;
}

void FViveOpenXREyeTracker::DrawDebug(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (!bSessionStarted)
	{
		return;
	}

	const XrSpaceLocationFlags ValidFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;
	const XrSpaceLocationFlags TrackedFlags = XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;

	if ((EyeTrackerSpaceLocation.locationFlags & ValidFlags) != ValidFlags)
	{
		return;
	}

	FColor DrawColor = FColor::Yellow;
	if ((EyeTrackerSpaceLocation.locationFlags & TrackedFlags) == TrackedFlags)
	{
		DrawColor = FColor::Green;
	}

	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
	const XrPosef& Pose = EyeTrackerSpaceLocation.pose;
	FTransform EyeTrackerTransform = ToFTransform(Pose, WorldToMetersScale);

	FVector GazeDirection = EyeTrackerTransform.TransformVector(FVector::ForwardVector);
	FVector GazeOrigin = EyeTrackerTransform.GetLocation();
	FVector DebugPos = GazeOrigin + (GazeDirection * 100.0f);
	DrawDebugSphere(HUD->GetWorld(), DebugPos, 20.0f, 16, DrawColor);
}

/************************************************************************/
/* FViveOpenXREyeTrackerModule                                          */
/************************************************************************/

FViveOpenXREyeTrackerModule::FViveOpenXREyeTrackerModule()
{
}

void FViveOpenXREyeTrackerModule::StartupModule()
{
	IEyeTrackerModule::StartupModule();

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableHTCEyeTracker"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableHTCEyeTracker = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableHTCEyeTracker = true;
		}
	}

	if (m_bEnableHTCEyeTracker)
	{
		UE_LOG(LogViveOpenXREyeTracker, Log, TEXT("Enable HTCEyeTracker."));
		FString ConflictingPluginName = TEXT("OpenXREyeTracker");

		// Check if the plugin is enabled
#if WITH_EDITOR
		if (IPluginManager::Get().FindPlugin(ConflictingPluginName)->IsEnabled())
		{
			UE_LOG(LogViveOpenXREyeTracker, Warning, TEXT("The plugin '%s' is enabled, which conflicts with HTC Eye Tracker feature. Please disable '%s' plugin to ensure proper functionality."), *ConflictingPluginName, *ConflictingPluginName);
		}
#endif
		IEyeTrackerModule::StartupModule();
		UE_LOG(LogViveOpenXREyeTracker, Log, TEXT("EyeTracker %p."), EyeTracker.Get());
		EyeTracker = TSharedPtr<FViveOpenXREyeTracker, ESPMode::ThreadSafe>(new FViveOpenXREyeTracker());
		UE_LOG(LogViveOpenXREyeTracker, Log, TEXT("EyeTracker is valid %d."), EyeTracker.IsValid());
		OnDrawDebugHandle = AHUD::OnShowDebugInfo.AddRaw(this, &FViveOpenXREyeTrackerModule::OnDrawDebug);

	}
	else
	{
		UE_LOG(LogViveOpenXREyeTracker, Log, TEXT("Disable HTCEyeTracker."));
		return;
	}
}

void FViveOpenXREyeTrackerModule::ShutdownModule()
{
	AHUD::OnShowDebugInfo.Remove(OnDrawDebugHandle);
}

TSharedPtr<class IEyeTracker, ESPMode::ThreadSafe> FViveOpenXREyeTrackerModule::CreateEyeTracker()
{
	return EyeTracker;
}

void FViveOpenXREyeTrackerModule::OnDrawDebug(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (CVarEnableOpenXREyetrackingDebug.GetValueOnGameThread())
	{
		if (EyeTracker.IsValid())
		{
			EyeTracker->DrawDebug(HUD, Canvas, DisplayInfo, YL, YPos);
		}
	}
}

bool FViveOpenXREyeTrackerModule::IsEyeTrackerConnected() const
{
	if (EyeTracker.IsValid())
	{
		EEyeTrackerStatus Status = EyeTracker->GetEyeTrackerStatus();
		if ((Status != EEyeTrackerStatus::NotTracking) && (Status != EEyeTrackerStatus::NotConnected))
		{
			return true;
		}
	}

	return false;
}

/************************************************************************/
/* BlueprintFunctionLibray												*/
/************************************************************************/

FXrGazeDataHTC FViveOpenXREyeTracker::GetEyeGazeValidDatas()
{
	FXrGazeDataHTC OutEyeGazeValidDataHTC;
	if (!isHTCEyeTrackerSupported) return OutEyeGazeValidDataHTC;

	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

	OutEyeGazeValidDataHTC.rightEyeGazeValid = (bool)EyeGazesHTC.gazes[XR_EYE_POSITION_RIGHT_HTC].isValid;
	OutEyeGazeValidDataHTC.leftEyeGazeValid = (bool)EyeGazesHTC.gazes[XR_EYE_POSITION_LEFT_HTC].isValid;

	return OutEyeGazeValidDataHTC;
}

FXrPupilDataHTC FViveOpenXREyeTracker::GetPupilDatas()
{
	FXrPupilDataHTC OutPupilDataHTC;
	if (!isHTCEyeTrackerSupported) return OutPupilDataHTC;

	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

	OutPupilDataHTC.rightEyeDiameterValid = (bool)PupilDatasHTC.pupils[XR_EYE_POSITION_RIGHT_HTC].isDiameterValid;
	OutPupilDataHTC.leftEyeDiameterValid = (bool)PupilDatasHTC.pupils[XR_EYE_POSITION_LEFT_HTC].isDiameterValid;

	OutPupilDataHTC.rightEyePositionValid = (bool)PupilDatasHTC.pupils[XR_EYE_POSITION_RIGHT_HTC].isPositionValid;
	OutPupilDataHTC.leftEyePositionValid = (bool)PupilDatasHTC.pupils[XR_EYE_POSITION_LEFT_HTC].isPositionValid;

	OutPupilDataHTC.rightEyePupilDiameter = PupilDatasHTC.pupils[XR_EYE_POSITION_RIGHT_HTC].pupilDiameter;
	OutPupilDataHTC.leftEyePupilDiameter = PupilDatasHTC.pupils[XR_EYE_POSITION_LEFT_HTC].pupilDiameter;

	OutPupilDataHTC.rightEyePupilPosition.X = PupilDatasHTC.pupils[XR_EYE_POSITION_RIGHT_HTC].pupilPosition.x * WorldToMetersScale;
	OutPupilDataHTC.rightEyePupilPosition.Y = PupilDatasHTC.pupils[XR_EYE_POSITION_RIGHT_HTC].pupilPosition.y * WorldToMetersScale;

	OutPupilDataHTC.leftEyePupilPosition.X = PupilDatasHTC.pupils[XR_EYE_POSITION_LEFT_HTC].pupilPosition.x * WorldToMetersScale;
	OutPupilDataHTC.leftEyePupilPosition.Y = PupilDatasHTC.pupils[XR_EYE_POSITION_LEFT_HTC].pupilPosition.y * WorldToMetersScale;
	
	return OutPupilDataHTC;
}

FXrEyeGeometricDataHTC FViveOpenXREyeTracker::GetEyeGeometricDatas()
{
	FXrEyeGeometricDataHTC OutEyeGeometricDataHTC;
	
	if (!isHTCEyeTrackerSupported) return OutEyeGeometricDataHTC;

	OutEyeGeometricDataHTC.rightEyeIsValid = (bool)EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_RIGHT_HTC].isValid;
	OutEyeGeometricDataHTC.leftEyeIsValid = (bool)EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_LEFT_HTC].isValid;

	OutEyeGeometricDataHTC.rightEyeOpenness = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_RIGHT_HTC].eyeOpenness;
	OutEyeGeometricDataHTC.leftEyeOpenness = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_LEFT_HTC].eyeOpenness;

	OutEyeGeometricDataHTC.rightEyeSqueeze = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_RIGHT_HTC].eyeSqueeze;
	OutEyeGeometricDataHTC.leftEyeSqueeze = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_LEFT_HTC].eyeSqueeze;

	OutEyeGeometricDataHTC.rightEyeWide = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_RIGHT_HTC].eyeWide;
	OutEyeGeometricDataHTC.leftEyeWide = EyeGeometricDatasHTC.geometrics[XR_EYE_POSITION_LEFT_HTC].eyeWide;
	
	return OutEyeGeometricDataHTC;
}