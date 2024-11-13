// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRAndroidHandInteractionBP.generated.h"

UCLASS()
class VIVEOPENXRHANDINTERACTION_API UViveOpenXRAndroidHandInteractionBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Strength (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Select Value'", ToolTip = "Retrieves the pinch strength of left or right hand."))
	static float GetPinchStrength(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Rotation (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", ToolTip = "Retrieves the pinch rotation of left or right hand."))
	static FQuat GetPinchRotation(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Position (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", ToolTip = "Retrieves the pinch position of left or right hand."))
	static FVector GetPinchPosition(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Active (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", ToolTip = "Checks if the pinch motion is active."))
	static bool GetPinchActive(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Valid (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", ToolTip = "Checks if the pinch motion is valid."))
	static bool GetPinchValid(bool isLeft);

	UFUNCTION(
		BlueprintCallable,
		Category = "ViveOpenXRAndroid|OpenXR|HandInteraction",
		meta = (DisplayName = "Get Pinch Tracked (Deprecated)", DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", ToolTip = "Checks if the pinch motion is tracked."))
	static bool GetPinchTracked(bool isLeft);
};
