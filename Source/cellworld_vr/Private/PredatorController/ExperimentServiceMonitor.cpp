#include "PredatorController/ExperimentServiceMonitor.h"

#include "ParticleHelper.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "PredatorController/AIControllerPredator.h"
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
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s"), *InMessage));
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

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] GetWorld() failed!")); return false;
	}

	// Define spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Specify the location and rotation for the new actor
	FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation
	FLocation SpawnLocation;
	SpawnLocation.x = 0.5;
	SpawnLocation.y = 0.5;
	FVector SpawnVector = UExperimentUtils::CanonicalToVr(SpawnLocation,MapLength,WorldScale);
	// Spawn the character
	PredatorBasic = GetWorld()->SpawnActor<APredatorBasic>(APredatorBasic::StaticClass(), SpawnVector, Rotation, SpawnParams);

	// Ensure the character was spawned
	if (!PredatorBasic)
	{
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] Spawn APredatorBasic Failed!"));
		return false;
	}
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

	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SendStartExperimentRequest()] Sending: StartExperimentRequest."));
	URequest* Request = ClientIn->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);
	
	if (!Request) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return nullptr; }
	
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::SendStartExperimentRequest()] Bound to delegates: StartExperimentRequest."));
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	
	ExperimentInfo.Status = EExperimentStatus::WaitingExperiment;
	ExperimentInfo.StartExperimentRequestBody = StartExperimentRequestBody;
	return Request;
}

/* start experiment service episode stream
 * should ONLY be called from game mode by the BP_door trigger !!!  */
bool AExperimentServiceMonitor::StartEpisode(UMessageClient* ClientIn, const FString& ExperimentNameIn) {
	if (!this->ValidateClient(ClientIn)) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, client not valid.")); return false; }
	if (!this->ValidateExperimentName(ExperimentNameIn)) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, experiment name not valid.")); return false; }
	if (ExperimentInfo.Status != EExperimentStatus::WaitingEpisode
		&& ExperimentInfo.Status != EExperimentStatus::InExperiment)
	{
		UE_LOG(LogExperiment, Log, TEXT("Can't start episode, no active experiment ENUM."));
		return false;
	}
	FStartEpisodeRequest RequestBody;
	RequestBody.experiment_name = ExperimentInfo.ExperimentNameActive;
	
	const FString RequestString = UExperimentUtils::StartEpisodeRequestToJsonString(RequestBody);
	StartEpisodeRequest = ClientIn->SendRequest("start_episode", RequestString, TimeOut);
	
	if (!StartEpisodeRequest) { return false; }

	StartEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestResponse);
	StartEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut);

	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	printScreen("[SendStartEpisodeRequest()] WaitingEpisode!");

	return true;
}

bool AExperimentServiceMonitor::ValidateClient(UMessageClient* ClientIn)
{
	if (!ClientIn->IsValidLowLevelFast())
	{
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
bool AExperimentServiceMonitor::StopEpisode() 
{
	if (bTimerRunning)
	{
		printScreen("Calling OnTimerFinished()!");
		this->OnTimerFinished();
		ExperimentInfo.SetStatus(EExperimentStatus::FailedEpisodeTimer);
	}
	
	if (!this->ValidateClient(Client))
	{
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopEpisode()] Can't stop episode, Experiment Service client not valid."));
		UE_DEBUG_BREAK();
		return false;
	}
	
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive))
	{
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, experiment name not valid."));
		return false;
	}
	
	if (this->ExperimentInfo.Status != EExperimentStatus::InEpisode) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, not in episode."));
		return false;
	}
	
	FFinishEpisodeRequest RequestBody;
	RequestBody.experiment_name = ExperimentInfo.ExperimentNameActive;

	const FString RequestString = UExperimentUtils::FinishEpisodeRequestToJsonString(RequestBody);
	StopEpisodeRequest = Client->SendRequest("finish_episode", RequestString, 5.0f);
	if (!StopEpisodeRequest->IsValidLowLevelFast())
	{
		printScreen("[AExperimentServiceMonitor::StopEpisode] StopEpisodeRequest not valid. Failed to Bind Responses.");
		return false;
	}
	
	StopEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestResponse);
	StopEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut);
	ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode);

	if (!this->RemoveDelegatesPredatorRoute())
	{
		printScreen("Failed to Remove delegates from MessageRoutePredator!");
	}
	
	printScreen("[AExperimentServiceMonitor::StopEpisode()] OK");
	return true;
}

