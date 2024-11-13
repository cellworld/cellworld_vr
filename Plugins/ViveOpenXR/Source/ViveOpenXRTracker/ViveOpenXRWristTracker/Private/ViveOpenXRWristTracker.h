// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include "IViveOpenXRWristTrackerModule.h"
#include "ViveOpenXRWristTrackerEnums.h"
#include "XRMotionControllerBase.h"
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Containers/Ticker.h"
#include "IOpenXRExtensionPlugin.h"
#include "IInputDevice.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/StrongObjectPtr.h"

#include "OpenXRCore.h"
#include "OpenXRHMD.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRWristTracker, Log, All);

namespace Side
{
	const int RIGHT = 0;
	const int LEFT = 1;
	const int COUNT = 2;
}

namespace WristTrackerKeys
{
	/// ---- Left hand only ----
	// "/input/menu/click"
	const FKey WristTracker_Left_Menu_Click("WristTracker_Left_Menu_Click");
	// "/input/x/click"
	const FKey WristTracker_Left_X_Click("WristTracker_Left_X_Click");

	/// ---- Right hand only ----
	// "/input/system/click"
	const FKey WristTracker_Right_System_Click("WristTracker_Right_System_Click");
	// "/input/a/click"
	const FKey WristTracker_Right_A_Click("WristTracker_Right_A_Click");
}

namespace WristTrackerMotionSource
{
	const FName Left    = TEXT("LeftWristTracker");
	const FName Right   = TEXT("RightWristTracker");
}

namespace WristTrackerRolePath
{
	const FString Left  = FString("/user/wrist_htc/left");
	const FString Right = FString("/user/wrist_htc/right");
}

namespace WristTrackerActionPath
{
	const FString LeftPose      = FString("/user/wrist_htc/left/input/entity_htc/pose");
	const FString LeftPrimary   = FString("/user/wrist_htc/left/input/x/click");
	const FString LeftMenu      = FString("/user/wrist_htc/left/input/menu/click");
	const FString RightPose     = FString("/user/wrist_htc/right/input/entity_htc/pose");
	const FString RightPrimary  = FString("/user/wrist_htc/right/input/a/click");
	const FString RightMenu     = FString("/user/wrist_htc/right/input/system/click");
}

