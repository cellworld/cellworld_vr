// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRPlaneDetectionModule.h"
#include "OpenXRCore.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "ARBlueprintLibrary.h"
#include "IOpenXRARModule.h"

DEFINE_LOG_CATEGORY(ViveOXRPlaneDetection);

FOpenXRHMD* FViveOpenXRPlaneDetection::hmd = nullptr;
FViveOpenXRPlaneDetection* FViveOpenXRPlaneDetection::instance = nullptr;
bool FViveOpenXRPlaneDetection::isPlaneDetectionSupported = false;

FOpenXRHMD* FViveOpenXRPlaneDetection::HMD() {
	if (hmd != nullptr)
		return hmd;
	if (GEngine->XRSystem.IsValid())
	{
		hmd = static_cast<FOpenXRHMD*>(GEngine->XRSystem->GetHMDDevice());
	}
	return hmd;
}

FViveOpenXRPlaneDetection* FViveOpenXRPlaneDetection::Instance()
{
	if (instance != nullptr)
	{
		return instance;
	}
	else
	{
		if (GEngine->XRSystem.IsValid() && HMD() != nullptr)
		{
			for (IOpenXRExtensionPlugin* Module : HMD()->GetExtensionPlugins())
			{
				if (Module->GetDisplayName() == TEXT("ViveOpenXRPlaneDetection"))
				{
					instance = static_cast<FViveOpenXRPlaneDetection*>(Module);
					break;
				}
			}
		}
		return instance;
	}
}


void FViveOpenXRPlaneDetection::StartupModule()
{
	check(GConfig && GConfig->IsReadyForUse());
	FString modeName;
	if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnablePlaneDetection"), modeName, GEngineIni))
	{
		if (modeName.Equals("False"))
		{
			m_bEnablePlaneDetection = false;
		}
		else if (modeName.Equals("True"))
		{
			m_bEnablePlaneDetection = true;
		}
	}

	if (m_bEnablePlaneDetection)
	{
		UE_LOG(ViveOXRPlaneDetection, Log, TEXT("Enable PlaneDetection."));
		instance = this;
	}
	else
	{
		UE_LOG(ViveOXRPlaneDetection, Log, TEXT("Disable PlaneDetection."));
		instance = nullptr;
		return;
	}

	RegisterOpenXRExtensionModularFeature();
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("StartupModule() Finished."));
}

void FViveOpenXRPlaneDetection::ShutdownModule()
{
	instance = nullptr;
	UnregisterOpenXRExtensionModularFeature();
}

FString FViveOpenXRPlaneDetection::GetDisplayName()
{
	return FString(TEXT("ViveOpenXRPlaneDetection"));
}

bool FViveOpenXRPlaneDetection::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (m_bEnablePlaneDetection)
	{
		UE_LOG(ViveOXRPlaneDetection, Log, TEXT("GetRequiredExtensions() Add PlaneDetection Extension Name %s."), ANSI_TO_TCHAR(XR_EXT_PLANE_DETECTION_EXTENSION_NAME));
		OutExtensions.Add(XR_EXT_PLANE_DETECTION_EXTENSION_NAME);
	}
	return true;
}

