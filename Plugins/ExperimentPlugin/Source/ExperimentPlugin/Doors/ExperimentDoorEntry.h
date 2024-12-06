#pragma once
#include "ExperimentDoorBase.h"
#include "ExperimentDoorEntry.generated.h"

UCLASS()
class EXPERIMENTPLUGIN_API AExperimentDoorEntry : public AExperimentDoorBase {

	GENERATED_BODY()
public:
	AExperimentDoorEntry();

	virtual void BeginPlay() override;
	virtual void OnValidEventTrigger() override;
	virtual void Server_OnEventTrigger_Implementation() override;
	virtual void OnEventCooldownFinished() override;
};
