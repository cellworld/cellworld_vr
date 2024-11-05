// ReSharper disable CppTooWideScopeInitStatement
#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AExperimentServiceMonitor::OnExperimentFinished(const int InPlayerIndex) {
	UE_LOG(LogExperiment, Log, TEXT("AExperimentServiceMonitor::OnExperimentFinished"))
}

//TODO - add argument to include MessageType (Log, Warning, Error, Fatal)
void printScreen(const FString InMessage) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s"), *InMessage));
	UE_LOG(LogExperiment, Warning, TEXT("%s"), *InMessage);
}

bool AExperimentServiceMonitor::ValidateLevel(UWorld* InWorld, const FString InLevelName) {
	
	if (!InWorld->IsValidLowLevelFast()) { printScreen("[AExperimentServiceMonitor::ValidateLevel] World is not valid."); return false; }
	
	if (!InLevelName.IsEmpty()) { UE_LOG(LogExperiment, Warning, TEXT("LevelName: %s"), *InLevelName); }
	else { printScreen("[AExperimentServiceMonitor::ValidateLevel] LevelName is empty!"); return false; }

	FString MapName = InWorld->GetMapName();
	MapName.RemoveFromStart(InWorld->StreamingLevelsPrefix);
	UE_LOG(LogExperiment, Warning, TEXT("map name: %s"), *MapName);

	return true;
}

/* todo: make this take input ACharacter and spawn that one*/
bool AExperimentServiceMonitor::SpawnAndPossessPredator() {

	if (!GetWorld()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] GetWorld() failed!"));
		return false;
	}

	// Define spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Specify the location and rotation for the new actor
	const FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation
	FLocation SpawnLocation;
	SpawnLocation.x = 0.5;
	SpawnLocation.y = 0.5;
	const FVector SpawnVector = UExperimentUtils::CanonicalToVr(SpawnLocation, this->MapLength, this->WorldScale);
	
	// Spawn the character
	this->PredatorBasic = GetWorld()->SpawnActor<APredatorBasic>(APredatorBasic::StaticClass(), SpawnVector, Rotation, SpawnParams);

	// Ensure the character was spawned
	if (!this->PredatorBasic) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] Spawn APredatorBasic Failed!"));
		return false;
	}

	// this->PredatorBasic->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f)*this->PredatorScaleFactor*this->WorldScale);
	this->SetPredatorIsVisible(false);
	return true;
}

/* stop connection for ClientIn */
bool AExperimentServiceMonitor::Disconnect(UMessageClient* ClientIn) {
	if (ClientIn->IsValidLowLevelFast() && ClientIn->IsConnected())
	{
		printScreen("[AExperimentServiceMonitor::Disconnect] Client Disconnected: Processing.");
		return ClientIn->Disconnect();
	}
	printScreen("[AExperimentServiceMonitor::Disconnect] Client Disconnected: Failed.");
	return false; 
}

UMessageClient* AExperimentServiceMonitor::CreateNewClient() {
	UMessageClient* NewClient = UMessageClient::NewMessageClient();
	return NewClient; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString& ExperimentNameIn) {
	if (!this->ValidateClient(TrackingClient)) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, Experiment Service client not valid.")); return false; }
	
	if (ValidateClient(TrackingClient)) {
		FMessage Message;
		Message.header = "close";
		Message.body   = "";
		if (!TrackingClient->SendMessage(Message)) {
			UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Failed to send."))
		}
	}
	return true;
}

/* start experiment service episode stream
 * should ONLY be called from game mode by the BP_door trigger !!!  */
bool AExperimentServiceMonitor::StartEpisode() {
	if (!ensure(ExperimentManager)) { return false; }
	if (ExperimentManager->IsInEpisode()) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::StartEpisode] Failed. ExperimentManager->bInEpisode = true"))
		return false;
	}
	/* start new episode */
	if (!this->ResetTrackingAgent()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StartEpisode] ResetTrackingAgent() Call Failed!"))
	} else {
		UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::StartEpisode] ResetTrackingAgent() Call OK!"))
	}
	
	return true;
}

