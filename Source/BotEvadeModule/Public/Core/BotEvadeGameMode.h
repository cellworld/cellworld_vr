// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Client/ExperimentClient.h"
#include "BotEvadeGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBotEvadeGameMode, Log, All);

/**
 * 
 */
UCLASS()
class BOTEVADEMODULE_API ABotEvadeGameMode : public AGameMode {
	GENERATED_BODY()

public:
	ABotEvadeGameMode();

	/* base functions */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	void SpawnExperimentServiceMonitor();
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	void OnUpdatePreyPosition();
	
	bool StartPositionSamplingTimer(float InRateHz);
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;

	UPROPERTY(VisibleAnywhere)
	TArray<APlayerController*> ClientPlayerControllers;

	UPROPERTY(VisibleAnywhere)
	UEventTimer* EventTimer;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AExperimentClient> ExperimentClient = nullptr;

	UPROPERTY(EditAnywhere)
	int WorldScale = 15.0f;

private:
	TArray<class APlayerStart*> FreePlayerStarts;
};
