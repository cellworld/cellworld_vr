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
