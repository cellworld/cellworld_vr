// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Modules/ModuleManager.h"
#include "ViveOpenXRSceneUnderstandingFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXR, Log, All);

UENUM(BlueprintType, Category = "ViveOpenXR|OpenXR|SceneUnderstanding")
enum class EXrBoundType : uint8
{
  None = 0  UMETA(Hidden),
  Sphere = 1,
  OrientedBox = 2,
  Frustum = 3,
};

UENUM(BlueprintType, Category = "ViveOpenXR|OpenXR|SceneUnderstanding")
enum class EXrSceneComputeConsistency : uint8
{
  None = 0  UMETA(Hidden),
  SnapshotComplete = 1,
  SnapshotIncompleteFast = 2,
  OcclusionOptimized = 3,
};

UENUM(BlueprintType, Category = "ViveOpenXR|OpenXR|SceneUnderstanding")
enum class EXrMeshComputeLod : uint8
{
  None = 0  UMETA(Hidden),
  Coarse = 1,
  Medium = 2,
  Fine = 3,
  Unlimited = 4,
};

UCLASS(ClassGroup = OpenXR)
class VIVEOPENXRSCENEUNDERSTANDING_API UViveOpenXRSceneUnderstandingFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Scene Understanding Enabled", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void GetIsSceneUnderstandingEnabled(bool& result);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Scene Compute Sphere Bound", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void SetSceneComputeSphereBound(FVector center, float radius);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Scene Compute Oriented Box Bound", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void SetSceneComputeOrientedBoxBound(FRotator rotator, FVector position, FVector extents);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Scene Compute Frustum Bound", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void SetSceneComputeFrustumBound(FRotator rotator, FVector position, float angleUp, float angleDown, float angleRight, float angleLeft, float farDistance);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Scene Compute Bounds", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void ClearSceneComputeBounds(EXrBoundType type);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Scene Compute Consistency", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void SetSceneComputeConsistency(EXrSceneComputeConsistency consistency);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Mesh Compute Lod", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void SetMeshComputeLod(EXrMeshComputeLod lod);

  UFUNCTION(BlueprintCallable, meta = (DisplayName = "Enable Cosmos Pass Through", Keywords = "ViveOpenXR Scene Understanding"), Category = "ViveOpenXR|SceneUnderstanding")
    static void OpenPassThroughEXE(FString path, FString param);
};
