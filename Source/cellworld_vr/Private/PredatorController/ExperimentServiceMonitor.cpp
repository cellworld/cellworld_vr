// ReSharper disable CppTooWideScopeInitStatement
#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AExperimentServiceMonitor::OnExperimentFinished(const int InPlayerIndex) {

	UE_LOG(LogExperiment, Log, TEXT("OnExperimentFinished: Player #%i finished!"), InPlayerIndex)
	
	UExperimentMonitorData* Data = ExperimentManager->GetDataByIndex(InPlayerIndex);
	if (Data->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Log,
			TEXT("OnExperimentFinished: Player #%i finished: Episodes: %i; TotalTime: %0.2f;"),
			InPlayerIndex, Data->EpisodesCompleted, Data->EpisodesCompletedTime)
	} else {
		UE_LOG(LogExperiment, Error, TEXT("Failed to get Data for Player #%i !"), InPlayerIndex)
	}
	
	if (!PlayerPawn->IsValidLowLevelFast() || !PlayerPawn->PlayerHUD->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error, TEXT("OnExperimentFinished: Failed to get PlayerPawn and exit."))
		return; 
	}
	PlayerPawn->PlayerHUD->SetNotificationVisibility(ESlateVisibility::Visible);

	/* todo; unregister player from ExperimentManager */
	UE_LOG(LogExperiment, Log, TEXT("OnExperimentFinished Player #%i: Called everything OK!"), InPlayerIndex)
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

	this->PredatorBasic->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f)*this->PredatorScaleFactor*this->WorldScale);
	return true;
}

/* stop connection for ClientIn */
bool AExperimentServiceMonitor::Disconnect(UMessageClient* ClientIn)
{
	if (ClientIn->IsValidLowLevelFast() && ClientIn->IsConnected())
	{
		printScreen("[AExperimentServiceMonitor::Disconnect] Client Disconnected: Processing.");
		return ClientIn->Disconnect();
	}
	printScreen("[AExperimentServiceMonitor::Disconnect] Client Disconnected: Failed.");
	return false; 
}

UMessageClient* AExperimentServiceMonitor::CreateNewClient()
{
	UMessageClient* NewClient = UMessageClient::NewMessageClient();
	return NewClient; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString& ExperimentNameIn) {
	if (!this->ValidateClient(Client)) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, Experiment Service client not valid.")); return false; }
	if (ExperimentInfo.ExperimentNameActive == "") {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, not an active experiment."));
		return false;
	}

	FFinishExperimentRequest RequestBody;
	RequestBody.experiment_name = ExperimentInfo.ExperimentNameActive;

	const FString RequestString = UExperimentUtils::FinishExperimentRequestToJsonString(RequestBody);
	StopExperimentRequest = Client->SendRequest("finish_experiment", RequestString, TimeOut);
	printScreen(RequestString);
	if (!StopExperimentRequest) { return false; }

	StopExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentResponse); // uses same one as start/stop
	StopExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentTimedOut);
	
	if (!StopExperimentRequest->IsValidLowLevelFast()) { return false; }
	return true; 
}

void AExperimentServiceMonitor::HandleStopExperimentResponse(const FString ResponseIn)
{
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] %s"), *ResponseIn);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Experiment response: %s"), *ResponseIn));

	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;

	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] Experiment name: %s"), *ExperimentNameActive);

}

void AExperimentServiceMonitor::HandleStopExperimentTimedOut()
{
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentTimedOut()] Finish experiment request timed out!"))
}

