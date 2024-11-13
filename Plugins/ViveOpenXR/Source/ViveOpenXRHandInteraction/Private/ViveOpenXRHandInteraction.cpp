// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRHandInteraction.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "UObject/UObjectIterator.h"
#include "IOpenXRExtensionPlugin.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"

#include "EnhancedInputLibrary.h"
#include "EnhancedInputSubsystemInterface.h"
#include "EnhancedInputModule.h"
#include "PlayerMappableInputConfig.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "OpenXRInputSettings.h"

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "EnhancedInputEditorSubsystem.h"
#endif

#define LOCTEXT_NAMESPACE "FViveOpenXRHandInteraction"

DEFINE_LOG_CATEGORY(LogViveOpenXRHandInteraction);

FViveOpenXRHandInteraction* FViveOpenXRHandInteraction::m_Instance = nullptr;

FViveOpenXRHandInteraction::FViveInteractionController::FViveInteractionController()
	: RolePath(XR_NULL_PATH)
	//, PinchPoseAction(XR_NULL_HANDLE)
	//, PinchPoseActionPath(XR_NULL_PATH)
	, AimPoseAction(XR_NULL_HANDLE)
    , AimPoseActionPath(XR_NULL_PATH)
    , GripPoseAction(XR_NULL_HANDLE)
    , GripPoseActionPath(XR_NULL_PATH)
    //, PokePoseAction(XR_NULL_HANDLE)
    //, PokePoseActionPath(XR_NULL_PATH)
	//, PinchAction(XR_NULL_HANDLE)
	//, PinchActionPath(XR_NULL_PATH)
	, AimAction(XR_NULL_HANDLE)
	, AimActionPath(XR_NULL_PATH)
	, GraspAction(XR_NULL_HANDLE)
	, GraspActionPath(XR_NULL_PATH)
	, AimDeviceId(-1)
	, GripDeviceId(-1)
	//, PinchActionState({ XR_TYPE_ACTION_STATE_BOOLEAN })
	//, AimActionState({ XR_TYPE_ACTION_STATE_BOOLEAN })
	//, GraspActionState({ XR_TYPE_ACTION_STATE_BOOLEAN })
	//, PinchValueActionState({ XR_TYPE_ACTION_STATE_FLOAT })
	, AimValueActionState({ XR_TYPE_ACTION_STATE_FLOAT })
	, GraspValueActionState({ XR_TYPE_ACTION_STATE_FLOAT })
	//, PinchKey(TEXT(""))
	, AimKey(TEXT(""))
	, GraspKey(TEXT(""))
{
	SubactionPaths.RemoveAll([](const int& num) {
		return true;
		});
	//PinchInputActions.Empty();
	AimInputActions.Empty();
	GraspInputActions.Empty();
}

void FViveOpenXRHandInteraction::FViveInteractionController::SetupPath(FString InRolePath, FString InPinchPoseActionPath, FString InAimPoseActionPath, FString InGripPoseActionPath, FString InPokePoseActionPath, FString InPinchActionPath, FString InAimActionPath, FString InGraspActionPath)
{
	RolePath = FOpenXRPath(InRolePath);
	//PinchPoseActionPath = FOpenXRPath(InPinchPoseActionPath);
	AimPoseActionPath = FOpenXRPath(InAimPoseActionPath);
	GripPoseActionPath = FOpenXRPath(InGripPoseActionPath);
	//PokePoseActionPath = FOpenXRPath(InPokePoseActionPath);
	//PinchActionPath = FOpenXRPath(InPinchActionPath);
	AimActionPath = FOpenXRPath(InAimActionPath);
	GraspActionPath = FOpenXRPath(InGraspActionPath);
	SubactionPaths.Reset();
	SubactionPaths.Emplace(RolePath);
}

void FViveOpenXRHandInteraction::FViveInteractionController::HTCSetupPath(FString InRolePath, FString InAimPoseActionPath, FString InGripPoseActionPath, FString InAimActionPath, FString InGraspActionPath)
{
	RolePath = FOpenXRPath(InRolePath);
	AimPoseActionPath = FOpenXRPath(InAimPoseActionPath);
	GripPoseActionPath = FOpenXRPath(InGripPoseActionPath);
	AimActionPath = FOpenXRPath(InAimActionPath);
	GraspActionPath = FOpenXRPath(InGraspActionPath);
	SubactionPaths.Reset();
	SubactionPaths.Emplace(RolePath);
}

