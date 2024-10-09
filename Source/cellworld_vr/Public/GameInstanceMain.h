#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Array.h" 
#include "GameModeMain.h"
#include "ExperimentPlugin/Public/Structs.h"
#include "GameInstanceMain.generated.h"

/**
 * This file controls variables that need to be present from start to finish of entire experiment. 
 * Variables in UGameInstance are present (and accessible if public) during the whole runtime of 
 * the game/experiment executable. If you want variables that die after a game mode ends (or a trial 
 * for example) refer to GameState.
 */
UCLASS()
class CELLWORLD_VR_API UGameInstanceMain : public UGameInstance
{
	GENERATED_BODY()
public:

	UGameInstanceMain();

	AGameModeMain* GameMode = nullptr;

	/* store main pawn for long term acces basically */

	/* override functions frmo UGameInstance */
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;
	AActor* GetLevelActorFromTag(const FName& TagIn);

	/* Experiment Parameters */
	TSharedPtr<FExperimentParameters> ExperimentParameters; 
	
	/* can we go to next phase? terminates widget transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanTransition = 0;

	/* controller mode selection (VR or Mouse + Keyboard) */

	// AActor* GetLevelActorFromName(const FName& NameIn);
	FVector GetLevelScale(const AActor* LevelActor);
	void OpenLevel(const FString& InLevelName);
	void SetWorldScale(float WorldScaleIn);
	UPROPERTY(EditAnywhere)
	float WorldScale = 5.0f;
};