const void* FViveOpenXRPlaneDetection::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	if (!m_bEnablePlaneDetection) return InNext;

	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("Entry PlaneDetection OnCreateSession."));

	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreatePlaneDetectorEXT", (PFN_xrVoidFunction*)&xrCreatePlaneDetectorEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyPlaneDetectorEXT", (PFN_xrVoidFunction*)&xrDestroyPlaneDetectorEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrBeginPlaneDetectionEXT", (PFN_xrVoidFunction*)&xrBeginPlaneDetectionEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetPlaneDetectionStateEXT", (PFN_xrVoidFunction*)&xrGetPlaneDetectionStateEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetPlaneDetectionsEXT", (PFN_xrVoidFunction*)&xrGetPlaneDetectionsEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetPlanePolygonBufferEXT", (PFN_xrVoidFunction*)&xrGetPlanePolygonBufferEXT));

	XrSystemPlaneDetectionPropertiesEXT systemPlaneDetectionProperties = {};
	systemPlaneDetectionProperties.type = XR_TYPE_SYSTEM_PLANE_DETECTION_PROPERTIES_EXT;
	systemPlaneDetectionProperties.next = nullptr;
	XrSystemProperties systemProperties = {};
	systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	systemProperties.next = &systemPlaneDetectionProperties;
	XrResult result = xrGetSystemProperties(InInstance, InSystem, &systemProperties);
	if (XR_FAILED(result))
	{
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("OnCreateSession() xrGetSystemProperties failed with result %d."), result);
	}
	else
	{
		isPlaneDetectionSupported = (systemPlaneDetectionProperties.supportedFeatures & XR_PLANE_DETECTION_CAPABILITY_PLANE_DETECTION_BIT_EXT) > 0;
	}
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("OnCreateSession() Is PlaneDetection supported: %d."), isPlaneDetectionSupported);

	if (IOpenXRARModule::IsAvailable())
	{
		m_PlaneLocationGuids.Empty();
		m_TrackedMeshHolder = IOpenXRARModule::Get().GetTrackedMeshHolder();
		// Register application lifetime delegates
		FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &FViveOpenXRPlaneDetection::OnApplicationResume);
	}

	return InNext;
}

void FViveOpenXRPlaneDetection::PostCreateSession(XrSession InSession)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("Entry PlaneDetection PostCreateSession InSession %lu."), InSession);
	m_Session = InSession;
}

void FViveOpenXRPlaneDetection::OnStartARSession(class UARSessionConfig* SessionConfig)
{
	m_bHorizontalPlaneDetection = SessionConfig->ShouldDoHorizontalPlaneDetection();
	m_bVerticalPlaneDetection = SessionConfig->ShouldDoVerticalPlaneDetection();
}

void FViveOpenXRPlaneDetection::OnDestroySession(XrSession InSession)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("Entry PlaneDetection OnDestorySession."));

	if (m_bEnablePlaneDetection)
	{
		if (m_PlaneDetector != XR_NULL_HANDLE) {
			DestroyPlaneDetector(m_PlaneDetector);
		}
	}
}

IOpenXRCustomCaptureSupport* FViveOpenXRPlaneDetection::GetCustomCaptureSupport(const EARCaptureType CaptureType)
{
	//if (CaptureType == EARCaptureType::SceneUnderstanding) return this;
	return nullptr;
}

