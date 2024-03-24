

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Actor.h"
#include "GameStateMain.generated.h"

/**
 * 
 */
UCLASS()
class CELLWORLDVR_API AGameStateMain : public AGameStateBase
{

public: 

	GENERATED_BODY()

	/* game state variable */
	AGameStateMain* MyGameState;

	/* save data location */
	FString SaveDataLocation; 

protected:
	//	Called when the state transitions to InProgress
	virtual void HandleMatchHasStarted(); 
	virtual bool IsMatchInProgress() const; 
	virtual bool HasMatchEnded() const override;
	virtual bool HasMatchStarted() const override;
	virtual void HandleBeginPlay(); 

};