class FViveOpenXRWristTracker : 
	public IOpenXRExtensionPlugin, 
	public IInputDevice,
	public FXRMotionControllerBase
{
public:
	struct FViveWristTracker
	{
		XrPath          RolePath;
		XrAction        PoseAction;
		XrPath          PoseActionPath;
		XrAction        PrimaryAction;
		XrPath          PrimaryActionPath;
		XrAction        MenuAction;
		XrPath          MenuActionPath;
		int32           DeviceId;
		TArray<XrPath>  SubactionPaths;

		XrActionStateBoolean PrimaryActionState;
		XrActionStateBoolean MenuActionState;

		FKey PrimaryKey;
		FKey MenuKey;
		TArray<TObjectPtr<const UInputAction>> PrimaryInputActions;
		TArray<TObjectPtr<const UInputAction>> MenuInputActions;

		FViveWristTracker();

		void SetupPath(FString InRolePath, FString InPoseActionPath, FString InPrimaryActionPath, FString InMenuActionPath);
		int32 AddTrackedDevices(class FOpenXRHMD* HMD);
		void GetSuggestedBindings(TArray<XrActionSuggestedBinding>& OutSuggestedBindings);
		void AddAction(XrActionSet& InActionSet, XrAction& OutAction, FOpenXRPath InBindingPath, XrActionType InActionType);
		void AddActions(XrActionSet& InActionSet);
		FName GetActionName(FOpenXRPath ActionPath);
		void SyncActionStates(XrSession InSession);
		void CheckAndAddEnhancedInputAction(FEnhancedActionKeyMapping EnhancedActionKeyMapping);
	};

public:
	FViveOpenXRWristTracker(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FViveOpenXRWristTracker();

	/************************************************************************/
	/* IOpenXRExtensionPlugin                                               */
	/************************************************************************/

	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ViveOpenXRWristTracker"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	//virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual void PostSyncActions(XrSession InSession) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;

	static inline FViveOpenXRWristTracker* GetInstance() {
		return m_Instance;
	}

	inline float GetWorldToMeterScale() { return WorldToMetersScale_; }

	bool m_bEnableWristTracker = false;
protected:
	static FViveOpenXRWristTracker* m_Instance;

private:
	FViveWristTracker LeftWristTracker;
	FViveWristTracker RightWristTracker;

	bool bSessionStarted = false;
	bool bActionsAttached = false;

	XrInstance Instance = XR_NULL_HANDLE;
	XrActionSet WristTrackerActionSet = XR_NULL_HANDLE;
	
	class FOpenXRHMD* OpenXRHMD = nullptr;
	int32 DeviceIndex = 0;
	float WorldToMetersScale_ = 100;

public:
	/************************************************************************/
	/* IMotionController                                                    */
	/************************************************************************/
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual ETrackingStatus GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const override;
	virtual FName GetMotionControllerDeviceTypeName() const override;
	virtual void EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const override;

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	virtual bool SetPlayerMappableInputConfig(TObjectPtr<class UPlayerMappableInputConfig> InputConfig) override;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

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
	TArray<TTuple<FName, XrActionStateBoolean*>> KeyActionStates;
	
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	TStrongObjectPtr<class UPlayerMappableInputConfig> MappableInputConfig;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	void SendInputEvent_Legacy();
	void SendInputEvent_EnhancedInput();


#pragma region Deprecated public function
public:
	FQuat GetTrackerRotation(bool isLeft)
	{
		FRotator rotation;
		FVector location;
		FName motionSource = WristTrackerMotionSource::Left;
		if (!isLeft) motionSource = WristTrackerMotionSource::Right;
		GetControllerOrientationAndPosition(DeviceIndex, motionSource, rotation, location, WorldToMetersScale_);
		return rotation.Quaternion();
	}
	FVector GetTrackerPosition(bool isLeft)
	{
		FRotator rotation;
		FVector location;
		FName motionSource = WristTrackerMotionSource::Left;
		if (!isLeft) motionSource = WristTrackerMotionSource::Right;
		GetControllerOrientationAndPosition(DeviceIndex, motionSource, rotation, location, WorldToMetersScale_);
		return location;
	}
	bool GetTrackerActive(bool isLeft)
	{
		if (isLeft) return OpenXRHMD->GetIsTracked(LeftWristTracker.DeviceId);
		else return OpenXRHMD->GetIsTracked(RightWristTracker.DeviceId);
	}
	bool GetTrackerValid(bool isLeft)
	{
		if (isLeft) return OpenXRHMD->GetIsTracked(LeftWristTracker.DeviceId);
		else return OpenXRHMD->GetIsTracked(RightWristTracker.DeviceId);
	}
	bool GetTrackerTracked(bool isLeft)
	{
		if (isLeft) return OpenXRHMD->GetIsTracked(LeftWristTracker.DeviceId);
		else return OpenXRHMD->GetIsTracked(RightWristTracker.DeviceId);
	}
	bool GetTrackerKeyDown(bool isLeft, TrackerButton key)
	{
		switch (key)
		{
		case TrackerButton::Menu:
			return isLeft ? (LeftWristTracker.MenuActionState.currentState > 0) : (RightWristTracker.MenuActionState.currentState > 0);
		case TrackerButton::Primary:
			return isLeft ? (LeftWristTracker.PrimaryActionState.currentState > 0) : (RightWristTracker.PrimaryActionState.currentState > 0);
		default:
			break;
		}
		return false;
	}
#pragma endregion
};

class FViveOpenXRWristTrackerModule : public IViveOpenXRWristTrackerModule
{
public:
	FViveOpenXRWristTrackerModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	FViveOpenXRWristTracker* GetWristTracker();

private:
	TSharedPtr<FViveOpenXRWristTracker> WristTracker;
};
