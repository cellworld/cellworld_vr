// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IEyeTrackerModule.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IViveOpenXREyeTrackerModule : public IEyeTrackerModule
{
public:
	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though. Your module might have been
	* unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	bool m_bEnableHTCEyeTracker = false;

	static inline IViveOpenXREyeTrackerModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IViveOpenXREyeTrackerModule>("ViveOpenXREyeTracker");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if
	* IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ViveOpenXREyeTracker");
	}

	virtual FString GetModuleKeyName() const override
	{
		return TEXT("ViveOpenXREyeTracker");
	};
};
