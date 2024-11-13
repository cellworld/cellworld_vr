// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRFacialTrackingFunctionLibrary.h"
#include "ViveOpenXRFacialTracking.h"

void UViveOpenXRFacialTrackingFunctionLibrary::GetIsFacialTrackingEnabled(bool& result)
{
	if (!GetViveOpenXRFacialTrackingModule().FTSource.IsValid()) return;
	result = GetViveOpenXRFacialTrackingModule().FTSource->m_bEnableFacialTracking;
}
/*
bool UViveOpenXRFacialTrackingFunctionLibrary::CreateFacialTracker(EXrFacialTrackingType trackingType)
{
	XrFacialTrackingTypeHTC trackType = static_cast<XrFacialTrackingTypeHTC>(trackingType);
	if (!GetViveOpenXRFacialTrackingModule().FTSource.IsValid()) return false;
	return GetViveOpenXRFacialTrackingModule().FTSource->CreateFacialTracker(trackType);
}

bool UViveOpenXRFacialTrackingFunctionLibrary::DestroyFacialTracker(EXrFacialTrackingType trackingType)
{
	XrFacialTrackingTypeHTC trackType = static_cast<XrFacialTrackingTypeHTC>(trackingType);
	if (!GetViveOpenXRFacialTrackingModule().FTSource.IsValid()) return false;
	return GetViveOpenXRFacialTrackingModule().FTSource->DestroyFacialTracker(trackType);
}
*/
bool UViveOpenXRFacialTrackingFunctionLibrary::GetEyeFacialExpressions(bool& isActive, TMap<EEyeShape, float>& blendshapes)
{
	if (!GetViveOpenXRFacialTrackingModule().FTSource.IsValid()) return false;
	bool isSuccess = GetViveOpenXRFacialTrackingModule().FTSource->GetEyeExpressions(isActive);
	blendshapes = GetViveOpenXRFacialTrackingModule().FTSource->eyeShapes;
	return isSuccess;
}

bool UViveOpenXRFacialTrackingFunctionLibrary::GetLipFacialExpressions(bool& isActive, TMap<ELipShape, float>& blendshapes)
{
	if (!GetViveOpenXRFacialTrackingModule().FTSource.IsValid()) return false;
	bool isSuccess = GetViveOpenXRFacialTrackingModule().FTSource->GetLipExpressions(isActive);
	blendshapes = GetViveOpenXRFacialTrackingModule().FTSource->lipShapes;
	return isSuccess;
}