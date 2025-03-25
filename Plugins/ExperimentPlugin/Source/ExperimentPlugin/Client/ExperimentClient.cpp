#include "ExperimentClient.h"
// ReSharper disable CppTooWideScopeInitStatement
#include "Engine/CoreSettings.h"
#include "Net/UnrealNetwork.h"
#include "ExperimentPlugin/DataManagers/ExperimentManager.h"
#include "Sound/SoundCue.h"

AExperimentClient::AExperimentClient() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> OnCaptureCueLoad(
			 TEXT("SoundCue'/Game/SoundFX/fail_sound_cue.fail_sound_cue'")
		 );

	if (OnCaptureCueLoad.Object != nullptr) {
		OnCaptureSoundCue = OnCaptureCueLoad.Object;
	}

	// OnCaptureSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("OnCaptureSoundComponent"));
	// OnCaptureSoundComponent->SetAutoActivate(false);
	//
	// if (OnCaptureSoundCue->IsValidLowLevelFast() && OnCaptureSoundComponent->IsValidLowLevel()) {
	// 	OnCaptureSoundComponent->SetSound(OnCaptureSoundCue);
	// 	OnCaptureSoundComponent->SetVolumeMultiplier(3.0f);
	// }
}

void AExperimentClient::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExperimentClient, PredatorBasic);
	DOREPLIFETIME(AExperimentClient, OcclusionsStruct);
	DOREPLIFETIME(AExperimentClient, OnCaptureSoundCue);
}

void AExperimentClient::OnExperimentFinished(const int InPlayerIndex) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnExperimentFinished]"))
}

bool AExperimentClient::Server_SpawnOcclusions_Validate() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Server_SpawnOcclusions_Validate]"))
	return true;
}

void AExperimentClient::Server_SpawnOcclusions_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Server_SpawnOcclusions_Implementation]"))
	OcclusionsStruct.SpawnAll(GetWorld(), true, false, OffsetOriginTransform);
}

void AExperimentClient::SetWorldOrigin(const FVector& InWorldOriginA, const FVector& InWorldOriginB) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetWorldOrigin] OriginA: %s | OriginB: %s"),
		*InWorldOriginA.ToString(),
		*InWorldOriginB.ToString())

	if (!TrackingClient) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetWorldOrigin] TrackingClient is NULL"));
		return;
	}

	if (!TrackingClient->IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetWorldOrigin] TrackingClient not connected!"));
		return;
	}

	FString OriginInfo = FString::Printf(TEXT("%0.4f,%0.4f,%0.4f,%0.4f"),
		InWorldOriginA.X, InWorldOriginA.Y,
		InWorldOriginB.X, InWorldOriginB.Y);

	const FMessage MessageOrigin = UMessageClient::NewMessage("set_vr_origin",OriginInfo);
	// Message.header = "set_vr_origin";
	// Message.body = OriginInfo;
	// const FMessage MessageOut = UMessageClient::NewMessage("prey_step", UExperimentUtils::StepToJsonString(Step));

	if (!TrackingClient->SendMessage(MessageOrigin)) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetWorldOrigin] Failed to send."))
	}else { UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetWorldOrigin] Send origin OK!")) }
}

bool AExperimentClient::Server_AttachOcclusionsToArena_Validate() {
	return true; 
}

void AExperimentClient::Server_AttachOcclusionsToArena_Implementation() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Server_AttachOcclusionsToArena_Implementation]"))
}

bool AExperimentClient::Server_PlayCaptureSound_Validate() {
	return true; 
}

void AExperimentClient::Server_PlayCaptureSound_Implementation() {
	if (OnCaptureSoundCue->IsValidLowLevelFast() && PredatorBasic->IsValidLowLevelFast()) {
		const FVector OnCaptureSoundLocation = PredatorBasic->GetActorLocation();
		UE_LOG(LogTemp, Log, TEXT("[Server_PlayCaptureSound_Implementation] Playing sound at location: %s!"),
			*OnCaptureSoundLocation.ToString())
		Multicast_PlayCaptureSound(OnCaptureSoundLocation);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[Server_PlayCaptureSound_Implementation] Sound not valid!"))
}

bool AExperimentClient::Multicast_PlayCaptureSound_Validate(const FVector Location) {
	UE_LOG(LogTemp, Log, TEXT("[Multicast_PlayCaptureSound_Validate]"))
	return true;
}

void AExperimentClient::Multicast_PlayCaptureSound_Implementation(const FVector Location) {
	if (OnCaptureSoundCue->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Log, TEXT("[Multicast_PlayCaptureSound_Implementation] Playing sound!"))
		UGameplayStatics::PlaySoundAtLocation(this,OnCaptureSoundCue, Location);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[Multicast_PlayCaptureSound_Implementation] Sound not valid!"))
}

//TODO - add argument to include MessageType (Log, Warning, Error, Fatal)
void printScreen(const FString InMessage) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s"), *InMessage));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *InMessage);
}

bool AExperimentClient::ValidateLevel(UWorld* InWorld, const FString InLevelName) {
	if (!InWorld->IsValidLowLevelFast()) {
		printScreen("[AExperimentClient::ValidateLevel] World is not valid.");
		return false;
	}

	if (!InLevelName.IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("LevelName: %s"), *InLevelName); }
	else {
		printScreen("[AExperimentClient::ValidateLevel] LevelName is empty!");
		return false;
	}

	FString MapName = InWorld->GetMapName();
	MapName.RemoveFromStart(InWorld->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Warning, TEXT("map name: %s"), *MapName);

	return true;
}

