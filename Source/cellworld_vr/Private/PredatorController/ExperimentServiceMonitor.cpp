#include "PredatorController/ExperimentServiceMonitor.h"
#include "PredatorController/AIControllerPredator.h"
#include "Kismet/GameplayStatics.h"

//#include "GenericPlatform/GenericPlatformProcess.h"

void printScreen(const FString message) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s"), *message));
}


// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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
bool AExperimentServiceMonitor::StopConnection(UMessageClient* Client)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::StopConnection] Disconecting."));
	return Client->Disconnect();
}

bool AExperimentServiceMonitor::DisconnectAll()
{

	if (bInEpisode) {
		if (!this->StopEpisode()) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Stopped active episode."));
			return false;
		}
	}

	if (!this->StopExperiment(ExperimentNameActive)) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Stopped active experiment."));
		return false;
	}

	///* stop connections if clients exist */
	//if (!ExperimentServiceClient || !ExperimentServiceClient->IsConnected()) {
	//	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Disconnected from Tracking service."));
	//	return false;
	//}

	//if (!TrackingServiceClient || !TrackingServiceClient->IsConnected()) {
	//	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectAll()] Disconnected from Tracking service."));
	//	return false;
	//}

	//ExperimentServiceClient->Disconnect();
	//TrackingServiceClient->Disconnect();

	return true;
}

/* sends request to experiment service to start experiment */
bool AExperimentServiceMonitor::StartExperiment(const FString ExperimentNameIn) {

	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not valid.")); return false; }
	if (!ExperimentServiceClient->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not connected.")); return false; }

	/* set up world info (defaults to hexagonal and canonical) */
	FWorldInfo WorldInfo;
	WorldInfo.occlusions = "21_05";

	/* set up request body */
	FStartExperimentRequest StartExperimentRequestBody;
	StartExperimentRequestBody.duration = 30;
	StartExperimentRequestBody.subject_name = ExperimentNameIn;
	StartExperimentRequestBody.world = WorldInfo;

	const FString StartExperimentRequestBodyString = UExperimentUtils::StartExperimentRequestToJsonString(StartExperimentRequestBody);
	StartExperimentRequest = ExperimentServiceClient->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);

	if (!StartExperimentRequest) { return false; }

	StartExperimentRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	StartExperimentRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);
	
	return true; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString ExperimentNameIn) {
	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not valid.")); return false; }
	if (!ExperimentServiceClient->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not connected.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, not an active experiment.")); return false; }
	
	stop_experiment_request = this->SendFinishExperimentRequest(ExperimentNameActive); // returns true; handle if false
	stop_experiment_request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentResponse);
	stop_experiment_request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentTimedOut);
	if (!stop_experiment_request) { return false; }
	return true; 
}

/* DEPRECATED */
URequest* AExperimentServiceMonitor::SendStartExperimentRequest(const FString ExperimentNameIn)
{
	return nullptr; 

	//if (!ExperimentServiceClient) { return nullptr; }
	//FStartEpisodeRequest request_body;
	//request_body.experiment_name = ExperimentNameIn;

	//FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	//URequest* request = ExperimentServiceClient->SendRequest("start_experiment", request_string, 5.0f);

	//if (!request) { return nullptr; }

	//request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse); // uses same one as start/stop
	//request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);

	//return request;
}

URequest* AExperimentServiceMonitor::SendFinishExperimentRequest(const FString ExperimentNameIn)
{
	if (!ExperimentServiceClient) { return nullptr; }
	FFinishExperimentRequest request_body;
	request_body.experiment_name = ExperimentNameIn;

	FString request_string = UExperimentUtils::FinishExperimentRequestToJsonString(request_body);
	URequest* request = ExperimentServiceClient->SendRequest("finish_experiment", request_string, 5.0f);

	if (!request) { return nullptr; }

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentResponse); // uses same one as start/stop
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleFinishExperimentTimedOut);

	return request;
}

