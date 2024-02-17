// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CharacterPredator.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PredatorController/ExperimentServiceMonitor.h"
#include "UObject/UObjectGlobals.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIControllerPredator.generated.h"

/**
 * 
 */
UCLASS()
class CELLWORLD_VR_API AAIControllerPredator : public AAIController
{
	GENERATED_BODY()

	AAIControllerPredator(const FObjectInitializer& ObjectInitializer);

public: 
	virtual void OnPossess(APawn* InPawn) override; 
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override; 

	void StartFollowingTarget();
	void StartPatrol();
	FVector GenerateRandomPredatorPath();
	FNavLocation NewLocation; 
	FVector new_location;
	AActor* ActorLastPerceived; 
	AExperimentServiceMonitor* ExperimentServiceMonitor; 
	bool IsTargetInSight(AActor* TargetActor);
	bool SpawnExperimentServiceMonitor();
	bool CheckIfLocationIsValid(FVector Location);

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent; 
	//class UBehaviorTree* BehaviorTree;

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComponent;
	//class UBlackboard* Blackboard;

	/* target */
	UPROPERTY(BlueprintReadWrite)
	class ACharacterPredator* Agent;

	UFUNCTION()
	void OnPerception(AActor* Actor, FAIStimulus Stimulus);
	class UAISenseConfig_Sight* Sight; 

	bool bFollowingTarget = false; 
	FAIStimulus StimulusLog; 

	/* pawn sensing stuff */
	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent; 

	// blackboard key id to detect prey (or other target actor)
	uint8 TargetKeyID;
	uint8 TargetLocation;


};