/* todo: make this take input ACharacter and spawn that one*/
bool AExperimentClient::SpawnAndPossessPredator() {
	if (!GetWorld()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SpawnAndPossessPredator] GetWorld() failed!"));
		return false;
	}
	
	// Define spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Specify the location and rotation for the new actor
	const FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation
	FLocation SpawnLocation;
	SpawnLocation.x = 0.9;
	SpawnLocation.y = 0.5;

	const FVector SpawnVector = UExperimentUtils::CanonicalToVrV2(SpawnLocation, this->MapLength, this->WorldScale);
	const FVector SpawnVectorAdjusted = SpawnVector;

	this->PredatorBasic = GetWorld()->SpawnActor<AExperimentPredator>(
		AExperimentPredator::StaticClass(),
		SpawnVectorAdjusted, Rotation, SpawnParams);

	if (!ensure(this->PredatorBasic)) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::SpawnAndPossessPredator()] Spawn APredatorBasic Failed!"));
		return false;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SpawnAndPossessPredator] Predator Scale (WS*SF): %0.2f"),
		OffsetOriginTransform.GetScale3D().X * PredatorScaleFactor);
	
	this->PredatorBasic->SetActorScale3D(
		FVector(1.0f, 1.0f, 1.0f) * PredatorScaleFactor * OffsetOriginTransform.GetScale3D().X);
	this->SetPredatorIsVisible(false);
	return true;
}

/* stop connection for ClientIn */
bool AExperimentClient::Disconnect(UMessageClient* ClientIn) {
	if (ClientIn->IsValidLowLevelFast() && ClientIn->IsConnected()) {
		printScreen("[AExperimentClient::Disconnect] Client Disconnected: Processing.");
		return ClientIn->Disconnect();
	}
	printScreen("[AExperimentClient::Disconnect] Client Disconnected: Failed.");
	return false;
}

UMessageClient* AExperimentClient::CreateNewClient() {
	UMessageClient* NewClient = UMessageClient::NewMessageClient();
	return NewClient;
}

bool AExperimentClient::StopExperiment(const FString& ExperimentNameIn) {
	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "[AExperimentClient::StopExperiment] Can't stop experiment, Experiment Service client not valid."
		       ));
		return false;
	}

	if (ValidateClient(TrackingClient)) {
		FMessage Message;
		Message.header = "close";
		Message.body = "";
		if (!TrackingClient->SendMessage(Message)) {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::StopExperiment] Failed to send."))
		}
	}
	return true;
}

/* start experiment service episode stream
 * should ONLY be called from game mode by the BP_door trigger !!!  */
bool AExperimentClient::StartEpisode() {
	if (!ensure(ExperimentManager)) { return false; }
	if (ExperimentManager->IsInEpisode()) {
		UE_LOG(LogTemp, Log,
		       TEXT("[AExperimentClient::StartEpisode] Failed. ExperimentManager->bInEpisode = true"))
		return false;
	}
	/* start new episode */
	if (!this->ResetTrackingAgent()) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::StartEpisode] ResetTrackingAgent() Call Failed!"))
	}
	else {
		UE_LOG(LogTemp, Log,
		       TEXT("[AExperimentClient::StartEpisode] ResetTrackingAgent() Call OK!"))
	}

	return true;
}

bool AExperimentClient::ValidateClient(UMessageClient* ClientIn) {
	if (!ClientIn->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::ValidateClient] Client not valid."));
		return false;
	}

	if (!ClientIn->IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::ValidateClient] Client not connected."));
		return false;
	}
	return true;
}

bool AExperimentClient::ValidateExperimentName(const FString& ExperimentNameIn) {
	if (ExperimentNameIn == "") { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentClient::StopEpisode(const bool bForce) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::StopEpisode] Called; bForce = %i"), bForce)

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return false; }

	if (!ExperimentManager->IsInEpisode()) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::StopEpisode] Can't stop episode, not in episode. Status: %i"),
		       ExperimentInfo.Status);
		ExperimentManager->OnEpisodeFinishedFailedDelegate.Broadcast("Not in episode.");
		return false;
	}

	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogTemp, Fatal,
		       TEXT("[AExperimentClient::StopEpisode] TrackingClient not valid! Failed to send STOP!"));
	}

	StopEpisodeRequest = TrackingClient->SendRequest("stop", "", 15.0f);
	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return false; }
	StopEpisodeRequest->AddToRoot();

	StopEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentClient::HandleStopEpisodeRequestResponse);
	StopEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentClient::HandleStopEpisodeRequestTimedOut);

	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::StopEpisode] StopEpisode sent OK"));
	return true;
}

/* Handle episode response */
void AExperimentClient::HandleStartEpisodeRequestResponse(const FString response) {
	UE_LOG(LogTemp, Warning, TEXT("[HandleStartEpisodeRequestResponse] %s"), *response);

	if (!ensure(response == "success")) {
		UE_LOG(LogTemp, Error,
		       TEXT("[HandleStartEpisodeRequestResponse] Failed to start episode! Unkown reason!"))
		// ExperimentInfo.SetStatus(EExperimentStatus::ErrorStartEpisode);
		this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");
		StartEpisodeRequest->RemoveFromRoot();
		return;
	}

	if (!ensure(StartEpisodeRequest)) { return; }
	this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");
	StartEpisodeRequest->RemoveFromRoot();

	UE_LOG(LogTemp, Log, TEXT("[HandleStartEpisodeRequestResponse] Preparing to ResetAgentTracking!"))
	if (!this->ResetTrackingAgent()) {
		UE_LOG(LogTemp, Error,
		       TEXT("[HandleStartEpisodeRequestResponse] ResetTrackingAgent() Failed!"))
	}
	else {
		UE_LOG(LogTemp, Log,
		       TEXT("[HandleStartEpisodeRequestResponse] ResetTrackingAgent() OK!"))
	}
}

