#pragma once

#include "CoreMinimal.h"
#include "ExperimentPlugin/Characters/ExperimentPawn.h"
#include "ExperimentPlugin/Characters/ExperimentCharacter.h"
#include "GameFramework/PlayerController.h"
#include "ExperimentPlayerControllerVR.generated.h"

/**
 *
 */
UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPlayerControllerVR : public APlayerController
{
	GENERATED_BODY()

public:
	AExperimentPlayerControllerVR();

	/* overrides */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	//AGameModeMain* GameMode;

	/* game flow */
	void ResetOrigin();
	void QuitGame();
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void RestartGame();

	/* pawn to control */
	UPROPERTY(VisibleAnywhere)
	AExperimentCharacter* PossessedCharacter;
};
