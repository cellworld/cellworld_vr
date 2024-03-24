// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "HeadMountedDisplay.h"
#include "PawnDebug.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementDetectedDebug, FVector, Location);

UCLASS()
class CELLWORLDVR_API APawnDebug : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnDebug();

	UPROPERTY()
		FOnMovementDetectedDebug MovementDetectedEvent;

	void ResetOrigin();
	const float capsule_height = 175.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY() UCharacterMovementComponent* OurMovementComponentChar;
	class UCapsuleComponent* CollisionCylinder;
	class USkeletalMeshComponent* CharacterMeshComponent;
	class UCharacterMovementComponent* CharacterMoveComponent;
	class UCameraComponent* Camera;
	UCapsuleComponent* CapsuleComponent;
	class UArrowComponent* Arrow;
	class USkeletalMeshComponent* CharacterMesh;
	class UCharacterMovementComponent* CharMoveComp;
	class UMotionControllerComponent* MotionControllerLeft;
	class UMotionControllerComponent* MotionControllerLeftAim;
	class UMotionControllerComponent* MotionControllerRight;
	class UMotionControllerComponent* MotionControllerRightAim;
	class UWidgetInteractionComponent* WidgetInteractionLeft;
	class UWidgetInteractionComponent* WidgetInteractionRight;
	class UWidgetInteractionComponent* WidgetInteraction;

	/* overlap events */
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual UPawnMovementComponent* GetMovementComponent() const override;
	FHitResult OutHit;
	ETeleportType TeleportType = ETeleportType::None;
	void UpdateMovementComponent(FVector InputVector, bool bForce);
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	FVector current_location;
	FVector HMDPosition;
	FXRHMDData HMDData;

	bool DetectMovement();
	void OnMovementDetected();
	FVector _old_location;
	FVector _new_location;
	float _movement_threshold = 5;
};