URequest* AExperimentServiceMonitor::SendStartExperimentRequest(UMessageClient* ClientIn, FString ExperimentNameIn)
{
	/* set up world info (defaults to hexagonal and canonical) */
	FWorldInfo WorldInfo;
	WorldInfo.occlusions = "21_05";

	/* set up request body */
	FStartExperimentRequest StartExperimentRequestBody;
	StartExperimentRequestBody.duration = 30;
	StartExperimentRequestBody.subject_name = "ExperimentNameIn";
	StartExperimentRequestBody.world = WorldInfo;

	const FString StartExperimentRequestBodyString = UExperimentUtils::StartExperimentRequestToJsonString(StartExperimentRequestBody);

	URequest* Request = ClientIn->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);
	
	if (!Request) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return nullptr; }
	
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	
	ExperimentInfo.Status = EExperimentStatus::WaitingExperiment;
	ExperimentInfo.StartExperimentRequestBody = StartExperimentRequestBody;
	return Request;
}

/* start experiment service episode stream
 * should ONLY be called from game mode by the BP_door trigger !!!  */
bool AExperimentServiceMonitor::StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn) {

	if (ExperimentInfo.Status == EExperimentStatus::InEpisode) {
		printScreen("[AExperimentServiceMonitor::StartEpisode] Already in Episode!");
		return false;
	}
	
	if (!this->ValidateClient(ClientIn)) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StartEpisode] Can't start episode, client not valid."));
		return false;
	}
	
	if (!this->ValidateExperimentName(ExperimentNameIn)){
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StartEpisode] Can't start episode, experiment name not valid."));
		return false;
	}

	FStartEpisodeRequest RequestBody;
	RequestBody.experiment_name = ExperimentInfo.ExperimentNameActive;
	
	const FString RequestString = UExperimentUtils::StartEpisodeRequestToJsonString(RequestBody);
	StartEpisodeRequest = ClientIn->SendRequest("start_episode", RequestString, TimeOut);
	
	if (!StartEpisodeRequest) { return false; }

	StartEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestResponse);
	StartEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut);
	
	return true;
}

bool AExperimentServiceMonitor::ValidateClient(UMessageClient* ClientIn)
{
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

bool AExperimentServiceMonitor::ValidateExperimentName(const FString& ExperimentNameIn)
{
	if (ExperimentNameIn == "") { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode() {
	
	if (ExperimentInfo.Status != EExperimentStatus::InEpisode) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, not in episode."));
		return false;
	}
	
	ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode);
	double ElapsedTime = -1.0f;
	if (StopTimerEpisode(ElapsedTime)) {
		// ExperimentManager->OnEpisodeFinished(PlayerIndex, ElapsedTime);
		UE_LOG(LogExperiment, Log, TEXT("Player 0: Finished episode. Collected %i samples in %0.2f seconds (~%0.2f) Hz"),
			FrameCountPrey, ElapsedTime, (float)FrameCountPrey/ElapsedTime)
		ExperimentManager->OnEpisodeFinishedDelegate.Broadcast(PlayerIndex, ElapsedTime);
	} else {
		UE_LOG(LogExperiment, Error, TEXT("Failed to StopTimerEpisode() in StopEpisode()"))
	}
	
	if (!ValidateClient(Client)) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::StopEpisode()] Can't stop episode, Experiment Service client not valid."));
		return false;
	}

	// ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	FFinishEpisodeRequest RequestBody;
	RequestBody.experiment_name = ExperimentInfo.ExperimentNameActive;

	const FString RequestString = UExperimentUtils::FinishEpisodeRequestToJsonString(RequestBody);
	StopEpisodeRequest = Client->SendRequest("finish_episode", RequestString, 5.0f);
	if (!StopEpisodeRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::StopEpisode] StopEpisodeRequest not valid. Failed to Bind Responses."))
		return false;
	}
	
	StopEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestResponse);
	StopEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut);
	return true;
}

bool AExperimentServiceMonitor::StartTimerEpisode() {
	if (Stopwatch->IsValidLowLevel() && Stopwatch->IsRunning()) {
		UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::StartTimerEpisode] Can't start timer. bTimerRunning is True."))
		return false;
	}

	Stopwatch = NewObject<UStopwatch>(this, UStopwatch::StaticClass());

	if (!Stopwatch->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StartTimerEpisode()] UStopwatch is NULL!"))
		return false;
	}
	Stopwatch->Start(); 
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::StartTimerEpisode] Started timer OK!"))
	return true;
}

