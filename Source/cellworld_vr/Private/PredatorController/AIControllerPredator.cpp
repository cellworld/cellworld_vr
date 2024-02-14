// Fill out your copyright notice in the Description page of Project Settings.
#include "PredatorController/AIControllerPredator.h"
//#include "PredatorController/PatrolPath.cpp"
#include "NavigationSystem.h"
#include "Runtime/AIModule/Classes/Perception/AISenseConfig_Sight.h"

AAIControllerPredator::AAIControllerPredator(const FObjectInitializer& ObjectInitializer)
{

	// Initialize the behavior tree and blackboard references
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponentPredatorController"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponentPredatorController"));

	//// Find and store the behavior tree asset
	//static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeObj(TEXT("BehaviorTree'/Game/AIPredator/BehaviorTree_Predator_BP.BehaviorTree_Predator_BP'"));
	//if (BehaviorTreeObj.Succeeded())
	//{
	//	//BehaviorTreeComponent = BehaviorTreeObj.Class; // Store the found behavior tree in a UBehaviorTree* member variable
	//}
	//	
	//// Find and store the blackboard data asset
	//static ConstructorHelpers::FObjectFinder<UBlackboardData> BlackboardDataObj(TEXT("BlackboardData'/Game/AIPredator/Blackboard_Predator_BP.Blackboard_Predator_BP'"));
	//if (BlackboardDataObj.Succeeded())
	//{
	//	//BlackboardComponent = BlackboardDataObj.Class; // Store the found blackboard data in a UBlackboardData* member variable
	//}

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	//Create a Sight And Hearing Sense
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	Sight->SightRadius = 1000.f;
	Sight->LoseSightRadius = Sight->SightRadius + 800.f;
	Sight->PeripheralVisionAngleDegrees = 85.0f;

	//Tell the senses to detect everything
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->SetMaxAge(0.2); // after 4 seconds of not seeing me, start patrolling again bro

	//Register the sight sense to our Perception Component
	AIPerceptionComponent->ConfigureSense(*Sight);
	AIPerceptionComponent->SetDominantSense(Sight->GetSenseImplementation());

}

/* collects informatino from the predator chracter before running behavior tree (AI) */
void AAIControllerPredator::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ACharacterPredator* CharacterPredator = Cast<ACharacterPredator>(InPawn);
	//BehaviorTreeComponent->SetDynamicSubtree(BehaviorTreeObj)
	if (CharacterPredator != nullptr && CharacterPredator->BehaviorTreeComponentChar != nullptr) {
		BlackboardComponent->InitializeBlackboard(*CharacterPredator->BehaviorTreeComponentChar->BlackboardAsset);

		TargetKeyID    = BlackboardComponent->GetKeyID("TargetActor");
		TargetLocation = BlackboardComponent->GetKeyID("TargetLocation");

		BehaviorTreeComponent->StartTree(*CharacterPredator->BehaviorTreeComponentChar);
	}
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerPredator::OnPerception);

	///* bind Onperception, gets called automatically when other actor detected */

	//UBehaviorTree* BehaviorTree; 
	//if (BehaviorTreeObj.Succeeded())
	//{
	//	BehaviorTree = BehaviorTreeObj.Object;
	//}
	//UBlackboardData* BlackBoardData;

	//if (BlackBoardData)
	//{
	//	BlackboardComponent->InitializeBlackboard(BlackBoardData);
	//}

	//if (BehaviorTree)
	//{
	//	RunBehaviorTree(BehaviorTree); // This automatically uses the BehaviorTreeComponent
	//}
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

	//NewLocation = GenerateRandomPredatorPath();
	//this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), NewLocation);

}

FVector AAIControllerPredator::GenerateRandomPredatorPath() {
	FVector ResultLocation; 
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return FVector::ZeroVector;
	}
	NavSys->K2_GetRandomReachablePointInRadius(GetWorld(), FVector(-2340.0f, -1110.0f, 0.0f), ResultLocation, 2000.0f);
	return ResultLocation; 
}

bool AAIControllerPredator::CheckIfLocationIsValid(FVector Location)
{

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Error,TEXT("[AAIControllerPredator::CheckIfLocationIsValid(FVector Location)] Navigation System NULL."))
		return false;
	}

	const ANavigationData* NavData = NavSys->GetNavDataForProps(this->GetNavAgentPropertiesRef());
	if (!NavData)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAIControllerPredator::CheckIfLocationIsValid(FVector Location)] Navigation Data NULL."))
		return false;
	}

	FPathFindingQuery Query(this, *NavData, this->GetNavAgentLocation(), Location);
	if (!NavSys->TestPathSync(Query))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAIControllerPredator::CheckIfLocationIsValid(FVector Location)] Navigation path NULL."))
		return false;
	}

	return true;
}

/* check status of chase */
void AAIControllerPredator::Tick(float DeltaTime)
{
	/* if currently following target, let the predator continue its thing */
	if (StimulusLog.GetAge() >= Sight->GetMaxAge() || StimulusLog.GetAge() == 0.0f) {
		bFollowingTarget = false; 
	}

	if (!bFollowingTarget) {
		NewLocation = GenerateRandomPredatorPath();
	}

}

/* detects another character. Body of this function handles it (for now).
* eventually, another function will do the processing 
*/
void AAIControllerPredator::OnPerception(AActor* Actor, FAIStimulus Stimulus)
{
	bFollowingTarget = true; 
	StimulusLog = Stimulus; 

	APawn* Chr = Cast<APawn>(Actor);
	if (Chr == nullptr) { 
		return; 
	}

	SetFocus(Stimulus.WasSuccessfullySensed() ? Chr : nullptr);

	/* update target location */
	FVector actor_location_vector_last = Actor->GetActorLocation();
	FNavLocation new_location;

	//Set you NavAgentProps properties here (radius, height, etc)
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) {
		UE_DEBUG_BREAK();
		return;
	}

	bool bProjectedLocationValid = NavSys->ProjectPointToNavigation(actor_location_vector_last, new_location, FVector::ZeroVector);
	if (!bProjectedLocationValid) {
		UE_DEBUG_BREAK(); 
	}
	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location.Location);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("projected location: %i "), bProjectedLocationValid));


}
