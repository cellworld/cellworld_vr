#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/MenuInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BotEvadeGameInstance.generated.h"

// class UOptionsMenu;
// class UMainMenu;

UCLASS()
class UBotEvadeGameInstance : public UGameInstance, public IMenuInterface {

	GENERATED_BODY()
	
	UBotEvadeGameInstance();

public:
	/* UGameInstance overrides */
	virtual void Init() override;	

	/* new functions */
	TSubclassOf<UUserWidget> MenuClass;
	TSubclassOf<UUserWidget> OptionsMenuClass;
	// TObjectPtr<UMainMenu> Menu;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY()
	FString GameServerName;
	
	/* IMenuInterface overrides */
	UFUNCTION(Exec, BlueprintCallable)
	void MainMenu();

	UFUNCTION(Exec, BlueprintCallable)
	virtual void LoadMainMenu() override;

	UFUNCTION(Exec)
	virtual void Host(const FString& ServerName) override;

	// Join Server by IP address
	UFUNCTION(Exec)
	virtual void JoinByAddress(const FString& Address) override;

	// Join Server by Name
	UFUNCTION(Exec)
	virtual void JoinNetServer(const uint32 ServerIndex) override;
	
	UFUNCTION(Exec)
	virtual void RefreshServerList() override;

	UFUNCTION(Exec)
	virtual void ClearServerList() override;

};