// essentially "StartEpisode"
bool AExperimentClient::ResetTrackingAgent() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::ResetTrackingAgent] Preparing to send!"))

	ResetRequest = TrackingClient->SendRequest("reset", "", 10.0f);
	if (!ensureMsgf(ResetRequest, TEXT("[AExperimentClient::ResetTrackingAgent] ResetRequest is NULL"))) { return false; }
	ResetRequest->AddToRoot();

	ResetRequest->ResponseReceived.AddDynamic(this, &AExperimentClient::HandleResetRequestResponse);
	ResetRequest->TimedOut.AddDynamic(this, &AExperimentClient::HandleResetRequestTimedOut);
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::ResetTrackingAgent] Sent OK!"))
	return true;
}

void AExperimentClient::HandleStartEpisodeError(const FString InMessage) {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::HandleStartEpisodeError] %s"), *InMessage)
	ResetWorldState();
}

void AExperimentClient::HandleResetRequestResponse(const FString InResponse) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleResetRequestResponse] Response: %s"),
	       *InResponse)

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }

	if (InResponse != "success") {
		// ExperimentInfo.SetStatus(EExperimentStatus::ErrorResetTrackingAgent);
		ExperimentManager->SetInEpisode(false);
		ExperimentManager->OnEpisodeStartedFailedDelegate.Broadcast(
			"[HandleResetRequestResponse] did not receive 'success'");
		if (ResetRequest) {
			RequestRemoveDelegates(ResetRequest, "ResetRequest");
			ResetRequest->RemoveFromRoot();
		}
		return;
	}

	ExperimentManager->OnProcessStartEpisodeResponseDelegate.Broadcast();
	if (ensure(ResetRequest)) {
		this->RequestRemoveDelegates(ResetRequest, "ResetRequest");
		ResetRequest->RemoveFromRoot();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleResetRequestResponse] Exiting OK"))
	// if (this->SpawnAndPossessPredator()) { UE_LOG(LogTemp, Log, TEXT("Spawned predator: OK")); }
	// else { UE_LOG(LogTemp, Error, TEXT("Spawned predator: FAILED")); }
	Server_PlayCaptureSound();
	bResetSuccessDbg = true;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppPassValueParameterByConstReference
void AExperimentClient::OnTimedOut(const FString InMessage) {
	UE_LOG(LogTemp, Fatal, TEXT("[AExperimentClient::OnTimedOut] %s"), *InMessage);
	ResetWorldState();
	// todo: start process restart connection and game 	
}

void AExperimentClient::HandleResetRequestTimedOut() {
	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }

	if (ResetRequest) {
		this->RequestRemoveDelegates(ResetRequest, "ResetRequest");
		ResetRequest->RemoveFromRoot();
	}
	
	UE_LOG(LogTemp, Error, TEXT("[HandleResetRequestTimedOut] Reset request timed out!"))
	UE_LOG(LogTemp, Log, TEXT("[HandleResetRequestTimedOut] Broadcasting OnTimedOutDelegate"))
	ExperimentManager->OnTimedOutDelegate.Broadcast("ResetRequest timed out!");
}

void AExperimentClient::OnSubscribeResult(const bool bSubscribeResult) {
	UE_LOG(LogTemp, Log, TEXT("OnSubscribeResult: %i"), bSubscribeResult)
	if (bSubscribeResult) { UE_LOG(LogTemp, Log, TEXT("Subscription successful!")) }
	else { UE_LOG(LogTemp, Error, TEXT("Subscription Failed!")) }
}

void AExperimentClient::OnResetResult(const bool bResetResult) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnResetResult] DEPRECATED"))
}

void AExperimentClient::SetPredatorIsVisible(const bool bNewVisibility) {
	if (this->PredatorBasic->IsValidLowLevelFast()) {
		this->PredatorBasic->SetActorHiddenInGame(!bNewVisibility);
		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetPredatorIsVisible] IsActorHiddenInGame(%i)"),
		       !bNewVisibility);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetActorHiddenInGame] PredatorBasic NULL"));
	}
}

void AExperimentClient::HandleStopEpisodeError(const FString InMessage) {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::HandleStopEpisodeError] %s"), *InMessage)
	ExperimentInfo.SetStatus(EExperimentStatus::ErrorFinishEpisode);
	// todo: move to OnEpisodeFinishedFailedDelegateHandler
	ResetWorldState();
}

void AExperimentClient::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error,
	       TEXT("[AExperimentClient::HandleStartEpisodeRequestTimedOut] Episode request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
}

void AExperimentClient::HandleStopEpisodeRequestResponse(const FString response) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleStopEpisodeRequestResponse] %s"),
	       *response);

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::HandleStopEpisodeRequestResponse] ExperimentManager is NULL!"));
		return;
	}

	if (response != "success") {
		ExperimentManager->OnEpisodeFinishedFailedDelegate.Broadcast(
			"[HandleStopEpisodeRequestResponse] did not receive 'success'");
		return;
	}
	ExperimentManager->OnProcessStopEpisodeResponseDelegate.Broadcast();
	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return; }
	RequestRemoveDelegates(StopEpisodeRequest, "StopEpisodeRequest");
	StopEpisodeRequest->RemoveFromRoot();
}

/* handle experiment service timeout */
void AExperimentClient::HandleStopEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::HandleStopEpisodeRequestTimedOut] Called"))

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	ExperimentManager->OnTimedOutDelegate.Broadcast("StopEpisode timed out!");

	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return; }
	RequestRemoveDelegates(StopEpisodeRequest, "StopEpisodeRequest");
	StopEpisodeRequest->RemoveFromRoot();
}

