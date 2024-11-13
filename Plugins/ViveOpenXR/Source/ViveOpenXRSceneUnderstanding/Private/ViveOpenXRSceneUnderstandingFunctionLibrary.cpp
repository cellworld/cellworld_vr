// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRSceneUnderstandingFunctionLibrary.h"
#include "OpenXRCore.h"
#include "Engine/EngineTypes.h"
#include "ViveOpenXRSceneUnderstanding.h"

#define LOCTEXT_NAMESPACE "FViveOpenXRSceneUnderstandingModule"
DEFINE_LOG_CATEGORY(LogViveOpenXR);

namespace ViveOpenXR
{
	static class FViveOpenXRSceneUnderstandingModule* g_ViveOpenXRModule;

	class FViveOpenXRSceneUnderstandingModule : public IModuleInterface
	{
	public:
		FSceneUnderstanding SceneUnderstanding;

		/** IModuleInterface implementation */
		void StartupModule() override
		{
			SceneUnderstanding.Register();
			g_ViveOpenXRModule = this;
		}

		void ShutdownModule() override
		{
			g_ViveOpenXRModule = nullptr;
			SceneUnderstanding.Unregister();
		}
	};
}// namespace ViveOpenXR

void UViveOpenXRSceneUnderstandingFunctionLibrary::GetIsSceneUnderstandingEnabled(bool& result)
{
	result = ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding;
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::SetSceneComputeSphereBound(FVector center, float radius)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;
	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.SetSceneComputeSphereBound(ToXrVector(center), radius);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::SetSceneComputeOrientedBoxBound(FRotator rotator, FVector position, FVector extents)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	FRotationConversionCache rotationConver;
	FQuat orientation = rotationConver.RotatorToQuat(rotator);
	XrVector3f xrVectorExt;
	xrVectorExt.x = extents.X;
	xrVectorExt.y = extents.Y;
	xrVectorExt.z = extents.Z;
	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.SetSceneComputeOrientedBoxBound(ToXrQuat(orientation), ToXrVector(position), xrVectorExt);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::SetSceneComputeFrustumBound(FRotator rotator, FVector position, float angleUp, float angleDown, float angleRight, float angleLeft, float farDistance)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	FRotationConversionCache rotationConver;
	FQuat orientation = rotationConver.RotatorToQuat(rotator);
	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.SetSceneComputeFrustumBound(ToXrQuat(orientation), ToXrVector(position), angleUp, angleDown, angleRight, angleLeft, farDistance);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::ClearSceneComputeBounds(EXrBoundType type)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.ClearSceneComputeBounds((ViveOpenXR::EXrSceneBoundType)type);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::SetSceneComputeConsistency(EXrSceneComputeConsistency consistency)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.SetSceneComputeConsistency((XrSceneComputeConsistencyMSFT)consistency);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::SetMeshComputeLod(EXrMeshComputeLod lod)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.SetMeshComputeLod((XrMeshComputeLodMSFT)lod);
}

void UViveOpenXRSceneUnderstandingFunctionLibrary::OpenPassThroughEXE(FString path, FString param)
{
	if (!ViveOpenXR::g_ViveOpenXRModule->SceneUnderstanding.m_bEnableSceneUnderstanding) return;

	FProcHandle currHandle = FPlatformProcess::CreateProc(*path, *param, true, false, false, nullptr, 0, nullptr, nullptr);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(ViveOpenXR::FViveOpenXRSceneUnderstandingModule, ViveOpenXRSceneUnderstanding)

