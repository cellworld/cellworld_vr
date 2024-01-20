#pragma once
#include "PawnMain.h"
#include "GameModeMain.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameEngine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h" 
#include "Kismet/KismetMathLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "PawnMainMovementComponent.h"

// Sets default values
AGameModeMain* GameMode; // forward declare to avoid circular dependency

APawnMain::APawnMain() : Super()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	/* create collision component */
	UCapsuleComponent* CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(0.5f, 1.50f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 

	/* camera component */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	/* ** TEST ** create instance of our movement component */
	OurMovementComponentChar = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("CharacterMovementComponent"));
	OurMovementComponentChar->MaxStepHeight = 100.0f;
	OurMovementComponentChar->MaxWalkSpeed = 1'200.0f;
	OurMovementComponentChar->MaxAcceleration = 4000.0f;
	OurMovementComponentChar->BrakingDecelerationWalking = 4'000.0f;
	OurMovementComponentChar->bDeferUpdateMoveComponent = true;
	OurMovementComponentChar->SetActive(true);
	OurMovementComponentChar->UpdatedComponent = RootComponent;


	///* auto-possess */
	EAutoReceiveInput::Type::Player0;
	EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called to bind functionality to input
void APawnMain::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	/* map toggling stuff */
}

UPawnMovementComponent* APawnMain::GetMovementComponent() const
{
	return OurMovementComponentChar;
}

void APawnMain::UpdateMovementComponent(FVector InputVector, bool bForce)
{
	/* Apply movement, called by MoveForward() or MoveRight().
	FVector InputVector (scaled 0-1) */

	/*
	Note: Felix, 8/30/2023
	Could be more elegant,OurMovementComponentChar->AddInputVector() and then
	OurMovementComponentChar->GetLastInputVector() is the ideal way to use it.
	This would prevent having to pass FVector InputVector into UpdateMovementComponent().
	OurMovementComponentChar->ConsumeInputVector() also returns 0.
	For now, we have this working.
	*/

	OurMovementComponentChar->SafeMoveUpdatedComponent(
		InputVector,
		OurMovementComponentChar->UpdatedComponent->GetComponentQuat(),
		bForce,
		OutHit,
		TeleportType);

}

UCameraComponent* APawnMain::GetCameraComponent()
{
	return this->Camera;
}

void APawnMain::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector ActorForwardVector = GetActorForwardVector();
			ActorForwardVector.Z = 0.0;
			OurMovementComponentChar->AddInputVector(ActorForwardVector * AxisValue, true);
			this->UpdateMovementComponent(ActorForwardVector * AxisValue, /*force*/ true);
		}
	}
}

void APawnMain::MoveRight(float AxisValue)
{
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			this->UpdateMovementComponent(GetActorRightVector() * AxisValue, /* force */true);
		}
	}
}

void APawnMain::Turn(float AxisValue)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += AxisValue;
	SetActorRotation(NewRotation);
}

void APawnMain::LookUp(float AxisValue)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Pitch += AxisValue;
	SetActorRotation(NewRotation);
}

void APawnMain::QuitGame()
{
	// end match 
	// get game mode ->EndMatch(); 
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Quit game.")));
	}

	GameMode = (AGameModeMain*)GetWorld()->GetAuthGameMode();
	GameMode->EndMatch();
}

// Called when the game starts or when spawned
void APawnMain::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APawnMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APawnMain::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APawnMain::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

