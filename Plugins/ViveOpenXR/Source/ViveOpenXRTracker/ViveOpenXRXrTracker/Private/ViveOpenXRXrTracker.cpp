// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRXrTracker.h"
#include "OpenXRCore.h"
#include "Engine/World.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor.h"
#endif

DEFINE_LOG_CATEGORY(LogViveOpenXRXrTracker);

FViveOpenXRXrTracker* FViveOpenXRXrTracker::m_Instance = nullptr;

FViveOpenXRXrTracker::FViveXrTracker::FViveXrTracker()
	: XrTrackerUserPath(XR_NULL_PATH)
	, xrTrackerPosePath(XR_NULL_PATH)
	, xrTrackerPoseAction(XR_NULL_HANDLE)
	, XrTrackerSpace(XR_NULL_HANDLE)
	, XrTrackerSpaceLocation({ XR_TYPE_ACTION_STATE_BOOLEAN })
	, XrTrackerStatePose({ XR_TYPE_ACTION_STATE_POSE })
	, DeviceId(0)
{
	XrTrackerPaths.RemoveAll([](const int& num) {
		return true;
		});
}

void FViveOpenXRXrTracker::FViveXrTracker::CreateSpace(XrSession InSession)
{
	XrActionSpaceCreateInfo actionSpaceCreateInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
	XrSpace	tempSpace = XR_NULL_HANDLE;

	actionSpaceCreateInfo.action = xrTrackerPoseAction;
	actionSpaceCreateInfo.subactionPath = XrTrackerUserPath;
	actionSpaceCreateInfo.poseInActionSpace = ToXrPose(FTransform::Identity);
	XR_ENSURE(xrCreateActionSpace(InSession, &actionSpaceCreateInfo, &tempSpace));
	XrTrackerSpace = MoveTemp(tempSpace);
	if (XrTrackerSpace == XR_NULL_HANDLE)
	{
		UE_LOG(LogViveOpenXRXrTracker, Warning, TEXT("GetControllerTrackingStatus Spcae is Null."));
	}
}

void FViveOpenXRXrTracker::FViveXrTracker::AddAction(XrActionSet& InActionSet, const FName& InName, const TArray<XrPath>& InSubactionPaths)
{
	check(InActionSet != XR_NULL_HANDLE);
	if (xrTrackerPoseAction != XR_NULL_HANDLE) {
		xrDestroyAction(xrTrackerPoseAction);
		xrTrackerPoseAction = XR_NULL_HANDLE;
	}
	char ActionName[NAME_SIZE];
	InName.GetPlainANSIString(ActionName);
	XrActionCreateInfo Info;
	Info.type = XR_TYPE_ACTION_CREATE_INFO;
	Info.next = nullptr;
	Info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	Info.countSubactionPaths = InSubactionPaths.Num();
	Info.subactionPaths = InSubactionPaths.GetData();
	FCStringAnsi::Strcpy(Info.actionName, XR_MAX_ACTION_NAME_SIZE, ActionName);
	FCStringAnsi::Strcpy(Info.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, ActionName);
	XR_ENSURE(xrCreateAction(InActionSet, &Info, &xrTrackerPoseAction));
}

void FViveOpenXRXrTracker::FViveXrTracker::DestroySpace()
{
	if (XrTrackerSpace)
	{
		XR_ENSURE(xrDestroySpace(XrTrackerSpace));
	}
	XrTrackerSpace = XR_NULL_HANDLE;
}

FViveOpenXRXrTracker::FViveOpenXRXrTracker(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
	// Register modular feature manually
	IModularFeatures::Get().RegisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	IModularFeatures::Get().RegisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));

	m_Instance = this;
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("FViveOpenXRXrTracker() register extension feature Xr Tracker %p."), m_Instance);
}

FViveOpenXRXrTracker::~FViveOpenXRXrTracker()
{
	// Unregister modular feature manually
	IModularFeatures::Get().UnregisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	IModularFeatures::Get().UnregisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
}

bool FViveOpenXRXrTracker::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_EnableXrTracker)
	{
#if PLATFORM_WINDOWS
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetRequiredExtensions() XR_HTC_path_enumeration."));
		OutExtensions.Add("XR_HTC_path_enumeration");
#endif
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetRequiredExtensions() XR_HTC_vive_xr_tracker_interaction."));
		OutExtensions.Add("XR_HTC_vive_xr_tracker_interaction");
	}
	return true;
}

