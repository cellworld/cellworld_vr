// Copyright HTC Corporation All Rights Reserved.

#include "ViveOpenXRCosmosController.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "FViveOpenXRControllerModule"

DEFINE_LOG_CATEGORY(LogViveOpenXRCosmosController);

namespace CosmosKeys
{
	// Regular Controller Keys
	const FKey Cosmos_Left_X_Click("Cosmos_Left_X_Click");
	const FKey Cosmos_Left_Y_Click("Cosmos_Left_Y_Click");
	const FKey Cosmos_Left_Menu_Click("Cosmos_Left_Menu_Click");
	const FKey Cosmos_Left_Grip_Click("Cosmos_Left_Grip_Click");
	const FKey Cosmos_Left_Trigger_Click("Cosmos_Left_Trigger_Click");
	const FKey Cosmos_Left_Trigger_Axis("Cosmos_Left_Trigger_Axis");
	const FKey Cosmos_Left_Thumbstick_X("Cosmos_Left_Thumbstick_X");
	const FKey Cosmos_Left_Thumbstick_Y("Cosmos_Left_Thumbstick_Y");
	const FKey Cosmos_Left_Thumbstick_2D("Cosmos_Left_Thumbstick_2D");
	const FKey Cosmos_Left_Thumbstick_Click("Cosmos_Left_Thumbstick_Click");
	const FKey Cosmos_Left_Thumbstick_Touch("Cosmos_Left_Thumbstick_Touch");
	//const FKey Cosmos_Left_Bumper_Click("Cosmos_Left_Bumper_Click");

	const FKey Cosmos_Right_A_Click("Cosmos_Right_A_Click");
	const FKey Cosmos_Right_B_Click("Cosmos_Right_B_Click");
	const FKey Cosmos_Right_Grip_Click("Cosmos_Right_Grip_Click");
	const FKey Cosmos_Right_Trigger_Click("Cosmos_Right_Trigger_Click");
	const FKey Cosmos_Right_Trigger_Axis("Cosmos_Right_Trigger_Axis");
	const FKey Cosmos_Right_Thumbstick_X("Cosmos_Right_Thumbstick_X");
	const FKey Cosmos_Right_Thumbstick_Y("Cosmos_Right_Thumbstick_Y");
	const FKey Cosmos_Right_Thumbstick_2D("Cosmos_Right_Thumbstick_2D");
	const FKey Cosmos_Right_Thumbstick_Click("Cosmos_Right_Thumbstick_Click");
	const FKey Cosmos_Right_Thumbstick_Touch("Cosmos_Right_Thumbstick_Touch");
	//const FKey Cosmos_Right_Bumper_Click("Cosmos_Right_Bumper_Click");
}

void FViveOpenXRCosmosControllerModule::StartupModule()
{
#pragma region COSMOS KEYS
	EKeys::AddMenuCategoryDisplayInfo("Cosmos", LOCTEXT("CosmosSubCategory", "HTC Cosmos"), TEXT("GraphEditor.PadEvent_16x"));

	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_X_Click, LOCTEXT("Cosmos_Left_X_Click", "Cosmos (L) X Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Y_Click, LOCTEXT("Cosmos_Left_Y_Click", "Cosmos (L) Y Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Menu_Click, LOCTEXT("Cosmos_Left_Menu_Click", "Cosmos (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Grip_Click, LOCTEXT("Cosmos_Left_Grip_Click", "Cosmos (L) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Trigger_Click, LOCTEXT("Cosmos_Left_Trigger_Click", "Cosmos (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Trigger_Axis, LOCTEXT("Cosmos_Left_Trigger_Axis", "Cosmos (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Thumbstick_X, LOCTEXT("Cosmos_Left_Thumbstick_X", "Cosmos (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Thumbstick_Y, LOCTEXT("Cosmos_Left_Thumbstick_Y", "Cosmos (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddPairedKey(FKeyDetails(CosmosKeys::Cosmos_Left_Thumbstick_2D, LOCTEXT("Cosmos_Left_Thumbstick_2D", "Cosmos (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"), CosmosKeys::Cosmos_Left_Thumbstick_X, CosmosKeys::Cosmos_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Thumbstick_Click, LOCTEXT("Cosmos_Left_Thumbstick_Click", "Cosmos (L) Thumbstick"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	//EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Bumper_Click, LOCTEXT("Cosmos_Left_Bumper_Click", "Cosmos (L) Bumper"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Left_Thumbstick_Touch, LOCTEXT("Cosmos_Left_Thumbstick_Touch", "Cosmos (L) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));

	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_A_Click, LOCTEXT("Cosmos_Right_A_Click", "Cosmos (R) A Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_B_Click, LOCTEXT("Cosmos_Right_B_Click", "Cosmos (R) B Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Grip_Click, LOCTEXT("Cosmos_Right_Grip_Click", "Cosmos (R) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Trigger_Click, LOCTEXT("Cosmos_Right_Trigger_Click", "Cosmos (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Trigger_Axis, LOCTEXT("Cosmos_Right_Trigger_Axis", "Cosmos (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Thumbstick_X, LOCTEXT("Cosmos_Right_Thumbstick_X", "Cosmos (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Thumbstick_Y, LOCTEXT("Cosmos_Right_Thumbstick_Y", "Cosmos (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddPairedKey(FKeyDetails(CosmosKeys::Cosmos_Right_Thumbstick_2D, LOCTEXT("Cosmos_Right_Thumbstick_2D", "Cosmos (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "Cosmos"), CosmosKeys::Cosmos_Right_Thumbstick_X, CosmosKeys::Cosmos_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Thumbstick_Click, LOCTEXT("Cosmos_Right_Thumbstick_Click", "Cosmos (R) Thumbstick"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Thumbstick_Touch, LOCTEXT("Cosmos_Right_Thumbstick_Touch", "Cosmos (R) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
	//EKeys::AddKey(FKeyDetails(CosmosKeys::Cosmos_Right_Bumper_Click, LOCTEXT("Cosmos_Right_Bumper_Click", "Cosmos (R) Bumper"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "Cosmos"));
#pragma endregion

#if PLATFORM_ANDROID
	m_bEnableCosmosController = false;
	return;
#endif

	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableCosmosController"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnableCosmosController = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnableCosmosController = true;
		}
	}

	if (m_bEnableCosmosController)
	{
		UE_LOG(LogViveOpenXRCosmosController, Log, TEXT("Enable Cosmos controller input."));
	}
	else
	{
		UE_LOG(LogViveOpenXRCosmosController, Log, TEXT("Disable Cosmos controller input."));
		return;
	}

	RegisterOpenXRExtensionModularFeature();
}

void FViveOpenXRCosmosControllerModule::ShutdownModule()
{
	UnregisterOpenXRExtensionModularFeature();
}


bool FViveOpenXRCosmosControllerModule::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnableCosmosController)
	{
		OutExtensions.Add("XR_HTC_vive_cosmos_controller_interaction");
	}
	return true;
}

bool FViveOpenXRCosmosControllerModule::GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
{
	if (!m_bEnableCosmosController) return false;

	OutKeyPrefix = "Cosmos";
	OutHasHaptics = true;
	return xrStringToPath(InInstance, "/interaction_profiles/htc/vive_cosmos_controller", &OutPath) == XR_SUCCESS;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FViveOpenXRCosmosControllerModule, ViveOpenXRCosmosController)
	