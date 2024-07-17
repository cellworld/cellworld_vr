#include "GameModeMain.h"
#include "EngineUtils.h"
#include "PawnMain.h" 
#include "GameStateMain.h"
#include "PawnDebug.h"
#include "PredatorController/AIControllerPredator.h"
#include "AsyncLoadingScreenLibrary.h"
#include "MouseKeyboardPlayerController.h"
#include "PlayerControllerVR.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstanceMain.h"
#include "cellworld_vr/cellworld_vr.h"
#include "Misc/OutputDeviceNull.h"

AGameModeMain::AGameModeMain()
{
	// vr or WASD? 
	if (bUseVR){ PlayerControllerClass = APlayerControllerVR::StaticClass(); }
	else { PlayerControllerClass = AMouseKeyboardPlayerController::StaticClass(); }
	
	DefaultPawnClass = APawnMain::StaticClass();
	GameStateClass = AGameStateMain::StaticClass();
	
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
}

void AGameModeMain::SpawnExperimentServiceMonitor()
{
	if (GetWorld())
	{
		// Define spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Specify the location and rotation for the new actor
		FVector Location(0.0f, 0.0f, 0.0f); // Change to desired spawn location
		FRotator Rotation(0.0f, 0.0f, 0.0f); // Change to desired spawn rotation

		// Spawn the character
		ExperimentServiceMonitor = GetWorld()->SpawnActor<AExperimentServiceMonitor>(AExperimentServiceMonitor::StaticClass(), Location, Rotation, SpawnParams);

		ExperimentServiceMonitor->ExperimentInfo.OnExperimentStatusChangedEvent.AddDynamic(this, &AGameModeMain::OnExperimentStatusChanged);

	}
}

void AGameModeMain::EndGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[ AGameModeMain::EndGame()] Force quit."));
	FGenericPlatformMisc::RequestExit(false);
}

AActor* AGameModeMain::GetLevelActorFromName(const FName& ActorNameIn)
{
	// Assuming this code is within a member function of an actor or game mode
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* FoundActor = *ActorItr;
		if (FoundActor->GetName() == ActorNameIn)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found actor: %s"), *FoundActor->GetName());
			return FoundActor;
		}
	}
	return nullptr;
}

/* 
* Updates GameInstance with HP keys. Will use variables inside GameInstanceMain.h 
* to find, load, and process the HP keys. 
*/

void AGameModeMain::SpawnAndPossessPlayer(FVector spawn_location, FRotator spawn_rotation)
{
	// if (!bUseVR)
	// {
	// 	return;
	// }
	
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("[AGameModeMain::SpawnAndPossessPlayer]!")));

	/* to do: remove, no need for this if player start is present in map. if it isn''t, location will be taken care of by experiment service */

	if(!GetWorld() || !GetWorld()->GetFirstPlayerController()) { UE_DEBUG_BREAK(); return; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	PlayerPawn = GetWorld()->SpawnActor<APawnMain>(DefaultPawnClass, spawn_location, spawn_rotation, SpawnParams);
	if (!PlayerPawn)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("[AGameModeMain::SpawnAndPossessPlayer] Cast failed!")));
		return;
	};

	// Find the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->Possess(PlayerPawn);
	}
}

// todo: should ESMonitor be attached to each individual pawn? 
bool AGameModeMain::AttachClientToPlayer(TObjectPtr<UMessageClient> ClientIn, TObjectPtr<APawnMain> PawnIn)
{
	return false;
}

void AGameModeMain::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AGameModeMain::InitGameState()
{
	Super::InitGameState();
}

void AGameModeMain::SpawnGetCLMonitorComponentActor()
{
	/*FTransform tSpawnTransform;
	FVector TempLoc = { 0.0f, 0.0f, 0.0f };
	FRotator TempRot = tSpawnTransform.GetRotation().Rotator();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetCLMonitorComponentActor = Cast<AGeCLMonitorComponentActor>(GetWorld()->SpawnActor(AGetCLMonitorComponentActor::StaticClass(), &TempLoc, &TempRot, SpawnInfo));*/
}

/* spawn all logging actors, some may contain threads but they handle themselves. 
* right now, there's only one, but im gonna call this function to maintain consitency.
*/

void AGameModeMain::SpawnAllLoggingActor()
{
	/* eye-tracker */
	//AGameModeMain::SpawnGetCLMonitorComponentActor();
}

void AGameModeMain::StartLoadingScreen()
{
	UAsyncLoadingScreenLibrary::SetEnableLoadingScreen(true);
}

void AGameModeMain::StopLoadingScreen()
{
	UAsyncLoadingScreenLibrary::StopLoadingScreen();
}

