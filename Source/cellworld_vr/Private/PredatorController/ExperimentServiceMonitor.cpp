// Fill out your copyright notice in the Description page of Project Settings.


#include "PredatorController/ExperimentServiceMonitor.h"
#include "PredatorController/AIControllerPredator.h"
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

void AExperimentServiceMonitor::HandleExperimentServiceMessage(FMessage message) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleExperimentServiceMessage] %s"),*message.body);
}

bool AExperimentServiceMonitor::SubscribeToExperimentService(FString header) {
	
	//ExperimentServerClient = UMessageClient::NewMessageClient();
	if (!ExperimentServerClient) { return false; }
	ExperimentServerClient->Subscribe();

	ExperimentServerClientRoute = ExperimentServerClient->AddRoute(header);

	if (!ExperimentServerClientRoute) { return false; }

	ExperimentServerClientRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleExperimentServiceMessage);
	return true;
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
URequest* AExperimentServiceMonitor::SendEpisodeRequest(const FString experiment, const FString header)
{
	FStartEpisodeRequest request_body;
	request_body.experiment_name = experiment;
	FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	URequest* request = ExperimentServerClient->SendRequest(header, request_string, 5.0f);
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
	start_episode_request = this->SendEpisodeRequest(experiment, "start_episode"); // returns true; handle if false
	if (!start_episode_request) { return false; }
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceMonitor::StopEpisode(const FString experiment) {

	start_episode_request = this->SendEpisodeRequest(experiment, "finish_episode"); // returns true; handle if false
	if (!stop_episode_request) { return false; }
	return true;
}

/* checks if subscription was successful */
void AExperimentServiceMonitor::HandleSubscriptionResponse(const FString message) {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleSubscriptionResponse] %s"), *message);
	if (message != "success") {
		UE_DEBUG_BREAK();
	}
}

void AExperimentServiceMonitor::HandleSubscriptionTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceMonitor::HandleSubscriptionTimedOut] Timed out!"));

}

/* subscribes to server and calls UpdatePredator() when messages[header] matches input header. */
bool AExperimentServiceMonitor::SubscribeToServer(FString header)
{

	if (!PredatorMessageClient) { return false; }

	URequest* request = PredatorMessageClient->Subscribe();
	if (!request) { return false; }

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscriptionResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceMonitor::HandleSubscriptionTimedOut);

	MessageRoute = PredatorMessageClient->AddRoute(header);
	if (!MessageRoute) { return false; }
	MessageRoute->MessageReceived.AddDynamic(this, &AExperimentServiceMonitor::UpdatePredator);

	return true;
}

/* temp function */
bool AExperimentServiceMonitor::ServerConnect()
{
	PredatorMessageClient = UMessageClient::NewMessageClient();

	if (PredatorMessageClient == nullptr) {
		return false;
	}

	bConnectedToServer = PredatorMessageClient->Connect(ServerIPMessage, PortMessageServer);
	return bConnectedToServer;
}

/* creates a message client; todo: implement and replace old */
UMessageClient* AExperimentServiceMonitor::ServerConnectMessageClient(const FString IP, const int port, bool& result)
{
	UMessageClient* Client = UMessageClient::NewMessageClient();

	if (Client == nullptr){
		result = false;
		return Client;
	}
	result = Client->Connect(IP, port);
	return Client;
}

void AExperimentServiceMonitor::ServerConnectAttempts(int attempts)
{
	int i = 0; 
	while (i <= attempts) {
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::ServerConnectAttempts] Trying to connect."));
		this->ServerConnect();
		i++; 

		if (bConnectedToServer) { 
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Predator controller connection: Success."));
			return; 
		}
	}
}

void AExperimentServiceMonitor::UpdatePredator(FMessage message)
{
	if (PredatorMessageClient == nullptr) { return; }
	n_samples++;
	FStep step = UExperimentUtils::JsonStringToStep(message.body); 
	FVector new_location_ue = UExperimentUtils::canonicalToVr(step.location,map_length); // ue --> unreal engine units 
	
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("New location: %f %f %f"), new_location_ue.X, new_location_ue.Y, new_location_ue.Z));
	//AController* Controller = CharacterPredator->GetController();
	AAIControllerPredator* AIControllerPredator = Cast<AAIControllerPredator>(CharacterPredator->GetController());
	if (!AIControllerPredator) { return; }
	AIControllerPredator->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location_ue);
	bCanUpdatePreyPosition = true;
}

bool AExperimentServiceMonitor::UpdatePreyPosition(FMessage message)
{
	/* todo: get prey location */
	if (!bCanUpdatePreyPosition){ return false; }

	UE_LOG(LogTemp, Warning, TEXT("prey_step: %s"),*message.body);
	PredatorMessageClient->SendMessage(message);

	/* don't overload server with messages before its done processing previous prey update. */
	bCanUpdatePreyPosition = false;
	return true;
}

// Called when the game starts or when spawned
void AExperimentServiceMonitor::BeginPlay()
{
	Super::BeginPlay();

	this->ServerConnectAttempts(5);

	if (!bConnectedToServer) {
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, TEXT("[AExperimentServiceMonitor::BeginPlay()] Failed to connect to server."));
	}
	if (!this->SubscribeToServer(predator_step_header)) {
		UE_DEBUG_BREAK();
	}

	if (!this->SpawnAndPossessPredator()) {
		UE_DEBUG_BREAK();
	}
}

void AExperimentServiceMonitor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bConnectedToServer) { return; }
}