bool AExperimentServiceMonitor::ValidateClient(UMessageClient* ClientIn) {
	if (!ClientIn->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::ValidateClient] Client not valid."));
		return false;
	}
	
	if (!ClientIn->IsConnected()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::ValidateClient] Client not connected."));
		return false;
	}
	return true;
}

bool AExperimentServiceMonitor::ValidateExperimentName(const FString& ExperimentNameIn) {
	if (ExperimentNameIn == "") { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode(const bool bForce) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::StopEpisode] Called; bForce = %i"), bForce)

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return false; }
	
	if (!ExperimentManager->IsInEpisode()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopEpisode] Can't stop episode, not in episode. Status: %i"),
				ExperimentInfo.Status);
		ExperimentManager->OnEpisodeFinishedFailedDelegate.Broadcast("Not in episode."); 
		return false;
	}

	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogExperiment, Fatal,
				TEXT("[AExperimentServiceMonitor::StopEpisode] TrackingClient not valid! Failed to send STOP!"));
	}
	
	StopEpisodeRequest = TrackingClient->SendRequest("stop","",15.0f);
	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return false; }
	StopEpisodeRequest->AddToRoot();

	StopEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestResponse);
	StopEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut);
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::StopEpisode] StopEpisode sent OK"));
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStartEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[HandleStartEpisodeRequestResponse] %s"), *response);
	
	if (!ensure(response == "success")) {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleStartEpisodeRequestResponse] Failed to start episode! Unkown reason!"))
		// ExperimentInfo.SetStatus(EExperimentStatus::ErrorStartEpisode);
		this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");
		StartEpisodeRequest->RemoveFromRoot();
		return;
	}

	if (!ensure(StartEpisodeRequest)) { return; }
	this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");
	StartEpisodeRequest->RemoveFromRoot();

	UE_LOG(LogExperiment, Log, TEXT("[HandleStartEpisodeRequestResponse] Preparing to ResetAgentTracking!"))	
	if (!this->ResetTrackingAgent()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleStartEpisodeRequestResponse] ResetTrackingAgent() Failed!"))
	} else {
		UE_LOG(LogExperiment, Log,
			TEXT("[HandleStartEpisodeRequestResponse] ResetTrackingAgent() OK!"))
	}
}

// essentially "StartEpisode"
bool AExperimentServiceMonitor::ResetTrackingAgent() {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::ResetTrackingAgent] Preparing to send!"))

	ResetRequest = TrackingClient->SendRequest("reset","",10.0f);
	if (!ensureMsgf(ResetRequest, TEXT("ResetRequest is NULL"))) { return false; }
	ResetRequest->AddToRoot();
	
	ResetRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestResponse);
	ResetRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestTimedOut);
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::ResetTrackingAgent] Sent OK!"))
	return true; 
}

void AExperimentServiceMonitor::HandleStartEpisodeError(const FString InMessage) {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeError] %s"), *InMessage)
	ResetWorldState(); 
}

void AExperimentServiceMonitor::HandleResetRequestResponse(const FString InResponse) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse] Response: %s"),
		*InResponse)

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	
	if (InResponse != "success") {
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorResetTrackingAgent);
		ExperimentManager->OnEpisodeStartedFailedDelegate.Broadcast("[HandleResetRequestResponse] did not receive 'success'");
		if (ResetRequest) {
			RequestRemoveDelegates(ResetRequest,"ResetRequest");
			ResetRequest->RemoveFromRoot();
		}
		return; 
	}

	ExperimentManager->OnProcessStartEpisodeResponseDelegate.Broadcast();
	if (ensure(ResetRequest)) {
		this->RequestRemoveDelegates(ResetRequest, "ResetRequest");
		ResetRequest->RemoveFromRoot();
	}

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse] Exiting OK"))
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppPassValueParameterByConstReference
void AExperimentServiceMonitor::OnTimedOut(const FString InMessage) {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::OnTimedOutDelegate] %s"), *InMessage);
	ResetWorldState();

	// if (GetWorld()) {
	// 	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::OnTimedOut] RESTARTING GAME!"))
	// 	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
	// }
	

	// todo: start process restart connection and game 	
	// if(!SetupConnections()) {
	// 	UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::OnTimedOut] SetupConnections failed"));
	// }else {
	// 	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnTimedOut] SetupConnections OK"));
	// }
}