bool AExperimentServiceMonitor::StopTimerEpisode(double& InElapsedTime) {
	if (!Stopwatch->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopTimerEpisode] Failed, Stopwatch  is null."));
		return false;
	}

	if (!Stopwatch->IsRunning()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopTimerEpisode] Failed, Stopwatch not running."));
		return false; 
	}

	InElapsedTime = Stopwatch->Lap();
	Stopwatch->Reset();
	// Stopwatch->MarkAsGarbage();
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::StopTimerEpisode] OK"));
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStartEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[HandleStartEpisodeRequestResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response != "success") {
		// avoid multiple calls if already in episode and avoid setting status as error (just an artifact)
		if (ExperimentInfo.Status == EExperimentStatus::InEpisode) {
			UE_LOG(LogExperiment, Warning,
				TEXT("[HandleStartEpisodeRequestResponse] Failed to start episode! Already in episode"));
			return;
		}
		
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleStartEpisodeRequestResponse] Failed to start episode! Unkown reason!"))
		// ExperimentInfo.SetStatus(EExperimentStatus::ErrorStartEpisode);
		this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");
		return;
	}

	this->RequestRemoveDelegates(StartEpisodeRequest, "StartEpisodeRequest");

	UE_LOG(LogExperiment, Log, TEXT("[HandleGetOcclusionsResponse] Preparing to ResetAgentTracking!"))	
	if (!this->ResetTrackingAgent()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleGetOcclusionsResponse] ResetTrackingAgent() Failed!"))
	} else {
		UE_LOG(LogExperiment, Log,
			TEXT("[HandleGetOcclusionsResponse] ResetTrackingAgent() OK!"))
	}
}

bool AExperimentServiceMonitor::ResetTrackingAgent() {
	UE_LOG(LogExperiment, Log, TEXT("[ResetTrackingAgent] Preparing to send!"))
	ResetRequest = TrackingClient->SendRequest("reset","",15.0f);
	if (!ResetRequest) {
		UE_LOG(LogExperiment, Error, TEXT("[ResetTrackingAgent] Failed to create and send ResetRequest!"));
		return false; 
	}
	
	ResetRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestResponse);
	ResetRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestTimedOut);
	UE_LOG(LogExperiment, Log, TEXT("[ResetTrackingAgent] Sent OK!"))
	return true; 
}

void AExperimentServiceMonitor::HandleResetRequestResponse(const FString InResponse) {
	UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestResponse] Response: %s"),*InResponse)
	if (InResponse != "success") {
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorResetTrackingAgent);
		if (ExperimentManager) {
			UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestResponse] Broadcasting OnResetResultDelegate false!"))
			ExperimentManager->OnResetResultDelegate.Broadcast(false);
		}
		return; 
	}

	if (ExperimentManager) {
		ExperimentManager->OnResetResultDelegate.Broadcast(true);
		UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestResponse] Broadcasting OnResetResultDelegate true!"))
	}else {
		UE_LOG(LogExperiment, Error, TEXT("[HandleResetRequestResponse] ExperimentManager NULL!"))
	}

	// ExperimentInfo.SetStatus(EExperimentStatus::InEpisode);
	
	FrameCountPrey = 0;
	// ExperimentInfo.SetStatus(EExperimentStatus::InEpisode); // todo: change flag 
	UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestResponse] ResetTrackingAgent OK!"))
	this->RequestRemoveDelegates(ResetRequest, "ResetRequest");

	/* get cell locations */
	if (!this->SendGetOcclusionLocationsRequest()) {
		UE_LOG(LogExperiment, Error, TEXT("HandleStartEpisodeRequestResponse: Failed to SendGetOcclusionLocationsRequest"))
	}
}

