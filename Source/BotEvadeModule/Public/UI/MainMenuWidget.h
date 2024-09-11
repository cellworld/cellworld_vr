#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/MultiplayerSubsystem.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMainMenu, Log, All);

UCLASS()
class UMainMenuWidget : public UUserWidget {
	GENERATED_BODY()
public:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/* button delegates (FOnClicked) */
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void JoinByIPAddressButtonClicked();

	UFUNCTION()
	void QuitButtonClicked(); 

	/* setup */
	void MenuTearDown();

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath);

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UButton* HostButton;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UButton* JoinButton;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UButton* QuitButton;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UButton* JoinByIPAddressButton;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	int32 NumPublicConnections{10};

	UPROPERTY()
	class UMultiplayerSubsystem* MultiplayerSubsystem; 

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("FreeForAll")};

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	FString PathToLobby{TEXT("")};
};
