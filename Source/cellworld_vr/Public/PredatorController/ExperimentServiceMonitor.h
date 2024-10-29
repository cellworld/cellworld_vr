#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectPtr.h"
#include "Tools/GenericClock.h"
#include "MessageClient.h"
#include "ExperimentUtils.h"
#include "PawnMain.h"
#include "ExperimentComponents/ExperimentMonitorData.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "ExperimentComponents/Occlusion.h"
#include "DrawDebugHelpers.h"
#include "PredatorBasic.h"
#include "cellworld_vr/cellworld_vr.h"
#include "MiscUtils/Timer/EventTimer.h"
#include "ExperimentServiceMonitor.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProcessStopEpisodeResponseDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProcessStartEpisodeResponseDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperimentStatusChanged, EExperimentStatus, ExperimentStatusIn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCapture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimedOut, FString, InMessage); // check if we can pass by reference

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEpisodeStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpisodeStartedFailedDelegate, FString, InMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEpisodeStartedSuccessDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyEpisodeStarted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEpisodeFinishedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEpisodeFinishedSuccessDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEpisodeFinishedFailedDelegate, FString, InMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyEpisodeFinished);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubscribeResultDelegate, bool, bSubscribeResult); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResetResultDelegate, bool, bResetResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubscribeStatusChangedDelegate, bool, bResetResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyOnEpisodeStarted); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNotifyOnExperimentFinished, int, InPlayerIndex); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNotifyOnEpisodeFinished, bool, bEpisodeFinishedResult);

//todo: move this to its own file in ExperimentPlugin or BotEvadeModule
UCLASS(Blueprintable)
class  UExperimentManager : public UObject {
	GENERATED_BODY()
	UExperimentManager(): MaxEpisodeTotalTime(1800.0f) {
		OnEpisodeFinishedSuccessDelegate.AddDynamic(this, &ThisClass::OnEpisodeFinished);
		OnEpisodeStartedSuccessDelegate.AddDynamic(this , &ThisClass::OnEpisodeStarted);
		OnSubscribeStatusChangedDelegate.AddDynamic(this, &ThisClass::OnSubscribeStatusChanged);
		OnProcessStopEpisodeResponseDelegate.AddDynamic(this, &ThisClass::ProcessStopEpisodeResponse);
		OnProcessStartEpisodeResponseDelegate.AddDynamic(this, &ThisClass::ProcessStartEpisodeResponse);
		// OnEpisodeFinishedFailedDelegate.Add(this, &ThisClass::) // todo
		// OnEpisodeStartedFailedDelegate.Add(this, &ThisClass::) // todo
		OnCaptureDelegate.AddDynamic(this, &ThisClass::OnCapture);
	}
public:
	UPROPERTY(EditAnywhere)
	TArray<int> ActivePlayers = {};
	
	UPROPERTY(EditAnywhere)
	TArray<APawnMain*> ActivePlayersPawns = {};

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UExperimentMonitorData>> Data = {};

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStopwatch> Stopwatch;

	float MaxEpisodeTotalTime = 30.0f;

	UPROPERTY()
	int8 PlayerIndex = 0;
		
	/* experiment */
	FNotifyOnExperimentFinished NotifyOnExperimentFinishedDelegate;
	FOnTimedOut OnTimedOutDelegate;

	/* subscriptions */
	FSubscribeStatusChangedDelegate OnSubscribeStatusChangedDelegate;
	FOnSubscribeResultDelegate      OnSubscribeResultDelegate;

	/* on reset (temp) */
	FOnResetResultDelegate OnResetResultDelegate;
	
	/* episode started */
	FOnEpisodeStartedDelegate        OnEpisodeStartedDelegate;
	FOnEpisodeStartedFailedDelegate  OnEpisodeStartedFailedDelegate;
	FOnEpisodeStartedSuccessDelegate OnEpisodeStartedSuccessDelegate; 
	FNotifyOnEpisodeStarted          NotifyOnEpisodeStarted;
	FOnProcessStartEpisodeResponseDelegate OnProcessStartEpisodeResponseDelegate;
	
	FOnCapture OnCaptureDelegate;
	FNotifyEpisodeStarted  NotifyEpisodeStarted;
	FNotifyEpisodeFinished NotifyEpisodeFinished; 
	
