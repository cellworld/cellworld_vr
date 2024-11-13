// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViveOpenXRPassthroughFunctionLibrary.h"
#include "VivePassthroughEnums.h"
#include "ViveOpenXRAndroidPassthroughFunctionLibrary.generated.h"


UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRPASSTHROUGH_API UViveOpenXRAndroidPassthroughFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Is Passthrough Enabled'",
		DisplayName = "Get Is Passthrough Enabled (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
    static void GetIsPassthroughEnabled(bool& result);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Create Passthrough Underlay'",
		DisplayName = "Create Passthrough Underlay (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool CreatePassthroughUnderlay(EXrPassthroughLayerForm inPassthroughLayerForm);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Destroy Passthrough Underlay'",
		DisplayName = "Destroy Passthrough Underlay (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool DestroyPassthroughUnderlay();

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Alpha'",
		DisplayName = "Set Passthrough Alpha (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughAlpha(float alpha);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh'",
		DisplayName = "Set Passthrough Mesh (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMesh(const TArray<FVector>& vertices, const TArray<int32>& indices);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh Transform'",
		DisplayName = "Set Passthrough Mesh Transform (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMeshTransform(EProjectedPassthroughSpaceType meshSpaceType, FTransform meshTransform);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh Transform Space'",
		DisplayName = "Set Passthrough Mesh Transform Space (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMeshTransformSpace(EProjectedPassthroughSpaceType meshSpaceType);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh Transform Location'",
		DisplayName = "Set Passthrough Mesh Transform Position (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMeshTransformPosition(FVector meshPosition);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh Transform Rotation'",
		DisplayName = "Set Passthrough Mesh Transform Orientation (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMeshTransformOrientation(FRotator meshOrientation);

	UFUNCTION(BlueprintPure, meta = (
		DeprecatedFunction, DeprecationMessage = "Please use the new function 'Set Passthrough Mesh Transform Scale'",
		DisplayName = "Set Passthrough Mesh Transform Scale (Deprecated)", Keywords = "ViveOpenXRAndroid Passthrough"), Category = "ViveOpenXRAndroid|OpenXR|Passthrough")
	static bool SetPassthroughMeshTransformScale(FVector meshScale);
};
