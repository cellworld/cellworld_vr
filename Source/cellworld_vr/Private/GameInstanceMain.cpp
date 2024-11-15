#include "GameInstanceMain.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UGameInstanceMain::UGameInstanceMain() {
	
}

void UGameInstanceMain::Init() {
	Super::Init();
	UE_LOG(LogExperiment, Warning, TEXT("Initializing UGameInstanceMain"))

	ExperimentParameters = MakeShared<FExperimentParameters>();
	ExperimentParameters->bSpawnExperimentService = true;
	
	if (ExperimentParameters) {
		UE_LOG(LogExperiment, Log, TEXT("[UGameInstanceMain::Init] FExperimentParameters Created."))
	}else {
		UE_LOG(LogExperiment, Error, TEXT("[UGameInstanceMain::Init] FExperimentParameters NULL!."))
	}
}

void UGameInstanceMain::Shutdown() {
}

void UGameInstanceMain::StartGameInstance() {
	Super::StartGameInstance();
}

/* get first instance of level actor using tag
 * can return nullptr */
AActor* UGameInstanceMain::GetLevelActorFromTag(const FName& TagIn)
{
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* FoundActor = *ActorItr;
		if (FoundActor->IsValidLowLevelFast())
		{
			if (FoundActor->ActorHasTag(TagIn)) { return FoundActor; } // should only be a single maze instance
		}
	}
	return nullptr;
}

FVector UGameInstanceMain::GetLevelScale(const AActor* LevelActor) {
	
	if (LevelActor->IsValidLowLevelFast()) { return LevelActor->GetActorScale3D(); }
	LevelActor = this->GetLevelActorFromTag(FName("HabitatMaze")); 	// if actor passed in is null, try to get the actor yourself
	if (!LevelActor->IsValidLowLevelFast()) { return FVector::ZeroVector; } 
	return LevelActor->GetActorScale3D(); 
}

void UGameInstanceMain::OpenLevel(const FString& InLevelName) {
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::OpenLevel] Attempting to OpenLevel: `%s`"), *InLevelName);
	UGameplayStatics::OpenLevel(this, FName(*InLevelName));
}

void UGameInstanceMain::SetWorldScale(const float WorldScaleIn) {
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::SetWorldScale]: New scale: %f"), WorldScaleIn);
	this->WorldScale = WorldScaleIn; 
}


// todo: declare as ufunction(blueprintcallable)
bool UGameInstanceMain::ExperimentStartEpisode() {
	UE_LOG(LogExperiment, Log, TEXT("[UGameInstanceMain::ExperimentStartEpisode] Calling StartEpisode()"))

	if (!ensure(IsValid(ExperimentServiceMonitor))) { return false; }
	UE_LOG(LogExperiment, Log, TEXT("[UGameInstanceMain::ExperimentStartEpisode] Calling StartEpisode()"))
	return ExperimentServiceMonitor->StartEpisode();
}

bool UGameInstanceMain::ExperimentStopEpisode() {
	if (!ensure(IsValid(ExperimentServiceMonitor))) {
		UE_LOG(LogExperiment, Warning,
			   TEXT("[UGameInstanceMain::ExperimentStopEpisode] Failed to destroy, Already pending kill."));
		return false;
	}
	UE_LOG(LogExperiment, Log, TEXT("[UGameInstanceMain::ExperimentStopEpisode] Calling StopEpisode(false)"))
	return ExperimentServiceMonitor->StopEpisode(false);
}

bool UGameInstanceMain::ExperimentStopExperiment(const FString ExperimentNameIn) {
	UE_LOG(LogExperiment, Log, TEXT("[UGameInstanceMain::ExperimentStopEpisode] Calling StopEpisode(false)"))

	if (!IsValid(ExperimentServiceMonitor)) { return false; }
	ExperimentServiceMonitor->StopExperiment(ExperimentNameIn);
	return false;
}

// todo: declare as ufunction(blueprintcallable)
bool UGameInstanceMain::SpawnExperimentServiceMonitor(UWorld* InWorld) {
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::SpawnExperimentServiceMonitor] Called"));
	
	// ReSharper disable once CppLocalVariableMayBeConst
	ESpawnActorCollisionHandlingMethod CollisionHandlingMethod =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
	FTransform SpawnTransformExperimentServiceMonitor = {};
	SpawnTransformExperimentServiceMonitor.SetLocation(FVector::ZeroVector);
	SpawnTransformExperimentServiceMonitor.SetRotation(FRotator::ZeroRotator.Quaternion());

	ExperimentServiceMonitor = GetWorld()->SpawnActorDeferred<AExperimentServiceMonitor>(
		AExperimentServiceMonitor::StaticClass(), SpawnTransformExperimentServiceMonitor, nullptr, nullptr, CollisionHandlingMethod);
	ExperimentServiceMonitor->WorldScale = this->WorldScale;
	ExperimentServiceMonitor->FinishSpawning(SpawnTransformExperimentServiceMonitor);

	check(ExperimentServiceMonitor->IsValidLowLevelFast())
	if (!ensure(ExperimentServiceMonitor->IsValidLowLevelFast())) { return false; }
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::SpawnExperimentServiceMonitor] Exiting!"));
	return true;
}