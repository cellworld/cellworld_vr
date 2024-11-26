#include "ExperimentPawn.h"

// #include "../../../../../Source/BotEvadeModule/Public/Client/ExperimentClient.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Net/UnrealNetwork.h"

AExperimentPawn::AExperimentPawn() : Super() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick		   = true;
	bAlwaysRelevant                        = true;
	SetReplicates(true);
	
	// set our turn rates for input
	BaseTurnRate   = 45.f;
	BaseLookUpRate = 45.f;

	/* create origin for tracking */
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd); // overlap events 
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetIsReplicated(true);
	
	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetMobility(EComponentMobility::Movable);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalfHeight)); // todo: make sure this is OK
	Camera->bUsePawnControlRotation = false; // todo: add flag, true for VR
	Camera->SetupAttachment(RootComponent);
	Camera->SetIsReplicated(true);

	/* create HUD widget and attach to camera */
	HUDWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HUDWidget"));
	HUDWidgetComponent->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
	HUDWidgetComponent->SetOnlyOwnerSee(true);
	HUDWidgetComponent->SetVisibility(true);
	HUDWidgetComponent->SetRelativeLocation(FVector(100.0f, 0.0f, -30.0f));
	HUDWidgetComponent->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	HUDWidgetComponent->SetRelativeRotation(FRotator(0.0f, -180.0f, 0.0f));
	HUDWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HUDWidgetComponent->SetWidgetSpace(EWidgetSpace::World); 
	HUDWidgetComponent->SetDrawSize(FVector2d(1920, 1080));

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

	/* create instance of our movement component */
	OurMovementComponentChar = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("CharacterMovementComponent"));
	OurMovementComponentChar->MaxStepHeight = 100.0f;
	OurMovementComponentChar->MaxWalkSpeed = 5000.0f;
	OurMovementComponentChar->MaxAcceleration = 5000.0f;
	OurMovementComponentChar->BrakingDecelerationWalking = 4'000.0f;
	OurMovementComponentChar->bDeferUpdateMoveComponent = false;
	OurMovementComponentChar->SetActive(true);
	OurMovementComponentChar->UpdatedComponent = RootComponent;

	// Main Menu Attachment Point - Where to attach menu relative to motion controller
	MainMenuAttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MainMenuAttachmentPoint"));
	MainMenuAttachmentPoint->SetupAttachment(MotionControllerRight);
	MainMenuAttachmentPoint->SetRelativeRotation(FRotator(60.0f, -180.0f, 0.0f));
	MainMenuAttachmentPoint->SetRelativeScale3D(FVector(0.05f,0.05f,0.05f));

	// Model Spawn Point - Where to attach spawned model (anchor)
	ModelSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ModelSpawnPoint"));
	ModelSpawnPoint->SetupAttachment(MotionControllerRight);
	ModelSpawnPoint->SetRelativeLocation(FVector(7.0f,0.0f,13.0f));
	
	// XR Device Visualization Right - To visualize motion controller
	XRDeviceVisualizationRight = CreateDefaultSubobject<UXRDeviceVisualizationComponent>(TEXT("XRDeviceVisualizationRight"));
	XRDeviceVisualizationRight->SetIsVisualizationActive(true);
	XRDeviceVisualizationRight->SetVisibility(true);
	XRDeviceVisualizationRight->AttachToComponent(MotionControllerRight, FAttachmentTransformRules::KeepRelativeTransform);
	XRDeviceVisualizationRight->DisplayModelSource = FName("OculusXRHMD");
	XRDeviceVisualizationRight->MotionSource = FName("Right");

	const FString HandMeshReferencePath =
		TEXT("/Script/Engine.StaticMesh'/OculusXR/Meshes/RightMetaQuestTouchPlus.RightMetaQuestTouchPlus'");
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		HandMesh(*HandMeshReferencePath);
	if (HandMesh.Succeeded()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::AExperimentPawn] Found BP: %s"), *HandMeshReferencePath);
		XRDeviceVisualizationRight->SetCustomDisplayMesh(HandMesh.Object);
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::AExperimentPawn] Failed to load controller mesh!"))
	}

	// Spline Component - Spatial Anchoring Stuff
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComponent->SetupAttachment(MotionControllerRight);

	// Spline Mesh - Attach to Spline Component
	SplineMesh = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMesh"));
	SplineMesh->SetupAttachment(SplineComponent);

	// Anchor Menu Attachment Point
	AnchorMenuAttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AnchorMenuAttachmentPoint"));
	AnchorMenuAttachmentPoint->SetupAttachment(MotionControllerRight);
	AnchorMenuAttachmentPoint->SetRelativeLocation(FVector(0.0f,13.5f,10.0f));
	AnchorMenuAttachmentPoint->SetRelativeRotation(FRotator(0.0f,60.0f,-180.0f));
	AnchorMenuAttachmentPoint->SetRelativeScale3D(FVector(0.05f,0.05f,0.05f));
	
	XRPassthroughLayer = CreateDefaultSubobject<UOculusXRPassthroughLayerComponent>(TEXT("OculusXRPassthroughLayer"));
	if (XRPassthroughLayer) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::AExperimentPawn] XRPassthroughLayer valid"))
		// XRPassthroughLayer->SetMobility(EComponentMobility::Movable);
		// XRPassthroughLayer->SetVisibility(true);
		// XRPassthroughLayer->bLiveTexture    = false;
		// XRPassthroughLayer->bSupportsDepth  = false;
		// XRPassthroughLayer->bNoAlphaChannel = false;
		// XRPassthroughLayer->bAutoActivate   = false;
		// XRPassthroughLayer->SetPriority(0);
		// XRPassthroughLayer->SetUVRect(FBox2d{FVector2d<double>{0.0,0.0},FVector2d<double>{100.0,100.0}});
		// XRPassthroughLayer->SetQuadSize(FVector2d(0.0,100.0));
	}else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::AExperimentPawn] XRPassthroughLayer null "));
	}

	// Create a sphere component
	SphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	SphereMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	SphereMeshComponent->SetRelativeLocation(FVector(0.0f,0.0f, 182.0f)); // 182cm-> ~6ft
	SphereMeshComponent->SetRelativeScale3D(FVector(3.0f, 3.0f,3.0f));
	SphereMeshComponent->SetupAttachment(RootComponent);

	UStaticMesh* StaticMesh = CreateDefaultSubobject<UStaticMesh>(TEXT("StaticMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (SphereMeshAsset.Succeeded()) {
		UE_LOG(LogTemp, Log,TEXT("[AExperimentPawn::AExperimentPawn] Set Skeletal mesh: OK"))
		SphereMeshComponent->SetStaticMesh(SphereMeshAsset.Object);
	} else {
		UE_LOG(LogTemp,Error,TEXT("[AExperimentPawn::AExperimentPawn] Set Skeletal mesh: Failed"));
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> SphereMaterialAsset(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (SphereMaterialAsset.Succeeded()) {
		UE_LOG(LogTemp,Log,TEXT("[AExperimentPawn::AExperimentPawn] Set Material: OK"))
		SphereMeshComponent->SetMaterial(0, SphereMaterialAsset.Object);
	}else {
		UE_LOG(LogTemp, Error,TEXT("[AExperimentPawn::AExperimentPawn] Set Material: Failed"));
	}
}

void AExperimentPawn::OnRep_CurrentLocation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::OnRep_CurrentLocation] %s (Server? %i)"),
		*CurrentRotation.ToString(),HasAuthority())
}