int32 FViveOpenXRHandInteraction::FViveInteractionController::AddTrackedDevices(FOpenXRHMD* HMD)
{
	if (HMD)
	{
		//DeviceId = HMD->AddTrackedDevice(PinchPoseAction, PinchPoseActionPath);
		//UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("AddTrackedDevices ID: %d"), DeviceId);
		AimDeviceId = HMD->AddTrackedDevice(AimPoseAction, AimPoseActionPath);
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("AddTrackedDevices ID: %d"), AimDeviceId);
		GripDeviceId = HMD->AddTrackedDevice(GripPoseAction, GripPoseActionPath);
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("AddTrackedDevices ID: %d"), GripDeviceId);
		//DeviceId = HMD->AddTrackedDevice(PokePoseAction, PokePoseActionPath);
		//UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("AddTrackedDevices ID: %d"), DeviceId);
	}
	return AimDeviceId;
}

void FViveOpenXRHandInteraction::FViveInteractionController::GetSuggestedBindings(TArray<XrActionSuggestedBinding>& OutSuggestedBindings)
{
	//OutSuggestedBindings.Add(XrActionSuggestedBinding{ PinchPoseAction, PinchPoseActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ AimPoseAction, AimPoseActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ GripPoseAction, GripPoseActionPath });
	//OutSuggestedBindings.Add(XrActionSuggestedBinding{ PokePoseAction, PokePoseActionPath });
	//OutSuggestedBindings.Add(XrActionSuggestedBinding{ PinchAction, PinchActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ AimAction, AimActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ GraspAction, GraspActionPath });
}

void FViveOpenXRHandInteraction::FViveInteractionController::AddAction(XrActionSet& InActionSet, XrAction& OutAction, FOpenXRPath InBindingPath, XrActionType InActionType)//, const TArray<XrPath>& InSubactionPaths)
{
	check(InActionSet != XR_NULL_HANDLE);
	if (OutAction != XR_NULL_HANDLE) {
		xrDestroyAction(OutAction);
		OutAction = XR_NULL_HANDLE;
	}

	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("xrCreateAction %s"), *InBindingPath.ToString());

	char ActionName[NAME_SIZE];
	GetActionName(InBindingPath).GetPlainANSIString(ActionName);

	XrActionCreateInfo Info;
	Info.type = XR_TYPE_ACTION_CREATE_INFO;
	Info.next = nullptr;
	Info.actionType = InActionType;
	Info.countSubactionPaths = SubactionPaths.Num();
	Info.subactionPaths = SubactionPaths.GetData();
	FCStringAnsi::Strcpy(Info.actionName, XR_MAX_ACTION_NAME_SIZE, ActionName);
	FCStringAnsi::Strcpy(Info.localizedActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE, ActionName);
	XR_ENSURE(xrCreateAction(InActionSet, &Info, &OutAction));

}

void  FViveOpenXRHandInteraction::FViveInteractionController::AddActions(XrActionSet& InActionSet)
{
	//AddAction(InActionSet, PinchPoseAction, PinchPoseActionPath, XR_ACTION_TYPE_POSE_INPUT);
	AddAction(InActionSet, AimPoseAction, AimPoseActionPath, XR_ACTION_TYPE_POSE_INPUT);
	AddAction(InActionSet, GripPoseAction, GripPoseActionPath, XR_ACTION_TYPE_POSE_INPUT);
	//AddAction(InActionSet, PokePoseAction, PokePoseActionPath, XR_ACTION_TYPE_POSE_INPUT);
	//AddAction(InActionSet, PinchAction, PinchActionPath, XR_ACTION_TYPE_FLOAT_INPUT);
	AddAction(InActionSet, AimAction, AimActionPath, XR_ACTION_TYPE_FLOAT_INPUT);
	AddAction(InActionSet, GraspAction, GraspActionPath, XR_ACTION_TYPE_FLOAT_INPUT);
}

