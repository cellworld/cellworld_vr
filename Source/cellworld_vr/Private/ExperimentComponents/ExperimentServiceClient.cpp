// Fill out your copyright notice in the Description page of Project Settings.


#include "ExperimentComponents/ExperimentServiceClient.h"
#include "PredatorController/ExperimentServiceMonitor.h"
#include "PredatorController/AIControllerPredator.h"
#include "Kismet/GameplayStatics.h"
#include "ExperimentComponents/ExperimentServiceClient.h"

//#include "GenericPlatform/GenericPlatformProcess.h"

//TODO - add argument to include MessageType (Log, Warning, Error, Fatal)
void AExperimentServiceClient::printScreen(const FString InMessage) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s"), *InMessage));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *InMessage);
}

// Sets default values
AExperimentServiceClient::AExperimentServiceClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

/* sends request to experiment service to start experiment */
bool AExperimentServiceClient::StartExperiment(const FString InSubjectName) {

	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not valid.")); return false; }
	if (!Client->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't start experiment, Experiment Service client not connected.")); return false; }

	/* set up world info (defaults to hexagonal and canonical) */
	FWorldInfoV2 WorldInfo;
	WorldInfo.occlusions = "21_05";

	/* set up request body */
	FStartExperimentRequest StartExperimentRequestBody;
	StartExperimentRequestBody.duration = 30;
	StartExperimentRequestBody.subject_name = InSubjectName;
	StartExperimentRequestBody.world = WorldInfo;

	const FString StartExperimentRequestBodyString = UExperimentUtils::StartExperimentRequestToJsonString(StartExperimentRequestBody);
	this->printScreen(StartExperimentRequestBodyString);
	URequest* Request = Client->SendRequest("start_experiment", StartExperimentRequestBodyString, 5.0f);

	if (!Request) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::StartExperiment()] StartExperimentRequest is NULL.")); return false; }

	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleStartExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleStartExperimentTimedOut);
	return true; 
}

bool AExperimentServiceClient::StopExperiment(const FString ExperimentNameIn) {
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not valid.")); return false; }
	if (!Client->IsConnected()) { UE_LOG(LogTemp, Error, TEXT("Can't stop experiment, Experiment Service client not connected.")); return false; }
	//if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Warning, TEXT("Can't stop experiment, not an active experiment.")); return false; }
	
	URequest* Request = this->SendFinishExperimentRequest(ExperimentNameActive); // returns true; handle if false
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleFinishExperimentResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleFinishExperimentTimedOut);
	if (!Request) { return false; }
	return true; 
}

bool AExperimentServiceClient::Disconnect()
{
	return false;
}

/* DEPRECATED */
URequest* AExperimentServiceClient::SendStartExperimentRequest(const FString ExperimentNameIn)
{
	return nullptr; 
}

URequest* AExperimentServiceClient::SendFinishExperimentRequest(const FString ExperimentNameIn)
{
	if (!Client) { return nullptr; }
	FFinishExperimentRequest request_body;
	request_body.experiment_name = ExperimentNameIn;

	FString request_string = UExperimentUtils::FinishExperimentRequestToJsonString(request_body);
	URequest* request = Client->SendRequest("finish_experiment", request_string, 5.0f);

	if (!request) { return nullptr; }

	request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleFinishExperimentResponse); // uses same one as start/stop
	request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleFinishExperimentTimedOut);

	return request;
}

void AExperimentServiceClient::HandleFinishExperimentResponse(const FString ResponseIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleFinishExperimentResponse] %s"), *ResponseIn);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Experiment response: %s"), *ResponseIn));

	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;

	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleFinishExperimentResponse] Experiment name: %s"), *ExperimentNameActive);
	bool bStopAllConnections = true; 

	if (!bStopAllConnections) { return; }
	
	if (Client->Disconnect()) { UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleFinishExperimentResponse] Disconnected clients successfully.")); return; }
	else{ UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::HandleFinishExperimentResponse] Disconnect clients failed."));}
}

void AExperimentServiceClient::HandleFinishExperimentTimedOut()
{
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::HandleFinishExperimentTimedOut()] Finish experiment request timed out!"))
}

