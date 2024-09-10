#include "Core/BotEvadeGameInstance.h"

UBotEvadeGameInstance::UBotEvadeGameInstance() {
	// Log a message indicating the constructor is being called
	UE_LOG(LogTemp, Warning, TEXT("Game Instance Constructor"));

	// Find the main menu widget class
	// const ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
	// const ConstructorHelpers::FClassFinder<UUserWidget> OptionsMenuBPClass(TEXT("/Game/MenuSystem/WBP_OptionsMenu"));

	// Ensure that the menu widget class is found
	// if (!ensure(MenuBPClass.Class != nullptr)) return;
	// if (!ensure(OptionsMenuBPClass.Class != nullptr)) return;

	// Set the found menu widget class to MenuClass
	// MenuClass = MenuBPClass.Class;
	// OptionsMenuClass = OptionsMenuBPClass.Class;
}

void UBotEvadeGameInstance::Init() {
	Super::Init();

}

void UBotEvadeGameInstance::MainMenu() {}

void UBotEvadeGameInstance::LoadMainMenu() {}

void UBotEvadeGameInstance::Host(const FString& ServerName) {}

void UBotEvadeGameInstance::JoinByAddress(const FString& Address) {}

void UBotEvadeGameInstance::JoinNetServer(const uint32 ServerIndex) {}

void UBotEvadeGameInstance::RefreshServerList() {}

void UBotEvadeGameInstance::ClearServerList() {}
