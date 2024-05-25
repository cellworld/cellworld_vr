#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentUtils.h"
#include "PredatorController/CharacterPredator.h"
//#include "PredatorController/ControllerTypes.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "../ExperimentComponents/ExperimentServiceClient.h"
#include "ExperimentServiceMonitor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);


UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor
{ 
	GENERATED_BODY()
	
public:	
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	AExperimentServiceMonitor();

	UMessageClient* TrackingServiceClient;
	UMessageClient* ExperimentServiceClient;
	
	UMessageRoute* ExperimentServiceClientRoute;
	UMessageRoute* ExperimentServicePreyRoute;

	UMessageRoute* MessageRoute; 
	UMessageRoute* TrackingServiceRoute; 
	UMessageRoute* TrackingServiceRoutePrey; 
	UMessageRoute* TrackingServiceRoutePredator; 

	URequest* start_episode_request;
	URequest* stop_episode_request;

	URequest* StartExperimentRequest;
	URequest* stop_experiment_request;

	URequest* StartEpisodeRequest;

	URequest* TrackingServiceRequest; 

	//const FString header_experiment_service			= "predator_step";
	const FString header_tracking_service			= "send_step";
	const FString header_tracking_service_prey		= "prey_step";
	const FString header_tracking_service_predator  = "predator_step";
	const FString experiment_name                   = "test_experiment";

	/* server stuff */
	//const FString ServerIPMessage    = "192.168.137.25"; // lab pc 
	const FString ServerIPMessage    = "127.0.0.1"; // localhost
	//const FString ServerIPMessage    = "129.105.90.64"; // lab pc - ethernet
	const int PortTrackingService    = 4510;
	const int PortExperimentService  = 4540; 
	bool bCanUpdatePreyPosition      = false;
	bool bConnectedTrackingService	 = false; 
	bool bConnectedExperimentService = false; 
	bool bConnectedToServer			 = false;

	/* world stuff */
	float map_length                = 5100;
	int frame_count                 = 0; 

	/* coordinate system stuff */
	const float MapLength = 235.185;
	int WorldScale        = 15; 

	/* setup */
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
	bool StopConnection(UMessageClient* Client);
	bool DisconnectAll();
	bool DisconnectClients(); 
	/* will be used in BP to called by door opening (start episode)*/
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();
	
		
	bool TrackingServiceCreateMessageClient();
	bool GetPlayerPawn();

	/* abort if anything goes wrong */
	void SelfDestruct(const FString InErrorMessage);
	
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

	/* Experiment */
	UFUNCTION()
		void HandleStartExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleStartExperimentTimedOut();
	UFUNCTION()
		URequest* SendStartExperimentRequest(const FString ExperimentNameIn);
	UFUNCTION()
		URequest* SendFinishExperimentRequest(const FString ExperimentNameIn);
	UFUNCTION()
		void HandleFinishExperimentResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleFinishExperimentTimedOut();
	
	/* update players */
	UFUNCTION()
		void UpdatePredator(const FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(const FVector Location);

	/* other */
	UFUNCTION()
		void HandleTrackingServiceMessage(const FMessage message);
	UFUNCTION()
		void HandleTrackingServiceUnroutedMessage(const FMessage message);
	UFUNCTION()
		void HandleExperimentServiceUnroutedMessage(const FMessage message);
	UFUNCTION()
		bool IsExperimentActive(const FString ExperimentNameIn);

	/* experiment control */
	UPROPERTY(BlueprintReadWrite)
		FString ExperimentNameActive;
	
	/* episode controll */
	UPROPERTY(BlueprintReadWrite)
		bool bInExperiment = false;
	UPROPERTY(BlueprintReadWrite)
		bool bInEpisode = false;

	/* occlusion control */
	FOcclusions OcclusionsStruct; 
	TArray<FLocation> OcclusionLocationsAll;
	TArray<int32> OcclusionIDsIntArr;
	TArray<AOcclusion*> OcclusionActorArr; 
	bool bIsOcclusionLoaded = false;
	bool SetOcclusionVisibility(TArray<int32> VisibleOcclusionIDsIn);

	UFUNCTION()
		URequest* SendGetOcclusionLocationsRequest();
	UFUNCTION()
		void HandleGetOcclusionLocationsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionLocationsTimedOut();
	UFUNCTION()
		bool SendGetOcclusionsRequest();
	UFUNCTION()
		void SpawnOcclusions(const TArray<int32> OcclusionIDsIn, const TArray<FLocation> Locations);
	UFUNCTION()
		void HandleGetOcclusionsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionsTimedOut();
	UFUNCTION()
		void HandleOcclusionLocation(const FMessage MessageIn);

		bool test();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const FString SubjectName = "vr_dude";
	const float TimeOut = 5.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPLayReason) override;

};