void AExperimentServiceMonitor::HandleResetRequestTimedOut() {

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	
	if (ResetRequest) {
		this->RequestRemoveDelegates(ResetRequest, "ResetRequest");
		ResetRequest->RemoveFromRoot(); 
	}
	UE_LOG(LogExperiment, Error, TEXT("[HandleResetRequestTimedOut] Reset request timed out!"))
	UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestTimedOut] Broadcasting OnTimedOutDelegate"))
	ExperimentManager->OnTimedOutDelegate.Broadcast("ResetRequest timed out!");
}

void AExperimentServiceMonitor::OnSubscribeResult(const bool bSubscribeResult) {
	UE_LOG(LogExperiment, Log, TEXT("OnSubscribeResult: %i"), bSubscribeResult)
	if (bSubscribeResult) {
		UE_LOG(LogExperiment, Log, TEXT("Subscription successful!"))	
	}else {
		UE_LOG(LogExperiment, Error, TEXT("Subscription Failed!"))	
	}
}

void AExperimentServiceMonitor::OnResetResult(const bool bResetResult) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnResetResult] DEPRECATED"))
}

void AExperimentServiceMonitor::SetPredatorIsVisible(const bool bNewVisibility) {
	if (this->PredatorBasic->IsValidLowLevelFast()) {
		this->PredatorBasic->SetActorHiddenInGame(!bNewVisibility);
		UE_LOG(LogExperiment, Log,
				TEXT("[AExperimentServiceMonitor::SetPredatorIsVisible] IsActorHiddenInGame(%i)"), !bNewVisibility);
	} else {
		UE_LOG(LogExperiment, Error,
				TEXT("[AExperimentServiceMonitor::SetActorHiddenInGame] PredatorBasic not valid"));
	}
}

void AExperimentServiceMonitor::HandleStopEpisodeError(const FString InMessage) {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeError] %s"), *InMessage)
	ExperimentInfo.SetStatus(EExperimentStatus::ErrorFinishEpisode); // todo: move to OnEpisodeFinishedFailedDelegateHandler
	ResetWorldState();
}

void AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut] Episode request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
}

void AExperimentServiceMonitor::HandleStopEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] %s"),
		*response);

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] ExperimentManager is NULL!"));
		return;
	}
	
	if (response != "success") {
		ExperimentManager->OnEpisodeFinishedFailedDelegate.Broadcast("[HandleStopEpisodeRequestResponse] did not receive 'success'");
		return;
	}
	ExperimentManager->OnProcessStopEpisodeResponseDelegate.Broadcast(); 
	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return; }
	RequestRemoveDelegates(StopEpisodeRequest, "StopEpisodeRequest");
	StopEpisodeRequest->RemoveFromRoot();
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut] Called"))

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	ExperimentManager->OnTimedOutDelegate.Broadcast("StopEpisode timed out!");

	if (!ensure(StopEpisodeRequest->IsValidLowLevelFast())) { return; }
	RequestRemoveDelegates(StopEpisodeRequest, "StopEpisodeRequest");
	StopEpisodeRequest->RemoveFromRoot();
}

