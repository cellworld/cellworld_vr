
#include "HUDExperiment.h"

#include "cellworld_vr/cellworld_vr.h"
#include "Components/EditableTextBox.h"

void UHUDExperiment::NativeConstruct() {
	Super::NativeConstruct();
	this->Init();
}

void UHUDExperiment::SetTimeRemaining(const FString& TimeRemainingIn) {
	TextBoxTimeRemaining->SetText(FText::FromString(TimeRemainingIn));
}

void UHUDExperiment::Init() {
	UE_LOG(LogExperiment, Log, TEXT("UHUDExperiment::Init"))
	SetTimeRemaining(FString("N/A"));
	SetCurrentStatus(FString("Init"));

	/* Notification inits */
	SetNotificationText("Experiment Complete! Press `Right Menu Button` or contact experimenter.");
	SetNotificationVisibility(ESlateVisibility::Hidden);
	// TextBoxNotification->SetIsEnabled(false);
	TextBoxNotification->SetIsReadOnly(true);
	TextBoxNotification->SetJustification(ETextJustify::Center);
	TextBoxNotification->SetForegroundColor(FLinearColor::Black);
	
}

void UHUDExperiment::SetCurrentStatus(const FString& CurrentStatusIn)
{
	TextBoxCurrentStatus->SetText(FText::FromString(CurrentStatusIn));
}

void UHUDExperiment::SetNotificationVisibility(const ESlateVisibility InVisibility) {
	if (!TextBoxNotification) {
		UE_LOG(LogExperiment, Error, TEXT("TextBoxNotification not valid!"))
		return; 
	}
	TextBoxNotification->SetVisibility(InVisibility);
}

void UHUDExperiment::SetNotificationText(const FString& InNotification) {
	if (!TextBoxNotification) {
		UE_LOG(LogExperiment, Error, TEXT("TextBoxNotification not valid!"))
		return; 
	}
	TextBoxNotification->SetText(FText::FromString(InNotification));
}