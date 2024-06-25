#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDExperiment.generated.h"

UCLASS(Abstract)

class CELLWORLD_VR_API UHUDExperiment : public UUserWidget
{
	GENERATED_BODY()
public:
	
	/* set remaining time */
	void SetCurrentStatus(const FString& CurrentStatusIn); // todo; change to take in Enum
	void SetTimeRemaining(const FString& TimeRemainingIn);
	void Init();
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UEditableTextBox* TextBoxTimeRemaining;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UEditableTextBox* TextBoxCurrentStatus;
};


