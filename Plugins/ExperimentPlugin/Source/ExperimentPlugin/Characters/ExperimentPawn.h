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
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MiscUtils/Timers/EventTimer.h"
#include "OculusXRPassthrough/Public/OculusXRPassthroughLayerComponent.h"
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

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CurrentLocation)
	FVector CurrentLocation;
	
	UFUNCTION()
	void OnRep_CurrentLocation();
	
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CurrentRotation)
	FRotator CurrentRotation;

	UFUNCTION()
	void OnRep_CurrentRotation();
	
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
	FUpdateMovement MultiDelegate_UpdateMovement;

	UPROPERTY(EditAnywhere)
	bool bUseVR = false;

	APlayerController* GetGenericController();
	bool StartPositionSamplingTimer(const float InRateHz);
	bool StopPositionSamplingTimer();
	void SetupSampling();

	// todo:
	// MOVE BOTEVADEGAMEMODE TO EXPERIMENTGAMEMODE IN PLUGIN
	// class ABotEvadeGameMode; 
	// UPROPERTY(EditAnywhere)
	// AExperimentGameMode* BotEvadeGameMode = nullptr;
	
	/* temp */
	FVector RelLoc;

	UPROPERTY(EditAnywhere)
		// TObjectPtr<UEventTimer> EventTimer;
		UEventTimer* EventTimer;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	uint32 samps = 0;

	
	//AGameModeMain* GameMode = nullptr; 

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
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
	UFUNCTION(NetMulticast,Reliable,BlueprintCallable)
	void Multi_UpdateMovement(const FVector& InLocation, const FRotator& InRotation);

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
	
	FORCEINLINE void UpdateRoomScaleLocation() {
		const FVector CapsuleLocation = this->CapsuleComponent->GetComponentLocation();
		FVector CameraLocation = Camera->GetComponentLocation();
		CameraLocation.Z = 0.0f;
		FVector DeltaLocation = CameraLocation - CapsuleLocation;
		DeltaLocation.Z = 0.0f;
		AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
		VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
		this->CapsuleComponent->SetWorldLocation(CameraLocation);
	}


	UFUNCTION(BlueprintCallable)
	void UpdateMovement();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_UpdateMovement(const FVector& InLocation, const FRotator& InRotation);
	bool Server_UpdateMovement_Validate(const FVector& InLocation, const FRotator& InRotation);
	void Server_UpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation);

	
	FORCEINLINE void Move() {
		FVector FinalLocation = {};
		FRotator FinalRotation = {};
		// if ((UHeadMountedDisplayFunctionLibrary::GetHMDWornState() == EHMDWornState::Worn)) {
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected()) {
			UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Move] HMD Connected"))
			FVector HMDLocation = {};
			FRotator HMDRotation = {};
			UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
			CurrentLocation = HMDLocation + this->VROrigin->GetComponentLocation();
			CurrentRotation = HMDRotation;
			UpdateRoomScaleLocation();
		} else {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::Move] HMD is not connected!"))
			CurrentLocation = RootComponent->GetComponentLocation();
			CurrentRotation = GetActorRotation();
		}
	}
};
