#include "PawnMain.h"
#include "GameModeMain.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Components/EditableTextBox.h"
#include "Components/StereoLayerComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "InstanceCulling/InstanceCullingContext.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGameModeMain* GameMode; // forward declare to avoid circular dependency

APawnMain::APawnMain() : Super() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	/* create origin for tracking */
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;

	/* create collision component */
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &APawnMain::OnOverlapBegin); // overlap events
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &APawnMain::OnOverlapEnd); // overlap events 
	CapsuleComponent->SetupAttachment(RootComponent);

	/* create camera component as root so pawn moves with camera */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetMobility(EComponentMobility::Movable);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, -CapsuleHalfHeight)); // todo: make sure this is OK
	Camera->bUsePawnControlRotation = false; // todo: add flag, true for VR
	Camera->SetupAttachment(RootComponent);

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

	// const FSoftClassPath PlayerHUDClassRef(
	// 	TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Interfaces/BP_HUDExperiment.BP_HUDExperiment_C'"));
	// PlayerHUDClass = PlayerHUDClassRef.TryLoadClass<UHUDExperiment>();
	// if (PlayerHUDClass) {
	// 	// HUDWidgetComponent->SetWidgetClass(PlayerHUDClass);
	// 	UE_LOG(LogExperiment, Log, TEXT("[APawnMain::APawnMain()] Found HUD experiment!"));
	// }
	// else {
	// 	UE_LOG(LogExperiment, Error, TEXT("[APawnMain::APawnMain()] Couldn't find HUD experiment!"));
	// }
}

// Called to bind functionality to input
void APawnMain::SetupPlayerInputComponent(class UInputComponent* InInputComponent) {
	Super::SetupPlayerInputComponent(InInputComponent);
}

UCameraComponent* APawnMain::GetCameraComponent()
{
	return this->Camera;
}

void APawnMain::DbgStartExperiment()
{
}

void APawnMain::DbgStopExperiment()
{
}

void APawnMain::DbgStopEpisode()
{
}

void APawnMain::DbgStartEpisode()
{
}

void APawnMain::ValidateHMD()
{
	return;
	// todo: get PC, if PCVR, set UseVR flag = true;
	// todo idea 2: get flag from GameMode (check if it was set first, then do PC approach)
}

bool APawnMain::DetectMovementVR()
{
	return false;
}

bool APawnMain::DetectMovementWASD()
{
	return false;
}

// todo: attach as delegate to a timer 
bool APawnMain::DetectMovement() {
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

void APawnMain::UpdateRoomScaleLocation() {
	const FVector CapsuleLocation = this->CapsuleComponent->GetComponentLocation();

	FVector CameraLocation = Camera->GetComponentLocation();
	CameraLocation.Z = 0.0f;

	FVector DeltaLocation = CameraLocation - CapsuleLocation;
	DeltaLocation.Z = 0.0f;

	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	this->CapsuleComponent->SetWorldLocation(CameraLocation);
}

void APawnMain::OnMovementDetected() {
	FVector FinalLocation = {};
	FRotator FinalRotation = {};
	if (bUseVR) {
		if ((UHeadMountedDisplayFunctionLibrary::GetHMDWornState() == EHMDWornState::Worn)) {
			FVector HMDLocation = {};
			FRotator HMDRotation = {};
			UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
			FinalLocation = HMDLocation + this->VROrigin->GetComponentLocation();
			FinalRotation = HMDRotation;
			UpdateRoomScaleLocation();
		} else {
			UE_LOG(LogExperiment, Error, TEXT("OnMovementDetected: HMD is not being worn! Using WASD mode!"))
			FinalLocation = RootComponent->GetComponentLocation();
			FinalRotation = GetActorRotation();
		}
	} else { // only gets called when in WASD 
		FinalLocation = RootComponent->GetComponentLocation();
		FinalRotation = GetActorRotation();
	}
		
	MovementDetectedEvent.Broadcast(FinalLocation, FinalRotation);
}

void APawnMain::ResetOrigin() {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue,
		FString::Printf(TEXT("Resetting origin.")));
}

void APawnMain::RestartGame() {
	FName level_loading = TEXT("L_Loading");
	UGameplayStatics::OpenLevel(this, level_loading, true);
}

void APawnMain::QuitGame() {
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red,
		                                 FString::Printf(TEXT("[APawnMain::QuitGame()] Quit game.")));
	}
}

APlayerController* APawnMain::GetGenericController()
{
	TObjectPtr<APlayerController> PlayerControllerOut = nullptr;
	if (this->IsValidLowLevelFast())
	{
		AController* ControllerTemp = this->GetController();
		PlayerControllerOut = Cast<APlayerController>(ControllerTemp);
	}
	return PlayerControllerOut;
}

bool APawnMain::HUDResetTimer(const float DurationIn) const
{
	if (!PlayerHUD->IsValidLowLevelFast()) { return false; }

	PlayerHUD->SetTimeRemaining(LexToString(DurationIn));

	return true;
}