/* update predator's location using step message from tracking service */
void AExperimentClient::UpdatePredator(const FMessage& InMessage) {
	if (!bCanUpdatePrey) { return; }

	if (PredatorBasic->IsValidLowLevelFast()) {
		// ReSharper disable once CppUseStructuredBinding
		const FStep StepOut = UExperimentUtils::JsonStringToStep(InMessage.body);
		// const FVector VectorConverted = UExperimentUtils::CanonicalToVrV2(StepOut.location, MapLength,
		// 	OffsetOriginTransform.GetScale3D().X);
		//
		// FVector ForwardVector = OffsetOriginTransform.GetRotation().GetForwardVector();
		// ForwardVector.Normalize();
		// FVector RightVector   = OffsetOriginTransform.GetRotation().GetRightVector();
		// RightVector.Normalize();
		// const FVector NewRelativeLocation	= (ForwardVector * VectorConverted.X) + (-RightVector * VectorConverted.Y);

		FVector FinalLocation;
		FinalLocation.X = StepOut.location.x;
		FinalLocation.Y = StepOut.location.y;
		FinalLocation.Z += 25.0f*OffsetOriginTransform.GetScale3D().X;
		UE_LOG(LogTemp, Log, TEXT("[UpdatePredator] Location: %s"), *FinalLocation.ToString())
		/* rotation */
		const FRotator FinalRotation = FRotator(0,OffsetOriginTransform.GetRotation().Z + StepOut.rotation,0);
		
		FTransform UpdateTransform;
		UpdateTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f)*OffsetOriginTransform.GetScale3D().X / 5);
		UpdateTransform.SetLocation(FinalLocation);
		UpdateTransform.SetRotation(FinalRotation.Quaternion());
		PredatorBasic->SetActorTransform(UpdateTransform);
		FrameCountPredator++;

	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::UpdatePredator] PredatorBasic NULL"));
	}
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentClient::UpdatePreyPosition(const FVector InVector, const FRotator InRotation) {
	// if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	// if (!ExperimentManager->IsInEpisode()) { return; }
	if (!bConnectedToServer) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentClient::UpdatePreyPosition] bConnectedToServer false"))
		return;
	}
	if (!bResetSuccessDbg) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentClient::UpdatePreyPosition] bResetSuccessDbg false"))
		return;
	}
	
	if (!bCanUpdatePrey) { return; }
		
	if (!ensure(this->ValidateClient(TrackingClient))) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentClient::UpdatePreyPosition] TrackingClient NULL"))
		return;
	}

	if (FrameCountPrey == 0) {
		FirstLocationDebug = InVector;
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentClient::UpdatePreyPosition] FirstLocationDebug: %s"),
			*FirstLocationDebug.ToString())
	}

	/* prepare Step */

	const FVector OriginVector = OffsetOriginTransform.GetLocation();
	const FRotator OriginRotation = OffsetOriginTransform.GetRotation().Rotator();
	const FString InLocationString = FString::Printf(TEXT("%0.2f,%0.2f,%0.2f"), InVector.X, InVector.Y, InVector.Z);
	const FString InRotationString = FString::Printf(TEXT("%0.2f,%0.2f,%0.2f"), InRotation.Roll, InRotation.Pitch, InRotation.Yaw);
	const FString InOriginLocation = FString::Printf(TEXT("%0.2f,%0.2f,%0.2f"), OriginVector.X, OriginVector.Y, OriginVector.Z);
	const FString InOriginRotation = FString::Printf(TEXT("%0.2f,%0.2f,%0.2f"), OriginRotation.Roll, OriginRotation.Pitch, OriginRotation.Yaw);
	const FString InOriginScale    = FString::Printf(TEXT("%0.2f"), OffsetOriginTransform.GetScale3D().X);
	// invec, inrot, originvec, originrot, originscale
	FString DataString = FString::Printf(TEXT("%s,%s,%s,%s,%s"),
		*InLocationString,
		*InRotationString,
		*InOriginLocation,
		*InOriginRotation,
		*InOriginScale);
	
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::UpdatePreyPosition] Data: %s"), *DataString)
	// UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::UpdatePreyPosition] Data: %s"), *Step.data)
	// // flip y-axis 
	// FVector InVectorFlipped = InVector;
	// InVectorFlipped.Y *= -1;
	//
	// FVector OffsetFlipped = OffsetOriginTransform.GetLocation();
	// OffsetFlipped.Y *= -1; 
	
	// const FVector InVectorRelative = InVectorFlipped - OffsetFlipped; // relative location
	// const FVector RotatedVector = UKismetMathLibrary::GreaterGreater_VectorRotator(InVectorRelative,OffsetOriginTransform.GetRotation().Rotator());
	// const FLocation Location = UExperimentUtils::VrToCanonical(RotatedVector, MapLength, OffsetOriginTransform.GetScale3D().X);
	// UE_LOG(LogTemp, Log, TEXT("[UpdatePreyPosition] ==== USING NEW LOCATION"))
	// FLocation Location;
	// Location.x = RotatedVector.X;
	// Location.y = RotatedVector.Y;

	FStep Step;
	Step.agent_name = "prey";
	Step.frame = FrameCountPrey;
	Step.location.x    = InVector.X;
	Step.location.y    = InVector.Y;
	Step.rotation    = InRotation.Yaw;
	Step.data = "VR";

	// UE_LOG(LogTemp, Log, TEXT("[UpdatePreyPosition] ==== RotatedVector: %s"), *RotatedVector.ToString())
	// UE_LOG(LogTemp, Log, TEXT("[UpdatePreyPosition] Step: %s ==== "), *UExperimentUtils::StepToJsonString(Step))
	
	if (ensure(ExperimentManager->IsValidLowLevelFast() && ExperimentManager->Stopwatch->IsValidLowLevelFast())) {
		Step.time_stamp = ExperimentManager->Stopwatch->GetElapsedTime();
	} else { Step.time_stamp = -1.0f; }
	
	const FMessage MessageOut = UMessageClient::NewMessage("prey_step", UExperimentUtils::StepToJsonString(Step));
	if (!ensure(TrackingClient->SendMessage(MessageOut))) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::UpdatePreyPosition] Failed: Send prey step!"))
		// todo: notifyondisconnect
		return;
	}	
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::UpdatePreyPosition] Sent frame: %i | location: %s"),
		FrameCountPrey,
		*InVector.ToString())
	FrameCountPrey += 1;
}

