// Copyright HTC Corporation. All Rights Reserved.

#include "ViveOpenXRSceneUnderstanding.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Async/Async.h"
#include "ViveOpenXRRuntimeSettings.h"



class IOpenXRARTrackedMeshHolder;

namespace ViveOpenXR
{
	SharedOpenXRScene::SharedOpenXRScene(FSceneUnderstandingExtesionDispatchTable SU_ext, XrSceneMSFT scene)
		:SU_ext(SU_ext)
		, m_Scene(scene)
	{
	}

	SharedOpenXRScene::~SharedOpenXRScene()
	{
		if (SU_ext.xrDestroySceneMSFT != nullptr && m_Scene != nullptr)
		{
			XR_ENSURE(SU_ext.xrDestroySceneMSFT(m_Scene));
		}
	}

	FSceneUnderstanding::FSceneUnderstanding()
	{
		m_SceneComputeFeatures.AddUnique(XrSceneComputeFeatureMSFT::XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT);
		m_ReferenceSpaceType = XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_STAGE;
		m_SceneComputeConsistency = XrSceneComputeConsistencyMSFT::XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_INCOMPLETE_FAST_MSFT;
		m_MeshComputeLod = XrMeshComputeLodMSFT::XR_MESH_COMPUTE_LOD_COARSE_MSFT;
	}

	FSceneUnderstanding::~FSceneUnderstanding()
	{
	}

	void FSceneUnderstanding::Register()
	{
		UE_LOG(LogViveOpenXR, Log, TEXT("Entry SceneUnderstanding Register."));

		RegisterOpenXRExtensionModularFeature();
		check(GConfig && GConfig->IsReadyForUse());
		FString modeName;
		if (GConfig->GetString(TEXT("/Script/ViveOpenXRRuntimeSettings.ViveOpenXRRuntimeSettings"), TEXT("bEnableSceneUnderstanding"), modeName, GEngineIni))
		{
			if (modeName.Equals("False"))
			{
				m_bEnableSceneUnderstanding = false;
			}
			else if (modeName.Equals("True"))
			{
				m_bEnableSceneUnderstanding = true;
			}
		}

#if PLATFORM_ANDROID
		if (m_bEnableSceneUnderstanding)
		{
			m_bEnableSceneUnderstanding = false;
			UE_LOG(LogViveOpenXR, Warning, TEXT("Vive OpenXR Scene Understanding is not supported on Vive AIO devices."));
		}
#endif

		if (m_bEnableSceneUnderstanding)
		{
			UE_LOG(LogViveOpenXR, Log, TEXT("Enable Scene Understanding."));
		}
		else
		{
			UE_LOG(LogViveOpenXR, Log, TEXT("Disable Scene Understanding."));
		}
	}

	void FSceneUnderstanding::Unregister()
	{
		Stop();
		UnregisterOpenXRExtensionModularFeature();
	}

