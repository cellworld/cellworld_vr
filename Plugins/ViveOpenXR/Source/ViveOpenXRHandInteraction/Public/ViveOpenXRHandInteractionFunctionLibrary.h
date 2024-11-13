// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRHandInteractionFunctionLibrary.generated.h"

UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRHANDINTERACTION_API UViveOpenXRHandInteractionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// Pose
	UFUNCTION(
		BlueprintPure,
		Category = "ViveOpenXR|HandInteraction")
	static void GetHandInteractionAimPose(bool isRight, bool& valid, FVector& location, FRotator& rotation);
	
	UFUNCTION(
		BlueprintPure,
		Category = "ViveOpenXR|HandInteraction")
	static void GetHandInteractionGripPose(bool isRight, bool& valid, FVector& location, FRotator& rotation);
	
	// Deprecated 
	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new Enhanced Input Action to get 'Select Value'", DisplayName = "Get HandInteraction Select Value", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteraction")
	static void GetHandInteractionSelectValue(bool isRight, bool& valid, float& value);

	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new  Enhanced Input Action to get 'Squeeze Value'", DisplayName = "Get HandInteraction Squeeze Value", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteraction")
	static void GetHandInteractionSqueezeValue(bool isRight, bool& valid, float& value);
	
	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Squeeze Value'", DisplayName = "HandInteraction Right SqueezeValue (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionRightSqueezeValue(float& result);

	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Squeeze Value'", DisplayName = "HandInteraction Left SqueezeValue (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionLeftSqueezeValue(float& result);

	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Select Value'", DisplayName = "HandInteraction Right SelectValue (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionRightSelectValue(float& result);

	UFUNCTION(BlueprintPure, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Select Value'", DisplayName = "HandInteraction Left SelectValue (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionLeftSelectValue(float& result);
	
	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", DisplayName = "HandInteraction Right AimPose (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionRightAimPose(FVector& position, FRotator& rotation);

	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Aim Pose'", DisplayName = "HandInteraction Left AimPose (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionLeftAimPose(FVector& position, FRotator& rotation);

	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Grip Pose'", DisplayName = "HandInteraction Right GripPose (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionRightGripPose(FVector& position, FRotator& rotation);

	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Please use the new function 'Get HandInteraction Grip Pose'", DisplayName = "HandInteraction Left GripPose (Deprecated)", Keywords = "ViveOpenXR Hand Interaction"), Category = "ViveOpenXR|HandInteractionTesting")
	static void HandInteractionLefttGripPose(FVector& position, FRotator& rotation);
	
};
