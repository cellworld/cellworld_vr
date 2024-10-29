#pragma once
#include "CoreMinimal.h"
#include "ARSharedWorldGameMode.h"
#include "PredatorController/ExperimentServiceMonitor.h"
#include "GameFramework/GameModeBase.h"
#include "PawnMain.h"
#include "PredatorController/ExperimentServiceMonitor.h"
#include "BotEvadeGameMode.generated.h"

UCLASS()
class ABotEvadeGameMode : public AARSharedWorldGameMode  {
		GENERATED_UCLASS_BODY()
public:
	ABotEvadeGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	void SpawnExperimentServiceMonitor();
	/* functions for door and experiment control */

	UPROPERTY(EditAnywhere)
	bool bUseVR = true;
	
	UPROPERTY(EditAnywhere)
	float WorldScale = 5.0f;
	
	UPROPERTY()
	bool bSpawnExperimentService = false;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AExperimentServiceMonitor> ExperimentServiceMonitor = nullptr;

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool ExperimentStartEpisode(); 

	UFUNCTION(BlueprintCallable, Category = Experiment)
	bool ExperimentStopEpisode();
	
private:
	virtual void Tick(float DeltaSeconds) override;
	virtual void Logout(AController* Exiting) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EndMatch() override;
	virtual void StartPlay() override;
};
