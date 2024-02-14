// Fill out your copyright notice in the Description page of Project Settings.
#include "PredatorController/AIControllerPredator.h"
#include "NavigationSystem.h"
#include "Runtime/AIModule/Classes/Perception/AISenseConfig_Sight.h"
AAIControllerPredator::AAIControllerPredator(const FObjectInitializer& ObjectInitializer)
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponentPredatorController"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponentPredatorController"));
	
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	//Create a Sight And Hearing Sense
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	Sight->SightRadius = 1000.f;
	Sight->LoseSightRadius = Sight->SightRadius + 400.f;
	Sight->PeripheralVisionAngleDegrees = 60.f;

	//Tell the senses to detect everything
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->SetMaxAge(4); // after 4 seconds of not seeing me, start patrolling again bro

	//Register the sight sense to our Perception Component
	AIPerceptionComponent->ConfigureSense(*Sight);
	AIPerceptionComponent->SetDominantSense(Sight->GetSenseImplementation());

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

	/* bind Onperception, gets called automatically when other actor detected */
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerPredator::OnPerception);
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

	return false;
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
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Detected target."));

	APawn* Chr = Cast<APawn>(Actor);
	if (Chr == nullptr) { return; }

	SetFocus(Stimulus.WasSuccessfullySensed() ? Chr : nullptr);

	/* update target location */
	FVector actor_location_vector_last = Actor->GetActorLocation();
	FNavLocation new_location;
	FVector QueryingExtent = FVector(100.0f, 100.0f, 0.0f);


	//Set you NavAgentProps properties here (radius, height, etc)
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) {
		UE_DEBUG_BREAK();
		return;
	}
	////Set you NavAgentProps properties here (radius, height, etc)

	bool bProjectedLocationValid = NavSys->ProjectPointToNavigation(actor_location_vector_last, new_location, QueryingExtent);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("projected location: %i "), bProjectedLocationValid));
	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location.Location);
	
	/*if (CheckIfLocationIsValid(new_location.Location)) {
		this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location.Location);
	}
	else {
		UE_DEBUG_BREAK();
	}
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("New actor loc: %f, %f, %f "), new_location.Location.X, new_location.Location.Y, new_location.Location.Z));
	}*/
}