bool AExperimentServiceMonitor::StartTimerEpisode(const float DurationIn, FTimerHandle TimerHandleIn)
{
	if (!TimerHandleIn.IsValid())
	{
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::StartTimerEpisode] Failed, TimerHandleIn not valid."));
		return false;
	}
	
	GetWorldTimerManager().SetTimer(TimerHandleIn, this, &AExperimentServiceMonitor::OnTimerFinished,
		DurationIn, false, -1.0f);
	bTimerRunning = true;
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStartEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));

	// remove delegates
	if (StartEpisodeRequest->IsValidLowLevelFast())
	{
		/*StartEpisodeRequest->ResponseReceived.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestResponse);
		StartEpisodeRequest->TimedOut.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut);*/
		this->RequestRemoveDelegates(StartEpisodeRequest);
		printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Removed bindings from StartEpisodeRequest");
	}
	
	if (response == "fail") {
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorStartEpisode);
		return;
	}

	ExperimentInfo.SetStatus(EExperimentStatus::InEpisode);

	// printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Calling routes and sub.");
	// if (this->SubscribeToTracking())
	// {
	// 	printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Send Subscribe to TRACKING: OK.");
	// 	if (!this->RoutePredatorMessages())
	// 	{
	// 		printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Failed.");
	// 	}
	// }else { printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Send Subscribe to TRACKING: Failed."); }

	if (!this->ResetTrackingAgent())
	{
		printScreen("Failed to reset agent!");
	}
	// todo: set back up 
	// if (!this->RemoveDelegatesPredatorRoute() || !this->RoutePredatorMessages()){
	// 	printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Failed.");
	// }

	this->SendGetOcclusionLocationsRequest();
}

bool AExperimentServiceMonitor::ResetTrackingAgent()
{
	printScreen("[AExperimentServiceMonitor::ResetTrackingAgent()]");
	if (!this->ValidateClient(TrackingClient))
	{
		printScreen("[AExperimentServiceMonitor::ResetTrackingAgent()] TrackingClient not valid.");
		return false;
	}
	
	const FMessage MessageReset = UMessageClient::NewMessage("reset",
		"");
	
	return 	TrackingClient->SendMessage(MessageReset);
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut()] Episode request timed out!"))
	ExperimentInfo.SetStatus(EExperimentStatus::ErrorTimedOutEpisode);
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStopEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] %s"), *response);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		UE_DEBUG_BREAK();
		ExperimentInfo.SetStatus(EExperimentStatus::ErrorFinishEpisode);
		UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] Failed to finish episode ENUM."));
		return;
	}
	
	ExperimentInfo.SetStatus(EExperimentStatus::FinishedEpisode); // todo: make permanent (delete below)
	// todo: temp - this will be called in multiplayer queue when player's turn is next after validation
	ExperimentInfo.SetStatus(EExperimentStatus::WaitingEpisode);
	OcclusionsStruct.SetAllHidden();

	// cleanup
	if (StopEpisodeRequest)
	{
		this->RequestRemoveDelegates(StopEpisodeRequest);
		StopEpisodeRequest = nullptr;
		printScreen("Removing delegates: StopEpisodeRequest");
	}else{printScreen("Removing delegates: StopEpisodeRequest not valid!");}

	
	// if (StopEpisodeRequest){
	// 	StopEpisodeRequest->ResponseReceived.RemoveAll(this);
	// 	StopEpisodeRequest->TimedOut.RemoveAll(this);
	// 	// StopEpisodeRequest  = nullptr;
	// 	// StartEpisodeRequest = nullptr;
	// 	printScreen(FString("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] Removed delegates."));
	// }
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
	ExperimentNameActive                   = ExperimentInfo.StartExperimentResponse.experiment_name; // temp
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

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(FMessage message)
{
	UE_LOG(LogExperiment, Log, TEXT("[AExperimentServiceMonitor::UpdatePredator] message.body:\n%s"), *message.body);
	if (!this->ValidateClient(TrackingClient))
	{
		UE_DEBUG_BREAK();
		return;
	}
	const FStep StepOut = UExperimentUtils::JsonStringToStep(message.body); 
	// const FVector LocationUEOut = UExperimentUtils::CanonicalToVr(StepOut.location,MapLength, WorldScale); // ue --> unreal engine units 
	
	PredatorBasic->SetActorLocation(UExperimentUtils::CanonicalToVr(StepOut.location, MapLength, WorldScale));
	PredatorBasic->SetActorRotation(FRotator(0.0,StepOut.rotation,0.0f));
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector vector)
{
	if (!this->ValidateClient(TrackingClient))
	{
		printScreen("[AExperimentServiceMonitor::UpdatePreyPosition] Cannot send step. Client not valid.");
		// UE_DEBUG_BREAK();
		return;
	}
	
	FrameCountPrey += 1; 
	FLocation Location = UExperimentUtils::VrToCanonical(vector, MapLength, 4.0f);

	/* prepare Step */
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = FrameCountPrey;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: get actual rotation of player  
	send_step.time_stamp = FrameCountPrey;
	// todo: change to timer, not frame 
	// ExperimentInfo.StartExperimentRequestBody.duration - GetWorldTimerManager().GetTimerRemaining(TimerHandle); 
	
	/* send message to ES */
	const FMessage MessageOut = UMessageClient::NewMessage(ExperimentHeaders.PreyStep,
		UExperimentUtils::StepToJsonString(send_step));
	
	if (!TrackingClient->SendMessage(MessageOut)){
		printScreen("[AExperimentServiceMonitor::UpdatePreyPosition] Send prey step: FAILED");
	} else {
		UE_LOG(LogExperiment,Log,
			TEXT("[AExperimentServiceMonitor::UpdatePreyPosition] Send prey step: OK  (%s)"), *MessageOut.header);
	}
}