bool AExperimentServiceMonitor::DisconnectClients() {
	/* stop connections if clients exist */
	if (!ExperimentServiceClient || !ExperimentServiceClient->IsConnected()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service."));
		return false;
	}

	if (!TrackingServiceClient || !TrackingServiceClient->IsConnected()) {
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service."));
		return false;
	}

	if (!ExperimentServiceClient->Disconnect()) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service.")); return false; }
	if (!TrackingServiceClient->Disconnect()) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::DisconnectClients()] Disconnected from Tracking service.")); return false; }
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
bool AExperimentServiceMonitor::StartEpisode() {
	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, Experiment Service client not valid.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, no active experiment.")); return false; }
	
	//start_episode_request = this->SendStartEpisodeRequest(ExperimentNameActive, "start_episode"); // returns true; handle if false
	if (!this->SendStartEpisodeRequest(ExperimentNameActive, "start_episode")) { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode() 
{
	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Can't stop episode, Experiment Service client not valid.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Warning, TEXT("Can't stop episode, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("Can't stop episode, no active experiment.")); return false; }
	if (!bInEpisode) { UE_LOG(LogTemp,Warning,TEXT("Can't stop episode, no active episode."));  return false; }

	FFinishEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameActive;

	FString request_string = UExperimentUtils::FinishEpisodeRequestToJsonString(request_body);
	URequest* request = ExperimentServiceClient->SendRequest("finish_episode", request_string, 5.0f);

	if (!request) { return false; }
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);

	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleEpisodeRequestResponse(const FString response) {

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::EpisodeResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		UE_DEBUG_BREAK();
		return;
	}

	this->SendGetOcclusionLocationsRequest();
	bInEpisode = true; // todo: fix, HandelEpisodeRequest both starts and stops, need unique functions 
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::EpisodeTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] %s"), *ResponseIn);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Experiment response: %s"), *ResponseIn));
	
	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;
	
	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Experiment name: %s"), *ExperimentNameActive);
	
	bInExperiment = true;
	
	/* todo: change, this is just for debugging while I add dynamic doors to Maze Level*/
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Warning, TEXT("map name: %s"), *MapName);

	if (MapName == "L_Maze") {

		if (!this->StartEpisode()) { 
			return; 
		}
	}

}

void AExperimentServiceMonitor::HandleStartExperimentTimedOut()
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] Start experiment timed out"));
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Start experiment timed out"));
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStartEpisodeRequest(const FString ExperimentNameIn, const FString header)
{
	if (!ExperimentServiceClient) { return nullptr; }

	FStartEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameIn;
	
	FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	StartEpisodeRequest = ExperimentServiceClient->SendRequest("start_episode", request_string, 5.0f);
	
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
	URequest* request = ExperimentServiceClient->SendRequest(header, request_string, 5.0f);

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
		UE_DEBUG_BREAK();
	}
}

/* Handle experiment service response timeout (during subscription) */
void AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceTimedOut] ES response timed out!"));
}

/* Handle tracking service response (during subscription) */
void AExperimentServiceMonitor::HandleTrackingServiceResponse(const FString message) {
	const FString msg = "[AExperimentServiceMonitor::HandleTrackingServiceResponse]" + message; 
	printScreen(msg);
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleTrackingServiceResponse] ES: %s"), *message);
	if (message != "success") {
		UE_DEBUG_BREAK();
	}
}

/* Handle tracking service response timeout (during subscription */
void AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut] Timed out!"));
}

/* create tracking service client */
bool AExperimentServiceMonitor::TrackingServiceCreateMessageClient() {
	TrackingServiceClient = UMessageClient::NewMessageClient();
	if (!TrackingServiceClient) {
		return false;  
	}
	return true;
}

/* connect tro tracking service. Connection parameters are defined in header file. */
bool AExperimentServiceMonitor::ConnectToTrackingService() {
	TrackingServiceClient = UMessageClient::NewMessageClient();
	
	if (!TrackingServiceClient) { return false; }

	/* connect tracking client to server */
	int att_max = 5;
	int att = 0;
	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting to Tracking Service."));
	while (att < att_max) {
		if (TrackingServiceClient->Connect(ServerIPMessage, PortTrackingService)) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting Tracking Service SUCCESS. (attempt #: %i)"), att);
			break;
		}
		att += 1;
	}

	if (!TrackingServiceClient->IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting to Tracking Service FAILED!"));
		return false;
	}

	return true;
}