/* start experiment service episode stream */
bool AExperimentServiceClient::StartEpisode() {
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, Experiment Service client not valid.")); return false; }
	if (ExperimentNameActive.Len() < 1) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, experiment name not valid.")); return false; }
	if (!bInExperiment) { UE_LOG(LogTemp, Error, TEXT("Can't start episode, no active experiment.")); return false; }
	
	// send request 
	if (!this->SendStartEpisodeRequest(ExperimentNameActive, "start_episode")) { return false; }
	
	return true;
}

/* stop experiment service episode stream */
bool AExperimentServiceClient::StopEpisode() 
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
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestTimedOut);

	return true;
}

/* Handle episode response */
void AExperimentServiceClient::HandleEpisodeRequestResponse(const FString response) {

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::EpisodeResponse] %s"), *response);
	if (GEngine) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Episode response: %s"), *response));
	
	if (response == "fail") {
		bInEpisode = false;
		UE_DEBUG_BREAK();
		return;
	}

	bInEpisode = true; // todo: fix, HandelEpisodeRequest both starts and stops, need unique functions 
	this->SendGetOcclusionLocationsRequest();
}

/* handle experiment service timeout */
void AExperimentServiceClient::HandleEpisodeRequestTimedOut() {
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::EpisodeTimedOut()] Episode request timed out!"))
}

void AExperimentServiceClient::HandleStartExperimentResponse(const FString ResponseIn)
{
	const FString msg = "[AExperimentServiceClient::HandleStartExperimentResponse] " + ResponseIn;
	printScreen(msg);
	
	/* convert to usable format */
	FStartExperimentResponse StartExperimentResponse = UExperimentUtils::JsonStringToStartExperimentResponse(*ResponseIn);
	ExperimentNameActive = StartExperimentResponse.experiment_name;
	
	if (ExperimentNameActive == "") {
		UE_DEBUG_BREAK();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleStartExperimentResponse] Experiment name: %s"), *ExperimentNameActive);
	
	bInExperiment = true;
	this->StartEpisode();
}

void AExperimentServiceClient::HandleStartExperimentTimedOut()
{
	printScreen("[AExperimentServiceClient::HandleStartExperimentTimedOut] Start Experiment TIMED OUT!");
}

/* create and send simple experiment service request */
bool AExperimentServiceClient::SendStartEpisodeRequest(const FString ExperimentNameIn, const FString header)
{
	if (!Client) { return false; }

	FStartEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameIn;
	
	FString request_string = UExperimentUtils::StartEpisodeRequestToJsonString(request_body);
	URequest* Request = Client->SendRequest("start_episode", request_string, 5.0f);
	
	if (!Request) { return false; }

	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestTimedOut);
	
	return true;
}

/* create and send simple experiment service request */
bool AExperimentServiceClient::SendStopEpisodeRequest(const FString& ExperimentNameIn, const FString& header)
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::SendStopEpisodeRequest] ExperimentNameIn: %s"), *ExperimentNameIn);
	if (!Client) { return false; }

	FFinishEpisodeRequest request_body;
	request_body.experiment_name = ExperimentNameIn;

	const FString request_string = UExperimentUtils::FinishEpisodeRequestToJsonString(request_body);
	URequest* request = Client->SendRequest(header, request_string, 5.0f);

	if (!request) { return false; }
	request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestResponse);
	request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleEpisodeRequestTimedOut);

	return true;
}

