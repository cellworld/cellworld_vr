// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRWristTrackerFunctionLibrary.h"
#include "ViveOpenXRWristTracker.h"

FViveOpenXRWristTracker* GetWristTracker()
{
	TArray<FViveOpenXRWristTrackerModule*> InputDeviceModules = IModularFeatures::Get().GetModularFeatureImplementations<FViveOpenXRWristTrackerModule>(IInputDeviceModule::GetModularFeatureName());
	for (auto InputDeviceModule : InputDeviceModules)
	{
		if (InputDeviceModule->GetModuleKeyName().IsEqual(IViveOpenXRWristTrackerModule::ViveOpenXRWristTrackerModularKeyName))
			return InputDeviceModule->GetWristTracker();
	}

	return nullptr;
}

void UViveOpenXRWristTrackerFunctionLibrary::GetWristTrackerLocationAndRotation(bool isRight, FVector& location, FRotator& rotation, bool& valid)
{
	location = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;
	valid = false;

	auto WristTracker = GetWristTracker();
	if (WristTracker == nullptr) return;

	FName motionSource = WristTrackerMotionSource::Left;
	if (isRight) motionSource = WristTrackerMotionSource::Right;
	if(WristTracker->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		valid = true;
		WristTracker->GetControllerOrientationAndPosition(0, motionSource, rotation, location, WristTracker->GetWorldToMeterScale());
	}
}

bool UViveOpenXRWristTrackerFunctionLibrary::IsWrsitTrackerKeyDown(bool isRight, TrackerButton key)
{
	auto WristTracker = GetWristTracker();
	return WristTracker->GetTrackerKeyDown(!isRight, key);;
}
