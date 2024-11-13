// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IViveOpenXRFacialTrackingModule : public IModuleInterface
{
public:
	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though. Your module might have been
	* unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline IViveOpenXRFacialTrackingModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IViveOpenXRFacialTrackingModule>("ViveOpenXRFacialTracking");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if
	* IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ViveOpenXRFacialTracking");
	}

	virtual TSharedPtr<class ILiveLinkSource> CreateLiveLinkSource() = 0;

	virtual TSharedPtr<class ILiveLinkSource> GetLiveLinkSource() = 0;
	/**
	 * Checks if the LiveLinkSource as been created.
	 *
	 * @return True if the LiveLinkSource has been created with GetLiveLinkSource or AddLiveLinkSource
	 */
	virtual bool IsLiveLinkSourceValid() const = 0;
};