void FViveOpenXRPlaneDetection::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (!m_bEnablePlaneDetection || !isPlaneDetectionSupported) return;
	if (m_PlaneDetector == XR_NULL_HANDLE) {
		XrPlaneDetectorCreateInfoEXT createInfo{};
		createInfo.type = XR_TYPE_PLANE_DETECTOR_CREATE_INFO_EXT;
		createInfo.next = nullptr;
		createInfo.flags = XR_PLANE_DETECTOR_ENABLE_CONTOUR_BIT_EXT;
		XrPlaneDetectorEXT planeDetectorExt{};
		if (CreatePlaneDetector(&createInfo, &planeDetectorExt)) {
			m_PlaneDetector = planeDetectorExt;

			XrPlaneDetectorBeginInfoEXT beginInfoExt{};
			beginInfoExt.type = XR_TYPE_PLANE_DETECTOR_BEGIN_INFO_EXT;
			beginInfoExt.next = nullptr;
			beginInfoExt.baseSpace = TrackingSpace;
			beginInfoExt.time = DisplayTime;
			beginInfoExt.orientationCount = 4;
			const XrPlaneDetectorOrientationEXT orientations[4] = {
				XR_PLANE_DETECTOR_ORIENTATION_HORIZONTAL_UPWARD_EXT,
				XR_PLANE_DETECTOR_ORIENTATION_HORIZONTAL_DOWNWARD_EXT,
				XR_PLANE_DETECTOR_ORIENTATION_VERTICAL_EXT,
				XR_PLANE_DETECTOR_ORIENTATION_ARBITRARY_EXT,
			};
			beginInfoExt.orientationCount = sizeof(orientations);
			beginInfoExt.orientations = orientations;

			const XrPlaneDetectorSemanticTypeEXT semanticTypes[5] = {
				XR_PLANE_DETECTOR_SEMANTIC_TYPE_UNDEFINED_EXT,
				XR_PLANE_DETECTOR_SEMANTIC_TYPE_CEILING_EXT,
				XR_PLANE_DETECTOR_SEMANTIC_TYPE_FLOOR_EXT,
				XR_PLANE_DETECTOR_SEMANTIC_TYPE_WALL_EXT,
				XR_PLANE_DETECTOR_SEMANTIC_TYPE_PLATFORM_EXT,
			};
			beginInfoExt.semanticTypeCount = sizeof(semanticTypes);
			beginInfoExt.semanticTypes = semanticTypes;

			beginInfoExt.maxPlanes = 100;
			beginInfoExt.minArea = 0.01f;
			beginInfoExt.boundingBoxPose = { { 0, 0, 0, 1 }, { 0, 0, 0 } };
			beginInfoExt.boundingBoxExtent = { 100, 100, 100 };

			if (BeginPlaneDetection(m_PlaneDetector, &beginInfoExt)) {
				m_bPlaneDetectionBegin = true;
			}
		}
	}

	if (!m_bHorizontalPlaneDetection && !m_bVerticalPlaneDetection) return;

	if (m_bPlaneDetectionBegin) {
		XrPlaneDetectionStateEXT stateExt;
		if (GetPlaneDetectionState(m_PlaneDetector, &stateExt)) {
			EViveOpenXRPlaneDetectionState state = (EViveOpenXRPlaneDetectionState)stateExt;
			if (state == EViveOpenXRPlaneDetectionState::None) {
				//UE_LOG(ViveOXRPlaneDetection, Log, TEXT("PlaneDetection state is None"));
				DestroyPlaneDetector(m_PlaneDetector);
			}
			if (state == EViveOpenXRPlaneDetectionState::Pending) {
				//UE_LOG(ViveOXRPlaneDetection, Log, TEXT("PlaneDetection state is Pending"));
			}
			if (state == EViveOpenXRPlaneDetectionState::Done) {
				//UE_LOG(ViveOXRPlaneDetection, Log, TEXT("PlaneDetection state is Done"));				

				XrPlaneDetectorGetInfoEXT infoExt{};
				infoExt.type = XR_TYPE_PLANE_DETECTOR_GET_INFO_EXT;
				infoExt.next = nullptr;
				infoExt.baseSpace = TrackingSpace;
				infoExt.time = DisplayTime;

				XrPlaneDetectorLocationsEXT locationsExt{};
				locationsExt.type = XR_TYPE_PLANE_DETECTOR_LOCATIONS_EXT;
				locationsExt.next = nullptr;
				locationsExt.planeLocationCapacityInput = 0;
				locationsExt.planeLocationCountOutput = 0;
				locationsExt.planeLocations = nullptr;
				if (GetPlaneDetections(m_PlaneDetector, &infoExt, &locationsExt)) {
					if (locationsExt.planeLocationCountOutput != 0)
					{
						locationsExt.planeLocations = new XrPlaneDetectorLocationEXT[locationsExt.planeLocationCountOutput];
						locationsExt.planeLocationCapacityInput = locationsExt.planeLocationCountOutput;

						if (GetPlaneDetections(m_PlaneDetector, &infoExt, &locationsExt))
						{
							// Cache all planeLocations id to compare with AR cache.
							TArray<FGuid> cache_PlaneLocationGuids;
							cache_PlaneLocationGuids.SetNum((int)locationsExt.planeLocationCountOutput);

							// Attach all plane raw data to AR tracked plane.
							for (int i = 0; i < (int)locationsExt.planeLocationCountOutput; i++)
							{
								EARObjectClassification currentType;
								auto& locationExt = locationsExt.planeLocations[i];
								const FGuid& planeGuid = PlaneIdToFGuid(locationExt.planeId);

								cache_PlaneLocationGuids.Add(planeGuid);

								// If already set data for tracked plane will jump to next.
								if (m_PlaneLocationGuids.Contains(planeGuid))
									continue;

								if (planeGuid.IsValid())
								{
									m_TrackedMeshHolder->StartMeshUpdates();

									m_PlaneLocationGuids.Add(planeGuid);

									switch ((EViveOpenXRPlaneDetectorSemanticType)locationExt.semanticType)
									{
									case EViveOpenXRPlaneDetectorSemanticType::Undefined:
										currentType = EARObjectClassification::Unknown;
										break;
									case EViveOpenXRPlaneDetectorSemanticType::Ceiling:
										currentType = EARObjectClassification::Ceiling;
										if (!m_bHorizontalPlaneDetection)
										{
											continue;
										}
										break;
									case EViveOpenXRPlaneDetectorSemanticType::Floor:
										currentType = EARObjectClassification::Floor;
										if (!m_bHorizontalPlaneDetection)
										{
											continue;
										}
										break;
									case EViveOpenXRPlaneDetectorSemanticType::Wall:
										currentType = EARObjectClassification::Wall;
										if (!m_bVerticalPlaneDetection)
										{
											continue;
										}
										break;
									case EViveOpenXRPlaneDetectorSemanticType::Platform:
										currentType = EARObjectClassification::Table;
										if (!m_bHorizontalPlaneDetection)
										{
											continue;
										}
										break;
									default:
											currentType = EARObjectClassification::NotApplicable;
										break;
									}

									FOpenXRPlaneUpdate* planeUpdate = m_TrackedMeshHolder->AllocatePlaneUpdate(planeGuid);

									auto& rot = locationExt.pose.orientation;
									auto& pos = locationExt.pose.position;
									auto& extents = locationExt.extents;
									float w2m = HMD()->GetWorldToMetersScale();

									planeUpdate->Type = currentType;

									planeUpdate->Extent = FVector(extents.height * w2m / 2, extents.width * w2m / 2,  0);

									auto forwardRot = FQuat(FRotator(180, 0, 0));
									planeUpdate->LocalToTrackingTransform = FTransform(ToFQuat(rot) * forwardRot, ToFVector(pos, w2m));

									planeUpdate->SpatialMeshUsageFlags = (EARSpatialMeshUsageFlags)((int32)EARSpatialMeshUsageFlags::Visible |
										(int32)EARSpatialMeshUsageFlags::Collision);

									m_TrackedMeshHolder->EndMeshUpdates();
								}
							}

							// Check and clean AR tracked plane cache.
							TArray<FGuid> removeGroup;
							for (int i = 0; i < m_PlaneLocationGuids.Num(); i++)
							{
								if (cache_PlaneLocationGuids.Contains(m_PlaneLocationGuids[i]))
									continue;
								else
								{
									removeGroup.Add(m_PlaneLocationGuids[i]);
									m_TrackedMeshHolder->RemovePlane(m_PlaneLocationGuids[i]);
								}
							}

							for (int i = 0; i < removeGroup.Num(); i++)
							{
								m_PlaneLocationGuids.Remove(removeGroup[i]);
							}
						}
					}
				}
				delete[] locationsExt.planeLocations;
				m_bPlaneDetectionBegin = false;
			}
			if (state == EViveOpenXRPlaneDetectionState::Error) {
				//UE_LOG(ViveOXRPlaneDetection, Log, TEXT("PlaneDetection state is Error"));
				DestroyPlaneDetector(m_PlaneDetector);
			}
			if (state == EViveOpenXRPlaneDetectionState::Fatal) {
				//UE_LOG(ViveOXRPlaneDetection, Log, TEXT("PlaneDetection state is Fatal"));
				DestroyPlaneDetector(m_PlaneDetector);
			}
		}
	}
}

