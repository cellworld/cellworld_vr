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
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IXRTrackingSystem.h"
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
	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->bUsePawnControlRotation = true;
	//Camera->AddRelativeLocation(FVector(0.0f, 0.0f, capsule_half_height));
	RootComponent = Camera; 

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(capsule_radius, capsule_half_height);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 


	/* create instance of our movement component */
	OurMovementComponentChar = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("CharacterMovementComponent"));
	OurMovementComponentChar->MaxStepHeight = 100.0f;
	OurMovementComponentChar->MaxWalkSpeed = 4000.0f;
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
	InInputComponent->BindAction("MenuToggleAny", IE_Pressed, this, &APawnMain::ResetOrigin);
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
	return APawnMain::Camera;
}

void APawnMain::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector ActorForwardVector = GetActorForwardVector();
			ActorForwardVector.Z = 0.0;
			//Camera->AddRelativeLocation(ActorForwardVector * AxisValue, true);
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

void APawnMain::ResetOrigin() 
{
	//FQuat Quat = FQuat();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	APawnMain::SetActorLocationAndRotation(FVector(0.0f, 0.0f, capsule_half_height), FRotator::ZeroRotator, false);
	//APawnMain::RootComponent->AddRelativeLocation(FVector(0.0f, 0.0f, capsule_half_height));

	/* reset height of pawn to match height of HMD */
	//float CameraHeight = Camera->GetComponentLocation().Z - GetActorLocation().Z; // 1.75 m (human) * 1.4 (scale of world to human)
	//UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(GetRootComponent());
	//CapsuleComponent->SetCapsuleHalfHeight(CameraHeight / 2);
	//APawnMain::SetWorldLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
}

void APawnMain::RestartGame() {

	FName level_loading = TEXT("L_Loading");
	UGameplayStatics::OpenLevel(this, level_loading, true);
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

float IPDtoUU() {
	const float IPD_cm = 6.50f; // interpupillary distance 
	return IPD_cm * 100;
}

// Called every frame
void APawnMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Loc = this->Camera->GetComponentLocation();
	UE_LOG(LogTemp, Warning, TEXT("camera: %f, %f, %f."), Loc.X, Loc.Y, Loc.Z);
	Loc.Z = 0;
	
	//this->RootComponent->SetWorldLocation(Loc);

	//this->SetActorLocation(Loc);

	//const FRotator Rot = this->Camera->GetComponentRotation();
	//UE_LOG(LogTemp, Warning, TEXT("Rot: %f, %f, %f."), Rot.Yaw, Rot.Pitch, Rot.Roll);

	Loc = this->RootComponent->GetComponentLocation();
	UE_LOG(LogTemp, Warning, TEXT("root: %f, %f, %f."), Loc.X, Loc.Y, Loc.Z);

	Loc = this->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("pawn: %f, %f, %f."), Loc.X, Loc.Y, Loc.Z);

}

void APawnMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//UE_LOG(LogTemp, Warning, TEXT("[APawnMain::EndPlay] Destroying pawn."));
	//APawnMain::Destroy(EEndPlayReason::Destroyed);
}

void APawnMain::Reset()
{
	Super::Reset();

}

void APawnMain::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APawnMain::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

