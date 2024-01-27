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
//#include "PawnMainMovementComponent.h"

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
	//Camera->AddRelativeLocation(FVector(0.0f, 0.0f, capsule_half_height), false);
	//Camera->AddRelativeRotation(FRotator(-180.0, 0.0f, 0.0f), false);
	RootComponent = Camera; 

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(capsule_radius, capsule_half_height);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 

	///* auto-possess */
	EAutoReceiveInput::Type::Player0;
	//EAutoPossessAI::PlacedInWorldOrSpawned;
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
void APawnMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//current_location = this->RootComponent->GetComponentLocation();
	//UE_LOG(LogTemp, Warning, TEXT("[APawnMain::Tick] actor location: %f %f %f."), current_location.X, current_location.Y, current_location.Z);

	//FQuat DeviceRotation;
	//FVector DevicePosition;
	//FVector FinalPosition;

	//GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, DeviceRotation, DevicePosition);
	//UE_LOG(LogTemp, Warning, TEXT("[APawnMain::Tick] device location: %f %f %f.\n"), DevicePosition.X, DevicePosition.Y, current_location.Z);

	//APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	//FinalPosition = this->GetActorRotation().RotateVector(DevicePosition) + PlayerController->PlayerCameraManager->GetCameraLocation();
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

