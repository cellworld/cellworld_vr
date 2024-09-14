// ReSharper disable CppTooWideScopeInitStatement
#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

//TODO - add argument to include MessageType (Log, Warning, Error, Fatal)
void printScreen(const FString InMessage) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s"), *InMessage));
	UE_LOG(LogExperiment, Warning, TEXT("%s"), *InMessage);
}

bool AExperimentServiceMonitor::ValidateLevel(UWorld* InWorld, const FString InLevelName) {
	
	if (!InWorld->IsValidLowLevelFast()) { printScreen("[AExperimentServiceMonitor::ValidateLevel] World is not valid."); return false; }
	
	if (!InLevelName.IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("LevelName: %s"), *InLevelName); }
	else { printScreen("[AExperimentServiceMonitor::ValidateLevel] LevelName is empty!"); return false; }

	FString MapName = InWorld->GetMapName();
	MapName.RemoveFromStart(InWorld->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Warning, TEXT("map name: %s"), *MapName);

	return true;
}

/* todo: make this take input ACharacter and spawn that one*/
bool AExperimentServiceMonitor::SpawnAndPossessPredator() {

	if (!GetWorld()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] GetWorld() failed!"));
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
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] Spawn APredatorBasic Failed!"));
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

/* sends request to experiment service to start experiment */
bool AExperimentServiceMonitor::StartExperiment(const FString& ExperimentNameIn) {
	
	// if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not valid.")); return false; }
	// if (!Client->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not connected.")); return false; }
	//
	// /* set up world info (defaults to hexagonal and canonical) */
	// FWorldInfo WorldInfo;
	// WorldInfo.occlusions = "21_05";
	//
	// /* set up request body */
	// FStartExperimentRequest StartExperimentRequestBody;
	// StartExperimentRequestBody.duration = 30;
	// StartExperimentRequestBody.subject_name = ExperimentNameIn;
	// StartExperimentRequestBody.world = WorldInfo;
	//
	// const FString StartExperimentRequestBodyString = UExperimentUtils::StartExperimentRequestToJsonString(StartExperimentRequestBody);
	// // printScreen(StartExperimentRequestBodyString);
	//
	// UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::StartExperiment()] Sending StartExperimentRequest."));
	// StartExperimentRequest = Client->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);
	//
	// if (StartExperimentRequest != nullptr)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return false;
	// }
	//
	// // NOTE: in v2, i would do this binding in whoever receives the request back and bind to other handlers 
	// UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::StartExperiment()] Bound delegates to StartExperimentRequest."));
	// StartExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	// StartExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::StartExperiment()] DON'T USE ME!"));
	return false;
	// return true; 
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

URequest* AExperimentServiceMonitor::SendFinishExperimentRequest(const FString& ExperimentNameIn)
{
	// if (!Client) { return nullptr; }
	// FFinishExperimentRequest RequestBody;
	// RequestBody.experiment_name = ExperimentNameIn;
	//
	// FString RequestString = UExperimentUtils::FinishExperimentRequestToJsonString(RequestBody);
	// StopExperimentRequest = Client->SendRequest("finish_experiment", RequestString, TimeOut);
	//
	// if (!StopExperimentRequest) { return nullptr; }
	//
	// StopExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentResponse); // uses same one as start/stop
	// StopExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentTimedOut);

	return nullptr;
}

void AExperimentServiceMonitor::HandleStopExperimentResponse(const FString ResponseIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] %s"), *ResponseIn);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Experiment response: %s"), *ResponseIn));

	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;

	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] Experiment name: %s"), *ExperimentNameActive);

}

void AExperimentServiceMonitor::HandleStopExperimentTimedOut()
{
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentTimedOut()] Finish experiment request timed out!"))
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
	
	if (!Request) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return nullptr; }
	
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	
	ExperimentInfo.Status = EExperimentStatus::WaitingExperiment;
	ExperimentInfo.StartExperimentRequestBody = StartExperimentRequestBody;
	return Request;
}

/* start experiment service episode stream
 * should ONLY be called from game mode by the BP_door trigger !!!  */
