#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Tools/GenericClock.h"
#include "MessageClient.h"
#include "PawnMain.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentPlugin/DataManagers/ExperimentManager.h"
#include "DrawDebugHelpers.h"
#include "PredatorBasic.h"
#include "BotEvadeModule/Public/Client/ExperimentClient.h"
#include "MiscUtils/Timers/EventTimer.h"
#include "ExperimentServiceMonitor.generated.h"

UCLASS()
class CELLWORLD_VR_API AExperimentServiceMonitor : public AActor { 
	GENERATED_BODY()
	
public:	
	AExperimentServiceMonitor();

	UPROPERTY(EditAnywhere, Blueprintable)
	FServerInfo ServerInfo = FServerInfo();
	
	/* ==== server stuff ==== */
	// const FString ServerIP         = "192.168.1.199";  // static vr backpack win11 PACKAGED ONLY
	// const int TrackingPort	           = 4791;
	
	/* DEBUG */
	bool bTimerRunning = false;
	GenericClock::FStopWatch StopWatch;
	const float TimerFrequency = 0.5; 
	float TimeElapsedTick = 0.0f;
	uint32 FrameCountPrey = 0;
	uint32 FrameCountPredator = 0;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStopwatch> Stopwatch;

	UPROPERTY()
	TObjectPtr<UExperimentManager> ExperimentManager;

	/* called when the whole experiment is completed */
	// FNotifyOnExperimentFinished NotifyOnExperimentFinishedDelegate;
	
	UFUNCTION()
	void OnExperimentFinished(const int InPlayerIndex);
	
	FTimerHandle TimerHandle;
	FTimerHandle* TimerHandlePtr = &TimerHandle;
	FTimerManager TimerManager;
	
	/* ==== main experiment service components ==== */
	UPROPERTY()
		TObjectPtr<UMessageClient> Client;
	UPROPERTY()
		TObjectPtr<UMessageClient> TrackingClient;

	/* Message Routes */
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRouteOnCapture;
	UPROPERTY()
		TObjectPtr<UMessageRoute> MessageRoutePredator;

	/* Requests */
	UPROPERTY()
		TObjectPtr<URequest> StartExperimentRequest;
	UPROPERTY()
		TObjectPtr<URequest> StartEpisodeRequest;
	UPROPERTY()
		TObjectPtr<URequest> StopExperimentRequest;
	UPROPERTY()
		TObjectPtr<URequest> StopEpisodeRequest;
	UPROPERTY()
		TObjectPtr<URequest> SubscribeRequest;
	UPROPERTY()
		TObjectPtr<URequest> TrackingSubscribeRequest;
	UPROPERTY()
		TObjectPtr<URequest> GetOcclusionsRequest;
	UPROPERTY()
		TObjectPtr<URequest> GetOcclusionLocationRequest;
	UPROPERTY()
		TObjectPtr<URequest> ResetRequest;

	/* ==== status stuff ==== */
	UPROPERTY()
		FExperimentInfo ExperimentInfo {};
	UPROPERTY()
		FExperimentHeaders ExperimentHeaders; 
	
	bool bCanUpdatePreyPosition      = false;
	bool bConnectedTrackingService	 = false; 
	bool bConnectedExperimentService = false; 
	bool bConnectedToServer			 = false;

	/* ==== world stuff ==== */
	int FrameCount        = 0; // todo: will probably delete 
	const float MapLength = 235.185;
	const float PredatorScaleFactor = 0.5f; 
	float WorldScale      = 1.0f;
	
	/* experiment params */
	const float PositionSamplingRate = 90.0f; 
		
	/* ==== setup ==== */
	bool SpawnAndPossessPredator();
	UPROPERTY()
		TObjectPtr<APredatorBasic> PredatorBasic = nullptr;

	/* functions called by GameMode and Blueprints */
	static UMessageClient* CreateNewClient();
	static bool ValidateClient(UMessageClient* ClientIn);
	static bool ValidateExperimentName(const FString& ExperimentNameIn);
	static bool Disconnect(UMessageClient* ClientIn);
	
	bool StopExperiment(const FString& ExperimentNameIn);
	
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StartEpisode();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool StopEpisode(const bool bForce = false);
	UPROPERTY(EditAnywhere)
		TObjectPtr<APawnMain> PlayerPawnActive = nullptr;
	UPROPERTY(EditAnywhere)
		TObjectPtr<APawnMain> PlayerPawn = nullptr;
	UPROPERTY(EditAnywhere)
		int PlayerIndex = -1; 

