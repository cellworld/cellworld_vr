// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRAndroidHandInteractionBP.h"
#include "ViveOpenXRHandInteraction.h"
#include "ViveOpenXRHandInteractionFunctionLibrary.h"

FViveOpenXRHandInteraction* GetHandInteraction_ForDeprecatedUsage()
{
	TArray<FViveOpenXRHandInteractionModule*> InputDeviceModules = IModularFeatures::Get().GetModularFeatureImplementations<FViveOpenXRHandInteractionModule>(IInputDeviceModule::GetModularFeatureName());
	for (auto InputDeviceModule : InputDeviceModules)
	{
		if (InputDeviceModule->GetModuleKeyName().IsEqual(IViveOpenXRHandInteractionModule::ViveOpenXRHandInteractionModularKeyName))
			return InputDeviceModule->GetHandInteraction();
	}

	return nullptr;
}

float UViveOpenXRAndroidHandInteractionBP::GetPinchStrength(bool isLeft)
{
	return 0;
}

FQuat UViveOpenXRAndroidHandInteractionBP::GetPinchRotation(bool isLeft)
{
	auto HandInteraction = GetHandInteraction_ForDeprecatedUsage();
	if (HandInteraction != nullptr)
		return HandInteraction->GetAimRotation(isLeft);

	return FQuat::Identity;
}

FVector UViveOpenXRAndroidHandInteractionBP::GetPinchPosition(bool isLeft)
{
	auto HandInteraction = GetHandInteraction_ForDeprecatedUsage();
	if (HandInteraction != nullptr)
		return HandInteraction->GetAimPosition(isLeft);

	return FVector::ZeroVector;
}

bool UViveOpenXRAndroidHandInteractionBP::GetPinchActive(bool isLeft)
{
	return false;
}

bool UViveOpenXRAndroidHandInteractionBP::GetPinchValid(bool isLeft)
{
	return false;
}

bool UViveOpenXRAndroidHandInteractionBP::GetPinchTracked(bool isLeft)
{
	return false;
}
