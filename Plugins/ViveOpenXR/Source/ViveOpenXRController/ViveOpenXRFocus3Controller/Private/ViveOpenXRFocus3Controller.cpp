// Copyright HTC Corporation All Rights Reserved.

#include "ViveOpenXRFocus3Controller.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "FViveOpenXRFocus3ControllerModule"

DEFINE_LOG_CATEGORY(LogViveOpenXRFocus3Controller);

namespace Focus3Keys
{
	// Regular Controller Keys
	const FKey Focus3_Left_X_Click("Focus3_Left_X_Click");
	const FKey Focus3_Left_Y_Click("Focus3_Left_Y_Click");
	const FKey Focus3_Left_Menu_Click("Focus3_Left_Menu_Click");
	const FKey Focus3_Left_Grip_Click("Focus3_Left_Grip_Click");
	const FKey Focus3_Left_Grip_Touch("Focus3_Left_Grip_Touch");
	const FKey Focus3_Left_Grip_Axis("Focus3_Left_Grip_Axis");
	const FKey Focus3_Left_Trigger_Click("Focus3_Left_Trigger_Click");
	const FKey Focus3_Left_Trigger_Touch("Focus3_Left_Trigger_Touch");
	const FKey Focus3_Left_Trigger_Axis("Focus3_Left_Trigger_Axis");
	const FKey Focus3_Left_Thumbstick_X("Focus3_Left_Thumbstick_X");
	const FKey Focus3_Left_Thumbstick_Y("Focus3_Left_Thumbstick_Y");
	const FKey Focus3_Left_Thumbstick_2D("Focus3_Left_Thumbstick_2D");
	const FKey Focus3_Left_Thumbstick_Click("Focus3_Left_Thumbstick_Click");
	const FKey Focus3_Left_Thumbstick_Touch("Focus3_Left_Thumbstick_Touch");
	const FKey Focus3_Left_Thumbrest_Touch("Focus3_Left_Thumbrest_Touch");

	const FKey Focus3_Right_A_Click("Focus3_Right_A_Click");
	const FKey Focus3_Right_B_Click("Focus3_Right_B_Click");
	const FKey Focus3_Right_Grip_Click("Focus3_Right_Grip_Click");
	const FKey Focus3_Right_Grip_Touch("Focus3_Right_Grip_Touch");
	const FKey Focus3_Right_Grip_Axis("Focus3_Right_Grip_Axis");
	const FKey Focus3_Right_Trigger_Click("Focus3_Right_Trigger_Click");
	const FKey Focus3_Right_Trigger_Touch("Focus3_Right_Trigger_Touch");
	const FKey Focus3_Right_Trigger_Axis("Focus3_Right_Trigger_Axis");
	const FKey Focus3_Right_Thumbstick_X("Focus3_Right_Thumbstick_X");
	const FKey Focus3_Right_Thumbstick_Y("Focus3_Right_Thumbstick_Y");
	const FKey Focus3_Right_Thumbstick_2D("Focus3_Right_Thumbstick_2D");
	const FKey Focus3_Right_Thumbstick_Click("Focus3_Right_Thumbstick_Click");
	const FKey Focus3_Right_Thumbstick_Touch("Focus3_Right_Thumbstick_Touch");
	const FKey Focus3_Right_Thumbrest_Touch("Focus3_Right_Thumbrest_Touch");
}

