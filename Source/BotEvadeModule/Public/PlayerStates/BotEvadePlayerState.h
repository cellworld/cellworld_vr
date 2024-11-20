// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BotEvadePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BOTEVADEMODULE_API ABotEvadePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void SetActivePlayer(); 
};
