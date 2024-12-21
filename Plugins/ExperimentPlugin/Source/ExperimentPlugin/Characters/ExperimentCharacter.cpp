#include "ExperimentCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"
#include "ExperimentPlugin/PlayerControllers/ExperimentPlayerControllerVR.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

AExperimentCharacter::AExperimentCharacter() {

	/* network stuff */
	bReplicates      = true;
	bNetLoadOnClient = true;
	
	/* set our turn rates for input */
	BaseTurnRate	 = 45.f;
	BaseLookUpRate	 = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;
	bSimGravityDisabled			= true;
	
	
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	// VROrigin->SetupAttachment(GetCapsuleComponent());
	RootComponent = VROrigin;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	GetCapsuleComponent()->SetMobility(EComponentMobility::Movable);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	GetCapsuleComponent()->SetupAttachment(RootComponent);
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->SetUpdatedComponent(GetCapsuleComponent());

	// Create a follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	XRPassthroughLayer = CreateDefaultSubobject<UOculusXRPassthroughLayerComponent>(TEXT("OculusXRPassthroughLayer"));
	if (XRPassthroughLayer) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::AExperimentCharacter] XRPassthroughLayer valid"))
		XRPassthroughLayer->bSupportsDepth  = true;
		XRPassthroughLayer->bLiveTexture    = false;
		XRPassthroughLayer->bNoAlphaChannel = true;
		XRPassthroughLayer->SetAutoActivate(true);
		XRPassthroughLayer->AttachToComponent(Camera,FAttachmentTransformRules::KeepRelativeTransform);
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::AExperimentCharacter] - XRPassthroughLayer null "));
	}

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

	// GetMovementComponent()->UpdatedComponent = RootComponent;
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMPClassObject(TEXT("InputMappingContext'/Game/SpatialAnchorsSample/Inputs/Mappings/IMC_VRPawn'"));
	if (IMPClassObject.Succeeded()) {
		DefaultMappingContext = IMPClassObject.Object;
	}
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentCharacter::AExperimentCharacter] DefaultMappingContext found? %s"),
		DefaultMappingContext ? *FString("valid") : *FString("NULL"));
}

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
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentCharacter::Server_UpdateMovement_Implementation] Calling: ExperimentGameMode->OnUpdatePreyPosition"))
		ExperimentGameMode->OnUpdatePreyPosition(InLocation,InRotation);
	}
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

bool AExperimentCharacter::Server_OnExperimentStarted_Validate() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Validate] Called!"))
	return true;
}

void AExperimentCharacter::Server_OnExperimentStarted_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] Called!"))

	// if (!HasAuthority()) {
	// 	UE_LOG(LogTemp, Log,
	// 		TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] No authority!"))
	// 	return;
	// }
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] Called!"))
	// if (UWorld* World = GetWorld()) {
	// 	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] World valid!"))
	//
	// 	AGameModeBase* GameMode = World->GetAuthGameMode();
	// 	if (AExperimentGameMode* ExperimentGameMode = Cast<AExperimentGameMode>(GameMode)){
	// 		UE_LOG(LogTemp, Log,
	// 			TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] ExperimentGameMode valid!"))
	//
	// 		if (ensure(ExperimentGameMode->ExperimentClient)) {
	// 			UE_LOG(LogTemp, Log,
	// 				TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] ExperimentClient valid!"))
	// 			APlayerController* PlayerController = Cast<APlayerController>(GetController());
	// 			if (!ensure(PlayerController)) { return; }
	// 			ExperimentGameMode->UpdateNetOwnerHabitat(PlayerController, true);
	// 			UE_LOG(LogTemp, Log,
	// 				TEXT("[AExperimentCharacter::Server_OnExperimentStarted_Implementation] PlayerController valid!"))
	// 		}
	// 	}
	// }
	// get game mode
	// get experiment client
	// UpdateHabitatNetOwner 
}

bool AExperimentCharacter::Server_RegisterActorOwner_Validate(AActor* InActor, const bool bForceUpdate) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_RegisterActorOwner_Validate] Called!"))

	return true;
}

void AExperimentCharacter::Server_RegisterActorOwner_Implementation(AActor* InActor, const bool bForceUpdate) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_RegisterActorOwner_Implementation] Called!"))

	if (!ensure(InActor)) { return; }

	if (!HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_RegisterActorOwner_Implementation] No authority!"))
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!ensure(PlayerController)) { return; }

	if (!InActor->HasNetOwner() || bForceUpdate) {
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentGameMode::Server_RegisterActorOwner_Implementation] Updating Habitat NetOwner. Habitat does not have owner or you pass `force` flag (bForceUpdate = %i)"),
			bForceUpdate)
		InActor->SetOwner(PlayerController);
		InActor->SetReplicateMovement(true);
		TArray<AActor*> ChildActors;
		InActor->GetAllChildActors(ChildActors,true);
		for (AActor* ChildActor : ChildActors) {
			if (ChildActor) {
				ChildActor->SetOwner(PlayerController);
				ChildActor->SetReplicates(true);
				UE_LOG(LogTemp, Log,
					TEXT("[AExperimentGameMode::Server_RegisterActorOwner_Implementation] Updated owner for child actor: %s to %s (HasNetOwner?: %i)"),
					*ChildActor->GetName(),
					*PlayerController->GetName(),
					ChildActor->HasNetOwner());
			}
			if(GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
					TEXT("FINISHED SETTING HABITAT OWNER"));
		}
	}
}