// This will be called every frame.
const void* FViveOpenXRPlaneDetection::OnSyncActions(XrSession InSession, const void* InNext)
{
	return InNext;
}

bool FViveOpenXRPlaneDetection::CreatePlaneDetector(const XrPlaneDetectorCreateInfoEXT* createInfo, XrPlaneDetectorEXT* planeDetector)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("CreatePlaneDetector"));
	if (!m_bEnablePlaneDetection || xrCreatePlaneDetectorEXT == nullptr || !isPlaneDetectionSupported) return false;
	XrResult result = xrCreatePlaneDetectorEXT(m_Session, createInfo, planeDetector);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("xrCreatePlaneDetectorEXT failed with error code %d."), result);
		return false;
	}
	return true;

}

bool FViveOpenXRPlaneDetection::DestroyPlaneDetector(XrPlaneDetectorEXT planeDetector)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("DestroyPlaneDetector"));
	if (!m_bEnablePlaneDetection || xrDestroyPlaneDetectorEXT == nullptr) return false;	
	XrResult result = xrDestroyPlaneDetectorEXT(planeDetector);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("DestroyPlaneDetector failed with error code %d."), result);
		return false;
	}
	return true;
}

bool FViveOpenXRPlaneDetection::BeginPlaneDetection(XrPlaneDetectorEXT planeDetector, const XrPlaneDetectorBeginInfoEXT* beginInfo)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("BeginPlaneDetection"));
	if (!m_bEnablePlaneDetection || xrBeginPlaneDetectionEXT == nullptr) return false;
	XrResult result = xrBeginPlaneDetectionEXT(planeDetector, beginInfo);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("BeginPlaneDetection failed with error code %d."), result);
		return false;
	}
	return true;
}