void HandleUnroutedExperiment(FMessage message) {
	printScreen(message.body);
	printScreen(message.header);
}

void AExperimentServiceMonitor::HandleUnroutedMessage(FMessage message)
{
	printScreen("[AExperimentServiceMonitor::HandleUnroutedMessage] " 
		+ message.header + " | " +  message.body);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

/* todo: finish this */
bool AExperimentServiceMonitor::IsExperimentActive(const FString ExperimentNameIn)
{
	return false;
}

void AExperimentServiceMonitor::AttachAgent(TObjectPtr<APawn> PawnIn)
{
	printScreen("[AExperimentServiceMonitor::AttachAgent]");
}

/* gets player pawn from world */
bool AExperimentServiceMonitor::GetPlayerPawn()
{
	if (!GetWorld()) { return false; }

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] No pawn found."));
		return false;
	}

	// Attempt to cast to APawnMain or APawnDebug and assign to PlayerPawn if successful.
	if (Cast<APawnMain>(Pawn)) {
		APawnMain* PlayerPawn = Cast<APawnMain>(Pawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
		UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnMain found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	}else {
		const FString ErrorMessage = "[AExperimentServiceMonitor::GetPlayerPawn()] No valid pawn found nor assigned.";
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
		return false;
	}

	return true;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString InErrorMessage)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *InErrorMessage));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying. Reason: %s "), *InErrorMessage);

	if (!this->IsValidLowLevelFast() && Client->IsValidLowLevel() && Client->IsConnected())
	{
		this->Disconnect(Client);
	}
}

void AExperimentServiceMonitor::HandleSubscribeToTrackingResponse(FString Response)
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToTrackingResponse] " + Response);
	
	// if (TrackingSubscribeRequest->IsValidLowLevelFast())
	// {
	// 	TrackingSubscribeRequest->ResponseReceived.RemoveAll(this);
	// 	TrackingSubscribeRequest->TimedOut.RemoveAll(this);
	// 	TrackingSubscribeRequest = nullptr;
	// }
}

void AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut] Timed out!" );
}

