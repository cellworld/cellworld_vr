#include "PredatorController/ExperimentServiceMonitor.h"
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
	UE_LOG(LogTemp, Warning, TEXT("%s"), *InMessage);
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
bool AExperimentServiceMonitor::StopConnection(UMessageClient* ClientIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::StopConnection] Disconecting."));
	return ClientIn->Disconnect();
}

bool AExperimentServiceMonitor::DisconnectAll()
{
	if (bInEpisode) {
		if (!this->StopEpisode()) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Stopped active episode."));
			return false;
		}
	}else { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] No active episode.")); }

	if (!this->StopExperiment(ExperimentNameActive)) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Stopped active experiment."));
		return false;
	}else{ UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] No active experiment.")); }

	return true;
}

/* sends request to experiment service to start experiment */
bool AExperimentServiceMonitor::StartExperiment(const FString ExperimentNameIn) {

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

	StartExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	StartExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	return true; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString ExperimentNameIn) {
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not valid.")); return false; }
	if (!Client->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not connected.")); return false; }
	//if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, not an active experiment.")); return false; }
	
	stop_experiment_request = this->SendFinishExperimentRequest(ExperimentNameActive); // returns true; handle if false
	stop_experiment_request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentResponse);
	stop_experiment_request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentTimedOut);
	if (!stop_experiment_request) { return false; }
	return true; 
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
	printScreen(StartExperimentRequestBodyString);
	URequest* Request = ClientIn->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);

	if (!Request) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::StartExperiment()] StartExperimentRequest is NULL.")); return nullptr; }

	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);

	return Request;
}

URequest* AExperimentServiceMonitor::SendFinishExperimentRequest(const FString ExperimentNameIn)
{
	if (!Client) { return nullptr; }
	FFinishExperimentRequest request_body;
	request_body.experiment_name = ExperimentNameIn;

	FString request_string = UExperimentUtils::FinishExperimentRequestToJsonString(request_body);
	URequest* request = Client->SendRequest("finish_experiment", request_string, 5.0f);

	if (!request) { return nullptr; }

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentResponse); // uses same one as start/stop
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentTimedOut);

	return request;
}

bool AExperimentServiceMonitor::DisconnectClients() {
	/* stop connections if clients exist */
	if (!Client || !Client->IsConnected()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service."));
		return false;
	}

	if (!Client->Disconnect()) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service.")); return false; }
	return true;
}

void AExperimentServiceMonitor::HandleFinishExperimentResponse(const FString ResponseIn)
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
	bool bStopAllConnections = true; 

	if (!bStopAllConnections) { return; }
	
	if (this->DisconnectClients()) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] Disconnected clients successfully.")); return; }
	else{ UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentResponse] Disconnect clients failed."));}
}

void AExperimentServiceMonitor::HandleFinishExperimentTimedOut()
{
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::HandleFinishExperimentTimedOut()] Finish experiment request timed out!"))
}

/* start experiment service episode stream */
bool AExperimentServiceMonitor::StartEpisode(UMessageClient* ClientIn) {
	if (!ClientIn) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, client not valid.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, no active experiment.")); return false; }
	
	// send request 
	printScreen("[StartEpisode()] Sending request!");
	if (!this->SendStartEpisodeRequest(ClientIn, ExperimentNameActive)) { return false; }
	
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode() 
{
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't stop episode, Experiment Service client not valid.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Warning, TEXT("Can't stop episode, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("Can't stop episode, no active experiment.")); return false; }
	//if (!bInEpisode) { UE_LOG(LogTemp,Warning,TEXT("Can't stop episode, no active episode."));  return false; }

	FFinishEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameActive;

	FString request_string = UExperimentUtils::FinishEpisodeRequestToJsonString(request_body);
	URequest* request = Client->SendRequest("finish_episode", request_string, 5.0f);

	if (!request) { return false; }
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);

	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleEpisodeRequestResponse(const FString response) {

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::EpisodeResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		bInEpisode = false;
		UE_DEBUG_BREAK();
		return;
	}

	bInEpisode = true; // todo: fix, HandleEpisodeRequest both starts and stops, need unique functions
	printScreen(FString("Exiting HandleStartEpisodeRequest"));
	this->SendGetOcclusionLocationsRequest();
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::EpisodeTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn)
{
	const FString msg = "[AExperimentServiceMonitor::HandleStartExperimentResponse] " + ResponseIn;
	printScreen(msg);
	
	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;
	
	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Experiment name: %s"), *ExperimentNameActive);
	
	bInExperiment = true;
	this->SendStartEpisodeRequest(Client, ExperimentNameActive);
}

