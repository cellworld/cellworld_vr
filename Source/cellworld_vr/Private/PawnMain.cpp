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

	/* create origin for tracking */
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(_capsule_radius, _capsule_half_height);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 
	CapsuleComponent->SetupAttachment(RootComponent);

	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetMobility(EComponentMobility::Movable);
	Camera->SetRelativeLocation(FVector(0.0f,0.0f,_capsule_half_height*-2)); // todo: make sure this is OK
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(RootComponent);

	/*Create Motion Controllers*/
	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerLeft"));
	MotionControllerLeft->CreationMethod = EComponentCreationMethod::Native;
	MotionControllerLeft->SetCanEverAffectNavigation(false);
	MotionControllerLeft->bEditableWhenInherited = true;
	MotionControllerLeft->MotionSource = FName("Left");
	MotionControllerLeft->SetVisibility(false, false);
	MotionControllerLeft->SetupAttachment(RootComponent);

	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionControllerRight"));
	MotionControllerRight->CreationMethod = EComponentCreationMethod::Native;
	MotionControllerRight->SetCanEverAffectNavigation(false);
	MotionControllerRight->bEditableWhenInherited = true;
	MotionControllerRight->MotionSource = FName("Right");
	MotionControllerRight->SetVisibility(false, false);
	MotionControllerRight->SetupAttachment(RootComponent);
}

// Called to bind functionality to input
void APawnMain::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);
}

UCameraComponent* APawnMain::GetCameraComponent()
{
	return APawnMain::Camera;
}

void APawnMain::StartExperiment()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("[APawnMain::StartExperiment()]")));
	return;
}

void APawnMain::StartEpisode()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("[APawnMain::StartEpisode()]")));
	return; 
}

bool APawnMain::DetectMovement()
{
	bool _blocation_updated = false;

	if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("HMD not enabled"));
		return false;
	}

	FVector NewLocation;
	FRotator NewRotation;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(NewRotation, NewLocation);

	if (!NewLocation.Equals(_old_location, 2)) {
		_blocation_updated = true;
		_old_location = NewLocation;
	}
	else {
		_blocation_updated = false; 
	}

	_new_location = NewLocation;
	return _blocation_updated;
}

void APawnMain::UpdateRoomScaleLocation()
{
	FVector DeltaLocation = Camera->GetComponentLocation() - this->CapsuleComponent->GetComponentLocation();
	DeltaLocation.Z = .0f;
	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	const FVector LocOrigin = VROrigin->GetComponentLocation();
	const FVector LocCamera = Camera->GetComponentLocation();
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Origin : %f, %f, %f"), LocOrigin.X, LocOrigin.Y, LocOrigin.Z));
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, FString::Printf(TEXT("Camera: %f, %f, %f"), LocCamera.X, LocCamera.Y, LocCamera.Z));
}

void APawnMain::OnMovementDetected()
{
	MovementDetectedEvent.Broadcast(_new_location);
	UE_LOG(LogTemp, Log, TEXT("[APawnMain::OnMovementDetected()] Movement detected."));
	this->UpdateRoomScaleLocation();
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, FString::Printf(TEXT("Movement detected")));
}

void APawnMain::ResetOrigin() 
{
	//UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Resetting origin.")));
	//FRotator HMDRotation;
	//FVector HMDLocation;
	//UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
	//Camera->SetWorldLocation(FVector(380, -1790, 80));
	//this->SetActorLocation(FVector(380, -1790, 80));
	//Camera->AddRelativeRotation(HMDRotation, false); // original
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
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("[APawnMain::QuitGame()] Quit game.")));
	}
}

// Called when the game starts or when spawned
void APawnMain::BeginPlay()
{
	Super::BeginPlay();
	//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.0f, EOrientPositionSelector::OrientationAndPosition);
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

