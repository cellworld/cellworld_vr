// Fill out your copyright notice in the Description page of Project Settings.

#include "PawnDebug.h"
#include "GameModeMain.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HeadMountedDisplayTypes.h"
#include "IXRTrackingSystem.h"

//AGameModeMain* GameMode;

APawnDebug::APawnDebug()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	RootComponent = CapsuleComponent;

	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(20.325, 45.0f); // (radius,half-height in cm) 16" width, 6' tall 
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnDebug::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnDebug::OnOverlapEnd); // overlap events 

	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetMobility(EComponentMobility::Movable);
	Camera->bUsePawnControlRotation = false;
	Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	/* create instance of our movement component */
	OurMovementComponentChar = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("CharacterMovementComponent"));
	OurMovementComponentChar->MaxStepHeight = 100.0f;
	OurMovementComponentChar->MaxWalkSpeed = 9000.0f;
	OurMovementComponentChar->MaxAcceleration = 9000.0f;
	OurMovementComponentChar->BrakingDecelerationWalking = 4'000.0f;
	OurMovementComponentChar->bDeferUpdateMoveComponent = false;
	OurMovementComponentChar->SetActive(true);
	OurMovementComponentChar->UpdatedComponent = RootComponent;
	
	///* auto-possess */
	EAutoReceiveInput::Type::Player0;

	/* todo: add bIsInVR state */
	this->Camera->bLockToHmd = false;

	// Calculate the rotation needed to align the component's forward vector with the pawn's forward vector
	FRotator TargetRotation = this->RootComponent->GetForwardVector().Rotation() - this->Camera->GetForwardVector().Rotation();
	// Apply the calculated rotation to the component
	Camera->AddRelativeRotation(TargetRotation);
}

bool APawnDebug::DetectMovement()
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

void APawnDebug::OnMovementDetected()
{
	MovementDetectedEvent.Broadcast(_new_location);
	UE_LOG(LogTemp, Log, TEXT("[APawnDebug::OnMovementDetected()] Movement detected."));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, FString::Printf(TEXT("Movement detected")));
}

void APawnDebug::ResetOrigin()
{
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	//FRotator orientation;
	//GEngine->XRSystem->GetHMDDevice()->EnableHMD();
	//if (GEngine->XRSystem->GetHMDDevice()->IsHMDConnected()) {
	//	GEngine->XRSystem->GetHMDData(this, HMDData);
	//	HMDPosition = HMDData.Position;
	//}
	//else {
	//	UE_DEBUG_BREAK();
	//}
	//this->Camera->SetRelativeRotation(HMDData.Rotation);
	//APawnDebug::SetActorLocationAndRotation(FVector(0.0f, 0.0f, 10), FRotator::ZeroRotator, false);
}

// Called when the game starts or when spawned
void APawnDebug::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APawnDebug::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/* check if we moved */
	if (this->DetectMovement()) {
		this->OnMovementDetected();
	}
}

// Called to bind functionality to input
void APawnDebug::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UPawnMovementComponent* APawnDebug::GetMovementComponent() const
{
	return OurMovementComponentChar;
}

void APawnDebug::UpdateMovementComponent(FVector InputVector, bool bForce)
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
	//this->RootComponent->AddWorldOffset(InputVector);
	OurMovementComponentChar->SafeMoveUpdatedComponent(
		InputVector,
		OurMovementComponentChar->UpdatedComponent->GetComponentQuat(),
		bForce,
		OutHit,
		TeleportType);
}

void APawnDebug::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector CameraForwardVector = this->Camera->GetForwardVector();
			CameraForwardVector.Z = 0.0;
			this->UpdateMovementComponent(CameraForwardVector * AxisValue * 7.5, /*force*/ true);
		}
	}
}

void APawnDebug::MoveRight(float AxisValue)
{
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector CameraRightVector = this->Camera->GetRightVector();
			CameraRightVector.Z = 0.0;
			this->UpdateMovementComponent(CameraRightVector * AxisValue * 7.5, /* force */true);
		}
	}
}

/* doesn't work with VR */
void APawnDebug::Turn(float AxisValue)
{
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Yaw += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

/* doesn't work with VR */
void APawnDebug::LookUp(float AxisValue)
{
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Pitch += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

void APawnDebug::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APawnDebug::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