void AExperimentServiceMonitor::HandleStartExperimentTimedOut()
{
	printScreen("[AExperimentServiceMonitor::HandleStartExperimentTimedOut] Start Experiment TIMED OUT!");
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStartEpisodeRequest(UMessageClient* ClientIn,const FString ExperimentNameIn)
{
	FStartEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameIn;
	
	FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	printScreen("[SendStartEpisodeRequest()] Sending request!");
	StartEpisodeRequest = ClientIn->SendRequest("start_episode", request_string, 5.0f);
	
	if (!StartEpisodeRequest) { return nullptr; }

	StartEpisodeRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	StartEpisodeRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);
	
	return StartEpisodeRequest;
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStopEpisodeRequest(const FString ExperimentNameIn, const FString header)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::SendStopEpisodeRequest] ExperimentNameIn: %s"), *ExperimentNameIn);
	FFinishEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameIn;

	FString request_string = UExperimentUtils::FinishEpisodeRequestToJsonString(request_body);
	URequest* request = Client->SendRequest(header, request_string, 5.0f);

	if (!request) { return nullptr; }
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);

	return request;
}

/* handle experiment service message received */
void AExperimentServiceMonitor::HandleExperimentServiceMessage(FMessage message) {
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, FString::Printf(TEXT("experiment message : %s"), *message.body));
	}
}

/* handle experiment service message timeout */
void AExperimentServiceMonitor::HandleExperimentServiceMessageTimedOut(FMessage message) {
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("experiment timeout message : %s"), *message.body));
	}
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
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceResponse] ES response: %s"), *message);
	if (message != "success") {
		return; // failed 
	}
	bConnectedExperimentService = true;
}

/* Handle experiment service response timeout (during subscription) */
void AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceTimedOut] ES response timed out!"));
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
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut] Timed out!"));
}

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(const FMessage message)
{
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::UpdatePredator] message.body:\n%s"), *message.body);

	if (Client == nullptr) { return; }
	frame_count++;
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

	FLocation Location = UExperimentUtils::VrToCanonical(vector, MapLength, WorldScale);

	/* prepare Step */
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = frame_count;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: get actual rotation of player  
	send_step.time_stamp = frame_count; // todo: change to timer, not frame 

	/* convert FStep to JsonString */
	FString body = UExperimentUtils::StepToJsonString(send_step); 

	/* send message to ES */
	FMessage message = UMessageClient::NewMessage(header_prey_location, body);
	UE_LOG(LogTemp, Log, TEXT("sending prey step: (route: %s; body: %s"),*header_prey_location,*message.body);
	if (Client->IsConnected()) {
		Client->SendMessage(message);
	}

	/* don't overload server with messages before it is done processing previous prey update. */
	bCanUpdatePreyPosition = true; // todo: TRUE ONLY FOR DEBUGGING PURPOSES 
}

/* handle tracking service message coming to default "send_step" route */
void AExperimentServiceMonitor::HandleTrackingServiceMessage(const FMessage message)
{
	if (GEngine) { if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("tracking message: (%s) %s"),*message.header, *message.body)); }
}

/* handles unrouted messages coming from tracking service. messages that do not have a route set up inside 
this class (ExperimentServiceMonitor) will end up here */
void AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage(const FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

void HandleUnroutedExperiment(const FMessage message) {
	printScreen(message.body);
	printScreen(message.header);
}

void AExperimentServiceMonitor::HandleExperimentServiceUnroutedMessage(const FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

/* todo: finish this */
bool AExperimentServiceMonitor::IsExperimentActive(const FString ExperimentNameIn)
{
	//UExperimentUtils::Start
	return false;
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

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString InErrorMessage)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *InErrorMessage));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying. Reason: %s "), *InErrorMessage);

	this->StopEpisode();
	if (!this->IsValidLowLevelFast())
	{
		this->DisconnectAll();
	}
}