/* update predator's location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(const FMessage& InMessage) {
	// if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return;}
	// if (!ExperimentManager->IsInEpisode()) { return; }
	if(!bCanUpdatePrey) { return; }
	
	if (PredatorBasic->IsValidLowLevelFast()) {
		// ReSharper disable once CppUseStructuredBinding
		const FStep StepOut = UExperimentUtils::JsonStringToStep(InMessage.body);
		FVector NewLocation = UExperimentUtils::CanonicalToVr(StepOut.location, MapLength, WorldScale);
		NewLocation.Z = 182.0f*2; // 6ft
		PredatorBasic->SetActorLocation(NewLocation);
		FrameCountPredator++;
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::UpdatePredator] Received valid predator step."));
	}
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector InVector, const FRotator InRotation) {
	// if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	// if (!ExperimentManager->IsInEpisode()) { return; }
	if (!bCanUpdatePrey) { return; }
	if (!ensure(this->ValidateClient(TrackingClient))) { return; }

	/* prepare Step */
	FStep Step;
	Step.data = "VR";
	Step.agent_name = "prey";
	Step.frame = FrameCountPrey;
	Step.location = UExperimentUtils::VrToCanonical(InVector, MapLength, this->WorldScale);
	Step.rotation = InRotation.Yaw; // todo: get actual rotation of player  

	if (ensure(ExperimentManager->IsValidLowLevelFast() && ExperimentManager->Stopwatch->IsValidLowLevelFast())) {
		Step.time_stamp = ExperimentManager->Stopwatch->GetElapsedTime();
	} else {
		Step.time_stamp = -1.0f;
	}
	 
	const FMessage MessageOut = UMessageClient::NewMessage("prey_step",
	UExperimentUtils::StepToJsonString(Step));
	
	if (!TrackingClient->SendMessage(MessageOut)) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::UpdatePreyPosition] Failed: Send prey step!"))
		// todo: notifyondisconnect
		return;
	}
	FrameCountPrey += 1; 
}

void AExperimentServiceMonitor::HandleUnroutedMessage(const FMessage InMessage) {
	printScreen("[AExperimentServiceMonitor::HandleUnroutedMessage] " 
		+ InMessage.header + " | " +  InMessage.body);
}

/* gets player pawn from world */
bool AExperimentServiceMonitor::SetupPlayerUpdatePosition() {
	if (!GetWorld()) { return false; }
	
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SetupPlayerUpdatePosition()] No pawn found."))
		return false;
	}
	
	if (Cast<APawnMain>(Pawn)) {
		PlayerPawn = Cast<APawnMain>(Pawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupPlayerUpdatePosition()] APawnMain found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SetupPlayerUpdatePosition()] No valid pawn found nor assigned."));
		return false;
	}

	return true;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString InErrorMessage)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *InErrorMessage));
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying. Reason: %s "), *InErrorMessage);

	if (!this->IsValidLowLevelFast() && Client->IsValidLowLevel() && Client->IsConnected())
	{
		this->Disconnect(Client);
	}
}

void AExperimentServiceMonitor::HandleSubscribeToTrackingResponse(FString Response) {

	UE_LOG(LogExperiment, Log, TEXT("[HandleSubscribeToTrackingResponse] Response: %s"), *Response)
	if (Response != "success" ) {
		if (ExperimentManager) {
			ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(false);
		} else {
			UE_LOG(LogExperiment, Error,
				TEXT("[HandleSubscribeToTrackingResponse] ExperimentManager is NULL!"))
			RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
			TrackingSubscribeRequest->RemoveFromRoot();
		}
		return;
	}

	// means result is success 
	if (ExperimentManager->IsValidLowLevel()){
		ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(true);
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleSubscribeToTrackingResponse] ExperimentManager is NULL!"))
	}

	RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
}

void AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut() {
	
	UE_LOG(LogExperiment, Error, TEXT("[HandleSubscribeToTrackingTimedOut] Timed out!"));
	if (ExperimentManager) {
		ExperimentManager->OnSubscribeStatusChangedDelegate.Broadcast(false);
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleSubscribeToTrackingTimedOut] ExperimentManager NULL!"));
	}
	
	this->RequestRemoveDelegates(TrackingSubscribeRequest, "RequestRemoveDelegates");
	if (PlayerPawn && PlayerPawn->PlayerHUD) {
		PlayerPawn->PlayerHUD->SetNotificationText("SubscribeToTracking() timed out!");
		PlayerPawn->PlayerHUD->SetNotificationVisibility(ESlateVisibility::Visible);
	}
	
	RequestRemoveDelegates(TrackingSubscribeRequest, "TrackingSubscribeRequest");
	TrackingSubscribeRequest->RemoveFromRoot();
}

