// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "IOpenXRExtensionPlugin.h"
#include "OpenXRCommon.h"
#include "CoreMinimal.h"
//#include "IViveOpenXRExtensionPlugin.h"
//#include "ViveOpenXRExtensionPluginDefinition.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRPassthrough, Log, All);

class FViveOpenXRPassthrough : public IModuleInterface, public IOpenXRExtensionPlugin
{
public:
	FViveOpenXRPassthrough(){}
	virtual ~FViveOpenXRPassthrough(){}

	/************************************************************************/
	/* IModuleInterface                                                     */
	/************************************************************************/
	virtual void StartupModule() override;
	virtual void ShutdownModule() override
	{
		UnregisterOpenXRExtensionModularFeature();
	}

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ViveOpenXRPassthrough"));
	}

	/** IOpenXRExtensionPlugin implementation */
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual const void* OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags) override;
	virtual void UpdateCompositionLayers(XrSession InSession, TArray<const XrCompositionLayerBaseHeader*>& Headers) override;

	//Passthrough Underlay
	bool CreatePassthrough(XrPassthroughFormHTC layerForm);
	bool DestroyPassthrough();

	//For projected passthrough
	bool SetPassthroughAlpha(float alpha);
	bool SetPassthroughMesh(uint32_t inVertexCount, const XrVector3f* inVertexBuffer, uint32_t inIndexCount, const uint32_t* inIndexBuffer);
	bool SetPassthroughMeshTransform(XrSpace meshSpace, XrPosef meshPose, XrVector3f meshScale);
	bool SetPassthroughMeshTransformSpace(XrSpace meshSpace);
	bool SetPassthroughMeshTransformPosition(XrVector3f meshPosition);
	bool SetPassthroughMeshTransformOrientation(XrQuaternionf meshOrientation);
	bool SetPassthroughMeshTransformScale(XrVector3f meshScale);

	XrSpace GetHeadlockXrSpace();

public:
	bool m_bEnablePassthrough = false;

private:
	XrInstance m_XrInstance;
	XrSystemId m_XrSystemId;
	XrSession m_Session = XR_NULL_HANDLE;

	XrSpace m_HeadLockSpace = XR_NULL_HANDLE;

	XrPassthroughFormHTC m_CurrentLayerForm = XrPassthroughFormHTC::XR_PASSTHROUGH_FORM_MAX_ENUM_HTC;
	bool isPassthroughCreated = false;
	bool toBeDestroyed = false;

	//OpenXR Function Ptrs
	PFN_xrCreatePassthroughHTC xrCreatePassthroughHTC = nullptr;
	PFN_xrDestroyPassthroughHTC xrDestroyPassthroughHTC = nullptr;

	//Info of current passthrough
	XrPassthroughHTC passthroughHandle = XR_NULL_HANDLE;
	XrCompositionLayerPassthroughHTC* passthroughCompositionLayerInfoPtr = nullptr;
	XrPassthroughColorHTC passthroughColorInfo{};
	XrPassthroughMeshTransformInfoHTC* passthroughMeshTransformInfoPtr = nullptr;
};