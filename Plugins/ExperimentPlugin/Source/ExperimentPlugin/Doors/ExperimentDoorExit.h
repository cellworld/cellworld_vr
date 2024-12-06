#pragma once
#include "ExperimentDoorBase.h"
#include "ExperimentDoorExit.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentDoorExit : public AExperimentDoorBase {

	GENERATED_BODY()
public:
	AExperimentDoorExit();

	virtual void BeginPlay() override;
	virtual void OnValidEventTrigger() override;
	virtual void Server_OnEventTrigger_Implementation() override;

};
