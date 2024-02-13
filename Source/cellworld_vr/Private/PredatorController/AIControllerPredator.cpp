// Fill out your copyright notice in the Description page of Project Settings.

#include "PredatorController/AIControllerPredator.h"

AAIControllerPredator::AAIControllerPredator(const FObjectInitializer& ObjectInitializer)
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponentPredatorController"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponentPredatorController"));
}

/* collects informatino from the predator chracter before running behavior tree (AI) */
void AAIControllerPredator::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ACharacterPredator* CharacterPredator = Cast<ACharacterPredator>(InPawn);

	if (CharacterPredator != nullptr && CharacterPredator->BehaviorTreeComponentChar != nullptr) {
		BlackboardComponent->InitializeBlackboard(*CharacterPredator->BehaviorTreeComponentChar->BlackboardAsset);

		TargetKeyID    = BlackboardComponent->GetKeyID("TargetActor");
		TargetLocation = BlackboardComponent->GetKeyID("TargetLocation");

		BehaviorTreeComponent->StartTree(*CharacterPredator->BehaviorTreeComponentChar);

	}
}

void AAIControllerPredator::BeginPlay()
{
	Super::BeginPlay(); 

	if (!Agent) {
		ACharacterPredator* Chr = Cast<ACharacterPredator>(GetPawn());
		if (!Chr) { return; }
		Agent = Chr;
	}

	/* check is SO is assigned to pawn */
	if (Agent->SmartObject) {
		FGameplayTag SubTag;
		BehaviorTreeComponent->SetDynamicSubtree(SubTag, Agent->SmartObject->SubTree);
	}

}
