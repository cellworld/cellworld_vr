// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "PredatorController/CharacterPredator.h"
#include "PredatorController/ControllerTypes.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperimentServiceMonitor();

	UMessageClient* TrackingServiceClient;
	UMessageClient* ExperimentServiceClient;
	
	UMessageRoute* ExperimentServiceClientRoute;
	UMessageRoute* ExperimentServiceRoute;

	UMessageRoute* MessageRoute; 
	UMessageRoute* TrackingServiceRoute; 
	UMessageRoute* TrackingServiceRoutePrey; 
	UMessageRoute* TrackingServiceRoutePredator; 

	URequest* start_episode_request;
	URequest* stop_episode_request;

	URequest* start_experiment_request;
	URequest* stop_experiment_request;

	URequest* TrackingServiceRequest; 

	const FString header_experiment_service			= "predator_step";
	const FString header_tracking_service			= "send_step";
	const FString header_tracking_service_prey		= "prey_step";
	const FString header_tracking_service_predator  = "predator_step";
	const FString experiment_name = "test_experiment";

	const FString ServerIPMessage   = "127.0.0.1";
	const int PortTrackingService   = 4510;
	const int PortExperimentService = 4540; 
	float map_length                = 5100;
	int frame_count                 = 0; 

	bool bConnectedToServer     = false;
	bool bCanUpdatePreyPosition = false;

	/* connection control */
	bool bConnectedToExperimentService  = false; 
	bool bCOnnectedToTrackingService    = false; 
	
	/* subscription control */
	bool bSubscribedToTrackingService   = false;
	bool bRoutedMessagesTrackingService = false; 

	/* episode controll */
	bool bInExperiment  = false;
	bool bInEpisode		= false; 

	/* setu[ */
	const FString predator_step_header = "predator_step";
	bool SubscribeToTrackingService();
	bool ConnectToTrackingService(); 
	bool TrackingServiceRouteMessages(); 

	bool SubscribeToExperimentServiceServer(FString header);

	ACharacterPredator* CharacterPredator;
	bool SpawnAndPossessPredator();

	//bool SubscribeToExperimentService(FString header);
	bool StartExperiment(const FString ExperimentNameIn);
	bool StopExperiment(const FString ExperimentNameIn);
	bool StartEpisode(const FString ExperimentNameIn);
	bool StopEpisode(const FString ExperimentNameIn);
	bool StopConnection(UMessageClient* Client);
	
	bool TrackingServiceCreateMessageClient();

	bool ConnectTrackingService();
	
	bool GetPlayerPawn();

	/* abort if anything goes wrong */
	void SelfDestruct(const FString ErrorMessageIn);
	
	/* ==== delegates ==== */

	/* experiment service */
	UFUNCTION()
		void HandleTrackingServiceMessagePredator(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceMessagePrey(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceMessage(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceMessageTimedOut(FMessage message);
	UFUNCTION()
		void HandleExperimentServiceResponse(const FString message);
	UFUNCTION()
		void HandleExperimentServiceResponseTimedOut();
	/* tracking service */
	UFUNCTION()
		void HandleTrackingServiceResponse(const FString message);
	UFUNCTION()
		void HandleTrackingServiceResponseTimedOut();
	/* Episode */
	UFUNCTION()
		URequest* SendStartEpisodeRequest(const FString ExperimentNameIn, const FString header);
	UFUNCTION()
		URequest* SendStopEpisodeRequest(const FString ExperimentNameIn, const FString header);
	UFUNCTION()
		void HandleEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStartExperimentResponse(const FString ResponseIn);

	UFUNCTION()
		void HandleStartExperimentTimedOut();
	UFUNCTION()
		void UpdatePredator(FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(FVector Location);
	UFUNCTION()
		void HandleTrackingServiceMessage(FMessage message);
	UFUNCTION()
		void HandleTrackingServiceUnroutedMessage(FMessage message);



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};