void AExperimentPawn::OnRep_CurrentRotation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::OnRep_CurrentRotation] %s (Server? %i)"),
		*CurrentRotation.ToString(), HasAuthority())
}

// Called to bind functionality to input
void AExperimentPawn::SetupPlayerInputComponent(class UInputComponent* InInputComponent) {
	Super::SetupPlayerInputComponent(InInputComponent);
	InInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	InInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);

	InInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InInputComponent->BindAxis("TurnRate", this, &ThisClass::TurnAtRate);
	InInputComponent->BindAxis("LookUpRate", this, &ThisClass::LookUpAtRate);

}

void AExperimentPawn::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AExperimentPawn::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

UCameraComponent* AExperimentPawn::GetCameraComponent() {
	return this->Camera;
}

void AExperimentPawn::ValidateHMD()
{
	return;
	// todo: get PC, if PCVR, set UseVR flag = true;
	// todo idea 2: get flag from GameMode (check if it was set first, then do PC approach)
}

bool AExperimentPawn::DetectMovementVR()
{
	return false;
}

bool AExperimentPawn::DetectMovementWASD()
{
	return false;
}

// todo: attach as delegate to a timer 
bool AExperimentPawn::DetectMovement() {
	bool _blocation_updated = false;
	FVector NewLocation;
	FRotator NewRotation;

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()) {
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(NewRotation, NewLocation);
		this->UpdateRoomScaleLocation();
	} else { // called in WASD 
		if (GetWorld() && GetWorld()->GetFirstPlayerController()) {
			NewLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		}
	}

	return true;
}

