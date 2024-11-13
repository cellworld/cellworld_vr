// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "IInputDeviceModule.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IViveOpenXRWristTrackerModule : public IInputDeviceModule
{
public:
	static FName ViveOpenXRWristTrackerModularKeyName;

	/** Returns modular key name for this module */
	static FName GetModuleKeyName()
	{
		return ViveOpenXRWristTrackerModularKeyName;
	}
};