bool FViveOpenXRXrTracker::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_EnableXrTracker)
	{
#if PLATFORM_ANDROID
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetOptionalExtensions() XR_HTC_path_enumeration."));
		OutExtensions.Add("XR_HTC_path_enumeration");
#endif
	}
	return true;
}

void FViveOpenXRXrTracker::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
}

void FViveOpenXRXrTracker::PostCreateSession(XrSession InSession)
{
	m_Session = InSession;
}

void FViveOpenXRXrTracker::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("AttachActionSets() Entry."));
	if (!m_EnableXrTracker) return;

	for (auto& viveXrTracker : viveXrTrackerMap)
	{
		//viveXrTracker.Value.AddTrackedDevices(OpenXRHMD); //Wait for subacitonPath
	}
	OutActionSets.Add(XrTrackerActionSet);

	bActionsAttached = true;
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("AttachActionSets() finished."));
}

void FViveOpenXRXrTracker::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	if (!m_EnableXrTracker) return;

	check(XrTrackerActionSet != XR_NULL_HANDLE);
	OutActiveSets.Add(XrActiveActionSet{ XrTrackerActionSet, XR_NULL_PATH });
}

const void* FViveOpenXRXrTracker::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));

	Instance = InInstance;

	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		OpenXRHMD = (FOpenXRHMD*)GEngine->XRSystem.Get();
	}

	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("OnCreateSession() entry."));
	if (!m_EnableXrTracker) return InNext;

	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("Entry XrTracker OnCreateSession."));