/* route tracking service messages (steps) */
bool AExperimentServiceMonitor::TrackingServiceRouteMessages() {

	/* route to send data*/
	//TrackingServiceRoute = TrackingServiceClient->AddRoute(header_tracking_service);
	
	/* handle messages received with unknown headers/routes */
	TrackingServiceClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage);

	/* prey route */
	TrackingServiceRoutePrey = TrackingServiceClient->AddRoute(header_tracking_service_prey);

	/* predator route */
	TrackingServiceRoutePredator = TrackingServiceClient->AddRoute(header_tracking_service_predator);

	/* bind only if all routes valid */
	//if (!TrackingServiceRoute && !TrackingServiceRoutePrey && !TrackingServiceRoutePredator) {
	if (!TrackingServiceRoutePrey && !TrackingServiceRoutePredator) {
		return false;
	}

	//TrackingServiceRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessage);
	//TrackingServiceRoutePrey->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessagePrey);
	//TrackingServiceRoutePredator->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessagePredator);

	return true; 
}


/* subscribes to server and calls UpdatePredator() when messages[header] matches input header. */
bool AExperimentServiceMonitor::SubscribeToTrackingService()
{
	bConnectedToTrackingService = this->ConnectToTrackingService();
	if (!bConnectedToTrackingService) {
		printScreen("[AExperimentServiceMonitor::SubscribeToTrackingService()] ConnectToTrackingService() failed");
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SubscribeToTrackingService()] Retuning. Connecting to Tracking Service FAILED!"));
		return false;
	}

	/* 3. subscribe */
	TrackingServiceRequest = TrackingServiceClient->Subscribe();
	if (!TrackingServiceRequest) { 
		printScreen("[AExperimentServiceMonitor::SubscribeToTrackingService()] if (!TrackingServiceRequest) .");
		return false;  
	}

	TrackingServiceRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceResponse);
	TrackingServiceRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut);
	
	return true;
}

/* subscribe to experiment service monitor */
bool AExperimentServiceMonitor::SubscribeToExperimentServiceServer(FString header)
{	
	ExperimentServiceClient = UMessageClient::NewMessageClient();

	if (!ExperimentServiceClient) { return false; }
	ExperimentServiceClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceUnroutedMessage);
	int att_max = 5;
	int att = 0;
	while (att < att_max) {
		if (ExperimentServiceClient->Connect(ServerIPMessage, PortExperimentService)) { 
			bConnectedToExperimentService = true;
			FPlatformProcess::Sleep(0.2);
			break; 
		}
		att += 1;
	}

	URequest* ExperimentServiceRequestSubscribe = ExperimentServiceClient->Subscribe();
	if (!ExperimentServiceRequestSubscribe) {
		return false; 
	}

	ExperimentServiceRequestSubscribe->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceResponse);
	ExperimentServiceRequestSubscribe->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut);

	/* Update prey route */
	ExperimentServicePreyRoute = ExperimentServiceClient->AddRoute(header);
	if (!ExperimentServicePreyRoute) { return false;  }
	ExperimentServicePreyRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::UpdatePredator);
	
	return true;
}

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(const FMessage message)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *message.body);

	if (TrackingServiceClient == nullptr) { return; }
	frame_count++;
	FStep step = UExperimentUtils::JsonStringToStep(message.body); 
	FVector new_location_ue = UExperimentUtils::CanonicalToVr(step.location,MapLength, WorldScale); // ue --> unreal engine units 
	
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("New location: %f %f %f"), new_location_ue.X, new_location_ue.Y, new_location_ue.Z));
	
	//AController* Controller = CharacterPredator->GetController();
	AAIControllerPredator* AIControllerPredator = Cast<AAIControllerPredator>(CharacterPredator->GetController());
	if (!AIControllerPredator) { return; }
	
	AIControllerPredator->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location_ue);
	bCanUpdatePreyPosition = true;
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(const FVector vector)
{
/*
	self.time_stamp = time_stamp 
	self.rotation = rotation 
*/
	FLocation Location = UExperimentUtils::VrToCanonical(vector, MapLength, WorldScale);

	/* prepare Step */
	//FString header_prey = "send_step";
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = frame_count;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: change! 
	send_step.time_stamp = frame_count; // todo: change to querycounterelapsedtime!

	/* convert FStep to JsonString */
	FString body = UExperimentUtils::StepToJsonString(send_step); 

	/* send message to ES */
	FMessage message = UMessageClient::NewMessage(header_tracking_service, body);
	UE_LOG(LogTemp, Log, TEXT("sending prey step: (route: %s; body: %s"),*header_tracking_service,*message.body);
	if (TrackingServiceClient->IsConnected()) {
		TrackingServiceClient->SendMessage(message);
	}

	/* don't overload server with messages before its done processing previous prey update. */
	//bCanUpdatePreyPosition = false;
}

