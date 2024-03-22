// Fill out your copyright notice in the Description page of Project Settings.


#include "PredatorController/ExperimentServiceMonitor.h"
#include "PredatorController/AIControllerPredator.h"
#include "Kismet/GameplayStatics.h"
#include "ExperimentUtils.h"
//#include "GenericPlatform/GenericPlatformProcess.h"

// Sets default values
AExperimentServiceMonitor::AExperimentServiceMonitor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

/* todo: make this take input ACharacter and spawn that one*/
bool AExperimentServiceMonitor::SpawnAndPossessPredator() {

	if (GetWorld())
	{
		// Define spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Specify the location and rotation for the new actor
		FVector Location(-2426.0f, 1264.0f, 90.0f); // Change to desired spawn location
		FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation

		// Spawn the character
		CharacterPredator = GetWorld()->SpawnActor<ACharacterPredator>(ACharacterPredator::StaticClass(), Location, Rotation, SpawnParams);

		// Ensure the character was spawned
		if (CharacterPredator)
		{
			return true;
		}
	}
	return false;
}

/* stop connection for ClientIn */
bool AExperimentServiceMonitor::StopConnection(UMessageClient* Client)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::StopConnection] Disconecting."));
	return Client->Disconnect();
}

/* sends request to experiment service to start experiment */
bool AExperimentServiceMonitor::StartExperiment(const FString ExperimentNameIn) {

	if (!ExperimentServiceClient) { return false; }
	if (!ExperimentServiceClient->IsConnected()) { return false; }

	FWorldInfo world_info; 
	FString world_info_string = UExperimentUtils::WorldInfoToJsonString(world_info);

	FStartExperimentRequestVR request_body;
	request_body.subject_name = ExperimentNameIn;
	
	FString request_string = UExperimentUtils::StartExperimentRequestToJsonString(request_body);
	start_experiment_request = ExperimentServiceClient->SendRequest("start_experiment_vr", request_string, 5.0f);

	if (!start_experiment_request) { return false; }

	start_experiment_request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentResponse);
	start_experiment_request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleStartExperimentTimedOut);

	return true; 
}

bool AExperimentServiceMonitor::StopExperiment(const FString ExperimentNameIn) {

	if (!ExperimentServiceClient->IsConnected()) { return false; }
	return true; 
}


/* start experiment service episode stream */
bool AExperimentServiceMonitor::StartEpisode(const FString ExperimentNameIn) {
	start_episode_request = this->SendStartEpisodeRequest(ExperimentNameIn, "start_episode"); // returns true; handle if false
	if (!start_episode_request) { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode(const FString ExperimentNameIn) {

	start_episode_request = this->SendStopEpisodeRequest(ExperimentNameIn, "stop_episode"); // returns true; handle if false
	if (!stop_episode_request) { return false; }
	return true;
}

/* Handle episode response */
void AExperimentServiceMonitor::HandleEpisodeRequestResponse(const FString response) {

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::EpisodeResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response != "success") {
		UE_DEBUG_BREAK();
	}
	bInEpisode = true;
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::HandleEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::EpisodeTimedOut()] Episode request timed out!"))
}

void AExperimentServiceMonitor::HandleStartExperimentResponse(const FString ResponseIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleStartExperimentResponse] %s"), *ResponseIn);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Episode response: %s"), *ResponseIn));

	if (ResponseIn != "success") {
		UE_DEBUG_BREAK();
	}
	bInExperiment = true;
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
	URequest* request = ExperimentServiceClient->SendRequest(header, request_string, 5.0f);
	
	if (!request) { return nullptr; }

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleEpisodeRequestTimedOut);
	
	return request;
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStopEpisodeRequest(const FString ExperimentNameIn, const FString header)
{
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

/* attempt to connect tracking service */
bool AExperimentServiceMonitor::ConnectTrackingService() {
	return TrackingServiceClient->Connect(ServerIPMessage, PortTrackingService);
}

/* connect tro tracking service. Connection parameters are defined in header file. */
bool AExperimentServiceMonitor::ConnectToTrackingService() {
	TrackingServiceClient = UMessageClient::NewMessageClient();

	if (!TrackingServiceClient) {
		return false;
	}

	/* connect tracking client to server */
	int att_max = 5;
	int att = 0;
	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting to Tracking Service."));
	while (att < att_max) {
		if (TrackingServiceClient->Connect(ServerIPMessage, PortTrackingService)) {
			UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::TrackingServiceSetRoute()] Connecting Tracking Service SUCCESS."));
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
	TrackingServiceRoute = TrackingServiceClient->AddRoute(header_tracking_service);
	
	/* handle messages received with unknown headers/routes */
	TrackingServiceClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage);

	/* prey route */
	TrackingServiceRoutePrey = TrackingServiceClient->AddRoute(header_tracking_service_prey);

	/* predator route */
	TrackingServiceRoutePredator = TrackingServiceClient->AddRoute(header_tracking_service_predator);

	/* bind only if all routes valid */
	if (!TrackingServiceRoute && !TrackingServiceRoutePrey && !TrackingServiceRoutePredator) {
		return false;
	}

	TrackingServiceRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessage);
	TrackingServiceRoutePrey->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessagePrey);
	TrackingServiceRoutePredator->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessagePredator);

	return true; 
}