void AExperimentServiceMonitor::HandleResetRequestTimedOut() {
	if (PlayerPawn && PlayerPawn->PlayerHUD) {
		UE_LOG(LogExperiment, Log,
			TEXT("[HandleResetRequestTimedOut] Notifying timeout to PlayerPawn->PlayerHUD!"))
		PlayerPawn->PlayerHUD->SetNotificationText("Reset Request timed out!");
		PlayerPawn->PlayerHUD->SetNotificationVisibility(ESlateVisibility::Visible);
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleResetRequestTimedOut] Can't notify with PlayerHUD. PlayerPawn or PlayerHUD is NULL!"))
	}

	if (ExperimentManager) {
		UE_LOG(LogExperiment, Log, TEXT("[HandleResetRequestTimedOut] Broadcasting OnResetResultDelegate false!"))
		ExperimentManager->OnResetResultDelegate.Broadcast(false);
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleResetRequestTimedOut] Failed to broadcast OnResetResultDelegate, Experiment Manager is NULL!"))
	}

	this->RequestRemoveDelegates(ResetRequest, "ResetRequest");
	ResetRequest = nullptr;
	UE_LOG(LogExperiment, Fatal, TEXT("[HandleResetRequestTimedOut] Reset request timed out!"))
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
	// todo: bind to state machine FOnStatusChanged -> InEpisode (and move InEpisode to after reset success)
	bCanUpdatePrey = bResetResult;
	UE_LOG(LogExperiment, Log, TEXT("[OnResetResult] bCanUpdatePrey = %i"), bResetResult)

	if (!bResetResult) {
		UE_LOG(LogExperiment, Error, TEXT("[OnResetResult] Reset Failed!"))
		ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);	
		return; 
	}

	ExperimentInfo.SetStatus(EExperimentStatus::InEpisode);
	
	if (!this->StartTimerEpisode()) {
		UE_LOG(LogExperiment, Error, TEXT("[OnResetResult] Failed to start Timer!"))
	}
	
	if (!PlayerPawn->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error, TEXT("[OnResetResult] Could not find valid player pawn and will start sampling!"))
		return; 
	}
	
	UE_LOG(LogExperiment, Log, TEXT("[OnResetResult] Found valid player pawn"))
	
	// if(!PlayerPawn->StartPositionSamplingTimer(PositionSamplingRate)) {
	// 	UE_LOG(LogExperiment, Error, TEXT("OnResetResult Failed to StartPositionSamplingTimer!"))
	// 	return; 
	// }

	UE_LOG(LogExperiment, Log, TEXT("[OnResetResult] OK!"))
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut()] Episode request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStopEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[HandleStopEpisodeRequestResponse] %s"), *response);
	
	if (response == "fail") {
		UE_DEBUG_BREAK();
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorFinishEpisode);
		UE_LOG(LogExperiment, Error, TEXT("[HandleStopEpisodeRequestResponse] Failed to finish episode ENUM."));
		return;
	}
	
	// ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode); // todo: make permanent (delete below)
	bCanUpdatePrey = false;
	
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	OcclusionsStruct.SetAllHidden();
	
	// todo: make this primary function
	if (TrackingClient->IsValidLowLevelFast()) {
		FMessage MessageOut;
		MessageOut.header = "stop";
		MessageOut.body   = ""; //todo: add time elapsed info
		
		if (ExperimentManager->IsValidLowLevel()) {
			if (const UExperimentMonitorData* DataTemp = ExperimentManager->GetDataByIndex(PlayerIndex)) {
				MessageOut.body = FString::SanitizeFloat(DataTemp->EpisodesCompletedTime);
				UE_LOG(LogExperiment, Log, TEXT("DataTemp->EpisodesCompletedTime: %s"),*MessageOut.body)
			}
		}
		
		UE_LOG(LogExperiment, Log, TEXT("StopEpisode to AT: %s"),*MessageOut.body)
		const bool bMessageResult = TrackingClient->SendMessage(MessageOut);
		if (!bMessageResult) { printScreen("Failed to send STOP! Is TrackingClient connected? "); }
	} else { printScreen("TrackingClient not valid! Failed to send STOP!"); }

	// cleanup
	if (StopEpisodeRequest->IsValidLowLevelFast()) {
		this->RequestRemoveDelegates(StopEpisodeRequest, "StopEpisodeRequest");
		StopEpisodeRequest = nullptr;
		printScreen("Removing delegates: StopEpisodeRequest");
	} else { printScreen("Removing delegates: StopEpisodeRequest not valid!"); }
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn) {
	/* convert to usable format */
	ExperimentInfo.StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentInfo.ExperimentNameActive    = ExperimentInfo.StartExperimentResponse.experiment_name;
	
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive)) {
		UE_LOG(LogExperiment, Error,
			TEXT("[HandleStartExperimentResponse] Failed to start experiment. ExperimentNameIn not valid."));
		UE_DEBUG_BREAK();
		ExperimentInfo.Status = EExperimentStatus::ErrorStartExperiment;
	}
	
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Experiment name: %s"),
		*ExperimentInfo.StartExperimentResponse.experiment_name);
	
	ExperimentInfo.SetStatus(EExperimentStatus::InExperiment);
		// this->StartEpisode(Client, ExperimentInfo.ExperimentNameActive); // only used to debug VR 
}

