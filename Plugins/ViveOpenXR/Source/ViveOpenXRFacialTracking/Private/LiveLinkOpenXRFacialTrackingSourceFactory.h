// Copyright HTC Corporation. All Rights Reserved.

#pragma once

#include "LiveLinkSourceFactory.h"
#include "LiveLinkOpenXRFacialTrackingSourceFactory.generated.h"

UCLASS()
class ULiveLinkOpenXRFacialTrackingSourceFactory : public ULiveLinkSourceFactory
{
public:

	GENERATED_BODY()

	virtual FText GetSourceDisplayName() const;
	virtual FText GetSourceTooltip() const;
	virtual EMenuType GetMenuType() const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;
};