void AExperimentCharacter::OnRep_Owner() {
	Super::OnRep_Owner();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::OnRep_Owner] NewOwner: %s "), *GetOwner()->GetName())
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::OnRep_Owner] NewOwner (Net): %s "), *GetNetOwner()->GetName())
}

void AExperimentCharacter::SetupSampling() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::SetupSampling] Running on Android"))
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
#if WITH_EDITOR
	UE_LOG(LogTemp, Warning,
		TEXT("[AExperimentCharacter::UpdateMovement] running with editor, forcing update to work with MetaXR simulator!"))
	if (bUseVR) {
		// todo: bUseVR - Make variable 
		FRotator HMDRotation {};
		FVector HMDLocation {};
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
		CurrentLocation = HMDLocation + this->VROrigin->GetComponentLocation();
		CurrentRotation = HMDRotation;
		UpdateRoomScaleLocation();
	}
	Server_UpdateMovement(CurrentLocation, CurrentRotation);
	return;
#endif
	if (HasAuthority()) {
		// UE_LOG(LogTemp, Warning,TEXT("[AExperimentCharacter::UpdateMovement] Ran from server!"))
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

	// UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateRoomScaleLocation] VROrigin: Location: %s | Rotation: %s"),
	// 	*VROrigin->GetComponentLocation().ToString(), *VROrigin->GetComponentRotation().ToString())
	//
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::UpdateRoomScaleLocation] Capsule: Location: %s | Rotation: %s"),
	// 	*GetCapsuleComponent()->GetComponentLocation().ToString(), *GetCapsuleComponent()->GetComponentRotation().ToString())
	//
}

void AExperimentCharacter::BeginPlay() {
	Super::BeginPlay();
	
	if (HasAuthority()) { /* is server */
		// UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] Running on server. Enabling replication."))
		// SetReplicateMovement(true);
		// SetReplicates(true);
	} else { /* is client */
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] Running on client."))
		SetupSampling();
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected()) {
			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] HMD is connected, setting origin and resetting HMD."))
			UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
			UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
		}
		// if (AExperimentPlayerControllerVR* CurrentPlayerController = Cast<AExperimentPlayerControllerVR>(GetController())) {
		// 	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] AExperimentPlayerController is valid. Changed input mode to GameOnly."))
		// 	FInputModeGameOnly InputModeData;
		// 	CurrentPlayerController->SetInputMode(InputModeData);
		// 	// CurrentPlayerController->SetShowMouseCursor(false);
		// }
	}
	
	/* Add Input Mapping Context */
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] PlayerController valid"))
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::BeginPlay] Added EnhancedInput MappingContext"))
			}
	}
}

void AExperimentCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(AExperimentCharacter, MotionControllerRight)
}

void AExperimentCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::PossessedBy] Called "))
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::PossessedBy] PossessedBy: %s"),
		NewController ? *NewController->GetName() : TEXT("Null"))
}

void AExperimentCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	SetReplicateMovement(true);
	
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
	// if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
	// 	EnhancedInputComponent->BindAction(Select)
	// }

}

bool AExperimentCharacter::Server_UpdateOwner_Validate(APlayerController* InOwnerPlayerController) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_UpdateOwner_Validate]"))
	return true;
}
void AExperimentCharacter::Server_UpdateOwner_Implementation(APlayerController* InOwnerPlayerController) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_UpdateOwner_Implementation]"))
	if (InOwnerPlayerController) {
		SetOwner(InOwnerPlayerController);
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Server_UpdateOwner_Implementation] New owner set (%s)"),
			*InOwnerPlayerController->GetName())
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentCharacter::Server_UpdateOwner_Implementation] InOwnerPlayerController is NULL"))
	}
}

void AExperimentCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (!HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] Client PlayerState PlayerName: %s"),
			GetPlayerState() ? *GetPlayerState()->GetPlayerName() : TEXT("NULL"))
	}
	
	if (AController* CurrentController = GetController()) {
		if (CurrentController->IsLocalPlayerController()) {

			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] bIsLocalPlayerController = %s"),
				CurrentController->IsLocalPlayerController() ? TEXT("true") : TEXT("false"))
			
			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] AController NetOwner: %s"),
				CurrentController->HasNetOwner() ? *CurrentController->GetNetOwner()->GetName() : TEXT("NULL (no owner)"))

			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] IsPlayerControlled: %s"),
				IsPlayerControlled() ? TEXT("true") : TEXT("false"))
			
			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] Character has Owner? %s"),
				HasNetOwner() ? *GetNetOwner()->GetName() : TEXT("NULL")) // todo: set 
			
			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] Input enabled: %s"),
				InputEnabled() ? TEXT("true") : TEXT("false"))

			UE_LOG(LogTemp, Log, TEXT("[AExperimentCharacter::Tick] bBlockInput: %s"),
					bBlockInput ? TEXT("true") : TEXT("false"))

		}
	}
	
	/*if (bUseVR) { // todo: bUseVR - Make variable 
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
	}*/
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
	UE_LOG(LogTemp, Log,TEXT("AExperimentCharacter::TouchStarted"))
	Jump();
}

void AExperimentCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location) {
	UE_LOG(LogTemp, Log,TEXT("AExperimentCharacter::TouchStopped"))
	StopJumping();
}

void AExperimentCharacter::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	UE_LOG(LogTemp, Log,TEXT("[AExperimentCharacter::TurnAtRate] %0.2f"), Rate)

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AExperimentCharacter::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	UE_LOG(LogTemp, Log,TEXT("[AExperimentCharacter::LookUpAtRate] %0.2f"), Rate)
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

