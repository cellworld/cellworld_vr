#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ExperimentPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class EXPERIMENTPLUGIN_API AExperimentPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

};
