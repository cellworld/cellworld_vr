// Fill out your copyright notice in the Description page of Project Settings.
#include "PredatorController/AIControllerPredator.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavQueryFilter.h"

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

	/* random navigation */
	FVector ResultLocation;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return;
	}
	bool bSuccess = NavSys->K2_GetRandomReachablePointInRadius(GetWorld(), FVector(-2340.0f, -1110.0f, 0.0f), ResultLocation, 2000.0f);

	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), ResultLocation);

}
