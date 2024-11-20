#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/StereoLayerComponent.h"

#include "HUDExperiment.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CELLWORLD_VR_API UCustomStereoLayerComponent : public UStereoLayerComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable) // if bp is needed
	void SetStereoLayerType(EStereoLayerType passedInStereoLayerType) {
		StereoLayerType = passedInStereoLayerType;
	}
};

UCLASS(Abstract)

class CELLWORLD_VR_API UHUDExperiment : public UUserWidget {
	GENERATED_BODY()
public:
	
	/* set remaining time */
<<<<<<< HEAD
	void SetCurrentStatus(const FString& CurrentStatusIn); 
=======
	void SetCurrentStatus(const FString& CurrentStatusIn); // todo; change to take in Enum
	void SetNotificationVisibility(ESlateVisibility InVisibility);
	void SetNotificationText(const FString& InNotification);
>>>>>>> main
	void SetTimeRemaining(const FString& TimeRemainingIn);
	void Init();
	
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere)
	UCustomStereoLayerComponent* StereoLayerComponent;
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UEditableTextBox* TextBoxTimeRemaining;

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UEditableTextBox* TextBoxCurrentStatus;
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UEditableTextBox* TextBoxNotification;

};


