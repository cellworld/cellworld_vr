// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperimentPlugin.h"
#include "MessageClient.h"
#include "TCPMessages.h"
#include "ExperimentUtils.h"
#include "PredatorController/CharacterPredator.h"
// #include "PredatorController/ControllerTypes.h"
#include "PawnMain.h"
#include "PawnDebug.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentServiceClient.generated.h"

// USTRUCT()
// struct FOcclusions{
// 	GENERATED_USTRUCT_BODY()
// public:
//
// 	FOcclusions() { OcclusionAllArr = {}; }
//
// 	bool bLocationsLoaded = false; 
// 	bool bCurrentLocationsLoaded = false; 
// 	TArray<AOcclusion*> OcclusionAllArr;
// 	TArray<AOcclusion*> CurrentVisibleArr; 
//
// 	TArray<FLocation> OcclusionAllLocationsArr;
// 	TArray<int32> OcclusionIDsIntArr;
//
// 	/* canonical */
// 	void SetAllLocations(const TArray<FLocation> LocationsIn) {
// 		OcclusionAllLocationsArr = LocationsIn; 
// 		bLocationsLoaded = true;
// 	}
//
// 	void SetCurrentLocationsByIndex(TArray<int32> OcclusionIndexIn) {
// 		OcclusionIDsIntArr = OcclusionIndexIn; 
// 		bCurrentLocationsLoaded = true;
// 	}
//
// 	bool SpawnAll(UWorld* WorldRefIn, const bool bVisibilityIn, const bool bEnableCollisonIn, const FVector WorldScaleVecIn) {
// 		if (OcclusionAllLocationsArr.Num() < 1) { UE_LOG(LogTemp, Error, TEXT("FOcclusions.SpawnAll() Failed. OcclusionAllLocationsArr is empty."));  return false; }
// 		if (!WorldRefIn) { UE_LOG(LogTemp, Error, TEXT("[FOcclusions.SpawnAll()] Failed due to invalid UWorld object.")); return false; }
// 		FRotator Rotation(0.0f, 0.0f, 0.0f); // Desired spawn rotation
// 		const float ScaleOffset = 0.99157164105; 
// 		const float MapLength = 235.185290;
// 		FActorSpawnParameters SpawnParams;
// 		for (int i = 0; i < OcclusionAllLocationsArr.Num(); i++) {
// 			AOcclusion* MyMeshActor = WorldRefIn->SpawnActor<AOcclusion>(
// 				AOcclusion::StaticClass(),
// 				UExperimentUtils::CanonicalToVr(OcclusionAllLocationsArr[i], MapLength, 15*ScaleOffset), // todo: make scale dynamic
// 				Rotation,
// 				SpawnParams);
// 			 
// 			MyMeshActor->SetActorScale3D(FVector(15* ScaleOffset,15*ScaleOffset, 15)); // todo: change to WorldScaleVecIn
// 			MyMeshActor->SetActorHiddenInGame(bVisibilityIn);
// 			MyMeshActor->SetActorEnableCollision(bEnableCollisonIn);
//
// 			OcclusionAllArr.Add(MyMeshActor);
// 			UE_LOG(LogTemp, Warning, TEXT("Spawned occlusion (%i/%i)."), i, 50);
// 		}
//
// 		return true;
// 	}
//
// 	void SetVisibilityAll(const bool bVisibilityIn) {
// 		for (AOcclusion* Occlusion : OcclusionAllArr) {
// 			Occlusion->SetActorHiddenInGame(true);
// 			Occlusion->SetActorEnableCollision(false);
// 		}
// 	}
//
// 	void SetVisibilityArr(const TArray<int32> IndexArray) {
// 		for (int i = 0; i < IndexArray.Num(); i++) {
// 			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(false);
// 			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(true);
// 			UE_LOG(LogTemp, Log, TEXT("SetVisibilityArr (%i)"), i);
// 		}
// 	}
//
// 	void SetCurrentVisibility(const bool bVisibilityIn, const bool bEnableCollisonIn) {
// 		for (int i = 0; i < OcclusionIDsIntArr.Num(); i++) {
// 			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorHiddenInGame(bVisibilityIn);
// 			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorEnableCollision(bEnableCollisonIn);
// 			UE_LOG(LogTemp, Log, TEXT("SetCirrentVisibility (%i)"), i);
// 		}
// 	}
// };

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