bool AExperimentServiceMonitor::SubscribeToTracking() {
	if (!ValidateClient(TrackingClient)) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SubscribeToTracking] ClientIn not valid."))
		return false;
	}

	if (ExperimentManager && ExperimentManager->bSubscribed) {
		UE_LOG(LogExperiment, Warning,
			TEXT("[SubscribeToTracking] Already subscribed! ExperimentManager->bSubscribed = %i"),
			ExperimentManager->bSubscribed)
		return true;
	}
	
	UE_LOG(LogExperiment, Log,TEXT("[SubscribeToTracking] Preparing to send! TrackingSubscribeRequest."));
	TrackingSubscribeRequest = TrackingClient->SendRequest("!subscribe","",10.0f); // TC is just slower...
	if (!ensure(TrackingSubscribeRequest->IsValidLowLevelFast())) { return false; }
	TrackingSubscribeRequest->AddToRoot(); 

	TrackingSubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingResponse);
	TrackingSubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut);
	UE_LOG(LogExperiment, Log, TEXT("[SubscribeToTracking] OK!"))
	return true;
}

void AExperimentServiceMonitor::RequestRemoveDelegates(URequest* InRequest, const FString& InRequestString) {
	bool Result = false; 
	if (InRequest->IsValidLowLevelFast()) {
		InRequest->RemoveFromRoot();
		InRequest->ResponseReceived.RemoveAll(this);
		InRequest->TimedOut.RemoveAll(this);
		InRequest = nullptr; 
		Result = true; 
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::RequestRemoveDelegates] Failed. RequestIn (%s) NULL"), *InRequestString)
	}
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::RequestRemoveDelegates] Request: %s; Result: %i"), *InRequestString, Result)
}

void AExperimentServiceMonitor::OnStatusChanged(EExperimentStatus ExperimentStatusIn) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnStatusChanged] New status: %i"), ExperimentStatusIn)
}

bool AExperimentServiceMonitor::SendGetOcclusionsRequest() {
	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogExperiment, Error, TEXT("[SendGetOcclusionsRequest] Can't send get occlusion request, Experiment service client NULL."));
		return false;
	}

	GetOcclusionsRequest = TrackingClient->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!GetOcclusionsRequest) { return false; }
	GetOcclusionsRequest->AddToRoot();
	
	GetOcclusionsRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	GetOcclusionsRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);
	return true;
}

void AExperimentServiceMonitor::HandleGetOcclusionsResponse(const FString ResponseIn)  {
	/* start empty */
	OcclusionIDsIntArr.Empty(); 

	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr; 
	const FString OcclusionIDsTemp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	const int32 Samples = OcclusionIDsTemp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);
	
	UE_LOG(LogExperiment, Log,
		TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Ccclusions in world (%i): %s"),
		Samples, *OcclusionIDsTemp);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString OcclusionValue : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*OcclusionValue)) {
			OcclusionsStruct.OcclusionIDsIntArr.Add(FCString::Atoi(*OcclusionValue));
		} else { SamplesLost++; }
	}
	
	if (OcclusionsStruct.bAllLocationsLoaded) {
		OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionsStruct.OcclusionIDsIntArr);
		OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, true, false);
	}else {
		UE_LOG(LogExperiment, Fatal,
			TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Occlusions are NOT loaded!"));
	}
	
	if (GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] RequestRemoveDelegates(GetOcclusionsRequest)"));
		this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
	}else {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
	}
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsTimedOut] Get occlusion request timed out!"));
	if (!GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsTimedOut] Failed to remove delegates, GetOcclusionsRequest not valid."));
		return;
	}
	
	UE_LOG(LogExperiment, Log,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Removed delegates for: GetOcclusionsRequest"));
	this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
}