bool AExperimentServiceMonitor::SubscribeToTracking()
{
	if (!this->ValidateClient(TrackingClient))
	{
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SubscribeToTracking] ClientIn not valid."))
		printScreen("[AExperimentServiceMonitor::SubscribeToTracking] ClientIn not valid!");
		return false;
	}
	
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::SubscribeToTracking()] Sending Request: TrackingSubscribeRequest."));
	TrackingSubscribeRequest = TrackingClient->SendRequest("!subscribe","",-5.0f);
	if (!TrackingSubscribeRequest->IsValidLowLevelFast())
	{
		// UE_DEBUG_BREAK();
		printScreen("[AExperimentServiceMonitor::SubscribeToTracking] SubscribeRequest not valid!");
		UE_LOG(LogExperiment, Fatal, TEXT("[AExperimentServiceMonitor::SubscribeToTracking] TrackingSubscribeRequest not valid."))
		return false;
	}

	/* moved to BeginPlay() for testing */
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::SubscribeToTracking()] Bound delegates to TrackingSubscribeRequest."));
	TrackingSubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingResponse);
	TrackingSubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToTrackingTimedOut);
	return true;
}

/* deprecated todo: delete*/
bool AExperimentServiceMonitor::SubscribeToServer(UMessageClient* ClientIn)
{
	// printScreen("[AExperimentServiceMonitor::SubscribeToServer] INSIDE!");
	// if (!this->ValidateClient(ClientIn))
	// {
	// 	printScreen("[AExperimentServiceMonitor::SubscribeToServer] ClientIn not valid!");
	// 	return false;
	// }
	//
	// SubscribeRequest = ClientIn->Subscribe();
	// if (!SubscribeRequest->IsValidLowLevelFast())
	// {
	// 	printScreen("[AExperimentServiceMonitor::SubscribeToServer] SubscribeRequest not valid!");
	// 	return false;
	// }
	//
	// SubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToServerResponse);
	// SubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToServerTimedOut);
	// printScreen("[AExperimentServiceMonitor::SubscribeToServer] SubscribeRequest bound to delegates!");
	// return true;
	printScreen("[AExperimentServiceMonitor::SubscribeToServer] DONT USE ME!");
	return false;
}

void AExperimentServiceMonitor::RequestRemoveDelegates(URequest* RequestIn)
{
	if (RequestIn->IsValidLowLevelFast())
	{
		RequestIn->ResponseReceived.RemoveAll(this);
		RequestIn->TimedOut.RemoveAll(this);
		printScreen("[AExperimentServiceMonitor::RequestRemoveDelegates] OK.");
		return;
	}
	printScreen("[AExperimentServiceMonitor::RequestRemoveDelegates] Failed.");
	printScreen("[AExperimentServiceMonitor::RequestRemoveDelegates] TURNED OFF!");
}

void AExperimentServiceMonitor::HandleSubscribeToServerResponse(FString MessageIn)
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToServerResponse] " + MessageIn);
	
	// if (SubscribeRequest->IsValidLowLevelFast())
	// {
	// 	SubscribeRequest->ResponseReceived.RemoveAll(this);
	// 	SubscribeRequest->TimedOut.RemoveAll(this);
	// 	SubscribeRequest = nullptr;
	// }
	// this->RoutePredatorMessages();
}

void AExperimentServiceMonitor::HandleSubscribeToServerTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToServerResponse] TimedOut subscription!");
}

void AExperimentServiceMonitor::OnStatusChanged(const EExperimentStatus ExperimentStatusIn)
{
	printScreen("[AExperimentServiceMonitor::OnStatusChanged]");
	// OnExperimentStatusChangedEvent.Broadcast(ExperimentStatusIn);
	// ExperimentInfo.OnExperimentStatusChangedEvent.RemoveDynamic(this, &AExperimentServiceMonitor::OnStatusChanged);
}