bool FViveOpenXRPlaneDetection::GetPlaneDetectionState(XrPlaneDetectorEXT planeDetector, XrPlaneDetectionStateEXT* state)
{
	if (!m_bEnablePlaneDetection || xrGetPlaneDetectionStateEXT == nullptr) return false;
	XrResult result = xrGetPlaneDetectionStateEXT(planeDetector, state);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("GetPlaneDetectionState failed with error code %d."), result);
		return false;
	}
	return true;

}

bool FViveOpenXRPlaneDetection::GetPlaneDetections(XrPlaneDetectorEXT planeDetector, const XrPlaneDetectorGetInfoEXT* info, XrPlaneDetectorLocationsEXT* locations)
{
	UE_LOG(ViveOXRPlaneDetection, Log, TEXT("GetPlaneDetections"));
	if (!m_bEnablePlaneDetection || xrGetPlaneDetectionsEXT == nullptr) return false;
	XrResult result = xrGetPlaneDetectionsEXT(planeDetector, info, locations);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("GetPlaneDetections failed with error code %d."), result);
		return false;
	}
	return true;
}

bool FViveOpenXRPlaneDetection::GetPlanePolygonBuffer(XrPlaneDetectorEXT planeDetector, uint64_t planeId, uint32_t polygonBufferIndex, XrPlaneDetectorPolygonBufferEXT* polygonBuffer)
{
	if (!m_bEnablePlaneDetection || xrGetPlanePolygonBufferEXT == nullptr) return false;
	polygonBuffer->type = XR_TYPE_PLANE_DETECTOR_POLYGON_BUFFER_EXT;
	polygonBuffer->next = nullptr;
	polygonBuffer->vertexCapacityInput = 0;
	polygonBuffer->vertexCountOutput = 0;
	polygonBuffer->vertices = nullptr;
	XrResult result = xrGetPlanePolygonBufferEXT(planeDetector, planeId, polygonBufferIndex, polygonBuffer);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("GetPlanePolygonBuffer 1 failed with error code %d."), result);
		return false;
	}
	if (polygonBuffer->vertexCountOutput == 0) return true;

	polygonBuffer->vertices = new XrVector2f[polygonBuffer->vertexCountOutput];
	polygonBuffer->vertexCapacityInput = polygonBuffer->vertexCountOutput;
	result = xrGetPlanePolygonBufferEXT(planeDetector, planeId, polygonBufferIndex, polygonBuffer);
	if (XR_FAILED(result)) {
		UE_LOG(ViveOXRPlaneDetection, Error, TEXT("GetPlanePolygonBuffer 2 failed with error code %d."), result);
		return false;
	}

	return true;
}

void FViveOpenXRPlaneDetection::OnApplicationResume()
{
	// Reset plane detector to refesh plane data if any adjustment from background
	if (m_PlaneDetector != XR_NULL_HANDLE) {
		if (DestroyPlaneDetector(m_PlaneDetector)) {
			m_PlaneDetector = XR_NULL_HANDLE;
		}
	}
}

IMPLEMENT_MODULE(FViveOpenXRPlaneDetection, ViveOpenXRPlaneDetection)