bool AExperimentServiceMonitor::SendGetOcclusionLocationsRequest() {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Starting request."));
	if (!ensure(ValidateClient(TrackingClient))) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid."));
		return false;
	}

	GetOcclusionLocationRequest = TrackingClient->SendRequest("get_cell_locations", "21_05", 10.0f);
	if (!ensure(GetOcclusionLocationRequest)) { return false; }
	GetOcclusionLocationRequest->AddToRoot();
	
	GetOcclusionLocationRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse);
	GetOcclusionLocationRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut);

	return true;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {
	UE_LOG(LogExperiment, Log,
		TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Response:%s"),
		*ResponseIn);

	if (!OcclusionsStruct.bAllLocationsLoaded) { OcclusionsStruct.SetAllLocations(ResponseIn); }
	if (!OcclusionsStruct.bSpawnedAll){
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Spawning all!"));
		constexpr int HeightScaleFactor = 3;
		OcclusionsStruct.SpawnAll(GetWorld(), true, false,
			FVector(WorldScale, WorldScale, WorldScale*HeightScaleFactor));
	} else {
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] All locations already spawned. Skipping spawn!"));
	}

	if (GetOcclusionLocationRequest) {
		RequestRemoveDelegates(GetOcclusionLocationRequest, "GetOcclusionLocationRequest");
		GetOcclusionLocationRequest->RemoveFromRoot();
	}

	UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Exited OK"))

	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Failed to SendGetOcclusionsRequest"))
	} else {
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Sent SendGetOcclusionsRequest OK!"))
	}
	
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));

	// get locations in our specific world configuration (21_05 by defualt)
	if (OcclusionsStruct.bAllLocationsLoaded && OcclusionsStruct.bSpawnedAll) {
		UE_LOG(LogExperiment, Warning,
			TEXT("[HandleGetOcclusionLocationsTimedOut] Occlusions already set, we can bypass this!"));
		if (!this->SendGetOcclusionsRequest()) {
			UE_LOG(LogExperiment, Log,
				TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Failed to SendGetOcclusionsRequest"))
		} else {
			UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] Sent SendGetOcclusionsRequest OK!"))
		}
	}
}

bool AExperimentServiceMonitor::ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn) {
	UE_LOG(LogExperiment, Log,
		TEXT("[AExperimentServiceMonitor::ConnectToServer] Attempting connection: %s:%i"),
		*IPAddressIn, PortIn)

	if (!ClientIn->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error, TEXT("[ConnectToServer()] Failed to validate client!"));
		return false; 
	}
	uint8 AttemptCurr = 0;

	while (AttemptCurr < MaxAttemptsIn) {
		if (ClientIn->Connect(IPAddressIn, PortIn)) {
			UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::ConnectToServer()] Success (attempt #: %i/%i)"), AttemptCurr + 1, MaxAttemptsIn);
			break;
		}
		AttemptCurr += 1;
	}

	if (!ClientIn->IsConnected()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::ConnectToServer()] Failed to connect to server!"));
		return false;
	}
	
	return true;
}

bool AExperimentServiceMonitor::RoutePredatorMessages() {
	if (!this->ValidateClient(TrackingClient)) {
		printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] TrackingClient not valid.");
		return false;
	}
	if (MessageRoutePredator->IsValidLowLevelFast()){ MessageRoutePredator = nullptr; }
	MessageRoutePredator = TrackingClient->AddRoute(ExperimentHeaders.PredatorStep);
	if (!MessageRoutePredator) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::RoutePredatorMessages] MessageRoutePredator not valid."));
		return false;
	}

	MessageRoutePredator->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleUpdatePredator);
	if (TrackingClient->IsValidLowLevel()){ TrackingClient->UnroutedMessageEvent.RemoveAll(this); }
	TrackingClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleUnroutedMessage);

	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::RoutePredatorMessages] OK"));
	return true;
}

