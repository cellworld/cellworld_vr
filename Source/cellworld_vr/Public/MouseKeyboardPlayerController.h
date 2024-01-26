#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PawnDebug.h"
#include "MouseKeyboardPlayerController.generated.h"

/**
 *
 */
UCLASS()
class CELLWORLD_VR_API AMouseKeyboardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMouseKeyboardPlayerController();

	/* overrides */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	/* game flow */
	void ResetOrigin();
	void QuitGame();
	void RestartGame();

	/* movement */
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	void Jump();

	/* pawn to control */
	APawnDebug* PossessedPawn;

};
