﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MiscUtils/Timers/EventTimer.h"
// #include "XRDeviceVisualizationComponent.h"
#include "MotionControllerComponent.h"
#include "EXperimentCharacter.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	const bool bUseVR = false;
	UFUNCTION()
	void SetupSampling();
	UFUNCTION()
	void UpdateMovement();
	bool StartPositionSamplingTimer(const float InRateHz);
	UPROPERTY(EditAnywhere)
	// TObjectPtr<UEventTimer> EventTimer;
	UEventTimer* EventTimer;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_UpdateMovement(const FVector& InLocation, const FRotator& InRotation);
	bool Server_UpdateMovement_Validate(const FVector& InLocation, const FRotator& InRotation);
	void Server_UpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation);
	
public:
	// Sets default values for this character's properties
	AExperimentCharacter();
	
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<USceneComponent> VROrigin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UMotionControllerComponent> MotionControllerLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpatialComponents")
	TObjectPtr<UMotionControllerComponent> MotionControllerRight;

	UPROPERTY(VisibleAnywhere)
	FVector CurrentLocation;
	
	UPROPERTY(VisibleAnywhere)
	FRotator CurrentRotation;

	UFUNCTION(BlueprintCallable)
	void UpdateRoomScaleLocation();
	
	virtual void BeginPlay() override;

/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void Multi_OnUpdateMovement(const FVector& InLocation, const FRotator& InRotation);
	bool Multi_OnUpdateMovement_Validate(const FVector& InLocation, const FRotator& InRotation);
	void Multi_OnUpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation);
	

	/** Resets HMD orientation in VR. */
	void OnResetVR();
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	void UpdateMovementComponent(FVector InputVector, bool bForce);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

public:

	// Pointer to the online session interface
	// IOnlineSessionPtr OnlineSessionInterface;

protected:

private:

};
