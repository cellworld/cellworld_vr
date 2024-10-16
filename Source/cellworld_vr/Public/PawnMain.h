#pragma once
#include "CoreMinimal.h"
#include "Components/CustomCharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include <Components/SplineComponent.h>
#include <Components/SplineMeshComponent.h>
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/HUDExperiment.h"
#include "HeadMountedDisplay.h"
#include "Containers/Array.h" 
#include "GameFramework/CharacterMovementComponent.h" // test
#include "MotionControllerComponent.h"
#include "MiscUtils/Timer/EventTimer.h"
#include "PawnMain.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementDetected, FVector, Location, FRotator, Rotation);

UCLASS()
class CELLWORLD_VR_API APawnMain : public APawn {
	GENERATED_BODY()

public:
	
	/* debug stuff */
	int DebugTimeRemaining = 0;
	void DebugHUDAddTime();
	
	// Sets default values for this pawn's properties
	APawnMain();
	
	UPROPERTY()
	FOnMovementDetected MovementDetectedEvent;

	UPROPERTY(EditAnywhere)
		bool bUseVR = true;

	void ResetOrigin();
	void RestartGame();
	void QuitGame();
	APlayerController* GetGenericController();
	bool HUDResetTimer(float DurationIn) const;
	bool CreateAndInitializeWidget();
	bool StartPositionSamplingTimer(const float InRateHz);
	bool StopPositionSamplingTimer();
	void DestroyHUD();

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
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY() UCharacterMovementComponent* OurMovementComponentChar;
	void UpdateMovementComponent(FVector InputVector, bool bForce);
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	/* === properties === */
	//UPROPERTY(VisibleDefaultsOnly, meta = (Category = "Default"))
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(EditAnywhere)
	UWidgetComponent* HUDWidgetComponent; 
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> VROrigin;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCustomCharacterMovementComponent> MovementComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMotionControllerComponent> MotionControllerLeft;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMotionControllerComponent> MotionControllerRight;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCustomStereoLayerComponent> StereoLayerComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> MainMenuAttachmentPoint;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> ModelAttachmentPoint;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> ModelSpawnPoint;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UXRDeviceVisualizationComponent> XRDeviceVisualizationRight;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> SplineComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineMeshComponent> SplineMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> AnchorMenuAttachmentPoint;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UHUDExperiment> PlayerHUDClass;

	
	FTimerHandle TimerHandleDetectMovement;
	FTimerHandle* TimerHandleDetectMovementPtr = &TimerHandleDetectMovement; 
	
	UPROPERTY()
	class UHUDExperiment* PlayerHUD = nullptr;
	
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
	void DbgStartExperiment();
	void DbgStopExperiment();
	void DbgStopEpisode();
	void DbgStartEpisode();
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

	FVector _old_location; 
	FVector _new_location;
	float _movement_threshold = 5; 
};
