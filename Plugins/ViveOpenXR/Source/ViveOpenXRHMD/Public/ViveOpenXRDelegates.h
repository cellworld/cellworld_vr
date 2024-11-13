// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Define delegates for ViveOpenXR. The common usage is to broadcast events in IOpenXRExtensionPlugin::OnEvent(...).
 * Register the corresponding delegates of UViveOpenXRNotificationsComponent to FViveOpenXRDelegates for letting developer receive events in blueprint via ViveOpenXRNotificationsComponent.
 */
class VIVEOPENXRHMD_API FViveOpenXRDelegates
{
public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDisplayRefreshRateChanged, const float, const float);
	static FDisplayRefreshRateChanged DisplayRefreshRateChanged;

private:
	FViveOpenXRDelegates() {};
};