	/* episode finished */
	FOnProcessStopEpisodeResponseDelegate OnProcessStopEpisodeResponseDelegate; 
	FOnEpisodeFinishedSuccessDelegate OnEpisodeFinishedSuccessDelegate;
	FOnEpisodeFinishedFailedDelegate OnEpisodeFinishedFailedDelegate;
	FNotifyOnEpisodeFinished NotifyOnEpisodeFinished;

	/* flag to see if we are subscribed to CellworldGame */
	UPROPERTY(EditAnywhere)
		bool bSubscribed = false; 
	UPROPERTY(EditAnywhere)
		bool bInEpisode  = false; 

	UFUNCTION()
	void OnStartEpisodeFailed() {
		UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::OnStartEpisodeFailed] Called"))
		if (!IsInEpisode()) {
			UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::OnStartEpisodeFailed] already in episode"))
			return;
		}
	}
	
	UFUNCTION()
	void OnFinishEpisodeFailed() {
		UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::OnFinishEpisodeFailed] Called"))
		if(!ensure(Stopwatch->IsValidLowLevelFast()) && Stopwatch->IsRunning()) { Stopwatch->Reset(); }
		
	}
	UFUNCTION()
	void OnSubscribeStatusChanged(const bool bSubscribeStatus) {
		bSubscribed = bSubscribeStatus;
		UE_LOG(LogExperiment, Log, TEXT("[OnSubscribeStatusChanged] bSubscribed = %i"),
			bSubscribed)
	}
	
	UFUNCTION()
	void SetActivePlayerIndex(const int InPlayerIndex) {
		UE_LOG(LogExperiment, Log,
			TEXT("[UExperimentManager::SetActivePlayerIndex] Setting new PlayerIndex. Old: %i; New: %i"),
			PlayerIndex, InPlayerIndex)
		PlayerIndex = InPlayerIndex; 
	}
	
	/* adds new player to ActivePlayer array.
	 * @return Index of new player. -1 if Index is already used or error. 
	 */
	UFUNCTION()
	int RegisterNewPlayer(APawnMain* InPawn, UExperimentMonitorData* InExperimentData) {
		if (!InPawn->IsValidLowLevelFast() || !InExperimentData->IsValidLowLevelFast()) {
			UE_LOG(LogExperiment, Error, TEXT("Failed to register player. APawn is not valid."))
			return -1;
		}
		
		const int NewPlayerIndex = Data.Add(InExperimentData);
		UE_LOG(LogExperiment, Warning, TEXT("RegisterNewPlayer(): Registered new player: Player %i"), NewPlayerIndex)
		return NewPlayerIndex;
	}

	UFUNCTION()
	bool IsPlayerRegistered(const int InPlayerIndex) const {
		if (!Data.IsValidIndex(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("[IsPlayerRegistered]  InPlayerIndex not registered/valid."))
			return false; 
		}
		return true; 
	}

	UFUNCTION()
	bool IsExperimentDone(const int InPlayerIndex) const {
		if (!IsPlayerRegistered(InPlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("Player %i: Can't determine if experiment is done. Player is not registered!"),
				InPlayerIndex)
			return false;
		}

		const float ElapsedTimeTotal = (Data)[InPlayerIndex]->GetEpisodesCompletedTime();
		if (ElapsedTimeTotal >= MaxEpisodeTotalTime) {
			UE_LOG(LogExperiment, Warning,
				TEXT("Player %i: Experiment completed! Target time reached! PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
			return true; 
		}
		
		UE_LOG(LogExperiment, Log, TEXT("Player %i: Experiment still not complete!  PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
		return false;
	}

	UFUNCTION()
	void OnCapture() {
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnCapture] Called"))

		
		// if done:
		if (!IsInEpisode()) {
			UE_LOG(LogExperiment, Error,TEXT("[UExperimentManager::OnCapture] Failed. not in episode"))
			OnEpisodeFinishedFailedDelegate.Broadcast("not in episode");
		}

		OnEpisodeFinishedSuccessDelegate.Broadcast();
	}
	
	UFUNCTION()
	bool CreateTimer() {
		Stopwatch = NewObject<UStopwatch>(this, UStopwatch::StaticClass());
		if (!Stopwatch) {
			UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::CreateTimer] Failed!"))
			return false; 
		}
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::CreateTimer] OK"))
		return true; 
	}

	UFUNCTION()
	bool StopTimerEpisode(double& InElapsedTime) {
		// if (!ensure(InElapsedTime)) {
		// 	UE_LOG(LogExperiment, Error,
		// 		TEXT("[UExperimentManager::StopTimerEpisode] Failed, InElapsedTime  is NULL."));
		// }
		
		if (!Stopwatch->IsValidLowLevel()) {
			UE_LOG(LogExperiment, Error,
				TEXT("[UExperimentManager::StopTimerEpisode] Failed, Stopwatch  is NULL."));
			return false;
		}

		if (!Stopwatch->IsRunning()) {
			UE_LOG(LogExperiment, Error,
				TEXT("[UExperimentManager::StopTimerEpisode] Failed, Stopwatch not running."));
			return false; 
		}

		InElapsedTime = Stopwatch->Lap();
		Stopwatch->Reset();
		UE_LOG(LogExperiment,Log,TEXT("[UExperimentManager::StopTimerEpisode] OK"));
		return true;
	}

	UFUNCTION()
	bool StartTimerEpisode() {
		if (Stopwatch->IsValidLowLevel() && Stopwatch->IsRunning()) {
			UE_LOG(LogExperiment, Warning, TEXT("[UExperimentManager::StartTimerEpisode] Can't start timer. bTimerRunning is True."))
			return false;
		}

		Stopwatch = NewObject<UStopwatch>(this, UStopwatch::StaticClass());

		if (!Stopwatch->IsValidLowLevel()) {
			UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::StartTimerEpisode()] UStopwatch is NULL!"))
			return false;
		}
		Stopwatch->Start(); 
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::StartTimerEpisode] Started timer OK!"))
		return true;
	}

	UFUNCTION()
	bool IsInEpisode() const { return bInEpisode; }

	UFUNCTION()
	void SetInEpisode(const bool bNewInEpisode) {
		UE_LOG(LogExperiment, Log,
			TEXT("[UExperimentStatus::SetInEpisode] Old: %i; New: %i"),
			bInEpisode, bNewInEpisode)
		bInEpisode = bNewInEpisode;
	}
	
	UFUNCTION()
	void OnEpisodeStarted() {
		UE_LOG(LogExperiment, Log, TEXT("IsInEpisode? %i"), IsInEpisode())
		check(!IsInEpisode())
		if(!ensure(this->StartTimerEpisode())) { return; }
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeStarted] setting: bInEpisode = true"))
		SetInEpisode(true);
		NotifyEpisodeStarted.Broadcast();
	}

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void ProcessStartEpisodeResponse() {
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Called"))
		if (IsInEpisode()) {
			UE_LOG(LogExperiment, Log,
				TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Broadcasting: OnEpisodeStartedFailedDelegate"))
			OnEpisodeStartedFailedDelegate.Broadcast("[ProcessStartEpisodeResponse] bInEpisode = true;");
		} else {
			UE_LOG(LogExperiment, Log,
				TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Broadcasting: OnEpisodeStartedSuccessDelegate"))
			OnEpisodeStartedSuccessDelegate.Broadcast();
		}
	}

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void ProcessStopEpisodeResponse() {
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::ProcessStopEpisodeResponse] ProcessStopEpisodeResponse"))
		if(!IsInEpisode()) {
			OnEpisodeFinishedFailedDelegate.Broadcast("[ProcessStopEpisodeResponse] ur not in an episode gangly");
			return;
		}
		UE_LOG(LogExperiment, Log,
			TEXT("[UExperimentManager::ProcessStopEpisodeResponse] broadcasting: OnEpisodeFinishedSuccessDelegate"))
		OnEpisodeFinishedSuccessDelegate.Broadcast(); // yipee
	}
	
	/* adds new player to ActivePlayer array.
	 * @param const int InPlayerIndex - Player index to modify 
	 * @param const int InEpisodeDuration - Duration of completed episode
	 * @return void. 
	 */
	// ReSharper disable once CppMemberFunctionMayBeConst
	UFUNCTION()
	void OnEpisodeFinished() {
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Called"))
		check(bInEpisode == true);
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeStarted] setting: bInEpisode = true"))
		SetInEpisode(false);
		NotifyEpisodeFinished.Broadcast();

		// check if player is in list
		if (!ensure(Data.IsValidIndex(PlayerIndex))) {
			UE_LOG(LogExperiment, Error,
				TEXT("[UExperimentManager::OnEpisodeFinished] Failed to modify ExperimentData. InPlayerIndex not valid."))
			return; 
		}

		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Player finished episode: %i"),
			PlayerIndex)
		
		double EpisodeDurationTemp = 0.0f; 
		if (!ensure(this->StopTimerEpisode(EpisodeDurationTemp))) {
			UE_LOG(LogExperiment, Error, TEXT("[UExperimentManager::OnEpisodeFinished] StopTimerEpisode FAILED"))
		}else {
			UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeFinished] StopTimerEpisode OK"))
		}
		
		Data[PlayerIndex]->AddEpisodeAndCompletedTime(EpisodeDurationTemp);
		
		UE_LOG(LogExperiment, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Added new episode to data! Player: %i, Duration: %0.3f. Total time: %0.2f."),
					PlayerIndex, EpisodeDurationTemp, Data[PlayerIndex]->GetEpisodesCompletedTime())
		
		// check if we finished everything and if so, notify delegates 
		if (IsExperimentDone(PlayerIndex)) {
			NotifyOnExperimentFinishedDelegate.Broadcast(PlayerIndex);
			UE_LOG(LogExperiment, Log,
				TEXT("[UExperimentManager::OnEpisodeFinished] Player (%i) finished experiment! Broadcasting to delegates (NotifyOnExperimentFinishedDelegate)."),
				PlayerIndex)
		}

	}
	
	/* Get FExperimentMonitorData by player index.
	 * @param (const int) InPlayerIndex - Player index to get 
	 * @return pointer to player's FExperimentMonitorData. 
	 */
	UExperimentMonitorData* GetDataByIndex() const {
		if (!Data.IsValidIndex(PlayerIndex)) {
			UE_LOG(LogExperiment, Error,
				TEXT("[GetDataByIndex] Failed to get data. Player index not valid."))
			return nullptr; 
		}
		return (Data.IsValidIndex(PlayerIndex)) ? (Data)[PlayerIndex] : nullptr;
	}

};

