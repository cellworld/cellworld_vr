#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(BlueprintReadWrite, Blueprintable,EditAnywhere,Category="Input|Enhanced Input")
	UInputMappingContext* DefaultMappingContext;
	/* game flow */
	void ResetOrigin();
	void QuitGame();
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void RestartGame();

	UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
	void Client_SetInputModeGameOnly();
	bool Client_SetInputModeGameOnly_Validate();
	void Client_SetInputModeGameOnly_Implementation(); 

	/* pawn to control */
	UPROPERTY(VisibleAnywhere, Replicated)
	AExperimentCharacter* PossessedCharacter;
};
