#pragma once
#include "ExperimentDoorBase.h"
#include "ExperimentDoorLobby.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentDoorLobby : public AExperimentDoorBase {

	GENERATED_BODY()
public:
	AExperimentDoorLobby();

	virtual void OnValidEventTrigger() override;
	virtual void BeginPlay() override;
	// virtual void Server_OnEventTrigger() override;
	virtual void Server_OnEventTrigger_Implementation() override;
	virtual void OnEventCooldownFinished() override;
};