#if PLATFORM_WINDOWS
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumeratePathsForInteractionProfileHTC", (PFN_xrVoidFunction*)&xrEnumeratePathsForInteractionProfileHTC));
#endif

	if (!bPathEnumerated)
	{
		XrActionSetCreateInfo xrTrackerActionInfo;
		xrTrackerActionInfo.type = XR_TYPE_ACTION_SET_CREATE_INFO;
		xrTrackerActionInfo.next = nullptr;
		FCStringAnsi::Strcpy(xrTrackerActionInfo.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "xrtracker");
		FCStringAnsi::Strcpy(xrTrackerActionInfo.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "VIVE OpenXR Xr Tracker Action Set");
		xrTrackerActionInfo.priority = 0;
		XR_ENSURE(xrCreateActionSet(Instance, &xrTrackerActionInfo, &XrTrackerActionSet));

		// Enumerate tracker user paths
		uint32_t userCount = 0;
		uint32_t userCountInput = 0;

		// initialization arrays
		TArray<XrPath> XrTrackerUserPaths;
		XrTrackerUserPaths.Empty();
		TArray<XrPath> XrTrackerPaths;
		XrTrackerPaths.Empty();
		XrTrackerBindings.Empty();

		// Create enumerateInfo
		XrPathsForInteractionProfileEnumerateInfoHTC enumerateInfo;
		enumerateInfo.type = XR_TYPE_PATHS_FOR_INTERACTION_PROFILE_ENUMERATE_INFO_HTC;
		// Get the user paths with the input interaction profile.
		xrTrackerInteractionProfile = FOpenXRPath("/interaction_profiles/htc/vive_xr_tracker");
		enumerateInfo.interactionProfile = xrTrackerInteractionProfile;

		// Use XR_NULL_PATH pathenumerate to get supported userspath under interactionprofile
		enumerateInfo.userPath = XR_NULL_PATH;

		TArray<XrPath> XrTrackerEnumerateUserPaths;
		XrTrackerEnumerateUserPaths.Empty();
#if PLATFORM_WINDOWS
		// Enumerate user paths
		XR_ENSURE(xrEnumeratePathsForInteractionProfileHTC(Instance, &enumerateInfo, 0, &userCount, XrTrackerEnumerateUserPaths.GetData()));
		userCountInput = userCount;

		bool enumerateUltimatePath = false;

		if (userCountInput > 0)
		{
			// Get how meny user paths
			for (uint32_t i = 0; i < userCount; i++)
			{
				XrTrackerEnumerateUserPaths.Emplace(XR_NULL_PATH);
			}
			// Get user paths names
			XR_ENSURE(xrEnumeratePathsForInteractionProfileHTC(Instance, &enumerateInfo, userCountInput, &userCount, XrTrackerEnumerateUserPaths.GetData()));

			for (uint32_t i = 0; i < userCountInput; i++)
			{
				FOpenXRPath path = XrTrackerEnumerateUserPaths[i];
				if (path.ToString().Contains("ultimate"))
					XrTrackerUserPaths.Emplace(XrTrackerEnumerateUserPaths[i]);
			}
			if (XrTrackerUserPaths.Num() >= 1)
				enumerateUltimatePath = true;
			else
				UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("xrEnumeratePathsForInteractionProfileHTC no any path."));
		}
		if (!enumerateUltimatePath)
		{
			XrTrackerUserPaths.Empty();
			for (int i = 0; i < 5; i++)
			{
				XrTrackerUserPaths.Emplace(FOpenXRPath(xrTrackerPaths[i]));
			}
		}
		TArray<XrPath> XrTrackerSupportPaths;
		// Get inputs for each user
		for (int i = 0; i < XrTrackerUserPaths.Num(); i++)
		{
			FName XrTrackerName = (FName)("UltimateTracker" + FString::FormatAsNumber(i + 1));
			FViveXrTracker viveXrTracker = FViveXrTracker();

			viveXrTrackerMap.Add(XrTrackerName, viveXrTracker);

			FOpenXRPath XrTrackerUser = XrTrackerUserPaths[i];

			viveXrTrackerMap[XrTrackerName].XrTrackerUserPath = XrTrackerUser;
			uint32_t pathCount = 0;
			uint32_t pathCountInput = 0;
			// Get the input and output source paths with user path.
			enumerateInfo.userPath = viveXrTrackerMap[XrTrackerName].XrTrackerUserPath;

			// Enumerate user input paths
			XR_ENSURE(xrEnumeratePathsForInteractionProfileHTC(Instance, &enumerateInfo, 0, &pathCount, XrTrackerSupportPaths.GetData()));
			// Get how meny input paths
			pathCountInput = pathCount;
			XrPath XrTrackerSupportPath;
			if (pathCountInput >= 0)
			{
				for (uint32_t j = 0; j < pathCountInput; j++)
				{
					XrTrackerSupportPaths.Emplace(XR_NULL_PATH);

					XR_ENSURE(xrEnumeratePathsForInteractionProfileHTC(Instance, &enumerateInfo, pathCountInput, &pathCount, XrTrackerSupportPaths.GetData()));
					XrTrackerSupportPath = XrTrackerSupportPaths[j];

					uint32 PathCount = 0;
					char PathChars[XR_MAX_PATH_LENGTH];
					XrResult Result = xrPathToString(Instance, XrTrackerSupportPath, XR_MAX_PATH_LENGTH, &PathCount, PathChars);
					if (!FString(PathCount - 1, PathChars).Contains("pose"))
						continue;
#if PLATFORM_ANDROID
					FString poseStr = FString(PathCount - 1, PathChars);
#endif
#if PLATFORM_WINDOWS
					FString poseStr = XrTrackerUser.ToString().Append(*FString(PathCount - 1, PathChars));
#endif
					XrPath posePath = static_cast<FOpenXRPath>(poseStr);
					XrTrackerPaths.Emplace(posePath);
					viveXrTrackerMap[XrTrackerName].xrTrackerPosePath = posePath;
					break;
				}
			}
			else
			{
				UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("No Pose path."));
				XrTrackerSupportPath = FOpenXRPath(xrTrackerPosePaths[i]);
				XrTrackerPaths.Emplace(XrTrackerSupportPath);
				viveXrTrackerMap[XrTrackerName].xrTrackerPosePath = XrTrackerSupportPath;
			}