	const void* FSceneUnderstanding::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
	{
		if (!m_bEnableSceneUnderstanding) return InNext;

		UE_LOG(LogViveOpenXR, Log, TEXT("Entry SU OnCreateSession."));

		// Get XrSpace related function pointers.
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateReferenceSpaces", (PFN_xrVoidFunction*)&Space_ext.xrEnumerateReferenceSpaces));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateReferenceSpace", (PFN_xrVoidFunction*)&Space_ext.xrCreateReferenceSpace));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroySpace", (PFN_xrVoidFunction*)&Space_ext.xrDestroySpace));

		// Get XR_MSFT_scene_understanding function pointers.
		XR_ENSURE(xrGetInstanceProcAddr(
			InInstance, "xrEnumerateSceneComputeFeaturesMSFT", (PFN_xrVoidFunction*)&SU_ext.xrEnumerateSceneComputeFeaturesMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSceneObserverMSFT", (PFN_xrVoidFunction*)&SU_ext.xrCreateSceneObserverMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroySceneObserverMSFT", (PFN_xrVoidFunction*)&SU_ext.xrDestroySceneObserverMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSceneMSFT", (PFN_xrVoidFunction*)&SU_ext.xrCreateSceneMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroySceneMSFT", (PFN_xrVoidFunction*)&SU_ext.xrDestroySceneMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrComputeNewSceneMSFT", (PFN_xrVoidFunction*)&SU_ext.xrComputeNewSceneMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSceneComputeStateMSFT", (PFN_xrVoidFunction*)&SU_ext.xrGetSceneComputeStateMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSceneComponentsMSFT", (PFN_xrVoidFunction*)&SU_ext.xrGetSceneComponentsMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrLocateSceneComponentsMSFT", (PFN_xrVoidFunction*)&SU_ext.xrLocateSceneComponentsMSFT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSceneMeshBuffersMSFT", (PFN_xrVoidFunction*)&SU_ext.xrGetSceneMeshBuffersMSFT));

		// Reset scene computation bounds.
		m_SceneSphereBounds.Empty();
		m_SceneOrientedBoxBounds.Empty();
		m_SceneFrustumBounds.Empty();
		//m_MeshComputeLod = {};

		return InNext;
	}

	const void* FSceneUnderstanding::OnBeginSession(XrSession InSession, const void* InNext)
	{
		if (!m_bEnableSceneUnderstanding) return InNext;

		UE_LOG(LogViveOpenXR, Log, TEXT("Entry SceneUnderstanding OnBeginSession."));

		m_Session = InSession;
		static FName SystemName(TEXT("OpenXR"));
		if (!GEngine->XRSystem.IsValid() || (GEngine->XRSystem->GetSystemName() != SystemName))
		{
			return InNext;
		}
		XRTrackingSystem = GEngine->XRSystem.Get();
		if (IOpenXRARModule::IsAvailable())
		{
			m_TrackedMeshHolder = IOpenXRARModule::Get().GetTrackedMeshHolder();
		}

		// Enumerate supported referemce space types.
		TArray<XrReferenceSpaceType> supportedReferenceSpaceTypes;
		uint32 supportedReferenceSpaceTypeCount = 0;
		XR_ENSURE(Space_ext.xrEnumerateReferenceSpaces(InSession, 0, &supportedReferenceSpaceTypeCount, supportedReferenceSpaceTypes.GetData()));
		supportedReferenceSpaceTypes.SetNum(supportedReferenceSpaceTypeCount);
		XR_ENSURE(Space_ext.xrEnumerateReferenceSpaces(InSession, (uint32)supportedReferenceSpaceTypes.Num(), &supportedReferenceSpaceTypeCount, supportedReferenceSpaceTypes.GetData()));

		// Get a supported reference space type. Prefer the stage space type.
		for (auto&& type : supportedReferenceSpaceTypes)
		{
			m_ReferenceSpaceType = type;
			if (type == XrReferenceSpaceType::XR_REFERENCE_SPACE_TYPE_STAGE)
			{
				break;
			}
		}

		// Create a reference space.
		XrReferenceSpaceCreateInfo referenceSpaceCreateInfo;
		referenceSpaceCreateInfo.type = XrStructureType::XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
		referenceSpaceCreateInfo.next = NULL;
		referenceSpaceCreateInfo.referenceSpaceType = m_ReferenceSpaceType;
		referenceSpaceCreateInfo.poseInReferenceSpace.orientation = { 0, 0, 0, 1 };
		referenceSpaceCreateInfo.poseInReferenceSpace.position = { 0, 0, 0 };
		XR_ENSURE(Space_ext.xrCreateReferenceSpace(InSession, &referenceSpaceCreateInfo, &m_XrSpace));

		m_bOpenXRReady = true;
		
		return InNext;
	}

	bool FSceneUnderstanding::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		UE_LOG(LogViveOpenXR, Log, TEXT("Entry SceneUnderstanding GetRequiredExtensions."));
		if (m_bEnableSceneUnderstanding)
		{
			UE_LOG(LogViveOpenXR, Log, TEXT("Add SceneUnderstanding Extension Name."));
			OutExtensions.Add(XR_MSFT_SCENE_UNDERSTANDING_EXTENSION_NAME);
		}
		
		return true;
	}

	void FSceneUnderstanding::ComputeNewScene(XrTime displayTime)
	{
		// Compute a new scene.
		XrVisualMeshComputeLodInfoMSFT visualMeshComputeLodInfo;
		visualMeshComputeLodInfo.type = XrStructureType::XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT;
		visualMeshComputeLodInfo.next = NULL;
		visualMeshComputeLodInfo.lod = m_MeshComputeLod;
		TArray<XrSceneComputeFeatureMSFT> sceneComputeFeatures = { XrSceneComputeFeatureMSFT::XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT };
		XrNewSceneComputeInfoMSFT newSceneComputeInfo;
		newSceneComputeInfo.type = XrStructureType::XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT;
		newSceneComputeInfo.next = &visualMeshComputeLodInfo;
		newSceneComputeInfo.requestedFeatureCount = (uint32_t)sceneComputeFeatures.Num();
		newSceneComputeInfo.requestedFeatures = sceneComputeFeatures.GetData();
		newSceneComputeInfo.consistency = m_SceneComputeConsistency;
		newSceneComputeInfo.bounds.space = m_XrSpace;
		newSceneComputeInfo.bounds.time = displayTime;
		newSceneComputeInfo.bounds.sphereCount = (uint32_t)m_SceneSphereBounds.Num();
		newSceneComputeInfo.bounds.spheres = m_SceneSphereBounds.GetData();
		newSceneComputeInfo.bounds.boxCount = (uint32_t)m_SceneOrientedBoxBounds.Num();
		newSceneComputeInfo.bounds.boxes = m_SceneOrientedBoxBounds.GetData();
		newSceneComputeInfo.bounds.frustumCount = (uint32_t)m_SceneFrustumBounds.Num();
		newSceneComputeInfo.bounds.frustums = m_SceneFrustumBounds.GetData();

		XR_ENSURE(SU_ext.xrComputeNewSceneMSFT(m_SceneObserver, &newSceneComputeInfo));
	}

	void FSceneUnderstanding::LocateObjects(XrSceneMSFT scene, XrSpace BaseSpace, XrTime Time, const TArray<XrUuidMSFT>& Identifiers, TArray<XrSceneComponentLocationMSFT>& Locations)
	{
		XrSceneComponentsLocateInfoMSFT sceneComponentLocateInfo{ XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT };
		sceneComponentLocateInfo.baseSpace = BaseSpace;
		sceneComponentLocateInfo.time = Time;
		sceneComponentLocateInfo.componentIdCount = static_cast<uint32_t>(Identifiers.Num());
		sceneComponentLocateInfo.componentIds = Identifiers.GetData();
		bool isEmpty = (Locations.Num() == 0) ? true : false;
		Locations.SetNumZeroed(Identifiers.Num());

		XrSceneComponentLocationsMSFT sceneVisualMeshComponentLocations{ XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT };
		sceneVisualMeshComponentLocations.locationCount = static_cast<uint32_t>(Locations.Num());
		sceneVisualMeshComponentLocations.locations = Locations.GetData();

		XR_ENSURE(SU_ext.xrLocateSceneComponentsMSFT(scene, &sceneComponentLocateInfo, &sceneVisualMeshComponentLocations));
	}

	void FSceneUnderstanding::ProcessMeshDataUpdate(FMeshDataUpdate&& meshDataUpdate, XrTime displayTime, XrSpace trackingSpace)
	{
		m_MeshCollisionInfo = MoveTemp(meshDataUpdate.meshCollisionInfo);

		TArray<XrUuidMSFT> meshUuids;
		meshDataUpdate.meshes.GetKeys(meshUuids);
		if (meshUuids.Num() == 0)
		{
			UE_LOG(LogViveOpenXR, Warning, TEXT("ProcessMeshDataUpdate_meshUuids.Num(): %d"), meshUuids.Num());
			return;
		}
		LocateObjects(meshDataUpdate.m_SharedOpenXRScene->GetScene(), trackingSpace, displayTime, meshUuids, m_Locations);

		m_TrackedMeshHolder->StartMeshUpdates();
		for (const auto& Elem : m_PreviousMeshes)
		{
			const XrUuidMSFT& meshUuid = Elem.Key;
			if (!meshDataUpdate.meshes.Contains(meshUuid))
			{
				const FGuid& meshGuid = Elem.Value.meshGuid;
				if (meshGuid.IsValid())
				{
					m_TrackedMeshHolder->RemoveMesh(meshGuid);
				}
			}
		}
		m_TrackedMeshHolder->EndMeshUpdates();
		m_PreviousMeshes.Empty();

		for (const auto& Elem : meshDataUpdate.meshes)
		{
			FMeshInfo meshInfo;
			meshInfo.meshGuid = Elem.Value.meshGuid;
			m_PreviousMeshes.Add(Elem.Key, meshInfo);
		}

		if (m_SharedScene.IsValid())
		{
			m_SharedScene.Reset();
		}

		m_SharedScene = MoveTemp(meshDataUpdate.m_SharedOpenXRScene);
		m_Meshes = MoveTemp(meshDataUpdate.meshes);
	}

	void FSceneUnderstanding::UpdateMeshesLocations(XrTime time, XrSpace space)
	{
		if (m_ScanState != EMeshesScanState::Locating)
		{
			return;
		}
		if (m_Meshes.Num() == 0 || m_SharedScene == nullptr)
		{
			m_ScanState = EMeshesScanState::Idle;
			return;
		}
		const float worldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

		TArray<XrUuidMSFT> meshesUuid;
		m_Meshes.GetKeys(meshesUuid);

		XrUuidMSFT uuid = meshesUuid[m_LocateCurrentFrame];
		XrSceneComponentLocationMSFT location = m_Locations[m_LocateCurrentFrame];
		m_TrackedMeshHolder->StartMeshUpdates();

		for (int i = 0; i < m_NumOfMeshToDrawPerFrame; i++)
		{
			const FMeshInfo* meshInfo = m_PreviousMeshes.Find(uuid);
			if (meshInfo != nullptr)
			{
				auto meshUpdate = MakeShared<FOpenXRMeshUpdate>();
				const FGuid& meshGuid = meshInfo->meshGuid;
				meshUpdate->Id = meshGuid;
				meshUpdate->SpatialMeshUsageFlags = (EARSpatialMeshUsageFlags)((int32)EARSpatialMeshUsageFlags::Visible |
					(int32)EARSpatialMeshUsageFlags::Collision);
				if (IsPoseValid(location.flags))
				{
					meshUpdate->TrackingState = EARTrackingState::Tracking;
					meshUpdate->LocalToTrackingTransform = ToFTransform(location.pose, worldToMetersScale);
				}
				else
				{
					meshUpdate->TrackingState = EARTrackingState::NotTracking;
					meshUpdate->LocalToTrackingTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
				}
				m_TrackedMeshHolder->ObjectUpdated(MoveTemp(meshUpdate));
			}
			m_LocateCurrentFrame++;
			if (m_LocateCurrentFrame >= m_Meshes.Num())
			{
				m_LocateCurrentFrame = 0;
				m_ScanState = EMeshesScanState::Idle;
				break;
			}
		}

		m_TrackedMeshHolder->EndMeshUpdates();
	}

	TSharedPtr<FMeshDataUpdate> UpdateMeshes(const FSceneUnderstandingExtesionDispatchTable& su_ext, XrSceneMSFT scene, TMap<XrUuidMSFT, FMeshInfo>&& previousMeshes, float worldToMeterScale)
	{
		// Create a shared scene to be stored in mesh data.
		auto sharedMeshData = MakeShared<FMeshDataUpdate>();
		auto& meshesMap = sharedMeshData->meshes;
		auto& meshCollisionInfo = sharedMeshData->meshCollisionInfo;

		// Create a shared scene to be stored in mesh data.
		TSharedPtr<SharedOpenXRScene> sharedScene = MakeShareable(new SharedOpenXRScene(su_ext, scene));

		// Stage 1: Get scene visual mesh components.
		XrSceneComponentsGetInfoMSFT sceneComponentsGetInfo;
		sceneComponentsGetInfo.type = XrStructureType::XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT;
		sceneComponentsGetInfo.next = NULL;
		sceneComponentsGetInfo.componentType = XrSceneComponentTypeMSFT::XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT;
		// First get the buffer capacity.
		XrSceneComponentsMSFT sceneComponents;
		sceneComponents.type = XrStructureType::XR_TYPE_SCENE_COMPONENTS_MSFT;
		sceneComponents.next = NULL;
		sceneComponents.componentCapacityInput = 0;
		sceneComponents.components = NULL;
		XR_ENSURE(su_ext.xrGetSceneComponentsMSFT(scene, &sceneComponentsGetInfo, &sceneComponents));
		// Create scene components by the provided capacity.
		const uint32_t componentCount = sceneComponents.componentCountOutput;
		TArray<XrSceneComponentMSFT> components;
		components.SetNum(componentCount);
		sceneComponents.componentCapacityInput = sceneComponents.componentCountOutput;
		sceneComponents.components = components.GetData();
		// Also add an instance int the structure chain for getting scene visual mesh components.
		XrSceneMeshesMSFT sceneMeshes;
		TArray<XrSceneMeshMSFT> meshes;
		meshes.SetNum(componentCount);
		sceneMeshes.type = XrStructureType::XR_TYPE_SCENE_MESHES_MSFT;
		sceneMeshes.next = NULL;
		sceneMeshes.sceneMeshCount = sceneComponents.componentCountOutput;
		sceneMeshes.sceneMeshes = meshes.GetData();
		sceneComponents.next = &sceneMeshes;
		// Call xrGetSceneComponentsMSFT() again to fill out the scene components and scene visual mesh components.
		XR_ENSURE(su_ext.xrGetSceneComponentsMSFT(scene, &sceneComponentsGetInfo, &sceneComponents));

		const int32_t count = components.Num();
		for (int32_t componentIndex = 0; componentIndex < count; componentIndex++)
		{
			const XrSceneComponentMSFT& sceneComponent = components[componentIndex];
			const XrSceneMeshMSFT& sceneMesh = meshes[componentIndex];

			XrUuidMSFT meshId;
			FMemory::Memcpy(&meshId, &sceneComponent.id, sizeof(XrUuidMSFT));

			FMeshInfo meshInfo;
			meshInfo.meshId = meshId;
			uint64_t meshBufferId = 0;
			meshBufferId = sceneMesh.meshBufferId;

			FGuid meshGuid{};
			if (meshBufferId != 0)
			{
				const auto* previousMeshData = previousMeshes.Find(meshId);
				if (previousMeshData != nullptr && previousMeshData->meshGuid.IsValid())
				{
					meshGuid = previousMeshData->meshGuid;
				}
				else
				{
					meshGuid = FGuid::NewGuid();
				}
			}

			FMeshUpdate& meshUpdate = meshesMap.Add(meshId);
			meshUpdate.meshGuid = meshGuid;

			// GetMeshBuffers
			if (meshBufferId != 0)
			{
				XrSceneMeshBuffersGetInfoMSFT sceneMeshBuffersGetInfo;
				sceneMeshBuffersGetInfo.type = XrStructureType::XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT;
				sceneMeshBuffersGetInfo.next = NULL;
				sceneMeshBuffersGetInfo.meshBufferId = meshBufferId;

				// Create buffers on the structure chain of XrSceneMeshBuffersMSFT.
				// Set input capacity to zero to get buffer capacity.
				XrSceneMeshBuffersMSFT sceneMeshBuffers;
				sceneMeshBuffers.type = XrStructureType::XR_TYPE_SCENE_MESH_BUFFERS_MSFT;
				XrSceneMeshVertexBufferMSFT sceneMeshVerticesBuffer;
				sceneMeshVerticesBuffer.type = XrStructureType::XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT;
				sceneMeshVerticesBuffer.vertexCapacityInput = 0;
				sceneMeshVerticesBuffer.vertices = NULL;
				XrSceneMeshIndicesUint32MSFT sceneMeshIndicesUint32Buffer;
				sceneMeshIndicesUint32Buffer.type = XrStructureType::XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT;
				sceneMeshIndicesUint32Buffer.indexCapacityInput = 0;
				sceneMeshIndicesUint32Buffer.indices = NULL;
				// Chain the structure instances.
				sceneMeshBuffers.next = &sceneMeshVerticesBuffer;
				sceneMeshVerticesBuffer.next = &sceneMeshIndicesUint32Buffer;
				sceneMeshIndicesUint32Buffer.next = NULL;
				// Call xrGetSceneMeshBuffersMSFT() to get buffer capacity.
				XR_ENSURE(su_ext.xrGetSceneMeshBuffersMSFT(sharedScene->GetScene(), &sceneMeshBuffersGetInfo, &sceneMeshBuffers));

				// Create buffers by the capacity.
				const uint32_t vertexCount = sceneMeshVerticesBuffer.vertexCountOutput;
				TArray<XrVector3f> vertices;
				vertices.SetNum(vertexCount);
				const uint32_t indexCount = sceneMeshIndicesUint32Buffer.indexCountOutput;
				meshUpdate.indices.SetNum(indexCount);
				sceneMeshVerticesBuffer.vertexCapacityInput = sceneMeshVerticesBuffer.vertexCountOutput;
				sceneMeshVerticesBuffer.vertices = vertices.GetData();
				sceneMeshIndicesUint32Buffer.indexCapacityInput = sceneMeshIndicesUint32Buffer.indexCountOutput;
				sceneMeshIndicesUint32Buffer.indices = meshUpdate.indices.GetData();
				// Call xrGetSceneMeshBuffersMSFT() again to fill out buffers.
				XR_ENSURE(su_ext.xrGetSceneMeshBuffersMSFT(sharedScene->GetScene(), &sceneMeshBuffersGetInfo, &sceneMeshBuffers));
				vertices.SetNum(vertexCount);
				meshUpdate.indices.SetNum(indexCount);

				meshUpdate.vertices.SetNum(vertexCount);
				for (int32 i = 0; i < vertices.Num(); i++)
				{
					meshUpdate.vertices[i] = FVector(-vertices[i].z * worldToMeterScale, vertices[i].x * worldToMeterScale, vertices[i].y * worldToMeterScale);
				}

				for (size_t i = 0; i < indexCount; i += 3)
				{
					// Swap the second and the third index in a triangle.
					Swap(meshUpdate.indices[i + 1], meshUpdate.indices[i + 2]);
				}
				meshCollisionInfo.Add(meshGuid, TrackedMeshCollision(meshUpdate.vertices, meshUpdate.indices));
			}
		}
		// Store the shared scene in order to manage the destruction of scene.
		sharedMeshData->m_SharedOpenXRScene = MoveTemp(sharedScene);
		return sharedMeshData;
	}

	void FSceneUnderstanding::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
	{
		if (m_bEnableSceneUnderstanding && !m_bOpenXRReady)
		{
			UE_LOG(LogViveOpenXR, Error, TEXT("is EnableSceneUnderstanded: %s"), (m_bEnableSceneUnderstanding ? TEXT("true") : TEXT("false")));
			UE_LOG(LogViveOpenXR, Error, TEXT("is OpenXRReady: %s"), (m_bOpenXRReady ? TEXT("true") : TEXT("false")));
			return;
		}

		if (m_SceneObserver == XR_NULL_HANDLE && m_TrackedMeshHolder != nullptr)
		{
			// Create a scene observer.
			XrSceneObserverCreateInfoMSFT sceneObserverCreateInfo;
			sceneObserverCreateInfo.type = XrStructureType::XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT;
			sceneObserverCreateInfo.next = NULL;

			XR_ENSURE(SU_ext.xrCreateSceneObserverMSFT(InSession, &sceneObserverCreateInfo, &m_SceneObserver));
		}
		if (!m_bEnableSceneUnderstanding) return;
		if (m_SceneObserver == XR_NULL_HANDLE || XRTrackingSystem == nullptr || m_TrackedMeshHolder == nullptr)
		{
			UE_LOG(LogViveOpenXR, Error, TEXT("is SceneObserver null: %s"), (m_SceneObserver == XR_NULL_HANDLE ? TEXT("true") : TEXT("fasle")));
			UE_LOG(LogViveOpenXR, Error, TEXT("is XRTrackingSystem null: %s"), (XRTrackingSystem == nullptr ? TEXT("true") : TEXT("false")));
			UE_LOG(LogViveOpenXR, Error, TEXT("is TrackedMeshHolder null: %s"), (XRTrackingSystem == nullptr ? TEXT("true") : TEXT("false")));
			return;
		}
		// Check the scene compute state.
		XrSceneComputeStateMSFT computeState;
		XR_ENSURE(SU_ext.xrGetSceneComputeStateMSFT(m_SceneObserver, &computeState));

		if (m_ScanState == EMeshesScanState::Idle)
		{
			if (m_bEnableSceneUnderstanding)
			{
				ComputeNewScene(DisplayTime);
				m_ScanState = EMeshesScanState::Waiting;
			}
			else
			{
				// Do locate if Scene Understand has been stopped and have existing meshes.
				if (m_Meshes.Num() != 0 && m_SharedScene == nullptr)
				{
					TArray<XrUuidMSFT> meshUuids;
					m_Meshes.GetKeys(meshUuids);
					LocateObjects(m_SharedScene->GetScene(), TrackingSpace, DisplayTime, meshUuids, m_Locations);
					m_ScanState = EMeshesScanState::Locating;
				}
			}
		}
		else if (m_ScanState == EMeshesScanState::Waiting)
		{
			switch (computeState)
			{
			case XrSceneComputeStateMSFT::XR_SCENE_COMPUTE_STATE_NONE_MSFT:
				// Compute a new scene at the end of the function.
				m_ScanState = EMeshesScanState::Idle;
				break;
			case XrSceneComputeStateMSFT::XR_SCENE_COMPUTE_STATE_UPDATING_MSFT:
				// Wait for scene computation.
				m_ScanState = EMeshesScanState::Waiting;
				break;
			case XrSceneComputeStateMSFT::XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT:
			{
				// Compute a new scene at the end of the function.

				// Create a scene of the computation
				XrSceneCreateInfoMSFT sceneCreateInfo;
				sceneCreateInfo.type = XrStructureType::XR_TYPE_SCENE_CREATE_INFO_MSFT;
				sceneCreateInfo.next = NULL;
				XrSceneMSFT scene;
				XR_ENSURE(SU_ext.xrCreateSceneMSFT(m_SceneObserver, &sceneCreateInfo, &scene));

				TPromise<TSharedPtr<FMeshDataUpdate>> Promise;
				m_ScanedMeshFuture = Promise.GetFuture();
				AsyncTask(ENamedThreads::AnyThread,
					[su_ext = SU_ext, sceneObserver = m_SceneObserver, scene = MoveTemp(scene), previousMeshes = m_PreviousMeshes,
					promise = MoveTemp(Promise), worldToMetersScale = XRTrackingSystem->GetWorldToMetersScale()]() mutable {
					promise.SetValue(UpdateMeshes(su_ext, MoveTemp(scene), MoveTemp(previousMeshes), worldToMetersScale));
				});
				m_ScanState = EMeshesScanState::Updating;
				break;
			}
			case XrSceneComputeStateMSFT::XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT:
				// Compute a new scene at the end of the function.
				m_ScanState = EMeshesScanState::Idle;
				break;
			default:
				// Compute a new scene at the end of the function.
				m_ScanState = EMeshesScanState::Idle;
				break;
			}
		}
		else if (m_ScanState == EMeshesScanState::Updating)
		{
			if (m_ScanedMeshFuture.IsReady())
			{
				ProcessMeshDataUpdate(MoveTemp(*m_ScanedMeshFuture.Get()), DisplayTime, TrackingSpace);
				m_ScanedMeshFuture.Reset();
				m_ScanState = EMeshesScanState::AddMeshesToTrackedMeshes;
			}
		}
		else if (m_ScanState == EMeshesScanState::AddMeshesToTrackedMeshes)
		{
			if (m_SharedScene == nullptr || m_Meshes.Num() == 0)
			{
				m_ScanState = EMeshesScanState::Idle;
				return;
			}

			const float worldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
			m_TrackedMeshHolder->StartMeshUpdates();
			TArray<XrUuidMSFT> meshesUuid;
			m_Meshes.GetKeys(meshesUuid);

			for (int i = 0; i < m_NumOfMeshToDrawPerFrame; i++)
			{
				const XrUuidMSFT& planeUuid = meshesUuid[m_LocateCurrentFrame];
				const FGuid planeGuid = XrUuidMSFTToFGuid(planeUuid);
				FMeshUpdate& mesh = m_Meshes.FindChecked(planeUuid);
				const FGuid& meshGuid = mesh.meshGuid;
				const auto& location = m_Locations[m_LocateCurrentFrame];
				TArray<FVector3f> meshVertices;
				meshVertices = FVectorDoubleToFVectorFloat(mesh.vertices);

				if (meshGuid.IsValid())
				{
					FOpenXRMeshUpdate* meshUpdate = m_TrackedMeshHolder->AllocateMeshUpdate(meshGuid);
					meshUpdate->Type = EARObjectClassification::Unknown;
//#if ENGINE_MINOR_VERSION == 0  
//					meshUpdate->Vertices = MoveTemp(mesh.vertices);
//#elif ENGINE_MINOR_VERSION == 1
					meshUpdate->Vertices = MoveTemp(meshVertices);
//#endif
					meshUpdate->Indices = MoveTemp(mesh.indices);

					if (IsPoseValid(location.flags))
					{
						meshUpdate->LocalToTrackingTransform = ToFTransform(location.pose, worldToMetersScale);
					}
					else
					{
						meshUpdate->LocalToTrackingTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
					}

					meshUpdate->SpatialMeshUsageFlags = (EARSpatialMeshUsageFlags)((int32)EARSpatialMeshUsageFlags::Visible |
						(int32)EARSpatialMeshUsageFlags::Collision);
				}

				m_LocateCurrentFrame++;
				if (m_LocateCurrentFrame >= m_Meshes.Num())
				{
					m_LocateCurrentFrame = 0;
					m_ScanState = EMeshesScanState::Locating;
					break;
				}
			}

			m_TrackedMeshHolder->EndMeshUpdates();
		}
		UpdateMeshesLocations(DisplayTime, TrackingSpace);
	}

	TArray<FVector3f> FSceneUnderstanding::FVectorDoubleToFVectorFloat(TArray<FVector> input)
	{
		TArray<FVector3f> results;
		FVector3f result;
		results.Empty();
		for (int i = 0; i < input.Num(); i++)
		{
			result.X = input[i].X;
			result.Y = input[i].Y;
			result.Z = input[i].Z;
			results.Add(result);
		}
		return results;
	}

	bool FSceneUnderstanding::OnToggleARCapture(const bool bOnOff)
	{
		//m_bEnableSceneUnderstanding = true;
		return true;
	}

	void FSceneUnderstanding::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
	{
	}

	IOpenXRCustomCaptureSupport* FSceneUnderstanding::GetCustomCaptureSupport(const EARCaptureType CaptureType)
	{
		return CaptureType == EARCaptureType::SpatialMapping ? this : nullptr;;
	}

	TArray<FARTraceResult> FSceneUnderstanding::OnLineTraceTrackedObjects(const TSharedPtr<FARSupportInterface, ESPMode::ThreadSafe> ARCompositionComponent, const FVector Start, const FVector End, const EARLineTraceChannels TraceChannels)
	{
		TArray<FARTraceResult> results;
		TArray<UARMeshGeometry*> meshes = UARBlueprintLibrary::GetAllGeometriesByClass<UARMeshGeometry>();
		for (UARMeshGeometry* mesh : meshes)
		{
			auto CollisionInfo = m_MeshCollisionInfo.Find(mesh->UniqueId);
			if (CollisionInfo != nullptr)
			{
				FVector HitPoint, HitNormal;
				float HitDistance;
				if (CollisionInfo->Collides(Start, End, mesh->GetLocalToWorldTransform(), HitPoint, HitNormal, HitDistance))
				{
					results.Add(FARTraceResult(ARCompositionComponent, HitDistance, TraceChannels,
						FTransform(HitNormal.ToOrientationQuat(), HitPoint), mesh));
				}
			}
		}
		return results;
	}

	void FSceneUnderstanding::SetSceneComputeSphereBound(XrVector3f center, float radius)
	{
		m_SceneSphereBounds.SetNum(1);
		auto& bound = m_SceneSphereBounds[0];
		bound.center = center;
		bound.radius = radius;
	}

	void FSceneUnderstanding::SetSceneComputeOrientedBoxBound(XrQuaternionf orientation, XrVector3f position, XrVector3f extents)
	{
		m_SceneOrientedBoxBounds.SetNum(1);
		auto& bound = m_SceneOrientedBoxBounds[0];
		bound.pose.orientation = orientation;
		bound.pose.position = position;
		bound.extents = extents;
	}

	void FSceneUnderstanding::SetSceneComputeFrustumBound(XrQuaternionf orientation, XrVector3f position, float angleUp, float angleDown, float angleRight, float angleLeft, float farDistance)
	{
		m_SceneFrustumBounds.SetNum(1);
		auto& bound = m_SceneFrustumBounds[0];
		bound.pose.orientation = orientation;
		bound.pose.position = position;
		bound.fov.angleUp = angleUp;
		bound.fov.angleDown = angleDown;
		bound.fov.angleRight = angleRight;
		bound.fov.angleLeft = angleLeft;
		bound.farDistance = farDistance;
	}

	void FSceneUnderstanding::ClearSceneComputeBounds(EXrSceneBoundType type)
	{
		switch (type)
		{
		case EXrSceneBoundType::XR_SCENE_BOUND_SPHERE_TYPE:
			m_SceneSphereBounds.Empty();
			break;
		case EXrSceneBoundType::XR_SCENE_BOUND_ORIENTED_BOX_TYPE:
			m_SceneOrientedBoxBounds.Empty();
			break;
		case EXrSceneBoundType::XR_SCENE_BOUND_FRUSTUM_TYPE:
			m_SceneFrustumBounds.Empty();
			break;
		default:
			break;
		}
	}

	void FSceneUnderstanding::SetSceneComputeConsistency(XrSceneComputeConsistencyMSFT consistency)
	{
		m_SceneComputeConsistency = consistency;
	}

	void FSceneUnderstanding::SetMeshComputeLod(XrMeshComputeLodMSFT lod)
	{
		m_MeshComputeLod = lod;
	}

	void FSceneUnderstanding::Stop()
	{
		if (!m_bEnableSceneUnderstanding) return;
		m_PreviousMeshes.Empty();
		m_Meshes.Empty();
		m_ScanState = EMeshesScanState::Idle;
		m_bOpenXRReady = false;

		// Destroy the SharedOpenXRScene if it's valid.
		if (m_SharedScene.IsValid())
		{
			m_SharedScene.Reset();
		}

		if (m_SceneObserver != XR_NULL_HANDLE)
		{
			// Destroy the scene observer.
			XR_ENSURE(SU_ext.xrDestroySceneObserverMSFT(m_SceneObserver));
		}
	}

	void FSceneUnderstanding::OnDestroySession(XrSession InSession)
	{
		if (m_bEnableSceneUnderstanding)
		{
			if (m_XrSpace != XR_NULL_HANDLE)
			{
				// Destroy the reference space.
				XR_ENSURE(Space_ext.xrDestroySpace(m_XrSpace));
			}
		}
	}
}