/* subscribes to server and calls UpdatePredator() when messages[header] matches input header. */
bool AExperimentServiceMonitor::SubscribeToTrackingService()
{
	/* 
	steps: 
		1. connect 
		2. add route 
		3. subscribe 
	*/

	bCOnnectedToTrackingService = this->ConnectToTrackingService();

	if (!bCOnnectedToTrackingService) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SubscribeToTrackingService()] Retuning. Connecting to Tracking Service FAILED!"));
		return false;
	}

	/* 3. subscribe */
	TrackingServiceRequest = TrackingServiceClient->Subscribe();
	TrackingServiceRequest->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceResponse);
	TrackingServiceRequest->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut);
	
	return true;
}

/* subscribe to experiment service monitor */
bool AExperimentServiceMonitor::SubscribeToExperimentServiceServer(FString header)
{	
	ExperimentServiceClient = UMessageClient::NewMessageClient();

	if (!ExperimentServiceClient) { 
		return false; 
	}

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

	ExperimentServiceRoute = ExperimentServiceClient->AddRoute(header);
	if (!ExperimentServiceRoute) { 
		return false; 
	}

	ExperimentServiceRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::UpdatePredator);
	return true;
}

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceMonitor::UpdatePredator(FMessage message)
{
	if (TrackingServiceClient == nullptr) { return; }
	frame_count++;
	FStep step = UExperimentUtils::JsonStringToStep(message.body); 
	FVector new_location_ue = UExperimentUtils::canonicalToVr(step.location,map_length); // ue --> unreal engine units 
	
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("New location: %f %f %f"), new_location_ue.X, new_location_ue.Y, new_location_ue.Z));
	
	//AController* Controller = CharacterPredator->GetController();
	AAIControllerPredator* AIControllerPredator = Cast<AAIControllerPredator>(CharacterPredator->GetController());
	if (!AIControllerPredator) { return; }
	
	AIControllerPredator->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location_ue);
	bCanUpdatePreyPosition = true;
}

/* get updated player position and send to prey route via tracking service client */
void AExperimentServiceMonitor::UpdatePreyPosition(FVector vector)
{
/*
	self.time_stamp = time_stamp 
	self.rotation = rotation 
*/
	FLocation Location; 
	Location.x = vector.X;
	Location.y = vector.Y;

	/* prepare Step */
	FString header_prey = "send_step";
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = frame_count;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: change! 
	send_step.time_stamp = frame_count; // todo: change!

	/* convert FStep to JsonString */
	FString body = UExperimentUtils::StepToJsonString(send_step); 

	/* send message to ES */
	FMessage message = UMessageClient::NewMessage(header_tracking_service, body);

	UE_LOG(LogTemp, Log, TEXT("send_step: (%s) %s"),*header_tracking_service,*message.body);
	if (TrackingServiceClient->IsConnected()) {
		TrackingServiceClient->SendMessage(message);
	}

	/* don't overload server with messages before its done processing previous prey update. */
	bCanUpdatePreyPosition = false;
}

/* handle tracking service message coming to default "send_step" route */
void AExperimentServiceMonitor::HandleTrackingServiceMessage(FMessage message)
{
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("tracking message: (%s) %s"),*message.header, *message.body));
	}
}

/* handles unrouted messages coming from tracking service. messages that do not have a route set up inside 
this class (ExperimentServiceMonitor) will end up here */
void AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage(FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("tracking unrouted: (%s) %s"), *message.header, *message.body));
}

/* gets player pawn from world */
bool AExperimentServiceMonitor::GetPlayerPawn()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] No pawn found."));
		return false;
	}

	// Attempt to cast to APawnMain or APawnDebug and assign to PlayerPawn if successful.
	if (Cast<APawnMain>(Pawn)) {
		APawnMain* PlayerPawn = Cast<APawnMain>(Pawn); // Assuming PlayerPawn is a member of type APawn* or APawnMain*
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnMain found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	}
	else if (Cast<APawnDebug>(Pawn)) {
		APawnDebug* PlayerPawn = Cast<APawnDebug>(Pawn); // Adjust according to the actual type of PlayerPawn
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] APawnDebug found and assigned."));
		PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);
	}
	else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("[AExperimentServiceMonitor::GetPlayerPawn()] Pawn found, but it is neither APawnMain nor APawnDebug."));
		return false;
	}

	return true;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceMonitor::SelfDestruct(const FString ErrorMessageIn)
{
	const FString DebugMessage = TEXT("[AExperimentServiceMonitor::SelfDestruct] Tracking and Experiment ABORTED. Something happened.");
	if (GEngine) GEngine->AddOnScreenDebugMessage(1, 30.0f, FColor::Red, DebugMessage);

	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Something went wrong. Destroying."));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceMonitor::SelfDestruct()] Error Message: %s."), *ErrorMessageIn);
	
	// Make sure to check if the actor is valid and has not already been marked for destruction.
	if (!this->IsPendingKill())
	{
		/* stop connections if clients exist */
		if (ExperimentServiceClient && ExperimentServiceClient->IsConnected()) { 
			ExperimentServiceClient->Disconnect(); 
		}
		if (TrackingServiceClient && TrackingServiceClient->IsConnected()) { 
			TrackingServiceClient->Disconnect(); 
		}
		
		// Destroy the actor.
		this->Destroy();
	}
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();

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
	if (!this->StartExperiment(experiment_name)) {
		this->SelfDestruct(FString("StartExperiment failed."));
		return; 
	}


	/* start episode in ES */
	//if (!bInExperiment || !this->StartEpisode(FString(experiment_name))){
	if (!this->StartEpisode(FString(experiment_name))){
		this->SelfDestruct(FString("Start episode failed"));
		return;
	}

	if (!this->SpawnAndPossessPredator()) {
		this->SelfDestruct(FString("spawn and possess predator failed."));
		return;
	}
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

