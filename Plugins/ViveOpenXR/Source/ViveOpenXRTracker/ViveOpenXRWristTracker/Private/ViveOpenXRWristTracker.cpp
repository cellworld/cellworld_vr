// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRWristTracker.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "UObject/UObjectIterator.h"
#include "IOpenXRExtensionPlugin.h"
#include "Modules/ModuleManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"

#include "EnhancedInputLibrary.h"
#include "EnhancedInputSubsystemInterface.h"
#include "EnhancedInputModule.h"
#include "PlayerMappableInputConfig.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "OpenXRInputSettings.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "EnhancedInputEditorSubsystem.h"
#endif

DEFINE_LOG_CATEGORY(LogViveOpenXRWristTracker);

/************************************************************************/
/* FViveOpenXRWristTracker                                              */
/************************************************************************/

FViveOpenXRWristTracker* FViveOpenXRWristTracker::m_Instance = nullptr;

FViveOpenXRWristTracker::FViveWristTracker::FViveWristTracker()
	: RolePath(XR_NULL_PATH)
	, PoseAction(XR_NULL_HANDLE)
	, PoseActionPath(XR_NULL_PATH)
	, PrimaryAction(XR_NULL_HANDLE)
	, PrimaryActionPath(XR_NULL_PATH)
	, MenuAction(XR_NULL_HANDLE)
	, MenuActionPath(XR_NULL_PATH)
	, DeviceId(-1)
	, PrimaryActionState({ XR_TYPE_ACTION_STATE_BOOLEAN })
	, MenuActionState({ XR_TYPE_ACTION_STATE_BOOLEAN })
	, PrimaryKey(TEXT(""))
	, MenuKey(TEXT(""))
{
	SubactionPaths.RemoveAll([](const int& num) {
		return true;
		});
	PrimaryInputActions.Empty();
	MenuInputActions.Empty();
}

void FViveOpenXRWristTracker::FViveWristTracker::SetupPath(FString InRolePath, FString InPoseActionPath, FString InPrimaryActionPath, FString InMenuActionPath)
{
	RolePath = FOpenXRPath(InRolePath);
	PoseActionPath = FOpenXRPath(InPoseActionPath);
	PrimaryActionPath = FOpenXRPath(InPrimaryActionPath);
	MenuActionPath = FOpenXRPath(InMenuActionPath);
	SubactionPaths.Reset();
	SubactionPaths.Emplace(RolePath);
}

int32 FViveOpenXRWristTracker::FViveWristTracker::AddTrackedDevices(FOpenXRHMD* HMD)
{
	if (HMD)
	{
		DeviceId = HMD->AddTrackedDevice(PoseAction, PoseActionPath);
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("AddTrackedDevices ID: %d"), DeviceId);
	}
	return DeviceId;
}

void FViveOpenXRWristTracker::FViveWristTracker::GetSuggestedBindings(TArray<XrActionSuggestedBinding>& OutSuggestedBindings)
{
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ PoseAction, PoseActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ PrimaryAction, PrimaryActionPath });
	OutSuggestedBindings.Add(XrActionSuggestedBinding{ MenuAction, MenuActionPath });
}

void FViveOpenXRWristTracker::FViveWristTracker::AddAction(XrActionSet& InActionSet, XrAction& OutAction, FOpenXRPath InBindingPath, XrActionType InActionType)//, const TArray<XrPath>& InSubactionPaths)
{
	check(InActionSet != XR_NULL_HANDLE);
	if (OutAction != XR_NULL_HANDLE) {
		xrDestroyAction(OutAction);
		OutAction = XR_NULL_HANDLE;
	}

	UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("xrCreateAction %s"), *InBindingPath.ToString());

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

void FViveOpenXRWristTracker::FViveWristTracker::AddActions(XrActionSet& InActionSet)
{
	AddAction(InActionSet, PoseAction, PoseActionPath, XR_ACTION_TYPE_POSE_INPUT);
	AddAction(InActionSet, PrimaryAction, PrimaryActionPath, XR_ACTION_TYPE_BOOLEAN_INPUT);
	AddAction(InActionSet, MenuAction, MenuActionPath, XR_ACTION_TYPE_BOOLEAN_INPUT);
}

