#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "MotionControllerComponent.h"
#include "OculusXRPassthroughLayerComponent.h"
#include "GameFramework/Pawn.h"
#include "Containers/Array.h" 
#include "BotEvadePawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementDetectedNew, FVector, Location, FRotator, Rotation);

class USceneComponent;
class UCameraComponent;
class USplineComponent;
class USplineMeshComponent;
class UMotionControllerComponent;
class UOculusXRPassthroughLayerComponent;

UCLASS(Blueprintable)
class ABotEvadePawn : public APawn {
	GENERATED_BODY()
public:
	ABotEvadePawn();

	UPROPERTY(BlueprintCallable, Blueprintable)
	FOnMovementDetectedNew MovementDetectedEvent;

	UFUNCTION()
	bool StartPositionSamplingTimer(const float InRateHz = 60.0f);

	UFUNCTION()
	bool StopPositionSamplingTimer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;

	UPROPERTY();
	TObjectPtr<UCharacterMovementComponent> OurMovementComponentChar;

	void UpdateMovementComponent(FVector InputVector, bool bForce);
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	// Default Scene Root
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	// Motion Controllers
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UMotionControllerComponent> MotionControllerL;

	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UMotionControllerComponent> MotionControllerR;

	// Camera
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	// Oculus XR Passthrough Layer
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UOculusXRPassthroughLayerComponent> OculusXRPassthroughLayer;

	// Main Menu Attachment Point
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> MainMenuAttachmentPoint;

	// Model Spawn Point
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> ModelSpawnPoint;

	// Anchor Menu Attachment Point
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> AnchorMenuAttachmentPoint;

	// Spline
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USplineComponent> Spline;

	// Spline Mesh
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<USplineMeshComponent> SplineMesh;

	// Spatial Anchor Manager
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UActorComponent> SpatialAnchorManager_AC;

	// Menu Manager
	UPROPERTY(Blueprintable, EditAnywhere, Category = "Components")
	TObjectPtr<UActorComponent> MenuManager_AC;
};