/* update predator ai's goal location using step message from tracking service */
void AExperimentServiceClient::UpdatePredator(const FMessage message){
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::UpdatePredator] message.body:\n%s"), *message.body);

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
void AExperimentServiceClient::UpdatePreyPosition(const FVector vector){
/*
	self.time_stamp = time_stamp 
	self.rotation = rotation 
*/
	FLocation Location = UExperimentUtils::VrToCanonical(vector, MapLength, WorldScale);

	/* prepare Step */
	//FString header_prey = "send_step";
	FStep send_step; 
	send_step.agent_name = "prey";
	send_step.frame = FrameCount;
	send_step.location = Location; 
	send_step.rotation = 0.0f; // todo: change! 
	send_step.time_stamp = FrameCount; // todo: change to querycounterelapsedtime!

	/* convert FStep to JsonString */
	FString body = UExperimentUtils::StepToJsonString(send_step); 

	/* send message to ES */
	FMessage message = UMessageClient::NewMessage("prey_step", body);
	UE_LOG(LogTemp, Log, TEXT("sending prey step: (route: %s; body: %s"),"prey_step",*message.body);
	if (Client->IsConnected()) {
		Client->SendMessage(message);
	}

	/* don't overload server with messages before its done processing previous prey update. */
	//bCanUpdatePreyPosition = false;
}

/* todo: finish this */
bool AExperimentServiceClient::IsExperimentActive(const FString ExperimentNameIn)
{
	return false;
}

/* destroy this actor. This is primarily used as an abort */
void AExperimentServiceClient::SelfDestruct(const FString InErrorMessage)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[AExperimentServiceClient::SelfDestruct] Tracking and Experiment ABORTED. Something happened: %s"), *InErrorMessage));
	UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::SelfDestruct()] Something went wrong. Destroying. Reason: %s "), *InErrorMessage);

	this->StopEpisode();
	// Make sure to check if the actor is valid and has not already been marked for destruction.
	if (!this->IsValidLowLevelFast()) { Client->Disconnect(); }
}

void AExperimentServiceClient::HandleMessagePredator(FMessage message)
{
}

void AExperimentServiceClient::HandleMessagePrey(FMessage message)
{
}

void AExperimentServiceClient::HandleMessage(FMessage message)
{
}

void AExperimentServiceClient::HandleUnroutedMessage(const FMessage message)
{
}

void AExperimentServiceClient::HandleTimedOut(FMessage message)
{
}

void AExperimentServiceClient::HandleResponse(const FString message)
{
}

/* get occlusions in our specific experiment (FWorldInfo.occlusions; default: "21_05") */
bool AExperimentServiceClient::SendGetOcclusionsRequest()
{
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("Cant send get occlusion request, Experiment service client not valid.")); return false; } // experiment service client not valid
	URequest* Request = Client->SendRequest("get_occlusions", "21_05", TimeOut);
	if (!Request) { return false; } // failed to send request
	FPlatformProcess::Sleep(0.5);
	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleGetOcclusionsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleGetOcclusionsTimedOut);

	return true;
}

void AExperimentServiceClient::SpawnOcclusions(const TArray<int32> OcclusionIDsIn, const TArray<FLocation> Locations) {
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceClient::HandleGetOcclusionsResponse(const FString ResponseIn) 
{
	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleGetOcclusionsResponse] Occlusion IDs (raw): %s"), *ResponseIn);
	
	/* start empty */
	// OcclusionIDsIntArr.Empty(); 
	TArray<int32> OcclusionIDsTemp; 
	/* process the array before using */
	TArray<FString> OcclusionIDsStringArr; 
	FString OcclusionIDs_temp = ResponseIn.Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT(""));
	int32 Samples = OcclusionIDs_temp.ParseIntoArray(OcclusionIDsStringArr, TEXT(","), true);

	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceClient::HandleGetOcclusionsResponse] Number of occlusions in configuration: %i"),Samples);

	/* convert to integer array */
	int32 SamplesLost = 0; 
	for (FString value : OcclusionIDsStringArr) {
		if (FCString::IsNumeric(*value)) OcclusionIDsTemp.Add(FCString::Atoi(*value));
		else SamplesLost++;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::HandleGetOcclusionsResponse] Number of occlusions lost druing AtoI: %i"),SamplesLost);
	// todo: pass this information to game state 
	// OcclusionsStruct.SetCurrentLocationsByIndex(OcclusionIDsTemp);
	// OcclusionsStruct.SpawnAll(GetWorld(), true, false, FVector(15.0f, 15.0f, 15.0f));
	// OcclusionsStruct.SetVisibilityArr(OcclusionIDsTemp);
	//OcclusionsStruct.SetAllLocations()
	//SpawnOcclusions(OcclusionIDsIntArr, OcclusionLocationsAll);
	return;
}

