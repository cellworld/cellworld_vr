#pragma once
#include "CoreMinimal.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "GameFramework/GameMode.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/Client/ExperimentClient.h"
#include "ExperimentGameMode.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentGameMode : public AGameMode {
	GENERATED_BODY()
public:
	AExperimentGameMode();

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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AExperimentCharacter> ExperimentCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AExperimentClient> ExperimentClient = nullptr; 
	
	UFUNCTION()
	virtual void SpawnExperimentServiceMonitor();

	UFUNCTION()
	virtual void OnUpdatePreyPosition(const FVector& InLocation, const FRotator& InRotation);
	UFUNCTION()
	virtual void HandleUpdatePosition(FVector InLocation, FRotator InRotation);

	virtual bool StartPositionSamplingTimer(float InRateHz);
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	UFUNCTION(BlueprintCallable)
	bool ExperimentStartEpisode();
	UFUNCTION(BlueprintCallable)
	bool ExperimentStopEpisode();

	UPROPERTY(VisibleAnywhere)
	TArray<APlayerController*> ClientPlayerControllers;

	UPROPERTY(VisibleAnywhere)
	UEventTimer* EventTimer;

	// UPROPERTY(EditAnywhere)
	// TObjectPtr<AExperimentClient> ExperimentClient = nullptr;

	UPROPERTY(EditAnywhere)
	int WorldScale = 5.0f;

private:
	TArray<class APlayerStart*> FreePlayerStarts;
};