bool AExperimentServiceMonitor::StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn) {
	if (ExperimentInfo.Status == EExperimentStatus::InEpisode){
		printScreen("[AExperimentServiceMonitor::StartEpisode] Already in Episode!");
		return false;
	}
	
	if (!this->ValidateClient(ClientIn)) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartEpisode] Can't start episode, client not valid."));
		return false;
	}
	
	if (!this->ValidateExperimentName(ExperimentNameIn)){
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartEpisode] Can't start episode, experiment name not valid."));
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
	
	if (!ClientIn->IsConnected())
	{
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
	
	if (this->ExperimentInfo.Status != EExperimentStatus::InEpisode) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, not in episode."));
		return false;
	}

	bTimerRunning = GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr);
	if (bTimerRunning) {
		if (!this->StopTimerEpisode(*TimerHandlePtr)) {
			UE_LOG(LogExperiment, Error, TEXT("Failed to stop timer!"))
		}
	}
	
	ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode);
	// todo: get duration of this episode and pass into ExperimentManager->OnEpisodeFinished()
	// ExperimentManager->OnEpisodeFinished(0, 30.f);
	
	if (!this->ValidateClient(Client)) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::StopEpisode()] Can't stop episode, Experiment Service client not valid."));
		return false;
	}
	
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive)) {
		UE_LOG(LogExperiment, Fatal,
			TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, experiment name not valid."));
		return false;
	}
	
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);

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

bool AExperimentServiceMonitor::StartTimerEpisode(const float DurationIn, FTimerHandle& TimerHandleIn) {
	// check(!bTimerRunning);
	if (bTimerRunning) {
		UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::StartTimerEpisode] Can't start timer. bTimerRunning is True."))
		return false;
	}
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandleIn, this,
		&AExperimentServiceMonitor::OnTimerFinished,
		DurationIn, false, -1.0f);

	bTimerRunning = true;
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::StartTimerEpisode] Started timer OK!"))
	return true;
}

bool AExperimentServiceMonitor::StopTimerEpisode(FTimerHandle& TimerHandleIn) {
	if (!TimerHandleIn.IsValid()) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopTimerEpisode] Failed, TimerHandleIn is null."));
		return false;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(TimerHandleIn);
	TimerHandleIn.Invalidate();
	bTimerRunning = TimerHandleIn.IsValid();

	check(!bTimerRunning) // needs false
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::StopTimerEpisode] StopeTimerEpisode OK"));
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStartEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response != "success") {
		// avoid multiple calls if already in episode and avoid setting status as error (just an artifact)
		if (ExperimentInfo.Status == EExperimentStatus::InEpisode) {
			UE_LOG(LogExperiment, Warning,
				TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Failed to start episode! Already in episode"));
			return;
		}
		
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Failed to start episode! Unkown reason!"))
		// ExperimentInfo.SetStatus(EExperimentStatus::ErrorStartEpisode);
		return;
	}
	
	/* resubscribe */
	const bool bSubscribeResult = this->SubscribeToTracking();
	if (!bSubscribeResult) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Failed to subscribe to tracking!"));
		return;
	}
	
	const bool bResetSuccess = this->ResetTrackingAgent();
	check(bResetSuccess)
	if (!bResetSuccess) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] failed to reset tracking agent."));
		return; 
	}
	
	// todo: remove resettrackingagent from SubscribeToTracking(); 
	this->SendGetOcclusionLocationsRequest();
	
	// remove delegates
	if (StartEpisodeRequest->IsValidLowLevelFast()) {
		/*StartEpisodeRequest->ResponseReceived.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestResponse);
		StartEpisodeRequest->TimedOut.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut);*/
		this->RequestRemoveDelegates(StartEpisodeRequest);
		printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Removed bindings from StartEpisodeRequest");
	}
}

bool AExperimentServiceMonitor::ResetTrackingAgent() {
	printScreen("[AExperimentServiceMonitor::ResetTrackingAgent()] Preparing to send!");

	ResetRequest = TrackingClient->SendRequest("reset","",10.0f);
	if (!ResetRequest) {
		UE_LOG(LogExperiment, Error, TEXT("Failed to create reset request!"));
		return false; 
	}
	ResetRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestResponse);
	ResetRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleResetRequestTimedOut);

	return true; 
}