bool AExperimentServiceMonitor::RouteOnCapture() {
	if (!ensure(this->ValidateClient(TrackingClient))) { return false; }
	
	MessageRouteOnCapture = TrackingClient->AddRoute("on_capture");
	if (!ensure(MessageRouteOnCapture->IsValidLowLevelFast())) { return false; }
	
	MessageRouteOnCapture->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleOnCapture);
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::RouteOnCapture] OK"));
	return true;
}

bool AExperimentServiceMonitor::SetupConnections() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SetupConnections] Entering"));
	return false;

	if (TrackingClient->IsValidLowLevelFast() && TrackingClient->IsConnected()) {
		UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::SetupConnections] disconnecting"));
		if (!TrackingClient->Disconnect()){
			UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SetupConnections] disconnect FAILED"));
		}else {
			UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupConnections] disconnect OK"));
		}
	}

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupConnections] setting trackingclient to nullptr"));
	TrackingClient = nullptr;
	TrackingClient = this->CreateNewClient();
	if (!ensure(TrackingClient->IsValidLowLevelFast())) { return false; }
	
	constexpr int AttemptsMax = 3;

	if (!this->ConnectToServer(TrackingClient, AttemptsMax, *ServerInfo.IP, ServerInfo.Port)) {
		UE_LOG(LogExperiment, Fatal,
			TEXT("[AExperimentServiceMonitor::SetupConnections] Connect to Tracking: Failed! (IP: %s)"), *ServerInfo.IP);
		return false;
	}

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupConnections()] Connect to Tracking: OK! (IP: %s)"),
		*ServerInfo.IP);
	
	if (!ensure(this->RoutePredatorMessages())) { return false;  }
	if (!ensure(this->RouteOnCapture())) { return false; }
	if (ExperimentManager && ExperimentManager->bSubscribed) {
		if (TrackingClient) {
			UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupConnections()] Unsubscribing"))
			FMessage Message = FMessage();
			Message.header = "!unsubscribe";
			Message.body   = "";
			if(TrackingClient->SendMessage(Message)) {
				ExperimentManager->bSubscribed = false;
				UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetupConnections()] Unsubscribing OK"))
			}
		}
	}
	if (!ensure(this->SubscribeToTracking())) { return false; }
	
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SetupConnections] Exiting OK"));
	return true;
}

bool AExperimentServiceMonitor::Test() {

	TrackingClient = this->CreateNewClient();
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::Test] Connecting to IP:Port => %s:%i."),
		*ServerInfo.IP, ServerInfo.Port);

	/* connect tracking service */
	constexpr int AttemptsMax = 3;

	if (!this->ConnectToServer(TrackingClient, AttemptsMax, *ServerInfo.IP, ServerInfo.Port)) {
		UE_LOG(LogExperiment, Fatal,
			TEXT("[AExperimentServiceMonitor::Test] Connect to Tracking: Failed! (IP: %s)"), *ServerInfo.IP);
		return false;
	}

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::Test()] Connect to Tracking: OK! (IP: %s)"),
		*ServerInfo.IP);
	
	if (!ensure(this->RoutePredatorMessages())) { return false;  }
	if (!ensure(this->RouteOnCapture())) { return false; }
	if (!ensure(this->SubscribeToTracking())) { return false; }

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->SetupPlayerUpdatePosition()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::Test] Player Pawn NOT found!"));
		return false;
	}
	
	UExperimentMonitorData* NewExperimentMonitorData = NewObject<UExperimentMonitorData>(this);
	if (ExperimentManager) {
		PlayerIndex = ExperimentManager->RegisterNewPlayer(PlayerPawn, NewExperimentMonitorData);
		ExperimentManager->SetActivePlayerIndex(PlayerIndex);
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::Test] Failed to Register new player. ExperimentManager not valid!"))
	}

	/* get cell locations */
	if (!this->SendGetOcclusionLocationsRequest()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::Test] Failed to SendGetOcclusionLocationsRequest"))
	}

	/* start experiment  */
	return true;
}

void AExperimentServiceMonitor::SetOcclusionVisibility(bool bNewVisibility) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetOcclusionVisibility] Called"))
	if (ensure(OcclusionsStruct.bCurrentLocationsLoaded)) {
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SetOcclusionVisibility] Using last loaded locations (%i)"),
			OcclusionsStruct.OcclusionIDsIntArr.Num())
		OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, !bNewVisibility, false);
	}
}

