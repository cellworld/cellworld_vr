// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BotEvadeGameMode.h"
#include "CharacterComponents/MenuCharacterVR.h"

DEFINE_LOG_CATEGORY(LogBotEvadeGameMode);

ABotEvadeGameMode::ABotEvadeGameMode(){

	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Initializing ABotEvadeGameMode()"))

	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = AMenuCharacterVR::StaticClass();
}

void ABotEvadeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ABotEvadeGameMode::InitGameState()
{
	Super::InitGameState();
}

void ABotEvadeGameMode::StartPlay() {
	Super::StartPlay();
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("StartPlay()"))
}

void ABotEvadeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABotEvadeGameMode::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	UE_LOG(LogBotEvadeGameMode, Log, TEXT("BotEvadeGameMode: Tick"));

	UWorld* World = GetWorld();
	if (!World) {
		UE_LOG(LogBotEvadeGameMode, Error, TEXT("Tick: World is not valid!"))
		return;
	}
	
	FString PlayerName = "NULL";
	
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		const APlayerController* PlayerController = Iterator->Get();
		const int PlayerIdx = Iterator.GetIndex(); 
		if (!PlayerController) {
			UE_LOG(LogBotEvadeGameMode, Error, TEXT("BotEvadeGameMode: Found PlayerController: %s (%i) is not valid!"), *PlayerController->GetName(), Iterator.GetIndex());
		}
			
		PlayerName = PlayerController->GetName();

		UE_LOG(LogBotEvadeGameMode, Log, TEXT("Found PlayerController: %s (%i) is valid!"),
			*PlayerController->GetName(), PlayerIdx);

		if (PlayerController->IsLocalController()) {

			bool bInputComponentActive = false;
			bool bInputEnabled = false; 
			const bool bCharacterIsValid = PlayerController->GetCharacter() != nullptr; // true if not nullptr 
			
			if (bCharacterIsValid) {
				bInputComponentActive = PlayerController->GetCharacter()->InputComponent->IsActive();
				PlayerController->GetCharacter()->InputComponent->Activate();
				bInputEnabled = PlayerController->GetCharacter()->InputEnabled();
			}

			UE_LOG(LogBotEvadeGameMode, Log,
				TEXT("Player Controller %s (%i) is  local player. bInputComponentActive = %i; bInputEnabled = %i; bCharacterIsValid = %i"),
				*PlayerName, PlayerIdx, bInputComponentActive, bInputEnabled, bCharacterIsValid);
		}
	}
}

void ABotEvadeGameMode::OnPostLogin(AController* NewController) {
	Super::OnPostLogin(NewController);
	FString PlayerName = "nullptr";

	// check if valid 
	if (NewController->IsValidLowLevel()) { PlayerName = NewController->GetName(); }

	UE_LOG(LogBotEvadeGameMode, Log, TEXT("OnPostLogin: New player name: %s"),
		*PlayerName)
	
	// check if running on server 
	if (HasAuthority() && NewController->IsLocalController()) {

		// check if the current character is valid; if not, create and possess 
		UE_LOG(LogBotEvadeGameMode, Log, TEXT("AController (%s) is running as server"), *PlayerName)
		AMenuCharacterVR* ServerCharacter = Cast<AMenuCharacterVR>(NewController->GetPawn());
		APlayerController* NewPlayerController = Cast<APlayerController>(NewController);
		
		if (ServerCharacter->IsValidLowLevel() && NewPlayerController->IsValidLowLevel()) {

			UE_LOG(LogBotEvadeGameMode, Log, TEXT("OnPostLogin: (%s) (Server) Spawning AMenuCharacterVR"), *PlayerName )
			AMenuCharacterVR* NewCharacter = GetWorld()->SpawnActor<AMenuCharacterVR>(AMenuCharacterVR::StaticClass());

			if (NewCharacter->IsValidLowLevel()) {
				NewCharacter->InputComponent->SetActive(true);
				NewCharacter->AutoReceiveInput = EAutoReceiveInput::Player0; 
				NewPlayerController->EnableInput(NewPlayerController);
				NewPlayerController->Possess(NewCharacter);
				UE_LOG(LogBotEvadeGameMode, Warning, TEXT("Possessed %s as local controller."), *PlayerName)
			}
			
			// cast AController to APlayerController and enable character's input 
			// if (APlayerController* NewPlayerController = Cast<APlayerController>(NewController)) {
			// 	NewCharacter->EnableInput(NewPlayerController);
			// 	NewPlayerController->Possess(NewCharacter);
			// 	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Enabled input for %s."), *PlayerName)
			// } else {
			// 	UE_LOG(LogBotEvadeGameMode, Log, TEXT("Failed to enable input component for %s."), *PlayerName)
			// }
		} else {
			UE_LOG(LogBotEvadeGameMode, Log, TEXT("OnPostLogin: (%s) (Server) ServerCharacter ALREADY valid"), *PlayerName )
		}
	}
}