void AExperimentServiceMonitor::HandleResetRequestResponse(const FString InResponse) {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse()] %s"),*InResponse)
	if (InResponse != "success") {
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorResetTrackingAgent);
	}

	const bool bStartTimerResult = this->StartTimerEpisode(float(ExperimentInfo.StartExperimentResponse.duration),
		*TimerHandlePtr);
	
	if (!bStartTimerResult) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse] Failed to start Timer!"))
	}
	
	FrameCountPrey = 0;
	ExperimentInfo.SetStatus(EExperimentStatus::InEpisode); // todo: change flag 
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse] ResetTrackingAgent Successful!"))

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleResetRequestResponse] RequestRemoveDelegates(ResetRequest)"))
	this->RequestRemoveDelegates(ResetRequest);
}

void AExperimentServiceMonitor::HandleResetRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleResetRequestTimedOut()] Reset request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::ErrorTimedOutReset);
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut()] Episode request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStopEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] %s"), *response);
	
	if (response == "fail") {
		UE_DEBUG_BREAK();
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorFinishEpisode);
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] Failed to finish episode ENUM."));
		return;
	}
	
	// ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode); // todo: make permanent (delete below)
	// todo: temp - this will be called in multiplayer queue when player's turn is next after validation
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	OcclusionsStruct.SetAllHidden();

	// todo: make this primary function
	if (TrackingClient->IsValidLowLevelFast()) {
		FMessage MessageOut;
		MessageOut.header = "stop";
		MessageOut.body   = "";
		const bool bMessageResult = TrackingClient->SendMessage(MessageOut);
		if (!bMessageResult) { printScreen("Failed to send STOP!"); }
	}

	// cleanup
	if (StopEpisodeRequest->IsValidLowLevelFast()) {
		this->RequestRemoveDelegates(StopEpisodeRequest);
		StopEpisodeRequest = nullptr;
		printScreen("Removing delegates: StopEpisodeRequest");
	} else { printScreen("Removing delegates: StopEpisodeRequest not valid!"); }
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn)
{
	/* convert to usable format */
	ExperimentInfo.StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentInfo.ExperimentNameActive    = ExperimentInfo.StartExperimentResponse.experiment_name;
	
	// printScreen(ExperimentInfo.StartExperimentResponse.experiment_name);
	
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive)) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Failed to start experiment. ExperimentNameIn not valid."));
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
	} else { UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::UpdatePredator] Received valid predator step.")); }
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector vector) {
	if (ExperimentInfo.Status != EExperimentStatus::InEpisode) { return; }
	
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
	Step.location = UExperimentUtils::VrToCanonical(vector, MapLength, this->WorldScale);
	Step.rotation = 0.0f; // todo: get actual rotation of player  
	Step.time_stamp = this->GetTimeElapsed(); 

	// const FString PrintMe = "Location: (x|y)  |" + FString::SanitizeFloat(Step.location.x) +
	// 	" | " + FString::SanitizeFloat(Step.location.y) + " | time (s): " + FString::SanitizeFloat(Step.time_stamp) + 
	// 	" | frame: " + FString::SanitizeFloat(Step.frame);
	// printScreen(PrintMe);
	/* send message to ES */

	const FMessage MessageOut = UMessageClient::NewMessage("prey_step",
		UExperimentUtils::StepToJsonString(Step));
	
	if (!TrackingClient->SendMessage(MessageOut)) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::UpdatePreyPosition] Failed: Send prey step!"))
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

	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleSubscribeToTrackingResponse] %s"), *Response)

	if (Response == "success") {
		/* reset tracking agent */
		// const bool bResetSuccess = this->ResetTrackingAgent();
		// check(bResetSuccess)
		// if (!bResetSuccess) {
		// 	UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::StartEpisode] failed to reset tracking agent."));
		// }
	}else {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleSubscribeToTrackingResponse] Response failed! (%s)"),
			*Response)
	}

	if (TrackingSubscribeRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleSubscribeToTrackingResponse] RequestRemoveDelegates(TrackingSubscribeRequest)"))
		this->RequestRemoveDelegates(TrackingSubscribeRequest);
	}
}

void AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut() {
	
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut] Timed out!"));
	
	if (TrackingSubscribeRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut] RequestRemoveDelegates(TrackingSubscribeRequest)!"));
		this->RequestRemoveDelegates(TrackingSubscribeRequest);
		TrackingSubscribeRequest = nullptr;
	}
}

bool AExperimentServiceMonitor::SubscribeToTracking() {
	if (!this->ValidateClient(TrackingClient)) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SubscribeToTracking] ClientIn not valid."))
		return false;
	}
	
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::SubscribeToTracking()] Sending Request: TrackingSubscribeRequest."));

	TrackingSubscribeRequest = TrackingClient->SendRequest("!subscribe","",10.0f); // TC is just slower...
	if (!TrackingSubscribeRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SubscribeToTracking] TrackingSubscribeRequest not valid."))
		return false;
	}

	TrackingSubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingResponse);
	TrackingSubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut);
	return true;
}

void AExperimentServiceMonitor::RequestRemoveDelegates(URequest* RequestIn) {
	bool Result = false; 
	if (RequestIn->IsValidLowLevelFast()) {
		RequestIn->ResponseReceived.RemoveAll(this);
		RequestIn->TimedOut.RemoveAll(this);
		Result = true; 
	}
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::RequestRemoveDelegates] Result: %i"), Result)
}

// ReSharper disable once CppMemberFunctionMayBeStatic
/* Called when status*/
void AExperimentServiceMonitor::OnStatusChanged(EExperimentStatus ExperimentStatusIn) {
	const FString StatusString = FExperimentInfo::GetStatusString(&ExperimentStatusIn);
	printScreen("[AExperimentServiceMonitor::OnStatusChanged] " + StatusString);
}

/* get occlusions in our specific experiment (FWorldInfo.occlusions; default: "21_05") */
bool AExperimentServiceMonitor::SendGetOcclusionsRequest() {
	if (!Client) { UE_LOG(LogExperiment, Error, TEXT("[SendGetOcclusionsRequest()] Cant send get occlusion request, Experiment service client not valid.")); return false; } // experiment service client not valid
	GetOcclusionsRequest = Client->SendRequest("get_occlusions", "21_05", TimeOut);

	if (!GetOcclusionsRequest) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::SendGetOcclusionsRequest] Failed to GetOcclusionsRequest"))
		return false;
	}
	
	GetOcclusionsRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	GetOcclusionsRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);
	return true;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionsResponse(const FString ResponseIn) 
{
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Occlusion IDs (raw): %s"), *ResponseIn);

	/* start empty */
	OcclusionIDsIntArr.Empty(); 
	TArray<int32> OcclusionIDsTemp; 

	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr; 
	const FString OcclusionIDs_temp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	const int32 Samples = OcclusionIDs_temp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Number of occlusions in configuration: %i"),Samples);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString value : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*value)) OcclusionIDsTemp.Add(FCString::Atoi(*value));
		else { SamplesLost++; }
	}

	OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionIDsTemp);
	OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp, false, true);
	
	if (!GetOcclusionsRequest->IsValidLowLevelFast()) {
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
		return;
	}

	this->RequestRemoveDelegates(GetOcclusionsRequest);
	
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("Get occlussion request timed out!"));
	if (!GetOcclusionsRequest->IsValidLowLevelFast())
	{
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
		return;
	}
	this->RequestRemoveDelegates(GetOcclusionsRequest);
}