FName FViveOpenXRWristTracker::FViveWristTracker::GetActionName(FOpenXRPath ActionPath)
{
	TArray<FString> Tokens;
	ActionPath.ToString().ParseIntoArray(Tokens, TEXT("/"));
	Tokens[4].RemoveFromEnd("_htc");
	FString ActionNameString = Tokens[2] + "_" + Tokens[4] + "_" + Tokens[5];
	//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("GetActionName %s"), *ActionNameString);
	return FName(ActionNameString);
}

void FViveOpenXRWristTracker::FViveWristTracker::SyncActionStates(XrSession InSession)
{
	// left button x click / right button a click
	XrActionStateGetInfo PrimaryActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	PrimaryActionStateInfo.action = PrimaryAction;
	//PrimaryActionStateInfo.subactionPath = SubactionPaths[0];
	XR_ENSURE(xrGetActionStateBoolean(InSession, &PrimaryActionStateInfo, &PrimaryActionState));

	// left button menu click / right button system click
	XrActionStateGetInfo MenuActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	MenuActionStateInfo.action = MenuAction;
	//MenuActionStateInfo.subactionPath = SubactionPaths[0];
	XR_ENSURE(xrGetActionStateBoolean(InSession, &MenuActionStateInfo, &MenuActionState));
}

void FViveOpenXRWristTracker::FViveWristTracker::CheckAndAddEnhancedInputAction(FEnhancedActionKeyMapping EnhancedActionKeyMapping)
{
	if (EnhancedActionKeyMapping.Key == PrimaryKey)
	{
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Key = %s."), *EnhancedActionKeyMapping.Key.GetFName().ToString());
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Action = %s."), *EnhancedActionKeyMapping.Action.GetFName().ToString());
		PrimaryInputActions.Emplace(EnhancedActionKeyMapping.Action);
	}

	if (EnhancedActionKeyMapping.Key == MenuKey)
	{
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Key = %s."), *EnhancedActionKeyMapping.Key.GetFName().ToString());
		//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CheckAndAddEnhanceInputAction() Action = %s."), *EnhancedActionKeyMapping.Action.GetFName().ToString());
		MenuInputActions.Emplace(EnhancedActionKeyMapping.Action);
	}
}

FViveOpenXRWristTracker::FViveOpenXRWristTracker(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
	IModularFeatures::Get().RegisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	RegisterOpenXRExtensionModularFeature();

	m_Instance = this;
	UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("FViveOpenXRWristTracker() register extension feature WristTracker %p."), m_Instance);
}

FViveOpenXRWristTracker::~FViveOpenXRWristTracker()
{
	IModularFeatures::Get().UnregisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	UnregisterOpenXRExtensionModularFeature();
	bActionsAttached = false;
}

bool FViveOpenXRWristTracker::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableWristTracker)
	{
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("GetRequiredExtensions() XR_HTC_vive_wrist_tracker_interaction."));
		OutExtensions.Add("XR_HTC_vive_wrist_tracker_interaction");
	}
	return true;
}

//bool FViveOpenXRWristTracker::GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
//{
//	if (!m_bEnableWristTracker) return false; 
//	UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("GetInteractionProfile() /interaction_profiles/htc/vive_wrist_tracker."));
//	OutKeyPrefix = "WristTracker";
//	OutHasHaptics = false;
//	return xrStringToPath(InInstance, "/interaction_profiles/htc/vive_wrist_tracker", &OutPath) == XR_SUCCESS;
//}