/* get occlusions in our specific experiment (FWorldInfo.occlusions; default: "21_05") */
bool AExperimentServiceMonitor::SendGetOcclusionsRequest()
{
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Cant send get occlusion request, Experiment service client not valid.")); return false; } // experiment service client not valid
	GetOcclusionsRequest = Client->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!GetOcclusionsRequest) { return false; } // failed to send request
	GetOcclusionsRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	GetOcclusionsRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);
	
	return true;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionsResponse(const FString ResponseIn) 
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Occlusion IDs (raw): %s"), *ResponseIn);
	
	/* start empty */
	OcclusionIDsIntArr.Empty(); 
	TArray<int32> OcclusionIDsTemp; 

	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr; 
	const FString OcclusionIDs_temp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	const int32 Samples = OcclusionIDs_temp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Number of occlusions in configuration: %i"),Samples);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString value : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*value)) OcclusionIDsTemp.Add(FCString::Atoi(*value));
		else SamplesLost++;
	}

	// UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(UGameplayStatics::GetGameInstance(GetWorld()));
	// if (GameInstance->IsValidLowLevelFast())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Set ExperimentService->WorldScale"));
	// 	this->WorldScale = GameInstance->WorldScale;
	// }
	
	OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionIDsTemp);
	OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp, false, true);
	if (!GetOcclusionsRequest->IsValidLowLevelFast())
	{
		UE_LOG(LogExperiment,Error,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Failed to remove delegates, GetOcclusionsRequest not valid."));
		return;
	}

	GetWorldTimerManager().SetTimer(TimerHandle, this, &AExperimentServiceMonitor::OnTimerFinished,
	float(ExperimentInfo.StartExperimentResponse.duration), false, -1.0f);
	bTimerRunning = true;

	if (!this->ResetTrackingAgent())
	{
		printScreen("COULDNT RESET!");
	}

	this->RequestRemoveDelegates(GetOcclusionsRequest);
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("Get occlussion request timed out!"));
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

	if (ExperimentInfo.Status != EExperimentStatus::InEpisode)
	{
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, not in episode."));
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

	// UE_LOG(LogExperiment,Warning,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] %s"), *ResponseIn)
	if (!OcclusionsStruct.bAllLocationsLoaded){ OcclusionsStruct.SetAllLocations(ResponseIn); }
	if (!OcclusionsStruct.bSpawnedAll){
		OcclusionsStruct.SpawnAll(GetWorld(), true, false,
			FVector(4.0, 4.0, 10.0f));
	}
	 
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogTemp, Log, TEXT("[SendGetOcclusionsRequest] send request false!"));
	}
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));
	return;
}

void AExperimentServiceMonitor::HandleOcclusionLocation(const FMessage MessageIn)
{
	UE_LOG(LogTemp, Warning, TEXT("OcclusionLocation: %s"), *MessageIn.body);
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

bool AExperimentServiceMonitor::RoutePredatorMessages()
{
	if (!this->ValidateClient(TrackingClient))
	{
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

bool AExperimentServiceMonitor::RemoveDelegatesPredatorRoute()
{
	if (!MessageRoutePredator->IsValidLowLevelFast() || !TrackingClient->IsValidLowLevelFast())
	{
		printScreen("[AExperimentServiceMonitor::RemoveDelegatesPredatorRoute] MessageRoutePredator or TC not valid!");
		return false;
	}

	MessageRoutePredator->MessageReceived.RemoveDynamic(this, &AExperimentServiceMonitor::HandleUpdatePredator);
	TrackingClient->UnroutedMessageEvent.RemoveDynamic(this, &AExperimentServiceMonitor::HandleUnroutedMessage);
	printScreen("[AExperimentServiceMonitor::RemoveDelegatesPredatorRoute] MessageRoutePredator: Delegates Removed!");
	return true;
}

bool AExperimentServiceMonitor::Test() {

	Client = AExperimentServiceMonitor::CreateNewClient();
	TrackingClient = AExperimentServiceMonitor::CreateNewClient();

	/* connect tracking service */
	constexpr int AttemptsMax = 5;
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::Test()] Attempting to connect to Experiment Service Client."));
	if (!this->ConnectToServer(Client, AttemptsMax, ServerIPMessage, ServerPort))
	{
		printScreen("[AExperimentServiceMonitor::Test] Connect SERVER failed."); return false;
	}

	/* connect to agent tracking */
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::Test()] Attempting to connect to Agent Tracking Client."));
	if (!this->ConnectToServer(TrackingClient, AttemptsMax, ServerIPMessage, TrackingPort))
	{
		printScreen("Connect to Tracking: failed."); return false;
	}

	/* subscribe Agent tracking */
	if (this->SubscribeToTracking())
	{
		printScreen("[AExperimentServiceMonitor::Test] Send Subscribe to TRACKING: OK.");
		this->RoutePredatorMessages();
	}else { printScreen("[AExperimentServiceMonitor::Test] Send Subscribe to TRACKING: Failed."); }

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->GetPlayerPawn()) { printScreen("Player Pawn NOT found!"); return false; }
	
	/* start experiment  */
	StartExperimentRequest = this->SendStartExperimentRequest(Client, "ExperimentNameIn");
	if (!StartExperimentRequest) { return false; }

	return true;
}

