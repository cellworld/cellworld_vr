#include "ExperimentComponents/BotEvadePawn.h"
#include "cellworld_vr/cellworld_vr.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"


ABotEvadePawn::ABotEvadePawn() {
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ABotEvadePawn::SetupUpdateRoomScaleLocation(UCameraComponent* InCameraComponent) {

	if (!ensure(InCameraComponent->IsValidLowLevelFast())) {
		UE_LOG(LogExperiment, Error,
			TEXT("[ABotEvadePawn::SetupUpdateRoomScaleLocation] InCameraComponent NULL!"))
	}

	CameraUpdateRoomscaleLocation = InCameraComponent;
	bSetupUpdateRoomScaleLocationComplete = true; 
}

void ABotEvadePawn::UpdateRoomScaleLocation() {
	if (!bSetupUpdateRoomScaleLocationComplete) {
		UE_LOG(LogExperiment, Error,
					TEXT("[ABotEvadePawn::UpdateRoomScaleLocation] bSetupUpdateRoomScaleLocationComplete false!"))
		return; 
	}
	if (!ensure(CapsuleComponent->IsValidLowLevelFast())) { return; }
	if (!ensure(CameraUpdateRoomscaleLocation->IsValidLowLevelFast())) { return; }
	const FVector CapsuleLocation = this->CapsuleComponent->GetComponentLocation();

	FVector CameraLocation = CameraUpdateRoomscaleLocation->GetComponentLocation();
	CameraLocation.Z = 0.0f;

	FVector DeltaLocation = CameraLocation - CapsuleLocation;
	DeltaLocation.Z = 0.0f;

	AddActorWorldOffset(DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	VROrigin->AddWorldOffset(-DeltaLocation, false, nullptr, ETeleportType::TeleportPhysics);
	this->CapsuleComponent->SetWorldLocation(CameraLocation);
}

void ABotEvadePawn::OnMovementDetected() {
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
			// UE_LOG(LogExperiment, Error, TEXT("[OnMovementDetected] HMD is not being worn! Returning."))
			FinalLocation = RootComponent->GetComponentLocation();
			FinalRotation = GetActorRotation();
		}
	} else { // only gets called when in WASD 
		FinalLocation = RootComponent->GetComponentLocation();
		FinalRotation = GetActorRotation();
	}
		
	MovementDetectedEvent.Broadcast(FinalLocation, FinalRotation);
}

bool ABotEvadePawn::StartPositionSamplingTimer(const float InRateHz) {
	UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer"))
	EventTimer = NewObject<UEventTimer>(this, UEventTimer::StaticClass());
	if (EventTimer->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer: Starting at %0.2f Hz."), InRateHz)
		EventTimer->SetRateHz(InRateHz); //todo: make sampling rate GI variable (or somewhere relevant) 
		EventTimer->bLoop = true;
		
		EventTimer->OnTimerFinishedDelegate.AddDynamic(this,
			&ABotEvadePawn::OnMovementDetected);
		
		if (!EventTimer->Start()) { return false; }
	} else {
		UE_LOG(LogExperiment, Error, TEXT("StartPositionSamplingTimer Failed! EventTimer is NULL!"))
		return false;
	}
	
	UE_LOG(LogExperiment, Log, TEXT("StartPositionSamplingTimer OK!"))
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
	constexpr float FS = 60.0f;
	if (!ensure(this->StartPositionSamplingTimer(FS))) {
		UE_LOG(LogExperiment, Error, TEXT("PawnMain: StartPositionSamplingTimer(%0.2f) Failed!"), FS)
	} else {
		UE_LOG(LogExperiment, Log, TEXT("PawnMain: StartPositionSamplingTimer(%0.2f) OK!"), FS)
	}
}

void ABotEvadePawn::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
void ABotEvadePawn::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (this->EventTimer->IsValidLowLevel()) {
		const bool bResultStopTimer = this->EventTimer->Stop();
		UE_LOG(LogExperiment, Log, TEXT("[APawnMain::EndPlay] Stop Timer result: %i"),bResultStopTimer)
	}
}
void ABotEvadePawn::Reset() { Super::Reset(); }
void ABotEvadePawn::MoveForward(float AxisValue) {}
void ABotEvadePawn::MoveRight(float AxisValue) {}
void ABotEvadePawn::Turn(float AxisValue) {}
void ABotEvadePawn::LookUp(float AxisValue) {}
