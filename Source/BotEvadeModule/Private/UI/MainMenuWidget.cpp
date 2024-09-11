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
	
	if (JoinByIPAddressButton) {
		UE_LOG(LogMainMenu, Log, TEXT("QuitButton bound to clicked event."))
		JoinByIPAddressButton->OnClicked.AddDynamic(this, &UMainMenuWidget::JoinByIPAddressButtonClicked);
	}
	
	return true;
}

void UMainMenuWidget::NativeDestruct() {
	MenuTearDown();
	Super::NativeDestruct();
}

void UMainMenuWidget::OnCreateSession(bool bWasSuccessful) {
	
	PathToLobby = "/Game/Test_OnlineSubsystem/L_Game?listen";
	UE_LOG(LogMainMenu, Log, TEXT("OnCreateSession: bWasSuccessful = %i; PathToLobby: = %s"),
		bWasSuccessful, *PathToLobby)

	// only travel is bWasSuccessful is true 
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString(TEXT("MainMenuWidget::OnCreateSession: Session created successfully!"))
			);
		}
		UWorld* World = GetWorld();
		if (World) {
			World->ServerTravel(PathToLobby);
		}
	} else {
		UE_LOG(LogMainMenu, Log, TEXT("MainMenuWidget::OnCreateSession: Failed to create session!"))
		if (GEngine) {
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

void UMainMenuWidget::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful) {
	UE_LOG(LogMainMenu, Log, TEXT("OnFindSessions"))

	if (MultiplayerSubsystem == nullptr) {
		UE_LOG(LogMainMenu, Error, TEXT("OnFindSessions: MultiplayerSubsystem is null"))
		return;
	}

	const int NumSessionsFound = SessionResults.Num();
	const FString msg = "Sessions found: " + FString::FromInt(NumSessionsFound);

	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1,5.f, FColor::Blue, msg); }
	UE_LOG(LogMainMenu, Warning, TEXT("OnFindSessions: Sessions found: %i"), NumSessionsFound)

	for (auto Result : SessionResults) {

		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		UE_LOG(LogMainMenu, Log, TEXT("Found a game session!"))
		UE_LOG(LogMainMenu, Log, TEXT("Found settings: %s"), *SettingsValue)
		
		if (SettingsValue == MatchType) {
			MultiplayerSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SessionResults.Num() == 0) {
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
			
			UE_LOG(LogMainMenu, Log, TEXT("UMainMenuWidget::OnJoinSession: Address = %s"),
				*Address)

			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController) {
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			} else { UE_LOG(LogMainMenu, Error, TEXT("Player controller not valid!")) }
		}else { UE_LOG(LogMainMenu, Error, TEXT("OnJoinSession: SessionInterface not valid!")) }
	}else {  }
}

void UMainMenuWidget::OnDestroySession(bool bWasSuccessful) {
	UE_LOG(LogMainMenu, Warning, TEXT("OnDestroySession!"))
}

void UMainMenuWidget::OnStartSession(bool bWasSuccessful) {

}

void UMainMenuWidget::HostButtonClicked() {
	UE_LOG(LogMainMenu, Log, TEXT("HostButtonClicked"))
	HostButton->SetIsEnabled(false);
	const int NumMaxPlayers = 100;
	
	UE_LOG(LogMainMenu, Log, TEXT("HostButtonClicked: Max players= %i ; MatchType = %s "),
		NumMaxPlayers, *MatchType);
	
	if (MultiplayerSubsystem) {
		MultiplayerSubsystem->CreateSession(NumMaxPlayers, MatchType);
	}
}

void UMainMenuWidget::JoinButtonClicked()
{
	UE_LOG(LogMainMenu, Log, TEXT("JoinButtonClicked"))

	JoinButton->SetIsEnabled(false);
	if (MultiplayerSubsystem) {
		UE_LOG(LogMainMenu, Log, TEXT("Looking for 10,000 sessions!"))
		MultiplayerSubsystem->FindSessions(10000);
	}
}

void UMainMenuWidget::JoinByIPAddressButtonClicked() {
	UE_LOG(LogMainMenu, Log, TEXT("JoinByIPAddressButtonClicked"))
	JoinByIPAddressButton->SetIsEnabled(false);
	if (MultiplayerSubsystem) {
		UE_LOG(LogMainMenu, Log, TEXT("Joining session via IPAddress!"))
		MultiplayerSubsystem->JoinSessionAddress("192.168.1.199"); // todo: make this modifiable
	}
}

void UMainMenuWidget::QuitButtonClicked() {
	UE_LOG(LogMainMenu, Log, TEXT("QuitButtonClicked"))
	FPlatformMisc::RequestExit(true);
}

void UMainMenuWidget::MenuTearDown(){
	
	UE_LOG(LogMainMenu, Log, TEXT("MenuTearDown"))

	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