/* handle tracking service message coming to default "send_step" route */
void AExperimentServiceMonitor::HandleTrackingServiceMessage(const FMessage message)
{
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("tracking message: (%s) %s"),*message.header, *message.body));
	}
}

/* handles unrouted messages coming from tracking service. messages that do not have a route set up inside 
this class (ExperimentServiceMonitor) will end up here */
void AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage(const FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

void AExperimentServiceMonitor::HandleExperimentServiceUnroutedMessage(const FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

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
	}
	else if (Cast<APawnDebug>(Pawn)) {
		APawnDebug* PlayerPawn = Cast<APawnDebug>(Pawn); // Adjust according to the actual type of PlayerPawn
		UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnDebug found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnDebug found and assigned."));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] Pawn found, but it is neither APawnMain nor APawnDebug."));
		return false;
	}

	return true;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString ErrorMessageIn)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *ErrorMessageIn));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying."));

	// Make sure to check if the actor is valid and has not already been marked for destruction.
	//if (!this->IsPendingKill())
	if (!this->IsValidLowLevelFast())
	{
		this->DisconnectAll();

		// Destroy the actor.
		//this->Destroy();
	}
}

/* get occlusions in our specific experiment (FWorldInfo.occlusions; default: "21_05") */
bool AExperimentServiceMonitor::SendGetOcclusionsRequest()
{
	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Cant send get occlusion request, Experiment service client not valid.")); return false; } // experiment service client not valid
	URequest* Request = ExperimentServiceClient->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!Request) { return false; } // failed to send request
	FPlatformProcess::Sleep(0.5);
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionsTimedOut);

	return true;
}

void AExperimentServiceMonitor::SpawnOcclusions(const TArray<int32> OcclusionIDsIn, const TArray<FLocation> Locations) {
	
	/*OcclusionsStruct.SetAllLocations(Locations);
	*/
	//if (!OcclusionsStruct.SpawnAll(true, false)) {
	//	UE_LOG(LogTemp, Error, TEXT("SpawnAll failed"));
	//	return; 
	//}

	//FRotator Rotation(0.0f, 0.0f, 0.0f); // Desired spawn rotation
	//FVector Location(0.0f, 0.0f, 0.0f); // Desired spawn location

	//FActorSpawnParameters SpawnParams;
	//for (int i = 0; i < Locations.Num(); i++) {
	//	AOcclusion* MyMeshActor = GetWorld()->SpawnActor<AOcclusion>(
	//		AOcclusion::StaticClass(), 
	//		UExperimentUtils::CanonicalToVr(Locations[i], 250, 15), 
	//		Rotation, 
	//		SpawnParams);

	//	MyMeshActor->SetActorScale3D(FVector(WorldScale,WorldScale, WorldScale));

	//	UE_LOG(LogTemp, Warning, TEXT("Spawned occlusion (%i/%i)."),i,Locations.Num());
	//}
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
	OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp);
	//OcclusionsStruct.SetAllLocations()
	//SpawnOcclusions(OcclusionIDsIntArr, OcclusionLocationsAll);
	return;
}