FName FViveOpenXRHandInteraction::FViveInteractionController::GetActionName(FOpenXRPath ActionPath)
{
	TArray<FString> Tokens;
	ActionPath.ToString().ParseIntoArray(Tokens, TEXT("/"));
	Tokens[4].RemoveFromEnd("_htc");
	FString ActionNameString = Tokens[2] + "_" + Tokens[4] + "_" + Tokens[5];
	//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("GetActionName %s"), *ActionNameString);
	return FName(ActionNameString);
}

void FViveOpenXRHandInteraction::FViveInteractionController::SyncActionStates(XrSession InSession)
{
	// left button Pinch / right button Pinch
	//XrActionStateGetInfo PinchActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	//PinchActionStateInfo.action = PinchAction;
	//PrimaryActionStateInfo.subactionPath = SubactionPaths[0];
	//XR_ENSURE(xrGetActionStateFloat(InSession, &PinchActionStateInfo, &PinchValueActionState));

	// left button Aim / right button Aim 
	XrActionStateGetInfo AimActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	AimActionStateInfo.action = AimAction;
	//MenuActionStateInfo.subactionPath = SubactionPaths[0];
	XR_ENSURE(xrGetActionStateFloat(InSession, &AimActionStateInfo, &AimValueActionState));
	
	// left button Grasp / right button Grasp 
	XrActionStateGetInfo GraspActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	GraspActionStateInfo.action = GraspAction;
	//MenuActionStateInfo.subactionPath = SubactionPaths[0];
	XR_ENSURE(xrGetActionStateFloat(InSession, &GraspActionStateInfo, &GraspValueActionState));
}

void FViveOpenXRHandInteraction::FViveInteractionController::CheckAndAddEnhancedInputAction(FEnhancedActionKeyMapping EnhancedActionKeyMapping)
{
	/*
	if (EnhancedActionKeyMapping.Key == PinchKey)
	{
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Key = %s."), *EnhancedActionKeyMapping.Key.GetFName().ToString());
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Action = %s."), *EnhancedActionKeyMapping.Action.GetFName().ToString());
		PinchInputActions.Emplace(EnhancedActionKeyMapping.Action);
	}
	*/
	if (EnhancedActionKeyMapping.Key == AimKey)
	{
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Key = %s."), *EnhancedActionKeyMapping.Key.GetFName().ToString());
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Action = %s."), *EnhancedActionKeyMapping.Action.GetFName().ToString());
		AimInputActions.Emplace(EnhancedActionKeyMapping.Action);
	}

	if (EnhancedActionKeyMapping.Key == GraspKey)
	{
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Key = %s."), *EnhancedActionKeyMapping.Key.GetFName().ToString());
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Action = %s."), *EnhancedActionKeyMapping.Action.GetFName().ToString());
		GraspInputActions.Emplace(EnhancedActionKeyMapping.Action);
	}
}

FViveOpenXRHandInteraction::FViveOpenXRHandInteraction(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
	IModularFeatures::Get().RegisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	RegisterOpenXRExtensionModularFeature();

	m_Instance = this;
	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("FViveOpenXRHandInteraction() register extension feature HandInteraction %p."), m_Instance);
}

FViveOpenXRHandInteraction::~FViveOpenXRHandInteraction()
{
	IModularFeatures::Get().UnregisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	UnregisterOpenXRExtensionModularFeature();
	bActionsAttached = false;
}

bool FViveOpenXRHandInteraction::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_EnableHandInteraction)
	{
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("GetRequiredExtensions() XR_HTC_hand_interaction."));
		//UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("GetRequiredExtensions() XR_EXT_hand_interaction."));
		OutExtensions.Add("XR_HTC_hand_interaction");
		//OutExtensions.Add("XR_EXT_hand_interaction");
	}

	return true;
}