void AExperimentClient::HandleUnroutedMessage(const FMessage InMessage) {
	printScreen("[AExperimentClient::HandleUnroutedMessage] "
		+ InMessage.header + " | " + InMessage.body);
}

/* gets player pawn from world */
bool AExperimentClient::SetupPlayerUpdatePosition(APawn* InPawn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetupPlayerUpdatePosition()] DEPRECATED RETURNING FALSE"))
	// if (!GetWorld()) { return false; }
	//
	// if (!InPawn) {
	// 	UE_LOG(LogTemp, Fatal, TEXT("[AExperimentClient::SetupPlayerUpdatePosition()] No pawn found."))
	// 	return false;
	// }
	//
	// if (Cast<AExperimentPawn>(InPawn)) {
	// 	PlayerPawn = Cast<AExperimentPawn>(InPawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
	// 	UE_LOG(LogTemp, Log,
	// 	       TEXT("[AExperimentClient::SetupPlayerUpdatePosition()] APawnMain found and assigned."));
	// 	PlayerPawn->MultiDelegate_UpdateMovement.AddDynamic(this, &ThisClass::UpdatePreyPosition);
	// }
	// else {
	// 	UE_LOG(LogTemp, Error,
	// 	       TEXT("[AExperimentClient::SetupPlayerUpdatePosition()] No valid pawn found nor assigned."));
	// 	return false;
	// }

	return false;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentClient::SelfDestruct(const FString InErrorMessage) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
	                                              FString::Printf(
		                                              TEXT(
			                                              "[AExperimentClient::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"),
		                                              *InErrorMessage));
	UE_LOG(LogTemp, Error,
	       TEXT("[AExperimentClient::SelfDestruct()] Something went wrong. Destroying. Reason: %s "),
	       *InErrorMessage);

	if (!this->IsValidLowLevelFast() && Client->IsValidLowLevel() && Client->IsConnected()) {
		this->Disconnect(Client);
	}
}

void AExperimentClient::HandleSubscribeToTrackingResponse(FString Response) {
	UE_LOG(LogTemp, Log, TEXT("[HandleSubscribeToTrackingResponse] Response: %s"), *Response)
	if (Response != "success") {
		if (ExperimentManager) { ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(false); }
		else {
			UE_LOG(LogTemp, Error,
			       TEXT("[HandleSubscribeToTrackingResponse] ExperimentManager is NULL!"))
			RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
			TrackingSubscribeRequest->RemoveFromRoot();
		}
		return;
	}

	// means result is success 
	if (ExperimentManager->IsValidLowLevel()) { ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(true); }
	else {
		UE_LOG(LogTemp, Error,
		       TEXT("[HandleSubscribeToTrackingResponse] ExperimentManager is NULL!"))
	}

	RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
}

void AExperimentClient::HandleSubscribeToTrackingTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[HandleSubscribeToTrackingTimedOut] Timed out!"));
	if (ExperimentManager) { ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(false); }
	else {
		UE_LOG(LogTemp, Error,
		       TEXT("[HandleSubscribeToTrackingTimedOut] ExperimentManager NULL!"));
	}

	this->RequestRemoveDelegates(TrackingSubscribeRequest, "RequestRemoveDelegates");

	RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
	TrackingSubscribeRequest->RemoveFromRoot();
}

bool AExperimentClient::SubscribeToTracking() {
	if (!ValidateClient(TrackingClient)) {
		UE_LOG(LogTemp, Fatal, TEXT("[AExperimentClient::SubscribeToTracking] ClientIn not valid."))
		return false;
	}

	if (ExperimentManager && ExperimentManager->bSubscribed) {
		UE_LOG(LogTemp, Warning,
		       TEXT("[SubscribeToTracking] Already subscribed! ExperimentManager->bSubscribed = %i"),
		       ExperimentManager->bSubscribed)
		return true;
	}

	UE_LOG(LogTemp, Log, TEXT("[SubscribeToTracking] Preparing to send! TrackingSubscribeRequest."));
	TrackingSubscribeRequest = TrackingClient->SendRequest("!subscribe", "", 10.0f); // TC is just slower...
	if (!ensure(TrackingSubscribeRequest->IsValidLowLevelFast())) { return false; }
	TrackingSubscribeRequest->AddToRoot();

	TrackingSubscribeRequest->ResponseReceived.AddDynamic(
		this, &AExperimentClient::HandleSubscribeToTrackingResponse);
	TrackingSubscribeRequest->TimedOut.AddDynamic(this, &AExperimentClient::HandleSubscribeToTrackingTimedOut);
	UE_LOG(LogTemp, Log, TEXT("[SubscribeToTracking] OK!"))
	return true;
}

void AExperimentClient::RequestRemoveDelegates(URequest* InRequest, const FString& InRequestString) {
	bool Result = false;
	if (InRequest->IsValidLowLevelFast()) {
		InRequest->RemoveFromRoot();
		InRequest->ResponseReceived.RemoveAll(this);
		InRequest->TimedOut.RemoveAll(this);
		InRequest = nullptr;
		Result = true;
	}
	else {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::RequestRemoveDelegates] Failed. RequestIn (%s) NULL"),
		       *InRequestString)
	}
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::RequestRemoveDelegates] Request: %s; Result: %i"),
	       *InRequestString, Result)
}

void AExperimentClient::OnStatusChanged(EExperimentStatus ExperimentStatusIn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnStatusChanged] New status: %i"), ExperimentStatusIn)
}

bool AExperimentClient::SendGetOcclusionsRequest() {
	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogTemp, Error,
		       TEXT("[SendGetOcclusionsRequest] Can't send get occlusion request, Experiment service client NULL."));
		return false;
	}

	GetOcclusionsRequest = TrackingClient->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!GetOcclusionsRequest) { return false; }
	GetOcclusionsRequest->AddToRoot();

	GetOcclusionsRequest->ResponseReceived.AddDynamic(this, &AExperimentClient::HandleGetOcclusionsResponse);
	GetOcclusionsRequest->TimedOut.AddDynamic(this, &AExperimentClient::HandleGetOcclusionsTimedOut);
	return true;
}

