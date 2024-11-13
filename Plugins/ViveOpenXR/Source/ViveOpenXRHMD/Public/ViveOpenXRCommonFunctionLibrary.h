// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRCommonFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ViveOXRCommon, Log, All);

/**
 * Blueprint function library for ViveOpenXR common API.
 */
UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRHMD_API UViveOpenXRCommonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Assume basespace is tracking space and time is current time
	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|Common",
		meta = (Tooltip = "Get the space's current reated transform based on tracking origin setting. In Unreal, the XrSpace is in form of Int64."))
	static bool LocateXrSpace(int64 space, FTransform& transform);

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|Common",
		meta = (Tooltip = "Destory the XrSpace. In Unreal, the XrSpace is in form of Int64."))
	static bool DestroyXrSpace(int64 space);
};
