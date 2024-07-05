#include "GameInstanceMain.h"
#include "EngineUtils.h"

UGameInstanceMain::UGameInstanceMain()
{
	/* directories (dir_)*/
	dir_project = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	dir_setup   = dir_project + "Setup/";
	dir_savedata = dir_project + "Data/";

	/* filepaths (file_)*/
	file_hp_client_keys = dir_setup + "HP_client_keys.csv";
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

// AActor* UGameInstanceMain::GetLevelActorFromName(const FName& NameIn)
// {
// 	// Assuming this code is within a member function of an actor or game mode
// 	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
// 	{
// 		AActor* FoundActor = *ActorItr;
// 		if (FoundActor->GetName() == NameIn)
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Found actor: %s"), *FoundActor->GetName());
// 			return FoundActor;
// 		}
// 	}
// 	return nullptr;
// }

const FVector UGameInstanceMain::GetLevelScale(AActor* LevelActor)
{
	UE_LOG(LogExperiment,Fatal,TEXT("DO NOT USE ME!"));
	return FVector::ZeroVector;
	// if (LevelActor->IsValidLowLevelFast())
	// {
	// 	return LevelActor->GetActorScale3D();
	// }
	// else
	// {
	// 	LevelActor = this->GetLevelActorFromName(FName("BP_Habitat_Actor_2"));
	// } 
	//
	// if (!LevelActor->IsValidLowLevelFast())
	// {
	// 	return FVector::ZeroVector;
	// } 
	// else{ return LevelActor->GetActorScale3D();} 
}

void UGameInstanceMain::SetWorldScale(const float WorldScaleIn)
{
	UE_LOG(LogTemp, Warning, TEXT("[UGameInstanceMain::SetWorldScale]: New scale: %f"), WorldScaleIn);
	this->WorldScale = WorldScaleIn; 
}


