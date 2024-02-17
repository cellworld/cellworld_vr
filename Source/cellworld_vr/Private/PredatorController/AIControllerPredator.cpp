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
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	// look for components based in BP 
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeAssetLoad(TEXT("BehaviorTree'/Game/AIPredator/Patrolling_BP.Patrolling_BP'"));
	if (BehaviorTreeAssetLoad.Succeeded()) {
		BehaviorTreeAsset = BehaviorTreeAssetLoad.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BlackboardAssetLoad(TEXT("BlackboardData'/Game/AIPredator/Blackboard_Predator_BP.Blackboard_Predator_BP'"));
	if (BlackboardAssetLoad.Succeeded()) {
		BlackboardAsset = BlackboardAssetLoad.Object;
	}

	//Create a Sight And Hearing Sense
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	Sight->SightRadius = 1200.f;
	Sight->LoseSightRadius = Sight->SightRadius + 400.f;
	Sight->PeripheralVisionAngleDegrees = 100.0f;

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
	if (CharacterPredator != nullptr && CharacterPredator->BehaviorTreeComponentChar != nullptr) {
		
		CharacterPredator->BehaviorTreeComponentChar->BlackboardAsset = BlackboardAsset;
		BlackboardComponent->InitializeBlackboard(*CharacterPredator->BehaviorTreeComponentChar->BlackboardAsset);

		TargetKeyID    = BlackboardComponent->GetKeyID("TargetActor");
		TargetLocation = BlackboardComponent->GetKeyID("TargetLocation");

		BehaviorTreeComponent->StartTree(*CharacterPredator->BehaviorTreeComponentChar);
	}
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerPredator::OnPerception);

}

/* begins a random walk */
void AAIControllerPredator::StartPatrol() 
{
	new_location = this->GenerateRandomPredatorPath();
	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location);
}

void AAIControllerPredator::SetNewLocation(FVector Location) {
	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), Location);
}

/* begins following target */
void AAIControllerPredator::StartFollowingTarget()
{
	/* update target location */
	FVector actor_location_vector_last = ActorLastPerceived->GetActorLocation();
	FNavLocation new_location_nav;

	//Set you NavAgentProps properties here (radius, height, etc)
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) {
		UE_DEBUG_BREAK();
		return;
	}

	bool bProjectedLocationValid = NavSys->ProjectPointToNavigation(actor_location_vector_last, new_location_nav, FVector::ZeroVector);
	if (!bProjectedLocationValid) {
		UE_DEBUG_BREAK();
	}

	/* actual movement */
	this->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), new_location_nav.Location);
}

void AAIControllerPredator::BeginPlay()
{
	Super::BeginPlay(); 

	this->StartPatrol();

	if (!this->SpawnExperimentServiceMonitor()) {
		GEngine->AddOnScreenDebugMessage(-1, 0.5, FColor::Red,TEXT("Failed to spawn ExperimentServiceMonitor"));
	}

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

bool AAIControllerPredator::IsTargetInSight(AActor* TargetActor)
{
	if (!TargetActor) return false;

	PerceptionComponent = GetPerceptionComponent();
	if (!PerceptionComponent) return false;

	TArray<AActor*> PerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	// Check if the TargetActor is in the list of perceived actors
	return PerceivedActors.Contains(TargetActor);
}

bool AAIControllerPredator::SpawnExperimentServiceMonitor() {

	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FActorSpawnParameters SpawnInfo;
	ExperimentServiceMonitor = GetWorld()->SpawnActor<AExperimentServiceMonitor>(Location, Rotation, SpawnInfo);
	if (!Cast<AExperimentServiceMonitor>(ExperimentServiceMonitor)) { return false; }
	return true;
}

/* check status of chase */
void AAIControllerPredator::Tick(float DeltaTime)
{
	/* if currently following target, let the predator continue its thing */

	if (!ActorLastPerceived) { return; }
	bool bInSight = this->IsTargetInSight(ActorLastPerceived);

	/* keep following if you still see target */
	if (bInSight) {
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Target is in sight. Following."));
		this->StartFollowingTarget();
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Target is NOT sight. Continuing to final path."));

	if (!this->GetCharacter()) {
		UE_DEBUG_BREAK();
		return;
	}

	/* check if standing still (reached final point or got stuck) */
	float Velocity;
	FVector VelocityVector;
	this->GetCharacter()->GetVelocity().ToDirectionAndLength(VelocityVector, Velocity);
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Velocity: %f"), Velocity));
	if (Velocity == 0) {
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("Reached last known location."));
		this->StartPatrol();
		return;
	}
}
/* detects another character. Body of this function handles it (for now).
* eventually, another function will do the processing 
*/
void AAIControllerPredator::OnPerception(AActor* Actor, FAIStimulus Stimulus)
{
	//bFollowingTarget = true; 
	ActorLastPerceived = Actor; 
	StimulusLog = Stimulus; 

	APawn* Chr = Cast<APawn>(Actor);
	if (Chr == nullptr) { 
		return; 
	}

	SetFocus(Stimulus.WasSuccessfullySensed() ? Chr : nullptr);


	this->StartFollowingTarget();

}
