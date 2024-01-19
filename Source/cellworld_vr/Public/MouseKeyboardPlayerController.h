#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PawnMain.h"
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

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	void QuitGame();
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	void Jump();

	APawnMain* PossessedPawn;

};
