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
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h" // test 
#include "Kismet/GameplayStatics.h"

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
	HUDWidgetComponent->SetRelativeLocation(FVector(250.0f, 30.0f, -20.0f));
	HUDWidgetComponent->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
	HUDWidgetComponent->SetRelativeRotation(FRotator(0.0f, -180.0f, 0.0f));
	HUDWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HUDWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HUDWidgetComponent->SetDrawSize(FVector2d(1080, 720));

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

	const FSoftClassPath PlayerHUDClassRef(
		TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Interfaces/BP_HUDExperiment.BP_HUDExperiment_C'"));
	if (TSubclassOf<UHUDExperiment> HUDClass = PlayerHUDClassRef.TryLoadClass<UHUDExperiment>())
	{
		HUDWidgetComponent->SetWidgetClass(HUDClass);
	}
	else { UE_LOG(LogExperiment, Error, TEXT("[APawnMain::APawnMain()] Couldn't find HUD experiment.")); }
}


// Called to bind functionality to input
void APawnMain::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
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
bool APawnMain::DetectMovement()
{
	bool _blocation_updated = false;
	FVector NewLocation;
	FRotator NewRotation;

	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(NewRotation, NewLocation);
		this->UpdateRoomScaleLocation();
	}
	else
	{
		// using WASD
		if (GetWorld() && GetWorld()->GetFirstPlayerController())
		{
			NewLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		}
	}

	return true;
}

void APawnMain::UpdateRoomScaleLocation()
{
	const FVector CapsuleLocation = this->CapsuleComponent->GetComponentLocation();

	FVector CameraLocation = Camera->GetComponentLocation();
	CameraLocation.Z = 0.0f;

	FVector DeltaLocation = CameraLocation - CapsuleLocation;
	DeltaLocation.Z = 0.0f;

	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	this->CapsuleComponent->SetWorldLocation(CameraLocation);
}

void APawnMain::OnMovementDetected()
{
	FVector FinalLocation = {};
	if (bUseVR) {
		FVector HMDLocation = {};
		FRotator HMDRotation = {};
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
		FinalLocation = HMDLocation + this->VROrigin->GetComponentLocation();
		this->UpdateRoomScaleLocation(); // todo: test with VR
	} else { FinalLocation = this->RootComponent->GetComponentLocation(); }

	MovementDetectedEvent.Broadcast(FinalLocation);
}

void APawnMain::ResetOrigin()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Resetting origin.")));
}

void APawnMain::RestartGame()
{
	FName level_loading = TEXT("L_Loading");
	UGameplayStatics::OpenLevel(this, level_loading, true);
}

void APawnMain::QuitGame()
{
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

bool APawnMain::CreateAndInitializeWidget()
{
	if (!PlayerHUDClass->IsValidLowLevelFast()) { return false; }

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController->IsValidLowLevelFast());

	if (!PlayerController->IsValidLowLevelFast())
	{
		UE_LOG(LogExperiment, Error, TEXT("[APawnMain::CreateAndInitializeWidget()] PlayerController not valid."));
		return false;
	}

	PlayerHUD = CreateWidget<UHUDExperiment>(PlayerController, PlayerHUDClass);
	if (!PlayerHUD->IsValidLowLevelFast())
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			                                 FString::Printf(TEXT(
				                                 "[APawnMain::CreateAndInitializeWidget()] PlayerHUD not valid.")));
		return false;
	}

	PlayerHUD->Init();

	if (!bUseVR) { PlayerHUD->AddToViewport(); }
	else { this->HUDWidgetComponent->SetWidget(PlayerHUD); }

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald,
		                                 FString::Printf(
			                                 TEXT("[APawnMain::CreateAndInitializeWidget()] OK. Using VR? %i"),
			                                 bUseVR));

	return true;
}

// Called when the game starts or when spawned
void APawnMain::BeginPlay()
{
	Super::BeginPlay();

	if (UUserWidget* HUDWidget = HUDWidgetComponent->GetWidget()) { PlayerHUD = Cast<UHUDExperiment>(HUDWidget); }
	check(PlayerHUD->IsValidLowLevelFast());

	AGameModeBase* GameModeTemp = GetWorld()->GetAuthGameMode();
	GameMode = Cast<AGameModeMain>(GameModeTemp);
	if (GameMode->IsValidLowLevelFast()) {
		bUseVR = GameMode->bUseVR;
		GameMode->PlayerPawn = this;
	}
	float DurationIn = 1.0f / 90.0f;
	
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	
	if (EventTimer->IsValidLowLevel()) {
		EventTimer->SetRateHz(90.0f);
		EventTimer->bLoop = true;
		
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&APawnMain::OnMovementDetected);
		
		if (EventTimer->Start()) {
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("[APawnMain::BeginPlay()] Timer STARTED OK!")));
		} else {
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("[APawnMain::BeginPlay()] Timer STARTED FAILED!")));}
	}else {
		UE_LOG(LogExperiment, Error, TEXT("[APawnMain::BeginPlay()] EventTimer is NULL!"))
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("[APawnMain::BeginPlay()] Timer NULLPTR!")));
	}
	
	// GetWorld()->GetTimerManager().SetTimer(TimerHandleDetectMovement, this,
	// 		&APawnMain::OnMovementDetected,
	// 		DurationIn, true, -1.0f);
	
	// todo: fix this dumb struct 
	// EventTimer.TimerDelegate.BindUObject(this, &APawnMain::OnMovementDetected);
	// if (!EventTimer.Start()) {
	// 	UE_LOG(LogExperiment, Error, TEXT("TIMER FAILED!"));
	// }
	
	// todo: check? seems to work fine in both. will leave as-is for now bc I may need to revisit this later
	if (bUseVR) { this->Camera->bUsePawnControlRotation = false; }
	else { this->Camera->bUsePawnControlRotation = false; }
}

void APawnMain::DebugHUDAddTime() {
	DebugTimeRemaining += 1;
	if ((DebugTimeRemaining % 10 == 0) && PlayerHUD->IsValidLowLevelFast())
	{
		PlayerHUD->SetTimeRemaining(FString::FromInt(DebugTimeRemaining));
		PlayerHUD->SetCurrentStatus(FString::FromInt(DebugTimeRemaining));
	}
}

/* Called every frame:
 * todo: implement elsewhere */
void APawnMain::Tick(float DeltaTime)
{
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

void APawnMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// this->DestroyHUD();
	if (this->EventTimer->IsValidLowLevel()) {
		const bool bResultStopTimer = this->EventTimer->Stop();
		UE_LOG(LogExperiment, Log, TEXT("[APawnMain::EndPlay] Stop Timer result: %i"),bResultStopTimer)
	}
}

void APawnMain::Reset()
{
	Super::Reset();
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
	//this->RootComponent->AddWorldOffset(InputVector);
	OurMovementComponentChar->SafeMoveUpdatedComponent(
		InputVector,
		OurMovementComponentChar->UpdatedComponent->GetComponentQuat(),
		bForce,
		OutHit,
		TeleportType);
}

void APawnMain::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		if (OurMovementComponentChar && (OurMovementComponentChar->UpdatedComponent == RootComponent))
		{
			FVector CameraForwardVector = this->Camera->GetForwardVector();
			CameraForwardVector.Z = 0.0;
			this->UpdateMovementComponent(CameraForwardVector * AxisValue * 2, /*force*/ true);
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
