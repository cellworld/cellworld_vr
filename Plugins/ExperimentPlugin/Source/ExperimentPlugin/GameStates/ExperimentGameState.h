#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameStateBase.h"
#include "ExperimentGameState.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentGameState : public AGameStateBase {
	GENERATED_BODY()
public:
	AExperimentGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_OnLobbyEntered();
	
};
