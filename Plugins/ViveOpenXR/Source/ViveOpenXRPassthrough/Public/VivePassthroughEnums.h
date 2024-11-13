// Copyright HTC Corporation. All Rights Reserved.

#pragma once
#include <stdint.h>
#include "VivePassthroughEnums.generated.h"

UENUM(BlueprintType, Category = "ViveOpenXR|Passthrough")
enum class EXrPassthroughLayerForm : uint8 {
	Planar = 0,
	Projected = 1
};

UENUM(BlueprintType, Category = "ViveOpenXR|Passthrough")
enum class EProjectedPassthroughSpaceType : uint8 {
	//XR_REFERENCE_SPACE_TYPE_VIEW at (0,0,0) with orientation (0,0,0,1) 
	Headlock = 0, 
	// When TrackingOrigin is EHMDTrackingOrigin::Stage:
	// XR_REFERENCE_SPACE_TYPE_STAGE at (0,0,0) with orientation (0,0,0,1) 
	// 
	// When TrackingOriginMode is EHMDTrackingOrigin::Eye:
	// XR_REFERENCE_SPACE_TYPE_LOCAL at (0,0,0) with orientation (0,0,0,1) 
	Worldlock = 1
};