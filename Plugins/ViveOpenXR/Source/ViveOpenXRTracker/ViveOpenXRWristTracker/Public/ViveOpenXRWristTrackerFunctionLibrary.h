// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ViveOpenXRWristTrackerEnums.h"

#include "ViveOpenXRWristTrackerFunctionLibrary.generated.h"

UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRWRISTTRACKER_API UViveOpenXRWristTrackerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintPure,
		Category = "ViveOpenXR|WristTracker")
	static void GetWristTrackerLocationAndRotation(bool isRight, FVector& location, FRotator& rotation, bool& valid);

	UFUNCTION(
		BlueprintPure,
		Category = "ViveOpenXR|WristTracker")
	static bool IsWrsitTrackerKeyDown(bool isRight, TrackerButton key);
};