USTRUCT(Blueprintable)
struct FOcclusions {
	GENERATED_USTRUCT_BODY()
public:

	FOcclusions() { OcclusionAllArr = {}; }

	UPROPERTY(EditAnywhere)
		bool bAllLocationsLoaded = false; 
	UPROPERTY(EditAnywhere)
		bool bCurrentLocationsLoaded = false;
	UPROPERTY(EditAnywhere)
		bool bSpawnedAll = false;
	UPROPERTY(EditAnywhere)
		TArray<AOcclusion*> OcclusionAllArr {};
	UPROPERTY(EditAnywhere)
		TArray<AOcclusion*> CurrentVisibleArr {}; 
	UPROPERTY(EditAnywhere)
		TArray<FLocation> AllLocations {};
	UPROPERTY(EditAnywhere)
		TArray<int32> OcclusionIDsIntArr {};

	/* canonical */
	void SetAllLocations(const FString& LocationsIn) {
		// UE_LOG(LogExperiment,Warning,TEXT("[FOcclusions.SetAllLocations] Locations: %s"),*LocationsIn);
		AllLocations = UExperimentUtils::OcclusionsParseAllLocations(LocationsIn); 
		bAllLocationsLoaded = true;
	}

	// ReSharper disable once CppPassValueParameterByConstReference
	void SetCurrentLocationsByIndex(const TArray<int32> OcclusionIndexIn) {
		
		UE_LOG(LogExperiment,Log,
			TEXT("[FOcclusions::SetCurrentLocationsByIndex] Number of occlusions in configuration: %i"),
			OcclusionIndexIn.Num())
		OcclusionIDsIntArr = OcclusionIndexIn; 
		bCurrentLocationsLoaded = true;
	}

