// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ExperimentPlugin/Client/ExperimentClient.h"
#include "ExperimentPlugin/GameModes/ExperimentGameMode.h"
#include "BotEvadeGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBotEvadeGameMode, Log, All);

/**
 * 
 */
UCLASS()
class BOTEVADEMODULE_API ABotEvadeGameMode : public AExperimentGameMode {
	GENERATED_BODY()

public:
	ABotEvadeGameMode();

	/* base functions */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void SpawnExperimentServiceMonitor() override;
	virtual void OnUpdatePreyPosition(const FVector& InLocation, const FRotator& InRotation) override;
	virtual void HandleUpdatePosition(FVector InLocation, FRotator InRotation) override;

	virtual bool StartPositionSamplingTimer(float InRateHz) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;

private:
	TArray<class APlayerStart*> FreePlayerStarts;
};