#elif PLATFORM_ANDROID
		XrTrackerUserPaths.Empty();
		for (int i = 0; i < 5; i++)
		{
			XrTrackerUserPaths.Emplace(FOpenXRPath(xrTrackerPaths[i]));
		}
		// Get inputs for each user
		for (int i = 0; i < XrTrackerUserPaths.Num(); i++)
		{
			FName XrTrackerName = (FName)("UltimateTracker" + FString::FormatAsNumber(i + 1));
			FViveXrTracker viveXrTracker = FViveXrTracker();

			viveXrTrackerMap.Add(XrTrackerName, viveXrTracker);

			FOpenXRPath XrTrackerUser = XrTrackerUserPaths[i];

			viveXrTrackerMap[XrTrackerName].XrTrackerUserPath = XrTrackerUser;
			uint32_t pathCount = 0;
			uint32_t pathCountInput = 0;
			// Get the input and output source paths with user path.
			XrPath XrTrackerSupportPath;
			XrTrackerSupportPath = FOpenXRPath(xrTrackerPosePaths[i]);
			XrTrackerPaths.Emplace(XrTrackerSupportPath);
			viveXrTrackerMap[XrTrackerName].xrTrackerPosePath = XrTrackerSupportPath;
#endif

			// CreateAction
			FString actionNameStr = "xrtracker" + FString::FormatAsNumber(i) + "pose";
			FName actionName(actionNameStr);

			viveXrTrackerMap[XrTrackerName].AddAction(XrTrackerActionSet, actionName, XrTrackerUserPaths);
			viveXrTrackerMap[XrTrackerName].GetSuggestedBindings(XrTrackerBindings);
			viveXrTrackerMap[XrTrackerName].XrTrackerPaths = XrTrackerPaths;
		}
	}

	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("AttachActionSets() Create XrInteractionProfileSuggestedBinding."));
	XrInteractionProfileSuggestedBinding SuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	SuggestedBindings.interactionProfile = xrTrackerInteractionProfile;
	SuggestedBindings.suggestedBindings = XrTrackerBindings.GetData();
	SuggestedBindings.countSuggestedBindings = (uint32)XrTrackerBindings.Num();
	XR_ENSURE(xrSuggestInteractionProfileBindings(Instance, &SuggestedBindings));
	bPathEnumerated = true;
	return InNext;
}

const void* FViveOpenXRXrTracker::OnBeginSession(XrSession InSession, const void* InNext)
{
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("OnBeginSession() entry."));
	if (!m_EnableXrTracker)
	{
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("OnBeginSession() m_EnableXrTracker is false."));
		return InNext;
	}

	for (auto& viveXrTracker : viveXrTrackerMap)
	{
		viveXrTracker.Value.CreateSpace(InSession);
	}

	bSessionStarted = true;
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("OnBeginSession() finished."));

	return InNext;
}

void FViveOpenXRXrTracker::OnDestroySession(XrSession InSession)
{
	if (bActionsAttached)
		bActionsAttached = false;
}

void FViveOpenXRXrTracker::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (!m_EnableXrTracker) return;
	const float WorldToMetersScale = OpenXRHMD->GetWorldToMetersScale();

	// Get XrTracker pose
	//TArray<FViveXrTracker> xrTrackers;
	//viveXrTrackerMap.GenerateValueArray(xrTrackers);
	//if (xrTrackers.Num() >= 0)
	//{
	//	for (int i = 0; i < xrTrackers.Num(); i++)
	//	{
	//		XrSpaceLocation XrTrackerSpaceLocation{ XR_TYPE_SPACE_LOCATION };
	//		XR_ENSURE(xrLocateSpace(xrTrackers[i].xrTrackerSpace, TrackingSpace, DisplayTime, &XrTrackerSpaceLocation));
	//		xrTrackers[i].XrTrackerSpaceLocation = XrTrackerSpaceLocation;
	//		const FTransform XrTrackerTransform = ToFTransform(xrTrackers[i].XrTrackerSpaceLocation.pose, WorldToMetersScale);
	//		FName XrTrackerName = (FName)("XrTracker " + FString::FormatAsNumber(i + 1));
	//		viveXrTrackerMap[XrTrackerName] = xrTrackers[i];
	//	}
	//}

	for (auto& viveXrTracker : viveXrTrackerMap)
	{
		XrSpaceLocation tempSpaceLocation{ XR_TYPE_SPACE_LOCATION };
		XR_ENSURE(xrLocateSpace(viveXrTracker.Value.XrTrackerSpace, TrackingSpace, DisplayTime, &tempSpaceLocation));
		viveXrTracker.Value.XrTrackerSpaceLocation = MoveTemp(tempSpaceLocation);
		//viveXrTracker.Value.XrTrackerSpaceLocation = XrTrackerSpaceLocation;
		//const FTransform XrTrackerTransform = ToFTransform(viveXrTracker.Value.XrTrackerSpaceLocation.pose, WorldToMetersScale);
	}
}