	/* ==== helper functions  ==== */
	bool ValidateLevel(UWorld* InWorld, const FString InLevelName);
	bool SetupPlayerUpdatePosition();
	
	void SelfDestruct(const FString InErrorMessage);
	
	/* ==== delegates ==== */
	UPROPERTY()
		FOnExperimentStatusChanged OnExperimentStatusChangedEvent;

	/* Called when status is changed. */
	UFUNCTION()
		void OnStatusChanged(const EExperimentStatus ExperimentStatusIn);

	UFUNCTION(BlueprintCallable, Category = Experiment)
		bool SubscribeToTracking();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingResponse(FString Response);
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void HandleSubscribeToTrackingTimedOut();
	UFUNCTION(BlueprintCallable, Category = Experiment)
		void RequestRemoveDelegates(URequest* InRequest, const FString& InRequestString = "");
	
	/* update predator stuff */
	UFUNCTION()
		void HandleUpdatePredator(const FMessage MessageIn);
	UFUNCTION()
		void HandleOnCapture(const FMessage MessageIn);
	UFUNCTION()
		float GetTimeRemaining() const;
 
	/* experiment service */
	UFUNCTION()
		void HandleStartEpisodeRequestResponse(const FString response);
	UFUNCTION()
		bool ResetTrackingAgent();
	UFUNCTION()
		void HandleStartEpisodeError(const FString InMessage = "");
	UFUNCTION()
		void HandleResetRequestResponse(const FString InResponse);
	UFUNCTION()
		void HandleResetRequestTimedOut();
	// will force a full system restart 
	UFUNCTION()
		void OnTimedOut(const FString InMessage);
	UFUNCTION()
		void OnSubscribeResult(bool bSubscribeResult);
	UFUNCTION()
		void OnResetResult(bool bResetResult);
	UFUNCTION()
		void HandleStartEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeRequestResponse(const FString response);
	UFUNCTION()
		void HandleStopEpisodeRequestTimedOut();
	UFUNCTION()
		void HandleStopEpisodeError(const FString InMessage = ""); 
	UFUNCTION()
		void SetPredatorIsVisible(bool bNewVisibility);

	/* gets location of all possible occlusions in cellworld  */
	UFUNCTION()
		bool SendGetOcclusionLocationsRequest();
	UFUNCTION()
		void HandleGetOcclusionLocationsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionLocationsTimedOut();
	UFUNCTION()
		bool SendGetOcclusionsRequest();
	UFUNCTION()
		void HandleGetOcclusionsResponse(const FString ResponseIn);
	UFUNCTION()
		void HandleGetOcclusionsTimedOut();
	UFUNCTION()
		void HandleUnroutedMessage(const FMessage InMessage);
	
	/* update agents */
	UPROPERTY(EditAnywhere)
		bool bCanUpdatePrey = false;
	UFUNCTION()
		void UpdatePredator(const FMessage& InMessage);
	UFUNCTION()
		void UpdatePreyPosition(const FVector InVector, const FRotator InRotation);

	/* experiment control */
	UPROPERTY(BlueprintReadWrite)
		FString ExperimentNameActive;
	
	/* occlusion control */
	FOcclusions OcclusionsStruct; 
	TArray<FLocation> OcclusionLocationsAll;
	TArray<int32> OcclusionIDsIntArr;
	bool bIsOcclusionLoaded = false;

	UFUNCTION()
		static bool ConnectToServer(UMessageClient* ClientIn, const int MaxAttemptsIn, const FString& IPAddressIn, const int PortIn);
	UFUNCTION()
		bool RoutePredatorMessages();
	UFUNCTION()
		bool RouteOnCapture();
	bool SetupConnections();
	UFUNCTION()
		void OnEpisodeStarted();
	UFUNCTION()
		void ResetWorldState();
	UFUNCTION()
		void OnEpisodeFinished();
	UFUNCTION(BlueprintCallable)
		void SetOcclusionVisibility(const bool bNewVisibility);
protected:
	bool Test();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const FString SubjectName = "vr_dude";
	const float TimeOut = 5.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
};