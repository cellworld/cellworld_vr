// Copyright HTC Corporation All Rights Reserved.

#pragma once

#include "IOpenXRExtensionPlugin.h"
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "OpenXRCore.h"

DECLARE_LOG_CATEGORY_EXTERN(LogViveOpenXRCosmosController, Log, All);

class FViveOpenXRCosmosControllerModule : public IModuleInterface, public IOpenXRExtensionPlugin
{
public:
	/************************************************************************/
	/* IModuleInterface                                                     */
	/************************************************************************/
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/************************************************************************/
	/* IOpenXRExtensionPlugin                                               */
	/************************************************************************/
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;

private:
	bool m_bEnableCosmosController = true;
};

