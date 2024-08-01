#include "GameInstanceMain.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineUtils.h"

UGameInstanceMain::UGameInstanceMain()
{
}

void UGameInstanceMain::Init()
{
	Super::Init();


}

void UGameInstanceMain::Shutdown()
{
}

void UGameInstanceMain::StartGameInstance()
{
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

void UGameInstanceMain::SetWorldScale(const float WorldScaleIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::SetWorldScale]: New scale: %f"), WorldScaleIn);
	this->WorldScale = WorldScaleIn; 
}
