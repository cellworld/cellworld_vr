#pragma once
#include "ExperimentDoorBase.h"
#include "ExperimentDoorLobby.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentDoorLobby : public AExperimentDoorBase {

	GENERATED_BODY()
public:
	AExperimentDoorLobby();

	virtual void OnValidEventTrigger() override;
	
};