void AExperimentServiceMonitor::HandleUpdatePredator(FMessage MessageIn)
{
	this->UpdatePredator(MessageIn);
	FrameCountPredator++;
}

void AExperimentServiceMonitor::OnTimerFinished()
{
	const float duration = (float)ExperimentInfo.StartExperimentResponse.duration;
	printScreen("pred fps: " + LexToString(FrameCountPredator/duration) + " | "
		+ FString::FromInt(FrameCountPredator) + "/" + LexToString(duration) );
	
	printScreen("prey fps: " + LexToString(FrameCountPrey/duration) + " | "
		+ FString::FromInt(FrameCountPrey) + "/" + LexToString(duration));
	
	// FrameCountPredator = 0;
	// FrameCountPrey = 0;
	printScreen("TIMER FINISHED!");
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	bTimerRunning = false;
	if (ExperimentInfo.Status == EExperimentStatus::InEpisode)
	{
		this->StopEpisode();
		ExperimentInfo.SetStatus(EExperimentStatus::FailedEpisodeTimer);
	}
}

float AExperimentServiceMonitor::GetTimeRemaining()
{
	if (!TimerHandle.IsValid() || !bTimerRunning) { return 0.0f; }
	
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle);	
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();
	
	ExperimentInfo.OnExperimentStatusChangedEvent.AddDynamic(this, &AExperimentServiceMonitor::OnStatusChanged);
	
	printScreen("[AExperimentServiceMonitor::BeginPlay()] BeginPlay()");
	if(this->SpawnAndPossessPredator()) { UE_LOG(LogExperiment, Warning, TEXT("Spawned predator: OK")); }
	else{ UE_LOG(LogExperiment, Warning, TEXT("Spawned predator: FAILED")); UE_DEBUG_BREAK(); }
	Test();
	// StopWatch.Start();
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green,
	// 		FString::Printf(
	// 			TEXT("[AGameModeMain::Tick()] FPS v2: %0.05f"), frames_predator_in/(TimeElapsedTick)));
		// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green,
		// 	FString::Printf(TEXT("[AGameModeMain::Tick()] tracking valid: %i"), SubscribeRequest->IsValidLowLevelFast()));
	// UE_LOG(LogExperiment, Warning, TEXT("%s"), *InMessage);
	// UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::Tick()] tracking valid: %i"), SubscribeRequest->IsValidLowLevelFast());
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPLayReason)
{
	Super::EndPlay(EndPLayReason);

	if (bTimerRunning){ this->OnTimerFinished(); }
	
	/* clear delegates */
	UE_LOG(LogExperiment,Log,TEXT("[AExperimentServiceMonitor::EndPlay] Removing delegates!"))
	if (StartExperimentRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(StartExperimentRequest); }
	else{printScreen("StartExperimentRequest not valid");}
	
	if (StartEpisodeRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(StartEpisodeRequest); }
	else{printScreen("StartEpisodeRequest not valid");}
	
	if (TrackingSubscribeRequest->IsValidLowLevelFast()) { this->RequestRemoveDelegates(TrackingSubscribeRequest); }
	else{printScreen("TrackingSubscribeRequest not valid");}

	if (MessageRoutePredator->IsValidLowLevelFast())
	{
		MessageRoutePredator->MessageReceived.RemoveAll(this);
		printScreen("MessageRoutePredator: Removed delegates.");
	}else{printScreen("MessageRoutePredator: not valid, could not remove delegates.");}

	if (this->RemoveDelegatesPredatorRoute()) { printScreen("MessageRoutePredator: Removed delegates.");
	}else{printScreen("MessageRoutePredator: not valid, could not remove delegates.");}
	
	// TrackingClient->Disconnect();
	// Client->Disconnect();

	// if (/*Client->IsValidLowLevelFast() && */IsValid(Client) && Client->IsConnected())
	// {
	// 	if (Client->Disconnect()) { printScreen("Disconnect success."); }
	// 	else{printScreen("Failed to disconnect.");}
	// }

}