void FViveOpenXRWristTracker::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	if (!m_bEnableWristTracker) return;

	check(Instance != XR_NULL_HANDLE);

	LeftWristTracker.AddTrackedDevices(OpenXRHMD);
	RightWristTracker.AddTrackedDevices(OpenXRHMD);
	OutActionSets.Add(WristTrackerActionSet);

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
				LeftWristTracker.CheckAndAddEnhancedInputAction(Mapping);
				RightWristTracker.CheckAndAddEnhancedInputAction(Mapping);
			}
		}
	}
	else
	{
		// Setup Key to ActionState map
		KeyActionStates.Emplace(WristTrackerKeys::WristTracker_Left_X_Click.GetFName(), &LeftWristTracker.PrimaryActionState);
		KeyActionStates.Emplace(WristTrackerKeys::WristTracker_Right_A_Click.GetFName(), &RightWristTracker.PrimaryActionState);
		KeyActionStates.Emplace(WristTrackerKeys::WristTracker_Left_Menu_Click.GetFName(), &LeftWristTracker.MenuActionState);
		KeyActionStates.Emplace(WristTrackerKeys::WristTracker_Right_System_Click.GetFName(), &RightWristTracker.MenuActionState);
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	bActionsAttached = true;
}

void FViveOpenXRWristTracker::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	if (!m_bEnableWristTracker) return;

	check(WristTrackerActionSet != XR_NULL_HANDLE);
	OutActiveSets.Add(XrActiveActionSet{WristTrackerActionSet, XR_NULL_PATH});
}

void FViveOpenXRWristTracker::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
}

const void* FViveOpenXRWristTracker::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (m_bEnableWristTracker)
	{
		Instance = InInstance;

		static FName SystemName(TEXT("OpenXR"));
		if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
		{
			OpenXRHMD = (FOpenXRHMD*)GEngine->XRSystem.Get();
		}

		bSessionStarted = true;

		// Setup FKey
		LeftWristTracker.PrimaryKey = WristTrackerKeys::WristTracker_Left_X_Click;
		LeftWristTracker.MenuKey = WristTrackerKeys::WristTracker_Left_Menu_Click;
		RightWristTracker.PrimaryKey = WristTrackerKeys::WristTracker_Right_A_Click;
		RightWristTracker.MenuKey = WristTrackerKeys::WristTracker_Right_System_Click;

		// Setup XrPath (include SubActionPaths)
		LeftWristTracker.SetupPath(WristTrackerRolePath::Left, WristTrackerActionPath::LeftPose, WristTrackerActionPath::LeftPrimary, WristTrackerActionPath::LeftMenu);
		RightWristTracker.SetupPath(WristTrackerRolePath::Right, WristTrackerActionPath::RightPose, WristTrackerActionPath::RightPrimary, WristTrackerActionPath::RightMenu);

		//	Create ActionSet
		if (WristTrackerActionSet != XR_NULL_HANDLE)
		{
			xrDestroyActionSet(WristTrackerActionSet);
			WristTrackerActionSet = XR_NULL_HANDLE;
		}
		{
			XrActionSetCreateInfo Info;
			Info.type = XR_TYPE_ACTION_SET_CREATE_INFO;
			Info.next = nullptr;
			FCStringAnsi::Strcpy(Info.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, "viveopenxrwristtrackeractionset");
			FCStringAnsi::Strcpy(Info.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, "VIVE OpenXR Wrist Tracker Action Set");
			Info.priority = 0;
			XR_ENSURE(xrCreateActionSet(Instance, &Info, &WristTrackerActionSet));
		}
		
		// Create Action
		LeftWristTracker.AddActions(WristTrackerActionSet);
		RightWristTracker.AddActions(WristTrackerActionSet);

		// Create suggested bindings
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("xrSuggestInteractionProfileBindings()"));
		TArray<XrActionSuggestedBinding> Bindings;
		LeftWristTracker.GetSuggestedBindings(Bindings);
		RightWristTracker.GetSuggestedBindings(Bindings);
		XrInteractionProfileSuggestedBinding InteractionProfileSuggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		InteractionProfileSuggestedBindings.interactionProfile = FOpenXRPath("/interaction_profiles/htc/vive_wrist_tracker");
		InteractionProfileSuggestedBindings.suggestedBindings = Bindings.GetData();
		InteractionProfileSuggestedBindings.countSuggestedBindings = Bindings.Num();
		XR_ENSURE(xrSuggestInteractionProfileBindings(Instance, &InteractionProfileSuggestedBindings));
	}

	return InNext;
}