void FViveOpenXRHandInteraction::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("AttachActionSets() Entry."));
	if (!m_EnableHandInteraction) return;

	check(Instance != XR_NULL_HANDLE);
	LeftInteractionController.AddTrackedDevices(OpenXRHMD);
	RightInteractionController.AddTrackedDevices(OpenXRHMD);
	OutActionSets.Add(HandInteractionActionSet);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// For enhanced input
	if (!MappableInputConfig)
	{
		// Attempt to load the default input config from the OpenXR input settings.
		UOpenXRInputSettings* InputSettings = GetMutableDefault<UOpenXRInputSettings>();
		if (InputSettings && InputSettings->MappableInputConfig.IsValid())
		{
			SetPlayerMappableInputConfig((UPlayerMappableInputConfig*)InputSettings->MappableInputConfig.TryLoad());
		}
	}

	if (MappableInputConfig)
	{
		for (const TPair<TObjectPtr<UInputMappingContext>, int32> MappingContext : MappableInputConfig->GetMappingContexts())
		{
			for (const FEnhancedActionKeyMapping& Mapping : MappingContext.Key->GetMappings())
			{
				LeftInteractionController.CheckAndAddEnhancedInputAction(Mapping);
				RightInteractionController.CheckAndAddEnhancedInputAction(Mapping);
			}
		}
	}
	else
	{
		// Setup Key to ActionState map
		//KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Left_Pinch_Value.GetFName(), &LeftInteractionController.PinchValueActionState);
		//KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Right_Pinch_Value.GetFName(), &RightInteractionController.PinchValueActionState);
		KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Left_Aim_Value.GetFName(), &LeftInteractionController.AimValueActionState);
		KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Right_Aim_Value.GetFName(), &RightInteractionController.AimValueActionState);
		KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Left_Grasp_Value.GetFName(), &LeftInteractionController.GraspValueActionState);
		KeyActionStates.Emplace(HandInteractionKeys::HandInteraction_Right_Grasp_Value.GetFName(), &RightInteractionController.GraspValueActionState);
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	bActionsAttached = true;
}

void FViveOpenXRHandInteraction::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	if (!m_EnableHandInteraction) return;

	check(HandInteractionActionSet != XR_NULL_HANDLE);
	OutActiveSets.Add(XrActiveActionSet{ HandInteractionActionSet, XR_NULL_PATH });
}

void FViveOpenXRHandInteraction::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
}

