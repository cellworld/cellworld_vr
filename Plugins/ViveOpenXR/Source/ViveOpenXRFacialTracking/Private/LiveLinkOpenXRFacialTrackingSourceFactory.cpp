// Copyright HTC Corporation. All Rights Reserved.

#include "LiveLinkOpenXRFacialTrackingSourceFactory.h"
#include "IViveOpenXRFacialTrackingModule.h"

#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"

#define LOCTEXT_NAMESPACE "ViveOpenXRFacialTracking"

FText ULiveLinkOpenXRFacialTrackingSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("FacialTrackingLiveLinkSourceName", "Vive OpenXR Facial Tracking Source");
}

FText ULiveLinkOpenXRFacialTrackingSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("FacialTrackingLiveLinkSourceTooltip", "Vive OpenXR Facial Tracking Key Points Source");
}

ULiveLinkOpenXRFacialTrackingSourceFactory::EMenuType ULiveLinkOpenXRFacialTrackingSourceFactory::GetMenuType() const
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		if (!IViveOpenXRFacialTrackingModule::Get().IsLiveLinkSourceValid() || !LiveLinkClient.HasSourceBeenAdded(IViveOpenXRFacialTrackingModule::Get().GetLiveLinkSource()))
		{
			return EMenuType::MenuEntry;
		}
	}
	return EMenuType::Disabled;
}

TSharedPtr<ILiveLinkSource> ULiveLinkOpenXRFacialTrackingSourceFactory::CreateSource(const FString& ConnectionString) const
{
	return IViveOpenXRFacialTrackingModule::Get().GetLiveLinkSource();
}

#undef LOCTEXT_NAMESPACE