/************************************************************************/
/* IMotionController                                                    */
/************************************************************************/
bool FViveOpenXRXrTracker::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (!MotionSource.ToString().Contains("UltimateTracker"))
		return false;

	if (viveXrTrackerMap.Num() == 0)
		return false;

	if (!viveXrTrackerMap.Contains(MotionSource)) {
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{
		XrPosef pose = viveXrTrackerMap[MotionSource].XrTrackerSpaceLocation.pose;
		const FTransform XrTrackerTransform = ToFTransform(pose, WorldToMetersScale);

		FQuat Orientation = ToFQuat(pose.orientation);
		OutPosition = XrTrackerTransform.GetLocation();
		OutOrientation = XrTrackerTransform.Rotator();

		return true;
	}
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetControllerOrientationAndPosition return false."));

	//FString motionSource = MotionSource.ToString();
	//if (ControllerIndex == DeviceIndex && motionSource.Contains("XrTracker"))
	//{
	//	int32 DeviceId = viveXrTrackerMap[MotionSource].DeviceId;
	//	FQuat Orientation;
	//	bool Success = OpenXRHMD->GetCurrentPose(DeviceId, Orientation, OutPosition);
	//	OutOrientation = FRotator(Orientation);
	//	return Success;
	//}
	//return false;

	return false;
}

bool FViveOpenXRXrTracker::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (!MotionSource.ToString().Contains("UltimateTracker"))
		return false;

	if (viveXrTrackerMap.Num() == 0)
		return false;

	if (!viveXrTrackerMap.Contains(MotionSource)) {
		//UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetControllerOrientationAndPosition MotionSource %s."), *MotionSource.ToString());
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{
		XrPosef pose = viveXrTrackerMap[MotionSource].XrTrackerSpaceLocation.pose;
		const FTransform XrTrackerTransform = ToFTransform(pose, WorldToMetersScale);

		FQuat Orientation = ToFQuat(pose.orientation);
		OutPosition = XrTrackerTransform.GetLocation();
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetControllerOrientationAndPosition OutPosition %s."), *OutPosition.ToString());
		OutOrientation = XrTrackerTransform.Rotator();

		OutbProvidedLinearVelocity = false;
		OutbProvidedAngularVelocity = false;
		OutbProvidedLinearAcceleration = false;
		OutLinearVelocity = FVector();
		OutAngularVelocityAsAxisAndLength = FVector();
		OutLinearAcceleration = FVector();
		return true;
	}
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetControllerOrientationAndPosition return false."));
	return false;
	//FTimespan Time;
	//bool OutTimeWasUsed = false;
	//return GetControllerOrientationAndPositionForTime(ControllerIndex, MotionSource, Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
}


bool FViveOpenXRXrTracker::GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	FString motionSource = MotionSource.ToString();
	if (ControllerIndex == DeviceIndex && motionSource.Contains("UltimateTracker"))
	{
		int32 DeviceId = viveXrTrackerMap[MotionSource].DeviceId;
		FQuat Orientation;
		bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
		OutOrientation = FRotator(Orientation);
		return Success;
	}
	return false;
}

ETrackingStatus FViveOpenXRXrTracker::GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return ETrackingStatus::NotTracked;
	}

	XrSession Session = OpenXRHMD->GetSession();
	if (Session == XR_NULL_HANDLE)
	{
		return ETrackingStatus::NotTracked;
	}

	XrActionStateGetInfo PoseActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };

	if (!MotionSource.ToString().Contains("UltimateTracker"))
		return ETrackingStatus::NotTracked;

	if (viveXrTrackerMap.Num() == 0)
		return ETrackingStatus::NotTracked;

	if (!viveXrTrackerMap.Contains(MotionSource)) {
		return ETrackingStatus::NotTracked;
	}

	FViveXrTracker xrTracker = viveXrTrackerMap[MotionSource];
	if (xrTracker.xrTrackerPoseAction == nullptr)
		return ETrackingStatus::NotTracked;

	PoseActionStateInfo.action = xrTracker.xrTrackerPoseAction;
	PoseActionStateInfo.subactionPath = xrTracker.xrTrackerPosePath;

	//XrActionStatePose State = { XR_TYPE_ACTION_STATE_POSE };
	//if (!XR_ENSURE(xrGetActionStatePose(Session, &PoseActionStateInfo, &State)))
	//{
	//	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("GetControllerTrackingStatus ETrackingStatus::NotTracked."));
	//	return ETrackingStatus::NotTracked;
	//}

	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("State.isActive"));

	return ETrackingStatus::Tracked;
}