const void* FViveOpenXRHandInteraction::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (m_EnableHandInteraction)
	{
		Instance = InInstance;

		static FName SystemName(TEXT("OpenXR"));
		if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
		{
			OpenXRHMD = (FOpenXRHMD*)GEngine->XRSystem.Get();
		}

		bSessionStarted = true;

		// Setup FKey
		//LeftInteractionController.PinchKey = HandInteractionKeys::HandInteraction_Left_Pinch_Value;
		LeftInteractionController.AimKey = HandInteractionKeys::HandInteraction_Left_Aim_Value;
		LeftInteractionController.GraspKey = HandInteractionKeys::HandInteraction_Left_Grasp_Value;
		//RightInteractionController.PinchKey = HandInteractionKeys::HandInteraction_Right_Pinch_Value;
		RightInteractionController.AimKey = HandInteractionKeys::HandInteraction_Right_Aim_Value;
		RightInteractionController.GraspKey = HandInteractionKeys::HandInteraction_Right_Grasp_Value;

		// Setup XrPath (include SubActionPaths)
		//LeftInteractionController.SetupPath(HandInteractionRolePath::Left, HandInteractionActionPath::LeftPinchPose, HandInteractionActionPath::LeftAimPose, HandInteractionActionPath::LeftGripPose, HandInteractionActionPath::LeftPokePose, HandInteractionActionPath::LeftPinchValue, HandInteractionActionPath::LeftAimValue, HandInteractionActionPath::LeftGraspValue);
		//RightInteractionController.SetupPath(HandInteractionRolePath::Right, HandInteractionActionPath::RightPinchPose, HandInteractionActionPath::RightAimPose, HandInteractionActionPath::RightGripPose, HandInteractionActionPath::RightPokePose, HandInteractionActionPath::RightPinchValue, HandInteractionActionPath::RightAimValue, HandInteractionActionPath::RightGraspValue);
		// Setup XrPath (include SubActionPaths)
		LeftInteractionController.HTCSetupPath(HTCHandInteractionRolePath::Left, HTCHandInteractionActionPath::LeftAimPose, HTCHandInteractionActionPath::LeftGripPose, HTCHandInteractionActionPath::LeftAimValue, HTCHandInteractionActionPath::LeftGraspValue);
		RightInteractionController.HTCSetupPath(HTCHandInteractionRolePath::Right, HTCHandInteractionActionPath::RightAimPose, HTCHandInteractionActionPath::RightGripPose, HTCHandInteractionActionPath::RightAimValue, HTCHandInteractionActionPath::RightGraspValue);

		//	Create ActionSet
		if (HandInteractionActionSet != XR_NULL_HANDLE)
		{
			xrDestroyActionSet(HandInteractionActionSet);
			HandInteractionActionSet = XR_NULL_HANDLE;
		}
		{
			XrActionSetCreateInfo Info;
			Info.type = XR_TYPE_ACTION_SET_CREATE_INFO;
			Info.next = nullptr;
			FCStringAnsi::Strcpy(Info.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "viveopenxrhandinteractionactionset");
			FCStringAnsi::Strcpy(Info.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "VIVE OpenXR Hand Interaction Action Set");
			Info.priority = 0;
			XR_ENSURE(xrCreateActionSet(Instance, &Info, &HandInteractionActionSet));
		}

		// Create Action
		LeftInteractionController.AddActions(HandInteractionActionSet);
		RightInteractionController.AddActions(HandInteractionActionSet);

		// Create suggested bindings
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("xrSuggestInteractionProfileBindings()"));
		TArray<XrActionSuggestedBinding> Bindings;
		LeftInteractionController.GetSuggestedBindings(Bindings);
		RightInteractionController.GetSuggestedBindings(Bindings);
		XrInteractionProfileSuggestedBinding InteractionProfileSuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		//InteractionProfileSuggestedBindings.interactionProfile = FOpenXRPath("/interaction_profiles/ext/hand_interaction_ext");
		InteractionProfileSuggestedBindings.interactionProfile = FOpenXRPath("/interaction_profiles/htc/hand_interaction");
		InteractionProfileSuggestedBindings.suggestedBindings = Bindings.GetData();
		InteractionProfileSuggestedBindings.countSuggestedBindings = Bindings.Num();
		XR_ENSURE(xrSuggestInteractionProfileBindings(Instance, &InteractionProfileSuggestedBindings));
	}

	return InNext;
}

void FViveOpenXRHandInteraction::OnDestroySession(XrSession InSession)
{
	if (bActionsAttached)
	{
		// If the session shut down, clean up.
		bActionsAttached = false;
		MappableInputConfig = nullptr;
		bSessionStarted = false;
		KeyActionStates.Reset();
	}
}

const void* FViveOpenXRHandInteraction::OnBeginSession(XrSession InSession, const void* InNext)
{
	if (m_EnableHandInteraction)
	{
		bSessionStarted = true;
	}

	return InNext;
}

void FViveOpenXRHandInteraction::PostSyncActions(XrSession InSession)
{
	if (!m_EnableHandInteraction) return;

	WorldToMetersScale_ = OpenXRHMD->GetWorldToMetersScale();

	LeftInteractionController.SyncActionStates(InSession);
	RightInteractionController.SyncActionStates(InSession);

	SendInputEvent_Legacy();
	SendInputEvent_EnhancedInput();
}

#pragma region
bool FViveOpenXRHandInteraction::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{	/*
		if (MotionSource == HandInteractionMotionSource::LeftPinch || MotionSource == HandInteractionMotionSource::RightPinch)
		{
			int32 DeviceId = LeftInteractionController.DeviceId;
			if (MotionSource == HandInteractionMotionSource::RightPinch) DeviceId = RightInteractionController.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetCurrentPose(DeviceId, Orientation, OutPosition);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
		*/
		if (MotionSource == HandInteractionMotionSource::LeftAim || MotionSource == HandInteractionMotionSource::RightAim)
		{
			int32 AimDeviceId = LeftInteractionController.AimDeviceId;
			if (MotionSource == HandInteractionMotionSource::RightAim) 
				AimDeviceId = RightInteractionController.AimDeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetCurrentPose(AimDeviceId, Orientation, OutPosition);

			OutOrientation = FRotator(Orientation);
			return Success;
		}
		if (MotionSource == HandInteractionMotionSource::LeftGrip || MotionSource == HandInteractionMotionSource::RightGrip)
		{
			int32 GripDeviceId = LeftInteractionController.GripDeviceId;
			if (MotionSource == HandInteractionMotionSource::RightGrip) 
				GripDeviceId = RightInteractionController.GripDeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetCurrentPose(GripDeviceId, Orientation, OutPosition);
			
			OutOrientation = FRotator(Orientation);
			return Success;
		}
		/*
		if (MotionSource == HandInteractionMotionSource::LeftPoke || MotionSource == HandInteractionMotionSource::RightPoke)
		{
			int32 DeviceId = LeftInteractionController.DeviceId;
			if (MotionSource == HandInteractionMotionSource::RightPoke) DeviceId = RightInteractionController.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetCurrentPose(DeviceId, Orientation, OutPosition);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
		*/
		
	}
	return false;
}

