// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRPassthrough.h"

DEFINE_LOG_CATEGORY(LogViveOpenXRPassthrough);


void FViveOpenXRPassthrough::StartupModule()
{
	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnablePassthrough"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnablePassthrough = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnablePassthrough = true;
		}
	}

	if (m_bEnablePassthrough)
	{
		UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Enable Passthrough."));
	}
	else
	{
		UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Disable Passthrough."));
		return;
	}

	RegisterOpenXRExtensionModularFeature();
	UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("StartupModule() Finished."));
}

const void* FViveOpenXRPassthrough::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!m_bEnablePassthrough) return InNext;

	UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Entry Passthrough OnCreateSession."));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreatePassthroughHTC", (PFN_xrVoidFunction*)&xrCreatePassthroughHTC));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyPassthroughHTC", (PFN_xrVoidFunction*)&xrDestroyPassthroughHTC));

	return InNext;
}

const void* FViveOpenXRPassthrough::OnBeginSession(XrSession InSession, const void* InNext)
{
	if (!m_bEnablePassthrough) return InNext;

	UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Entry Passthrough OnBeginSession InSession %lu."), InSession);

	static FName SystemName(TEXT("OpenXR"));

	//From OpenXRHMD.cpp
	uint32_t ReferenceSpacesCount;
	XR_ENSURE(xrEnumerateReferenceSpaces(InSession, 0, &ReferenceSpacesCount, nullptr));

	TArray<XrReferenceSpaceType> Spaces;
	Spaces.SetNum(ReferenceSpacesCount);
	for (auto& SpaceIter : Spaces)
		SpaceIter = XR_REFERENCE_SPACE_TYPE_VIEW;
	XR_ENSURE(xrEnumerateReferenceSpaces(InSession, (uint32_t)Spaces.Num(), &ReferenceSpacesCount, Spaces.GetData()));
	ensure(ReferenceSpacesCount == Spaces.Num());

	XrSpace HmdSpace = XR_NULL_HANDLE;
	XrReferenceSpaceCreateInfo SpaceInfo;

	ensure(Spaces.Contains(XR_REFERENCE_SPACE_TYPE_VIEW));
	SpaceInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	SpaceInfo.next = nullptr;
	SpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	SpaceInfo.poseInReferenceSpace = ToXrPose(FTransform::Identity);
	XR_ENSURE(xrCreateReferenceSpace(InSession, &SpaceInfo, &m_HeadLockSpace));

	return InNext;
}

bool FViveOpenXRPassthrough::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnablePassthrough)
	{
		UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("GetRequiredExtensions() Add Passthrough Extension Name %s."), ANSI_TO_TCHAR(XR_HTC_PASSTHROUGH_EXTENSION_NAME));
		OutExtensions.Add(XR_HTC_PASSTHROUGH_EXTENSION_NAME);
	}
	return true;
}

void FViveOpenXRPassthrough::PostCreateSession(XrSession InSession)
{
	UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Entry Passthrough PostCreateSession InSession %lu."), InSession);
	m_Session = InSession;
}

const void* FViveOpenXRPassthrough::OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags)
{
	//UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("Entry Passthrough OnEndProjectionLayer InSession %lu."), InSession);

	if (isPassthroughCreated)
	{
		if (toBeDestroyed) //Reclaim resources
		{
			if (passthroughCompositionLayerInfoPtr)
			{
				delete passthroughCompositionLayerInfoPtr;
				passthroughCompositionLayerInfoPtr = nullptr;
			}
			if (passthroughMeshTransformInfoPtr)
			{
				delete passthroughMeshTransformInfoPtr;
				passthroughMeshTransformInfoPtr = nullptr;
			}

			isPassthroughCreated = false;
		}
		else //Add passthrough info into next chain
		{
			if (passthroughCompositionLayerInfoPtr)
			{
				OutFlags |= XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;

				if (m_CurrentLayerForm == XR_PASSTHROUGH_FORM_PLANAR_HTC) //Assign InNext ptr to the passthrough struct to preserve next chain
				{
					passthroughCompositionLayerInfoPtr->next = InNext; 
				}
				else if (m_CurrentLayerForm == XR_PASSTHROUGH_FORM_PROJECTED_HTC && passthroughMeshTransformInfoPtr)
				{
					passthroughCompositionLayerInfoPtr->next = reinterpret_cast<const void*>(passthroughMeshTransformInfoPtr);
					passthroughMeshTransformInfoPtr->next = InNext;
				}
				return reinterpret_cast<const void*>(passthroughCompositionLayerInfoPtr);
			}
		}
	}

	return InNext;
}

void FViveOpenXRPassthrough::UpdateCompositionLayers(XrSession InSession, TArray<const XrCompositionLayerBaseHeader*>& Headers)
{
	if (!m_bEnablePassthrough) return;
	if (!isPassthroughCreated) return;
	if (passthroughCompositionLayerInfoPtr)
	{
		Headers.Add(reinterpret_cast<const XrCompositionLayerBaseHeader*>(passthroughCompositionLayerInfoPtr));
	}
	else
	{
		UE_LOG(LogViveOpenXRPassthrough, Warning, TEXT("UpdateCompositionLayers during EndFrame: passthroughCompositionLayerInfoPtr null reference."));
	}
}

