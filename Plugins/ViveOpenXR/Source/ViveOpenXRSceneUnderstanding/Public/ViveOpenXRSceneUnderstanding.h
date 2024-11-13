// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "ARBlueprintLibrary.h"
#include "CoreMinimal.h"
#include "IOpenXRARModule.h"
#include "IOpenXRARTrackedGeometryHolder.h"
#include "IOpenXRHMDModule.h"
#include "IOpenXRExtensionPlugin.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "TrackedMeshCollision.h"
#include "ViveOpenXRSceneUnderstandingFunctionLibrary.h"


class IOpenXRARTrackedMeshHolder;

inline bool operator==(const XrUuidMSFT& Lh, const XrUuidMSFT& Rh) noexcept
{
	return memcmp(&Rh, &Lh, sizeof(XrUuidMSFT)) == 0;
}

inline bool operator!=(const XrUuidMSFT& Lh, const XrUuidMSFT& Rh) noexcept
{
	return !(Lh == Rh);
}

// Allows XrUuidMSFT to be used as a key in TMap
inline uint32 GetTypeHash(const XrUuidMSFT& Uuid)
{
	static_assert(sizeof(XrUuidMSFT) == sizeof(uint64) * 2, "");
	const uint64* V = reinterpret_cast<const uint64*>(Uuid.bytes);
	return HashCombine(GetTypeHash(V[0]), GetTypeHash(V[1]));
}

inline FGuid XrUuidMSFTToFGuid(const XrUuidMSFT& Uuid)
{
	static_assert(sizeof(FGuid) == sizeof(XrUuidMSFT), "");
	FGuid OutGuid;
	FMemory::Memcpy(&OutGuid, &Uuid, sizeof(FGuid));
	return OutGuid;
}

namespace ViveOpenXR
{
	constexpr bool IsPoseValid(XrSpaceLocationFlags LocationFlags)
	{
		constexpr XrSpaceLocationFlags PoseValidFlags =
			XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
		return (LocationFlags & PoseValidFlags) == PoseValidFlags;
	}

	enum class EXrSceneBoundType
	{
		XR_SCENE_BOUND_SPHERE_TYPE = 1,
		XR_SCENE_BOUND_ORIENTED_BOX_TYPE = 2,
		XR_SCENE_BOUND_FRUSTUM_TYPE = 3,
		XR_SCENE_BOUND_MAX = 4
	};

	enum class EMeshesScanState
	{
		Idle,
		Waiting,
		Updating,
		AddMeshesToTrackedMeshes,
		Locating
	};

	struct FXrSpaceExtesionDispatchTable
	{
		PFN_xrEnumerateReferenceSpaces xrEnumerateReferenceSpaces;
		PFN_xrCreateReferenceSpace xrCreateReferenceSpace;
		PFN_xrDestroySpace xrDestroySpace;
	};

	struct FSceneUnderstandingExtesionDispatchTable
	{
		PFN_xrEnumerateSceneComputeFeaturesMSFT xrEnumerateSceneComputeFeaturesMSFT;
		PFN_xrCreateSceneObserverMSFT xrCreateSceneObserverMSFT;
		PFN_xrDestroySceneObserverMSFT xrDestroySceneObserverMSFT;
		PFN_xrCreateSceneMSFT xrCreateSceneMSFT;
		PFN_xrDestroySceneMSFT xrDestroySceneMSFT;
		PFN_xrComputeNewSceneMSFT xrComputeNewSceneMSFT;
		PFN_xrGetSceneComputeStateMSFT xrGetSceneComputeStateMSFT;
		PFN_xrGetSceneComponentsMSFT xrGetSceneComponentsMSFT;
		PFN_xrLocateSceneComponentsMSFT xrLocateSceneComponentsMSFT;
		PFN_xrGetSceneMeshBuffersMSFT xrGetSceneMeshBuffersMSFT;
	};

	struct FMeshInfo
	{
		XrUuidMSFT meshId;
		FGuid meshGuid;
		bool updated;
	};

	struct FMeshUpdate
	{
		FGuid meshGuid;
		TArray<FVector> vertices;
		TArray<MRMESH_INDEX_TYPE> indices;
	};

	class SharedOpenXRScene
	{
	public:
		/**
		 * @param[in] scene A valid scene, which is created by xrCreateSceneMSFT.
		 */
		SharedOpenXRScene(FSceneUnderstandingExtesionDispatchTable SU_ext, XrSceneMSFT scene);
		~SharedOpenXRScene();

		XrSceneMSFT GetScene() const { return m_Scene; };

		FSceneUnderstandingExtesionDispatchTable SU_ext{};

	private:
		XrSceneObserverMSFT m_sceneObserver;
		XrSceneMSFT m_Scene;
	};

