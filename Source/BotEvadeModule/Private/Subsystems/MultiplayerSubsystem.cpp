#include "Subsystems/MultiplayerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h" // required for version > 5.1 for `SEARCH_PRESENCE` macro
#include "OnlineSessionSettings.h"

DEFINE_LOG_CATEGORY(LogMSS);

UMultiplayerSubsystem::UMultiplayerSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem) {
		SessionInterface = Subsystem->GetSessionInterface();
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Blue,
				FString::Printf(TEXT("[UMultiplayerSubsystem::UMultiplayerSubsystem] Found subsystem: %s"),
					*Subsystem->GetSubsystemName().ToString())
			);
		}
	}else {
		UE_LOG(LogMSS, Error, TEXT("Session interface not valid/found!"))
	}
	
}

void UMultiplayerSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType) {
	UE_LOG(LogMSS, Log, TEXT("CreateSession"))
	DesiredNumPublicConnections = NumPublicConnections;
	DesiredMatchType = MatchType;
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,5.f, FColor::Blue,
			FString::Printf(TEXT("[UMultiplayerSubsystem::UMultiplayerSubsystem] Max players =  %i; MatchType = %s "),
				DesiredNumPublicConnections, *DesiredMatchType)
		);
	}
	
	if (!SessionInterface.IsValid()) {
		return;
	}

	FOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		UE_LOG(LogMSS, Warning, TEXT("Active session found, destroying."))
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
	}

	// Store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = true;
	// LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	// LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)) {
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(false); // Broadcast our own custom delegate
	}
}

void UMultiplayerSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid()) {
		UE_LOG(LogMSS, Log, TEXT("FindSessions: SessionInterface not valid!"))
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = true;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer->IsValidLowLevelFast()) {
		UE_LOG(LogMSS, Fatal, TEXT("FindSessions: ULocalPlayer is not valid!"))
		return;
	}
	
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())) {
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		UE_LOG(LogMSS, Log, TEXT("FindSessions: Failed to look for sessions!"))
	}
}

void UMultiplayerSubsystem::JoinSessionAddress(const FString& InIPAddress) {
	UE_LOG(LogMSS, Log, TEXT("[UMultiplayerSubsystem::JoinSessionAddress] Joining HOST via IP: %s"),
		*InIPAddress)

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	// Get the engine and ensure it is not null
	if (!SessionInterface.IsValid()) {
		UE_LOG(LogMSS, Error, TEXT("JoinSession: SessionInterface not valid = Unknown error"))
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	if (!GEngine) {
		UE_LOG(LogMSS, Error, TEXT("Failed to join session via IPAddress. GEngine not valid."))
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	UWorld* World = GetWorld();
	if (!World) {
		UE_LOG(LogMSS, Error, TEXT("Failed to join session via IPAddress. UWorld* World not valid."))
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return; 
	}

	APlayerController* PlayerController = World->GetFirstPlayerController(); 
	if (!PlayerController->IsValidLowLevel() || !PlayerController->IsLocalController()) {
		UE_LOG(LogMSS, Error, TEXT("Failed to join session via IPAddress. Error with GetFirstPlayerController()."))
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return; 
	}
	
	PlayerController->ClientTravel(InIPAddress, ETravelType::TRAVEL_Absolute); 
}

void UMultiplayerSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult) {
	UE_LOG(LogMSS, Log, TEXT("JoinSession"))
	if (!SessionInterface.IsValid()) {
		UE_LOG(LogMSS, Error, TEXT("JoinSession: SessionInterface not valid = Unknown error"))
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		UE_LOG(LogMSS, Log, TEXT("Unknown error joining session!"))
	}
}

void UMultiplayerSubsystem::DestroySession()
{
	UE_LOG(LogMSS, Warning, TEXT("Destroying session!"))
	if (!SessionInterface.IsValid()) {
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession)) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSubsystem::StartSession() {
	UE_LOG(LogMSS, Log, TEXT("StartSession"))
}

void UMultiplayerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
	UE_LOG(LogMSS, Log,
		TEXT("OnCreateSessionComplete: Successful = %i; SessionName = %s"),
		bWasSuccessful, *SessionName.ToString())
	
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Blue,
			FString::Printf(TEXT("OnCreateSessionComplete: bWasSuccessful =  %i; SesssionName = %s"),
				bWasSuccessful, *SessionName.ToString())
		);
	}
	
	if (SessionInterface) {
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSubsystem::OnFindSessionsComplete(bool bWasSuccessful){
	UE_LOG(LogMSS, Log,
		TEXT("OnFindSessionsComplete: Successful = %i"), bWasSuccessful)
	
	if (SessionInterface) {
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	
	if (LastSessionSearch->SearchResults.Num() <= 0) {
		UE_LOG(LogMSS, Log, TEXT("OnFindSessionsComplete: No sessions found!"))
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogMSS, Log,
		TEXT("OnJoinSessionComplete: SessionName = %s"), *SessionName.ToString())

	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogMSS, Log,
		TEXT("OnDestroySessionComplete: Successful = %i; SessionName = %s"),
		bWasSuccessful, *SessionName.ToString())
	
	if (SessionInterface) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	
	if (bWasSuccessful && bCreateSessionOnDestroy) {
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogMSS, Log,
		TEXT("OnStartSessionComplete: Successful = %i; SessionName = %s"),
		bWasSuccessful, *SessionName.ToString())
}
