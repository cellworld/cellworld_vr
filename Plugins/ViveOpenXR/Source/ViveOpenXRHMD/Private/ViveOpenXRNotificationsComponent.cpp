// Copyright HTC Corporation. All Rights Reserved.


#include "ViveOpenXRNotificationsComponent.h"
#include "ViveOpenXRDelegates.h"

UViveOpenXRNotificationsComponent::UViveOpenXRNotificationsComponent(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UViveOpenXRNotificationsComponent::OnRegister()
{
	Super::OnRegister();

	FViveOpenXRDelegates::DisplayRefreshRateChanged.AddUObject(this, &UViveOpenXRNotificationsComponent::DisplayRefreshRateChangedDelegate_Handler);
}
void UViveOpenXRNotificationsComponent::OnUnregister()
{
	Super::OnUnregister();

	FViveOpenXRDelegates::DisplayRefreshRateChanged.RemoveAll(this);
}