void AExperimentClient::HandleGetOcclusionsResponse(const FString ResponseIn) {
	/* start empty */
	OcclusionIDsIntArr.Empty();

	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr;
	const FString OcclusionIDsTemp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	const int32 Samples = OcclusionIDsTemp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogTemp, Log,
	       TEXT("[AExperimentClient::HandleGetOcclusionsResponse] Ccclusions in world (%i): %s"),
	       Samples, *OcclusionIDsTemp);

	/* convert to integer array */
	int32 SamplesLost = 0;
	for (FString OcclusionValue : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*OcclusionValue)) {
			OcclusionsStruct.OcclusionIDsIntArr.Add(FCString::Atoi(*OcclusionValue));
		}
		else { SamplesLost++; }
	}

	if (OcclusionsStruct.bAllLocationsLoaded) {
		OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionsStruct.OcclusionIDsIntArr);
		OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, false, false);
	} else {
		UE_LOG(LogTemp, Fatal,
		       TEXT("[AExperimentClient::HandleGetOcclusionsResponse] Occlusions are NOT loaded!"));
	}

	if (GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Log,
		       TEXT(
			       "[AExperimentClient::HandleGetOcclusionsResponse] RequestRemoveDelegates(GetOcclusionsRequest)"
		       ));
		this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
	} else {
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "[AExperimentClient::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."
		       ));
	}
}

void AExperimentClient::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogTemp, Warning,
	       TEXT("[AExperimentClient::HandleGetOcclusionsTimedOut] Get occlusion request timed out!"));
	if (!GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "[AExperimentClient::HandleGetOcclusionsTimedOut] Failed to remove delegates, GetOcclusionsRequest not valid."
		       ));
		return;
	}

	UE_LOG(LogTemp, Log,
	       TEXT("[AExperimentClient::HandleGetOcclusionsResponse] Removed delegates for: GetOcclusionsRequest"
	       ));
	this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
}

bool AExperimentClient::SendGetOcclusionLocationsRequest() {
	UE_LOG(LogTemp, Log,
	       TEXT("[AExperimentClient::SendGetOcclusionLocationsRequest] Starting request."));
	if (!ensure(ValidateClient(TrackingClient))) {
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "[AExperimentClient::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid."
		       ));
		return false;
	}
	
	GetOcclusionLocationRequest = TrackingClient->SendRequest("get_cell_locations", "21_05", 10.0f);
	if (!ensure(GetOcclusionLocationRequest)) { return false; }
	GetOcclusionLocationRequest->AddToRoot();
	
	GetOcclusionLocationRequest->ResponseReceived.AddDynamic(
		this, &AExperimentClient::HandleGetOcclusionLocationsResponse);
	GetOcclusionLocationRequest->TimedOut.AddDynamic(
		this, &AExperimentClient::HandleGetOcclusionLocationsTimedOut);

	return true;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentClient::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] Response:%s"), *ResponseIn);
	if (!OcclusionsStruct.bAllLocationsLoaded) { OcclusionsStruct.SetAllLocations(ResponseIn); }
	if (!OcclusionsStruct.bSpawnedAll) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] About to call SpawnAll"));
		if (HasAuthority()) {
			UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] Calling spawning all from server!"));
			Server_SpawnOcclusions(); // add arguments 
			// OcclusionsStruct.SpawnAll(GetWorld(), false, false, OffsetOriginTransform);
		} else {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] NO AUTHORITY"));
		}
	} else { UE_LOG(LogTemp, Log,
		TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] All locations already spawned. Skipping spawn!" ));
	}
	
	if (GetOcclusionLocationRequest) {
		RequestRemoveDelegates(GetOcclusionLocationRequest, "GetOcclusionLocationRequest");
		GetOcclusionLocationRequest->RemoveFromRoot();
	}

	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] Failed to SendGetOcclusionsRequest" ))
	} else {
		UE_LOG(LogTemp, Log,
			TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] Sent SendGetOcclusionsRequest OK!" ))
	}
	UE_LOG(LogTemp, Log,TEXT("[AExperimentClient::HandleGetOcclusionLocationsResponse] Exited OK"))
}

void AExperimentClient::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));

	// get locations in our specific world configuration (21_05 by defualt)
	if (OcclusionsStruct.bAllLocationsLoaded && OcclusionsStruct.bSpawnedAll) {
		UE_LOG(LogTemp, Warning,
		       TEXT("[HandleGetOcclusionLocationsTimedOut] Occlusions already set, we can bypass this!"));
		if (!this->SendGetOcclusionsRequest()) {
			UE_LOG(LogTemp, Log,
			       TEXT(
				       "[AExperimentClient::HandleGetOcclusionLocationsResponse] Failed to SendGetOcclusionsRequest"
			       ))
		}
		else {
			UE_LOG(LogTemp, Log,
			       TEXT(
			       	"[AExperimentClient::HandleGetOcclusionLocationsResponse] Sent SendGetOcclusionsRequest OK!"
			       ))
		}
	}
}

bool AExperimentClient::ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn,
                                                const FString& IPAddressIn, const int PortIn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::ConnectToServer] Attempting connection: %s:%i"),
	       *IPAddressIn, PortIn)

	if (!ClientIn->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[ConnectToServer()] Failed to validate client!"));
		return false;
	}
	
	uint8 AttemptCurr = 0;

	while (AttemptCurr < MaxAttemptsIn) {
		if (ClientIn->Connect(IPAddressIn, PortIn)) {
			UE_LOG(LogTemp, Log,TEXT("[AExperimentClient::ConnectToServer()] Success (attempt #: %i/%i)"),
			       AttemptCurr + 1,
			       MaxAttemptsIn);
			break;
		}
		AttemptCurr += 1;
	}

	if (!ClientIn->IsConnected()) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::ConnectToServer()] Failed to connect to server!"));
		return false;
	}

	return true;
}

