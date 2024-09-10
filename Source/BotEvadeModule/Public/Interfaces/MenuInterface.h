#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UMenuInterface : public UInterface {
	GENERATED_BODY()
};

class IMenuInterface {
	GENERATED_BODY()

public:
	// virtual bool Test() = 0; 
	virtual void Host(const FString& ServerName) = 0;
	virtual void JoinByAddress(const FString& Address) = 0;
	virtual void JoinNetServer(const uint32 ServerIndex) = 0;
	virtual void RefreshServerList() = 0;
	virtual void ClearServerList() = 0;
	virtual void LoadMainMenu() = 0;
};