	bool SpawnAll(UWorld* WorldRefIn, const bool bHiddenInGameIn, const bool bEnableCollisonIn, const FVector WorldScaleVecIn) {

		if (AllLocations.Num() < 1) { UE_LOG(LogExperiment, Error, TEXT("FOcclusions.SpawnAll() Failed. OcclusionAllLocationsArr is empty."));  return false; }
		if (!WorldRefIn) { UE_LOG(LogExperiment, Error, TEXT("[FOcclusions.SpawnAll()] Failed due to invalid UWorld object.")); return false; }

		const FRotator Rotation(0.0f, 0.0f, 0.0f); // Desired spawn rotation
		const float ScaleOffset = 0.99157164105; 
		const float MapLength = 235.185290;
		
		FActorSpawnParameters SpawnParams;
		for (int i = 0; i < AllLocations.Num(); i++) {
			AOcclusion* SpawnOcclusion = WorldRefIn->SpawnActor<AOcclusion>(
				AOcclusion::StaticClass(),
				UExperimentUtils::CanonicalToVr(AllLocations[i], MapLength, WorldScaleVecIn.X*ScaleOffset), // todo: make scale dynamic
				Rotation,
				SpawnParams);
			SpawnOcclusion->SetActorScale3D(WorldScaleVecIn*ScaleOffset); 
			SpawnOcclusion->SetActorHiddenInGame(bHiddenInGameIn);
			SpawnOcclusion->SetActorEnableCollision(bEnableCollisonIn);
			OcclusionAllArr.Add(SpawnOcclusion);
		}
		UE_LOG(LogExperiment, Warning, TEXT("[FOcclusions.SpawnAll] OK."))
		bSpawnedAll = true;
		return true;
	}

