// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXREyeTrackerFunctionLibrary.h"
#include "ViveOpenXREyeTracker.h"
#include "OpenXRHMD.h"

static FViveOpenXREyeTracker* FViveOpenXREyeTrackerPtr = nullptr;

FViveOpenXREyeTracker* GetViveOpenXREyeTrackerModulePtr()
{
	if (FViveOpenXREyeTrackerPtr != nullptr)
	{
		return FViveOpenXREyeTrackerPtr;
	}
	else
	{
		if (GEngine->XRSystem.IsValid())
		{
			auto HMD = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());
			for (IOpenXRExtensionPlugin* Module : HMD->GetExtensionPlugins())
			{
				if (Module->GetDisplayName() == TEXT("ViveOpenXREyeTracker"))
				{
					FViveOpenXREyeTrackerPtr = static_cast<FViveOpenXREyeTracker*>(Module);
					break;
				}
			}
		}
		return FViveOpenXREyeTrackerPtr;
	}
}

FXrGazeDataHTC UViveOpenXREyeTrackerFunctionLibrary::GetHTCEyeGazeValidData()
{
	FXrGazeDataHTC OutGazeValidData;
	if (!GetViveOpenXREyeTrackerModulePtr()) return OutGazeValidData;
	return GetViveOpenXREyeTrackerModulePtr()->GetEyeGazeValidDatas();
}

FXrPupilDataHTC UViveOpenXREyeTrackerFunctionLibrary::GetHTCPupilData()
{
	FXrPupilDataHTC OutPupilData;
	if (!GetViveOpenXREyeTrackerModulePtr()) return OutPupilData;
	return GetViveOpenXREyeTrackerModulePtr()->GetPupilDatas();
}

FXrEyeGeometricDataHTC UViveOpenXREyeTrackerFunctionLibrary::GetHTCEyeGeometricData()
{
	FXrEyeGeometricDataHTC OutEyeGeometricData;
	if (!GetViveOpenXREyeTrackerModulePtr()) return OutEyeGeometricData;
	return GetViveOpenXREyeTrackerModulePtr()->GetEyeGeometricDatas();
}


