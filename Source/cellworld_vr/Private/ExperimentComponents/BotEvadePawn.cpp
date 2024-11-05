#include "ExperimentComponents/BotEvadePawn.h"
#include "cellworld_vr/cellworld_vr.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"

ABotEvadePawn::ABotEvadePawn() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(30.0f, 175.0f/2);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
}

void ABotEvadePawn::SetupUpdateRoomScaleLocation(UCameraComponent* InCameraComponent) {
	UE_LOG(LogExperiment, Log,
				TEXT("[ABotEvadePawn::SetupUpdateRoomScaleLocation] Called!"))
	if (!ensure(InCameraComponent->IsValidLowLevelFast())) {
		UE_LOG(LogExperiment, Error,
			TEXT("[ABotEvadePawn::SetupUpdateRoomScaleLocation] InCameraComponent NULL!"))
		return; 
	}

	CameraUpdateRoomscaleLocation = InCameraComponent;
	bSetupUpdateRoomScaleLocationComplete = true;
	UE_LOG(LogExperiment, Log,
			TEXT("[ABotEvadePawn::SetupUpdateRoomScaleLocation] OK!"))
}

void ABotEvadePawn::UpdateRoomScaleLocation() {
	if (!bSetupUpdateRoomScaleLocationComplete) {
		UE_LOG(LogExperiment, Error,
					TEXT("[ABotEvadePawn::UpdateRoomScaleLocation] bSetupUpdateRoomScaleLocationComplete false!"))
		return; 
	}
	if (!ensure(CapsuleComponent->IsValidLowLevelFast())) { return; }
	if (!ensure(CameraUpdateRoomscaleLocation->IsValidLowLevelFast())) { return; }

	UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::UpdateRoomScaleLocation] Entered OK"))
	const FVector CapsuleLocation = this->CapsuleComponent->GetComponentLocation();

	FVector CameraLocation = CameraUpdateRoomscaleLocation->GetComponentLocation();
	CameraLocation.Z = 0.0f;

	FVector DeltaLocation = CameraLocation - CapsuleLocation;
	DeltaLocation.Z = 0.0f;

	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::None);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::None);
	this->CapsuleComponent->SetWorldLocation(CameraLocation);
}

void ABotEvadePawn::OnMovementDetected() {
	if (!ensure(bSetupUpdateRoomScaleLocationComplete)) {
		return;
	}
	FVector FinalLocation = {};
	FRotator FinalRotation = {};
	if (true) { // bUseVR
		if ((UHeadMountedDisplayFunctionLibrary::GetHMDWornState() == EHMDWornState::Worn)) {
			FVector HMDLocation = {};
			FRotator HMDRotation = {};
			UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
			FinalLocation = HMDLocation + this->VROrigin->GetComponentLocation();
			FinalRotation = HMDRotation;
			UpdateRoomScaleLocation();
		} else {
			UE_LOG(LogExperiment, Error, TEXT("[ABotEvadePawn::OnMovementDetected] HMD is not being worn! Returning."))
			FinalLocation = RootComponent->GetComponentLocation();
			FinalRotation = GetActorRotation();
		}
	} else { // only gets called when in WASD 
		FinalLocation = RootComponent->GetComponentLocation();
		FinalRotation = GetActorRotation();
	}
	if (MovementDetectedEvent.IsBound()) {
		MovementDetectedEvent.Broadcast(FinalLocation, FinalRotation);
	}
	// else {
	// 	UE_LOG(LogExperiment, Warning, TEXT("[ABotEvadePawn::OnMovementDetected] "
	// 								  "No functions bound to MovementDetectedEvent!"))
	// }
}

bool ABotEvadePawn::StartPositionSamplingTimer(const float InRateHz) {
	if (!ensure(bSetupUpdateRoomScaleLocationComplete)) {
		return false;
	}

	UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::StartPositionSamplingTimer] StartPositionSamplingTimer"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::StartPositionSamplingTimer] Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&ABotEvadePawn::OnMovementDetected);
		
		if (!EventTimer->Start()) {
			UE_LOG(LogExperiment, Error, TEXT("[ABotEvadePawn::StartPositionSamplingTimer:] Failed! EventTimer->Start()!"))
			return false;
		}
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[ABotEvadePawn::StartPositionSamplingTimer:] Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::StartPositionSamplingTimer:] StartPositionSamplingTimer OK!"))
	return true;
}

bool ABotEvadePawn::StopPositionSamplingTimer() {
	UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::StopPositionSamplingTimer] StopPositionSamplingTimer"))

	if (EventTimer->IsValidLowLevel()) {
		
		EventTimer->OnTimerFinishedDelegate.RemoveDynamic(this,
			&ABotEvadePawn::OnMovementDetected);
		
		if (!EventTimer->Stop()) {
			UE_LOG(LogExperiment, Error, TEXT("[ABotEvadePawn::StopPositionSamplingTimer] Failed to stop EventTimer!"))
			return false;
		}
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[ABotEvadePawn::StopPositionSamplingTimer] failed to stop EventTimer! EventTimer is NULL!"))
		return false;
	}
	
	EventTimer->MarkAsGarbage();
	return true;
}

void ABotEvadePawn::BeginPlay() {
	Super::BeginPlay();
	if (!bSetupUpdateRoomScaleLocationComplete){ return; }
	constexpr float FS = 60.0f;
	if (!ensure(this->StartPositionSamplingTimer(FS))) {
		UE_LOG(LogExperiment, Error, TEXT("ABotEvadePawn: StartPositionSamplingTimer(%0.2f) Failed!"), FS)
	} else {
		UE_LOG(LogExperiment, Log, TEXT("ABotEvadePawn: StartPositionSamplingTimer(%0.2f) OK!"), FS)
	}
}

void ABotEvadePawn::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABotEvadePawn::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (this->EventTimer->IsValidLowLevel()) {
		const bool bResultStopTimer = this->EventTimer->Stop();
		UE_LOG(LogExperiment, Log, TEXT("[ABotEvadePawn::EndPlay] Stop Timer result: %i"),bResultStopTimer)
	}
}
// void ABotEvadePawn::Reset() { Super::Reset(); }
// void ABotEvadePawn::MoveForward(float AxisValue) {}
// void ABotEvadePawn::MoveRight(float AxisValue) {}
// void ABotEvadePawn::Turn(float AxisValue) {}
// void ABotEvadePawn::LookUp(float AxisValue) {}
