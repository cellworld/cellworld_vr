#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"

DEFINE_LOG_CATEGORY(LogMainMenu);

void UMainMenuWidget::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	MatchType = TypeOfMatch;

	NumPublicConnections = NumberOfPublicConnections;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	// bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSubsystem>();
	}
	
	if (MultiplayerSubsystem) {
		MultiplayerSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMainMenuWidget::Initialize() {
	UE_LOG(LogMainMenu, Log, TEXT("Initialize"))
	
	if (!Super::Initialize()) { return false; }

	if (HostButton) {
		UE_LOG(LogMainMenu, Log, TEXT("HostButton bound to clicked event."))
		HostButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HostButtonClicked);
	}

	if (JoinButton) {
		UE_LOG(LogMainMenu, Log, TEXT("JoinButton bound to clicked event."))
		JoinButton->OnClicked.AddDynamic(this, &UMainMenuWidget::JoinButtonClicked);
	}

	if (QuitButton) {
		UE_LOG(LogMainMenu, Log, TEXT("QuitButton bound to clicked event."))
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::QuitButtonClicked);
	}
	return true;
}


void UMainMenuWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	// Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMainMenuWidget::OnCreateSession(bool bWasSuccessful)
{
	UE_LOG(LogMainMenu, Log, TEXT("OnCreateSession"))

	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMainMenuWidget::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	UE_LOG(LogMainMenu, Log, TEXT("OnFindSessions"))

	if (MultiplayerSubsystem == nullptr)
	{
		return;
	}

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMainMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result) {
	UE_LOG(LogMainMenu, Log, TEXT("OnJoinSession"))

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem) {
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid()) {
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController) {
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			} else { UE_LOG(LogMainMenu, Error, TEXT("Player controller not valid!")) }
		}else { UE_LOG(LogMainMenu, Error, TEXT("OnJoinSession: SessionInterface not valid!")) }
	}else {  }
}

void UMainMenuWidget::OnDestroySession(bool bWasSuccessful) {

}

void UMainMenuWidget::OnStartSession(bool bWasSuccessful){
}

void UMainMenuWidget::HostButtonClicked()
{
	UE_LOG(LogMainMenu, Log, TEXT("HostButtonClicked"))
	HostButton->SetIsEnabled(false);
	// if (UMultiplayerSubsystem)
	// {
	// 	UMultiplayerSubsystem->CreateSession(NumPublicConnections, MatchType);
	// }
}

void UMainMenuWidget::JoinButtonClicked()
{
	UE_LOG(LogMainMenu, Log, TEXT("JoinButtonClicked"))

	JoinButton->SetIsEnabled(false);
	// if (UMultiplayerSubsystem)
	// {
	// 	UMultiplayerSubsystem->FindSessions(10000);
	// }
}

void UMainMenuWidget::QuitButtonClicked() {
	UE_LOG(LogMainMenu, Log, TEXT("QuitButtonClicked"))
	FPlatformMisc::RequestExit(true);
}

void UMainMenuWidget::MenuTearDown(){
	
	UE_LOG(LogMainMenu, Log, TEXT("MenuTearDown"))

	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
