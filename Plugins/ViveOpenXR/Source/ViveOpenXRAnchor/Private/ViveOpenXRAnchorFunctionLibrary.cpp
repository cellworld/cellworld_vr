// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRAnchorFunctionLibrary.h"
#include "ViveOpenXRAnchorModule.h"

#include <string>

bool UViveOpenXRAnchorFunctionLibrary::IsAnchorSupported()
{
	if (!FViveOpenXRAnchor::Instance()) return false;
	return FViveOpenXRAnchor::Instance()->IsSupported();
}

bool UViveOpenXRAnchorFunctionLibrary::CreateSpatialAnchor(FVector position, FRotator rotation, FString name, int64& anchor)
{
    auto mod = FViveOpenXRAnchor::Instance();
    auto hmd = FViveOpenXRAnchor::HMD();
    if (!mod || !hmd) return false;

    float w2m = hmd->GetWorldToMetersScale();
    FQuat rot = FQuat(rotation);
    XrSpatialAnchorCreateInfoHTC createInfo = mod->MakeCreateInfo(position, rot, hmd->GetTrackingSpace(), name, w2m);
    anchor = 0;
    XrSpace space;
    if (!mod->CreateSpatialAnchor(&createInfo, &space))
        return false;
    anchor = (int64)space;
    return true;
}

bool UViveOpenXRAnchorFunctionLibrary::CreateSpatialAnchorT(FTransform transform, FString name, int64& anchor)
{
    return CreateSpatialAnchor(transform.GetLocation(), transform.GetRotation().Rotator(), name, anchor);
}

void UViveOpenXRAnchorFunctionLibrary::DestroySpatialAnchor(int64 anchor)
{
    auto mod = FViveOpenXRAnchor::Instance();
    if (!mod) return;
    xrDestroySpace((XrSpace)anchor);
}

bool UViveOpenXRAnchorFunctionLibrary::LocateAnchor(int64 anchor, FTransform& transform)
{
    auto mod = FViveOpenXRAnchor::Instance();
    if (!mod) return false;

    FVector translation;
    FRotator rotation;
    if (!mod->LocateAnchor((XrSpace)anchor, rotation, translation))
		return false;
    transform = FTransform(rotation, translation, FVector(1, 1, 1));
    return true;
}

bool UViveOpenXRAnchorFunctionLibrary::GetSpatialAnchorName(int64 anchor, FString& name)
{
    auto mod = FViveOpenXRAnchor::Instance();
    if (!mod) return false;
    XrSpatialAnchorNameHTC xrName{};
    xrName.name[0] = 0;
    if (!mod->GetSpatialAnchorName((XrSpace)anchor, &xrName)) return false;
    // Make sure the name is null-terminated
    xrName.name[XR_MAX_SPATIAL_ANCHOR_NAME_SIZE_HTC - 1] = 0;
    name = FString(xrName.name);
    return true;
}