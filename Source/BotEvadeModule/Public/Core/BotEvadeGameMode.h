// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BotEvadeModule/Public/CharacterComponents/PlayerCharacterDesktop.h"
#include "BotEvadeModule/Public/CharacterComponents/PlayerCharacterVR.h"
#include "BotEvadeGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBotEvadeGameMode, Log, All);

/**
 * 
 */
UCLASS()
class BOTEVADEMODULE_API ABotEvadeGameMode : public AGameMode {
	GENERATED_BODY()

public:
	ABotEvadeGameMode();

	/* base functions */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/* testing */
};
