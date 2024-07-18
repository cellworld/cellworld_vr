#pragma once
#include "CoreMinimal.h"
#include "Components/CustomCharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/HUDExperiment.h"
#include "HeadMountedDisplay.h"
#include "Containers/Array.h" 
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "MotionControllerComponent.h"
#include "PawnMain.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementDetected, FVector, Location);

UCLASS()
class CELLWORLD_VR_API APawnMain : public APawn
{
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
		bool bUseVR = false;

	void ResetOrigin();
	void RestartGame();
	void QuitGame();
	APlayerController* GetGenericController();
	bool HUDResetTimer(float DurationIn) const;
	bool CreateAndInitializeWidget();
	void DestroyHUD();

	/* temp */
	FVector RelLoc;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
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
	class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere)
	UWidgetComponent* HUDWidgetComponent; 
	USceneComponent* VROrigin; 
	UCapsuleComponent* CapsuleComponent;
	UCustomCharacterMovementComponent* MovementComponent; 
	class UMotionControllerComponent* MotionControllerLeft;
	class UMotionControllerComponent* MotionControllerRight;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UHUDExperiment> PlayerHUDClass;

	UPROPERTY()
	class UHUDExperiment* PlayerHUD = nullptr;
	
	// void SetupPlayerInputComponent(class UInputComponent* InInputComponent);

	/* overlap events */
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// to store overlap characters
	UPrimitiveComponent* OverlappedComponent;

	/* Movement Component */
	FHitResult OutHit;
	ETeleportType TeleportType = ETeleportType::TeleportPhysics;

	/* helpers for camera stuff */
	UCameraComponent* GetCameraComponent();
	void StartExperiment();
	void StartEpisode();
	void ValidateHMD();
	bool DetectMovementVR();
	bool DetectMovementWASD();

private: 
	const float _capsule_radius      = 30.0f;
	const float _player_height       = 175.0f; // 1.75 m
	const float _capsule_half_height = _player_height / 2;
	const FVector _camera_location = FVector(0.0f, 0.0f, _capsule_half_height);

	FVector current_location; 
	FVector HMDPosition;
	FXRHMDData HMDData;

	bool DetectMovement();
	void OnMovementDetected();
	void UpdateRoomScaleLocation();
	FVector _old_location; 
	FVector _new_location;
	float _movement_threshold = 5; 
};
