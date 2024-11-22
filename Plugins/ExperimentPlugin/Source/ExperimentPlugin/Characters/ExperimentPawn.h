#pragma once
#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include <Components/SplineComponent.h>
#include <Components/SplineMeshComponent.h>
#include "Components/SkeletalMeshComponent.h"
#include "MiscUtils/Timers/EventTimer.h"
#include "OculusXRPassthroughLayerComponent.h"
#include "XRDeviceVisualizationComponent.h" 
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "HeadMountedDisplay.h"
#include "Containers/Array.h" 
#include "GameFramework/CharacterMovementComponent.h" // test
#include "MotionControllerComponent.h"
#include "ExperimentPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateMovement, FVector, Location, FRotator, Rotation);

class UOculusXRPassthroughLayerComponent;
class UOculusXRStereoLayerShapeReconstructed;

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPawn : public APawn {
	GENERATED_BODY()

public:
	
	/* debug stuff */
	int DebugTimeRemaining = 0;
	
	// Sets default values for this pawn's properties
	AExperimentPawn();
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
	UPROPERTY()
	FUpdateMovement Multi_UpdateMovement;

	UPROPERTY(EditAnywhere)
	bool bUseVR = false;

	APlayerController* GetGenericController();
	bool StartPositionSamplingTimer(const float InRateHz);
	bool StopPositionSamplingTimer();
	void SetupSampling();

	/* temp */
	FVector RelLoc;

	UPROPERTY(EditAnywhere)
		// TObjectPtr<UEventTimer> EventTimer;
		UEventTimer* EventTimer;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	uint32 samps = 0;

	void UpdateSplineMesh(); 
	
	//AGameModeMain* GameMode = nullptr; 

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	UPROPERTY() UCharacterMovementComponent* OurMovementComponentChar;
	void UpdateMovementComponent(FVector InputVector, bool bForce);
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	/* === properties === */
	//UPROPERTY(VisibleDefaultsOnly, meta = (Category = "Default"))
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	UWidgetComponent* HUDWidgetComponent; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> VROrigin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UMotionControllerComponent> MotionControllerLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UMotionControllerComponent> MotionControllerRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> MainMenuAttachmentPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> ModelAttachmentPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> ModelSpawnPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UXRDeviceVisualizationComponent> XRDeviceVisualizationRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USplineMeshComponent> SplineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> AnchorMenuAttachmentPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UOculusXRPassthroughLayerComponent> XRPassthroughLayer;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "VisualComponents")
	TObjectPtr<UStaticMeshComponent> SphereMeshComponent;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	// TObjectPtr<UActorComponent> SpatialAnchorManager;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	// TObjectPtr<UActorComponent> MenuManagerComponent;


	
	UPROPERTY(EditAnywhere)
	UClass* SpatialAnchorManagerBPClass;
	
	FTimerHandle TimerHandleDetectMovement;
	FTimerHandle* TimerHandleDetectMovementPtr = &TimerHandleDetectMovement; 
	
	// void SetupPlayerInputComponent(class UInputComponent* InInputComponent);

	/* overlap events */
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// to store overlap characters
	UPROPERTY()
		TObjectPtr<UPrimitiveComponent> OverlappedComponent;

	/* Movement Component */
	FHitResult OutHit;
	ETeleportType TeleportType = ETeleportType::TeleportPhysics;

	/* helpers for camera stuff */
	UCameraComponent* GetCameraComponent();
	void ValidateHMD();
	bool DetectMovementVR();
	bool DetectMovementWASD();

private: 
	const float CapsuleRadius      = 30.0f;
	const float PlayerHeight       = 175.0f; // 1.75 m
	const float CapsuleHalfHeight = PlayerHeight / 2;

	FVector current_location; 
	FVector HMDPosition;
	FXRHMDData HMDData;
	
	UFUNCTION()
		bool DetectMovement();
	UFUNCTION()
		void OnMovementDetected();
	UFUNCTION()
		void UpdateRoomScaleLocation();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_UpdateMovement();

};
