// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/BotEvadeGameState.h"
ABotEvadeGameState::ABotEvadeGameState() {
	bReplicates = true;
}

void ABotEvadeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// todo: add variables for replication
	//	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}