void AExperimentServiceMonitor::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("Get occlussion request timed out!"));
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
	if (!ExperimentServiceClient) { UE_LOG(LogTemp, Error, TEXT("Cant send get occlusion request, Experiment service client not valid.")); return nullptr; }
	
	const FString BodyOut   = "bodyout";
	const FString HeaderOut = "get_cells_locations";
	URequest* Request = ExperimentServiceClient->SendRequest(HeaderOut,BodyOut,TimeOut);

	if (!Request) { return nullptr; }

	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut);

	return Request;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {
	UE_LOG(LogTemp, Log, TEXT("%s"), *ResponseIn);
	OcclusionLocationsAll = UExperimentUtils::OcclusionsParseAllLocations(ResponseIn);

	OcclusionsStruct.SetAllLocations(OcclusionLocationsAll); 
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogTemp, Log, TEXT("[SendGetOcclusionsRequest] send request false!"));
	}
	return;
}

void AExperimentServiceMonitor::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("Get cell location request timed out!"));
	return;
}

void AExperimentServiceMonitor::HandleOcclusionLocation(const FMessage MessageIn)
{
	UE_LOG(LogTemp, Warning, TEXT("OcclusionLocation: %s"), *MessageIn.body);
}

bool AExperimentServiceMonitor::test() {
	UMessageClient* Client = UMessageClient::NewMessageClient();

	if (!Client) { return false; }

	/* connect tracking client to server */
	int att_max = 5;
	int att = 0;
	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting to Tracking Service."));
	while (att < att_max) {
		if (Client->Connect("127.0.0.1", 4566)) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting Tracking Service SUCCESS."));
			break;
		}
		att += 1;
	}
	if (!Client->IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting to Tracking Service FAILED!"));
		return false;
	}

	const FString header = "get_occlusions";
	const FString body = "21_04";
	URequest* request = Client->SendRequest("get_cell_locations", "21_04", 10.0f);
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleGetOcclusionLocationsResponse);

	return true;
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();

	//test(); 

	/* connect tracking service */
	bSubscribedToTrackingService = this->SubscribeToTrackingService();
	if (!bSubscribedToTrackingService) { 
		this->SelfDestruct(FString("Subscribe to TS failed."));
		return;
	}

	/* route tracking messages */
	bRoutedMessagesTrackingService = this->TrackingServiceRouteMessages();
	if (!bRoutedMessagesTrackingService) { 
		this->SelfDestruct(FString("Create routes for TS failed."));
		return;
	}


	/* subscribe to ES */
	if (!this->SubscribeToExperimentServiceServer(header_tracking_service)) { 
		this->SelfDestruct(FString("Subscribe to ES failed."));
		return;
	}

	/* start experiment */
	if (!this->StartExperiment(SubjectName)) {
		this->SelfDestruct(FString("StartExperiment failed."));
		return; 
	}

	/*if (!this->SpawnAndPossessPredator()) {
		this->SelfDestruct(FString("spawn and possess predator failed."));
		return;
	}*/

	if (!this->GetPlayerPawn()) {
		this->SelfDestruct(FString("GetPlayerPawn() failed.."));
		return;
	}

	if (TrackingServiceClient->IsConnected() && GEngine) {
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Tracking service connected"));
	}

	/* not subscribing correctly */
	if (ExperimentServiceClient->IsConnected() && GEngine) {
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("experiment service connected"));
	}
	
}

/* run a (light!) command every frame */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bConnectedToServer) { return; }
}

void AExperimentServiceMonitor::EndPlay(const EEndPlayReason::Type EndPLayReason)
{
	Super::EndPlay(EndPLayReason);
	//this->DisconnectAll();
	this->SelfDestruct("EndPlay");


}

