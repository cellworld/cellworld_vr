#include "ExperimentCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PhysicsVolume.h"

bool AExperimentCharacter::Server_UpdateMovement_Validate(const FVector& InLocation, const FRotator& InRotation) {
	return true;
}

void AExperimentCharacter::
Server_UpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation) {
	// UE_LOG(LogTemp, Warning, TEXT("[AExperimentCharacter::Server_UpdateMovement_Implementation] InLocation: %s | InRotation: %s"),
	// 	*InLocation.ToString(), *InRotation.ToString())
	if (!HasAuthority()) {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentCharacter::Server_UpdateMovement_Implementation] Can't send to Client, doesn't have authority!"))
		return;
	}
	AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(GetWorld()->GetAuthGameMode());
	if (ExperimentGameMode) {
		UE_LOG(LogTemp, Error,
			TEXT("[AExperimentCharacter::Server_UpdateMovement_Implementation] Calling: ExperimentGameMode->OnUpdatePreyPosition"))
		ExperimentGameMode->OnUpdatePreyPosition(InLocation,InRotation);
	}
}

AExperimentCharacter::AExperimentCharacter() {
	// Set size for collision capsule
	
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bSimGravityDisabled = true;
	
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetupAttachment(RootComponent);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	GetCapsuleComponent()->SetMobility(EComponentMobility::Movable);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	bReplicates = true;

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

	GetMovementComponent()->UpdatedComponent = RootComponent;
	
	ACharacter::SetReplicateMovement(true);
}

void AExperimentCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("[APawnMain::OnOverlapBegin()] Hit something!")));
}

void AExperimentCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							 int32 OtherBodyIndex)
{
}

bool AExperimentCharacter::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::StartPositionSamplingTimer]"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::StartPositionSamplingTimer] Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&AExperimentCharacter::UpdateMovement);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::StartPositionSamplingTimer] StartPositionSamplingTimer Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::StartPositionSamplingTimer] OK!"))
	return true;
}

void AExperimentCharacter::SetupSampling() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] Running on Android"))
	if (bUseVR){
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Stage);
		Camera->bUsePawnControlRotation = false; 
	}else {
		Camera->bUsePawnControlRotation = true; 
	}
	// ReSharper disable once CppTooWideScopeInitStatement
	constexpr float FS = 60.0f;
	if (!ensure(this->StartPositionSamplingTimer(FS))) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::SetupSampling] StartPositionSamplingTimer(%0.2f) Failed!"), FS)
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::SetupSampling] StartPositionSamplingTimer(%0.2f) OK!"), FS)
	}
}

void AExperimentCharacter::UpdateMovement() {
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning,TEXT("[AExperimentPawn::UpdateMovement] Ran from server!"))
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateMovement] Running on client."))
		if (bUseVR) { // todo: bUseVR - Make variable 
			FRotator HMDRotation {};
			FVector HMDLocation {};
			UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
			CurrentLocation = HMDLocation + this->VROrigin->GetComponentLocation();
			CurrentRotation = HMDRotation;
			UpdateRoomScaleLocation();
			// Server_UpdateMovement(CurrentLocation, CurrentRotation);
		} else {
			CurrentLocation = RootComponent->GetComponentLocation();
			CurrentRotation = GetActorRotation();
		}
	}
	Server_UpdateMovement(CurrentLocation, CurrentRotation);
}

void AExperimentCharacter::UpdateRoomScaleLocation() {
	const FVector CapsuleLocation = GetCapsuleComponent()->GetComponentLocation();
	FVector CameraLocation = Camera->GetComponentLocation();
	CameraLocation.Z = 0.0f;
	FVector DeltaLocation = CameraLocation - CapsuleLocation;
	DeltaLocation.Z = 0.0f;
	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	GetCapsuleComponent()->SetWorldLocation(CameraLocation);

	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateRoomScaleLocation] VROrigin: Location: %s | Rotation: %s"),
		*VROrigin->GetComponentLocation().ToString(), *VROrigin->GetComponentRotation().ToString())

	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateRoomScaleLocation] Capsule: Location: %s | Rotation: %s"),
		*GetCapsuleComponent()->GetComponentLocation().ToString(), *GetCapsuleComponent()->GetComponentRotation().ToString())
	
}

void AExperimentCharacter::BeginPlay() {
	Super::BeginPlay();
#if !UE_SERVER
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] Running on client."))
	SetupSampling();
	SetReplicates(true);
	SetReplicateMovement(true);
#endif
}

void AExperimentCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AExperimentCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AExperimentCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AExperimentCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AExperimentCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AExperimentCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AExperimentCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AExperimentCharacter::OnResetVR);
}

bool AExperimentCharacter::Multi_OnUpdateMovement_Validate(const FVector& InLocation, const FRotator& InRotation) {
	UE_LOG(LogTemp,Log,TEXT("[AExperimentCharacter::Multi_OnUpdateMovement_Validate] InLocation: %s | InRotatio: %s"),
		*InLocation.ToString(), *InRotation.ToString())
	return true;
}

void AExperimentCharacter::Multi_OnUpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation) {
	UE_LOG(LogTemp,Log,TEXT("[AExperimentCharacter::Multi_OnUpdateMovement_Implementation] InLocation: %s | InRotatio: %s"),
		*InLocation.ToString(), *InRotation.ToString())	
}

void AExperimentCharacter::OnResetVR() {
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AExperimentCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location) {
	Jump();
}

void AExperimentCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location) {
	StopJumping();
}

void AExperimentCharacter::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AExperimentCharacter::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AExperimentCharacter::UpdateMovementComponent(FVector InputVector, bool bForce) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateMovementComponent] InputVector: %s"),
		*InputVector.ToString())

	FHitResult OutHit;
	GetMovementComponent()->SafeMoveUpdatedComponent(
	InputVector,
	GetMovementComponent()->UpdatedComponent->GetComponentQuat(),
	bForce,
	OutHit,
	ETeleportType::TeleportPhysics);
}

void AExperimentCharacter::MoveForward(float Value) {
	if (Value != 0.0f) {
		if (GetMovementComponent() && GetMovementComponent()->UpdatedComponent == RootComponent) {
			FVector CameraForwardVector = this->Camera->GetForwardVector();
			CameraForwardVector.Z = 0.0;
			this->UpdateMovementComponent(CameraForwardVector * Value * 10, /*force*/ true);
		}else {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::MoveForward] GetMovementComponent NULL"))
		}
	}
}

void AExperimentCharacter::MoveRight(float Value) {
	if (Value != 0.0f) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::MoveRight] value valid"))
		if (GetMovementComponent() && (GetMovementComponent()->UpdatedComponent == RootComponent)) {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::MoveRight] inside if"))
			FVector CameraRightVector = this->Camera->GetRightVector();
			CameraRightVector.Z = 0.0;
			this->UpdateMovementComponent(CameraRightVector * Value * 10, /* force */true);
		}else {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::MoveForward] GetMovementComponent NULL"))
		}
	}
}

