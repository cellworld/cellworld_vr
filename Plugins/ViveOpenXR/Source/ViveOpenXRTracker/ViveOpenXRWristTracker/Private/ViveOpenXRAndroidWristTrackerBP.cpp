// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRAndroidWristTrackerBP.h"
#include "ViveOpenXRWristTracker.h"
#include "ViveOpenXRWristTrackerFunctionLibrary.h"

FViveOpenXRWristTracker* GetWristTracker_ForDeprecatedUsage()
{
	TArray<FViveOpenXRWristTrackerModule*> InputDeviceModules = IModularFeatures::Get().GetModularFeatureImplementations<FViveOpenXRWristTrackerModule>(IInputDeviceModule::GetModularFeatureName());
	for (auto InputDeviceModule : InputDeviceModules)
	{
		if (InputDeviceModule->GetModuleKeyName().IsEqual(IViveOpenXRWristTrackerModule::ViveOpenXRWristTrackerModularKeyName))
			return InputDeviceModule->GetWristTracker();
	}

	return nullptr;
}

FQuat UViveOpenXRAndroidWristTrackerBP::GetTrackerRotation(bool isLeft)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerRotation(isLeft);

	return FQuat::Identity;
}

FVector UViveOpenXRAndroidWristTrackerBP::GetTrackerPosition(bool isLeft)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerPosition(isLeft);

	return FVector::ZeroVector;
}

bool UViveOpenXRAndroidWristTrackerBP::GetTrackerActive(bool isLeft)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerActive(isLeft);

	return false;
}

bool UViveOpenXRAndroidWristTrackerBP::GetTrackerValid(bool isLeft)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerValid(isLeft);

	return false;
}

bool UViveOpenXRAndroidWristTrackerBP::GetTrackerTracked(bool isLeft)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerTracked(isLeft);

	return false;
}

bool UViveOpenXRAndroidWristTrackerBP::GetTrackerKeyDown(bool isLeft, TrackerButton key)
{
	auto WristTracker = GetWristTracker_ForDeprecatedUsage();
	if (WristTracker != nullptr)
		return WristTracker->GetTrackerKeyDown(isLeft, key);

	return false;
}