	void SetAllHidden(){
		UE_LOG(LogExperiment, Log, TEXT("[FOcclusions.SetAllHidden()]"))
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(true);
			Occlusion->SetActorEnableCollision(false);
		}
	}

	void SetVisibilityAll(const bool bHiddenInGameIn) {
		for (AOcclusion* Occlusion : OcclusionAllArr) {
			Occlusion->SetActorHiddenInGame(bHiddenInGameIn);
			Occlusion->SetActorEnableCollision(false);
		}
	}

	/* set visibility and collisions given an array of occlusion index/IDs */
	void SetVisibilityArr(const TArray<int32> IndexArray, const bool bActorHiddenInGame, const bool bEnableCollision) {
		for (int i = 0; i < IndexArray.Num(); i++) {
			OcclusionAllArr[IndexArray[i]]->SetActorHiddenInGame(bActorHiddenInGame);
			OcclusionAllArr[IndexArray[i]]->SetActorEnableCollision(bEnableCollision);
		}
	}

	/* Set visibility of ALREADY SPAWNED OCCLUSIONS */
	void SetCurrentVisibility(const bool bVisibilityIn, const bool bEnableCollisonIn) {
		for (int i = 0; i < OcclusionIDsIntArr.Num(); i++) {
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorHiddenInGame(bVisibilityIn);
			OcclusionAllArr[OcclusionIDsIntArr[i]]->SetActorEnableCollision(bEnableCollisonIn);
			UE_LOG(LogTemp, Log, TEXT("SetCirrentVisibility (%i)"), i);
		}
	}
};

USTRUCT(Blueprintable)
struct FExperimentHeaders
{
	GENERATED_BODY()
	
	/* headers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString PreyStep     = "prey_step";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString PredatorStep = "predator_step";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentHeaders)
	FString OnCapture = "on_capture";
};

USTRUCT(Blueprintable)
struct FExperimentInfo
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		EExperimentStatus Status = EExperimentStatus::WaitingExperiment;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartExperimentRequest StartExperimentRequestBody;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartEpisodeRequest StartEpisodeRequestBody;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartExperimentResponse StartExperimentResponse;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FStartEpisodeResponse StartEpisodeResponse;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExperimentInfo)
		FString ExperimentNameActive = "";

	FOnExperimentStatusChanged OnExperimentStatusChangedEvent;
	
	void SetStatus(EExperimentStatus ExperimentStatusIn){
		UE_LOG(LogExperiment, Log, TEXT("[FExperimentInfo::SetStatus] New Status set: %i"), ExperimentStatusIn);
		Status = ExperimentStatusIn;
		OnExperimentStatusChangedEvent.Broadcast(ExperimentStatusIn);
	}
};

USTRUCT(Blueprintable)
struct FServerInfo {
	GENERATED_BODY()
public:
	FServerInfo() :
		Port(4791),
		IP(TEXT("192.168.1.199")) // main machine 
		{}
	
	int Port;
	FString IP;  // static vr backpack win11 PACKAGED ONLY
};

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