bool FViveOpenXRHandInteraction::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	// FTimespan initializes to 0 and GetControllerOrientationAndPositionForTime with time 0 will return the latest data.
	FTimespan Time;
	bool OutTimeWasUsed = false;
	return GetControllerOrientationAndPositionForTime(ControllerIndex, MotionSource, Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);

}

bool FViveOpenXRHandInteraction::GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{
		if (MotionSource == HandInteractionMotionSource::LeftAim || MotionSource == HandInteractionMotionSource::RightAim)
		{
			int32 DeviceId = LeftInteractionController.AimDeviceId;
			if (MotionSource == HandInteractionMotionSource::RightAim) 
				DeviceId = RightInteractionController.AimDeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
		if (MotionSource == HandInteractionMotionSource::LeftGrip || MotionSource == HandInteractionMotionSource::RightGrip)
		{
			int32 DeviceId = LeftInteractionController.GripDeviceId;
			if (MotionSource == HandInteractionMotionSource::RightGrip) 
				DeviceId = RightInteractionController.GripDeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
	}
	return false;
}

/*
		if (MotionSource == HandInteractionMotionSource::LeftPinch || MotionSource == HandInteractionMotionSource::RightPinch)
		{
			int32 DeviceId = LeftInteractionController.DeviceId;
			if (MotionSource == HandInteractionMotionSource::RightPinch) DeviceId = RightInteractionController.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
		if (MotionSource == HandInteractionMotionSource::LeftPoke || MotionSource == HandInteractionMotionSource::RightPoke)
		{
			int32 DeviceId = LeftInteractionController.DeviceId;
			if (MotionSource == HandInteractionMotionSource::RightPoke) DeviceId = RightInteractionController.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
*/

ETrackingStatus FViveOpenXRHandInteraction::GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const
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

	//if (MotionSource == HandInteractionMotionSource::LeftPinch)
	//{
		// left entity pose
		//PoseActionStateInfo.action = LeftInteractionController.PinchPoseAction;
	//}
	//else if (MotionSource == HandInteractionMotionSource::RightPinch)
	//{
		// right entity pose
		//PoseActionStateInfo.action = RightInteractionController.PinchPoseAction;
	//}
	if (MotionSource == HandInteractionMotionSource::LeftAim)
	{
		PoseActionStateInfo.action = LeftInteractionController.AimPoseAction;
	}
	else if (MotionSource == HandInteractionMotionSource::RightAim)
	{
		PoseActionStateInfo.action = RightInteractionController.AimPoseAction;
	}
	else if (MotionSource == HandInteractionMotionSource::LeftGrip)
	{
		PoseActionStateInfo.action = LeftInteractionController.GripPoseAction;
	}
	else if (MotionSource == HandInteractionMotionSource::RightGrip)
	{
		PoseActionStateInfo.action = RightInteractionController.GripPoseAction;
	}
	//else if (MotionSource == HandInteractionMotionSource::LeftPoke)
	//{
		// right entity pose
		//PoseActionStateInfo.action = LeftInteractionController.PokePoseAction;
	//}
	//else if (MotionSource == HandInteractionMotionSource::RightPoke)
	//{
		// right entity pose
		//PoseActionStateInfo.action = RightInteractionController.PokePoseAction;
	//}
	else
	{
		return ETrackingStatus::NotTracked;
	}
	PoseActionStateInfo.subactionPath = XR_NULL_PATH;

	XrActionStatePose State = { XR_TYPE_ACTION_STATE_POSE };
	if (!XR_ENSURE(xrGetActionStatePose(Session, &PoseActionStateInfo, &State)))
	{
		return ETrackingStatus::NotTracked;
	}

	return State.isActive ? ETrackingStatus::Tracked : ETrackingStatus::NotTracked;
}

