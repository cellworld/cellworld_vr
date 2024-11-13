// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXREyeTrackerFunctionLibrary.generated.h"

/**
 * Blueprint function library for ViveOpenXR Eye Tracker extension.
 */

UCLASS(ClassGroup = OpenXR)
class VIVEOPENXREYETRACKER_API UViveOpenXREyeTrackerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get HTC Eye Gaze Valid Data", Keywords = "ViveOpenXR EyeTracker"), Category = "ViveOpenXR|EyeTracker")
	static FXrGazeDataHTC GetHTCEyeGazeValidData();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get HTC Pupil Data", Keywords = "ViveOpenXR EyeTracker"), Category = "ViveOpenXR|EyeTracker")
	static FXrPupilDataHTC GetHTCPupilData();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get HTC Eye Geometric Data", Keywords = "ViveOpenXR EyeTracker"), Category = "ViveOpenXR|EyeTracker")
	static FXrEyeGeometricDataHTC GetHTCEyeGeometricData();
};
