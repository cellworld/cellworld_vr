// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/VRMenuCharacter.h"

#include "cellworld_vr/cellworld_vr.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AVRMenuCharacter::AVRMenuCharacter() {
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	if (UCharacterMovementComponent* CharacterMovementComponent = this->GetCharacterMovement()) {
		CharacterMovementComponent->GravityScale = 0.0f; 
	}
	
	// Create a Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;

	// Create Left Motion Controller
	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	LeftMotionController->SetupAttachment(RootComponent);
	LeftMotionController->SetTrackingSource(EControllerHand::Left);

	// Create Right Motion Controller
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetupAttachment(RootComponent);
	RightMotionController->SetTrackingSource(EControllerHand::Right);

	// Create Left Widget Interaction Component
	LeftWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("LeftWidgetInteraction"));
	LeftWidgetInteraction->SetupAttachment(LeftMotionController);
	LeftWidgetInteraction->bAutoActivate = true;
	LeftWidgetInteraction->InteractionDistance = 5000;
	LeftWidgetInteraction->SetCanEverAffectNavigation(false);
	LeftWidgetInteraction->TraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
	LeftWidgetInteraction->InteractionSource = EWidgetInteractionSource::World;
	LeftWidgetInteraction->PointerIndex = 0;
	LeftWidgetInteraction->bShowDebug = true; // Optional: enables a debug line to show interaction trace

	// Create Right Widget Interaction Component
	RightWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("RightWidgetInteraction"));
	RightWidgetInteraction->SetupAttachment(RightMotionController);
	RightWidgetInteraction->bAutoActivate = true;
	RightWidgetInteraction->InteractionDistance = 5000;
	RightWidgetInteraction->SetCanEverAffectNavigation(false);
	RightWidgetInteraction->TraceChannel = ECollisionChannel::ECC_WorldDynamic;
	RightWidgetInteraction->InteractionSource = EWidgetInteractionSource::World;
	RightWidgetInteraction->PointerIndex = 1;
	RightWidgetInteraction->bShowDebug = true; // Optional: enables a debug line to show interaction trace
}

// Called to bind functionality to input
void AVRMenuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("TriggerLeft", EInputEvent::IE_Pressed, this, &AVRMenuCharacter::OnTriggerLeftPressed);
	PlayerInputComponent->BindAction("TriggerRight", EInputEvent::IE_Pressed, this, &AVRMenuCharacter::OnTriggerRightPressed);

	PlayerInputComponent->BindAction("TriggerLeft", EInputEvent::IE_Released, this, &AVRMenuCharacter::OnTriggerLeftReleased);
	PlayerInputComponent->BindAction("TriggerRight", EInputEvent::IE_Released, this, &AVRMenuCharacter::OnTriggerRightReleased);
}

// Called when the game starts or when spawned
void AVRMenuCharacter::BeginPlay() {
	Super::BeginPlay();
	// Enable input for the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	EnableInput(PlayerController);
}

// Called every frame
void AVRMenuCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVRMenuCharacter::OnTriggerLeftPressed() {
	if (LeftWidgetInteraction) {
		UE_LOG(LogExperiment, Log, TEXT("[AVRMenuCharacter::OnTriggerLeftPressed] Valid"))
		LeftWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AVRMenuCharacter::OnTriggerRightPressed() {
	if (RightWidgetInteraction) {
		UE_LOG(LogExperiment, Log, TEXT("[AVRMenuCharacter::OnTriggerRightPressed] Valid"))
		RightWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
		RightWidgetInteraction->ReleaseKey(EKeys::LeftMouseButton);
	}
}

void AVRMenuCharacter::OnTriggerLeftReleased() {
	if (LeftWidgetInteraction) {
		UE_LOG(LogExperiment, Log, TEXT("[AVRMenuCharacter::OnTriggerLeftReleased] Valid"))
		LeftWidgetInteraction->ReleaseKey(EKeys::LeftMouseButton);
	}
}

void AVRMenuCharacter::OnTriggerRightReleased() {
	if (RightWidgetInteraction) {
		UE_LOG(LogExperiment, Log, TEXT("[AVRMenuCharacter::OnTriggerRightReleased] Valid"))
		RightWidgetInteraction->ReleaseKey(EKeys::LeftMouseButton);
	}
}



