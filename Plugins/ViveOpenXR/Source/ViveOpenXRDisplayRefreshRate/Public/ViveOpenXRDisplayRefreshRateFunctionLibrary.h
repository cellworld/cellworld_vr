// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRDisplayRefreshRateFunctionLibrary.generated.h"

/**
 * Blueprint function library for ViveOpenXR Display Refresh Rate extension.
 */
UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRDISPLAYREFRESHRATE_API UViveOpenXRDisplayRefreshRateFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Enumerate Display Refresh Rates", Keywords = "ViveOpenXR DisplayRefreshRate"), Category = "ViveOpenXR|DisplayRefreshRate")
	static bool EnumerateDisplayRefreshRates(TArray<float>& OutDisplayRefreshRates);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Display Refresh Rate", Keywords = "ViveOpenXR DisplayRefreshRate"), Category = "ViveOpenXR|DisplayRefreshRate")
	static bool GetDisplayRefreshRate(float& OutDisplayRefreshRate);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request Display Refresh Rate", Keywords = "ViveOpenXR DisplayRefreshRate"), Category = "ViveOpenXR|DisplayRefreshRate")
	static bool RequestDisplayRefreshRate(float InDisplayRefreshRate);
};
