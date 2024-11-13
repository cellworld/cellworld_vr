// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRAnchorFunctionLibrary.generated.h"

/**
 * Blueprint function library for ViveOpenXR HTC Anchor extension.
 */
UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRANCHOR_API UViveOpenXRAnchorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Check if Anchor is supported by requesting SystemProperties."))
	static bool IsAnchorSupported();

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Creates a spatial anchor with specified base space and pose in the space. The anchor is represented by an XrSpace. In Unreal, anchor is in the form of int64. And its pose can be tracked via xrLocateSpace. Once the anchor is no longer needed, call xrDestroySpace to erase the anchor."))
	static bool CreateSpatialAnchor(FVector position, FRotator rotation, FString name, int64& anchor);

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Similar to CreateSpatialAnchor, and take a transform for input. Transform's scale is ignored."))
	static bool CreateSpatialAnchorT(FTransform transform, FString name, int64& anchor);

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Call xrDestroySpace to erase the anchor."))
	static void DestroySpatialAnchor(int64 anchor);

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Call xrLocateSpace to get the anchor's current pose based on tracking origin setting."))
	static bool LocateAnchor(int64 anchor, FTransform& transform);

	UFUNCTION(/*BlueprintCallable,*/ Category = "ViveOpenXR|SpatialAnchor",
		meta = (Tooltip = "Gets the name given to the anchor when it was created."))
	static bool GetSpatialAnchorName(int64 anchor, FString& name);
};
