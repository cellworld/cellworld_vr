#pragma once
#include "CoreMinimal.h"
#include "ExperimentPlugin/DataManagers/ExperimentData.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProcessStopEpisodeResponseDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProcessStartEpisodeResponseDelegate);

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

UCLASS(Blueprintable)
class EXPERIMENTPLUGIN_API UExperimentManager : public UObject {
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
	TArray<AExperimentPawn*> ActivePlayersPawns = {};

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UExperimentData>> Data = {};

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
		UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::OnStartEpisodeFailed] Called"))
		if (!IsInEpisode()) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::OnStartEpisodeFailed] already in episode"))
			return;
		}
	}
	
	UFUNCTION()
	void OnFinishEpisodeFailed() {
		UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::OnFinishEpisodeFailed] Called"))
		if(!ensure(Stopwatch->IsValidLowLevelFast()) && Stopwatch->IsRunning()) { Stopwatch->Reset(); }
		
	}
	UFUNCTION()
	void OnSubscribeStatusChanged(const bool bSubscribeStatus) {
		bSubscribed = bSubscribeStatus;
		UE_LOG(LogTemp, Log, TEXT("[OnSubscribeStatusChanged] bSubscribed = %i"),
			bSubscribed)
	}
	
	UFUNCTION()
	void SetActivePlayerIndex(const int InPlayerIndex) {
		UE_LOG(LogTemp, Log,
			TEXT("[UExperimentManager::SetActivePlayerIndex] Setting new PlayerIndex. Old: %i; New: %i"),
			PlayerIndex, InPlayerIndex)
		PlayerIndex = InPlayerIndex; 
	}
	
	/* adds new player to ActivePlayer array.
	 * @return Index of new player. -1 if Index is already used or error. 
	 */
	UFUNCTION()
	int RegisterNewPlayer(AController* InController) {
		// todo: make sure player is not already registered/duplicate
		UExperimentData* NewExperimentMonitorData = NewObject<UExperimentData>(this);
		if (!InController->IsValidLowLevel()) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::RegisterNewPlayer] Failed to register player. InController is NULL"))
			return -1;
		}
		if (!NewExperimentMonitorData->IsValidLowLevelFast()) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::RegisterNewPlayer] Failed to register player. NewExperimentMonitorData NULL"))
			return -1;
		}
				
		const int NewPlayerIndex = Data.Add(NewExperimentMonitorData); // todo: make sure its unique
		UE_LOG(LogTemp, Warning, TEXT("[UExperimentManager::RegisterNewPlayer] Registered new player: Player %i"), NewPlayerIndex)
		return NewPlayerIndex;
	}

	UFUNCTION()
	bool IsPlayerRegistered(const int InPlayerIndex) const {
		if (!Data.IsValidIndex(InPlayerIndex)) {
			UE_LOG(LogTemp, Error,
				TEXT("[UExperimentManager::RegisterNewPlayer] InPlayerIndex not registered/valid."))
			return false; 
		}
		return true; 
	}

	UFUNCTION()
	bool IsExperimentDone(const int InPlayerIndex) const {
		if (!IsPlayerRegistered(InPlayerIndex)) {
			UE_LOG(LogTemp, Error,
				TEXT("[UExperimentManager::RegisterNewPlayer] Player %i: Can't determine if experiment is done. Player is not registered!"),
				InPlayerIndex)
			return false;
		}

		const float ElapsedTimeTotal = (Data)[InPlayerIndex]->GetEpisodesCompletedTime();
		if (ElapsedTimeTotal >= MaxEpisodeTotalTime) {
			UE_LOG(LogTemp, Warning,
				TEXT("Player %i: Experiment completed! Target time reached! PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
			return true; 
		}
		
		UE_LOG(LogTemp, Log, TEXT("Player %i: Experiment still not complete!  PlayerTime: %0.2f; Target: %0.2f"),
				InPlayerIndex, ElapsedTimeTotal, MaxEpisodeTotalTime)
		return false;
	}

	UFUNCTION()
	void OnCapture() {
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnCapture] Called"))

		
		// if done:
		if (!IsInEpisode()) {
			UE_LOG(LogTemp, Error,TEXT("[UExperimentManager::OnCapture] Failed. not in episode"))
			OnEpisodeFinishedFailedDelegate.Broadcast("not in episode");
		}

		OnEpisodeFinishedSuccessDelegate.Broadcast();
	}
	
	UFUNCTION()
	bool CreateTimer() {
		Stopwatch = NewObject<UStopwatch>(this, UStopwatch::StaticClass());
		if (!Stopwatch) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::CreateTimer] Failed!"))
			return false; 
		}
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::CreateTimer] OK"))
		return true; 
	}

	UFUNCTION()
	bool StopTimerEpisode(double& InElapsedTime) {
		UE_LOG(LogTemp, Log,
				TEXT("[UExperimentManager::StopTimerEpisode] Called"));
		// if (!ensure(InElapsedTime)) {
		// 	UE_LOG(LogTemp, Error,
		// 		TEXT("[UExperimentManager::StopTimerEpisode] Failed, InElapsedTime  is NULL."));
		// }
		
		if (!Stopwatch->IsValidLowLevel()) {
			UE_LOG(LogTemp, Error,
				TEXT("[UExperimentManager::StopTimerEpisode] Failed, Stopwatch  is NULL."));
			return false;
		}

		if (!Stopwatch->IsRunning()) {
			UE_LOG(LogTemp, Error,
				TEXT("[UExperimentManager::StopTimerEpisode] Failed, Stopwatch not running."));
			return false; 
		}

		InElapsedTime = Stopwatch->Lap();
		Stopwatch->Reset();
		UE_LOG(LogTemp,Log,TEXT("[UExperimentManager::StopTimerEpisode] OK"));
		return true;
	}

	UFUNCTION()
	bool StartTimerEpisode() {
		if (Stopwatch->IsValidLowLevel() && Stopwatch->IsRunning()) {
			UE_LOG(LogTemp, Warning, TEXT("[UExperimentManager::StartTimerEpisode] Can't start timer. bTimerRunning is True."))
			return false;
		}

		Stopwatch = NewObject<UStopwatch>(this, UStopwatch::StaticClass());

		if (!Stopwatch->IsValidLowLevel()) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::StartTimerEpisode()] UStopwatch is NULL!"))
			return false;
		}
		Stopwatch->Start(); 
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::StartTimerEpisode] Started timer OK!"))
		return true;
	}

	UFUNCTION()
	bool IsInEpisode() const { return bInEpisode; }

	UFUNCTION()
	void SetInEpisode(const bool bNewInEpisode) {
		UE_LOG(LogTemp, Log,
			TEXT("[UExperimentStatus::SetInEpisode] Old: %i; New: %i"),
			bInEpisode, bNewInEpisode)
		bInEpisode = bNewInEpisode;
	}
	
	UFUNCTION()
	void OnEpisodeStarted() {
		UE_LOG(LogTemp, Log, TEXT("IsInEpisode? %i"), IsInEpisode())
		check(!IsInEpisode())
		if(!ensure(this->StartTimerEpisode())) { return; }
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeStarted] setting: bInEpisode = true"))
		SetInEpisode(true);
		NotifyEpisodeStarted.Broadcast();
	}

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void ProcessStartEpisodeResponse() {
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Called"))
		if (IsInEpisode()) {
			UE_LOG(LogTemp, Log,
				TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Broadcasting: OnEpisodeStartedFailedDelegate"))
			OnEpisodeStartedFailedDelegate.Broadcast("[ProcessStartEpisodeResponse] OnEpisodeStartedFailedDelegate bInEpisode = true;");
		} else {
			UE_LOG(LogTemp, Log,
				TEXT("[UExperimentManager::ProcessStartEpisodeResponse] Broadcasting: OnEpisodeStartedSuccessDelegate"))
			OnEpisodeStartedSuccessDelegate.Broadcast();
		}
	}

	UFUNCTION()
	// ReSharper disable once CppMemberFunctionMayBeConst
	void ProcessStopEpisodeResponse() {
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::ProcessStopEpisodeResponse] ProcessStopEpisodeResponse"))
		if(!IsInEpisode()) {
			OnEpisodeFinishedFailedDelegate.Broadcast("[ProcessStopEpisodeResponse] ur not in an episode gangly");
			return;
		}
		UE_LOG(LogTemp, Log,
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
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Called"))
		check(bInEpisode == true);
		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] setting: bInEpisode = false"))
		SetInEpisode(false);
		NotifyEpisodeFinished.Broadcast();
		
		double EpisodeDurationTemp = 0.0f;
		if (!ensure(this->StopTimerEpisode(EpisodeDurationTemp))) {
			UE_LOG(LogTemp, Error, TEXT("[UExperimentManager::OnEpisodeFinished] StopTimerEpisode FAILED"))
		}else {
			UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] StopTimerEpisode OK"))
		}

		// check if player is in list
		if (ensure(Data.IsValidIndex(PlayerIndex))) {
			UE_LOG(LogTemp, Error,
				TEXT("[UExperimentManager::OnEpisodeFinished] Failed to modify ExperimentData. InPlayerIndex not valid."))
			UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Player finished episode: %i"),
				PlayerIndex)
			Data[PlayerIndex]->AddEpisodeAndCompletedTime(EpisodeDurationTemp);
			UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] Added new episode to data! Player: %i, Duration: %0.3f. Total time: %0.2f."),
				PlayerIndex, EpisodeDurationTemp, Data[PlayerIndex]->GetEpisodesCompletedTime())

			if (IsExperimentDone(PlayerIndex)) {
				NotifyOnExperimentFinishedDelegate.Broadcast(PlayerIndex);
				UE_LOG(LogTemp, Log,
					TEXT("[UExperimentManager::OnEpisodeFinished] Player (%i) finished experiment! Broadcasting to delegates (NotifyOnExperimentFinishedDelegate)."),
					PlayerIndex)
			}
			return; 
		}

		UE_LOG(LogTemp, Log, TEXT("[UExperimentManager::OnEpisodeFinished] TEMP - Player finished episode: %i"),
			PlayerIndex)
		
		// check if we finished everything and if so, notify delegates 

	}
	
	/* Get FExperimentMonitorData by player index.
	 * @param (const int) InPlayerIndex - Player index to get 
	 * @return pointer to player's FExperimentMonitorData. 
	 */
	TObjectPtr<UExperimentData> GetDataByIndex() const {
		if (!Data.IsValidIndex(PlayerIndex)) {
			UE_LOG(LogTemp, Error,
				TEXT("[GetDataByIndex] Failed to get data. Player index not valid."))
			return nullptr; 
		}
		return (Data.IsValidIndex(PlayerIndex)) ? (Data)[PlayerIndex] : nullptr;
	}

};