URequest* AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()
{
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Starting request."));
	if (!this->ValidateClient(Client))
	{
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid."));
		return nullptr;
	}
	
	const FString BodyOut   = "21_05";
	const FString HeaderOut = "get_cell_locations";
	URequest* Request = Client->SendRequest("get_cell_locations", "21_05", 10.0f);
	
	if (!Request) { return nullptr; }
	
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut);

	return Request;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {

	if (!OcclusionsStruct.bAllLocationsLoaded){ OcclusionsStruct.SetAllLocations(ResponseIn); }
	if (!OcclusionsStruct.bSpawnedAll){
		constexpr int HeightScaleFactor = 3;
		OcclusionsStruct.SpawnAll(GetWorld(), true, false,
			FVector(WorldScale, WorldScale, WorldScale*HeightScaleFactor));
	}
	
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogExperiment, Log, TEXT("[SendGetOcclusionsRequest] send request false!"));
	}
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));
	return;
}

void AExperimentServiceMonitor::HandleOcclusionLocation(const FMessage MessageIn)
{
	UE_LOG(LogExperiment, Warning, TEXT("OcclusionLocation: %s"), *MessageIn.body);
}

bool AExperimentServiceMonitor::ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn) {
	if (!ClientIn->IsValidLowLevel())
	{
		printScreen("[AExperimentServiceMonitor::ConnectToServer()] ClientIn is false!"); return false;
	}
	uint8 AttemptCurr = 0;

	while (AttemptCurr < MaxAttemptsIn) {
		printScreen("[AExperimentServiceMonitor::ConnectToServer()] ConnectToServer() Attempting");
		if (ClientIn->Connect(IPAddressIn, PortIn)) {
			UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::ConnectToServer()] Success (attempt #: %i/%i)"), AttemptCurr + 1, MaxAttemptsIn);
			break;
		}
		AttemptCurr += 1;
	}

	if (!ClientIn->IsConnected()) { printScreen("[AExperimentServiceMonitor::ConnectToServer()] Failed to connect to server!"); return false; }
	
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

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->GetPlayerPawn()) { printScreen("Player Pawn NOT found!"); return false; }
	UExperimentMonitorData* NewExperimentMonitorData = NewObject<UExperimentMonitorData>(this);
	
	if (ExperimentManager) {
		PlayerIndex = ExperimentManager->RegisterNewPlayer(PlayerPawn, NewExperimentMonitorData); //todo: 
		ExperimentManager->OnEpisodeFinished(0, 30.f);
	}
	UE_LOG(LogExperiment, Warning, TEXT("Registered player: %i"), PlayerIndex)

	/* start experiment  */
	// StartExperimentRequest = this->SendStartExperimentRequest(Client, "ExperimentNameIn");
	// if (!StartExperimentRequest) { return false; }

	return true;
}

void AExperimentServiceMonitor::HandleUpdatePredator(const FMessage MessageIn) {
	// UE_LOG(LogExperiment, Warning, TEXT("Predator: %s"), *MessageIn.body);
	this->UpdatePredator(MessageIn);
}

void AExperimentServiceMonitor::OnTimerFinished() {

	printScreen("[AExperimentServiceMonitor::OnTimerFinished]");
	if(!TimerHandlePtr->IsValid()) {
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::OnTimerFinished] TimerHandlePtr not valid!"))
		return;
	}

	check(bTimerRunning) // needs true
	check(TimerHandlePtr->IsValid()) // needs true

	GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
	bTimerRunning = GetWorld()->GetTimerManager().IsTimerActive(*TimerHandlePtr);

	check(!bTimerRunning) // needs false
	check(!TimerHandlePtr->IsValid()) // needs false
	
	FrameCountPredator = 0;
	FrameCountPrey = 0;

	if (ExperimentInfo.Status == EExperimentStatus::InEpisode) {
		this->StopEpisode();
	}
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

	if(this->SpawnAndPossessPredator()) { UE_LOG(LogExperiment, Warning, TEXT("Spawned predator: OK")); }
	else{ UE_LOG(LogExperiment, Warning, TEXT("Spawned predator: FAILED")); UE_DEBUG_BREAK(); }
	ExperimentManager = NewObject<UExperimentManager>(this, UExperimentManager::StaticClass());
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

	if (bTimerRunning && this->StopTimerEpisode(*TimerHandlePtr)) {
		UE_LOG(LogExperiment, Log,
			TEXT("[AExperimentServiceMonitor::EndPlay] Stopped game during episode, timer stopped OK."))
	}

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