bool APawnMain::CreateAndInitializeWidget() {

	if (!PlayerHUDClass->IsValidLowLevelFast() || !HUDWidgetComponent->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Error, TEXT("HUDClass or WidgetComponent not valid."));
		return false;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController->IsValidLowLevelFast());

	if (!PlayerController->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Error, TEXT("[APawnMain::CreateAndInitializeWidget()] PlayerController not valid."));
		return false;
	}

	PlayerHUD = CreateWidget<UHUDExperiment>(PlayerController, PlayerHUDClass);
	if (!PlayerHUD->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Error, TEXT("[APawnMain::CreateAndInitializeWidget()] PlayerHUD not valid."));
		return false;
	}
	
	HUDWidgetComponent->SetWidgetClass(PlayerHUDClass);
	
	if (!bUseVR) {
		HUDWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		// PlayerHUD->AddToViewport();
	} else {
		HUDWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
		HUDWidgetComponent->SetWidget(PlayerHUD);
	}
	
	UE_LOG(LogExperiment, Log, TEXT("CreateAndInitializeWidget: OK."));
	return true;
}

bool APawnMain::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer: Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&APawnMain::OnMovementDetected);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogExperiment, Error, TEXT("StartPositionSamplingTimer Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer OK!"))
	return true;
}

bool APawnMain::StopPositionSamplingTimer() {
	UE_LOG(LogExperiment, Log, TEXT("StopPositionSamplingTimer"))

	if (EventTimer->IsValidLowLevel()) {
		
		EventTimer->OnTimerFinishedDelegate.RemoveDynamic(this,
			&APawnMain::OnMovementDetected);
		
		if (!EventTimer->Stop()) {
			UE_LOG(LogExperiment, Error, TEXT("StopPositionSamplingTimer Failed to stop EventTimer!"))
			return false;
		}
	} else {
		UE_LOG(LogExperiment, Error, TEXT("StopPositionSamplingTimer failed to stop EventTimer! EventTimer is NULL!"))
		return false;
	}
	
	EventTimer->MarkAsGarbage();
	return true;
}

// Called when the game starts or when spawned
void APawnMain::BeginPlay() {
	Super::BeginPlay();

	// if (!this->CreateAndInitializeWidget()) {
	// 	UE_LOG(LogExperiment, Error, TEXT("Failed to init PlayerHUD widget"));
	// }

	if (!this->StartPositionSamplingTimer(90.0f)) {
		UE_LOG(LogExperiment, Error, TEXT("PawnMain: StartPositionSamplingTimer(90.0f) Failed!"))
	}else {
		UE_LOG(LogExperiment, Log, TEXT("PawnMain: StartPositionSamplingTimer(90.0f) OK!"))
	}

	// todo: check? seems to work fine in both. will leave as-is for now bc I may need to revisit this later
	if (bUseVR) { Camera->bUsePawnControlRotation = false; }
	else { Camera->bUsePawnControlRotation = true; }
}

void APawnMain::DebugHUDAddTime() {
	DebugTimeRemaining += 1;
	if ((DebugTimeRemaining % 10 == 0) && PlayerHUD->IsValidLowLevelFast())
	{
		PlayerHUD->SetTimeRemaining(FString::FromInt(DebugTimeRemaining));
		PlayerHUD->SetCurrentStatus(FString::FromInt(DebugTimeRemaining));
	}
}

void APawnMain::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void APawnMain::DestroyHUD()
{
	if (PlayerHUD)
	{
		PlayerHUD->RemoveFromParent();
		PlayerHUD = nullptr;
	}
}

void APawnMain::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	// this->DestroyHUD();
	if (this->EventTimer->IsValidLowLevel()) {
		const bool bResultStopTimer = this->EventTimer->Stop();
		UE_LOG(LogExperiment, Log, TEXT("[APawnMain::EndPlay] Stop Timer result: %i"),bResultStopTimer)
	}

	if (HUDWidgetComponent && HUDWidgetComponent->GetWidget()) {
		HUDWidgetComponent->DestroyComponent();
	}
}

void APawnMain::Reset()
{
	Super::Reset();
}

void APawnMain::UpdateMovementComponent(FVector InputVector, bool bForce) {
	OurMovementComponentChar->SafeMoveUpdatedComponent(
		InputVector,
		OurMovementComponentChar->UpdatedComponent->GetComponentQuat(),
		bForce,
		OutHit,
		TeleportType);
}

void APawnMain::MoveForward(float AxisValue) {
	if (AxisValue != 0.0f) {
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent)) {
			FVector CameraForwardVector = this->Camera->GetForwardVector();
			CameraForwardVector.Z = 0.0;
			this->UpdateMovementComponent(CameraForwardVector * AxisValue * 20, /*force*/ true);
		}
	}
}

void APawnMain::MoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector CameraRightVector = this->Camera->GetRightVector();
			CameraRightVector.Z = 0.0;
			this->UpdateMovementComponent(CameraRightVector * AxisValue * 2, /* force */true);
		}
	}
}

/* doesn't work with VR */
void APawnMain::Turn(float AxisValue)
{
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Yaw += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

/* doesn't work with VR */
void APawnMain::LookUp(float AxisValue)
{
	FRotator NewRotation = this->Camera->GetRelativeRotation();
	NewRotation.Pitch += AxisValue;
	this->Camera->SetRelativeRotation(NewRotation);
}

void APawnMain::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("[APawnMain::OnOverlapBegin()] Hit something!")));
}

void APawnMain::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             int32 OtherBodyIndex)
{
}