void AExperimentServiceMonitor::HandleStartExperimentTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleStartExperimentTimedOut] Start Experiment TIMED OUT!");
}

/* update predator's location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(const FMessage& InMessage) {
	if (ExperimentInfo.Status != EExperimentStatus::InEpisode) { return; }

	// UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::UpdatePredator] Received valid predator step."));
	if (PredatorBasic->IsValidLowLevelFast()) {
		// ReSharper disable once CppUseStructuredBinding
		const FStep StepOut = UExperimentUtils::JsonStringToStep(InMessage.body);
		PredatorBasic->SetActorLocation(UExperimentUtils::CanonicalToVr(StepOut.location, MapLength, WorldScale));
		PredatorBasic->SetActorRotation(FRotator(0.0,StepOut.rotation,0.0f));
		FrameCountPredator++;
	} else {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::UpdatePredator] Received valid predator step."));
	}
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector InVector, const FRotator InRotation) {
	// if (ExperimentInfo.Status != EExperimentStatus::InEpisode) { return; }
	if (!bCanUpdatePrey) { return; }
	
	if (!this->ValidateClient(TrackingClient)) {
		// todo: raise error to player hud 
		printScreen("[AExperimentServiceMonitor::UpdatePreyPosition] Cannot send step. Client not valid.");
		return;
	}

	/* prepare Step */
	FStep Step;
	Step.data = "VR";
	Step.agent_name = "prey";
	Step.frame = FrameCountPrey;
	Step.location = UExperimentUtils::VrToCanonical(InVector, MapLength, this->WorldScale);
	Step.rotation = InRotation.Yaw; // todo: get actual rotation of player  
	if (Stopwatch) {
		Step.time_stamp = Stopwatch->GetElapsedTime(); 
	}else {
		Step.time_stamp = -1.0f; 
	}
	const FMessage MessageOut = UMessageClient::NewMessage("prey_step",
		UExperimentUtils::StepToJsonString(Step));
	
	if (!TrackingClient->SendMessage(MessageOut)) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::UpdatePreyPosition] Failed: Send prey step!"))
		return;
	}
	FrameCountPrey += 1; 
}

void AExperimentServiceMonitor::HandleUnroutedMessage(const FMessage InMessage) {
	printScreen("[AExperimentServiceMonitor::HandleUnroutedMessage] " 
		+ InMessage.header + " | " +  InMessage.body);
}

/* todo: finish this */
bool AExperimentServiceMonitor::IsExperimentActive(const FString ExperimentNameIn) {
	return false;
}

