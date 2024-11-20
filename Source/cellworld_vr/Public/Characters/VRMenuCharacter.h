// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/Character.h"
#include "VRMenuCharacter.generated.h"

UCLASS()
class CELLWORLD_VR_API AVRMenuCharacter : public ACharacter {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRMenuCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMotionControllerComponent> RightMotionController; 

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMotionControllerComponent> LeftMotionController;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetInteractionComponent> LeftWidgetInteraction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetInteractionComponent> RightWidgetInteraction;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerRightPressed();

	UFUNCTION()
	void OnTriggerRightReleased();

	UFUNCTION()
	void OnTriggerLeftPressed();

	UFUNCTION()
	void OnTriggerLeftReleased();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
