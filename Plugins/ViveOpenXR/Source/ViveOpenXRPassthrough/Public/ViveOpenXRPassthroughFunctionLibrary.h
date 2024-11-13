// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "VivePassthroughEnums.h"
#include "ViveOpenXRPassthroughFunctionLibrary.generated.h"


UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRPASSTHROUGH_API UViveOpenXRPassthroughFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Passthrough Enabled", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
    static bool IsPassthroughEnabled();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Create Passthrough Underlay", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool CreatePassthroughUnderlay(EXrPassthroughLayerForm inPassthroughLayerForm);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Destroy Passthrough Underlay", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool DestroyPassthroughUnderlay();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Alpha", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughAlpha(float alpha);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMesh(const TArray<FVector>& vertices, const TArray<int32>& indices);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh Transform", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMeshTransform(EProjectedPassthroughSpaceType meshSpaceType, FTransform meshTransform);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh Transform Space", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMeshTransformSpace(EProjectedPassthroughSpaceType meshSpaceType);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh Transform Location", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMeshTransformLocation(FVector meshLocation);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh Transform Rotation", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMeshTransformRotation(FRotator meshRotation);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Passthrough Mesh Transform Scale", Keywords = "ViveOpenXR Passthrough"), Category = "ViveOpenXR|Passthrough")
	static bool SetPassthroughMeshTransformScale(FVector meshScale);
};
