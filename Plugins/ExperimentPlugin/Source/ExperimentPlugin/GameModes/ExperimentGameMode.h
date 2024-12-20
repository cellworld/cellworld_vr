#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/Client/ExperimentClient.h"
#include "ExperimentPlugin/HabitatComponents/Habitat.h"
#include "ExperimentGameMode.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentGameMode : public AGameModeBase {
	GENERATED_BODY()
public:
	AExperimentGameMode();

	/* == aactor overrides == */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void UpdateNetOwnerHabitat(AController* InHabitatNetOwnerController, bool bForceUpdate);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	// virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/* == temp | debug */
	UPROPERTY(/*Replicated*/)
	TObjectPtr<APawn> PawnToRegister = nullptr;
	UPROPERTY(/*Replicated*/)
	TObjectPtr<AController> NewPC = nullptr;
	
	/* == helpers == */
	
	UPROPERTY(EditDefaultsOnly,Category="Habitat Spawning")
	TSubclassOf<AHabitat> HabitatBPClass;
	
	TObjectPtr<AHabitat> FindHabitatInLevel() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AHabitat> Habitat;
	
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
	// virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	UFUNCTION(BlueprintCallable)
	bool ExperimentStartEpisode();
	UFUNCTION(BlueprintCallable)
	bool ExperimentStopEpisode();

	UPROPERTY(VisibleAnywhere)
	TArray<APlayerController*> ClientPlayerControllers;

	UPROPERTY(VisibleAnywhere)
	UEventTimer* EventTimer;

	UFUNCTION(BlueprintCallable)
	void SpawnHabitat(const FVector& InLocation, const int& InScale);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    UClass* MyActorClass;
	// UPROPERTY(EditAnywhere)
	// TObjectPtr<AExperimentClient> ExperimentClient = nullptr;

	UPROPERTY(EditAnywhere)
	int WorldScale = 5.0f;

private:
	TArray<class APlayerStart*> FreePlayerStarts;
};