/* get occlusions in our specific experiment (FWorldInfo.occlusions; default: "21_05") */
bool AExperimentServiceMonitor::SendGetOcclusionsRequest()
{
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Cant send get occlusion request, Experiment service client not valid.")); return false; } // experiment service client not valid
	URequest* Request = Client->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!Request) { return false; } // failed to send request
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);
	
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
	FString OcclusionIDs_temp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	int32 Samples = OcclusionIDs_temp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Number of occlusions in configuration: %i"),Samples);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString value : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*value)) OcclusionIDsTemp.Add(FCString::Atoi(*value));
		else SamplesLost++;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleGetOcclusionsResponse] Number of occlusions lost druing AtoI: %i"),SamplesLost);
	// todo: pass this information to game state 
	OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionIDsTemp);
	OcclusionsStruct.SpawnAll(GetWorld(), true, false, FVector(15.0f, 15.0f, 15.0f));
	OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp, false, true); 
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("Get occlussion request timed out!"));
	return;
}

void AExperimentServiceMonitor::SpawnOcclusions(const TArray<int32> OcclusionIDsIn, const TArray<FLocation> Locations) {
	return;
}

bool AExperimentServiceMonitor::SetOcclusionVisibility(TArray<int32> VisibleOcclusionIDsIn)
{
	if (!bIsOcclusionLoaded) { UE_LOG(LogTemp, Fatal, TEXT("[AExperimentServiceMonitor::SetOcclusionVisibility] Failed to change visibility. Occlusions not pre-loaded.")); return false; }
	
	for (int i = 0; VisibleOcclusionIDsIn.Num(); i++) {
		
	}
	
	return true;
}

URequest* AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()
{
	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Starting request."));
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid.")); return nullptr; }
	if (!bInEpisode){ UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, bInEpisode is false.")); return nullptr; }
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
	UE_LOG(LogTemp, Log, TEXT("[HandleGetOcclusionLocationsResponse] %s"), *ResponseIn);
	OcclusionLocationsAll = UExperimentUtils::OcclusionsParseAllLocations(ResponseIn);

	OcclusionsStruct.SetAllLocations(OcclusionLocationsAll); 
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogTemp, Log, TEXT("[SendGetOcclusionsRequest] send request false!"));
	}
	return;
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));
	return;
}

void AExperimentServiceMonitor::HandleOcclusionLocation(const FMessage MessageIn)
{
	UE_LOG(LogTemp, Warning, TEXT("OcclusionLocation: %s"), *MessageIn.body);
}

bool AExperimentServiceMonitor::ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const int PortIn)
{
	printScreen("[AExperimentServiceMonitor::ConnectToServer()] ConnectToServer() A");
	if (!ClientIn) { printScreen("[AExperimentServiceMonitor::ConnectToServer()] ClientIn is false!"); return false;  }
	int AttemptCurr = 0;

	while (AttemptCurr < MaxAttemptsIn) {
		printScreen("[AExperimentServiceMonitor::ConnectToServer()] ConnectToServer() Attempting");
		if (ClientIn->Connect(ServerIPMessage, 4970)) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::ConnectToServer()] Connecting Tracking Service SUCCESS. (attempt #: %i/%i)"), AttemptCurr + 1, MaxAttemptsIn);
			break;
		}
		AttemptCurr += 1;
	}

	if (!ClientIn->IsConnected()) { printScreen("[AExperimentServiceMonitor::ConnectToServer()] Failed to connect to server!"); return false; }

	return true;
}

bool AExperimentServiceMonitor::test() {

	/* connect tracking service */
	Client = UMessageClient::NewMessageClient();
	constexpr int AttemptsMax = 5; 
	if (!this->ConnectToServer(Client, AttemptsMax, ServerPort)) { printScreen("ConnectToServer() failed."); return false; }

	/* Bind to Pawn's OnMovementDetected() */
	if (!this->GetPlayerPawn()) { printScreen("Player Pawn found!"); return false; }
	
	/* Set up message routes */
	// if (!this->SubscribeToExperimentServiceServer(header_prey_location)) { printScreen("Subscribe to message server FAILED!"); return false; }
	
	/* start experiment  */
	StartExperimentRequest = this->SendStartExperimentRequest(Client, "ExperimentNameIn");
	if (!StartExperimentRequest) { return false; }
	
	return true;
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();
	printScreen("[AExperimentServiceMonitor::BeginPlay()] BeginPlay()");
	test(); 
	// if(this->SendGetOcclusionLocationsRequest())
	// {
	// 	printScreen("Sent: SendGetOcclusionLocationsRequest() ");
	// } else { printScreen("Failed"); }
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// if (Client->IsConnected()){ this->UpdatePreyPosition(FVector(0.0f, 1.0f, 2.0f)); }
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPLayReason)
{
	Super::EndPlay(EndPLayReason);
	//this->DisconnectAll();
	this->SelfDestruct("EndPlay");
}

