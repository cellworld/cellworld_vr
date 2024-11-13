// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ViveOpenXRNotificationsComponent.generated.h"


UCLASS( ClassGroup=(OpenXR), meta=(BlueprintSpawnableComponent) )
class VIVEOPENXRHMD_API UViveOpenXRNotificationsComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDisplayRefreshRateChangedDelegate, const float, FromDisplayRefreshRate, const float, ToDisplayRefreshRate);

	UPROPERTY(BlueprintAssignable)
	FDisplayRefreshRateChangedDelegate DisplayRefreshRateChangedDelegate;

public:
	void OnRegister() override;
	void OnUnregister() override;

private:
	void DisplayRefreshRateChangedDelegate_Handler(const float FromDisplayRefreshRate, const float ToDisplayRefreshRate) { DisplayRefreshRateChangedDelegate.Broadcast(FromDisplayRefreshRate, ToDisplayRefreshRate); }
};