/* gets player pawn from world */
bool AExperimentServiceMonitor::GetPlayerPawn() {
	if (!GetWorld()) { return false; }
	
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] No pawn found."))
		return false;
	}
	
	if (Cast<APawnMain>(Pawn)) {
		PlayerPawn = Cast<APawnMain>(Pawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnMain found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] No valid pawn found nor assigned."));
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
	if (!TrackingSubscribeRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Fatal, TEXT("[SubscribeToTracking] TrackingSubscribeRequest not valid."))
		return false;
	}

	TrackingSubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingResponse);
	TrackingSubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut);
	UE_LOG(LogExperiment, Log, TEXT("[SubscribeToTracking] OK!"))
	return true;
}

void AExperimentServiceMonitor::RequestRemoveDelegates(URequest* InRequest, const FString& InRequestString) {
	bool Result = false; 
	if (InRequest->IsValidLowLevelFast()) {
		InRequest->ResponseReceived.RemoveAll(this);
		InRequest->TimedOut.RemoveAll(this);
		InRequest = nullptr; 
		Result = true; 
	} else {
		UE_LOG(LogExperiment, Error, TEXT("[RequestRemoveDelegates] Failed. RequestIn (%s) NULL"), *InRequestString)
	}
	UE_LOG(LogExperiment, Log, TEXT("[RequestRemoveDelegates] Request: %s; Result: %i"), *InRequestString, Result)
}

void AExperimentServiceMonitor::OnStatusChanged(EExperimentStatus ExperimentStatusIn) {
	const FString StatusString = FExperimentInfo::GetStatusString(&ExperimentStatusIn);
	UE_LOG(LogExperiment, Log, TEXT("[OnStatusChanged] New status: %s"), *StatusString)
}

bool AExperimentServiceMonitor::SendGetOcclusionsRequest() {
	if (!Client) {
		UE_LOG(LogExperiment, Error, TEXT("[SendGetOcclusionsRequest] Cant send get occlusion request, Experiment service client NULL."));
		return false;
	} // experiment service client not valid

	GetOcclusionsRequest = Client->SendRequest("get_occlusions", "21_05", TimeOut);

	if (!GetOcclusionsRequest) {
		UE_LOG(LogExperiment, Error, TEXT("[SendGetOcclusionsRequest] SendGetOcclusionsRequest Failed. GetOcclusionsRequest NULL."))
		return false;
	}
	
	GetOcclusionsRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	GetOcclusionsRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);
	return true;
}

