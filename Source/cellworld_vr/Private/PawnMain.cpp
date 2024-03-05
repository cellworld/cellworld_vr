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
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h" 
#include "Kismet/KismetMathLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"

// Sets default values
AGameModeMain* GameMode; // forward declare to avoid circular dependency

APawnMain::APawnMain() : Super()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetMobility(EComponentMobility::Movable);
	Camera->bUsePawnControlRotation = true;
	RootComponent = Camera;

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(_capsule_radius, _capsule_half_height);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 

	/* auto-possess */
	//EAutoReceiveInput::Type::Player0;
	EAutoReceiveInput::Player0;
	ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
}

// Called to bind functionality to input
void APawnMain::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	/* map toggling stuff */
	InInputComponent->BindAction("ResetOrigin", IE_Pressed, this, &APawnMain::ResetOrigin);
}

UCameraComponent* APawnMain::GetCameraComponent()
{
	return APawnMain::Camera;
}

bool APawnMain::DetectMovement()
{
	/* process */
	bool _blocation_updated = false; 

	_new_location = this->GetActorLocation();

	if (!_new_location.Equals(_old_location, 2)) {
		_blocation_updated = true;
		_old_location = _new_location;
	}
	else {
		_blocation_updated = false; 
	}
	return _blocation_updated;
}

void APawnMain::OnMovementDetected()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Movement detected")));
	}
}

void APawnMain::ResetOrigin() 
{
	//UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	FRotator HMDRotation;
	FVector HMDLocation;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
	Camera->AddRelativeRotation(HMDRotation, false);
	//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.0f, EOrientPositionSelector::OrientationAndPosition);
	//this->SetActorLocation(FVector(500.0f, -300.0f, 0.0f), false);
}

void APawnMain::RestartGame() {

	FName level_loading = TEXT("L_Loading");
	UGameplayStatics::OpenLevel(this, level_loading, true);
}

void APawnMain::QuitGame()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Quit game.")));
	}

	GameMode = (AGameModeMain*)GetWorld()->GetAuthGameMode();
	GameMode->EndGame();
}

// Called when the game starts or when spawned
void APawnMain::BeginPlay()
{
	Super::BeginPlay();
	//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.0f, EOrientPositionSelector::OrientationAndPosition);
}

float IPDtoUU() {
	const float IPD_cm = 6.50f; // interpupillary distance 
	return IPD_cm * 100;
}

// Called every frame

/* todo: implement elsewhere */
void APawnMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	/* check if we moved */
	if (this->DetectMovement()) {
		this->OnMovementDetected();
	}

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