	struct FMeshDataUpdate
	{
		TSharedPtr<SharedOpenXRScene> m_SharedOpenXRScene;
		TMap<XrUuidMSFT, FMeshUpdate> meshes;
		TMap<FGuid, TrackedMeshCollision> meshCollisionInfo;
	};

	class FSceneUnderstanding : public IOpenXRExtensionPlugin, public IOpenXRCustomCaptureSupport
	{
	public:
		FSceneUnderstanding();
		~FSceneUnderstanding();

		XrSceneObserverMSFT m_SceneObserver = XR_NULL_HANDLE;

		void Register();
		void Unregister();

		/** IOpenXRExtensionPlugin implementation */
		virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
		virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
		virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
		virtual bool OnToggleARCapture(const bool bOnOff) override;
		virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
		virtual void OnDestroySession(XrSession InSession) override;
		virtual IOpenXRCustomCaptureSupport* GetCustomCaptureSupport(const EARCaptureType CaptureType) override;

		virtual TArray<FARTraceResult> OnLineTraceTrackedObjects(
			const TSharedPtr<FARSupportInterface, ESPMode::ThreadSafe> ARCompositionComponent, const FVector Start,
			const FVector End, const EARLineTraceChannels TraceChannels) override;

		/**
		 * Set a scene compute sphere bound.
		 * Existing sphere bound will be replaced.
		 */
		void SetSceneComputeSphereBound(XrVector3f center, float radius);
		/**
		 * Set a scene compute oriented box bound.
		 * Existing oriented box bound will be replaced.
		 */
		void SetSceneComputeOrientedBoxBound(XrQuaternionf orientation, XrVector3f position, XrVector3f extents);
		/**
		 * Set a scene compute oriented box bound.
		 * Existing frustum bound will be replaced.
		 */
		void SetSceneComputeFrustumBound(XrQuaternionf orientation, XrVector3f position,
			float angleUp, float angleDown, float angleRight, float angleLeft, float farDistance);
		/**
		 * Clear scene compute bounds of a specified type.
		 */
		void ClearSceneComputeBounds(EXrSceneBoundType type);
		void SetSceneComputeConsistency(XrSceneComputeConsistencyMSFT consistency);
		void SetMeshComputeLod(XrMeshComputeLodMSFT lod);

		bool m_bEnableSceneUnderstanding = false;

	private:
		void ComputeNewScene(XrTime displayTime);
		void LocateObjects(XrSceneMSFT scene, XrSpace BaseSpace, XrTime Time, const TArray<XrUuidMSFT>& Identifiers, TArray<XrSceneComponentLocationMSFT>& Locations);
		void ProcessMeshDataUpdate(FMeshDataUpdate&& meshDataUpdate, XrTime displayTime, XrSpace trackingSpace);
		void UpdateMeshesLocations(XrTime time, XrSpace space);
		TArray<FVector3f> FVectorDoubleToFVectorFloat(TArray<FVector> input);
		void Stop();

		bool m_bOpenXRReady = false;
		TSharedPtr<SharedOpenXRScene> m_SharedScene;
		TMap<XrUuidMSFT, FMeshInfo> m_PreviousMeshes;
		TMap<XrUuidMSFT, FMeshUpdate> m_Meshes;
		TArray<XrSceneComponentLocationMSFT> m_Locations;
		TMap<FGuid, TrackedMeshCollision> m_MeshCollisionInfo;
		int m_LocateCurrentFrame = 0;
		int m_NumOfMeshToDrawPerFrame = 5;
		FXrSpaceExtesionDispatchTable Space_ext{};
		FSceneUnderstandingExtesionDispatchTable SU_ext{};
		XrSession m_Session = XR_NULL_HANDLE;
		XrSpace m_XrSpace = XR_NULL_HANDLE;
		EMeshesScanState m_ScanState{ EMeshesScanState::Idle };
		TArray<XrSceneComputeFeatureMSFT> m_SceneComputeFeatures;

		// Bounds
		TArray<XrSceneSphereBoundMSFT> m_SceneSphereBounds;
		TArray<XrSceneOrientedBoxBoundMSFT> m_SceneOrientedBoxBounds;
		TArray<XrSceneFrustumBoundMSFT> m_SceneFrustumBounds;

		TFuture<TSharedPtr<FMeshDataUpdate>> m_ScanedMeshFuture;

		class IXRTrackingSystem* XRTrackingSystem = nullptr;
		IOpenXRARTrackedMeshHolder* m_TrackedMeshHolder = nullptr;
		XrReferenceSpaceType m_ReferenceSpaceType;
		XrSceneComputeConsistencyMSFT m_SceneComputeConsistency{ XrSceneComputeConsistencyMSFT::XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_INCOMPLETE_FAST_MSFT };
		XrMeshComputeLodMSFT m_MeshComputeLod{ XrMeshComputeLodMSFT::XR_MESH_COMPUTE_LOD_MEDIUM_MSFT };
	};
}// namespace ViveOpenXR