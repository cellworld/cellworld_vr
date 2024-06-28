#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "PredatorController/AIControllerPredator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

	// if (MapName != "L_Maze" || !this->StartEpisode(TODO)) { printScreen("[AExperimentServiceMonitor::HandleStartExperimentResponse] Attempt to StartEpisode failed."); return false; }
	// else { printScreen("Map name and Start episode ran fine"); }
	return true;
}

/* todo: make this take input ACharacter and spawn that one*/
bool AExperimentServiceMonitor::SpawnAndPossessPredator() {

	if (!GetWorld()) { UE_LOG(LogTemp, Fatal, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] GetWorld() failed!")); return false; }

	// Define spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Specify the location and rotation for the new actor
	FVector Location(-2426.0f, 1264.0f, 90.0f); // Change to desired spawn location
	FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation

	// Spawn the character
	CharacterPredator = GetWorld()->SpawnActor<ACharacterPredator>(ACharacterPredator::StaticClass(), Location, Rotation, SpawnParams);

	// Ensure the character was spawned
	if (!CharacterPredator) { UE_LOG(LogTemp, Fatal, TEXT("[AExperimentServiceMonitor::SpawnAndPossessPredator()] Spawn ACharacterPredator Failed!")); return false; }
	return false;
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
	
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not valid.")); return false; }
	if (!Client->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not connected.")); return false; }

	/* set up world info (defaults to hexagonal and canonical) */
	FWorldInfo WorldInfo;
	WorldInfo.occlusions = "21_05";

	/* set up request body */
	FStartExperimentRequest StartExperimentRequestBody;
	StartExperimentRequestBody.duration = 30;
	StartExperimentRequestBody.subject_name = ExperimentNameIn;
	StartExperimentRequestBody.world = WorldInfo;

	const FString StartExperimentRequestBodyString = UExperimentUtils::StartExperimentRequestToJsonString(StartExperimentRequestBody);
	printScreen(StartExperimentRequestBodyString);
	StartExperimentRequest = Client->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);

	if (StartExperimentRequest != nullptr) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return false; }

	// NOTE: in v2, i would do this binding in whoever receives the request back and bind to other handlers 
	StartExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	StartExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	return true; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString& ExperimentNameIn) {
	if (!Client) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, Experiment Service client not valid.")); return false; }
	if (!Client->IsConnected()) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, Experiment Service client not connected.")); return false; }
	// if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, not an active experiment.")); return false; }
	if (ExperimentInfo.ExperimentNameActive == "") {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopExperiment] Can't stop experiment, not an active experiment."));
		return false;
	}

	FFinishExperimentRequest RequestBody;
	RequestBody.experiment_name = ExperimentNameIn;

	const FString RequestString = UExperimentUtils::FinishExperimentRequestToJsonString(RequestBody);
	StopExperimentRequest = Client->SendRequest("finish_experiment", RequestString, TimeOut);

	if (!StopExperimentRequest) { return false; }

	StopExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentResponse); // uses same one as start/stop
	StopExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentTimedOut);
	
	if (!StopExperimentRequest->IsValidLowLevelFast()) { return false; }
	return true; 
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

	ExperimentInfo.Status = EExperimentStatus::WaitingEpisode;
	printScreen("[SendStartEpisodeRequest()] WaitingEpisode!");

	return true;
}

bool AExperimentServiceMonitor::ValidateClient(UMessageClient* ClientIn)
{
	if (ClientIn->IsValidLowLevelFast() && ClientIn->IsConnected()) { return true; }
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::ValidateClient] Client not valid."));
	UE_DEBUG_BREAK();
	return false;
}