void AExperimentServiceMonitor::OnEpisodeStarted() {

	// todo: get proper index from ExperimentManager; either fetch or pass as input 
	UE_LOG(LogExperiment, Log,
		TEXT("[AExperimentServiceMonitor::OnEpisodeStarted] Episode started player index: %i"),
		PlayerIndex)

	// if (ensure(OcclusionsStruct.bCurrentLocationsLoaded)) {
	// 	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnEpisodeStarted] Using last loaded locations (%i)"),
	// 		OcclusionsStruct.OcclusionIDsIntArr.Num())
	// 	OcclusionsStruct.SetVisibilityArr(OcclusionsStruct.OcclusionIDsIntArr, false, false);
	// }

	if (!ensure(PlayerPawn->IsValidLowLevel())) { return; }
	
	FrameCountPrey = 0;
	ExperimentInfo.SetStatus(EExperimentStatus::InEpisode);
	this->SetPredatorIsVisible(true);
	bCanUpdatePrey = true;
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnEpisodeStarted] Exiting OK"))
}

void AExperimentServiceMonitor::ResetWorldState() {
	bCanUpdatePrey = false;
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	OcclusionsStruct.SetAllHidden();
	this->SetPredatorIsVisible(false);
}

void AExperimentServiceMonitor::OnEpisodeFinished() {
	this->ResetWorldState();
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::OnEpisodeFinished] OK"));
}

void AExperimentServiceMonitor::HandleUpdatePredator(const FMessage MessageIn) {
	// UE_LOG(LogExperiment, Warning, TEXT("Predator: %s"), *MessageIn.body);
	this->UpdatePredator(MessageIn);
}

void AExperimentServiceMonitor::HandleOnCapture(const FMessage MessageIn) {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleOnCapture]"));
	UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::HandleOnCapture] Broadcasting OnEpisodeFinishedSuccessDelegate"));

	if (!ensure(ExperimentManager->IsValidLowLevelFast())) { return; }
	if (!ensure(ExperimentManager->IsInEpisode())) { return; }
	
	// ExperimentManager->OnEpisodeFinishedSuccessDelegate.Broadcast();
	ExperimentManager->ProcessStopEpisodeResponse();
}

float AExperimentServiceMonitor::GetTimeRemaining() const {
	if (!TimerHandlePtr->IsValid()){ return -1.0f; }
	if (!GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr)) { return -1.0f; }
	return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);	
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay() {
	Super::BeginPlay();
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::BeginPlay()] Starting!"));

	ExperimentInfo.OnExperimentStatusChangedEvent.AddDynamic(this, &AExperimentServiceMonitor::OnStatusChanged);
	
	// get world scale
	UGameInstanceMain* GI = Cast<UGameInstanceMain>(GetWorld()->GetGameInstance());
	check(GI->IsValidLowLevelFast())
	if (GI->IsValidLowLevelFast()) { this->WorldScale = GI->GetLevelScale(nullptr).X; }
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::BeginPlay()] WorldScale: %0.2f"), this->WorldScale);

	if(this->SpawnAndPossessPredator()) { UE_LOG(LogExperiment, Log, TEXT("Spawned predator: OK")); }
	else{ UE_LOG(LogExperiment, Error, TEXT("Spawned predator: FAILED")); UE_DEBUG_BREAK(); }
	
	ExperimentManager = NewObject<UExperimentManager>(this, UExperimentManager::StaticClass());
	ensure(ExperimentManager->IsValidLowLevel());

	if (ExperimentManager) {
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

		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::Test] Bound delegates: ExperimentManager"))
	}
	Test();
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (Stopwatch && Stopwatch->IsRunning()) { Stopwatch->Reset(); };
	if (TrackingClient->IsValidLowLevelFast()) { TrackingClient->Disconnect(); }
	
	/* clear delegates */
}
