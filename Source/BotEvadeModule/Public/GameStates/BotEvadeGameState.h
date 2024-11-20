// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BotEvadeGameState.generated.h"

/**
 * 
 */
UCLASS()
class BOTEVADEMODULE_API ABotEvadeGameState : public AGameState
{
	GENERATED_BODY()
public:
	ABotEvadeGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