void AExperimentPawn::Server_UpdateMovement_Implementation(const FVector& InLocation,const FRotator& InRotation) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Server_UpdateMovement_Implementation] InLocation: %s | InRotation: %s"),
		*InLocation.ToString(), *InRotation.ToString())

	APlayerController* OwnerController = Cast<APlayerController>(GetOwner());
	if (OwnerController) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Server_UpdateMovement_Implementation] Has Owner"))
	}

	Move();
	
	// should always be true 
	if (!HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Server_UpdateMovement_Implementation] No Authority"))
		return;
	}

	// if (this->BotEvadeGameMode && this->BotEvadeGameMode->ExperimentClient) {
	// 	this->BotEvadeGameMode->ExperimentClient->UpdatePreyPosition(InLocation,InRotation);
	// 	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Server_UpdateMovement_Implementation] Sent frame!"))
	// }
}

void AExperimentPawn::Multi_UpdateMovement_Implementation(const FVector& InLocation, const FRotator& InRotation) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Multi_UpdateMovement_Implementation] InLocation: %s | InRotation: %s"),
	*InLocation.ToString(), *InRotation.ToString())
}

bool AExperimentPawn::Server_UpdateMovement_Validate(const FVector& InLocation,const FRotator& InRotation) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::Server_UpdateMovement_Validate] InLocation: %s | InRotation: %s"),
		*InLocation.ToString(), *InRotation.ToString())
	
	APlayerController* OwnerController = Cast<APlayerController>(GetOwner());
	if (!OwnerController) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentPawn::Server_UpdateMovement_Validate] No valid owner!"));
		return false;
	}
	
	UE_LOG(LogTemp, Log,
		TEXT("[AExperimentPawn::Server_UpdateMovement_Validate] OwnerController: %s. Returning true"),
		*OwnerController->GetName());

	return true;
}

void AExperimentPawn::OnMovementDetected() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentPawn::OnMovementDetected] DEPRECATED"))
	FVector FinalLocation = {};
	FRotator FinalRotation = {};
#if PLATFORM_ANDROID
	UE_LOG(LogTemp, Log, TEXT("[OnMovementDetected] PLATFORM_ANDROID"))
	if ((UHeadMountedDisplayFunctionLibrary::GetHMDWornState() == EHMDWornState::Worn)) {
		UE_LOG(LogTemp, Log, TEXT("[OnMovementDetected] HMD worn"))
		FVector HMDLocation = {};
		FRotator HMDRotation = {};
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
		CurrentLocation = HMDLocation + this->VROrigin->GetComponentLocation();
		CurrentRotation = HMDRotation;
		UpdateRoomScaleLocation();

	}else {
		UE_LOG(LogTemp, Error, TEXT("[OnMovementDetected] HMD is not being worn! Using base."))
		CurrentLocation = RootComponent->GetComponentLocation();
		CurrentRotation = GetActorRotation();
	}
#elif PLATFORM_DESKTOP
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::OnMovementDetected] PLATFORM_DESKTOP"))
	CurrentLocation = RootComponent->GetComponentLocation();
	CurrentRotation = GetActorRotation();
#endif	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::OnMovementDetected] location: (%s); Rotation: (%s)"),
		*CurrentLocation.ToString(), *CurrentRotation.ToString())
	// Multi_UpdateMovement.Broadcast(FinalLocation, FinalRotation);
}

void AExperimentPawn::UpdateMovement() {
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning,TEXT("[AExperimentPawn::UpdateMovement] Ran from server!"))
	} else {
		FRotator HMDRotation {};
		FVector HMDLocation {};
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
		CurrentLocation = HMDLocation + this->VROrigin->GetComponentLocation();
		CurrentRotation = HMDRotation;
		UpdateRoomScaleLocation();
		Server_UpdateMovement(CurrentLocation, CurrentRotation);
	}
	UE_LOG(LogTemp, Warning,TEXT("[AExperimentPawn::UpdateMovement] MultiDelegate_ called!"))
	MultiDelegate_UpdateMovement.Broadcast(CurrentLocation, CurrentRotation);
}

APlayerController* AExperimentPawn::GetGenericController() {
	TObjectPtr<APlayerController> PlayerControllerOut = nullptr;
	if (this->IsValidLowLevelFast())
	{
		AController* ControllerTemp = this->GetController();
		PlayerControllerOut = Cast<APlayerController>(ControllerTemp);
	}
	return PlayerControllerOut;
}

bool AExperimentPawn::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::StartPositionSamplingTimer]"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::StartPositionSamplingTimer] Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&AExperimentPawn::UpdateMovement);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::StartPositionSamplingTimer] StartPositionSamplingTimer Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::StartPositionSamplingTimer] OK!"))
	return true;
}