bool AExperimentClient::RoutePredatorMessages() {
	if (!this->ValidateClient(TrackingClient)) {
		printScreen("[AExperimentClient::RoutePredatorMessages()] TrackingClient not valid.");
		return false;
	}
	if (MessageRoutePredator->IsValidLowLevelFast()) { MessageRoutePredator = nullptr; }
	MessageRoutePredator = TrackingClient->AddRoute(ExperimentHeaders.PredatorStep);
	if (!MessageRoutePredator) {
		UE_LOG(LogTemp, Error,
		       TEXT("[AExperimentClient::RoutePredatorMessages] MessageRoutePredator not valid."));
		return false;
	}

	MessageRoutePredator->MessageReceived.AddDynamic(this, &AExperimentClient::HandleUpdatePredator);
	if (TrackingClient->IsValidLowLevel()) { TrackingClient->UnroutedMessageEvent.RemoveAll(this); }
	TrackingClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentClient::HandleUnroutedMessage);

	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::RoutePredatorMessages] OK"));
	return true;
}

bool AExperimentClient::RouteOnCapture() {
	if (!ensure(this->ValidateClient(TrackingClient))) { return false; }

	MessageRouteOnCapture = TrackingClient->AddRoute("on_capture");
	if (!ensure(MessageRouteOnCapture->IsValidLowLevelFast())) { return false; }

	MessageRouteOnCapture->MessageReceived.AddDynamic(this, &AExperimentClient::HandleOnCapture);
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::RouteOnCapture] OK"));
	return true;
}

bool AExperimentClient::SetupConnections() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetupConnections] Entering"));
	return false;

	if (TrackingClient->IsValidLowLevelFast() && TrackingClient->IsConnected()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentClient::SetupConnections] disconnecting"));
		if (!TrackingClient->Disconnect()) {
			UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetupConnections] disconnect FAILED"));
		}
		else { UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetupConnections] disconnect OK")); }
	}

	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetupConnections] setting trackingclient to nullptr"));
	TrackingClient = nullptr;
	TrackingClient = this->CreateNewClient();
	if (!ensure(TrackingClient->IsValidLowLevelFast())) { return false; }

	constexpr int AttemptsMax = 3;

	if (!this->ConnectToServer(TrackingClient, AttemptsMax, *ServerInfo.IP, ServerInfo.Port)) {
		UE_LOG(LogTemp, Fatal,
		       TEXT("[AExperimentClient::SetupConnections] Connect to Tracking: Failed! (IP: %s)"),
		       *ServerInfo.IP);
		return false;
	}

	UE_LOG(LogTemp, Log,
	       TEXT("[AExperimentClient::SetupConnections()] Connect to Tracking: OK! (IP: %s)"),
	       *ServerInfo.IP);

	if (!ensure(this->RoutePredatorMessages())) { return false; }
	if (!ensure(this->RouteOnCapture())) { return false; }
	if (ExperimentManager && ExperimentManager->bSubscribed) {
		if (TrackingClient) {
			UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetupConnections()] Unsubscribing"))
			FMessage Message = FMessage();
			Message.header = "!unsubscribe";
			Message.body = "";
			if (TrackingClient->SendMessage(Message)) {
				ExperimentManager->bSubscribed = false;
				UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::SetupConnections()] Unsubscribing OK"))
			}
		}
	}
	if (!ensure(this->SubscribeToTracking())) { return false; }

	UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::SetupConnections] Exiting OK"));
	return true;
}

bool AExperimentClient::Test() {
	TrackingClient = this->CreateNewClient();

	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Test] Connecting to IP:Port => %s:%i."),
	       *ServerInfo.IP, ServerInfo.Port);

	/* connect tracking service */
	constexpr int AttemptsMax = 3;
	bConnectedToServer = this->ConnectToServer(TrackingClient, AttemptsMax, *ServerInfo.IP, ServerInfo.Port);
	if (!ensure(bConnectedToServer)) {
		UE_LOG(LogTemp, Fatal,
		       TEXT("[AExperimentClient::Test] Connect to Tracking: Failed! (IP: %s)"), *ServerInfo.IP);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Test()] Connect to Tracking: OK! (IP: %s)"),
	       *ServerInfo.IP);

	if (!ensure(this->RoutePredatorMessages())) { return false; }
	if (!ensure(this->RouteOnCapture())) { return false; }
	if (!ensure(this->SubscribeToTracking())) { return false; }
		
	/* moved to SpatialAnchorManager::Server_FinishSpawn_Implementation()  */
	/* TODO: MOVE THIS SHIT
	 *
	 * !!! WILL BE MOVED !!!
	 *
	 * 
	 */
	// if (!this->SendGetOcclusionLocationsRequest()) {
	// 	UE_LOG(LogTemp, Error,
	// 	       TEXT("[AExperimentClient::Test] Failed to SendGetOcclusionLocationsRequest"))
	// }

	return true;
}

void AExperimentClient::SetOcclusionVisibility(const bool bNewVisibility) {
	UE_LOG(LogTemp, Log,
		TEXT("[AExperimentClient::SetOcclusionVisibility] Called with value: %i"), bNewVisibility)

	if (ensure(OcclusionsStruct.bCurrentLocationsLoaded)) {
		UE_LOG(LogTemp, Log,
		       TEXT("[AExperimentClient::SetOcclusionVisibility] Using last loaded locations (%i)"),
		       OcclusionsStruct.OcclusionIDsIntArr.Num())
		OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, !bNewVisibility, false);
	}
}

