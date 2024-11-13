// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ViveOpenXRWristTrackerEnums.h"

#include "ViveOpenXRAndroidWristTrackerBP.generated.h"

UCLASS()
class VIVEOPENXRWRISTTRACKER_API UViveOpenXRAndroidWristTrackerBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Rotation (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get Wrist Tracker Location And Rotation'",
			ToolTip = "Retrieves the left or right tracker rotation."))
	static FQuat GetTrackerRotation(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Position (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get Wrist Tracker Location And Rotation'",
			ToolTip = "Retrieves the left or right tracker position."))
	static FVector GetTrackerPosition(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Active (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get Wrist Tracker Location And Rotation'",
			ToolTip = "Checks if the tracker is active."))
	static bool GetTrackerActive(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Valid (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get Wrist Tracker Location And Rotation'",
			ToolTip = "Checks if the tracker is valid."))
	static bool GetTrackerValid(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Tracked (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get Wrist Tracker Location And Rotation'",
			ToolTip = "Checks if the tracker is tracked."))
	static bool GetTrackerTracked(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|WristTracker",
		meta = (
			DisplayName = "Get Tracker Key Down (Deprecated)",
			DeprecatedFunction, DeprecationMessage = "Please use the new function 'Is Wrsit Tracker Key Down'",
			ToolTip = "Checks if a tracker button is pressed."))
	static bool GetTrackerKeyDown(bool isLeft, TrackerButton key);
};