bool AExperimentServiceMonitor::ValidateExperimentName(const FString& ExperimentNameIn)
{
	if (ExperimentNameIn == "") { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode() 
{
	if (!this->ValidateClient(Client)) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, Experiment Service client not valid.")); return false; }
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive)) { UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, experiment name not valid.")); return false; }

	if (ExperimentInfo.Status != EExperimentStatus::InEpisode) {
		UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::StopEpisode() ] Can't stop episode, no active episode. ENUM"));
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
	ExperimentInfo.Status = EExperimentStatus::FinishedEpisode;
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStartEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		ExperimentInfo.Status = EExperimentStatus::FailedStartEpisode;
		UE_DEBUG_BREAK();
		return;
	}

	// remove delegates 
	if (StartEpisodeRequest->IsValidLowLevelFast())
	{
		StartEpisodeRequest->ResponseReceived.RemoveDynamic(this,&AExperimentServiceMonitor::HandleStartEpisodeRequestResponse);
		StartEpisodeRequest->TimedOut.RemoveDynamic(this,&AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut);
		printScreen("[AExperimentServiceMonitor::HandleStartEpisodeRequestResponse] Removed bindings from StartEpisodeRequest");
	}
	ExperimentInfo.Status = EExperimentStatus::InEpisode;
	this->SendGetOcclusionLocationsRequest();
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStartEpisodeRequestTimedOut()] Episode request timed out!"))
	ExperimentInfo.Status = EExperimentStatus::TimedOutEpisode;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleStopEpisodeRequestResponse(const FString response) {

	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] %s"), *response);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		UE_DEBUG_BREAK();
		ExperimentInfo.Status = EExperimentStatus::FailedFinishEpisode;
		UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] Failed to finish episode ENUM."));
		return;
	}
	
	// ExperimentInfo.Status = EExperimentStatus::FinishedEpisode; // todo: make permanent (delete below)
	// todo: temp - this will be called in multiplayer queue when player's turn is next after validation
	ExperimentInfo.Status = EExperimentStatus::WaitingEpisode;
	OcclusionsStruct.SetAllHidden();

	// cleanup 
	if (StopEpisodeRequest){
		StopEpisodeRequest->ResponseReceived.RemoveAll(this);
		StopEpisodeRequest->TimedOut.RemoveAll(this);
		StopEpisodeRequest  = nullptr;
		StartEpisodeRequest = nullptr;
		printScreen(FString("[AExperimentServiceMonitor::HandleStopEpisodeRequestResponse] Removed delegates."));
	}
	
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut() {
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::HandleStopEpisodeRequestTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn)
{
	const FString msg = "[AExperimentServiceMonitor::HandleStartExperimentResponse] " + ResponseIn;
	// printScreen(msg);
	
	/* convert to usable format */
	ExperimentInfo.StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentInfo.ExperimentNameActive    = ExperimentInfo.StartExperimentResponse.experiment_name;
	ExperimentNameActive                   = ExperimentInfo.StartExperimentResponse.experiment_name; // temp
	// printScreen(ExperimentInfo.StartExperimentResponse.experiment_name);
	
	if (!this->ValidateExperimentName(ExperimentInfo.ExperimentNameActive)) {
		UE_LOG(LogExperiment, Error,
			TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Failed to start episode. ExperimentName not valid."));
		UE_DEBUG_BREAK();
		ExperimentInfo.Status = EExperimentStatus::FailedStartExperiment;
	}
	
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Experiment name: %s"),
		*ExperimentInfo.StartExperimentResponse.experiment_name);
	
	ExperimentInfo.Status = EExperimentStatus::InExperiment;
}

void AExperimentServiceMonitor::HandleStartExperimentTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleStartExperimentTimedOut] Start Experiment TIMED OUT!");
}

/* handle tracking service message through predator route */
void AExperimentServiceMonitor::HandleTrackingServiceMessagePredator(FMessage message)
{
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, FString::Printf(TEXT("predator tracking message : %s"), *message.body));
	}
	this->UpdatePredator(message);
}

void AExperimentServiceMonitor::HandleTrackingServiceMessagePrey(FMessage message)
{
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Prey tracking message: %s"), *message.body));
	}
}

/* Handle experiment service response (during subscription) */
void AExperimentServiceMonitor::HandleExperimentServiceResponse(const FString message) {
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceResponse] ES response: %s"), *message);
	if (message != "success") {
		return; // failed 
	}
	bConnectedExperimentService = true;
}

/* Handle experiment service response timeout (during subscription) */
void AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceTimedOut] ES response timed out!"));
}

/* Handle tracking service response (during subscription) */
void AExperimentServiceMonitor::HandleTrackingServiceResponse(const FString message) {
	const FString msg = "[AExperimentServiceMonitor::HandleTrackingServiceResponse] " + message; 
	printScreen(msg);
	if (message != "success") {
		return; // failed
	}
	bConnectedTrackingService = true;
}

