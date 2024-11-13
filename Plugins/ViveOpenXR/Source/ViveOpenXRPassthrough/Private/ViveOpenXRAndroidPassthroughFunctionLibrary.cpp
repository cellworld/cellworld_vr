// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRAndroidPassthroughFunctionLibrary.h"
#include "ViveOpenXRPassthrough.h"


void UViveOpenXRAndroidPassthroughFunctionLibrary::GetIsPassthroughEnabled(bool& result)
{
	result = UViveOpenXRPassthroughFunctionLibrary::IsPassthroughEnabled();
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::CreatePassthroughUnderlay(EXrPassthroughLayerForm inPassthroughLayerForm)
{
	return UViveOpenXRPassthroughFunctionLibrary::CreatePassthroughUnderlay(inPassthroughLayerForm);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::DestroyPassthroughUnderlay()
{
	return UViveOpenXRPassthroughFunctionLibrary::DestroyPassthroughUnderlay();
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughAlpha(float alpha)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughAlpha(alpha);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMesh(const TArray<FVector>& vertices, const TArray<int32>& indices)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMesh(vertices, indices);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMeshTransform(EProjectedPassthroughSpaceType meshSpaceType, FTransform meshTransform)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransform(meshSpaceType, meshTransform);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMeshTransformSpace(EProjectedPassthroughSpaceType meshSpaceType)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformSpace(meshSpaceType);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMeshTransformPosition(FVector meshPosition)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformLocation(meshPosition);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMeshTransformOrientation(FRotator meshOrientation)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformRotation(meshOrientation);
}

bool UViveOpenXRAndroidPassthroughFunctionLibrary::SetPassthroughMeshTransformScale(FVector meshScale)
{
	return UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformScale(meshScale);
}