FName FViveOpenXRHandInteraction::GetMotionControllerDeviceTypeName() const
{
	const static FName DefaultName(TEXT("OpenXRViveHandInteraction"));
	return DefaultName;
}

void FViveOpenXRHandInteraction::EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const
{
	check(IsInGameThread());

	//SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::LeftPinch));
	//SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::RightPinch));
	SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::LeftAim));
	SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::RightAim));
	SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::LeftGrip));
	SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::RightGrip));
	//SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::LeftPoke));
	//SourcesOut.Add(FMotionControllerSource(HandInteractionMotionSource::RightPoke));
}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
bool FViveOpenXRHandInteraction::SetPlayerMappableInputConfig(TObjectPtr<class UPlayerMappableInputConfig> InputConfig)
{
	if (bActionsAttached)
	{
		UE_LOG(LogHMD, Error, TEXT("Attempted to attach an input config while one is already attached for the current session."));

		return false;
	}

	MappableInputConfig = TStrongObjectPtr<class UPlayerMappableInputConfig>(InputConfig);
	return true;
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

void FViveOpenXRHandInteraction::SendInputEvent_Legacy()
{
	if (!m_EnableHandInteraction) return;

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

	for (auto& KeyActionState : KeyActionStates)
	{
		XrActionStateFloat state = *KeyActionState.Value;
		if (state.changedSinceLastSync)
		{
			FName keyName = KeyActionState.Key;
			UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("SendControllerEvents() %s = %d."), *keyName.ToString(), (uint8_t)state.currentState);
			if (state.isActive && state.currentState)
			{
				MessageHandler->OnControllerButtonPressed(keyName, DeviceMapper.GetPrimaryPlatformUser(), DeviceMapper.GetDefaultInputDevice(), /*IsRepeat =*/false);
			}
			else
			{
				MessageHandler->OnControllerButtonReleased(keyName, DeviceMapper.GetPrimaryPlatformUser(), DeviceMapper.GetDefaultInputDevice(), /*IsRepeat =*/false);
			}
		}
	}
}

void FViveOpenXRHandInteraction::SendInputEvent_EnhancedInput()
{
	if (!m_EnableHandInteraction) return;

	auto InjectEnhancedInput = [](XrActionStateFloat& State, TArray<TObjectPtr<const UInputAction>>& Actions)
	{
		FInputActionValue InputValue;
		TArray<TObjectPtr<UInputTrigger>> Triggers = {};
		TArray<TObjectPtr<UInputModifier>> Modifiers = {};

		InputValue = FInputActionValue(State.isActive ? (bool)State.currentState : false);
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("SendInputEvent_EnhancedInput()."));

		for (auto InputAction : Actions)
		{
			//if(State.isActive) UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("SendInputEvent_EnhancedInput() Action: %s, Value: %d"), *InputAction.GetFName().ToString(), (uint8_t)State.currentState);
			auto InjectSubsystemInput = [InputAction, InputValue, Triggers, Modifiers](IEnhancedInputSubsystemInterface* Subsystem)
			{
				if (Subsystem)
				{
					Subsystem->InjectInputForAction(InputAction, InputValue, Modifiers, Triggers);
				}
			};
			IEnhancedInputModule::Get().GetLibrary()->ForEachSubsystem(InjectSubsystemInput);

#if WITH_EDITOR
			if (GEditor)
			{
				// UEnhancedInputLibrary::ForEachSubsystem only enumerates runtime subsystems.
				InjectSubsystemInput(GEditor->GetEditorSubsystem<UEnhancedInputEditorSubsystem>());
			}
#endif
		}

	};

	//InjectEnhancedInput(LeftInteractionController.PinchValueActionState, LeftInteractionController.PinchInputActions);
	InjectEnhancedInput(LeftInteractionController.AimValueActionState, LeftInteractionController.AimInputActions);
	InjectEnhancedInput(LeftInteractionController.GraspValueActionState, LeftInteractionController.GraspInputActions);
	//InjectEnhancedInput(RightInteractionController.PinchValueActionState, RightInteractionController.PinchInputActions);
	InjectEnhancedInput(RightInteractionController.AimValueActionState, RightInteractionController.AimInputActions);
	InjectEnhancedInput(RightInteractionController.GraspValueActionState, RightInteractionController.GraspInputActions);
}
#pragma endregion IInputDevice overrides