void FViveOpenXRFocus3ControllerModule::StartupModule()
{
#pragma region Focus3 KEYS
	EKeys::AddMenuCategoryDisplayInfo("Focus3", LOCTEXT("Focus3SubCategory", "HTC Focus3"), TEXT("GraphEditor.PadEvent_16x"));

	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_X_Click, LOCTEXT("Focus3_Left_X_Click", "Focus3 (L) X Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Y_Click, LOCTEXT("Focus3_Left_Y_Click", "Focus3 (L) Y Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Menu_Click, LOCTEXT("Focus3_Left_Menu_Click", "Focus3 (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Grip_Click, LOCTEXT("Focus3_Left_Grip_Click", "Focus3 (L) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Grip_Touch, LOCTEXT("Focus3_Left_Grip_Touch", "Focus3 (L) Grip Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Grip_Axis, LOCTEXT("Focus3_Left_Grip_Axis", "Focus3 (L) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Trigger_Click, LOCTEXT("Focus3_Left_Trigger_Click", "Focus3 (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Trigger_Touch, LOCTEXT("Focus3_Left_Trigger_Touch", "Focus3 (L) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Trigger_Axis, LOCTEXT("Focus3_Left_Trigger_Axis", "Focus3 (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbstick_X, LOCTEXT("Focus3_Left_Thumbstick_X", "Focus3 (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbstick_Y, LOCTEXT("Focus3_Left_Thumbstick_Y", "Focus3 (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddPairedKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbstick_2D, LOCTEXT("Focus3_Left_Thumbstick_2D", "Focus3 (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "Focus3"), Focus3Keys::Focus3_Left_Thumbstick_X, Focus3Keys::Focus3_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbstick_Click, LOCTEXT("Focus3_Left_Thumbstick_Click", "Focus3 (L) Thumbstick"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbstick_Touch, LOCTEXT("Focus3_Left_Thumbstick_Touch", "Focus3 (L) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Left_Thumbrest_Touch, LOCTEXT("Focus3_Left_Thumbrest_Touch", "Focus3 (L) Thumbrest Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));

	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_A_Click, LOCTEXT("Focus3_Right_A_Click", "Focus3 (R) A Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_B_Click, LOCTEXT("Focus3_Right_B_Click", "Focus3 (R) B Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Grip_Click, LOCTEXT("Focus3_Right_Grip_Click", "Focus3 (R) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Grip_Touch, LOCTEXT("Focus3_Right_Grip_Touch", "Focus3 (R) Grip Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Grip_Axis, LOCTEXT("Focus3_Right_Grip_Axis", "Focus3 (R) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Trigger_Click, LOCTEXT("Focus3_Right_Trigger_Click", "Focus3 (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Trigger_Touch, LOCTEXT("Focus3_Right_Trigger_Touch", "Focus3 (R) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Trigger_Axis, LOCTEXT("Focus3_Right_Trigger_Axis", "Focus3 (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbstick_X, LOCTEXT("Focus3_Right_Thumbstick_X", "Focus3 (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbstick_Y, LOCTEXT("Focus3_Right_Thumbstick_Y", "Focus3 (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddPairedKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbstick_2D, LOCTEXT("Focus3_Right_Thumbstick_2D", "Focus3 (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "Focus3"), Focus3Keys::Focus3_Right_Thumbstick_X, Focus3Keys::Focus3_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbstick_Click, LOCTEXT("Focus3_Right_Thumbstick_Click", "Focus3 (R) Thumbstick"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbstick_Touch, LOCTEXT("Focus3_Right_Thumbstick_Touch", "Focus3 (R) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
	EKeys::AddKey(FKeyDetails(Focus3Keys::Focus3_Right_Thumbrest_Touch, LOCTEXT("Focus3_Right_Thumbrest_Touch", "Focus3 (R) Thumbrest Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Focus3"));
#pragma endregion

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableFocus3Controller"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableFocus3Controller = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableFocus3Controller = true;
		}
	}

	if (m_bEnableFocus3Controller)
	{
		UE_LOG(LogViveOpenXRFocus3Controller, Log, TEXT("Enable Focus3 controller input."));
	}
	else
	{
		UE_LOG(LogViveOpenXRFocus3Controller, Log, TEXT("Disable Focus3 controller input."));
		return;
	}

	RegisterOpenXRExtensionModularFeature();
}

void FViveOpenXRFocus3ControllerModule::ShutdownModule()
{
	UnregisterOpenXRExtensionModularFeature();
}


bool FViveOpenXRFocus3ControllerModule::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableFocus3Controller)
	{
		OutExtensions.Add("XR_HTC_vive_focus3_controller_interaction");
	}
	return true;
}

bool FViveOpenXRFocus3ControllerModule::GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
{
	if (!m_bEnableFocus3Controller) return false;

	OutKeyPrefix = "Focus3";
	OutHasHaptics = true;
	return xrStringToPath(InInstance, "/interaction_profiles/htc/vive_focus3_controller", &OutPath) == XR_SUCCESS;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FViveOpenXRFocus3ControllerModule, ViveOpenXRFocus3Controller)
	