void AExperimentServiceMonitor::HandleGetOcclusionsResponse(const FString ResponseIn)  {
	UE_LOG(LogExperiment, Warning, TEXT("HandleGetOcclusionsResponse: Occlusion IDs (raw): %s"), *ResponseIn);

	/* start empty */
	OcclusionIDsIntArr.Empty(); 
	TArray<int32> OcclusionIDsTemp; 

	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr; 
	const FString OcclusionIDs_temp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	const int32 Samples = OcclusionIDs_temp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogExperiment, Log, TEXT("[HandleGetOcclusionsResponse] Number of occlusions in configuration: %i"),Samples);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString OcclusionValue : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*OcclusionValue)) OcclusionIDsTemp.Add(FCString::Atoi(*OcclusionValue));
		else { SamplesLost++; }
	}

	OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionIDsTemp);
	OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp, false, true);
	
	if (GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Log,TEXT("[HandleGetOcclusionsResponse] RequestRemoveDelegates(GetOcclusionsRequest)"));
		this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
	}else {
		UE_LOG(LogExperiment, Error,TEXT("[HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
	}
	
	// UE_LOG(LogExperiment, Log, TEXT("[HandleGetOcclusionsResponse] Preparing to ResetAgentTracking!"))	
	// if (!this->ResetTrackingAgent()) {
	// 	UE_LOG(LogExperiment, Error,
	// 		TEXT("[HandleGetOcclusionsResponse] ResetTrackingAgent() Failed!"))
	// } else {
	// 	UE_LOG(LogExperiment, Log,
	// 		TEXT("[HandleGetOcclusionsResponse] ResetTrackingAgent() OK!"))
	// }
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("Get occlussion request timed out!"));
	if (!GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
		return;
	}
	
	UE_LOG(LogExperiment, Log,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Removed delegates for: GetOcclusionsRequest"));
	this->RequestRemoveDelegates(GetOcclusionsRequest, "GetOcclusionsRequest");
}

bool AExperimentServiceMonitor::SendGetOcclusionLocationsRequest() {
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Starting request."));
	if (!this->ValidateClient(Client)) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid."));
		return false;
	}
	
	const FString BodyOut   = "21_05";
	const FString HeaderOut = "get_cell_locations";
	GetOcclusionLocationRequest = Client->SendRequest("get_cell_locations", "21_05", 10.0f);
	
	if (!GetOcclusionLocationRequest) { return false; }
	
	GetOcclusionLocationRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse);
	GetOcclusionLocationRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut);

	return true;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {

	if (!OcclusionsStruct.bAllLocationsLoaded) { OcclusionsStruct.SetAllLocations(ResponseIn); }
	if (!OcclusionsStruct.bSpawnedAll){
		constexpr int HeightScaleFactor = 3;
		OcclusionsStruct.SpawnAll(GetWorld(), true, false,
			FVector(WorldScale, WorldScale, WorldScale*HeightScaleFactor));
	} else {
		UE_LOG(LogExperiment, Log, TEXT("[HandleGetOcclusionLocationsResponse] All locations already spawned. Skipping spawn!"));
	}

	RequestRemoveDelegates(GetOcclusionLocationRequest, "GetOcclusionLocationRequest");

	// get locations in our specific world configuration (21_05 by defualt) 
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogExperiment, Log,
			TEXT("[HandleGetOcclusionLocationsResponse] Failed to SendGetOcclusionsRequest"))
	} else {
		UE_LOG(LogExperiment, Log, TEXT("[HandleGetOcclusionLocationsResponse] Sent SendGetOcclusionsRequest OK!"))
	}
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));
	return;
}

bool AExperimentServiceMonitor::ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn) {
	if (!ClientIn->IsValidLowLevel()) {
		UE_LOG(LogExperiment, Error, TEXT("[ConnectToServer()] Failed to validate client!"));
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
		return false; }
	
	return true;
}

bool AExperimentServiceMonitor::RoutePredatorMessages() {
	if (!this->ValidateClient(TrackingClient)) {
		printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] TrackingClient not valid.");
		return false;
	}
	
	MessageRoutePredator = TrackingClient->AddRoute(ExperimentHeaders.PredatorStep);
	if (!MessageRoutePredator) {printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] RoutePredatorStep not valid."); return false; }

	MessageRoutePredator->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleUpdatePredator);
	TrackingClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleUnroutedMessage);

	printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] OK");
	return true;
}

bool AExperimentServiceMonitor::RemoveDelegatesPredatorRoute() {
	printScreen("[AExperimentServiceMonitor::RemoveDelegatesPredatorRoute()] turned off!");
	return false;
}