/* Handle tracking service response timeout (during subscription */
void AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut() {
	UE_LOG(LogExperiment, Warning, TEXT("[AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut] Timed out!"));
}

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(FMessage message)
{
	UE_LOG(LogExperiment, Error, TEXT("[AExperimentServiceMonitor::UpdatePredator] message.body:\n%s"), *message.body);

	if (Client == nullptr) { return; }
	FrameCount++;
	FStep step = UExperimentUtils::JsonStringToStep(message.body); 
	FVector new_location_ue = UExperimentUtils::CanonicalToVr(step.location,MapLength, WorldScale); // ue --> unreal engine units 
	
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("New location: %f %f %f"), new_location_ue.X, new_location_ue.Y, new_location_ue.Z));
	
	AAIControllerPredator* AIControllerPredator = Cast<AAIControllerPredator>(CharacterPredator->GetController());
	if (!AIControllerPredator) { return; }
	
	AIControllerPredator->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location_ue);
	bCanUpdatePreyPosition = true;
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector vector)
{
	if (Client == nullptr) { printScreen("Cannot send step. Client not valid."); return; }

	FLocation Location = UExperimentUtils::VrToCanonical(vector, MapLength, 4.0f);

	/* prepare Step */
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = FrameCount;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: get actual rotation of player  
	send_step.time_stamp = FrameCount; // todo: change to timer, not frame 

	/* convert FStep to JsonString */
	FString body = UExperimentUtils::StepToJsonString(send_step); 

	/* send message to ES */
	FMessage message = UMessageClient::NewMessage(header_prey_location, body);
	if (Client->IsConnected()) {
		Client->SendMessage(message);
	}

	/* don't overload server with messages before it is done processing previous prey update. */
	bCanUpdatePreyPosition = true; // todo: TRUE ONLY FOR DEBUGGING PURPOSES 
}

/* handle tracking service message coming to default "send_step" route */
void AExperimentServiceMonitor::HandleTrackingServiceMessage(FMessage message)
{
	if (GEngine) { if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("tracking message: (%s) %s"),*message.header, *message.body)); }
}

/* handles unrouted messages coming from tracking service. messages that do not have a route set up inside 
this class (ExperimentServiceMonitor) will end up here */
void AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage(FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

void HandleUnroutedExperiment(FMessage message) {
	printScreen(message.body);
	printScreen(message.header);
}

void AExperimentServiceMonitor::HandleUnroutedMessage(FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
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
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] No pawn found."));
		return false;
	}

	// Attempt to cast to APawnMain or APawnDebug and assign to PlayerPawn if successful.
	if (Cast<APawnMain>(Pawn)) {
		APawnMain* PlayerPawn = Cast<APawnMain>(Pawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
		UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnMain found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
		PlayerPawn->ResetOrigin();
	}
	else if (Cast<APawnDebug>(Pawn)) {
		APawnDebug* PlayerPawn = Cast<APawnDebug>(Pawn); // Adjust according to the actual type of PlayerPawn
		UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnDebug found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	}
	else {
		const FString ErrorMessage = "[AExperimentServiceMonitor::GetPlayerPawn()] No valid pawn found nor assigned.";
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] %s"), *ErrorMessage);
		return false;
	}

	return true;
}
//
// void RequestUnbindDelegates(URequest* RequestIn)
// {
// 	if (RequestIn)
// 	{
// 		RequestIn->ResponseReceived.RemoveAll();
// 		RequestIn
// 	}
// }

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString InErrorMessage)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *InErrorMessage));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying. Reason: %s "), *InErrorMessage);

	// this->StopEpisode();
	if (!this->IsValidLowLevelFast() && Client->IsValidLowLevel() && Client->IsConnected())
	{
		this->Disconnect(Client);
	}
}

bool AExperimentServiceMonitor::SubscribeToServer(TObjectPtr<UMessageClient> ClientIn)
{
	SubscribeRequest = ClientIn->Subscribe();
	if (!SubscribeRequest)
	{
		printScreen("[AExperimentServiceMonitor::SubscribeToServer] SubscribeRequest not valid!"); return false;
	}

	SubscribeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToServerResponse);
	SubscribeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscribeToServerTimedOut);
	return true;
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
}

void AExperimentServiceMonitor::HandleSubscribeToServerResponse(FString MessageIn)
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToServerResponse]");
	printScreen(MessageIn);
	
	this->RoutePredatorMessages();
	if (SubscribeRequest)
	{
		SubscribeRequest->ResponseReceived.RemoveAll(this);
		SubscribeRequest->TimedOut.RemoveAll(this);
	}
	
	// if(!this->RoutePredatorMessages()) { printScreen("[AExperimentServiceMonitor::Test()] Failed to route."); }
}

void AExperimentServiceMonitor::HandleSubscribeToServerTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleSubscribeToServerResponse]");
}

void AExperimentServiceMonitor::on_experiment_started()
{
	printScreen("[AExperimentServiceMonitor::on_experiment_started]");
}

void AExperimentServiceMonitor::on_experiment_finished()
{
	printScreen("[AExperimentServiceMonitor::on_experiment_finished]");
}

