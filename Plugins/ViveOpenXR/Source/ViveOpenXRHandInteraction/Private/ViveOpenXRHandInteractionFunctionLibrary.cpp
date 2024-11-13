// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRHandInteractionFunctionLibrary.h"
#include "ViveOpenXRHandInteraction.h"

FViveOpenXRHandInteraction* GetHandInteraction()
{
	TArray<FViveOpenXRHandInteractionModule*> InputDeviceModules = IModularFeatures::Get().GetModularFeatureImplementations<FViveOpenXRHandInteractionModule>(IInputDeviceModule::GetModularFeatureName());
	for (auto InputDeviceModule : InputDeviceModules)
	{
		if (InputDeviceModule->GetModuleKeyName().IsEqual(IViveOpenXRHandInteractionModule::ViveOpenXRHandInteractionModularKeyName))
			return InputDeviceModule->GetHandInteraction();
	}

	return nullptr;
}

void UViveOpenXRHandInteractionFunctionLibrary::GetHandInteractionAimPose(bool isRight, bool& valid, FVector& location, FRotator& rotation)
{
	valid = false;
	location = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator; 

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::LeftAim;
	if (isRight) motionSource = HandInteractionMotionSource::RightAim;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		valid = true;
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, location, HandInteraction->GetWorldToMeterScale());
	}
}

void UViveOpenXRHandInteractionFunctionLibrary::GetHandInteractionGripPose(bool isRight, bool& valid, FVector& location, FRotator& rotation)
{
	valid = false;
	location = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::LeftGrip;
	if (isRight) motionSource = HandInteractionMotionSource::RightGrip;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		valid = true;
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, location, HandInteraction->GetWorldToMeterScale());
	}
}

void UViveOpenXRHandInteractionFunctionLibrary::GetHandInteractionSelectValue(bool isRight, bool& valid, float& value)
{
	valid = false;
	value = -1;
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	valid = isRight ? GetHandInteraction()->RightInteractionController.AimValueActionState.isActive > 0: GetHandInteraction()->LeftInteractionController.AimValueActionState.isActive > 0;
	value = isRight ? GetHandInteraction()->RightInteractionController.AimValueActionState.currentState : GetHandInteraction()->LeftInteractionController.AimValueActionState.currentState;
	return;
}

void UViveOpenXRHandInteractionFunctionLibrary::GetHandInteractionSqueezeValue(bool isRight, bool& valid, float& value)
{
	valid = false;
	value = -1;
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	valid = isRight ? GetHandInteraction()->RightInteractionController.GraspValueActionState.isActive > 0 : GetHandInteraction()->LeftInteractionController.GraspValueActionState.isActive > 0;
	value = isRight ? GetHandInteraction()->RightInteractionController.GraspValueActionState.currentState : GetHandInteraction()->LeftInteractionController.GraspValueActionState.currentState;
	return;
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionRightSqueezeValue(float& result)
{
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	result = GetHandInteraction()->RightInteractionController.GraspValueActionState.currentState;
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionLeftSqueezeValue(float& result)
{
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	result = GetHandInteraction()->LeftInteractionController.GraspValueActionState.currentState;
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionRightSelectValue(float& result)
{
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	result = GetHandInteraction()->RightInteractionController.AimValueActionState.currentState;
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionLeftSelectValue(float& result)
{
	auto HandInteraction = GetHandInteraction();
	if (!GetHandInteraction()->m_EnableHandInteraction) return;
	result = GetHandInteraction()->LeftInteractionController.AimValueActionState.currentState;
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionRightAimPose(FVector& position, FRotator& rotation)
{
	position = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::RightAim;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, position, HandInteraction->GetWorldToMeterScale());
	}
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionLeftAimPose(FVector& position, FRotator& rotation)
{
	position = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::LeftAim;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, position, HandInteraction->GetWorldToMeterScale());
	}
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionRightGripPose(FVector& position, FRotator& rotation)
{
	position = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::RightGrip;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, position, HandInteraction->GetWorldToMeterScale());
	}
}

void UViveOpenXRHandInteractionFunctionLibrary::HandInteractionLefttGripPose(FVector& position, FRotator& rotation)
{
	position = FVector::ZeroVector;
	rotation = FRotator::ZeroRotator;

	auto HandInteraction = GetHandInteraction();
	if (HandInteraction == nullptr) return;

	FName motionSource = HandInteractionMotionSource::LeftGrip;
	if (HandInteraction->GetControllerTrackingStatus(0, motionSource) == ETrackingStatus::Tracked)
	{
		HandInteraction->GetControllerOrientationAndPosition(0, motionSource, rotation, position, HandInteraction->GetWorldToMeterScale());
	}
}