void FViveOpenXRWristTracker::OnDestroySession(XrSession InSession)
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

const void* FViveOpenXRWristTracker::OnBeginSession(XrSession InSession, const void* InNext)
{
	if (m_bEnableWristTracker)
	{
		bSessionStarted = true;
	}

	return InNext;
}

void FViveOpenXRWristTracker::PostSyncActions(XrSession InSession)
{
	if (!m_bEnableWristTracker) return;

	WorldToMetersScale_ = OpenXRHMD->GetWorldToMetersScale();

	LeftWristTracker.SyncActionStates(InSession);
	RightWristTracker.SyncActionStates(InSession);

	SendInputEvent_Legacy();
	SendInputEvent_EnhancedInput();
}

void FViveOpenXRWristTracker::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
}

#pragma region
bool FViveOpenXRWristTracker::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{
		if (MotionSource == WristTrackerMotionSource::Left || MotionSource == WristTrackerMotionSource::Right)
		{
			int32 DeviceId = LeftWristTracker.DeviceId;
			if (MotionSource == WristTrackerMotionSource::Right) DeviceId = RightWristTracker.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetCurrentPose(DeviceId, Orientation, OutPosition);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
	}
	return false;
}

bool FViveOpenXRWristTracker::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	// FTimespan initializes to 0 and GetControllerOrientationAndPositionForTime with time 0 will return the latest data.
	FTimespan Time;
	bool OutTimeWasUsed = false;
	return GetControllerOrientationAndPositionForTime(ControllerIndex, MotionSource, Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
}

bool FViveOpenXRWristTracker::GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return false;
	}

	if (ControllerIndex == DeviceIndex)
	{
		if (MotionSource == WristTrackerMotionSource::Left || MotionSource == WristTrackerMotionSource::Right)
		{
			int32 DeviceId = LeftWristTracker.DeviceId;
			if (MotionSource == WristTrackerMotionSource::Right) DeviceId = RightWristTracker.DeviceId;

			FQuat Orientation;
			bool Success = OpenXRHMD->GetPoseForTime(DeviceId, Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
			OutOrientation = FRotator(Orientation);
			return Success;
		}
	}

	return false;
}

ETrackingStatus FViveOpenXRWristTracker::GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const
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

	if (MotionSource == WristTrackerMotionSource::Left)
	{
		// left entity pose
		PoseActionStateInfo.action = LeftWristTracker.PoseAction;
	}
	else if (MotionSource == WristTrackerMotionSource::Right)
	{
		// right entity pose
		PoseActionStateInfo.action = RightWristTracker.PoseAction;
	}
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

FName FViveOpenXRWristTracker::GetMotionControllerDeviceTypeName() const
{
	const static FName DefaultName(TEXT("OpenXRViveWristTracker"));
	return DefaultName;
}

void FViveOpenXRWristTracker::EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const
{
	check(IsInGameThread());

	SourcesOut.Add(FMotionControllerSource(WristTrackerMotionSource::Left));
	SourcesOut.Add(FMotionControllerSource(WristTrackerMotionSource::Right));
}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
bool FViveOpenXRWristTracker::SetPlayerMappableInputConfig(TObjectPtr<class UPlayerMappableInputConfig> InputConfig)
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

