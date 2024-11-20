#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidgetBasic.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UCheckBox; 

UCLASS(Abstract)
class CELLWORLD_VR_API UMainMenuWidgetBasic : public UUserWidget {
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void Init();

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* SpawnExperimentServerCheckBox;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* ResetButton;

	// temp 
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UComboBoxString* PredatorSpeedComboBox;

	// todo
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	USlider* PredatorSpeedSlider;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UComboBoxString* VisualRangeComboBox;

	UFUNCTION()
	void OnSpawnExperimentServerCheckBoxStateChanged(const bool bIsChecked);
	
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

	UFUNCTION()
	void OnPredatorSpeedChanged(const FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnVisualRangeChanged(const FString SelectedItem, ESelectInfo::Type SelectionType);
};