void AExperimentServiceClient::HandleGetOcclusionsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("Get occlussion request timed out!"));
	return;
}

bool AExperimentServiceClient::SetOcclusionVisibility(TArray<int32> VisibleOcclusionIDsIn)
{
	if (!bIsOcclusionLoaded) { UE_LOG(LogTemp, Fatal, TEXT("[AExperimentServiceClient::SetOcclusionVisibility] Failed to change visibility. Occlusions not pre-loaded.")); return false; }
	
	for (int i = 0; VisibleOcclusionIDsIn.Num(); i++) {
		
	}
	
	return true;
}

URequest* AExperimentServiceClient::SendGetOcclusionLocationsRequest()
{
	UE_LOG(LogTemp, Log, TEXT("[AExperimentServiceClient::SendGetOcclusionLocationsRequest()] Starting request."));
	if (!Client) { UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, Experiment service client not valid.")); return nullptr; }
	if (!bInEpisode){ UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::SendGetOcclusionLocationsRequest()] Cant send get occlusion request, bInEpisode is false.")); return nullptr; }
	const FString BodyOut   = "21_05";
	const FString HeaderOut = "get_cell_locations";
	URequest* Request = Client->SendRequest("get_cell_locations", "21_04", 10.0f);

	if (!Request) { return nullptr; }

	Request->ResponseReceived.AddDynamic(this, &AExperimentServiceClient::HandleGetOcclusionLocationsResponse);
	Request->TimedOut.AddDynamic(this, &AExperimentServiceClient::HandleGetOcclusionLocationsTimedOut);

	return Request;
}

/* gets location of all possible occlusions in our given experiment/world configuration */
void AExperimentServiceClient::HandleGetOcclusionLocationsResponse(const FString ResponseIn) {
	UE_LOG(LogTemp, Log, TEXT("[HandleGetOcclusionLocationsResponse] %s"), *ResponseIn);
	// OcclusionLocationsAll = UExperimentUtils::OcclusionsParseAllLocations(ResponseIn);
	//
	// OcclusionsStruct.SetAllLocations(OcclusionLocationsAll); 
	if (!this->SendGetOcclusionsRequest()) {
		UE_LOG(LogTemp, Log, TEXT("[SendGetOcclusionsRequest] send request false!"));
	}
	return;
}

void AExperimentServiceClient::HandleGetOcclusionLocationsTimedOut() {
	UE_LOG(LogTemp, Warning, TEXT("[HandleGetOcclusionLocationsTimedOut] Get cell location request timed out!"));
	return;
}

void AExperimentServiceClient::HandleOcclusionLocation(const FMessage MessageIn)
{
	UE_LOG(LogTemp, Warning, TEXT("OcclusionLocation: %s"), *MessageIn.body);
}

bool AExperimentServiceClient::test() {

	///* connect tracking service */
	Client = UMessageClient::NewMessageClient();
	if (!Client) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::test()] Client NULL."));
		return false;
	}

	if (!Client->Connect(ServerIPMessage,Port))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AExperimentServiceClient::TrackingServiceSetRoute()] Connecting Client FAILED."));
		return false;
	}

	if (!Client->IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::TrackingServiceSetRoute()] Connecting to Client FAILED!"));
		return false;
	}

	if (!Client->Subscribe())
	{
		UE_DEBUG_BREAK();
		UE_LOG(LogTemp, Error, TEXT("[AExperimentServiceClient::TrackingServiceSetRoute()] Connecting to Client FAILED!"));
		return false;
	}

	this->StartExperiment("family");
	return true;
}

/* main stuff happens here */
void AExperimentServiceClient::BeginPlay(){

	Super::BeginPlay();
	test(); 
}

/* run a (light!) command every frame */
void AExperimentServiceClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Client->IsConnected() || Client->IsConnected()) { return; }
}

void AExperimentServiceClient::EndPlay(const EEndPlayReason::Type EndPLayReason)
{
	Super::EndPlay(EndPLayReason);
	this->SelfDestruct("EndPlay");
}