void FViveOpenXRWristTracker::SendInputEvent_Legacy()
{
	if (!m_bEnableWristTracker) return;

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();

	for (auto &KeyActionState : KeyActionStates)
	{
		XrActionStateBoolean state = *KeyActionState.Value;
		if (state.changedSinceLastSync)
		{
			FName keyName = KeyActionState.Key;
			//UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("SendControllerEvents() %s = %d."), *keyName.ToString(), (uint8_t)state.currentState);
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

void FViveOpenXRWristTracker::SendInputEvent_EnhancedInput()
{
	if (!m_bEnableWristTracker) return;

	auto InjectEnhancedInput = [](XrActionStateBoolean& State, TArray<TObjectPtr<const UInputAction>>& Actions)
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

	InjectEnhancedInput(LeftWristTracker.PrimaryActionState, LeftWristTracker.PrimaryInputActions);
	InjectEnhancedInput(LeftWristTracker.MenuActionState, LeftWristTracker.MenuInputActions);
	InjectEnhancedInput(RightWristTracker.PrimaryActionState, RightWristTracker.PrimaryInputActions);
	InjectEnhancedInput(RightWristTracker.MenuActionState, RightWristTracker.MenuInputActions);
}
#pragma endregion IInputDevice overrides

/************************************************************************/
/* FViveOpenXRWristTrackerModule                                        */
/************************************************************************/

#define LOCTEXT_NAMESPACE "FViveOpenXRWristTrackerModule"

FName IViveOpenXRWristTrackerModule::ViveOpenXRWristTrackerModularKeyName = FName(TEXT("ViveOpenXRWristTracker"));

FViveOpenXRWristTrackerModule::FViveOpenXRWristTrackerModule()
{
}

void FViveOpenXRWristTrackerModule::StartupModule()
{
	UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("StartupModule()"));
	IViveOpenXRWristTrackerModule::StartupModule();

	TSharedPtr<FGenericApplicationMessageHandler> DummyMessageHandler(new FGenericApplicationMessageHandler());
	CreateInputDevice(DummyMessageHandler.ToSharedRef());

	EKeys::AddMenuCategoryDisplayInfo("Wrist Tracker", LOCTEXT("WristTrackerSubCategory", "HTC Wrist Tracker"), TEXT("GraphEditor.PadEvent_16x"));

	/// ---- Left hand only ----
	// "/input/menu/click"
	EKeys::AddKey(FKeyDetails(WristTrackerKeys::WristTracker_Left_Menu_Click, LOCTEXT("WristTracker_Left_Menu_Click", "Wrist Tracker (L) Menu Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Wrist Tracker"));
	// "/input/x/click"
	EKeys::AddKey(FKeyDetails(WristTrackerKeys::WristTracker_Left_X_Click, LOCTEXT("WristTracker_Left_X_Click", "Wrist Tracker (L) Primary Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Wrist Tracker"));

	/// ---- Right hand only ----
	// "/input/system/click"
	EKeys::AddKey(FKeyDetails(WristTrackerKeys::WristTracker_Right_System_Click, LOCTEXT("WristTracker_Right_System_Click", "Wrist Tracker (R) Menu Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Wrist Tracker"));
	// "/input/a/click"
	EKeys::AddKey(FKeyDetails(WristTrackerKeys::WristTracker_Right_A_Click, LOCTEXT("WristTracker_Right_A_Click", "Wrist Tracker (R) Primary Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Wrist Tracker"));

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableWristTracker"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			GetWristTracker()->m_bEnableWristTracker = false;
		}
		else if (modeName.Equals("True"))
		{
			GetWristTracker()->m_bEnableWristTracker = true;
		}
	}

	if (GetWristTracker()->m_bEnableWristTracker)
	{
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("Enable Wrist controller."));
	}
	else
	{
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("Disable Wrist controller."));
	}
}

void FViveOpenXRWristTrackerModule::ShutdownModule()
{
	UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("ShutdownModule()"));
}

TSharedPtr<class IInputDevice> FViveOpenXRWristTrackerModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	if (!WristTracker.IsValid())
	{
		auto InputDevice = new FViveOpenXRWristTracker(InMessageHandler);
		WristTracker = TSharedPtr<FViveOpenXRWristTracker>(InputDevice);
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CreateInputDevice() new WristTracker %p"), WristTracker.Get());

		return WristTracker;
	}
	else
	{
		WristTracker.Get()->SetMessageHandler(InMessageHandler);
		UE_LOG(LogViveOpenXRWristTracker, Log, TEXT("CreateInputDevice() update WristTracker %p"), WristTracker.Get());

		return WristTracker;
	}
	return nullptr;
}

FViveOpenXRWristTracker* FViveOpenXRWristTrackerModule::GetWristTracker()
{
	return FViveOpenXRWristTracker::GetInstance();
}

IMPLEMENT_MODULE(FViveOpenXRWristTrackerModule, ViveOpenXRWristTracker);

