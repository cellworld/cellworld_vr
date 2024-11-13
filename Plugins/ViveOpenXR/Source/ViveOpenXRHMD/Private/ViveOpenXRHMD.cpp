// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRHMD.h"
#include "OpenXRCommon.h"

DEFINE_LOG_CATEGORY(LogViveOpenXRHMD);

bool FViveOpenXRHMD::GetCustomLoader(PFN_xrGetInstanceProcAddr* OutGetProcAddr)
{
#if PLATFORM_WINDOWS
	return false;
#endif

#if PLATFORM_ANDROID
	LoaderHandle = FPlatformProcess::GetDllHandle(TEXT("libVIVEopenxr_loader.so"));
	if (LoaderHandle == nullptr)
	{
		UE_LOG(LogViveOpenXRHMD, Error, TEXT("Unable to load libVIVEopenxr_loader.so."));
		return false;
	}

	UE_LOG(LogViveOpenXRHMD, Log, TEXT("libVIVEopenxr_loader.so loaded."));

	PFN_xrGetInstanceProcAddr xrGetInstanceProcAddrPtr = (PFN_xrGetInstanceProcAddr)FPlatformProcess::GetDllExport(LoaderHandle, TEXT("xrGetInstanceProcAddr"));
	if (xrGetInstanceProcAddrPtr == nullptr)
	{
		UE_LOG(LogViveOpenXRHMD, Error, TEXT("Unable to load OpenXR xrGetInstanceProcAddr."));
		return false;
	}
	*OutGetProcAddr = xrGetInstanceProcAddrPtr;

	UE_LOG(LogViveOpenXRHMD, Log, TEXT("OpenXR Android xrGetInstanceProcAddr loaded."));

	extern struct android_app* GNativeAndroidApp;
	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddrPtr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);
	if (xrInitializeLoaderKHR == nullptr)
	{
		UE_LOG(LogViveOpenXRHMD, Error, TEXT("Unable to load OpenXR Android xrInitializeLoaderKHR"));
		return false;
	}

	UE_LOG(LogViveOpenXRHMD, Log, TEXT("OpenXR Android xrInitializeLoaderKHR loaded."));

	XrLoaderInitInfoAndroidKHR LoaderInitializeInfoAndroid;
	LoaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
	LoaderInitializeInfoAndroid.next = NULL;
	LoaderInitializeInfoAndroid.applicationVM = GNativeAndroidApp->activity->vm;
	LoaderInitializeInfoAndroid.applicationContext = GNativeAndroidApp->activity->clazz;
	XR_ENSURE(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&LoaderInitializeInfoAndroid));

	UE_LOG(LogViveOpenXRHMD, Log, TEXT("OpenXR Android xrInitializeLoaderKHR finished."));

	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Loaded OpenXR Android loader."));

	return true;
#endif

	return false;
}

bool FViveOpenXRHMD::IsStandaloneStereoOnlyDevice()
{
#if PLATFORM_ANDROID
	return true;
#else
	return false;
#endif
}

bool FViveOpenXRHMD::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR GetRequiredExtensions"));
	return true;
}

bool FViveOpenXRHMD::GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR GetInteractionProfile"));
	return true;
}

bool FViveOpenXRHMD::GetSpectatorScreenController(FHeadMountedDisplayBase* InHMDBase, TUniquePtr<FDefaultSpectatorScreenController>& OutSpectatorScreenController)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR GetSpectatorScreenController"));
#if PLATFORM_ANDROID
	OutSpectatorScreenController = nullptr;
	return true;
#else
	//OutSpectatorScreenController = MakeUnique<FDefaultSpectatorScreenController>(InHMDBase);
	return false;
#endif
}

void FViveOpenXRHMD::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR AttachActionSets"));
}

void FViveOpenXRHMD::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR GetActiveActionSetsForSync"));
}

void FViveOpenXRHMD::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnEvent"));
	return;
}

const void* FViveOpenXRHMD::OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnCreateInstance"));
	return InNext;
}

const void* FViveOpenXRHMD::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnGetSystem"));
	return InNext;
}

const void* FViveOpenXRHMD::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnCreateSession"));
#if PLATFORM_ANDROID
	if (GRHISupportsRHIThread && GIsThreadedRendering && GUseRHIThread_InternalUseOnly)
	{
		SetRHIThreadEnabled(false, false);
	}
#endif
	return InNext;
}

const void* FViveOpenXRHMD::OnBeginSession(XrSession InSession, const void* InNext)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnBeginSession"));
	return InNext;
}

void FViveOpenXRHMD::OnDestroySession(XrSession InSession)
{
	UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnDestroySession"));
}

const void* FViveOpenXRHMD::OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnBeginFrame"));
	return InNext;
}

const void* FViveOpenXRHMD::OnBeginProjectionView(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnBeginProjectionView"));
	return InNext;
}

const void* FViveOpenXRHMD::OnBeginDepthInfo(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnBeginDepthInfo"));
	return InNext;
}

const void* FViveOpenXRHMD::OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnEndProjectionLayer"));
	OutFlags |= XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	return InNext;
}

const void* FViveOpenXRHMD::OnEndFrame(XrSession InSession, XrTime DisplayTime, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnEndFrame"));
	return InNext;
}

const void* FViveOpenXRHMD::OnSyncActions(XrSession InSession, const void* InNext)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR OnSyncActions"));
	return InNext;
}

void FViveOpenXRHMD::PostSyncActions(XrSession InSession)
{
	//UE_LOG(LogViveOpenXRHMD, Log, TEXT("Vive OpenXR PostSyncActions"));
	return;
}

IMPLEMENT_MODULE(FViveOpenXRHMD, ViveOpenXRHMD)