void AExperimentServiceMonitor::on_episode_started()
{
	printScreen("[AExperimentServiceMonitor::on_episode_started]");
}

void AExperimentServiceMonitor::on_episode_finished()
{
	printScreen("[AExperimentServiceMonitor::on_episode_finished]");
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

	UE_LOG(LogExperiment,Warning,TEXT("[AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse] %s"), *ResponseIn)
	// OcclusionsStruct.AllLocations = UExperimentUtils::OcclusionsParseAllLocations(ResponseIn);
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
	if (!ClientIn) { printScreen("[AExperimentServiceMonitor::ConnectToServer()] ClientIn is false!"); return false;  }
	uint8 AttemptCurr = 0;

	while (AttemptCurr < MaxAttemptsIn) {
		printScreen("[AExperimentServiceMonitor::ConnectToServer()] ConnectToServer() Attempting");
		if (ClientIn->Connect(IPAddressIn, PortIn)) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::ConnectToServer()] Connecting Tracking Service SUCCESS. (attempt #: %i/%i)"), AttemptCurr + 1, MaxAttemptsIn);
			break;
		}
		AttemptCurr += 1;
	}

	if (!ClientIn->IsConnected()) { printScreen("[AExperimentServiceMonitor::ConnectToServer()] Failed to connect to server!"); return false; }

	return true;
}

bool AExperimentServiceMonitor::RoutePredatorMessages()
{
	if (!Client) {printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] Client not valid."); return false;}
	
	Client->UnroutedMessageEvent.AddDynamic(this,&AExperimentServiceMonitor::HandleUnroutedMessage);
	RoutePredator = Client->AddRoute("predator_step");
	RouteOnEpisodeStarted = Client->AddRoute(on_episode_started_header); // todo: make sure I can delete 

	if (!RoutePredator) {printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] Route not valid."); return false; }
	// RoutePredator->MessageReceived.AddDynamic(this,&AExperimentServiceMonitor::HandleUpdatePredator);
	RouteOnEpisodeStarted->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleUpdatePredator);

	printScreen("[AExperimentServiceMonitor::RoutePredatorMessages()] OK");
	return true;
}

bool AExperimentServiceMonitor::Test() {

	/* connect tracking service */

	Client = AExperimentServiceMonitor::CreateNewClient();
	constexpr int AttemptsMax = 5; 
	if (!this->ConnectToServer(Client, AttemptsMax, ServerIPMessage, ServerPort)) { printScreen("ConnectToServer() failed."); return false; }

	if (!this->SubscribeToServer(Client)) { printScreen("[AExperimentServiceMonitor::Test] Sending Subscribe request: OK."); }
	else {printScreen("[AExperimentServiceMonitor::Test] Sending subscribe request: Failed."); }

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->GetPlayerPawn()) { printScreen("Player Pawn NOT found!"); return false; }
	
	/* start experiment  */
	StartExperimentRequest = this->SendStartExperimentRequest(Client, "ExperimentNameIn");
	if (!StartExperimentRequest) { return false; }

	return true;
}


void AExperimentServiceMonitor::HandleUpdatePredator(FMessage MessageIn)
{
	printScreen("[AExperimentServiceMonitor::HandleUpdatePredator]" + MessageIn.body);
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();
	printScreen("[AExperimentServiceMonitor::BeginPlay()] BeginPlay()");
	Test(); 
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPLayReason)
{
	Super::EndPlay(EndPLayReason);
	if (StartExperimentRequest) { this->RequestRemoveDelegates(StartExperimentRequest); }
	else{printScreen("StartExperimentRequest not valid");}
	if (StartEpisodeRequest) { this->RequestRemoveDelegates(StartEpisodeRequest); }
	else{printScreen("StartEpisodeRequest not valid");}
	
	// if (/*Client->IsValidLowLevelFast() && */IsValid(Client) && Client->IsConnected())
	// {
	// 	if (Client->Disconnect()) { printScreen("Disconnect success."); }
	// 	else{printScreen("Failed to disconnect.");}
	// }
	
	// if (StartEpisodeRequest->IsValidLowLevelFast()){
	// 	StartEpisodeRequest->ResponseReceived.RemoveDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	// 	StartEpisodeRequest->TimedOut.RemoveDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);
	// }
	// if (StopExperimentRequest->IsValidLowLevelFast()){
	// 	StopExperimentRequest->ResponseReceived.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentResponse);
	// 	StopExperimentRequest->TimedOut.RemoveDynamic(this, &AExperimentServiceMonitor::HandleStopExperimentTimedOut);
	// }

}
