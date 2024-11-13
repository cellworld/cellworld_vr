// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRDisplayRefreshRateFunctionLibrary.h"
#include "ViveOpenXRDisplayRefreshRateModule.h"
#include "OpenXRHMD.h"

static FViveOpenXRDisplayRefreshRate* FViveOpenXRDisplayRefreshRatePtr = nullptr;

FViveOpenXRDisplayRefreshRate* GetViveOpenXRDisplayRefreshRateModulePtr()
{
	if (FViveOpenXRDisplayRefreshRatePtr != nullptr)
	{
		return FViveOpenXRDisplayRefreshRatePtr;
	}
	else
	{
		if (GEngine->XRSystem.IsValid())
		{
			auto HMD = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());
			for (IOpenXRExtensionPlugin* Module : HMD->GetExtensionPlugins())
			{
				if (Module->GetDisplayName() == TEXT("ViveOpenXRDisplayRefreshRate"))
				{
					FViveOpenXRDisplayRefreshRatePtr = static_cast<FViveOpenXRDisplayRefreshRate*>(Module);
					break;
				}
			}
		}
		return FViveOpenXRDisplayRefreshRatePtr;
	}
}

bool UViveOpenXRDisplayRefreshRateFunctionLibrary::EnumerateDisplayRefreshRates(TArray<float>& OutDisplayRefreshRates)
{
	if (!GetViveOpenXRDisplayRefreshRateModulePtr()) return false;
	return GetViveOpenXRDisplayRefreshRateModulePtr()->EnumerateDisplayRefreshRates(OutDisplayRefreshRates);
}

bool UViveOpenXRDisplayRefreshRateFunctionLibrary::GetDisplayRefreshRate(float& OutDisplayRefreshRate)
{
	if (!GetViveOpenXRDisplayRefreshRateModulePtr()) return false;
	return GetViveOpenXRDisplayRefreshRateModulePtr()->GetDisplayRefreshRate(OutDisplayRefreshRate);
}

bool UViveOpenXRDisplayRefreshRateFunctionLibrary::RequestDisplayRefreshRate(float InDisplayRefreshRate)
{
	if (!GetViveOpenXRDisplayRefreshRateModulePtr()) return false;
	return GetViveOpenXRDisplayRefreshRateModulePtr()->RequestDisplayRefreshRate(InDisplayRefreshRate);
}