bool FViveOpenXRPassthrough::CreatePassthrough(XrPassthroughFormHTC layerForm)
{
	if (!m_bEnablePassthrough) return false;
	if (isPassthroughCreated) return false;

	XrPassthroughHTC newPassthroughHandle = XR_NULL_HANDLE;
	XrPassthroughCreateInfoHTC createInfo{ XR_TYPE_PASSTHROUGH_CREATE_INFO_HTC };

	createInfo.form = layerForm;

	//Create Passthrough handle
	XR_ENSURE(xrCreatePassthroughHTC(m_Session, &createInfo, &passthroughHandle));

	XrCompositionLayerPassthroughHTC* newPassthroughCompositionLayerInfoPtr = new XrCompositionLayerPassthroughHTC();
	newPassthroughCompositionLayerInfoPtr->type = XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_HTC;
	newPassthroughCompositionLayerInfoPtr->next = nullptr;
	newPassthroughCompositionLayerInfoPtr->layerFlags = 0;
	newPassthroughCompositionLayerInfoPtr->layerFlags |= 1UL << XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	newPassthroughCompositionLayerInfoPtr->space = XR_NULL_HANDLE;
	newPassthroughCompositionLayerInfoPtr->passthrough = passthroughHandle;

	XrPassthroughColorHTC newPassthroughColor = { XR_TYPE_PASSTHROUGH_COLOR_HTC, nullptr, 1.0 }; //default value
	newPassthroughCompositionLayerInfoPtr->color = newPassthroughColor;

	passthroughCompositionLayerInfoPtr = newPassthroughCompositionLayerInfoPtr;

	if (layerForm == XR_PASSTHROUGH_FORM_PROJECTED_HTC)
	{
		XrPassthroughMeshTransformInfoHTC* newPassthroughMeshTransformInfoPtr = new XrPassthroughMeshTransformInfoHTC();
		newPassthroughMeshTransformInfoPtr->type = XR_TYPE_PASSTHROUGH_MESH_TRANSFORM_INFO_HTC;
		newPassthroughMeshTransformInfoPtr->next = nullptr;
		newPassthroughMeshTransformInfoPtr->vertexCount = 0;
		newPassthroughMeshTransformInfoPtr->vertices = nullptr;
		newPassthroughMeshTransformInfoPtr->indexCount = 0;
		newPassthroughMeshTransformInfoPtr->indices = nullptr;
		newPassthroughMeshTransformInfoPtr->baseSpace = XR_NULL_HANDLE;
		newPassthroughMeshTransformInfoPtr->time = 0;
		newPassthroughMeshTransformInfoPtr->pose = ToXrPose(FTransform::Identity);
		newPassthroughMeshTransformInfoPtr->scale = ToXrVector(FVector::One());

		passthroughMeshTransformInfoPtr = newPassthroughMeshTransformInfoPtr;
	}

	isPassthroughCreated = true;
	toBeDestroyed = false;
	m_CurrentLayerForm = layerForm;

	return true;
}


bool FViveOpenXRPassthrough::DestroyPassthrough()
{
	if (!isPassthroughCreated) return false;

	if (passthroughCompositionLayerInfoPtr && !toBeDestroyed)
	{
		toBeDestroyed = true; //Reclaim resources in EndFrame
		XR_ENSURE(xrDestroyPassthroughHTC(passthroughHandle));
	}
	else
	{
		UE_LOG(LogViveOpenXRPassthrough, Log, TEXT("htcpassthrough_DestroyPassthrough: passthroughRenderInfo null reference."));
		return false;
	}

	return true;
}

//For projected passthrough
bool FViveOpenXRPassthrough::SetPassthroughAlpha(float alpha)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughCompositionLayerInfoPtr || toBeDestroyed) return false;

	passthroughCompositionLayerInfoPtr->color.alpha = alpha;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMesh(uint32_t inVertexCount, const XrVector3f* inVertexBuffer, uint32_t inIndexCount, const uint32_t* inIndexBuffer)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->vertexCount = inVertexCount;
	passthroughMeshTransformInfoPtr->vertices = inVertexBuffer;
	passthroughMeshTransformInfoPtr->indexCount = inIndexCount;
	passthroughMeshTransformInfoPtr->indices = inIndexBuffer;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMeshTransform(XrSpace meshSpace, XrPosef meshPose, XrVector3f meshScale)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->baseSpace = meshSpace;
	passthroughMeshTransformInfoPtr->pose = meshPose;
	passthroughMeshTransformInfoPtr->scale = meshScale;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMeshTransformSpace(XrSpace meshSpace)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->baseSpace = meshSpace;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMeshTransformPosition(XrVector3f meshPosition)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->pose.position = meshPosition;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMeshTransformOrientation(XrQuaternionf meshOrientation)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->pose.orientation = meshOrientation;

	return true;
}


bool FViveOpenXRPassthrough::SetPassthroughMeshTransformScale(XrVector3f meshScale)
{
	if (!m_bEnablePassthrough) return false;

	if (!isPassthroughCreated || !passthroughMeshTransformInfoPtr || toBeDestroyed) return false;

	passthroughMeshTransformInfoPtr->scale = meshScale;

	return true;
}

XrSpace FViveOpenXRPassthrough::GetHeadlockXrSpace()
{
	return m_HeadLockSpace;
}

IMPLEMENT_MODULE(FViveOpenXRPassthrough, ViveOpenXRPassthrough)