bool AExperimentServiceMonitor::Test() {
	/* run netprofile console command */
	const FString InCommand = "netprofile";
	UE_LOG(LogExperiment, Log, TEXT("[ExecuteConsoleCommand] Command: %s"), *InCommand)
	if (GEngine) {
		GEngine->Exec(GWorld, *InCommand); 
	}

	Client         = this->CreateNewClient();
	TrackingClient = this->CreateNewClient();

	/* connect tracking service */
	constexpr int AttemptsMax = 1;
	UE_LOG(LogExperiment, Log,TEXT("[AExperimentServiceMonitor::Test()] Attempting to connect to Experiment Service Client."));
	if (!this->ConnectToServer(Client, AttemptsMax, ServerIPMessage, ServerPort)) {
		printScreen("[AExperimentServiceMonitor::Test] Connect SERVER failed."); return false;
	}

	/* connect to agent tracking */
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::Test()] Attempting to connect to Agent Tracking Client."));
	if (!this->ConnectToServer(TrackingClient, AttemptsMax, ServerIPMessage, TrackingPort)) {
		printScreen("Connect to Tracking: failed."); return false;
	}

	if (!this->RoutePredatorMessages()) {
		UE_LOG(LogExperiment, Error, TEXT("RoutePredatorMessages FAILED!"))
	}

	if (!this->SubscribeToTracking()) {
		UE_LOG(LogExperiment, Error, TEXT("SubscribeToTracking FAILED!"))
	}

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->GetPlayerPawn()) { printScreen("Player Pawn NOT found!"); return false; }
	UExperimentMonitorData* NewExperimentMonitorData = NewObject<UExperimentMonitorData>(this);
	
	if (ExperimentManager) {
		PlayerIndex = ExperimentManager->RegisterNewPlayer(PlayerPawn, NewExperimentMonitorData); //todo: 
	} else {
		UE_LOG(LogExperiment, Error, TEXT(" Failed to Register new player. ExperimentManager not valid!"))
	}

	/* start experiment  */
	StartExperimentRequest = this->SendStartExperimentRequest(Client, "ExperimentNameIn");
	if (!StartExperimentRequest) { return false; }

	return true;
}

void AExperimentServiceMonitor::HandleUpdatePredator(const FMessage MessageIn) {
	// UE_LOG(LogExperiment, Warning, TEXT("Predator: %s"), *MessageIn.body);
	this->UpdatePredator(MessageIn);
}

float AExperimentServiceMonitor::GetTimeRemaining() const {
	if (!TimerHandlePtr->IsValid()){ return -1.0f; }
	if (!GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr)) { return -1.0f; }
	return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);	
}

float AExperimentServiceMonitor::GetTimeElapsed() const {
	if (!TimerHandlePtr->IsValid()) { return -1.0f; }
	if (!GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr)) { return -1.0f; }
	return GetWorld()->GetTimerManager().GetTimerElapsed(*TimerHandlePtr);
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
	ExperimentManager->NotifyOnExperimentFinishedDelegate.AddDynamic(this, &AExperimentServiceMonitor::OnExperimentFinished);
	ExperimentManager->OnSubscribeResultDelegate.AddDynamic(this, &AExperimentServiceMonitor::OnSubscribeResult);
	ExperimentManager->OnResetResultDelegate.AddDynamic(this, &AExperimentServiceMonitor::OnResetResult);
	// main loop 
	Test();
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (Stopwatch && Stopwatch->IsRunning()) { Stopwatch->Reset(); };
	if (TrackingClient->IsValidLowLevelFast()) { TrackingClient->Disconnect(); }
	if (Client->IsValidLowLevelFast()) { Client->Disconnect(); }
	
	/* clear delegates */
	// UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::EndPlay] Removing delegates!"))
	// if (StartExperimentRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(StartExperimentRequest); }
	// else{printScreen("StartExperimentRequest not valid");}
	//
	// if (StartEpisodeRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(StartEpisodeRequest); }
	// else{printScreen("StartEpisodeRequest not valid");}
	//
	// if (TrackingSubscribeRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(TrackingSubscribeRequest); }
	// else{printScreen("TrackingSubscribeRequest not valid");}

	// if (this->RemoveDelegatesPredatorRoute()) { printScreen("MessageRoutePredator: Removed delegates.");
	// }else{printScreen("MessageRoutePredator: not valid, could not remove delegates.");}
	//
	// TrackingClient->Disconnect();
	// Client->Disconnect();

	// if (/*Client->IsValidLowLevelFast() && */IsValid(Client) && Client->IsConnected())
	// {
	// 	if (Client->Disconnect()) { printScreen("Disconnect success."); }
	// 	else{printScreen("Failed to disconnect.");}
	// }

}
