// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveFacialExpressionEnums.h"
#include "ViveOpenXRFacialTrackingFunctionLibrary.generated.h"

UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRFACIALTRACKING_API UViveOpenXRFacialTrackingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Facial Tracking Enabled", Keywords = "ViveOpenXR Facial Tracking"), Category = "ViveOpenXR|FacialTracking")
    static void GetIsFacialTrackingEnabled(bool& result);
    /*
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create Facial Tracker", Keywords = "ViveOpenXR Facial Tracking"), Category = "ViveOpenXR|FacialTracking")
    static bool CreateFacialTracker(EXrFacialTrackingType trackingType);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy Facial Tracker", Keywords = "ViveOpenXR Facial Tracking"), Category = "ViveOpenXR|FacialTracking")
    static bool DestroyFacialTracker(EXrFacialTrackingType trackingType);
    */
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Eye Facial Expressions", Keywords = "ViveOpenXR Facial Tracking"), Category = "ViveOpenXR|FacialTracking")
    static bool GetEyeFacialExpressions(bool& isActive, TMap<EEyeShape, float>& blendshapes);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Lip Facial Expressions", Keywords = "ViveOpenXR Facial Tracking"), Category = "ViveOpenXR|FacialTracking")
    static bool GetLipFacialExpressions(bool& isActive, TMap<ELipShape, float>& blendshapes);
};
