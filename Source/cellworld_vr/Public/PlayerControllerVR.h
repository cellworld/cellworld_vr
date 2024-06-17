#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PawnMain.h"
#include "PlayerControllerVR.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLD_VR_API APlayerControllerVR : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerControllerVR();

	/* overrides */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void StartExperiment();
	void StartEpisode();
	virtual void Tick(float DeltaTime) override;
	//AGameModeMain* GameMode;

	/* game flow */
	void ResetOrigin();
	void QuitGame();
	void RestartGame();

	/* pawn to control */
	APawnMain* PossessedPawn;

};