bool AExperimentPawn::StopPositionSamplingTimer() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::StopPositionSamplingTimer]"))

	if (EventTimer->IsValidLowLevel()) {
		EventTimer->OnTimerFinishedDelegate.RemoveDynamic(this,
			&AExperimentPawn::UpdateMovement);
		if (!EventTimer->Stop()) {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::StopPositionSamplingTimer] Failed to stop EventTimer!"))
			return false;
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::StopPositionSamplingTimer] failed to stop EventTimer! EventTimer is NULL!"))
		return false;
	}
	
	EventTimer->MarkAsGarbage();
	return true;
}

void AExperimentPawn::SetupSampling() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::BeginPlay] Running on Android"))
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Stage);
	Camera->bUsePawnControlRotation = false;
	// ReSharper disable once CppTooWideScopeInitStatement
	constexpr float FS = 60.0f;
	if (!ensure(this->StartPositionSamplingTimer(FS))) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentPawn::SetupSampling] StartPositionSamplingTimer(%0.2f) Failed!"), FS)
	} else {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::SetupSampling] StartPositionSamplingTimer(%0.2f) OK!"), FS)
	}

	bUseVR = true; // todo: temporary!!!!!
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::BeginPlay] bUseVR = %i"), bUseVR)
}

// Called when the game starts or when spawned
void AExperimentPawn::BeginPlay() {
	Super::BeginPlay();
#if !UE_SERVER
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::BeginPlay] Running on client."))
	SetupSampling();
	SetReplicates(true);
	SetReplicateMovement(true);
#endif
}

void AExperimentPawn::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const {
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME(AExperimentPawn, SphereMeshComponent);
	DOREPLIFETIME(AExperimentPawn, Camera);
	DOREPLIFETIME(AExperimentPawn, CurrentLocation);
	DOREPLIFETIME(AExperimentPawn, CurrentRotation);
}

void AExperimentPawn::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (this->GetController()) {
		UE_LOG(LogTemp, Log, TEXT("Pawn %s is possessed by %s"), *this->GetName(), *this->GetController()->GetName());
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Pawn %s is not possessed"), *this->GetName());
	}
	// Move();
}

void AExperimentPawn::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (this->EventTimer->IsValidLowLevel()) {
		const bool bResultStopTimer = this->EventTimer->Stop();
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::EndPlay] Stop Timer result: %i"),bResultStopTimer)
	}
}

void AExperimentPawn::Reset()
{
	Super::Reset();
}

void AExperimentPawn::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	if (NewController){
		UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::PossessedBy] %s"), *NewController->GetName())
	}
}

void AExperimentPawn::UnPossessed() {
	Super::UnPossessed();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentPawn::UnPossessed] Called"))
}

void AExperimentPawn::UpdateMovementComponent(FVector InputVector, bool bForce) {
	OurMovementComponentChar->SafeMoveUpdatedComponent(
		InputVector,
		OurMovementComponentChar->UpdatedComponent->GetComponentQuat(),
		bForce,
		OutHit,
		TeleportType);
}

void AExperimentPawn::MoveForward(float AxisValue) {
	if (AxisValue != 0.0f) {
		UE_LOG(LogTemp,Log,TEXT("MoveForward! AxisValue: %0.2f"), AxisValue)
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent)) {
			UE_LOG(LogTemp,Log,TEXT("MoveForward! Attempting to move! "))
			FVector CameraForwardVector = this->Camera->GetForwardVector();
			CameraForwardVector.Z = 0.0;
			this->UpdateMovementComponent(CameraForwardVector * AxisValue * 10, /*force*/ true);
		}
	}
}

void AExperimentPawn::MoveRight(float AxisValue) {
	if (AxisValue != 0.0f) {
		UE_LOG(LogTemp,Log,TEXT("MoveRight! AxisValue: %0.2f"), AxisValue)

		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent)) {
			UE_LOG(LogTemp,Log,TEXT("MoveRight! Attempting to move! "))

			FVector CameraRightVector = this->Camera->GetRightVector();
			CameraRightVector.Z = 0.0;
			this->UpdateMovementComponent(CameraRightVector * AxisValue * 10, /* force */true);
		}
	}
}

/* doesn't work with VR */
void AExperimentPawn::Turn(float AxisValue) {
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Yaw += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

/* doesn't work with VR */
void AExperimentPawn::LookUp(float AxisValue) {
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Pitch += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

void AExperimentPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("[AExperimentPawn::OnOverlapBegin()] Hit something!")));
}

void AExperimentPawn::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             int32 OtherBodyIndex)
{
}