FViveOpenXRHandInteractionModule::FViveOpenXRHandInteractionModule()
{
}

FName IViveOpenXRHandInteractionModule::ViveOpenXRHandInteractionModularKeyName = FName(TEXT("ViveOpenXRHandInteraction"));

void FViveOpenXRHandInteractionModule::StartupModule()
{
	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("StartupModule() Entry."));
	//IViveOpenXRHandInteractionModule::StartupModule();

	TSharedPtr<FGenericApplicationMessageHandler> DummyMessageHandler(new FGenericApplicationMessageHandler());
	CreateInputDevice(DummyMessageHandler.ToSharedRef());

	EKeys::AddMenuCategoryDisplayInfo("Hand Interaction", LOCTEXT("HandInteractionSubCategory", "HTC Hand Interaction"), TEXT("GraphEditor.PadEvent_16x"));

	/// ---- Left hand only ----
	//EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Left_Pinch_Value, LOCTEXT("HandInteraction_Left_Pinch_Value", "Hand Interaction (L) Pinch Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));
	EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Left_Aim_Value, LOCTEXT("HandInteraction_Left_Aim_Value", "Hand Interaction (L) Aim Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));
	EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Left_Grasp_Value, LOCTEXT("HandInteraction_Left_Grasp_Value", "Hand Interaction (L) Grasp Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));

	/// ---- Right hand only ----
	//EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Right_Pinch_Value, LOCTEXT("HandInteraction_Right_Pinch_Value", "Hand Interaction (R) Pinch Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));
	EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Right_Aim_Value, LOCTEXT("HandInteraction_Right_Aim_Value", "Hand Interaction (R) Aim Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));
	EKeys::AddKey(FKeyDetails(HandInteractionKeys::HandInteraction_Right_Grasp_Value, LOCTEXT("HandInteraction_Right_Grasp_Value", "Hand Interaction (R) Grasp Value"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Hand Interaction"));

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableHandInteraction"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			GetHandInteraction()->m_EnableHandInteraction = false;
		}
		else if (modeName.Equals("True"))
		{
			GetHandInteraction()->m_EnableHandInteraction = true;
		}
	}

	if (GetHandInteraction()->m_EnableHandInteraction)
	{
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("Enable Hand Interaction."));
	}
	else
	{
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("Disable Hand Interaction."));
	}
	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("StartupModule() Finished."));
}

void FViveOpenXRHandInteractionModule::ShutdownModule()
{
	UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("ShutdownModule()"));
}

TSharedPtr<class IInputDevice> FViveOpenXRHandInteractionModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!HandInteractionController.IsValid())
	{
		auto InputDevice = new FViveOpenXRHandInteraction(InMessageHandler);
		HandInteractionController = TSharedPtr<FViveOpenXRHandInteraction>(InputDevice);
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("CreateInputDevice() new HandInteractionController %p"), HandInteractionController.Get());

		return HandInteractionController;
	}
	else
	{
		HandInteractionController.Get()->SetMessageHandler(InMessageHandler);
		UE_LOG(LogViveOpenXRHandInteraction, Log, TEXT("CreateInputDevice() update HandInteractionController %p"), HandInteractionController.Get());

		return HandInteractionController;
	}
	return nullptr;
}

FViveOpenXRHandInteraction* FViveOpenXRHandInteractionModule::GetHandInteraction()
{
	return FViveOpenXRHandInteraction::GetInstance();
}

IMPLEMENT_MODULE(FViveOpenXRHandInteractionModule, ViveOpenXRHandInteraction);

#undef LOCTEXT_NAMESPACE