// bound to FNotifyEpisodeStarted on success! 
void AExperimentClient::OnEpisodeStarted() {
	// todo: get proper index from ExperimentManager; either fetch or pass as input

	UE_LOG(LogTemp, Log,
	       TEXT("[AExperimentClient::OnEpisodeStarted] Called. Episode started player index: %i"), PlayerIndex)
	
	if (OcclusionsStruct.bCurrentLocationsLoaded) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnEpisodeStarted] Using last loaded locations (%i)"),
			OcclusionsStruct.OcclusionIDsIntArr.Num())
		OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, false, false);
	} else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentClient::OnEpisodeStarted] LOCATIONS NOT LOADED"))
	}
	
	FrameCountPrey     = 0;
	FrameCountPredator = 0;
	ExperimentInfo.SetStatus(EExperimentStatus::InEpisode);
	if (!PredatorBasic->IsValidLowLevelFast()) {
		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnEpisodeStarted] Predator not spawned. Calling Spawner."))
		const bool SpawnResult = SpawnAndPossessPredator();
		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnEpisodeStarted] SpawnResult: %s"),
			SpawnResult ? TEXT("True") : TEXT("False"))
	}
	SetPredatorIsVisible(true);
	bCanUpdatePrey = true;
}

void AExperimentClient::ResetWorldState() {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::ResetWorldState] called"))
	bCanUpdatePrey = false;
	if (!ensure(ExperimentManager->IsValidLowLevelFast())){ return; }
	ExperimentManager->SetInEpisode(false);
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	OcclusionsStruct.SetAllHidden();
	// keep Predator visible to acclimate player to predator's capture radius
	this->SetPredatorIsVisible(true); 
}

void AExperimentClient::OnEpisodeFinished() {
	this->ResetWorldState();
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::OnEpisodeFinished] OK"));
}

void AExperimentClient::HandleUpdatePredator(const FMessage MessageIn) {
	// UE_LOG(LogTemp, Warning, TEXT("Predator: %s"), *MessageIn.body);
	this->UpdatePredator(MessageIn);
}

void AExperimentClient::HandleOnCapture(const FMessage MessageIn) {
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleOnCapture]"));

	// if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	// if (!ensure(ExperimentManager->IsInEpisode())) { return; }

	// ExperimentManager->OnEpisodeFinishedSuccessDelegate.Broadcast();
	// ExperimentManager->ProcessStopEpisodeResponse();
	ExperimentManager->OnEpisodeFinished();
	/* todo: play sound! */
	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::HandleOnCapture]"));
	Server_PlayCaptureSound();
}

float AExperimentClient::GetTimeRemaining() const {
	if (!TimerHandlePtr->IsValid()) { return -1.0f; }
	if (!GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr)) { return -1.0f; }
	return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);
}

void TestConversions(const FLocation InputLocation) {
	constexpr float TestScale = 1.0f;
	
	UE_LOG(LogTemp, Warning, TEXT("[TestConversions] STARTED"))
	UE_LOG(LogTemp, Warning, TEXT("[TestConversions] InputLocation: (%0.2f, %0.2f)"),
		InputLocation.x,
		InputLocation.y)

	// convert from canonical to VR coordinates 
	const FVector Can2Vr = UExperimentUtils::CanonicalToVrV2(InputLocation, 235.185, TestScale);
	UE_LOG(LogTemp, Warning, TEXT("[TestConversions] Can2VR: (%0.2f, %0.2f)"),
		Can2Vr.X,
		Can2Vr.Y)

	// convert back to canonical (should be same as InputLocation)
	const FLocation VR2Can = UExperimentUtils::VrToCanonical(Can2Vr,235.185, TestScale);
	UE_LOG(LogTemp, Warning, TEXT("[TestConversions] VR2Can: (%0.2f, %0.2f)"),
		   VR2Can.x,
		   VR2Can.y)
	
	UE_LOG(LogTemp, Warning, TEXT("[TestConversions] DONE"))
}

/* main stuff happens here */
void AExperimentClient::BeginPlay() {
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::BeginPlay] Called"));
	
	ExperimentInfo.OnExperimentStatusChangedEvent.AddDynamic(this, &ThisClass::OnStatusChanged);

	ExperimentManager = NewObject<UExperimentManager>(this, UExperimentManager::StaticClass());
	ExperimentManager->AddToRoot();
	ensure(ExperimentManager->IsValidLowLevel());

	if (ensure(ExperimentManager)) {
		/* initial setups */
		ExperimentManager->NotifyOnExperimentFinishedDelegate.AddDynamic(this, &ThisClass::OnExperimentFinished);
		ExperimentManager->OnSubscribeResultDelegate.AddDynamic(this, &ThisClass::OnSubscribeResult);
		ExperimentManager->OnTimedOutDelegate.AddDynamic(this, &ThisClass::OnTimedOut);

		/* start */
		ExperimentManager->OnEpisodeStartedFailedDelegate.AddDynamic(this, &ThisClass::HandleStartEpisodeError);
		ExperimentManager->NotifyEpisodeStarted.AddDynamic(this, &ThisClass::OnEpisodeStarted);

		/* stop */
		ExperimentManager->NotifyEpisodeFinished.AddDynamic(this, &ThisClass::OnEpisodeFinished);
		ExperimentManager->OnEpisodeFinishedFailedDelegate.AddDynamic(this, &ThisClass::HandleStopEpisodeError);

		UE_LOG(LogTemp, Log, TEXT("[AExperimentClient::Test] Bound delegates: ExperimentManager"))
	}
	Test();
}


/* run a (light!) command every frame */
void AExperimentClient::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExperimentClient::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (Stopwatch && Stopwatch->IsRunning()) { Stopwatch->Reset(); };
	if (TrackingClient->IsValidLowLevelFast()) { TrackingClient->Disconnect(); }
	/* todo: clear delegates */
}