FName FViveOpenXRXrTracker::GetMotionControllerDeviceTypeName() const
{
	const static FName DefaultName(TEXT("OpenXRXrTracker"));
	return DefaultName;
}

void FViveOpenXRXrTracker::EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const
{
	check(IsInGameThread());

	for (int i = 1; i <= 5; i++)
	{
		FString name = "UltimateTracker" + FString::FormatAsNumber(i);
		SourcesOut.Add(FMotionControllerSource(FName(*name)));
	}
}

int32 FViveOpenXRXrTracker::FViveXrTracker::AddTrackedDevices(FOpenXRHMD* HMD)
{
	if (HMD)
	{
		DeviceId = HMD->AddTrackedDevice(xrTrackerPoseAction, xrTrackerPosePath);
	}
	return DeviceId;
}

void FViveOpenXRXrTracker::FViveXrTracker::GetSuggestedBindings(TArray<XrActionSuggestedBinding>& OutSuggestedBindings)
{
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ xrTrackerPoseAction, xrTrackerPosePath });
}

void FViveOpenXRXrTracker::FViveXrTracker::SyncActionStates(XrSession InSession)
{
	XrActionStateGetInfo GetXrTrackerStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	GetXrTrackerStateInfo.action = xrTrackerPoseAction;
	GetXrTrackerStateInfo.subactionPath = XrTrackerUserPath;
	XR_ENSURE(xrGetActionStatePose(InSession, &GetXrTrackerStateInfo, &XrTrackerStatePose));
}

#undef LOCTEXT_NAMESPACE

#define LOCTEXT_NAMESPACE "FViveOpenXRXrTrackerModule"

FName IViveOpenXRXrTrackerModule::ViveOpenXRXrTrackerModularKeyName = FName(TEXT("ViveOpenXRXrTracker"));

FViveOpenXRXrTrackerModule::FViveOpenXRXrTrackerModule()
{

}

void FViveOpenXRXrTrackerModule::StartupModule()
{
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("StartupModule() Entry."));

	IViveOpenXRXrTrackerModule::StartupModule();

	TSharedPtr<FGenericApplicationMessageHandler> DummyMessageHandler(new FGenericApplicationMessageHandler());
	CreateInputDevice(DummyMessageHandler.ToSharedRef());

	// Check if the modeule is enabled
	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableXrTracker"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{

			UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("StartupModule() Entry."));
			GetXrTracker()->m_EnableXrTracker = false;
		}
		else if (modeName.Equals("True"))
		{
			GetXrTracker()->m_EnableXrTracker = true;
		}
	}

	if (GetXrTracker()->m_EnableXrTracker)
	{
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("Enable Xr Tracker."));
	}
	else
	{
		UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("Disable Xr Tracker."));
	}
	UE_LOG(LogViveOpenXRXrTracker, Log, TEXT("StartupModule() Finished."));

}

void FViveOpenXRXrTrackerModule::ShutdownModule()
{
}

TSharedPtr<class IInputDevice> FViveOpenXRXrTrackerModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!XrTracker.IsValid())
	{
		auto InputDevice = new FViveOpenXRXrTracker(InMessageHandler);
		XrTracker = TSharedPtr<FViveOpenXRXrTracker>(InputDevice);

		return XrTracker;
	}
	else
	{
		XrTracker.Get()->SetMessageHandler(InMessageHandler);
		return XrTracker;
	}
	return nullptr;
}

FViveOpenXRXrTracker* FViveOpenXRXrTrackerModule::GetXrTracker()
{
	return FViveOpenXRXrTracker::GetInstance();
}

IMPLEMENT_MODULE(FViveOpenXRXrTrackerModule, ViveOpenXRXrTracker)