void AGameModeMain::OnUpdateHUDTimer(){

	float TimeRemaining = -1.0f;
	if (ExperimentServiceMonitor->IsValidLowLevelFast() && ExperimentServiceMonitor->TimerHandle.IsValid()){
		TimeRemaining = ExperimentServiceMonitor->GetTimeRemaining();
	}else{
		TimeRemaining = -1.0f;
		// GetWorldTimerManager().ClearTimer(TimerHUDUpdate);
		// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		// 	FString::Printf(TEXT("[AGameModeMain::OnUpdateHUDTimer()] Error with getting time from ESM.")));
	}

	if (PlayerPawn->IsValidLowLevelFast() && PlayerPawn->PlayerHUD->IsValidLowLevelFast())
	{
		PlayerPawn->PlayerHUD->SetTimeRemaining(FString::FromInt(FMath::FloorToInt(TimeRemaining+1))); // counter the round down
	}else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			FString::Printf(TEXT("[AGameModeMain::OnUpdateHUDTimer()] ERROR setting timer from game mode!.")));
	}

}

void AGameModeMain::OnExperimentStatusChanged(const EExperimentStatus ExperimentStatusIn)
{
	const TEnumAsByte<EExperimentStatus> ExperimentStatusInByted = ExperimentStatusIn;
	const FString EnumAsString = UEnum::GetValueAsString(ExperimentStatusInByted.GetValue());

	int32 Index;
	if (EnumAsString.FindChar(TEXT(':'), Index))
	{
		PlayerPawn->PlayerHUD->SetCurrentStatus(EnumAsString.Mid(Index + 2));
	}
}

void AGameModeMain::OnTimerFinished()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("[AGameModeMain::OnTimerFinished()]!")));
}

void AGameModeMain::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] Starting game!"));
	
	/* spawn player */
	FLocation SpawnLocation;
	SpawnLocation.x = 0.0f;
	SpawnLocation.y = 0.4f;
	FVector SpawnLocationVR = UExperimentUtils::CanonicalToVr(SpawnLocation,235.185,4.0f);
	SpawnLocationVR.Z += 100; 
	this->SpawnAndPossessPlayer(SpawnLocationVR, FRotator::ZeroRotator);
	
	GetWorldTimerManager().SetTimer(TimerHUDUpdate, this, &AGameModeMain::OnUpdateHUDTimer, 0.5f, true, -1.0f);

	// FName NameTemp = "BP_Habitat_Actor_2";
	// AActor* LevelActorBase = this->GetLevelActorFromName(NameTemp);
	// if (LevelActorBase->IsValidLowLevelFast())
	// {
	// 	UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] Found level actor! %s"),*LevelActorBase->GetName());
	// }else{ LevelActorBase = nullptr; }
	
	// UGameInstanceMain* GameInstance = Cast<UGameInstanceMain>(UGameplayStatics::GetGameInstance(GetWorld()));
	// if (GameInstance->IsValidLowLevelFast())
	// {
	// 	const FVector WorldScaleVector = GameInstance->GetLevelScale(nullptr);
	// 	if (WorldScaleVector != FVector::ZeroVector) { GameInstance->SetWorldScale(WorldScaleVector.X); }
	// 	else { UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] WorldScaleVector not valid!")); }
	// 	
	// } else {
	// 	UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] GameInstanceMain NOT found!"));
	// 	GameInstance = nullptr;
	// }
	//
	
	if (bSpawnExperimentService) { AGameModeMain::SpawnExperimentServiceMonitor(); }

	else { UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::StartPlay()] Not spawning Experiment Service!")); }
	
	AGameModeMain::StopLoadingScreen();

}

void AGameModeMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bSpawnExperimentService && this->ExperimentServiceMonitor->IsValidLowLevelFast())
	{
		this->ExperimentStopEpisode();
		this->ExperimentStopExperiment(ExperimentServiceMonitor->ExperimentNameActive);
	}

	// remove all timers from this object
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
}

void AGameModeMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AGameModeMain::ExperimentStartEpisode() { 
	if (!IsValid(ExperimentServiceMonitor)) { return false; }
	return ExperimentServiceMonitor->StartEpisode(ExperimentServiceMonitor->Client,
		ExperimentServiceMonitor->ExperimentInfo.ExperimentNameActive);  
}

bool AGameModeMain::ExperimentStopEpisode() {
	
	if (!IsValid(ExperimentServiceMonitor))
	{ 
		UE_LOG(LogExperiment, Warning, TEXT("[AGameModeMain::ExperimentStopEpisode()] Failed to destroy, Already pending kill.")); 
		return false; 
	}
	return ExperimentServiceMonitor->StopEpisode(); 
}

bool AGameModeMain::ExperimentStopExperiment(const FString ExperimentNameIn)
{
	if (!IsValid(ExperimentServiceMonitor)) { return false; }
	ExperimentServiceMonitor->StopExperiment(ExperimentNameIn);
	return false;
}
