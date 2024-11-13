// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRPassthroughFunctionLibrary.h"
#include "ViveOpenXRPassthrough.h"
#include "OpenXRHMD.h"

static FViveOpenXRPassthrough* FViveOpenXRPassthroughPtr = nullptr;

FViveOpenXRPassthrough* GetViveOpenXRPassthroughModulePtr()
{
	if (FViveOpenXRPassthroughPtr != nullptr)
	{
		return FViveOpenXRPassthroughPtr;
	}
	else
	{
		if (GEngine->XRSystem.IsValid())
		{
			auto HMD = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());
			for (IOpenXRExtensionPlugin* Module : HMD->GetExtensionPlugins())
			{
				if (Module->GetDisplayName() == TEXT("ViveOpenXRPassthrough"))
				{
					FViveOpenXRPassthroughPtr = static_cast<FViveOpenXRPassthrough*>(Module);
					break;
				}
			}
		}
		return FViveOpenXRPassthroughPtr;
	}
}

float GetOpenXRHMDWorldToMeterScale()
{
	return static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice())->GetWorldToMetersScale();
}

XrSpace GetOpenXRHMDTrackingSpace()
{
	return static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice())->GetTrackingSpace();
}

bool UViveOpenXRPassthroughFunctionLibrary::IsPassthroughEnabled()
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;
	return GetViveOpenXRPassthroughModulePtr()->m_bEnablePassthrough;
}


bool UViveOpenXRPassthroughFunctionLibrary::CreatePassthroughUnderlay(EXrPassthroughLayerForm inPassthroughLayerForm)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	XrPassthroughFormHTC passthroughLayerForm = static_cast<XrPassthroughFormHTC>(inPassthroughLayerForm);
	return GetViveOpenXRPassthroughModulePtr()->CreatePassthrough(passthroughLayerForm);

}

bool UViveOpenXRPassthroughFunctionLibrary::DestroyPassthroughUnderlay()
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	return GetViveOpenXRPassthroughModulePtr()->DestroyPassthrough();
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughAlpha(float alpha)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughAlpha(alpha);
}

XrVector3f* xrVertexBuffer = nullptr;
uint32_t* xrIndexBuffer = nullptr;

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMesh(const TArray<FVector>& vertices, const TArray<int32>& indices)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	int numVertices = vertices.Num();
	int numIndices = indices.Num();
	if ((numIndices % 3) != 0)
		return false;

	if (xrVertexBuffer) delete xrVertexBuffer;
	if (xrIndexBuffer) delete xrIndexBuffer;

	xrVertexBuffer = new XrVector3f[numVertices];
	xrIndexBuffer = new uint32_t[numIndices];

	for (int i = 0; i < numVertices; i++)
	{
		xrVertexBuffer[i] = ToXrVector(vertices[i], GetOpenXRHMDWorldToMeterScale());
	}

	for (int i = 0; i < numIndices; i++)
	{
		xrIndexBuffer[i] = (uint32_t)indices[i];
	}

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMesh((uint32_t)numVertices, xrVertexBuffer, (uint32_t)numIndices, xrIndexBuffer);
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransform(EProjectedPassthroughSpaceType meshSpaceType, FTransform meshTransform)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	XrSpace meshSpace = (meshSpaceType == EProjectedPassthroughSpaceType::Headlock ? GetViveOpenXRPassthroughModulePtr()->GetHeadlockXrSpace() : GetOpenXRHMDTrackingSpace());

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMeshTransform(meshSpace, ToXrPose(meshTransform, GetOpenXRHMDWorldToMeterScale()), XrVector3f{ (float)meshTransform.GetScale3D().Y, (float)meshTransform.GetScale3D().Z, (float)meshTransform.GetScale3D().X });
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformSpace(EProjectedPassthroughSpaceType meshSpaceType)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	XrSpace meshSpace = (meshSpaceType == EProjectedPassthroughSpaceType::Headlock ? GetViveOpenXRPassthroughModulePtr()->GetHeadlockXrSpace() : GetOpenXRHMDTrackingSpace());

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMeshTransformSpace(meshSpace);
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformLocation(FVector meshLcation)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMeshTransformPosition(ToXrVector(meshLcation, GetOpenXRHMDWorldToMeterScale()));
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformRotation(FRotator meshRotation)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMeshTransformOrientation(ToXrQuat(meshRotation.Quaternion()));
}

bool UViveOpenXRPassthroughFunctionLibrary::SetPassthroughMeshTransformScale(FVector meshScale)
{
	if (!GetViveOpenXRPassthroughModulePtr()) return false;

	return GetViveOpenXRPassthroughModulePtr()->SetPassthroughMeshTransformScale(XrVector3f{(float)meshScale.Y, (float)meshScale.Z, (float)meshScale.X});
}
