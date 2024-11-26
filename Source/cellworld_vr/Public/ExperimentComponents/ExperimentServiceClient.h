// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MessageClient.h"
#include "PredatorController/CharacterPredator.h"
#include "ExperimentPlugin/Occlusions/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentServiceClient.generated.h"

UCLASS()
class CELLWORLD_VR_API AExperimentServiceClient : public AActor
{
	GENERATED_BODY()
	
public:
	void printScreen(FString InMessage);
	// Sets default values for this actor's properties
	AExperimentServiceClient();
	UMessageClient* Client		 = nullptr;
	UMessageRoute* RoutePrey	 = nullptr; 
	UMessageRoute* RoutePredator = nullptr; 
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* players */
	ACharacterPredator* CharacterPredator = nullptr;

	/* experiment parameters */
	const FString experiment_name = "test_experiment";
	const float TimeOut = 5.0f;


	/* server parameters */
	const FString ServerIPMessage    = "127.0.0.1"; // localhost
	const int Port  = 4566; 
	bool bCanUpdatePreyPosition      = false;
	bool bConnectedTrackingService	 = false; 
	bool bConnectedExperimentService = false; 
	bool bConnectedToServer			 = false;

	/* world and coordinate system stuff */
	const float MapLength = 235.185;
	int WorldScale;
	int FrameCount        = 0; 

	/* setup */
	const FString predator_step_header = "predator_step";

	bool StartExperiment(const FString ExperimentNameIn);
	bool StopExperiment(const FString ExperimentNameIn);
	bool Disconnect();

	/* REQUESTS METHODS */
	
	/* will be used in BP to called by door opening (start episode)*/
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode();
		
	/* abort if anything goes wrong */
	void SelfDestruct(const FString InErrorMessage);
	
	/* ==== delegates ==== */

	/* update players */
	UFUNCTION()
		void UpdatePredator(const FMessage message);
	UFUNCTION()
		void UpdatePreyPosition(const FVector Location);

	/* predator */
	UFUNCTION()
		void HandleMessagePredator(FMessage message);
	UFUNCTION()
		void HandleMessagePrey(FMessage message);
	/* prey */
	UFUNCTION()
		void HandleTimedOut(FMessage message);
	UFUNCTION()
		void HandleResponse(const FString message);

	/* general messages */
	UFUNCTION()
		void HandleMessage(FMessage message);
	UFUNCTION()
		void HandleUnroutedMessage(const FMessage message);

	/* Episode */
	UFUNCTION()
		bool SendStartEpisodeRequest(const FString ExperimentNameIn, const FString header);
	UFUNCTION()
		bool SendStopEpisodeRequest(const FString& ExperimentNameIn, const FString& header);
	UFUNCTION()
		void HandleEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleEpisodeRequestTimedOut();

	/* Experiment flow */
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
	// FOcclusions OcclusionsStruct; 
	// TArray<FLocation> OcclusionLocationsAll;
	// TArray<int32> OcclusionIDsIntArr;
	// TArray<AOcclusion*> OcclusionActorArr; 
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPLayReason) override;


};
