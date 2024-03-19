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


bool AExperimentServiceMonitor::StopConnection(UMessageClient* Client)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::StopConnection] Disconecting."));
	return Client->Disconnect();
}

void AExperimentServiceMonitor::EpisodeResponse(const FString response) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::EpisodeResponse] %s"), *response);
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStartEpisodeRequest(const FString experiment, const FString header)
{
	if (!ExperimentServiceClient) { 
		return nullptr;  
	}

	FStartEpisodeRequest request_body;
	request_body.experiment_name = experiment;
	
	FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	URequest* request = ExperimentServiceClient->SendRequest(header, request_string, 5.0f);
	
	if (!request) { 
		return nullptr; 
	}

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::EpisodeResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::EpisodeTimedOut);
	
	return request;
}

/* create and send simple experiment service request */
URequest* AExperimentServiceMonitor::SendStopEpisodeRequest(const FString experiment, const FString header)
{
	FFinishEpisodeRequest request_body;
	request_body.experiment_name = experiment;

	FString request_string = UExperimentUtils::FinishEpisodeRequestToJsonString(request_body);
	URequest* request = ExperimentServiceClient->SendRequest(header, request_string, 5.0f);

	if (!request) { return nullptr; }
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::EpisodeResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::EpisodeTimedOut);

	return request;
}

/* handle experiment service timeout */
void AExperimentServiceMonitor::EpisodeTimedOut() {
	UE_LOG(LogTemp,Error,TEXT("[AExperimentServiceMonitor::EpisodeTimedOut()] Episode request timed out!"))
}

/* start experiment service episode stream */
bool AExperimentServiceMonitor::StartEpisode(const FString experiment) {
	start_episode_request = this->SendStartEpisodeRequest(experiment, "start_episode"); // returns true; handle if false
	if (!start_episode_request) { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode(const FString experiment) {

	start_episode_request = this->SendStopEpisodeRequest(experiment, "stop_episode"); // returns true; handle if false
	if (!stop_episode_request) { return false; }
	return true;
}

/* handle experiment service message received */
void AExperimentServiceMonitor::HandleExperimentServiceMessage(FMessage message) {
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("experiment message : %s"), *message.body));
	}
}

/* handle experiment service message timeout */
void AExperimentServiceMonitor::HandleExperimentServiceMessageTimedOut(FMessage message) {
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("experiment timeout message : %s"), *message.body));
	}
}

/* Handle experiment service response (during subscription) */
void AExperimentServiceMonitor::HandleExperimentServiceResponse(const FString message) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceResponse] ES: %s"), *message);
	if (message != "success") {
		UE_DEBUG_BREAK();
	}
}

/* Handle experiment service response timeout (during subscription */
void AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceTimedOut] ES Timed out!"));
}

/* Handle tracking service response (during subscription) */
void AExperimentServiceMonitor::HandleTrackingServiceResponse(const FString message) {
	UE_LOG(LogTemp, Warning, TEXT("[HandleTrackingServiceResponseResponse::HandleExperimentServiceResponse] ES: %s"), *message);
	if (message != "success") {
		UE_DEBUG_BREAK();
	}
}

/* Handle tracking service response timeout (during subscription */
void AExperimentServiceMonitor::HandleTrackingServiceResponseTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("HandleTrackingServiceResponseTimedOut] Timed out!"));
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

bool AExperimentServiceMonitor::ConnectToTrackingService() {
	TrackingServiceClient = UMessageClient::NewMessageClient();

	if (!TrackingServiceClient) {
		return false;
	}

	/* 1. connect tracking client to server */
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
	TrackingServiceRoute = TrackingServiceClient->AddRoute(header_tracking_service);
	TrackingServiceRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceMessage);
	TrackingServiceClient->UnroutedMessageEvent.AddDynamic(this, &AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage);
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

	URequest* request = ExperimentServiceClient->Subscribe();
	if (!request) { 
		return false; 
	}

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceResponseTimedOut);

	ExperimentServiceRoute = ExperimentServiceClient->AddRoute(header);
	if (!ExperimentServiceRoute) { 
		return false; 
	}

	ExperimentServiceRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::UpdatePredator);
	return true;
}

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
	//FMessage message = UMessageClient::NewMessage(header_prey, body);
	FMessage message = UMessageClient::NewMessage(header_tracking_service, body);

	//UE_LOG(LogTemp, Log, TEXT("send_step: %s"),*message.body);
	if (TrackingServiceClient->IsConnected()) {
		TrackingServiceClient->SendMessage(message);
	}

	/* don't overload server with messages before its done processing previous prey update. */
	bCanUpdatePreyPosition = false;
}

void AExperimentServiceMonitor::HandleTrackingServiceMessage(FMessage message)
{
	if (GEngine) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("tracking message : %s"), *message.body));
	}
}

void AExperimentServiceMonitor::HandleTrackingServiceUnroutedMessage(FMessage message)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("tracking timedout : %s"), *message.body));
}

/* gets player pawn from world */
bool AExperimentServiceMonitor::GetPlayerPawn()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Pawn) { return false; }

	PlayerPawn = Cast<APawnMain>(Pawn);
	if (!PlayerPawn) { return false; }
	
	PlayerPawn->MovementDetectedEvent.AddDynamic(this, &AExperimentServiceMonitor::UpdatePreyPosition);

	return true;
}

/* main stuff happens here */
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();

	/* connect tracking service */
	bSubscribedToTrackingService = this->SubscribeToTrackingService();
	if (!bSubscribedToTrackingService) { UE_DEBUG_BREAK(); }

	/* route tracking messages */
	bRoutedMessagesTrackingService = this->TrackingServiceRouteMessages();
	if (!bRoutedMessagesTrackingService) { UE_DEBUG_BREAK(); }

	if (!this->SubscribeToExperimentServiceServer(predator_step_header)) { UE_DEBUG_BREAK(); }

	if (!this->SpawnAndPossessPredator()) {
		UE_DEBUG_BREAK();
	}

	if (!this->GetPlayerPawn()) {
		UE_DEBUG_BREAK();
	}

	if (TrackingServiceClient->IsConnected() && GEngine) {
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Tracking service connected"));
	}

	/* not subscribing correctly */
	if (ExperimentServiceClient->IsConnected() && GEngine) {
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("experiment service connected"));
	}
}

/* hopefully never used */
void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bConnectedToServer) { return; }
}

