// Copyright HTC Corporation. All Rights Reserved.

#include "ViveRenderDoc.h"

#include "IVulkanDynamicRHI.h"
//#include "VulkanPlatformDefines.h"
#include "VulkanPlatform.h"
#include "VulkanRHIPrivate.h"
#include "VulkanContext.h"

DEFINE_LOG_CATEGORY(LogViveRenderDoc);

/*
VULKAN_HAS_DEBUGGING_ENABLED   true
VULKAN_HAS_VALIDATION_FEATURES true
VULKAN_SUPPORTS_DEBUG_UTILS    false
VK_EXT_debug_utils             true
*/

FViveRenderDoc::FViveRenderDoc()
{
	//Enable instance layer VK_LAYER_RENDERDOC_Capture. VK_LAYER_RENDERDOC_Capture will be used if the application launched by RenderDoc.
	TArray<const ANSICHAR*> InstanceExtensions;
	TArray<const ANSICHAR*> InstanceLayers;
	InstanceLayers.Add("VK_LAYER_RENDERDOC_Capture");
	IVulkanDynamicRHI::AddEnabledInstanceExtensionsAndLayers(InstanceExtensions, InstanceLayers);
}

const void* FViveRenderDoc::OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext)
{
	static bool bIsLaunchByRenderDoc = IsLaunchByRenderDoc();
	if(bIsLaunchByRenderDoc)
		InsertDebugUtilsLabel();

	return InNext;
}

void FViveRenderDoc::InsertDebugUtilsLabel()
{
	if(PLATFORM_ANDROID) {
		static int counter = 0;
		if(counter%500 == 0) {
			UE_LOG(LogViveRenderDoc, Log, TEXT("InsertDebugUtilsLabel. This function will be called per frame but log per 500 frame."));
		}
		counter++;

		if (InsertDebugUtilsLabelEXT == nullptr) {
			InsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)(void*)VulkanRHI::vkGetInstanceProcAddr(GVulkanRHI->GetInstance(), "vkCmdInsertDebugUtilsLabelEXT");
			UE_LOG(LogViveRenderDoc, Log, TEXT("GetInstanceProcAddr InsertDebugUtilsLabelEXT: %p"), InsertDebugUtilsLabelEXT);
		}

		if (InsertDebugUtilsLabelEXT != nullptr) {
			FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
			FVulkanCommandBufferManager* CmdMgr = FVulkanCommandListContext::GetVulkanContext(RHICmdList.GetContext()).GetCommandBufferManager();
			VkCommandBuffer CmdBuffer = CmdMgr->GetActiveCmdBuffer()->GetHandle();
			const VkDebugUtilsLabelEXT LabelInfo_Insert =
			{
				VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, // sType
				NULL,                                    // pNext
				"vr-marker,frame_end,type,application",  // pLabelName
				{ 1.0f, 1.0f, 1.0f, 1.0f }               // color
			};
			InsertDebugUtilsLabelEXT(CmdBuffer, &LabelInfo_Insert);
		} else {
			UE_LOG(LogViveRenderDoc, Error, TEXT("InsertDebugUtilsLabelEXT should NOT be nullptr"));
		}
	}
}

bool FViveRenderDoc::IsLaunchByRenderDoc()
{
	UE_LOG(LogViveRenderDoc, Log, TEXT("IsLaunchByRenderDoc"));
	bool result = false;

	//Dump instance extensions and layers
	TArray<const ANSICHAR*> InstanceExtensions;
	TArray<const ANSICHAR*> InstanceLayers;
	if(GDynamicRHI)
	{
		if((FString)(GDynamicRHI->GetName()) == TEXT("Vulkan")) {
			FVulkanDynamicRHI *vkDynamicRHI = static_cast<FVulkanDynamicRHI *>(GDynamicRHI);
			for (const ANSICHAR* Extension : vkDynamicRHI->GetInstanceExtensions())
			{
				FString ExtensionStr = ANSI_TO_TCHAR(Extension);
				UE_LOG(LogViveRenderDoc, Log, TEXT("Vulkan instance extension: %s"), *ExtensionStr);
			}
			for (const ANSICHAR* Layer : vkDynamicRHI->GetInstanceLayers())
			{
				FString LayerStr = ANSI_TO_TCHAR(Layer);
				UE_LOG(LogViveRenderDoc, Log, TEXT("Vulkan instance layer: %s"), *LayerStr);
				//Check instance layer
				if(LayerStr.Equals(FString(TEXT("VK_LAYER_RENDERDOC_Capture")))) {
					result = true;
				}
			}
		} else {
			UE_LOG(LogViveRenderDoc, Warning, TEXT("GDynamicRHI is NOT FVulkanDynamicRHI"));
		}
	} else {
		UE_LOG(LogViveRenderDoc, Warning, TEXT("GDynamicRHI is nullptr"));
	}

	//Dump device extensions and layers
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
	FVulkanDevice* VulkanDevice = FVulkanCommandListContext::GetVulkanContext(RHICmdList.GetContext()).GetDevice();
	if(VulkanDevice) {
		TArray<const ANSICHAR*> DeviceExtensions = VulkanDevice->GetDeviceExtensions();
		for (const ANSICHAR* Extension : DeviceExtensions)
		{
			FString ExtensionStr = ANSI_TO_TCHAR(Extension);
			UE_LOG(LogViveRenderDoc, Log, TEXT("Vulkan device extension: %s"), *ExtensionStr);
		}
		TArray<const ANSICHAR*> DeviceLayers;
		FVulkanPlatform::GetDeviceLayers(DeviceLayers);
		for (const ANSICHAR* Layer : DeviceLayers)
		{
			FString LayerStr = ANSI_TO_TCHAR(Layer);
			UE_LOG(LogViveRenderDoc, Log, TEXT("Vulkan device layer: %s"), *LayerStr);
		}
	}

	if(result) {
		UE_LOG(LogViveRenderDoc, Log, TEXT("Launched by RenderDoc, VK_LAYER_RENDERDOC_Capture is in use."));
	} else {
		UE_LOG(LogViveRenderDoc, Log, TEXT("Did NOT launch by RenderDoc, VK_LAYER_RENDERDOC_Capture is NOT in use."));
	}

	return result;
}

void FViveRenderDoc::DumpVulkanInfos()
{
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
	FVulkanCommandBufferManager* CmdMgr = FVulkanCommandListContext::GetVulkanContext(RHICmdList.GetContext()).GetCommandBufferManager();
	VkCommandBuffer CmdBuffer = CmdMgr->GetActiveCmdBuffer()->GetHandle();
	VkCommandBuffer UploadCmdBuffer = CmdMgr->GetUploadCmdBuffer()->GetHandle();
	FVulkanDevice* VulkanDevice = FVulkanCommandListContext::GetVulkanContext(RHICmdList.GetContext()).GetDevice();
	VkDevice Device = VulkanDevice->GetInstanceHandle();
	UE_LOG(LogViveRenderDoc, Log, TEXT("CmdBuffer: 0x%p, Device: 0x%p, UploadCmdBuffer: 0x%p"), CmdBuffer, Device, UploadCmdBuffer);
}

IMPLEMENT_MODULE(FViveRenderDoc, ViveRenderDoc)
