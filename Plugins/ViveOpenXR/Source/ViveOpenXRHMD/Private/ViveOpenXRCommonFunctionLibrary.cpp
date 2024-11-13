// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRCommonFunctionLibrary.h"
#include "OpenXRHMD.h"
#include "OpenXRCore.h"

DEFINE_LOG_CATEGORY(ViveOXRCommon);

bool UViveOpenXRCommonFunctionLibrary::LocateXrSpace(int64 space, FTransform& transform)
{
    FOpenXRHMD* hmd = nullptr;
    if (GEngine->XRSystem.IsValid())
        hmd = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());

    if (!hmd) {
        UE_LOG(ViveOXRCommon, Error, TEXT("LocateXrSpace() No OpenXRHMD"));
        return false;
    }
    XrTime time = hmd->GetDisplayTime();
    XrSpace baseSpace = hmd->GetTrackingSpace();
    XrSpaceLocation loc{};

    auto result = xrLocateSpace((XrSpace)space, baseSpace, time, &loc);
    if (XR_FAILED(result)) {
        UE_LOG(ViveOXRCommon, Error, TEXT("LocateXrSpace() xrLocateSpace failed.  result=%d."), result);
        return false;
    }

    if ((loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) == 0 ||
        (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) == 0) {
        UE_LOG(ViveOXRCommon, Error, TEXT("LocateXrSpace() locationFlags has not valid bits. Flags=%016llX"), loc.locationFlags);
        return false;
    }

    auto& p = loc.pose.position;
    auto& o = loc.pose.orientation;
    auto w2m = hmd->GetWorldToMetersScale();
    FVector translation = FVector(-p.z * w2m, p.x * w2m, p.y * w2m);
    FQuat rotation(-o.z, o.x, o.y, -o.w);
    rotation.Normalize();
    transform = FTransform(rotation, translation, FVector(1, 1, 1));
    return true;
}

bool UViveOpenXRCommonFunctionLibrary::DestroyXrSpace(int64 space)
{
    FOpenXRHMD* hmd = nullptr;
    if (GEngine->XRSystem.IsValid())
        hmd = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());

    if (!hmd) {
        UE_LOG(ViveOXRCommon, Error, TEXT("DestroyXrSpace() No OpenXRHMD"));
        return false;
    }
    return XR_SUCCEEDED(xrDestroySpace((XrSpace)space));
}


